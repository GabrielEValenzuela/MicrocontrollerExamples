/*
 * @file main.c
 * @brief Timer0, ADC, and DMA example to control LED based on temperature sensor
 *
 * A traditional wood-fired oven is monitored using an analog temperature sensor.
 * The temperature inside the oven is displayed using three LEDs:
 * - Green: Less than 40 degrees
 * - Yellow: Less than 70 degrees
 * - Red: Greater than 70 degrees
 *
 * Timer0 is used to trigger periodic ADC conversions every 60 seconds.
 * DMA is used to transfer ADC conversion data to a buffer automatically.
 * The temperature sensor is connected to ADC1 (PA1).
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/dma.h>

/* Constants */
#define GREEN_LED_PORT GPIOA
#define GREEN_LED_PIN  GPIO8  /* PA8 (Green LED) */
#define YELLOW_LED_PORT GPIOC
#define YELLOW_LED_PIN  GPIO15 /* PC15 (Yellow LED) */
#define RED_LED_PORT GPIOC
#define RED_LED_PIN GPIO14 /* PC14 (Red LED) */
#define ADC_CHANNEL_TEMP_SENSOR ADC_CHANNEL1  /* PA1 ADC1 */

#define TEMP_GREEN_THRESHOLD 40  /* 40 degrees */
#define TEMP_YELLOW_THRESHOLD 70 /* 70 degrees */

#define ADC_BUFFER_SIZE 16  /* Buffer size for averaging */

/* Global Variables */
uint16_t adc_buffer[ADC_BUFFER_SIZE];  // Buffer to store ADC values

/* Function Prototypes */
void system_clock_setup(void);
void gpio_setup(void);
void adc_setup(void);
void timer2_setup(void);
void dma_setup(void);
void control_leds_based_on_temp(uint16_t temp);

/**
 * @brief Configures the system clock to 72 MHz using an 8 MHz external crystal.
 */
void system_clock_setup(void)
{
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
}

/**
 * @brief Configures GPIO pins for the three LEDs.
 */
void gpio_setup(void)
{
    /* Enable GPIO clocks */
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOC);

    /* Configure PA8 (Green LED) as output */
    gpio_set_mode(GREEN_LED_PORT, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GREEN_LED_PIN);

    /* Configure PC14 (Red LED) and PC15 (Yellow LED) as output */
    gpio_set_mode(RED_LED_PORT, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, RED_LED_PIN);
    gpio_set_mode(YELLOW_LED_PORT, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, YELLOW_LED_PIN);
}

/**
 * @brief Configures ADC1 with DMA for temperature sensor readings.
 */
void adc_setup(void)
{
    /* Enable ADC1 clock */
    rcc_periph_clock_enable(RCC_ADC1);

    /* Configure ADC1 */
    adc_power_off(ADC1);
    adc_disable_scan_mode(ADC1);
    adc_set_continuous_conversion_mode(ADC1);
    adc_disable_external_trigger_regular(ADC1);
    adc_set_right_aligned(ADC1);
    adc_set_sample_time(ADC1, ADC_CHANNEL_TEMP_SENSOR, ADC_SMPR_SMP_55DOT5CYC); /* Set sample time */

    /* Calibrate ADC1 */
    adc_power_on(ADC1);
    adc_reset_calibration(ADC1);
    adc_calibrate(ADC1);
}

/**
 * @brief Configures Timer 2 to trigger ADC conversion every 60 seconds.
 */
void timer2_setup(void)
{
    /* Enable Timer 2 clock */
    rcc_periph_clock_enable(RCC_TIM2);

    /* Reset Timer 2 peripheral */
    timer_reset(TIM2);

    /* Set timer prescaler for 1Hz (1 tick per second) */
    timer_set_prescaler(TIM2, 7200 - 1);  // 72 MHz / 7200 = 10 kHz
    timer_set_period(TIM2, 60000 - 1);    // 10 kHz / 60000 = 1Hz (every 60 seconds)

    /* Enable Timer 2 interrupt for periodic triggering */
    timer_enable_irq(TIM2, TIM_DIER_UIE);
    nvic_enable_irq(NVIC_TIM2_IRQ);

    /* Start the timer */
    timer_enable_counter(TIM2);
}

/**
 * @brief Configures DMA1 to transfer ADC conversion data to memory buffer.
 */
