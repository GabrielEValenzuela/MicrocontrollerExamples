/*
 * @file main.c
 * @brief Timer0 and ADC example
 * 
 * A traditional wood-fired oven has been constructed to prepare the region's most flavorful roasted goat (cabritos).
 * Using an analog temperature sensor, the temperature inside the oven is monitored and displayed via three LEDs
 * (Green: Less than 40 degrees, Yellow: Less than 70 degrees, Red: Greater than 70 degrees).
 * 
 * This program uses Timer0 to trigger periodic ADC conversions every 60 seconds, reading the temperature value
 * and turning on the corresponding LED based on the temperature range. The temperature sensor is connected to ADC channel 7,
 * and the ADC is configured with a 12-bit resolution, providing 4096 discrete values between 0V and Vref (typically 3.3V).
 * The ADC sampling frequency is set to 100 kHz, and 8 readings are averaged to improve accuracy.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#ifdef __USE_MCUEXPRESSO
#include <cr_section_macros.h> /* MCUXpresso-specific macros */
#endif

#include "lpc17xx_timer.h"    /* Timer0 */
#include "lpc17xx_gpio.h"    /* GPIO */
#include "lpc17xx_pinsel.h"    /* Pin Configuration */
#include "lpc17xx_adc.h"    /* ADC */

/* Pin Definitions */
#define GREEN_LED ((uint32_t)(1 << 20)) /* P0.20 connected to LED */
#define YELLOW_LED ((uint32_t)(1 << 21)) /* P0.21 connected to LED */
#define RED_LED ((uint32_t)(1 << 22)) /* P0.22 connected to LED */
#define ADC_INPUT ((uint32_t)(1 << 2)) /* P0.2 connected to ADC */

#define GREEN_TMP 40
#define YELLOW_TMP 70

#define SECOND 10000

/* GPIO Direction Definitions */
#define INPUT  0
#define OUTPUT 1

/* Define frequency variables */
#define ADC_FREQ 100000 /* 100 kHz */

/* Boolean Values */
#define TRUE  1
#define FALSE 0

static uint32_t adc_read_value = 0;

/* Function declarations */
void configure_port(void);
void configure_adc(void);
void configure_timer_and_match(void);
void start_timer(void);
void turn_on_led(void);

/**
 * @brief Initialize the GPIO pins for the LEDs.
 */
void configure_port(void)
{
    PINSEL_CFG_Type pin_cfg_struct; /* Create a variable to store the configuration of the pin */

    /* Configure P0.20, P0.21, and P0.22 as GPIO for LED outputs */
    pin_cfg_struct.Portnum = PINSEL_PORT_0;           /* Port number is 0 */
    pin_cfg_struct.Pinnum = PINSEL_PIN_20;            /* Pin number 20 */
    pin_cfg_struct.Funcnum = PINSEL_FUNC_0;           /* Function number is 0 (GPIO) */
    pin_cfg_struct.Pinmode = PINSEL_PINMODE_PULLUP;   /* Pin mode is pull-up */
    pin_cfg_struct.OpenDrain = PINSEL_PINMODE_NORMAL; /* Normal mode */

    /* Configure the green LED pin */
    PINSEL_ConfigPin(&pin_cfg_struct);

    /* Configure the yellow LED pin */
    pin_cfg_struct.Pinnum = PINSEL_PIN_21;
    PINSEL_ConfigPin(&pin_cfg_struct);

    /* Configure the red LED pin */
    pin_cfg_struct.Pinnum = PINSEL_PIN_22;
    PINSEL_ConfigPin(&pin_cfg_struct);

    /* Set the LED pins as output */
    GPIO_SetDir(PINSEL_PORT_0, GREEN_LED | YELLOW_LED | RED_LED, OUTPUT);
}

/**
 * @brief Configure the ADC to sample the temperature sensor at 100 kHz.
 */
void configure_adc(void)
{
    ADC_Init(LPC_ADC, ADC_FREQ); /* Initialize the ADC peripheral with a 100 kHz sampling frequency */
    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_7, ENABLE); /* Enable ADC channel 7 */
    ADC_IntConfig(LPC_ADC, ADC_CHANNEL_7, ENABLE); /* Enable interrupt for ADC channel 7 */
}

