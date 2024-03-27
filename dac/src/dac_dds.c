/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2013 Karl Palsson <karlp@tweak.net.au>
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

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/dac.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

static void clock_setup(void)
{
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
    // rcc_periph_clock_enable(RCC_GPIOD);
    // gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO2);
}

static void dac_setup(void)
{
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO5);
    rcc_periph_clock_enable(RCC_DAC);
    // dac_disable(CHANNEL_2);
    // dac_disable_waveform_generation(CHANNEL_2);
    dac_enable(CHANNEL_2);
    // dac_set_trigger_source(DAC_CR_TSEL2_SW);
}

int main(void)
{
    clock_setup();
    dac_setup();

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

    const uint16_t square[256] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
        4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
        4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
        4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
        4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
        4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
        4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
        4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
        4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
        4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
        4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
        4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
        4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
        4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
        4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
        4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0};

    const uint16_t triangle[256] = {
        0, 32, 64, 96, 128, 160, 192, 224,
        256, 288, 320, 352, 384, 416, 448, 480,
        512, 544, 576, 608, 640, 672, 704, 736,
        768, 800, 832, 864, 896, 928, 960, 992,
        1024, 1055, 1087, 1119, 1151, 1183, 1215, 1247,
        1279, 1311, 1343, 1375, 1407, 1439, 1471, 1503,
        1535, 1567, 1599, 1631, 1663, 1695, 1727, 1759,
        1791, 1823, 1855, 1887, 1919, 1951, 1983, 2015,
        2047, 2079, 2111, 2143, 2175, 2207, 2239, 2271,
        2303, 2335, 2367, 2399, 2431, 2463, 2495, 2527,
        2559, 2591, 2623, 2655, 2687, 2719, 2751, 2783,
        2815, 2847, 2879, 2911, 2943, 2975, 3007, 3039,
        3070, 3102, 3134, 3166, 3198, 3230, 3262, 3294,
        3326, 3358, 3390, 3422, 3454, 3486, 3518, 3550,
        3582, 3614, 3646, 3678, 3710, 3742, 3774, 3806,
        3838, 3870, 3902, 3934, 3966, 3998, 4030, 4062,
        4094, 4062, 4030, 3998, 3966, 3934, 3902, 3870,
        3838, 3806, 3774, 3742, 3710, 3678, 3646, 3614,
        3582, 3550, 3518, 3486, 3454, 3422, 3390, 3358,
        3326, 3294, 3262, 3230, 3198, 3166, 3134, 3102,
        3070, 3039, 3007, 2975, 2943, 2911, 2879, 2847,
        2815, 2783, 2751, 2719, 2687, 2655, 2623, 2591,
        2559, 2527, 2495, 2463, 2431, 2399, 2367, 2335,
        2303, 2271, 2239, 2207, 2175, 2143, 2111, 2079,
        2047, 2015, 1983, 1951, 1919, 1887, 1855, 1823,
        1791, 1759, 1727, 1695, 1663, 1631, 1599, 1567,
        1535, 1503, 1471, 1439, 1407, 1375, 1343, 1311,
        1279, 1247, 1215, 1183, 1151, 1119, 1087, 1055,
        1024, 992, 960, 928, 896, 864, 832, 800,
        768, 736, 704, 672, 640, 608, 576, 544,
        512, 480, 448, 416, 384, 352, 320, 288,
        256, 224, 192, 160, 128, 96, 64, 32};

    const uint16_t sawtooth[256] = {
        0, 16, 32, 48, 64, 80, 96, 112,
        128, 144, 160, 176, 192, 208, 224, 240,
        256, 272, 288, 304, 320, 336, 352, 368,
        384, 400, 416, 432, 448, 464, 480, 496,
        512, 528, 544, 560, 576, 592, 608, 624,
        640, 656, 672, 688, 704, 720, 736, 752,
        768, 784, 800, 816, 832, 848, 864, 880,
        896, 912, 928, 944, 960, 976, 992, 1008,
        1024, 1039, 1055, 1071, 1087, 1103, 1119, 1135,
        1151, 1167, 1183, 1199, 1215, 1231, 1247, 1263,
        1279, 1295, 1311, 1327, 1343, 1359, 1375, 1391,
        1407, 1423, 1439, 1455, 1471, 1487, 1503, 1519,
        1535, 1551, 1567, 1583, 1599, 1615, 1631, 1647,
        1663, 1679, 1695, 1711, 1727, 1743, 1759, 1775,
        1791, 1807, 1823, 1839, 1855, 1871, 1887, 1903,
        1919, 1935, 1951, 1967, 1983, 1999, 2015, 2031,
        2047, 2063, 2079, 2095, 2111, 2127, 2143, 2159,
        2175, 2191, 2207, 2223, 2239, 2255, 2271, 2287,
        2303, 2319, 2335, 2351, 2367, 2383, 2399, 2415,
        2431, 2447, 2463, 2479, 2495, 2511, 2527, 2543,
        2559, 2575, 2591, 2607, 2623, 2639, 2655, 2671,
        2687, 2703, 2719, 2735, 2751, 2767, 2783, 2799,
        2815, 2831, 2847, 2863, 2879, 2895, 2911, 2927,
        2943, 2959, 2975, 2991, 3007, 3023, 3039, 3055,
        3070, 3086, 3102, 3118, 3134, 3150, 3166, 3182,
        3198, 3214, 3230, 3246, 3262, 3278, 3294, 3310,
        3326, 3342, 3358, 3374, 3390, 3406, 3422, 3438,
        3454, 3470, 3486, 3502, 3518, 3534, 3550, 3566,
        3582, 3598, 3614, 3630, 3646, 3662, 3678, 3694,
        3710, 3726, 3742, 3758, 3774, 3790, 3806, 3822,
        3838, 3854, 3870, 3886, 3902, 3918, 3934, 3950,
        3966, 3982, 3998, 4014, 4030, 4046, 4062, 4078};

    uint16_t p_acc, p_step;
    uint8_t addr = 0; // адрес ячейки

    p_acc = 0;     // аккумулятор фазы
    p_step = 1200; // код частоты 48 - 1кгц

    while (1)
    {
        addr = p_acc >> 8; // выделение старшей части аккумулятора фазы
        p_acc += p_step;   // шаг
        // if (addr > 4095)
        // {
        // 	p_acc = 0;
        // } // шаг
        dac_load_data_buffer_single(square[addr], RIGHT12, CHANNEL_2);
    }

    return 0;
}