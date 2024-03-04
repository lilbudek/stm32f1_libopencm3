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

/* Timer 2 count period, 16 microseconds for a 72MHz APB2 clock */
#define PERIOD 1

/* Globals */
//uint8_t waveform[256];
    uint8_t lut[256] = {
        0x7F, 0x82, 0x85, 0x88, 0x8B, 0x8F, 0x92, 0x95, 0x98, 0x9B,
        0x9E, 0xA1, 0xA4, 0xA7, 0xAA, 0xAD, 0xB0, 0xB2, 0xB5, 0xB8,
        0xBB, 0xBE, 0xC0, 0xC3, 0xC6, 0xC8, 0xCB, 0xCD, 0xD0, 0xD2,
        0xD4, 0xD7, 0xD9, 0xDB, 0xDD, 0xDF, 0xE1, 0xE3, 0xE5, 0xE7,
        0xE9, 0xEA, 0xEC, 0xEE, 0xEF, 0xF0, 0xF2, 0xF3, 0xF4, 0xF5,
        0xF7, 0xF8, 0xF9, 0xF9, 0xFA, 0xFB, 0xFC, 0xFC, 0xFD, 0xFD,
        0xFD, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFD, 0xFD,
        0xFD, 0xFC, 0xFC, 0xFB, 0xFA, 0xF9, 0xF9, 0xF8, 0xF7, 0xF5,
        0xF4, 0xF3, 0xF2, 0xF0, 0xEF, 0xEE, 0xEC, 0xEA, 0xE9, 0xE7,
        0xE5, 0xE3, 0xE1, 0xDF, 0xDD, 0xDB, 0xD9, 0xD7, 0xD4, 0xD2,
        0xD0, 0xCD, 0xCB, 0xC8, 0xC6, 0xC3, 0xC0, 0xBE, 0xBB, 0xB8,
        0xB5, 0xB2, 0xB0, 0xAD, 0xAA, 0xA7, 0xA4, 0xA1, 0x9E, 0x9B,
        0x98, 0x95, 0x92, 0x8F, 0x8B, 0x88, 0x85, 0x82, 0x7F, 0x7C,
        0x79, 0x76, 0x73, 0x6F, 0x6C, 0x69, 0x66, 0x63, 0x60, 0x5D,
        0x5A, 0x57, 0x54, 0x51, 0x4E, 0x4C, 0x49, 0x46, 0x43, 0x40,
        0x3E, 0x3B, 0x38, 0x36, 0x33, 0x31, 0x2E, 0x2C, 0x2A, 0x27,
        0x25, 0x23, 0x21, 0x1F, 0x1D, 0x1B, 0x19, 0x17, 0x15, 0x14,
        0x12, 0x10, 0x0F, 0x0E, 0x0C, 0x0B, 0x0A, 0x09, 0x07, 0x06,
        0x05, 0x05, 0x04, 0x03, 0x02, 0x02, 0x01, 0x01, 0x01, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x02,
        0x02, 0x03, 0x04, 0x05, 0x05, 0x06, 0x07, 0x09, 0x0A, 0x0B,
        0x0C, 0x0E, 0x0F, 0x10, 0x12, 0x14, 0x15, 0x17, 0x19, 0x1B,
        0x1D, 0x1F, 0x21, 0x23, 0x25, 0x27, 0x2A, 0x2C, 0x2E, 0x31,
        0x33, 0x36, 0x38, 0x3B, 0x3E, 0x40, 0x43, 0x46, 0x49, 0x4C,
        0x4E, 0x51, 0x54, 0x57, 0x5A, 0x5D, 0x60, 0x63, 0x66, 0x69,
        0x6C, 0x6F, 0x73, 0x76, 0x79, 0x7C};

/*--------------------------------------------------------------------*/
static void clock_setup(void)
{
	rcc_clock_setup_in_hse_12mhz_out_72mhz();
}

/*--------------------------------------------------------------------*/
static void gpio_setup(void)
{
	/* Port A and C are on AHB1 */
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOD);
	/* Set the digital test output on PD2 */
	gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO2);
	/* Set PA4 for DAC channel 1 to analogue, ignoring drive mode. */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO4);
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
	//timer_set_oc_slow_mode(TIM2, TIM_OC1);
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
	dma_set_priority(DMA2, 3, DMA_CCR_PL_LOW);
	dma_set_memory_size(DMA2, 3, DMA_CCR_MSIZE_8BIT );
	dma_set_peripheral_size(DMA2, 3, DMA_CCR_PSIZE_8BIT );
	dma_enable_memory_increment_mode(DMA2, 3);
	dma_enable_circular_mode(DMA2, 3);
	dma_set_read_from_memory(DMA2, 3);
	/* The register to target is the DAC1 8-bit right justified data
	   register */
	dma_set_peripheral_address(DMA2, 3, (uint32_t) &DAC_DHR8R1);
	/* The array v[] is filled with the waveform data to be output */
	dma_set_memory_address(DMA2, 3, (uint32_t) lut);
	dma_set_number_of_data(DMA2, 3, 256);
	dma_enable_transfer_complete_interrupt(DMA2, 3);
	//dma_channel_select(DMA2, 3, DMA_SxCR_CHSEL_7);
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
	if (dma_get_interrupt_flag(DMA2, 3, DMA_TCIF)) {
		dma_clear_interrupt_flags(DMA2, 3, DMA_TCIF);
		/* Toggle PD2 just to keep aware of activity and frequency. */
		gpio_toggle(GPIOD, GPIO2);
	}
}

/*--------------------------------------------------------------------*/
int main(void)
{
	/* Fill the array with funky waveform data */
	/* This is for dual channel 8-bit right aligned */
	// uint16_t i, x;
	// for (i = 0; i < 256; i++) {
	// 	if (i < 10) {
	// 		x = 10;
	// 	} else if (i < 121) {
	// 		x = 10 + ((i*i) >> 7);
	// 	} else if (i < 170) {
	// 		x = i/2;
	// 	} else if (i < 246) {
	// 		x = i + (80 - i/2);
	// 	} else {
	// 		x = 10;
	// 	}
	// 	waveform[i] = x;
        
	// }
	clock_setup();
	gpio_setup();
	timer_setup();
	dma_setup();
	dac_setup();
    //gpio_toggle(GPIOD, GPIO2);

	while (1);

	return 0;
}