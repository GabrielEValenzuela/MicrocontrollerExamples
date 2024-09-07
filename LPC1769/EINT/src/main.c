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
#include "lpc17xx_exti.h"    /* External interrupt handling */

/* Pin Definitions */
#define SWITCH_PIN ((uint32_t)(1 << 10))  /* P2.10 connected to switch */

/* GPIO Direction Definitions */
#define INPUT  0
#define OUTPUT 1

/* Boolean Values */
#define TRUE  1
#define FALSE 0

uint8_t dummy_counter = 0; /* Dummy counter */

/* Function prototypes */

/**
 * @brief Initialize the GPIO peripheral
 * @note This function initialize the GPIO pin 2.10 to generate an interrupt using the EINT0.
 */
void configure_port(void);

/**
 * @brief Initialize the external interrupt
 */
void configure_external_interrupt(void);

/**
 * @brief Overwrite the interrupt handle routine for GPIO (EINT0)
 */
void EINT0_IRQHandler(void);


void configure_port(void)
{
    PINSEL_CFG_Type pin_cfg; /* Create a variable to store the configuration of the pin */

    /* We need to configure the struct with the desired configuration */
    pin_cfg.Portnum = PINSEL_PORT_2;           /* The port number is 0 */
    pin_cfg.Pinnum = PINSEL_PIN_10;            /* The pin number is 22 */
    pin_cfg.Funcnum = PINSEL_FUNC_1;           /* The function number is 0 */
    pin_cfg.Pinmode = PINSEL_PINMODE_PULLUP;   /* The pin mode is pull-up */
    pin_cfg.OpenDrain = PINSEL_PINMODE_NORMAL; /* The pin is in the normal mode */

    /* Configure the pin */
    PINSEL_ConfigPin(&pin_cfg);

    pin_cfg.Pinnum = PINSEL_PIN_0; /* The pin number is 0 */
}

void configure_external_interrupt(void)
{
    EXTI_InitTypeDef exti_cfg;
	exti_cfg.EXTI_Line = EXTI_EINT0;
	exti_cfg.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
	exti_cfg.EXTI_polarity = EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;
	EXTI_Config(&exti_cfg);
}	

// Overwrite the interrupt handle routine for GPIO
void EINT0_IRQHandler(void)
{
    EXTI_ClearEXTIFlag(EXTI_EINT0); /* Clear the interrupt flag */
    dummy_counter++; /* Increment the dummy counter */
}


/**
 * @brief Main function.
 */
int main(void)
{
    SystemInit(); /* Initialize the system clock (default: 100 MHz) */

    configure_port(); /* Configure GPIO pins */

    configure_external_interrupt(); /* Configure external interrupt */

    NVIC_SetPriority(EINT0_IRQn, 1); /* Set the priority of EINT */ 
    // This is optional, but it is a good practice to set the priority of the interrupt depending on the application.

    while (TRUE)
    {
        /* Wait for SysTick interrupt */
    }

    return 0; /* Program should never reach this point */
}
