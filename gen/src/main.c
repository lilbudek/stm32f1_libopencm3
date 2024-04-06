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

static void gpio_setup(void);
static void dac_setup(void);
static void i2c_setup(void);
static void timers_setup(void);
static void nvic_setup(void);
void minus_freq(void);
void plus_freq(void);
void minus_signal(void);
void plus_signal(void);

#define STEP 192

uint16_t p_acc = 0;     // аккумулятор фазы
uint16_t p_step = STEP; // код частоты 192 - 1khz
uint16_t signal[256] = {0};
int num_sig = 0; // номер сигнала

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

uint16_t l_saw[256] = {248, 262, 276, 290, 304, 319, 333, 347, 361, 375, 389, 403, 417, 431, 446, 460, 474,
                       488, 502, 516, 530, 544, 559, 573, 587, 601, 615, 629, 643, 657, 671, 686, 700, 714, 728, 742, 756, 770, 784,
                       798, 813, 827, 841, 855, 869, 883, 897, 911, 925, 940, 954, 968, 982, 996, 1010, 1024, 1038, 1052, 1067, 1081,
                       1095, 1109, 1123, 1137, 1151, 1165, 1180, 1194, 1208, 1222, 1236, 1250, 1264, 1278, 1292, 1307, 1321, 1335, 1349,
                       1363, 1377, 1391, 1405, 1419, 1434, 1448, 1462, 1476, 1490, 1504, 1518, 1532, 1546, 1561, 1575, 1589, 1603, 1617,
                       1631, 1645, 1659, 1673, 1688, 1702, 1716, 1730, 1744, 1758, 1772, 1786, 1801, 1815, 1829, 1843, 1857, 1871, 1885,
                       1899, 1913, 1928, 1942, 1956, 1970, 1984, 1998, 2012, 2026, 2040, 2055, 2069, 2083, 2097, 2111, 2125, 2139, 2153,
                       2167, 2182, 2196, 2210, 2224, 2238, 2252, 2266, 2280, 2294, 2309, 2323, 2337, 2351, 2365, 2379, 2393, 2407, 2422,
                       2436, 2450, 2464, 2478, 2492, 2506, 2520, 2534, 2549, 2563, 2577, 2591, 2605, 2619, 2633, 2647, 2661, 2676, 2690,
                       2704, 2718, 2732, 2746, 2760, 2774, 2788, 2803, 2817, 2831, 2845, 2859, 2873, 2887, 2901, 2915, 2930, 2944, 2958,
                       2972, 2986, 3000, 3014, 3028, 3043, 3057, 3071, 3085, 3099, 3113, 3127, 3141, 3155, 3170, 3184, 3198, 3212, 3226,
                       3240, 3254, 3268, 3282, 3297, 3311, 3325, 3339, 3353, 3367, 3381, 3395, 3409, 3424, 3438, 3452, 3466, 3480, 3494,
                       3508, 3522, 3536, 3551, 3565, 3579, 3593, 3607, 3621, 3635, 3649, 3664, 3678, 3692, 3706, 3720, 3734, 3748, 3762,
                       3776, 3791, 3805, 3819, 3833, 3847};

uint16_t r_saw[256] = {3847, 3833, 3819, 3805, 3791, 3776, 3762, 3748, 3734, 3720, 3706, 3692, 3678, 3664,
                       3649, 3635, 3621, 3607, 3593, 3579, 3565, 3551, 3536, 3522, 3508, 3494, 3480, 3466, 3452, 3438, 3424, 3409,
                       3395, 3381, 3367, 3353, 3339, 3325, 3311, 3297, 3282, 3268, 3254, 3240, 3226, 3212, 3198, 3184, 3170, 3155,
                       3141, 3127, 3113, 3099, 3085, 3071, 3057, 3043, 3028, 3014, 3000, 2986, 2972, 2958, 2944, 2930, 2915, 2901,
                       2887, 2873, 2859, 2845, 2831, 2817, 2803, 2788, 2774, 2760, 2746, 2732, 2718, 2704, 2690, 2676, 2661, 2647,
                       2633, 2619, 2605, 2591, 2577, 2563, 2549, 2534, 2520, 2506, 2492, 2478, 2464, 2450, 2436, 2422, 2407, 2393,
                       2379, 2365, 2351, 2337, 2323, 2309, 2294, 2280, 2266, 2252, 2238, 2224, 2210, 2196, 2182, 2167, 2153, 2139,
                       2125, 2111, 2097, 2083, 2069, 2055, 2040, 2026, 2012, 1998, 1984, 1970, 1956, 1942, 1928, 1913, 1899, 1885,
                       1871, 1857, 1843, 1829, 1815, 1801, 1786, 1772, 1758, 1744, 1730, 1716, 1702, 1688, 1673, 1659, 1645, 1631,
                       1617, 1603, 1589, 1575, 1561, 1546, 1532, 1518, 1504, 1490, 1476, 1462, 1448, 1434, 1419, 1405, 1391, 1377,
                       1363, 1349, 1335, 1321, 1307, 1292, 1278, 1264, 1250, 1236, 1222, 1208, 1194, 1180, 1165, 1151, 1137, 1123,
                       1109, 1095, 1081, 1067, 1052, 1038, 1024, 1010, 996, 982, 968, 954, 940, 925, 911, 897, 883, 869, 855, 841,
                       827, 813, 798, 784, 770, 756, 742, 728, 714, 700, 686, 671, 657, 643, 629, 615, 601, 587, 573, 559, 544, 530,
                       516, 502, 488, 474, 460, 446, 431, 417, 403, 389, 375, 361, 347, 333, 319, 304, 290, 276, 262, 248};

