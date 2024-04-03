#include <stdio.h>
#include <wchar.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/dac.h>
#include <libopencm3/stm32/i2c.h>
#include <ssd1306_i2c.h>

static void gpio_setup(void)
{
    rcc_periph_clock_enable(RCC_GPIOD);
    rcc_periph_clock_enable(RCC_GPIOB);
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO5 | GPIO6 | GPIO7 | GPIO8);
    gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO2);
}

static void dac_setup(void)
{
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO5);
    rcc_periph_clock_enable(RCC_DAC);
    dac_enable(CHANNEL_2);
}

static void i2c_setup(void)
{
    rcc_periph_clock_enable(RCC_I2C2);
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
static void timers_setup(void)
{
    rcc_periph_clock_enable(RCC_TIM2);
    rcc_periph_clock_enable(RCC_TIM3);

    /* Set timer start value. */
    TIM_CNT(TIM2) = 1;
    TIM_CNT(TIM3) = 1;

    /* Set timer prescaler. 36MHz/36000 => 1000 counts per second. */
    TIM_PSC(TIM2) = 18;
    TIM_PSC(TIM3) = 36000;

    /* End timer value. If this is reached an interrupt is generated. */
    TIM_ARR(TIM2) = 10;
    TIM_ARR(TIM3) = 250;

    /* Update interrupt enable. */
    TIM_DIER(TIM2) |= TIM_DIER_UIE;
    TIM_DIER(TIM3) |= TIM_DIER_UIE;

    /* Start timer. */
    TIM_CR1(TIM2) |= TIM_CR1_CEN;
    TIM_CR1(TIM3) |= TIM_CR1_CEN;
    
}
static void nvic_setup(void)
{
    /* Without this the timer interrupt routine will never be called. */
    nvic_enable_irq(NVIC_TIM2_IRQ);
    nvic_set_priority(NVIC_TIM2_IRQ, 2);

    nvic_enable_irq(NVIC_TIM3_IRQ);
    nvic_set_priority(NVIC_TIM3_IRQ, 1);

}

uint16_t p_acc = 0;   // аккумулятор фазы
uint16_t p_step = 96; // код частоты 96 - 0.5khz
uint16_t signal[256] = {0};

uint16_t sinus[256] = {2048, 2092, 2136, 2180, 2224, 2268, 2312, 2355, 2399, 2442,
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

uint16_t square[256] = {3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847,
                        3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847,
                        3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847,
                        3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847,
                        3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847,
                        3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847,
                        3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847,
                        3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847,
                        3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847, 3847,
                        3847, 3847, 3847, 3847, 3847, 3847, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248,
                        248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248,
                        248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248,
                        248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248,
                        248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248,
                        248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248,
                        248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248,
                        248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248};

uint16_t triangle[256] = {248, 276, 304, 332, 360, 389, 417, 445, 473, 501, 529, 557, 585, 614, 642, 670, 698,
                          726, 754, 782, 810, 838, 867, 895, 923, 951, 979, 1007, 1035, 1063, 1092, 1120, 1148, 1176, 1204, 1232, 1260,
                          1288, 1316, 1345, 1373, 1401, 1429, 1457, 1485, 1513, 1541, 1570, 1598, 1626, 1654, 1682, 1710, 1738, 1766, 1794,
                          1823, 1851, 1879, 1907, 1935, 1963, 1991, 2019, 2048, 2076, 2104, 2132, 2160, 2188, 2216, 2244, 2272, 2301, 2329,
                          2357, 2385, 2413, 2441, 2469, 2497, 2525, 2554, 2582, 2610, 2638, 2666, 2694, 2722, 2750, 2779, 2807, 2835, 2863,
                          2891, 2919, 2947, 2975, 3003, 3032, 3060, 3088, 3116, 3144, 3172, 3200, 3228, 3257, 3285, 3313, 3341, 3369, 3397,
                          3425, 3453, 3481, 3510, 3538, 3566, 3594, 3622, 3650, 3678, 3706, 3735, 3763, 3791, 3819, 3847, 3819, 3791, 3763,
                          3735, 3706, 3678, 3650, 3622, 3594, 3566, 3538, 3510, 3481, 3453, 3425, 3397, 3369, 3341, 3313, 3285, 3257, 3228,
                          3200, 3172, 3144, 3116, 3088, 3060, 3032, 3003, 2975, 2947, 2919, 2891, 2863, 2835, 2807, 2779, 2750, 2722, 2694,
                          2666, 2638, 2610, 2582, 2554, 2525, 2497, 2469, 2441, 2413, 2385, 2357, 2329, 2301, 2272, 2244, 2216, 2188, 2160,
                          2132, 2104, 2076, 2048, 2019, 1991, 1963, 1935, 1907, 1879, 1851, 1823, 1794, 1766, 1738, 1710, 1682, 1654, 1626,
                          1598, 1570, 1541, 1513, 1485, 1457, 1429, 1401, 1373, 1345, 1316, 1288, 1260, 1232, 1204, 1176, 1148, 1120, 1092,
                          1063, 1035, 1007, 979, 951, 923, 895, 867, 838, 810, 782, 754, 726, 698, 670, 642, 614, 585, 557, 529, 501, 473,
                          445, 417, 389, 360, 332, 304, 276};

void tim2_isr(void)
{
    dac_load_data_buffer_single(signal[p_acc >> 8], RIGHT12, CHANNEL_2);
    p_acc += p_step;             // шаг
    TIM_SR(TIM2) &= ~TIM_SR_UIF; /* Clear interrrupt flag. */
}

void minus_freq(void)
{
    bool cur_val = 0;
    bool prev_val = 0;
    cur_val = gpio_get(GPIOB, GPIO5);
    if (cur_val == 1 && prev_val == 0)
    {
        p_step -= 96;
    }
    if (p_step == 0)
    {
        p_step = 96;
    }
    prev_val = cur_val;
}

void plus_freq(void)
{
    bool cur_val = 0;
    bool prev_val = 0;
    cur_val = gpio_get(GPIOB, GPIO6);
    if (cur_val == 1 && prev_val == 0)
    {
        p_step += 96;
    }
    prev_val = cur_val;
}

int num_sig = 0; // number signal
void minus_signal(void)
{
    bool cur_val = 0;
    bool prev_val = 0;
    cur_val = gpio_get(GPIOB, GPIO7);
    if (cur_val == 1 && prev_val == 0)
    {
        dac_disable(CHANNEL_2);
        num_sig -= 1;
        if (num_sig < 1)
        {
            num_sig = 1;
        }
        if (num_sig == 1)
        {
            for (int i = 0; i < 256; i++)
            {
                signal[i] = sinus[i];
            }
        }
        if (num_sig == 2)
        {
            for (int i = 0; i < 256; i++)
            {
                signal[i] = square[i];
            }
        }
        if (num_sig == 3)
        {
            for (int i = 0; i < 256; i++)
            {
                signal[i] = triangle[i];
            }
        }
        if (num_sig > 3)
        {
            num_sig = 3;
        }
        dac_enable(CHANNEL_2);
    }
    prev_val = cur_val;
}

void plus_signal(void)
{
    bool cur_val = 0;
    bool prev_val = 0;
    cur_val = gpio_get(GPIOB, GPIO8);
    if (cur_val == 1 && prev_val == 0)
    {
        dac_disable(CHANNEL_2);
        num_sig += 1;
        if (num_sig < 1)
        {
            num_sig = 1;
        }
        if (num_sig == 1)
        {
            for (int i = 0; i < 256; i++)
            {
                signal[i] = sinus[i];
            }
        }
        if (num_sig == 2)
        {
            for (int i = 0; i < 256; i++)
            {
                signal[i] = square[i];
            }
        }
        if (num_sig == 3)
        {
            for (int i = 0; i < 256; i++)
            {
                signal[i] = triangle[i];
            }
        }
        if (num_sig > 3)
        {
            num_sig = 3;
        }
        dac_enable(CHANNEL_2);
    }
    prev_val = cur_val;
}

void tim3_isr(void) // обработка кнопок
{
    minus_freq();
    plus_freq();
    minus_signal();
    plus_signal();
    TIM_SR(TIM3) &= ~TIM_SR_UIF; /* Clear interrrupt flag. */
}


int main(void)
{
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
    gpio_setup();
    nvic_setup();
    dac_setup();
    i2c_setup();
    ssd1306_init(I2C2, DEFAULT_7bit_OLED_SLAVE_ADDRESS, 128, 64);
    timers_setup();

    while (1)
    {
    int number = 0;
    wchar_t wstr[8]; // Буфер для wchar_t строки
    //Использование swprintf для преобразования int в wchar_t*
    swprintf(wstr, sizeof(wstr) / sizeof(wchar_t), L"%d", number);

    ssd1306_clear();
    ssd1306_drawWCharStr(0, 8, white, nowrap, L"Форма сигнала: Синус");
    ssd1306_drawWCharStr(0, 16, white, nowrap, L"Частота:");
    ssd1306_drawWCharStr(48, 16, white, nowrap, wstr);
    ssd1306_drawWCharStr(0, 24, white, nowrap, L"Шаг:");
    ssd1306_drawWCharStr(48, 24, white, nowrap, wstr);
    ssd1306_refresh();
    }


    return 0;
}