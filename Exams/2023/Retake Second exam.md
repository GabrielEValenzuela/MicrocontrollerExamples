# Exam 2023 - Retake Second

## Modules

- LPC17xx Peripheral Drivers (DAC, ADC, Timer, GPIO, DMA, EXTI)
- Embedded System Programming (signal processing and generation of a PWM signal, digital output based on voltage thresholds, waveform generation controlled by external interrupts)

## First Problem

Por un pin del ADC del microcontrolador LPC1769 ingresa una tensión de rango dinámico 0 a 3,3[v] proveniente de un sensor de temperatura. Debido a la baja tasa de variación de la señal, se pide tomar una muestra cada 30[s]. Pasados los 2[min] se debe promediar las últimas 4 muestras y en función de este valor, tomar una decisión sobre una salida digital de la placa:

- Si el valor es <1 [V] colocar la salida en 0 (0[V]).
- Si el valor es >= 1[V] y <=2[V] modular una señal PWM con un Ciclo de trabajo que va desde el 50% hasta el 90% proporcional al valor de tensión, con un periodo de 20[KHz].
- Si el valor es > 2[V] colocar la salida en 1 (3,3[V]).

<details><summary>Summary</summary>

Using the ADC, timers, and GPIOs of the LPC1769, this program should:

1. Sample a voltage signal every 30 seconds using the ADC.
2. After 2 minutes, calculate the average of the last 4 samples.
3. Based on the average voltage:
   - If **< 1V**, set the digital output pin to **0V**.
   - If **between 1V and 2V**, generate a **20 kHz PWM** with a duty cycle proportional to the voltage.
   - If **> 2V**, set the digital output pin to **3.3V**.
4. Handle the generation of the PWM signal using interrupts and timers.

Considering that:

1. Commented code: Ensure clarity in how each module works.
2. Efficient design: Optimize the ADC sampling and PWM signal generation process.

</details>

<details><summary>Solution</summary>