void tim2_isr(void)
{
    dac_load_data_buffer_single(signal[p_acc >> 8], RIGHT12, CHANNEL_2);
    p_acc += p_step;             // шаг
    TIM_SR(TIM2) &= ~TIM_SR_UIF; /* Clear interrrupt flag. */
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
    timers_setup();
    i2c_setup();
    ssd1306_init(I2C2, DEFAULT_7bit_OLED_SLAVE_ADDRESS, 128, 64);

    int f = 0;
    wchar_t freq[8]; // Буфер для wchar_t строки
    while (1)
    {
        f = p_step / STEP;
        // Использование swprintf для преобразования int в wchar_t*
        swprintf(freq, sizeof(freq) / sizeof(wchar_t), L"%d", f);

        ssd1306_clear();
        ssd1306_drawWCharStr(0, 0, white, nowrap, L"Форма сигнала:");
        if (num_sig == 1)
        {
            ssd1306_drawWCharStr(0, 8, white, nowrap, L"Синус");
        }
        if (num_sig == 2)
        {
            ssd1306_drawWCharStr(0, 8, white, nowrap, L"Меандр");
        }
        if (num_sig == 3)
        {
            ssd1306_drawWCharStr(0, 8, white, nowrap, L"Треугольник");
        }
        if (num_sig == 4)
        {
            ssd1306_drawWCharStr(0, 8, white, nowrap, L"Пила Левая");
        }
        if (num_sig == 5)
        {
            ssd1306_drawWCharStr(0, 8, white, nowrap, L"Пила Правая");
        }
        ssd1306_drawWCharStr(0, 16, white, nowrap, L"Частота");
        ssd1306_drawWCharStr(48, 16, white, nowrap, freq);
        ssd1306_drawWCharStr(64, 16, white, nowrap, L"кГц");
        ssd1306_drawWCharStr(0, 32, white, nowrap, L"Шаг");
        ssd1306_drawWCharStr(24, 32, white, nowrap, L"1 кГц (фикс)");
        ssd1306_refresh();
    }

    return 0;
}

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
    // TIM_CNT(TIM2) = 1;
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

void minus_freq(void)
{
    bool cur_val = 0;
    bool prev_val = 0;
    cur_val = gpio_get(GPIOB, GPIO5);
    if (cur_val == 1 && prev_val == 0)
    {
        p_step -= STEP;
    }
    if (p_step == 0)
    {
        p_step = STEP;
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
        p_step += STEP;
    }
    prev_val = cur_val;
}

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
        if (num_sig == 4)
        {
            for (int i = 0; i < 256; i++)
            {
                signal[i] = l_saw[i];
            }
        }
        if (num_sig == 5)
        {
            for (int i = 0; i < 256; i++)
            {
                signal[i] = r_saw[i];
            }
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
        if (num_sig == 4)
        {
            for (int i = 0; i < 256; i++)
            {
                signal[i] = l_saw[i];
            }
        }
        if (num_sig == 5)
        {
            for (int i = 0; i < 256; i++)
            {
                signal[i] = r_saw[i];
            }
        }
        if (num_sig > 5)
        {
            num_sig = 5;
        }
        dac_enable(CHANNEL_2);
    }
    prev_val = cur_val;
}
