/*
 * @file main.c
 * @brief Timer0 Example
 *
 * The timer resolution determines how finely we can control the timing. For the LPC1769, with a clock of 100 MHz and a
 * prescaler, the timer resolution is calculated as:
 *
 * Timer Resolution = (Prescaler + 1) / PCLK
 *
 * With a clock of 100 MHz and a prescaler of 0, the timer resolution is:
 *
 * Timer Resolution = (0 + 1) / 100 MHz = 10 ns
 *
 * If you set the prescaler to 100, the clock period becomes:
 *
 * Timer Resolution = (100 + 1) / 100 MHz = 1.01 µs
 *
 * The maximum time the timer can count before overflowing depends on the timer's resolution and the maximum 32-bit
 * count. Using the formula:
 *
 * Maximum Time = Timer Resolution * 2^32
 *
 * The maximum time the timer can count before overflowing is:
 *
 * Maximum Time = 10 ns * 2^32 = 429.4967296 s
 *
 * Which is approximately 429.5 seconds or 7 minutes and 9.5 seconds.
 *
 * With a prescaler of 100, the maximum time the timer can count before overflowing is:
 *
 * Maximum Time = 1.01 µs * 2^32 = 71.58 minutes
 *
 * Which is approximately 1 hour and 11 minutes.
 *
 * In this example, we will configure Timer0 to toggle four LEDs at different frequencies.
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
#define LED0 ((uint32_t)(1 << 20)) // P0.20
#define LED1 ((uint32_t)(1 << 21)) // P0.21
#define LED2 ((uint32_t)(1 << 22)) // P0.22
#define LED3 ((uint32_t)(1 << 23)) // P0.23

#define HALF_PERIOD                                                                                                    \
    500000 // Half period for toggle, the number 500000 was derived from the assumption of dividing the timer clock by 2

#define OUTPUT 1 // GPIO direction for output

/*
 * Because all match channels share the same Timer, its difficult to have different frequencies for each LED using just
 * one Timer. To solve this problem, we can take advantage that all LEDs share a base frequency and use a match counter
 * to keep track of the number of times the match value has been reached. This way, we can toggle the LEDs at different
 * frequencies without using multiple timers.
 *
 * This isnt the right solution, because it forces us to make all LEDs share a base frequency, in this case, LED0.
 * The other LEDs will have to toggle at a rate proportional to the base frequency, for example:
 *
 * LED1 has half the frequency of LED0
 * LED2 has half the frequency of LED1
 * LED3 has half the frequency of LED2.
 *
 * The file alternative.c shows a different approach to this problem, giving us more versatility and the ability to
 * adapt to situations where the frequencies of the different LEDs are not shared or do not mantain a defined
 * relationship.
 */
#define FREQ_LED0 2 // LED0 frequency in Hz -> Match at 25000 (2.5s) -> Toggle each time match is reached
#define FREQ_LED1 1 // LED1 frequency in Hz -> Match at 50000 (5s) -> Toggle every time the match counter is raised by 2
#define FREQ_LED2                                                                                                      \
    0.5 // LED2 frequency in Hz -> Match at 100000 (10s) -> Toggle every time the match counter is raised by 4
#define FREQ_LED3                                                                                                      \
    0.25 // LED3 frequency in Hz -> Match at 200000 (20s) -> Toggle every time the match counter is raised by 8
uint8_t match_counter = 0;

/* Prototype Functions */
void configure_timer_and_match(void);
void configure_port(void);
void start_timer(void);
void toggle_led(uint32_t led);

void configure_timer_and_match(void)
{
    TIM_TIMERCFG_Type timer_cfg_struct;
    TIM_MATCHCFG_Type match_cfg_struct;

    // Configure Timer0 in microsecond mode with a prescaler
    timer_cfg_struct.PrescaleOption = TIM_PRESCALE_USVAL; // Prescaler in microseconds
    timer_cfg_struct.PrescaleValue =
        100; // Prescaler for 100 MHz clock -> 1 µs resolution. Every 100us the timer will increment by 1
    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timer_cfg_struct);

    // Configure match channel for LED0 (2 Hz toggle rate)
    match_cfg_struct.MatchChannel = 0;
    match_cfg_struct.IntOnMatch = ENABLE;
    match_cfg_struct.StopOnMatch = DISABLE;
    match_cfg_struct.ResetOnMatch = ENABLE;
    match_cfg_struct.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
    match_cfg_struct.MatchValue = (uint32_t)(HALF_PERIOD / FREQ_LED0); // Half period for toggle
    TIM_ConfigMatch(LPC_TIM0, &match_cfg_struct);
}

void configure_port(void)
{
    PINSEL_CFG_Type pin_cfg_struct;

    // Configure P0.20 to P0.23 as GPIO
    pin_cfg_struct.Portnum = PINSEL_PORT_0;
    pin_cfg_struct.Pinnum = PINSEL_PIN_20;
    pin_cfg_struct.Funcnum = PINSEL_FUNC_0;
    pin_cfg_struct.Pinmode = PINSEL_PINMODE_PULLUP;
    pin_cfg_struct.OpenDrain = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&pin_cfg_struct);

    pin_cfg_struct.Pinnum = PINSEL_PIN_21;
    PINSEL_ConfigPin(&pin_cfg_struct);

    pin_cfg_struct.Pinnum = PINSEL_PIN_22;
    PINSEL_ConfigPin(&pin_cfg_struct);

    pin_cfg_struct.Pinnum = PINSEL_PIN_23;
    PINSEL_ConfigPin(&pin_cfg_struct);

    // Set P0.20 to P0.23 as output
    GPIO_SetDir(PINSEL_PORT_0, LED0 | LED1 | LED2 | LED3, OUTPUT);
}

void toggle_led(uint32_t led)
{
    if (GPIO_ReadValue(PINSEL_PORT_0) & led)
    {
        GPIO_ClearValue(PINSEL_PORT_0, led);
    }
    else
    {
        GPIO_SetValue(PINSEL_PORT_0, led);
    }
}

void TIMER0_IRQHandler(void)
{
    TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
    toggle_led(LED0);
    match_counter++;
    switch (match_counter)
    {
        case 2:
        case 6: toggle_led(LED1); break;
        case 4:
            toggle_led(LED1);
            toggle_led(LED2);
            break;
        case 8:
            toggle_led(LED1);
            toggle_led(LED2);
            toggle_led(LED3);
            match_counter = 0;
            break;
    }
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
