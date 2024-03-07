/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2014 Ken Sarkies <ksarkies@internode.on.net>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/dac.h>
#include <libopencm3/stm32/dma.h>
#include <signals.h>
/* Timer 2 count period, 16 microseconds for a 72MHz APB2 clock */
#define PERIOD 1

/* Globals */
uint16_t waveform[4096];


/*--------------------------------------------------------------------*/
static void clock_setup(void)
{
	rcc_clock_setup_in_hse_8mhz_out_72mhz();
}

/*--------------------------------------------------------------------*/
static void gpio_setup(void)
{
	/* Port A and C are on AHB1 */
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOD);
	/* Set the digital test output on PD2 */
	gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO2);
	/* Set PA4 for DAC channel 1 to analogue, ignoring drive mode. */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO4);
}

/*--------------------------------------------------------------------*/
static void timer_setup(void)
{
	/* Enable TIM2 clock. */
	rcc_periph_clock_enable(RCC_TIM2);
	rcc_periph_reset_pulse(RST_TIM2);
	/* Timer global mode: - No divider, Alignment edge, Direction up */
	timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT,
		       TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	timer_continuous_mode(TIM2);
	timer_set_period(TIM2, PERIOD);
	timer_disable_oc_output(TIM2, TIM_OC2 | TIM_OC3 | TIM_OC4);
	timer_enable_oc_output(TIM2, TIM_OC1);
	timer_disable_oc_clear(TIM2, TIM_OC1);
	timer_disable_oc_preload(TIM2, TIM_OC1);
	timer_set_oc_slow_mode(TIM2, TIM_OC1);
	timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_TOGGLE);
	timer_set_oc_value(TIM2, TIM_OC1, 1);
	timer_disable_preload(TIM2);
	/* Set the timer trigger output (for the DAC) to the channel 1 output
	   compare */
	timer_set_master_mode(TIM2, TIM_CR2_MMS_COMPARE_OC1REF);
	timer_enable_counter(TIM2);
}

/*--------------------------------------------------------------------*/
static void dma_setup(void)
{
	/* DAC channel 1 uses DMA controller 1 Stream 5 Channel 7. */
	/* Enable DMA2 clock and IRQ */
	rcc_periph_clock_enable(RCC_DMA2);
	nvic_enable_irq(NVIC_DMA2_CHANNEL3_IRQ);
	dma_channel_reset(DMA2, 3);
	dma_set_priority(DMA2, 3, DMA_CCR_PL_VERY_HIGH);
	dma_set_memory_size(DMA2, 3, DMA_CCR_MSIZE_16BIT);
	dma_set_peripheral_size(DMA2, 3, DMA_CCR_PSIZE_16BIT);
	dma_enable_memory_increment_mode(DMA2, 3);
	dma_enable_circular_mode(DMA2, 3);
	dma_set_read_from_memory(DMA2, 3);
	/* The register to target is the DAC1 12-bit right justified data
	   register */
	dma_set_peripheral_address(DMA2, 3, (uint32_t)&DAC_DHR12R1);
	/* The array v[] is filled with the waveform data to be output */
	dma_set_memory_address(DMA2, 3, (uint32_t)waveform);
	dma_set_number_of_data(DMA2, 3, 4095);
	dma_enable_transfer_complete_interrupt(DMA2, 3);
	// dma_channel_select(DMA2, 3, DMA_SxCR_CHSEL_7);
	dma_enable_channel(DMA2, 3);
}

/*--------------------------------------------------------------------*/
static void dac_setup(void)
{
	/* Enable the DAC clock on APB1 */
	rcc_periph_clock_enable(RCC_DAC);
	/* Setup the DAC channel 1, with timer 2 as trigger source.
	 * Assume the DAC has woken up by the time the first transfer occurs */
	dac_trigger_enable(CHANNEL_1);
	dac_set_trigger_source(DAC_CR_TSEL1_T2);
	dac_dma_enable(CHANNEL_1);
	dac_enable(CHANNEL_1);
}

/*--------------------------------------------------------------------*/
/* The ISR simply provides a test output for a CRO trigger */

void dma2_channel3_isr(void)
{
	if (dma_get_interrupt_flag(DMA2, 3, DMA_TCIF))
	{
		dma_clear_interrupt_flags(DMA2, 3, DMA_TCIF);
		/* Toggle PD2 just to keep aware of activity and frequency. */
		gpio_toggle(GPIOD, GPIO2);
		// gpio_set(GPIOD, GPIO2);
	}
}

/*--------------------------------------------------------------------*/
int main(void)
{
	uint32_t p_acc, p_step;
	uint16_t addr = 0; // адрес ячейки

	p_acc = 0;	  // аккумулятор фазы
	p_step = 256; // код частоты

	clock_setup();
	gpio_setup();
	timer_setup();
	dma_setup();
	dac_setup();

	while (1)
	{
		addr = p_acc >> 8; // выделение старшей части аккумулятора фазы
		p_acc += p_step;   // шаг
		if (addr > 4095)
		{
			p_acc = 0;
		} // шаг
		waveform[addr] = lut[addr];
	}

	return 0;
}