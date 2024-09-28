/*
 * @file main.c
 * @brief Timer0 Example
 *
 * This example demonstrates the use of Timer0 to toggle four LEDs at different frequencies using match interrupts.
 * It also explains the timer resolution and the maximum time the timer can count before overflowing.
 *
 * Timer Resolution:
 *   - Prescaler = 0, Clock = 100 MHz: Timer Resolution = 10 ns
 *   - Prescaler = 100, Clock = 100 MHz: Timer Resolution = 1.01 µs
 *
 * Maximum Time before Overflow:
 *   - Prescaler = 0: Maximum Time = 429.5 seconds (approx. 7 minutes 9.5 seconds)
 *   - Prescaler = 100: Maximum Time = 71.58 minutes (approx. 1 hour 11 minutes)
 *
 * In this example, we configure Timer0 to toggle four LEDs (P0.20 to P0.23) at different frequencies.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#ifdef __USE_MCUEXPRESSO
#include <cr_section_macros.h> /* MCUXpresso-specific macros */
#endif

#include "lpc17xx_gpio.h"   /* GPIO */
#include "lpc17xx_pinsel.h" /* Pin Configuration */
#include "lpc17xx_timer.h"  /* Timer0 */

/* Pin Definitions */
#define LED0 ((uint32_t)(1 << 20)) // P0.20
#define LED1 ((uint32_t)(1 << 21)) // P0.21
#define LED2 ((uint32_t)(1 << 22)) // P0.22
#define LED3 ((uint32_t)(1 << 23)) // P0.23

/* Frequency Definitions for LEDs */
#define FREQ_LED0 2    // LED0 frequency in Hz (2 Hz)
#define FREQ_LED1 1    // LED1 frequency in Hz (1 Hz)
#define FREQ_LED2 0.5  // LED2 frequency in Hz (0.5 Hz)
#define FREQ_LED3 0.25 // LED3 frequency in Hz (0.25 Hz)

#define PRESCALE 100 // Prescaler value for 100 MHz clock, 1 µs resolution

#define OUTPUT 1 // GPIO direction for output
#define INPUT  0 // GPIO direction for input

/* Half Period for Timer Match (assuming prescaler value gives 1µs resolution) */
#define HALF_PERIOD 500000 // Half period for toggling, derived from timer clock

/* Function Prototypes */
void configure_timer_and_match(void);
void configure_port(void);
void start_timer(void);

/**
 * @brief Configure Timer0 and match channels for toggling LEDs at different frequencies.
 */
void configure_timer_and_match(void)
{
    TIM_TIMERCFG_Type timer_cfg_struct;
    TIM_MATCHCFG_Type match_cfg_struct;

    // Configure Timer0 in microsecond mode with a prescaler
    timer_cfg_struct.PrescaleOption = TIM_PRESCALE_USVAL; // Prescaler in microseconds
    timer_cfg_struct.PrescaleValue = PRESCALE;            // Prescaler for 100 MHz clock -> 1 µs resolution
    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timer_cfg_struct);

    // Configure match channel for LED0 (2 Hz toggle rate)
    match_cfg_struct.MatchChannel = 0;
    match_cfg_struct.IntOnMatch = ENABLE;
    match_cfg_struct.StopOnMatch = DISABLE;
    match_cfg_struct.ResetOnMatch = ENABLE;
    match_cfg_struct.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;
    match_cfg_struct.MatchValue = (uint32_t)(HALF_PERIOD / FREQ_LED0); // Half period for toggle
    TIM_ConfigMatch(LPC_TIM0, &match_cfg_struct);

    // Configure match channel for LED1 (1 Hz toggle rate)
    match_cfg_struct.MatchChannel = 1;
    match_cfg_struct.MatchValue = (uint32_t)(HALF_PERIOD / FREQ_LED1);
    TIM_ConfigMatch(LPC_TIM0, &match_cfg_struct);

    // Configure match channel for LED2 (0.5 Hz toggle rate)
    match_cfg_struct.MatchChannel = 2;
    match_cfg_struct.MatchValue = (uint32_t)(HALF_PERIOD / FREQ_LED2);
    TIM_ConfigMatch(LPC_TIM0, &match_cfg_struct);

    // Configure match channel for LED3 (0.25 Hz toggle rate)
    match_cfg_struct.MatchChannel = 3;
    match_cfg_struct.MatchValue = (uint32_t)(HALF_PERIOD / FREQ_LED3);
    TIM_ConfigMatch(LPC_TIM0, &match_cfg_struct);
}

/**
 * @brief Configure GPIO for LEDs on pins P0.20 to P0.23.
 */
