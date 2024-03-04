#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/systick.h>

static void gpio_setup(void)
{
    rcc_clock_setup_in_hsi_out_48mhz();
    rcc_periph_clock_enable(RCC_GPIOD);
    gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO2);
}

static volatile uint64_t _millis = 0;

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

// Get the current value of the millis counter
uint64_t millis()
{
    return _millis;
}

// This is our interrupt handler for the systick reload interrupt.
// The full list of interrupt services routines that can be implemented is
// listed in libopencm3/include/libopencm3/stm32/f0/nvic.h
void sys_tick_handler(void)
{
    // Increment our monotonic clock
    _millis++;
}

// Delay a given number of milliseconds in a blocking manner
void delay(uint64_t duration)
{
    const uint64_t until = millis() + duration;
    while (millis() < until)
        ;
}

int main(void)
{
    gpio_setup();
    systick_setup();

    while (1)
    {
        gpio_toggle(GPIOD, GPIO2);
        delay(1000);
    }

    return 0;
}