/*
 * @file main.c
 * @brief Timer0, ADC, and DMA example for LPC1769
 *
 * A traditional wood-fired oven is monitored using an analog temperature sensor.
 * The temperature inside the oven is displayed using three LEDs:
 * - Green: Less than 30 degrees
 * - Yellow: Less than 50 degrees
 * - Red: Greater than 70 degrees
 *
 * This program uses Timer0 to trigger periodic ADC conversions every 60 seconds.
 * DMA is used to transfer ADC conversion data to a memory buffer automatically.
 * The temperature sensor is connected to ADC channel 7 (P0.2), and the ADC is configured
 * with a 12-bit resolution. DMA transfers the data to a buffer and the average reading is used to control LEDs.
 */

#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"

/* Pin Definitions */
#define GREEN_LED     ((uint32_t)(1 << 20)) /* P0.20 connected to LED */
#define YELLOW_LED    ((uint32_t)(1 << 21)) /* P0.21 connected to LED */
#define RED_LED       ((uint32_t)(1 << 22)) /* P0.22 connected to LED */
#define ADC_INPUT_PIN ((uint32_t)(1 << 2))  /* P0.2 connected to ADC channel 7 */

/* Temperature Thresholds */
#define GREEN_TMP  30
#define YELLOW_TMP 50
#define RED_TMP 70

/* Timer and ADC settings */
#define SECOND          10000
#define ADC_FREQ        100000        /* 100 kHz */
#define ADC_CHANNEL     ADC_CHANNEL_7 /* Using ADC channel 7 */
#define DMA_BUFFER_SIZE 16            /* Buffer size for averaging */

/* Global Variables */
static uint16_t adc_dma_buffer[DMA_BUFFER_SIZE]; /* Buffer to store ADC results */
static uint32_t adc_avg_value = 0;

/* Function declarations */
void configure_port(void);
void configure_adc(void);
void dma_setup(void);
void configure_timer_and_match(void);
void start_timer(void);
void turn_on_led(uint16_t avg_adc_value);
uint16_t average_adc_buffer(void);

/**
 * @brief Configure the GPIO pins for the LEDs.
 */
void configure_port(void)
{
    PINSEL_CFG_Type pin_cfg_struct;

    /* Configure P0.20, P0.21, and P0.22 as GPIO for LED outputs */
    pin_cfg_struct.Portnum = PINSEL_PORT_0;
    pin_cfg_struct.Funcnum = PINSEL_FUNC_0;           /* GPIO function */
    pin_cfg_struct.Pinmode = PINSEL_PINMODE_PULLUP;   /* Pin mode is pull-up */
    pin_cfg_struct.OpenDrain = PINSEL_PINMODE_NORMAL; /* Normal mode */

    /* Configure green LED (P0.20) */
    pin_cfg_struct.Pinnum = PINSEL_PIN_20;
    PINSEL_ConfigPin(&pin_cfg_struct);

    /* Configure yellow LED (P0.21) */
    pin_cfg_struct.Pinnum = PINSEL_PIN_21;
    PINSEL_ConfigPin(&pin_cfg_struct);

    /* Configure red LED (P0.22) */
    pin_cfg_struct.Pinnum = PINSEL_PIN_22;
    PINSEL_ConfigPin(&pin_cfg_struct);

    /* Set all LED pins as output */
    GPIO_SetDir(PINSEL_PORT_0, GREEN_LED | YELLOW_LED | RED_LED, OUTPUT);
}

/**
 * @brief Configure ADC to sample from channel 7 (P0.2) using DMA.
 */
void configure_adc(void)
{
    /* Initialize ADC */
    ADC_Init(LPC_ADC, ADC_FREQ);                  /* Initialize the ADC at 100 kHz */
    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL, ENABLE); /* Enable ADC channel 7 */
    ADC_BurstCmd(LPC_ADC, DISABLE);               /* Disable burst mode (timer-triggered) */
    ADC_DMACmd(LPC_ADC, ENABLE);                  /* Enable DMA for ADC */
}

/**
 * @brief Configure DMA to transfer ADC conversion data to memory buffer.
 */
