/*
 * @file main.c
 * @brief Toggles the LED on PC13 using a SysTick timer and EXTI0 interrupt.
 * Enable GPIO interrupt on PA0 to toggle the LED state on both rising and falling edges.
 * The onboard LED (connected to PC13 on STM32F103C8T6 Blue Pill) will blink using SysTick and toggle using EXTI0.
 * This file is based on examples from the libopencm3 project.
 */

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

/* Constants */
#define TRUE         1
#define FALLING      0
#define RISING       1

/* Pin Definitions */
#define LED_PORT    GPIOC
#define LED_PIN     GPIO13  /* PC13 connected to onboard LED */
#define SWITCH_PORT GPIOA
#define SWITCH_PIN  GPIO0   /* PA0 connected to button (switch) */

/* Define SysTick interval (in milliseconds) */
#define SYSTICK_INTERVAL_MS 100

/* EXTI state direction (falling or rising edge detection) */
static uint16_t exti_direction = FALLING;


/* Function Prototypes */
void system_clock_setup(void);
void gpio_setup(void);
void systick_setup(void);
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

    /* Configure PA0 as input (button) with floating input configuration */
    gpio_set_mode(SWITCH_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, SWITCH_PIN);
}

/**
 * @brief Configures SysTick to generate interrupts every SYSTICK_INTERVAL_MS.
 */
void systick_setup(void)
{
    /* Set the SysTick reload value to trigger interrupts at the specified interval */
    systick_set_reload(rcc_ahb_frequency / 1000 * SYSTICK_INTERVAL_MS - 1);

    /* Set SysTick clock source to AHB (core clock) */
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);

    /* Enable SysTick counter and interrupts */
    systick_counter_enable();
    systick_interrupt_enable();
}

/**
 * @brief Configures EXTI for PA0 to generate interrupts on both falling and rising edges.
 */
void exti_setup(void)
{
    /* Enable AFIO clock (for EXTI configuration) */
    rcc_periph_clock_enable(RCC_AFIO);

    /* Enable EXTI0 interrupt in the NVIC */
    nvic_enable_irq(NVIC_EXTI0_IRQ);

    /* Configure EXTI0 (PA0) for falling edge initially */
    exti_select_source(EXTI0, SWITCH_PORT);          /* Set PA0 as the EXTI0 source */
    exti_set_trigger(EXTI0, EXTI_TRIGGER_FALLING);   /* Trigger interrupt on falling edge */
    exti_enable_request(EXTI0);                      /* Enable EXTI0 interrupt */
}

/**
 * @brief SysTick interrupt handler.
 * Toggles the onboard LED every SYSTICK_INTERVAL_MS.
 */
void sys_tick_handler(void)
{
    toggle_led(); /* Toggle LED periodically */
}

/**
 * @brief EXTI0 interrupt handler for button press on PA0.
 * Toggles the LED state when the button is pressed (detects both falling and rising edges).
 */
void exti0_isr(void)
{
    /* Clear the EXTI0 interrupt flag */
    exti_reset_request(EXTI0);

    /* Toggle the LED and switch EXTI edge detection */
    if (exti_direction == FALLING)
    {
        gpio_set(LED_PORT, LED_PIN);         /* Turn on LED on falling edge */
        exti_direction = RISING;             /* Switch to rising edge detection */
        exti_set_trigger(EXTI0, EXTI_TRIGGER_RISING);
    }
    else
    {
        gpio_clear(LED_PORT, LED_PIN);       /* Turn off LED on rising edge */
        exti_direction = FALLING;            /* Switch to falling edge detection */
        exti_set_trigger(EXTI0, EXTI_TRIGGER_FALLING);
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
 * Initializes the system clock, GPIO, SysTick, and EXTI, then enters an infinite loop.
 */
int main(void)
{
    system_clock_setup();   /* Configure system clock */
    gpio_setup();           /* Configure GPIO pins */
    systick_setup();         /* Configure SysTick timer */
    exti_setup();            /* Configure EXTI for button press detection */

    /* Main loop (the program relies on interrupts for operation) */
    while (TRUE)
    {
        __wfi(); /* Wait for interrupt */
    }

    return 0;
}