/**
 * @brief Configure Timer0 to trigger an interrupt every 60 seconds.
 */
void configure_timer_and_match(void)
{
    TIM_TIMERCFG_Type timer_cfg_struct; /* Create a variable to store the configuration of the timer */

    timer_cfg_struct.PrescaleOption = TIM_PRESCALE_USVAL; /* Prescaler is in microseconds */
    timer_cfg_struct.PrescaleValue = (uint32_t) 100; /* Prescaler value is 100, giving a time resolution of ~1.01 µs */

    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timer_cfg_struct); /* Initialize Timer0 */

    TIM_MATCHCFG_Type match_cfg_struct; /* Create a variable to store the configuration of the match */

    match_cfg_struct.MatchChannel = 0; /* Match channel 0 */
    match_cfg_struct.IntOnMatch = ENABLE; /* Enable interrupt on match */
    match_cfg_struct.StopOnMatch = DISABLE; /* Do not stop the timer on match */
    match_cfg_struct.ResetOnMatch = ENABLE; /* Reset the timer on match */
    match_cfg_struct.ExtMatchOutputType = TIM_EXTMATCH_NOTHING; /* No external match output */
    match_cfg_struct.MatchValue = (uint32_t)(60 * SECOND); /* Match value set for 60 seconds */

    TIM_ConfigMatch(LPC_TIM0, &match_cfg_struct); /* Configure the match */
}

/**
 * @brief Start Timer0.
 */
void start_timer(void)
{
    TIM_Cmd(LPC_TIM0, ENABLE); /* Enable the timer */
    NVIC_EnableIRQ(TIMER0_IRQn); /* Enable the Timer0 interrupt */
}

/**
 * @brief Turn on the appropriate LED based on the temperature value.
 */
void turn_on_led(void)
{
    if (adc_read_value <= GREEN_TMP)
    {
        GPIO_SetValue(PINSEL_PORT_0, GREEN_LED);
        GPIO_ClearValue(PINSEL_PORT_0, RED_LED | YELLOW_LED);
    }
    else if (adc_read_value <= YELLOW_TMP)
    {
        GPIO_SetValue(PINSEL_PORT_0, YELLOW_LED);
        GPIO_ClearValue(PINSEL_PORT_0, RED_LED | GREEN_LED);
    }
    else
    {
        GPIO_SetValue(PINSEL_PORT_0, RED_LED);
        GPIO_ClearValue(PINSEL_PORT_0, YELLOW_LED | GREEN_LED);
    }
}

// ----------------- Interrupt Handler Functions -----------------

/**
 * @brief Timer0 Interrupt Handler.
 *        Clears the interrupt flag and starts an ADC conversion.
 */
void TIMER0_IRQHandler()
{
    TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT); /* Clear the interrupt flag */
    ADC_StartCmd(LPC_ADC, ADC_START_NOW); /* Start the ADC conversion */
}

/**
 * @brief ADC Interrupt Handler.
 *        Averages 8 ADC readings and updates the LED state.
 */
void ADC_IRQHandler()
{
    NVIC_DisableIRQ(ADC_IRQn); /* Disable the ADC interrupt */
    adc_read_value = 0;
    for (uint32_t lecture = 0; lecture < 8; ++lecture)
    {
        while (!(ADC_ChannelGetStatus(LPC_ADC, lecture, ADC_DATA_DONE))); /* Wait for the ADC conversion to finish */
        adc_read_value += ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_7); /* Read the ADC value */
    }
    adc_read_value /= 8; /* Calculate the average value */
    turn_on_led(); /* Call logic of LED switching */
    NVIC_EnableIRQ(ADC_IRQn); /* Enable the ADC interrupt */
}

/**
 * @brief Main function.
 */
int main(void)
{
    SystemInit(); /* Initialize the system clock (default: 100 MHz) */
    configure_port(); /* Configure the GPIO ports */
    configure_adc(); /* Configure the ADC */
    configure_timer_and_match(); /* Configure Timer0 */
    start_timer(); /* Start Timer0 */

    while (TRUE)
    {
        /* Wait for interrupts */
    }

    return 0; /* Program should never reach this point */
}