```c
/**
* @file r2-2023-ex1.c
* @brief Solution for the First Problem of the Retake for the Second 2023 Exam from Digital Electronics 3
* @author Ignacio Ledesma
* @license MIT
* @date 2024-11
*/

#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"

// Define ADC channel and digital output pin
#define ADC_CHANNEL_2 2              // ADC channel
#define OUTPUT_PIN (1 << 20)         // Assuming the output pin is P0.20
#define OUTPUT 1                     // Output pin configuration

#define N_SAMPLES 4                  // Number of samples for averaging

// Frequencies and times
#define ADC_FREQ 100000              // ADC sampling frequency (100 kHz)
#define PWM_PERIOD 50                // 1/20 kHz = 50 us

// Variables for storing samples and average
float samples[N_SAMPLES] = {0};      // Array for the last 4 samples
uint8_t index = 0;                  // Index for the sample array

// Function prototypes
void config_pins(void);
void init_adc(void);
void init_timer0(void);
void init_timer1(void);
float read_average(void);
void process_output(float average);
void update_sample(void);

int main(void) {
    // Initialization
    SystemInit();
    config_pins();
    init_adc();
    init_timer0();
    init_timer1();

    while (1) {
        // The main loop is controlled by timer interrupts
    }

    return 0;
}

void config_pins(void) {
    // Digital Output
    PINSEL_CFG_Type pin;
    pin.Portnum = PINSEL_PORT_0;
    pin.Pinnum = PINSEL_PIN_20;
    pin.Funcnum = PINSEL_FUNC_0; // GPIO
    pin.Pinmode = PINSEL_PINMODE_PULLDOWN;
    pin.OpenDrain = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&pin);
    GPIO_SetDir(PINSEL_PORT_0, OUTPUT_PIN, OUTPUT);  // Set P0.20 as output

    // ADC Channel 2
    pin.Pinnum = PINSEL_PIN_25; // Channel 2
    pin.Funcnum = PINSEL_FUNC_1; // ADC
    pin.Pinmode = PINSEL_PINMODE_TRISTATE; // Avoid imprecisions
    PINSEL_ConfigPin(&pin);
}

void init_adc(void) {
    ADC_Init(LPC_ADC, ADC_FREQ);
    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_2, ENABLE);  // Enable ADC channel 2
}

void init_timer0(void) {
    TIM_TIMERCFG_Type timer_cfg;
    TIM_MATCHCFG_Type match_cfg;

    // Configure Timer0 for interrupts every 30 seconds
    timer_cfg.PrescaleOption = TIM_PRESCALE_USVAL;
    timer_cfg.PrescaleValue = 1000000;  // 1 second
    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timer_cfg);

    // Set Match0 to occur every 30 seconds
    match_cfg.MatchChannel = 0;
    match_cfg.IntOnMatch = ENABLE;
    match_cfg.ResetOnMatch = ENABLE;
    match_cfg.StopOnMatch = DISABLE;
    match_cfg.MatchValue = 30;  // 30 seconds
    TIM_ConfigMatch(LPC_TIM0, &match_cfg);

    TIM_Cmd(LPC_TIM0, ENABLE);
    NVIC_EnableIRQ(TIMER0_IRQn);
}

void init_timer1(void) {
    TIM_TIMERCFG_Type timer_cfg;
    TIM_MATCHCFG_Type match_cfg;

    // Configure Timer1 for PWM generation
    timer_cfg.PrescaleOption = TIM_PRESCALE_USVAL;
    timer_cfg.PrescaleValue = 1;  // 1 microsecond per tick
    TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &timer_cfg);

    // Match0 = duty cycle end
    match_cfg.MatchChannel = 0;
    match_cfg.IntOnMatch = ENABLE;
    match_cfg.ResetOnMatch = DISABLE;
    match_cfg.StopOnMatch = DISABLE;
    match_cfg.MatchValue = 25;  // Duty cycle starts at 50% minimum
    TIM_ConfigMatch(LPC_TIM1, &match_cfg);

    // Match1 = new cycle
    match_cfg.MatchChannel = 1;
    match_cfg.IntOnMatch = ENABLE;
    match_cfg.ResetOnMatch = ENABLE;
    match_cfg.StopOnMatch = DISABLE;
    match_cfg.MatchValue = PWM_PERIOD;
    TIM_ConfigMatch(LPC_TIM1, &match_cfg);

    NVIC_EnableIRQ(TIMER1_IRQn);
}

void TIMER0_IRQHandler(void) {
    update_sample();  // Update ADC sample

    // After 2 minutes (4 samples), calculate the average and process the output
    if (index == 4) {
        index = 0;  // Reset index
        float average = read_average();
        process_output(average);
    }

    TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
}

void update_sample(void) {
    ADC_StartCmd(LPC_ADC, ADC_START_NOW);

    // Wait for conversion to complete
    while (!ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_2, ADC_DATA_DONE));

    uint16_t adc_value = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_2);
    float voltage = (adc_value / 4095.0) * 3.3; // Convert ADC to voltage

    samples[index] = voltage;
    index++;
}

float read_average(void) {
    float sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += samples[i];
    }
    return sum / 4;
}

void process_output(float average) {
    if (average < 1.0) {
        // Disable Timer1
        TIM_Cmd(LPC_TIM1, DISABLE);
        // Set digital output to 0V
        GPIO_ClearValue(PINSEL_PORT_0, OUTPUT_PIN);
    } else if (average >= 1.0 && average <= 2.0) {
        // Generate PWM with duty cycle proportional to average
        uint32_t duty_cycle_time = (50 + (40 * (average - 1.0))) * 50;  // Duty cycle 50%-90%
        TIM_UpdateMatchValue(LPC_TIM1, 0, duty_cycle_time);
        TIM_ResetCounter(LPC_TIM1);
        TIM_Cmd(LPC_TIM1, ENABLE);
    } else {
        // Disable Timer1
        TIM_Cmd(LPC_TIM1, DISABLE);
        // Set digital output to 3.3V
        GPIO_SetValue(PINSEL_PORT_0, OUTPUT_PIN);
    }
}

void TIMER1_IRQHandler(void) {
    if (TIM_GetIntStatus(LPC_TIM1, TIM_MR0_INT)) {
        GPIO_ClearValue(PINSEL_PORT_0, OUTPUT_PIN);  // Set output to 0
        TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT);
    } else {
        GPIO_SetValue(PINSEL_PORT_0, OUTPUT_PIN);  // Set output to 1
        TIM_ClearIntPending(LPC_TIM1, TIM_MR1_INT);
    }
}
```
</details>

