/**
 * @file main.c
 * @brief Toggles the LED on PC13 using a simple delay loop.
 */

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

/* Constants */
#define TRUE 1

/* Function Prototypes */
void systemInit(void);
void configure_gpio(void);

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
 * @brief Main function.
 * Initializes the system and toggles the LED on PC13.
 */
int main(void)
{
    systemInit();     /* Initialize the system clock and GPIO peripherals */
    configure_gpio(); /* Configure the GPIO pin for the LED */

    while (TRUE)
    {
        gpio_toggle(GPIOC, GPIO13); /* Toggle the LED on PC13 */

        /* Simple delay loop */
        for (volatile int i = 0; i < 1000000; i++)
        {
            __asm__("nop"); /* No operation */
        }
    }

    return 0; /* Program should never reach this point */
}