void dma_setup(void)
{
    /* Enable DMA1 clock */
    rcc_periph_clock_enable(RCC_DMA1);

    /* Reset DMA1 stream */
    dma_stream_reset(DMA1, DMA_CHANNEL1);

    /* Set DMA configuration */
    dma_set_peripheral_address(DMA1, DMA_CHANNEL1, (uint32_t)&ADC_DR(ADC1)); /* ADC data register */
    dma_set_memory_address(DMA1, DMA_CHANNEL1, (uint32_t)adc_buffer);        /* Memory buffer */
    dma_set_number_of_data(DMA1, DMA_CHANNEL1, ADC_BUFFER_SIZE);             /* Number of data items */
    dma_set_priority(DMA1, DMA_CHANNEL1, DMA_CCR_PL_LOW);
    dma_set_memory_size(DMA1, DMA_CHANNEL1, DMA_CCR_MSIZE_16BIT);  /* Memory size: 16 bits */
    dma_set_peripheral_size(DMA1, DMA_CHANNEL1, DMA_CCR_PSIZE_16BIT);  /* Peripheral size: 16 bits */
    dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL1);
    dma_enable_circular_mode(DMA1, DMA_CHANNEL1);  /* Enable circular mode */

    /* Start DMA transfer */
    dma_enable_channel(DMA1, DMA_CHANNEL1);

    /* Enable ADC DMA mode */
    adc_enable_dma(ADC1);
}

/**
 * @brief Computes the average of the ADC values in the buffer.
 * @return Averaged temperature value.
 */
uint16_t average_adc_value(void)
{
    uint32_t sum = 0;
    for (int i = 0; i < ADC_BUFFER_SIZE; i++) {
        sum += adc_buffer[i];
    }
    return (uint16_t)(sum / ADC_BUFFER_SIZE);
}

/**
 * @brief Control the LEDs based on the temperature reading.
 * @param temp The averaged ADC value corresponding to the temperature.
 */
void control_leds_based_on_temp(uint16_t temp)
{
    uint16_t temperature = (temp * 3.3 / 4096.0) * 100;  // Convert ADC value to temperature

    /* Control LEDs based on temperature range */
    if (temperature < TEMP_GREEN_THRESHOLD)
    {
        gpio_set(GREEN_LED_PORT, GREEN_LED_PIN);    /* Green LED on */
        gpio_clear(YELLOW_LED_PORT, YELLOW_LED_PIN);/* Yellow LED off */
        gpio_clear(RED_LED_PORT, RED_LED_PIN);      /* Red LED off */
    }
    else if (temperature < TEMP_YELLOW_THRESHOLD)
    {
        gpio_clear(GREEN_LED_PORT, GREEN_LED_PIN);  /* Green LED off */
        gpio_set(YELLOW_LED_PORT, YELLOW_LED_PIN);  /* Yellow LED on */
        gpio_clear(RED_LED_PORT, RED_LED_PIN);      /* Red LED off */
    }
    else
    {
        gpio_clear(GREEN_LED_PORT, GREEN_LED_PIN);  /* Green LED off */
        gpio_clear(YELLOW_LED_PORT, YELLOW_LED_PIN);/* Yellow LED off */
        gpio_set(RED_LED_PORT, RED_LED_PIN);        /* Red LED on */
    }
}

/**
 * @brief Timer 2 interrupt handler. Triggered every 60 seconds.
 * Reads the averaged ADC value from the buffer and controls the LEDs based on the temperature.
 */
void tim2_isr(void)
{
    if (timer_get_flag(TIM2, TIM_SR_UIF))
    {
        timer_clear_flag(TIM2, TIM_SR_UIF);  // Clear update interrupt flag

        uint16_t averaged_value = average_adc_value();  // Compute average ADC value
        control_leds_based_on_temp(averaged_value);     // Control LEDs based on temperature
    }
}

/**
 * @brief Main function.
 * Initializes system clock, GPIO, ADC, Timer, and DMA for periodic ADC conversion and LED control.
 */
int main(void)
{
    system_clock_setup();  /* Set up system clock */
    gpio_setup();          /* Configure GPIO pins */
    adc_setup();           /* Configure ADC1 */
    dma_setup();           /* Set up DMA for ADC */
    timer2_setup();        /* Configure Timer2 for periodic ADC conversion */

    while (TRUE)
    {
        __wfi();  /* Wait for interrupt */
    }

    return 0;
}
