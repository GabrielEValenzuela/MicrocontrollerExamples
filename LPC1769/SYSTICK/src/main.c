/*
 * @file main.c
 * @brief
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#ifdef __USE_MCUEXPRESSO
#include <cr_section_macros.h> /* MCUXpresso-specific macros */
#endif

#include "lpc17xx_gpio.h"    /* GPIO handling */
#include "lpc17xx_pinsel.h"  /* Pin function selection */
#include "lpc17xx_systick.h" /* SysTick handling */

/* Pin Definitions */
#define LED_PIN ((uint32_t)(1 << 22)) /* P0.22 connected to LED */

/* GPIO Direction Definitions */
#define INPUT  0
#define OUTPUT 1

/* Define time variables */
#define SYSTICK_TIME 10 /* Expressed in milliseconds */

/* Boolean Values */
#define TRUE  1
#define FALSE 0

/**
 * @brief Initialize the GPIO peripheral
 *
 */
void configure_port(void)
{
    PINSEL_CFG_Type led_pin_cfg; /* Create a variable to store the configuration of the pin */

    /* We need to configure the struct with the desired configuration */
    led_pin_cfg.Portnum = PINSEL_PORT_0;           /* The port number is 0 */
    led_pin_cfg.Pinnum = PINSEL_PIN_22;            /* The pin number is 22 */
    led_pin_cfg.Funcnum = PINSEL_FUNC_0;           /* The function number is 0 */
    led_pin_cfg.Pinmode = PINSEL_PINMODE_PULLUP;   /* The pin mode is pull-up */
    led_pin_cfg.OpenDrain = PINSEL_PINMODE_NORMAL; /* The pin is in the normal mode */

    /* Configure the pin */
    PINSEL_ConfigPin(&led_pin_cfg);

    /* Set the pins as input or output */
    GPIO_SetDir(PINSEL_PORT_0, LED_PIN, OUTPUT); /* Set the P0.22 pin as output */
}

void configure_systick(void)
{
    SYSTICK_InternalInit(SYSTICK_TIME); /* Initialize the SysTick timer with a time interval of 10 ms */
}

// Overwrite the interrupt handler routine for SysTick
void SysTick_Handler(void)
{
    SYSTICK_ClearCounterFlag(); /* Clear interrupt flag */

    if (GPIO_ReadValue(PINSEL_PORT_0) & LED_PIN)
    {
        GPIO_ClearValue(PINSEL_PORT_0, LED_PIN); /* Turn off LED */
    }
    else
    {
        GPIO_SetValue(PINSEL_PORT_0, LED_PIN); /* Turn on LED */
    }
}

/**
 * @brief Main function.
 * TODO: Add description
 */
int main(void)
{
    SystemInit(); /* Initialize the system clock (default: 100 MHz) */

    configure_port(); /* Configure GPIO pins */

    configure_systick(); /* Configure SysTick timer */

    SYSTICK_IntCmd(ENABLE); /* Enable SysTick interrupt */

    SYSTICK_Cmd(ENABLE); /* Enable SysTick counter */

    while (TRUE)
    {
        /* Wait for SysTick interrupt */
    }

    return 0; /* Program should never reach this point */
}
