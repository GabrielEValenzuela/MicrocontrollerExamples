/*
 * @file main.c
 * @brief Toggles the LED on PC13 using Timer2 capture event on PA0.
 * The LED toggles when the button connected to PA0 is pressed or released (capturing both edges).
 */

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

/* Constants */
#define TRUE         1

/* Pin Definitions */
#define LED_PORT    GPIOC
#define LED_PIN     GPIO13  /* PC13 connected to onboard LED */
#define SWITCH_PORT GPIOA
#define SWITCH_PIN  GPIO0   /* PA0 connected to button (switch) */

/* Function Prototypes */
void system_clock_setup(void);
void gpio_setup(void);
void timer2_setup(void);
void exti_setup(void);
void toggle_led(void);

/**
 * @brief Initializes the system clock to 72 MHz using an 8 MHz external crystal.
 */
void system_clock_setup(void)
{
    /* Configure the system clock to 72 MHz using PLL and 8 MHz HSE */
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
}

/**
 * @brief Configures GPIO pin PC13 as an output for the onboard LED and PA0 as input for the button (switch).
 */
void gpio_setup(void)
{
    /* Enable GPIO clocks */
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_GPIOA);

    /* Configure PC13 as output (LED) */
    gpio_set_mode(LED_PORT, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, LED_PIN);

    /* Configure PA0 as input (button) with pull-up */
    gpio_set_mode(SWITCH_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, SWITCH_PIN);
}

/**
 * @brief Configures Timer 2 for input capture on PA0.
 */
void timer2_setup(void)
{
    /* Enable Timer 2 clock */
    rcc_periph_clock_enable(RCC_TIM2);

    /* Reset Timer 2 peripheral */
    timer_reset(TIM2);

    /* Timer configuration */
    timer_set_prescaler(TIM2, 7200 - 1);  // Prescaler for 10 kHz timer clock
    timer_set_period(TIM2, 0xFFFF);       // Max period

    /* Configure Timer 2 capture channel 1 on PA0 */
    timer_ic_set_input(TIM2, TIM_IC1, TIM_IC_IN_TI1);       // Use TI1 as input for capture channel 1
    timer_ic_set_filter(TIM2, TIM_IC1, TIM_IC_OFF);         // No input filter
    timer_ic_set_polarity(TIM2, TIM_IC1, TIM_IC_BOTH);      // Capture on both rising and falling edges
    timer_ic_enable(TIM2, TIM_IC1);                         // Enable input capture for channel 1

    /* Enable the timer interrupt for capture events */
    nvic_enable_irq(NVIC_TIM2_IRQ);
    timer_enable_irq(TIM2, TIM_DIER_CC1IE);  // Enable interrupt on capture event

    /* Start Timer 2 */
    timer_enable_counter(TIM2);
}

/**
 * @brief Timer 2 capture event interrupt handler.
 * Toggles the LED when the capture event is detected on PA0.
 */
void tim2_isr(void)
{
    /* Check if the interrupt was triggered by the capture event */
    if (timer_get_flag(TIM2, TIM_SR_CC1IF))
    {
        timer_clear_flag(TIM2, TIM_SR_CC1IF);  // Clear the capture event flag
        toggle_led();                          // Toggle the LED on capture event
    }
}

/**
 * @brief Toggles the LED state.
 */
void toggle_led(void)
{
    gpio_toggle(LED_PORT, LED_PIN);
}

/**
 * @brief Main function.
 * Initializes the system clock, GPIO, and Timer 2 for input capture, then enters an infinite loop.
 */
int main(void)
{
    system_clock_setup();  /* Set up system clock */
    gpio_setup();          /* Configure GPIO pins */
    timer2_setup();        /* Configure Timer 2 for input capture on PA0 */

    /* Main loop (the program relies on interrupts for operation) */
    while (TRUE)
    {
        __wfi(); /* Wait for interrupt */
    }

    return 0;
}
