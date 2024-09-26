/*
 * @file main.c
 * @brief DMA example for LPC1769
 *
 * This example uses DMA to generate a sine wave on the DAC output.
 * The sine wave is constructed from a precomputed lookup table for 0° to 90°
 * and mirrored to create the full 360° waveform.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#ifdef __USE_MCUEXPRESSO
#include <cr_section_macros.h> /* MCUXpresso-specific macros */
#endif

#include "lpc17xx_dac.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_libcfg_default.h"
#include "lpc17xx_pinsel.h"

// Definitions
#define DMA_SIZE         60
#define NUM_SAMPLES      60 // Total number of samples for the full sine wave
#define WAVE_FREQUENCY   60 // Sine wave frequency: 60 Hz
#define CLOCK_DAC_MHZ    25 // DAC clock: 25 MHz (CCLK divided by 4)
#define TRUE             1
#define DMA_CHANNEL_ZERO 0

// Precomputed sine values from 0° to 90°, scaled to 0 - 10000 for easier computation
// These values are used to build a full 360° sine wave by symmetry
// For more information on the sine wave generation, refer to this article:
// https://community.infineon.com/gfawx74859/attachments/gfawx74859/KnowledgeBaseArticles/1863/1/Sine%20Wave%20generation.pdf
const uint32_t sine_lookup[16] = {
    0, 1045, 2079, 3090, 4067, 5000, 5877, 6691, 7431, 8090, 8660, 9135, 9510, 9781, 9945, 10000};

// DMA configuration structure
GPDMA_Channel_CFG_Type GPDMACfg;
GPDMA_LLI_Type DMA_LLI_Struct; // DMA linked list item for continuous transfer

// Function Prototypes

/**
 * @brief Configure the pin for DAC output (P0.26).
 */
void configure_pins(void);

/**
 * @brief Initialize and configure the DAC.
 */
void setup_dac(void);

/**
 * @brief Configure the DMA to transfer the waveform to the DAC.
 * @param table Pointer to the waveform table
 */
void configure_dma(uint32_t* table);

/**
 * @brief Create a full sine wave table for the DAC using 60 samples.
 * The sine wave values are computed based on the 0-90 degree values from the sine_lookup table.
 * @param table Pointer to the DAC waveform table
 * @param num_samples Number of samples in the waveform
 */
void create_waveform_table(uint32_t* table, uint32_t num_samples);

int main(void)
{
    uint32_t dac_waveform[NUM_SAMPLES]; // Buffer to store DAC waveform values

    // Configure pins, DAC, and DMA
    configure_pins();
    setup_dac();
    create_waveform_table(dac_waveform, NUM_SAMPLES); // Generate the full sine wave
    configure_dma(dac_waveform);                      // Set up DMA for continuous waveform output

    // Enable DMA channel 0 to start the waveform generation
    GPDMA_ChannelCmd(DMA_CHANNEL_ZERO, ENABLE);

    // Infinite loop, system operates using DMA
    while (TRUE)
    {
        __WFI(); // Wait for interrupt, keeping CPU in low-power mode
    }

    return 0;
}

void configure_pins(void)
{
    PINSEL_CFG_Type PinCfg;

    // Configure pin P0.26 as DAC output
    PinCfg.Funcnum = PINSEL_FUNC_2;           // DAC function
    PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL; // Disable open drain
    PinCfg.Pinmode = PINSEL_PINMODE_PULLUP;   // No pull-up or pull-down
    PinCfg.Pinnum = PINSEL_PIN_26;            // Pin number 26
    PinCfg.Portnum = PINSEL_PORT_0;           // Port number 0
    PINSEL_ConfigPin(&PinCfg);
}

void setup_dac(void)
{
    DAC_CONVERTER_CFG_Type DAC_Struct;
    uint32_t update_interval;

    // Configure DAC settings
    DAC_Struct.CNT_ENA = SET; // Enable DAC counter mode (timeout mode)
    DAC_Struct.DMA_ENA = SET; // Enable DAC DMA mode
    DAC_Init(LPC_DAC);        // Initialize the DAC

    // Calculate sample update interval for the desired waveform frequency
    update_interval = (CLOCK_DAC_MHZ * 1000000) / (WAVE_FREQUENCY * NUM_SAMPLES);
    DAC_SetDMATimeOut(LPC_DAC, update_interval); // Set the DAC timeout between samples

    // Apply the DAC configuration
    DAC_ConfigDAConverterControl(LPC_DAC, &DAC_Struct);
}

void configure_dma(uint32_t* table)
{
    // Set up the DMA linked list for continuous waveform transfer
    DMA_LLI_Struct.SrcAddr = (uint32_t)table;              // Source: DAC waveform table
    DMA_LLI_Struct.DstAddr = (uint32_t) & (LPC_DAC->DACR); // Destination: DAC register
    DMA_LLI_Struct.NextLLI = (uint32_t)&DMA_LLI_Struct;    // Point to itself for continuous transfer
    DMA_LLI_Struct.Control = DMA_SIZE | (2 << 18)          // Source width: 32-bit
                             | (2 << 21)                   // Destination width: 32-bit
                             | (1 << 26);                  // Increment source address

    // Initialize the DMA module
    GPDMA_Init();

    // Configure DMA channel for memory-to-peripheral transfer
    GPDMACfg.ChannelNum = DMA_CHANNEL_ZERO;         // Use channel 0
    GPDMACfg.SrcMemAddr = (uint32_t)table;          // Source: DAC waveform table
    GPDMACfg.DstMemAddr = 0;                        // No memory destination (peripheral)
    GPDMACfg.TransferSize = DMA_SIZE;               // Transfer size: 60 samples
    GPDMACfg.TransferWidth = 0;                     // Not used
    GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2P; // Memory-to-Peripheral transfer
    GPDMACfg.SrcConn = 0;                           // Source is memory
    GPDMACfg.DstConn = GPDMA_CONN_DAC;              // Destination: DAC connection
    GPDMACfg.DMALLI = (uint32_t)&DMA_LLI_Struct;    // Linked list for continuous transfer

    // Apply DMA configuration
    GPDMA_Setup(&GPDMACfg);
}

void create_waveform_table(uint32_t* table, uint32_t num_samples)
{
    uint32_t i;

    // Use precomputed sine values for 0° to 90°, scale to DAC range (0 - 1023)
    // We mirror the values for 90°-180° and 270°-360°, and invert them for 180°-270°.
    for (i = 0; i < num_samples; i++)
    {
        if (i <= 15) // 0° to 90°
        {
            table[i] = 512 + (512 * sine_lookup[i]) / 10000; // Scale and shift for DAC
            if (i == 15)                                     // Ensure maximum value at 90°
                table[i] = 1023;
        }
        else if (i <= 30) // 90° to 180°
        {
            table[i] = 512 + (512 * sine_lookup[30 - i]) / 10000; // Mirrored values
        }
        else if (i <= 45) // 180° to 270°
        {
            table[i] = 512 - (512 * sine_lookup[i - 30]) / 10000; // Inverted values
        }
        else // 270° to 360°
        {
            table[i] = 512 - (512 * sine_lookup[60 - i]) / 10000; // Mirrored inverted values
        }
        table[i] <<= 6; // Shift left to align with DAC register format
    }
}
