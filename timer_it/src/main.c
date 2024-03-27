/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2010 Thomas Otto <tommi@viadmin.org>
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
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/dac.h>

static void gpio_setup(void)
{
    rcc_periph_clock_enable(RCC_GPIOD);
    gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO2);
}

static void dac_setup(void)
{
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO5);
    rcc_periph_clock_enable(RCC_DAC);
    // dac_disable(CHANNEL_2);
    // dac_disable_waveform_generation(CHANNEL_2);
    dac_enable(CHANNEL_2);
    // dac_set_trigger_source(DAC_CR_TSEL2_SW);
}

static void nvic_setup(void)
{
    /* Without this the timer interrupt routine will never be called. */
    nvic_enable_irq(NVIC_TIM2_IRQ);
    nvic_set_priority(NVIC_TIM2_IRQ, 255);
}

uint16_t p_acc = 0; // аккумулятор фазы
uint16_t p_step = 1024; // код частоты 256 - 2.5khz
//uint8_t addr = 0; // адрес ячейки


const uint16_t sinus[256] = {
    2047, 2097, 2147, 2198, 2248, 2298, 2347, 2397,
    2446, 2496, 2544, 2593, 2641, 2689, 2737, 2784,
    2830, 2877, 2922, 2967, 3012, 3056, 3099, 3142,
    3184, 3226, 3266, 3306, 3346, 3384, 3422, 3458,
    3494, 3530, 3564, 3597, 3629, 3661, 3691, 3721,
    3749, 3776, 3803, 3828, 3852, 3875, 3897, 3918,
    3938, 3957, 3974, 3991, 4006, 4020, 4033, 4044,
    4055, 4064, 4072, 4079, 4084, 4088, 4092, 4093,
    4094, 4093, 4092, 4088, 4084, 4079, 4072, 4064,
    4055, 4044, 4033, 4020, 4006, 3991, 3974, 3957,
    3938, 3918, 3897, 3875, 3852, 3828, 3803, 3776,
    3749, 3721, 3691, 3661, 3629, 3597, 3564, 3530,
    3494, 3458, 3422, 3384, 3346, 3306, 3266, 3226,
    3184, 3142, 3099, 3056, 3012, 2967, 2922, 2877,
    2830, 2784, 2737, 2689, 2641, 2593, 2544, 2496,
    2446, 2397, 2347, 2298, 2248, 2198, 2147, 2097,
    2047, 1997, 1947, 1896, 1846, 1796, 1747, 1697,
    1648, 1598, 1550, 1501, 1453, 1405, 1357, 1310,
    1264, 1217, 1172, 1127, 1082, 1038, 995, 952,
    910, 868, 828, 788, 748, 710, 672, 636,
    600, 564, 530, 497, 465, 433, 403, 373,
    345, 318, 291, 266, 242, 219, 197, 176,
    156, 137, 120, 103, 88, 74, 61, 50,
    39, 30, 22, 15, 10, 6, 2, 1,
    0, 1, 2, 6, 10, 15, 22, 30,
    39, 50, 61, 74, 88, 103, 120, 137,
    156, 176, 197, 219, 242, 266, 291, 318,
    345, 373, 403, 433, 465, 497, 530, 564,
    600, 636, 672, 710, 748, 788, 828, 868,
    910, 952, 995, 1038, 1082, 1127, 1172, 1217,
    1264, 1310, 1357, 1405, 1453, 1501, 1550, 1598,
    1648, 1697, 1747, 1796, 1846, 1896, 1947, 1997};

void tim2_isr(void)
{
    // gpio_toggle(GPIOD, GPIO2);   /* LED on/off. */

    //addr = p_acc >> 8; // выделение старшей части аккумулятора фазы
    
    dac_load_data_buffer_single(sinus[p_acc >> 8], RIGHT12, CHANNEL_2);
    p_acc += p_step;   // шаг
    TIM_SR(TIM2) &= ~TIM_SR_UIF; /* Clear interrrupt flag. */
}





int main(void)
{
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
    gpio_setup();
    nvic_setup();
    dac_setup();

    // gpio_clear(GPIOB, GPIO7);	/* LED1 on */
    // gpio_set(GPIOB, GPIO6);		/* LED2 off */

    rcc_periph_clock_enable(RCC_TIM2);

    /*
     * The goal is to let the LED2 glow for a second and then be
     * off for a second.
     */

    /* Set timer start value. */
    TIM_CNT(TIM2) = 1;

    /* Set timer prescaler. 36MHz/36000 => 1000 counts per second. */
    TIM_PSC(TIM2) = 0;

    /* End timer value. If this is reached an interrupt is generated. */
    TIM_ARR(TIM2) = 4;

    /* Update interrupt enable. */
    TIM_DIER(TIM2) |= TIM_DIER_UIE;

    /* Start timer. */
    TIM_CR1(TIM2) |= TIM_CR1_CEN;

    while (1)
        ; /* Halt. */

    return 0;
}