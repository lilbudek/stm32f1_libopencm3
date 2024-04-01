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
    // rcc_periph_clock_enable(RCC_GPIOA);
    // gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO5);
    rcc_periph_clock_enable(RCC_DAC);
    // dac_enable(CHANNEL_2);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO5);
    dac_disable(CHANNEL_2);
    dac_disable_waveform_generation(CHANNEL_2);
    dac_enable(CHANNEL_2);
    dac_set_trigger_source(DAC_CR_TSEL2_SW);
}

static void nvic_setup(void)
{
    /* Without this the timer interrupt routine will never be called. */
    nvic_enable_irq(NVIC_TIM2_IRQ);
    nvic_set_priority(NVIC_TIM2_IRQ, 255);
}

uint16_t p_acc = 0;     // аккумулятор фазы
uint16_t p_step = 2560; // код частоты 128 - 1khz
// uint8_t addr = 0; // адрес ячейки

const uint16_t sinus[256] = {2048, 2092, 2136, 2180, 2224, 2268, 2312, 2355, 2399, 2442, 
2485, 2527, 2570, 2612, 2654, 2695, 2736, 2777, 2817, 2857, 2896, 2934, 2973, 3010, 3047, 
3084, 3119, 3155, 3189, 3223, 3256, 3288, 3320, 3351, 3381, 3410, 3439, 3466, 3493, 3519, 
3544, 3568, 3591, 3613, 3635, 3655, 3674, 3693, 3710, 3726, 3742, 3756, 3770, 3782, 3793, 
3803, 3812, 3821, 3828, 3833, 3838, 3842, 3845, 3846, 3847, 3846, 3845, 3842, 3838, 3833, 
3828, 3821, 3812, 3803, 3793, 3782, 3770, 3756, 3742, 3726, 3710, 3693, 3674, 3655, 3635, 
3613, 3591, 3568, 3544, 3519, 3493, 3466, 3439, 3410, 3381, 3351, 3320, 3288, 3256, 3223, 
3189, 3155, 3119, 3084, 3047, 3010, 2973, 2934, 2896, 2857, 2817, 2777, 2736, 2695, 2654, 
2612, 2570, 2527, 2485, 2442, 2399, 2355, 2312, 2268, 2224, 2180, 2136, 2092, 2048, 2003, 
1959, 1915, 1871, 1827, 1783, 1740, 1696, 1653, 1610, 1568, 1525, 1483, 1441, 1400, 1359, 
1318, 1278, 1238, 1199, 1161, 1122, 1085, 1048, 1011, 976, 940, 906, 872, 839, 807, 775, 
744, 714, 685, 656, 629, 602, 576, 551, 527, 504, 482, 460, 440, 421, 402, 385, 369, 353, 
339, 325, 313, 302, 292, 283, 274, 267, 262, 257, 253, 250, 249, 248, 249, 250, 253, 257, 
262, 267, 274, 283, 292, 302, 313, 325, 339, 353, 369, 385, 402, 421, 440, 460, 482, 504, 
527, 551, 576, 602, 629, 656, 685, 714, 744, 775, 807, 839, 872, 906, 940, 976, 1011, 1048, 
1085, 1122, 1161, 1199, 1238, 1278, 1318, 1359, 1400, 1441, 1483, 1525, 1568, 1610, 1653, 
1696, 1740, 1783, 1827, 1871, 1915, 1959, 2003};

void tim2_isr(void)
{
    // gpio_toggle(GPIOD, GPIO2);   /* LED on/off. */

    // addr = p_acc >> 8; // выделение старшей части аккумулятора фазы
    TIM_SR(TIM2) &= ~TIM_SR_UIF; /* Clear interrrupt flag. */
    dac_load_data_buffer_single(sinus[p_acc >> 8], RIGHT12, CHANNEL_2);
    dac_software_trigger(CHANNEL_2);
    p_acc += p_step;             // шаг
    
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