## Second Problem

Se tienen tres bloques de datos de 4KBytes de longitud cada uno en el cual se han guardado tres formas de onda. Cada muestra de la onda es un valor de 32 bits que se ha capturado desde el ADC. Las direcciones de inicio de cada bloque son representadas por macros del estilo DIRECCION_BLOQUE_N, con N=0,1,2.

Se pide que, usando DMA y DAC se genere una forma de onda por la salida analógica de la LPC1769.

La forma de onda cambiará en función de una interrupción externa conectada a la placa de la siguiente manera:

- 1er interrupción: Forma de onda almacenada en bloque 0, con frecuencia de señal de 60[KHz].
- 2da interrupción: Forma de onda almacenada en bloque 1 con frecuencia de señal de 120[KHz].
- 3ra interrupción: Forma de onda almacenada en bloque 0 y bloque 2 (una a continuación de la otra) con frecuencia de señal de 450[KHz].
- 4ta interrupción: Vuelve a comenzar con la forma de onda del bloque 0.

En cada caso se debe utilizar el menor consumo de energía posible del DAC.

<details><summary>Summary</summary>

The task requires generating waveforms using data stored in three memory blocks and outputting them via the DAC. DMA handles data transfers, and external interrupts control waveform selection and frequency changes. The implementation ensures minimal power consumption by using the DAC's low-power mode and efficient memory-to-peripheral data management. The requirements are:

1. Use the DAC in low-power mode to minimize power consumption.
2. Coordinate the DMA and DAC for efficient memory-to-peripheral transfers.
3. Handle external interrupts to switch blocks and waveform frequencies dynamically.
4. Ensure proper scaling of 32-bit samples to the 10-bit DAC output resolution.
5. Manage the transition between blocks without CPU intervention using DMA.

</details>

<details><summary>Solution</summary>

### **Warning**: this solution could have mistakes. Contributions are welcome! Please open an issue or submit a pull request if you have suggestions or improvements.

Missing improvements:

- Coordinate DMA with DAC for generating output signal
- Show contents of block 1 and 3 on the third interruption
- Ensure proper scaling of 32-bit samples to the 10-bit DAC output resolution.