void dma_setup(void)
{
    /* Initialize the DMA controller */
    GPDMA_Init();

    /* Configure DMA for ADC transfers */
    GPDMA_Channel_CFG_Type dma_config;
    dma_config.ChannelNum = 0;
    dma_config.SrcMemAddr = 0;                        /* Source is peripheral (ADC) */
    dma_config.DstMemAddr = (uint32_t)adc_dma_buffer; /* Destination is memory buffer */
    dma_config.TransferSize = DMA_BUFFER_SIZE;        /* Number of transfers */
    dma_config.TransferWidth = 0;                     /* Width is not used for ADC */
    dma_config.TransferType = GPDMA_TRANSFERTYPE_P2M; /* Peripheral to memory */
    dma_config.SrcConn = GPDMA_CONN_ADC;              /* ADC is the source */
    dma_config.DstConn = 0;                           /* Memory as destination */
    dma_config.DMALLI = 0;                            /* No linked list */

    GPDMA_Setup(&dma_config); /* Setup the DMA transfer */

    /* Enable DMA channel */
    GPDMA_ChannelCmd(0, ENABLE);
}

/**
 * @brief Configure Timer0 to trigger ADC conversion every 60 seconds.
 */
void configure_timer_and_match(void)
{
    /* Timer0 Configuration */
    TIM_TIMERCFG_Type timer_cfg;
    timer_cfg.PrescaleOption = TIM_PRESCALE_USVAL;
    timer_cfg.PrescaleValue = 100; /* 1 µs resolution */

    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timer_cfg);

    /* Match Configuration (every 60 seconds) */
    TIM_MATCHCFG_Type match_cfg;
    match_cfg.MatchChannel = 0;
    match_cfg.IntOnMatch = ENABLE;
    match_cfg.ResetOnMatch = ENABLE;
    match_cfg.StopOnMatch = DISABLE;
    match_cfg.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
    match_cfg.MatchValue = SECOND * 60; /* 60 seconds */

    TIM_ConfigMatch(LPC_TIM0, &match_cfg);

    NVIC_EnableIRQ(TIMER0_IRQn); /* Enable Timer0 interrupt */
}

/**
 * @brief Start Timer0.
 */
void start_timer(void)
{
    TIM_Cmd(LPC_TIM0, ENABLE); /* Start Timer0 */
}

/**
 * @brief Turn on the appropriate LED based on the average ADC value.
 * @param avg_adc_value The averaged ADC value (temperature reading).
 */
void turn_on_led(uint16_t avg_adc_value)
{
    if (avg_adc_value <= GREEN_TMP)
    {
        GPIO_SetValue(PINSEL_PORT_0, GREEN_LED);              /* Green LED on */
        GPIO_ClearValue(PINSEL_PORT_0, YELLOW_LED | RED_LED); /* Other LEDs off */
    }
    else if (avg_adc_value <= YELLOW_TMP)
    {
        GPIO_SetValue(PINSEL_PORT_0, YELLOW_LED);            /* Yellow LED on */
        GPIO_ClearValue(PINSEL_PORT_0, GREEN_LED | RED_LED); /* Other LEDs off */
    }
    else
    {
        GPIO_SetValue(PINSEL_PORT_0, RED_LED);                  /* Red LED on */
        GPIO_ClearValue(PINSEL_PORT_0, GREEN_LED | YELLOW_LED); /* Other LEDs off */
    }
}

/**
 * @brief Calculate the average value from the ADC buffer.
 * @return The averaged ADC value.
 */
uint16_t average_adc_buffer(void)
{
    uint32_t sum = 0;
    for (uint32_t i = 0; i < DMA_BUFFER_SIZE; i++)
    {
        sum += adc_dma_buffer[i];
    }
    return (uint16_t)(sum / DMA_BUFFER_SIZE);
}

/**
 * @brief Timer0 Interrupt Handler.
 *        Reads the ADC data via DMA and updates the LEDs.
 */
void TIMER0_IRQHandler(void)
{
    TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT); /* Clear the interrupt flag */

    /* Average the ADC buffer */
    adc_avg_value = average_adc_buffer();

    /* Update LEDs based on the average temperature */
    turn_on_led(adc_avg_value);

    /* Restart the DMA transfer for the next buffer */
    GPDMA_ChannelCmd(0, ENABLE);
}

/**
 * @brief Main function.
 */
int main(void)
{
    SystemInit();                /* Initialize system clock */
    configure_port();            /* Configure GPIO ports */
    configure_adc();             /* Configure ADC */
    dma_setup();                 /* Set up DMA for ADC */
    configure_timer_and_match(); /* Configure Timer0 */
    start_timer();               /* Start Timer0 */

    while (1)
    {
        __wfi(); /* Wait for interrupt */
    }

    return 0;
}