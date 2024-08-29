/*
 * @file main.c
 * @brief This project demonstrates how to configure GPIO pins in the LPC1769 using CMSIS.
 * The GPIO pins are configured as output and input, and the LED connected to P0.22 is toggled
 * based on the state of the input pin P0.0.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#ifdef __USE_MCUEXPRESSO
#include <cr_section_macros.h> /* MCUXpresso-specific macros */
#endif

#include "lpc17xx_gpio.h"   /* GPIO handling */
#include "lpc17xx_pinsel.h" /* Pin function selection */

/* Pin Definitions */
#define LED_PIN   ((uint32_t)(1 << 22)) /* P0.22 connected to LED */
#define INPUT_PIN ((uint32_t)(1 << 0))  /* P0.0 connected to input */

/* GPIO Direction Definitions */
#define INPUT  0
#define OUTPUT 1

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

    PINSEL_CFG_Type input_pin_cfg; /* Create a variable to store the configuration of the pin */

    /* We need to configure the struct with the desired configuration */
    input_pin_cfg.Portnum = PINSEL_PORT_0;           /* The port number is 0 */
    input_pin_cfg.Pinnum = PINSEL_PIN_0;             /* The pin number is 0 */
    input_pin_cfg.Funcnum = PINSEL_FUNC_0;           /* The function number is 0 */
    input_pin_cfg.Pinmode = PINSEL_PINMODE_PULLUP;   /* The pin mode is pull-up */
    input_pin_cfg.OpenDrain = PINSEL_PINMODE_NORMAL; /* The pin is in the normal mode */

    /**
     * Pro tip!
     * We can re-use the same variable to configure the pins, just change the values of the struct.
     * PINSEL_CFG_Type pin_cfg = {
     * .Portnum = PINSEL_PORT_0,
     * .Pinnum = PINSEL_PIN_22,
     * .Funcnum = PINSEL_FUNC_0,
     * .Pinmode = PINSEL_PINMODE_PULLUP,
     * .OpenDrain = PINSEL_PINMODE_NORMAL
     * };
     * PINSEL_ConfigPin(&pin_cfg);
     * pin_cfg.Pinnum = PINSEL_PIN_0;
     * pin_cfg.Funcnum = PINSEL_FUNC_1;
     * PINSEL_ConfigPin(&pin_cfg);
     */

    /* Configure the pins */
    PINSEL_ConfigPin(&led_pin_cfg);
    PINSEL_ConfigPin(&input_pin_cfg);

    /* Set the pins as input or output */
    GPIO_SetDir(PINSEL_PORT_0, LED_PIN, OUTPUT);  /* Set the P0.22 pin as output */
    GPIO_SetDir(PINSEL_PORT_0, INPUT_PIN, INPUT); /* Set the P0.0 pin as input */
}

/**
 * @brief Main function.
 * Initializes the system and toggles the LED based on the input pin state.
 */
int main(void)
{
    SystemInit(); /* Initialize the system clock (default: 100 MHz) */

    configure_port(); /* Configure GPIO pins */

    while (TRUE)
    {
        /* Simple delay loop */
        for (volatile int i = 0; i < 1000000; i++) /* Use volatile to prevent optimization */
        {
            __asm("nop"); /* No operation */
        }

        /* Toggle LED based on the input pin state */
        if (GPIO_ReadValue(PINSEL_PORT_0) & INPUT_PIN)
        {
            GPIO_SetValue(PINSEL_PORT_0, LED_PIN); /* Turn on LED */
        }
        else
        {
            GPIO_ClearValue(PINSEL_PORT_0, LED_PIN); /* Turn off LED */
        }
    }

    return 0; /* Program should never reach this point */
}
