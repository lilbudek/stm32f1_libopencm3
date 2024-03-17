#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/systick.h>

static void gpio_setup(void)
{
    rcc_clock_setup_in_hsi_out_48mhz();

    rcc_periph_clock_enable(RCC_GPIOB);

    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO1);

    rcc_periph_clock_enable(RCC_GPIOD);
    gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO2);
}


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


// This is our interrupt handler for the systick reload interrupt.
// The full list of interrupt services routines that can be implemented is
// listed in libopencm3/include/libopencm3/stm32/f0/nvic.h
bool cur_val = false;
void sys_tick_handler(void)
{
    // Increment our monotonic clock
    cur_val = gpio_get(GPIOB, GPIO1);
}


int main(void)
{
    // активный уровень 0
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
    gpio_setup();
    systick_setup();

    bool prev_val = false;
    bool led = false;
    bool state = false;

    while (1)
    {
        state = cur_val && (!prev_val); // 1 -> 0 = 1 в остальных случаях нули
        if (state)
        {
            led = !led;
            if (led)
            {
                gpio_set(GPIOD, GPIO2); // тушим 1
            }
            else
            {
                gpio_clear(GPIOD, GPIO2); // зажигаем 0
            }
        }
        prev_val = cur_val;
    }

    return 0;
}