/*
 * @file alternative.c
 * @brief Alternative of Timer0 Example
 *
 * In this case, we take advantage of the toggle configuration of the different Match channels of Timer2 instead of 
 * toggling the LEDs through the interrupt handler of Timer0.
 * 
 * We use Timer2 because is the only Timer in the LPC1769 that has all 4 match outputs available, while Timer0 only has 2.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#ifdef __USE_MCUEXPRESSO
#include <cr_section_macros.h> /* MCUXpresso-specific macros */
#endif

#include "lpc17xx_gpio.h"   /* GPIO */
#include "lpc17xx_pinsel.h" /* Pin Configuration */
#include "lpc17xx_timer.h"  /* Timer */

/* Pin Definitions */
#define LED0 ((uint32_t)(1 << 6)) // P0.6
#define LED1 ((uint32_t)(1 << 7)) // P0.7
#define LED2 ((uint32_t)(1 << 8)) // P0.8
#define LED3 ((uint32_t)(1 << 9)) // P0.9

#define FREQ_LED0 2    // LED0 frequency in Hz
#define FREQ_LED1 1    // LED1 frequency in Hz
#define FREQ_LED2 0.5  // LED2 frequency in Hz
#define FREQ_LED3 0.25 // LED3 frequency in Hz

#define HALF_PERIOD                                                                                                    \
    500000 // Half period for toggle, the number 500000 was derived from the assumption of dividing the timer clock by 2

#define OUTPUT 1 // GPIO direction for output

/* Prototype Functions */
void configure_timer_and_match(void);
void configure_port(void);

void configure_timer_and_match(void)
{
    TIM_TIMERCFG_Type timer_cfg_struct;
    TIM_MATCHCFG_Type match_cfg_struct;

    // Configure Timer2 in microsecond mode with a prescaler
    timer_cfg_struct.PrescaleOption = TIM_PRESCALE_USVAL; // Prescaler in microseconds
    timer_cfg_struct.PrescaleValue =
        100; // Prescaler for 100 MHz clock -> 1 µs resolution. Every 100us the timer will increment by 1
    TIM_Init(LPC_TIM2, TIM_TIMER_MODE, &timer_cfg_struct);

    // Configure match channel for LED0 (2 Hz toggle rate)
    match_cfg_struct.MatchChannel = 0;
    match_cfg_struct.IntOnMatch = DISABLE;
    match_cfg_struct.StopOnMatch = DISABLE;
    match_cfg_struct.ResetOnMatch = ENABLE;
    match_cfg_struct.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;
    match_cfg_struct.MatchValue = (uint32_t)(HALF_PERIOD / FREQ_LED0); // Half period for toggle
    TIM_ConfigMatch(LPC_TIM2, &match_cfg_struct);

    // Configure match channel for LED1 (1 Hz toggle rate)
    match_cfg_struct.MatchChannel = 1;
    match_cfg_struct.MatchValue = (uint32_t)(HALF_PERIOD / FREQ_LED1);
    TIM_ConfigMatch(LPC_TIM2, &match_cfg_struct);

    // Configure match channel for LED2 (0.5 Hz toggle rate)
    match_cfg_struct.MatchChannel = 2;
    match_cfg_struct.MatchValue = (uint32_t)(HALF_PERIOD / FREQ_LED2);
    TIM_ConfigMatch(LPC_TIM2, &match_cfg_struct);

    // Configure match channel for LED3 (0.25 Hz toggle rate)
    match_cfg_struct.MatchChannel = 3;
    match_cfg_struct.MatchValue = (uint32_t)(HALF_PERIOD / FREQ_LED3);
    TIM_ConfigMatch(LPC_TIM2, &match_cfg_struct);
}

void configure_port(void)
{
    PINSEL_CFG_Type pin_cfg_struct;

    // Configure Match outputs for Timer2 
    // MAT2.0
    pin_cfg_struct.Portnum = PINSEL_PORT_0;
    pin_cfg_struct.Pinnum = PINSEL_PIN_6;
    pin_cfg_struct.Funcnum = PINSEL_FUNC_3;
    pin_cfg_struct.Pinmode = PINSEL_PINMODE_PULLUP;
    pin_cfg_struct.OpenDrain = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&pin_cfg_struct);

    // MAT2.1 
    pin_cfg_struct.Pinnum = PINSEL_PIN_7;
    PINSEL_ConfigPin(&pin_cfg_struct);

    // MAT2.2
    pin_cfg_struct.Pinnum = PINSEL_PIN_8;
    PINSEL_ConfigPin(&pin_cfg_struct);

    // MAT2.3
    pin_cfg_struct.Pinnum = PINSEL_PIN_9;
    PINSEL_ConfigPin(&pin_cfg_struct);

    // Set the LED pins as output
    GPIO_SetDir(PINSEL_PORT_0, LED0 | LED1 | LED2 | LED3, OUTPUT);
}

/**
 * @brief Start Timer0.
 */
void start_timer(void)
{
    TIM_Cmd(LPC_TIM0, ENABLE); /* Start Timer0 */
}

int main(void)
{
    SystemInit(); // Initialize system clock

    configure_port();            // Configure GPIO
    configure_timer_and_match(); // Configure timer and match channels

    start_timer(); // Start timer

    while (TRUE)
    {
        // Infinite loop
    }

    return 0; // This never should be reached
}