```c
/**
* @file r2-2023-ex2.c
* @brief Solution for the Second Problem of the Retake for the Second 2023 Exam from Digital Electronics 3
* @author Ignacio Ledesma
* @license MIT
* @date 2024-11
*/

#include "LPC17xx.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_pinsel.h"

// Memory Block Addresses
#define BLOCK_0_ADDR 0x10000000  // Block 0
#define BLOCK_1_ADDR 0x10001000  // Block 1
#define BLOCK_2_ADDR 0x10002000  // Block 2

// DAC and DMA Configuration
#define SAMPLES_PER_BLOCK 1024
#define DAC_LOW_POWER 1
#define DAC_CLK_HZ 25000000  // DAC clock frequency (25 MHz)

// Waveform Frequencies
#define FREQ_60KHZ ((DAC_CLK_HZ) / (SAMPLES_PER_BLOCK * 60000))
#define FREQ_120KHZ ((DAC_CLK_HZ) / (SAMPLES_PER_BLOCK * 120000))
#define FREQ_450KHZ ((DAC_CLK_HZ) / (SAMPLES_PER_BLOCK * 450000))

// GPIO Pins
#define DAC_PIN ((uint32_t)(1 << 26))  // P0.26 DAC output
#define EXT_INT_PIN ((uint32_t)(1 << 10))  // P2.10 External interrupt (EINT0)

// Global Variables
volatile uint8_t current_waveform = 0;

// Function Prototypes
void configureDAC(void);
void configureDMA(uint32_t block_addr, uint32_t frequency);
void configureEXTI(void);
void configurePins(void);
void EINT0_IRQHandler(void);

int main(void) {
    SystemInit();

    // Configure peripherals
    configurePins();
    configureDAC();
    configureEXTI();

    // Start with block 0 and frequency 60 kHz
    configureDMA(BLOCK_0_ADDR, FREQ_60KHZ);

    while (1) {
        // System managed via interrupts
        __WFI();  // Wait for interrupt
    }
    return 0;
}

void configureDAC(void) {
    DAC_CONVERTER_CFG_Type dacConfig;

    dacConfig.CNT_ENA = ENABLE;  // Enable timeout mode
    dacConfig.DMA_ENA = ENABLE;  // Enable DMA
    DAC_ConfigDAConverterControl(LPC_DAC, &dacConfig);
    DAC_SetBias(LPC_DAC, DAC_LOW_POWER);  // Low-power mode
    DAC_Init(LPC_DAC);
}

void configureDMA(uint32_t block_addr, uint32_t frequency) {
    GPDMA_Channel_CFG_Type dmaConfig;

    // Stop current DMA transfer
    GPDMA_ChannelCmd(0, DISABLE);

    // Configure DMA channel
    dmaConfig.ChannelNum = 0;
    dmaConfig.SrcMemAddr = block_addr;  // Source: memory block
    dmaConfig.DstMemAddr = (uint32_t)&LPC_DAC->DACR;  // Destination: DAC
    dmaConfig.TransferSize = SAMPLES_PER_BLOCK;
    dmaConfig.TransferWidth = GPDMA_WIDTH_WORD;  // 32-bit transfer
    dmaConfig.TransferType = GPDMA_TRANSFERTYPE_M2P;  // Memory-to-peripheral
    dmaConfig.SrcConn = 0;  // Not applicable for memory
    dmaConfig.DstConn = GPDMA_CONN_DAC;
    dmaConfig.DMALLI = 0;  // No linked list

    GPDMA_Setup(&dmaConfig);
    DAC_SetDMATimeOut(LPC_DAC, frequency);  // Set timeout for the specified frequency
    GPDMA_ChannelCmd(0, ENABLE);
}

void configureEXTI(void) {
    EXTI_InitTypeDef extiConfig;

    extiConfig.EXTI_Line = EXTI_EINT0;
    extiConfig.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
    extiConfig.EXTI_polarity = EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;

    EXTI_Config(&extiConfig);
    EXTI_ClearEXTIFlag(EXTI_EINT0);
    NVIC_EnableIRQ(EINT0_IRQn);  // Enable in NVIC
}

void configurePins(void) {
    PINSEL_CFG_Type pinConfig;

    // Configure DAC output pin
    pinConfig.Portnum = PINSEL_PORT_0;
    pinConfig.Pinnum = 26;  // P0.26 DAC output
    pinConfig.Funcnum = PINSEL_FUNC_2;  // DAC function
    pinConfig.Pinmode = PINSEL_PINMODE_PULLUP;
    pinConfig.OpenDrain = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&pinConfig);

    // Configure external interrupt pin
    pinConfig.Portnum = PINSEL_PORT_2;
    pinConfig.Pinnum = 10;  // P2.10 External interrupt
    pinConfig.Funcnum = PINSEL_FUNC_1;  // EINT0 function
    pinConfig.Pinmode = PINSEL_PINMODE_PULLUP;
    pinConfig.OpenDrain = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&pinConfig);
}

void EINT0_IRQHandler(void) {
    // Update the current waveform
    current_waveform++;
    if (current_waveform > 3) current_waveform = 1;

    // Configure DMA for the selected waveform
    switch (current_waveform) {
        case 1:
            configureDMA(BLOCK_0_ADDR, FREQ_60KHZ);
            break;
        case 2:
            configureDMA(BLOCK_1_ADDR, FREQ_120KHZ);
            break;
        case 3:
            configureDMA(BLOCK_2_ADDR, FREQ_450KHZ);
            break;
    }

    EXTI_ClearEXTIFlag(EXTI_EINT0);  // Clear the interrupt flag
}
```
</details>