void configure_port(void)
{
    PINSEL_CFG_Type pin_cfg_struct;

    // Configure P0.20 to P0.23 as GPIO
    pin_cfg_struct.Portnum = PINSEL_PORT_0;
    pin_cfg_struct.Pinnum = PINSEL_PIN_20;
    pin_cfg_struct.Funcnum = PINSEL_FUNC_0;           // GPIO function
    pin_cfg_struct.Pinmode = PINSEL_PINMODE_PULLUP;   // Pull-up resistor
    pin_cfg_struct.OpenDrain = PINSEL_PINMODE_NORMAL; // Normal mode
    PINSEL_ConfigPin(&pin_cfg_struct);

    pin_cfg_struct.Pinnum = PINSEL_PIN_21;
    PINSEL_ConfigPin(&pin_cfg_struct);

    pin_cfg_struct.Pinnum = PINSEL_PIN_22;
    PINSEL_ConfigPin(&pin_cfg_struct);

    pin_cfg_struct.Pinnum = PINSEL_PIN_23;
    PINSEL_ConfigPin(&pin_cfg_struct);

    // Set P0.20 to P0.23 as output for LEDs
    GPIO_SetDir(PINSEL_PORT_0, LED0 | LED1 | LED2 | LED3, OUTPUT);
}

/**
 * @brief Start Timer0.
 */
void start_timer(void)
{
    // Start Timer0
    TIM_Cmd(LPC_TIM0, ENABLE);

    // Enable Timer0 interrupt
    NVIC_EnableIRQ(TIMER0_IRQn);
}

/**
 * @brief Timer0 interrupt handler to toggle LEDs.
 */
void TIMER0_IRQHandler(void)
{
    // Check and clear match interrupt for each channel

    if (TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT))
    {
        TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
        GPIO_SetValue(PINSEL_PORT_0, LED0); // Toggle LED0
    }
    if (TIM_GetIntStatus(LPC_TIM0, TIM_MR1_INT))
    {
        TIM_ClearIntPending(LPC_TIM0, TIM_MR1_INT);
        GPIO_SetValue(PINSEL_PORT_0, LED1); // Toggle LED1
    }
    if (TIM_GetIntStatus(LPC_TIM0, TIM_MR2_INT))
    {
        TIM_ClearIntPending(LPC_TIM0, TIM_MR2_INT);
        GPIO_SetValue(PINSEL_PORT_0, LED2); // Toggle LED2
    }
    if (TIM_GetIntStatus(LPC_TIM0, TIM_MR3_INT))
    {
        TIM_ClearIntPending(LPC_TIM0, TIM_MR3_INT);
        GPIO_SetValue(PINSEL_PORT_0, LED3); // Toggle LED3
    }
}

int main(void)
{
    SystemInit(); // Initialize system clock

    configure_port();            // Configure GPIO for LEDs
    configure_timer_and_match(); // Configure Timer0 and match channels

    start_timer(); // Start Timer0

    while (TRUE)
    {
        // Infinite loop
    }

    return 0; // This line should never be reached
}

/**
 * @brief Configure Timer1 for capture event on P1.18 (CAP1.0).
 */
void configTimer1(void)
{
    // P1.18 as CAP1.0
    PINSEL_CFG_Type pinCfg;
    pinCfg.Portnum = PINSEL_PORT_1;
    pinCfg.Pinnum = PINSEL_PIN_18;
    pinCfg.Funcnum = PINSEL_FUNC_3;
    pinCfg.Pinmode = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&pinCfg);

    // Timer1 Configuration
    TIM_TIMERCFG_Type timCfg;
    timCfg.PrescaleOption = TIM_PRESCALE_TICKVAL;
    timCfg.PrescaleValue = PRESCALE;

    // CAP1.0 Configuration for rising edge capture
    TIM_CAPTURECFG_Type capCfg;
    capCfg.CaptureChannel = 0;
    capCfg.FallingEdge = DISABLE;
    capCfg.IntOnCaption = ENABLE;
    capCfg.RisingEdge = ENABLE;

    // Initialize Timer1 in timer mode
    TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &timCfg);
    TIM_ConfigCapture(LPC_TIM1, &capCfg);
    TIM_Cmd(LPC_TIM1, ENABLE);

    // Enable Timer1 interrupt
    NVIC_EnableIRQ(TIMER1_IRQn);
}

/**
 * @brief Timer1 interrupt handler for capture event.
 */
void TIMER1_IRQHandler(void)
{
    TIM_ClearIntPending(LPC_TIM1, TIM_CR0_INT); // Clear capture interrupt

    static uint32_t oldCount = 0;
    uint32_t count = TIM_GetCaptureValue(LPC_TIM1, TIM_COUNTER_INCAP1);

    // Calculate period in microseconds
    uint32_t period = (count - oldCount) * 0.05; // Assuming 20 MHz clock (50 ns resolution)
    oldCount = count;                            // Update old count
    // We can use the period value here
}
