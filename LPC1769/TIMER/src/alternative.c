/*
 * @file alternative.c
 * @brief Alternative of Timer Example
 *
 * In this case, we take advantage of the toggle configuration for the Match channels of different Timers, instead of
 * toggling the LEDs through the Timer0 interrupt handler. This gives us more versatility and allows our program to
 * adapt to situations where the frequencies of the different LEDs are not shared or do not mantain a defined
 * relationship.
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
#define LED0 ((uint32_t)(1 << 6)) // P1.28 -> MAT0.0
#define LED1 ((uint32_t)(1 << 7)) // P1.22 -> MAT1.0
#define LED2 ((uint32_t)(1 << 8)) // P0.6 -> MAT2.0
#define LED3 ((uint32_t)(1 << 9)) // P0.10 -> MAT3.0

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
void start_timers(void);

void configure_timer_and_match(void)
{
    TIM_TIMERCFG_Type timer_cfg_struct;
    TIM_MATCHCFG_Type match_cfg_struct;

    // Configure Timer2 in microsecond mode with a prescaler
    timer_cfg_struct.PrescaleOption = TIM_PRESCALE_USVAL; // Prescaler in microseconds
    timer_cfg_struct.PrescaleValue =
        100; // Prescaler for 100 MHz clock -> 1 µs resolution. Every 100us the timer will increment by 1
    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timer_cfg_struct);
    TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &timer_cfg_struct);
    TIM_Init(LPC_TIM2, TIM_TIMER_MODE, &timer_cfg_struct);
    TIM_Init(LPC_TIM3, TIM_TIMER_MODE, &timer_cfg_struct);

    // Configure match channel for LED0
    match_cfg_struct.MatchChannel = 0;
    match_cfg_struct.IntOnMatch =
        DISABLE; // In this example, we don't need the interrupt because the toggle is done automatically
    match_cfg_struct.StopOnMatch = DISABLE;
    match_cfg_struct.ResetOnMatch = ENABLE;
    match_cfg_struct.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE; // !In this case, we take advantage of this feature
    match_cfg_struct.MatchValue = (uint32_t)(HALF_PERIOD / FREQ_LED0); // Half period for toggle
    TIM_ConfigMatch(LPC_TIM0, &match_cfg_struct);

    // Configure Timer1 and match channel for LED1
    match_cfg_struct.MatchValue = (uint32_t)(HALF_PERIOD / FREQ_LED1);
    TIM_ConfigMatch(LPC_TIM1, &match_cfg_struct);

    // Configure Timer2 and match channel for LED2
    match_cfg_struct.MatchValue = (uint32_t)(HALF_PERIOD / FREQ_LED2);
    TIM_ConfigMatch(LPC_TIM2, &match_cfg_struct);

    // Configure Timer3 and match channel for LED3
    match_cfg_struct.MatchValue = (uint32_t)(HALF_PERIOD / FREQ_LED3);
    TIM_ConfigMatch(LPC_TIM3, &match_cfg_struct);
}

void configure_port(void)
{
    PINSEL_CFG_Type pin_cfg_struct;

    // Configure Match outputs for all timers
    // MAT0.0 -> LED0
    pin_cfg_struct.Portnum = PINSEL_PORT_1;
    pin_cfg_struct.Pinnum = PINSEL_PIN_28;
    pin_cfg_struct.Funcnum = PINSEL_FUNC_3; // Match output
    pin_cfg_struct.Pinmode = PINSEL_PINMODE_PULLUP;
    pin_cfg_struct.OpenDrain = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&pin_cfg_struct);

    // MAT1.0 -> LED1
    pin_cfg_struct.Pinnum = PINSEL_PIN_22;
    PINSEL_ConfigPin(&pin_cfg_struct);

    // MAT2.0 -> LED2
    pin_cfg_struct.Portnum = PINSEL_PORT_0;
    pin_cfg_struct.Pinnum = PINSEL_PIN_6;
    PINSEL_ConfigPin(&pin_cfg_struct);

    // MAT3.0 -> LED3
    pin_cfg_struct.Pinnum = PINSEL_PIN_10;
    PINSEL_ConfigPin(&pin_cfg_struct);

    // Set the LED pins as output
    GPIO_SetDir(PINSEL_PORT_1, LED0 | LED1, OUTPUT);
    GPIO_SetDir(PINSEL_PORT_0, LED2 | LED3, OUTPUT);
}

/**
 * @brief Start all timers
 */
void start_timers(void)
{
    TIM_Cmd(LPC_TIM0, ENABLE); /* Start Timer0 */
    TIM_Cmd(LPC_TIM1, ENABLE); /* Start Timer1 */
    TIM_Cmd(LPC_TIM2, ENABLE); /* Start Timer2 */
    TIM_Cmd(LPC_TIM3, ENABLE); /* Start Timer3 */
}

int main(void)
{
    SystemInit(); // Initialize system clock

    configure_port();            // Configure GPIO
    configure_timer_and_match(); // Configure timer and match channels

    start_timers(); // Start timers

    while (TRUE)
    {
        // Infinite loop
    }

    return 0; // This never should be reached
}