## Third Problem

Describa las formas de disparo o inicio de conversión del ADC del microcontrolador LPC1769.

<details><summary>Solution</summary>

## ADC Conversion Trigger Modes in LPC1769

The **LPC1769** microcontroller offers multiple ways to trigger the start of an Analog-to-Digital Conversion (ADC). The conversion can be triggered manually (via software) or automatically (via hardware) using different sources such as timers or external signals.

### 1. **Software Trigger (Manual Start)**
The conversion can be initiated manually through software using the ADC Control Register (**ADCR**).

- **How to trigger a conversion:**
   - Write to the **START[26:24]** field in the **ADCR** register.
   - To start the conversion immediately, set the **START[26:24]** field to `001`.

**Example using CMSIS:**
```c
ADC_StartCmd(LPC_ADC, ADC_START_NOW); // Trigger ADC conversion immediately
```

### 2. **External Event Triggers**
The ADC can be configured to start conversion automatically using external events, such as timer match signals or external interrupts.

#### **a. Timer Match Signals**
- ADC conversion can be triggered when a match event occurs in Timer0, Timer1, Timer2, or Timer3.
- Useful for synchronizing ADC conversions with periodic signals.

**START Field Values for Timer Triggers:**
- `100`: Start ADC on match event MAT0.1.
- `101`: Start ADC on match event MAT0.3.
- `110`: Start ADC on match event MAT1.0.
- `111`: Start ADC on match event MAT1.1.

#### **b. External Interrupts**
- ADC conversion can start upon detecting an external interrupt, such as **EINT0**.
- To configure this, set **START[26:24]** in the **ADCR** register to `010`.

### 3. **Continuous Conversion (Burst Mode)**
The ADC can perform continuous conversions using **burst mode**. This mode enables the ADC to continuously convert selected channels without additional software or hardware triggers.

- **How to enable burst mode:**
   - Set the **BURST** bit in the **ADCR** register.
   - This mode is ideal for applications requiring rapid, continuous data acquisition.

### ADC Control Register (ADCR)
The **ADCR** register contains key fields to configure and trigger ADC conversions:
- **START[26:24]**: Defines the conversion trigger source:
  - `000`: No start.
  - `001`: Software-triggered conversion.
  - `010`: Triggered by external interrupt (EINT0).
  - `011`: Triggered by capture event CAP0.1.
  - `100`: Timer match MAT0.1.
  - `101`: Timer match MAT0.3.
  - `110`: Timer match MAT1.0.
  - `111`: Timer match MAT1.1.
- **BURST**: Enables continuous conversion mode.

### Summary of Trigger Modes
1. **Software Trigger:** Manual control by writing to the **ADCR** register.
2. **Timer Match Trigger:** Automatic start upon timer match events (e.g., MAT0.1, MAT1.0).
3. **External Interrupt Trigger:** Automatic start using external interrupts (e.g., **EINT0**).
4. **Burst Mode:** Continuous ADC conversions without external triggers.

These options provide flexibility to adapt the ADC configuration to various application requirements, whether controlled manually or synchronized with hardware events.

</details>
