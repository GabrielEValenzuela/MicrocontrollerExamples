/**
 * @file main.c
 * @brief Toggles the LED on PC13 using a systick.
 */

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

/* Constants */
#define TRUE         1
#define RELOAD_COUNT 8999 /* SysTick interrupt every N clock pulses: set reload to N-1 */

/* Function Prototypes */
void systemInit(void);
void configure_gpio(void);
void configure_systick(void);

/**
 * @brief Initializes the system clock and peripherals.
 */
void systemInit(void)
{
    /* Configure the system clock to run at 72 MHz using an 8 MHz external crystal */
    // Pro tip! To avoid warning messages, use the following syntax:
    // rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);

    rcc_clock_setup_in_hse_8mhz_out_72mhz(); /* Use the default configuration */

    /* Enable the clock for GPIOC */
    rcc_periph_clock_enable(RCC_GPIOC);
}

/**
 * @brief Configures GPIO pin PC13 as an output.
 */
void configure_gpio(void)
{
    /* Set PC13 as a push-pull output at 2 MHz */
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
}

/**
 * @brief Configures the systick timer.
 */
void configure_systick(void)
{
    /* 72MHz / 8 => 9000000 counts per second */
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);

    /* 9000000/9000 = 1000 overflows per second - every 1ms one interrupt */
    systick_set_reload(RELOAD_COUNT);

    systick_interrupt_enable();

    /* Start counting. */
    systick_counter_enable();
}

// Override the weak definition of the systick handler
void sys_tick_handler(void)
{
    /* Toggle the LED on PC13 */
    gpio_toggle(GPIOC, GPIO13);
}

/**
 * @brief Main function.
 * Initializes the system and toggles the LED on PC13.
 */
int main(void)
{
    systemInit();        /* Initialize the system clock and GPIO peripherals */
    configure_gpio();    /* Configure the GPIO pin for the LED */
    configure_systick(); /* Configure the SysTick timer */

    while (TRUE)
    {
    }

    return 0; /* Program should never reach this point */
}
