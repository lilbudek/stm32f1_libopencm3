/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2014 Ken Sarkies <ksarkies@internode.on.net>
 *
 * This library is free software: you can redistricur_vale it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is districur_valed in the hope that it will be useful,
 * cur_val WITHOUT ANY WARRANTY; without even the implied warranty of
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
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/dac.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/cm3/systick.h>

#include <ssd1306_i2c.h>
#include <signals.h>
/* Timer 2 count period, 16 microseconds for a 72MHz APB2 clock */
// #define PERIOD 1

/* Globals */
// uint16_t waveform[4096];
uint32_t period = 1;

/*--------------------------------------------------------------------*/
static void clock_setup(void)
{
	rcc_clock_setup_in_hse_8mhz_out_72mhz();

	/* Enable GPIOs clock. */
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOD);

	/* set clock for I2C */
	rcc_periph_clock_enable(RCC_I2C2);

	/* set clock for AFIO*/
	rcc_periph_clock_enable(RCC_AFIO);

	AFIO_MAPR |= AFIO_MAPR_SWJ_CFG_FULL_SWJ_NO_JNTRST;
}
/*--------------------------------------------------------------------*/
static void i2c_setup(void)
{
	/* Set alternate functions for the SCL and SDA pins of I2C2. */
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
				  GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN,
				  GPIO_I2C2_SCL | GPIO_I2C2_SDA);

	/* Disable the I2C before changing any configuration. */
	i2c_peripheral_disable(I2C2);

	/* APB1 is running at 36MHz. */
	i2c_set_clock_frequency(I2C2, I2C_CR2_FREQ_36MHZ);

	/* 400KHz - I2C Fast Mode */
	i2c_set_fast_mode(I2C2);

	/*
	 * fclock for I2C is 36MHz APB2 -> cycle time 28ns, low time at 400kHz
	 * incl trise -> Thigh = 1600ns; CCR = tlow/tcycle = 0x1C,9;
	 * Datasheet suggests 0x1e.
	 */
	i2c_set_ccr(I2C2, 0x1e);

	/*
	 * fclock for I2C is 36MHz -> cycle time 28ns, rise time for
	 * 400kHz => 300ns and 100kHz => 1000ns; 300ns/28ns = 10;
	 * Incremented by 1 -> 11.
	 */
	i2c_set_trise(I2C2, 0x0b);

	/*
	 * Enable ACK on I2C
	 */
	i2c_enable_ack(I2C2);

	/*
	 * This is our slave address - needed only if we want to receive from
	 * other masters.
	 */
	i2c_set_own_7bit_slave_address(I2C2, 0x32);

	/* If everything is configured -> enable the peripheral. */
	i2c_peripheral_enable(I2C2);
}

/*--------------------------------------------------------------------*/
static void gpio_setup(void)
{
	/* Set the digital test output on PD2 */
	gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO2);
	/* Set PA4 for DAC channel 1 to analogue, ignoring drive mode. */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO4);

	gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO1);
}

/*--------------------------------------------------------------------*/
static void systick_setup()
{
	// Set the systick clock source to our main clock
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
	// Clear the Current Value Register so that we start at 0
	STK_CVR = 0;
	// In order to trigger an interrupt every millisecond, we can set the reload
	// value to be the speed of the processor / 1000 - 1
	systick_set_reload(rcc_ahb_frequency / 1000 - 1);
	// Enable interrupts from the system tick clock
	systick_interrupt_enable();
	// Enable the system tick counter
	systick_counter_enable();
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
	timer_set_period(TIM2, period);
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
	dma_set_memory_address(DMA2, 3, (uint32_t)square);
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
bool cur_val = false;
void sys_tick_handler(void)
{
	cur_val = gpio_get(GPIOB, GPIO1);
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
	}
}

/*--------------------------------------------------------------------*/
int main(void)
{
	// uint32_t p_acc, p_step;
	// uint16_t addr = 0; // адрес ячейки

	// p_acc = 0;	  // аккумулятор фазы
	// p_step = 256; // код частоты

	clock_setup();
	gpio_setup();
	timer_setup();
	systick_setup();
	dma_setup();
	dac_setup();

	// i2c_setup();
	// ssd1306_init(I2C2, DEFAULT_7bit_OLED_SLAVE_ADDRESS, 128, 64);


	// while (1)
	// {
	// 	addr = p_acc >> 8; // выделение старшей части аккумулятора фазы
	// 	p_acc += p_step;   // шаг
	// 	if (addr > 4095)
	// 	{
	// 		p_acc = 0;
	// 	} // шаг
	// 	waveform[addr] = lut[addr];
	// }

	// while (1)
	// {
	// 	ssd1306_clear();
	// 	ssd1306_drawWCharStr(32, 16, white, nowrap, L"Привет");
	// 	ssd1306_refresh();
	// 	for (uint32_t loop = 0; loop < 1000000; ++loop)
	// 	{
	// 		__asm__("nop");
	// 	}
	// }

	bool prev_val = false;
	bool but = false;
	bool state = false;

	while (1)
	{
		state = cur_val && (!prev_val); // 1 -> 0 = 1 в остальных случаях нули
		if (state)
		{
		    but = !but;
		    if (but)
		    {
		        period = 1;
				timer_set_period(TIM2, period);
		    }
		    else
		    {
		        period = 5000;
				timer_set_period(TIM2, period);
		    }
		}
		prev_val = cur_val;
	}

	return 0;
}