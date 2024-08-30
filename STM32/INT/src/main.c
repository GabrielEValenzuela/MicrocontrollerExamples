/**
 * @file main.c
 * @brief Toggles the LED on PC13 using a systick. Enable GPIO interrupt on PA0 to toggle the LED state.
 */

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

/* Constants */
#define TRUE         1
#define RELOAD_COUNT 8999 /* SysTick interrupt every N clock pulses: set reload to N-1 */

/* Pin Definitions */
#define LED_PORT    GPIOC
#define LED_PIN     GPIO13 /* PC13 connected to onboard LED (on STM32F103C8T6 Blue Pill) */
#define SWITCH_PORT GPIOA
#define SWITCH_PIN  GPIO0 /* PA0 connected to switch */

/* Define SysTick interval */
#define SYSTICK_INTERVAL_MS 100

/* Function Prototypes */
void systemInit(void);
void configure_gpio(void);
void configure_systick(void);
void toggle_led(void);

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
    /* Enable GPIO clocks */
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_GPIOA);

    /* Configure LED pin as output */
    gpio_set_mode(LED_PORT, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, LED_PIN);

    /* Configure switch pin as input with pull-up */
    gpio_set_mode(SWITCH_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, SWITCH_PIN);
    gpio_set(SWITCH_PORT, SWITCH_PIN); /* Enable pull-up resistor */

    /* Enable EXTI interrupt for the switch pin */
    nvic_enable_irq(NVIC_EXTI0_IRQ); /* Enable EXTI0 interrupt (for PA0) */

    exti_select_source(EXTI0, SWITCH_PORT);        /* Select PA0 as the source for EXTI0 */
    exti_set_trigger(EXTI0, EXTI_TRIGGER_FALLING); /* Trigger on falling edge */
    exti_enable_request(EXTI0);                    /* Enable EXTI0 interrupt request */
}

/**
 * @brief Configures the systick timer.
 */
void configure_systick(void)
{
    systick_set_reload(rcc_ahb_frequency / 1000 * SYSTICK_INTERVAL_MS - 1); /* Set reload for SYSTICK_INTERVAL_MS */
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
    systick_counter_enable();
    systick_interrupt_enable();
}

/**
 * @brief SysTick interrupt handler.
 */
void sys_tick_handler(void)
{
    toggle_led();
}

/**
 * @brief EXTI0 interrupt handler for switch press on PA0.
 */
void exti0_isr(void)
{
    if (exti_get_flag_status(EXTI0))
    {
        exti_reset_request(EXTI0);
        toggle_led();
    }
}

/**
 * @brief Toggle the LED state.
 */
void toggle_led(void)
{
    gpio_toggle(LED_PORT, LED_PIN);
}

/**
 * @brief Main function.
 */
int main(void)
{
    rcc_clock_setup_in_hsi_out_48mhz(); /* Set up clock, running at 48 MHz */

    configure_gpio();    /* Configure GPIO pins */
    configure_systick(); /* Configure SysTick timer */

    nvic_set_priority(EXTI0, 0); /* Set priority for EXTI0 interrupt */

    while (TRUE)
    {
    }

    return 0;
}
