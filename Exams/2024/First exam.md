# Exam 2024 - First

## Modules

- LPC17xx Peripheral Drivers (GPIO, EXTI, Systick)
- Embedded System Programming (Interrupt Handling and Timing)

## First Problem

Un estacionamiento automatizado utiliza una barrera que se abre y cierra en función de la validación de un ticket de acceso utilizando una LPC1769 Rev. D trabajando a una frecuencia de CCLK a 70 [MHz]. Cuando el sistema detecta que un automóvil se ha posicionado frente a la barrera, se debe activar un sensor conectado al pin P2[4] mediante una interrupción externa (EINT). Una vez validado el ticket, el sistema activa un motor que abre la barrera usando el pin P0[15]. El motor  debe  estar  activado  por  X  segundos  y  luego  apagarse,  utilizando  el temporizador Systick para contar el tiempo. Si el ticket es inválido, se encenderá un LED rojo conectado al pin P1[5]. Para gestionar el tiempo de apertura de la barrera, existe un switch conectado al pin P3[4] que dispone de una ventana de configuración de 3 segundos gestionada por el temporizador Systick. Durante dicha ventana, se debe contar cuantas veces se presiona el switch y en función de dicha cantidad, establecer el tiempo de la barrera.

<details><summary>Summary</summary>

Design a system to control the barrier of an automated parking lot using the LPC1769. The requirements are:

    1. Use a sensor connected to pin P2.3 to detect when a car arrives or leaves.
        - Rising edge (car arrival): Turn on a ticket validator connected to P2.4.
            - If the ticket is valid, activate the barrier motor on P0.15 for a configurable amount of time.
            - If the ticket is invalid, turn on a red LED connected to P1.5.
        - Falling edge (car leaves): Turn off the ticket validator and LED. If the barrier was up, lower it after the configured time.
    2. The time the barrier remains open is configurable using a switch on P3.4.
        - The configuration is active for 3 seconds at startup, during which the switch is pressed multiple times to set the time.
        - Time options: 2s, 4s, 6s, or 8s.
    3. Use SysTick to handle timing.

Considering that:

    1. The microcontroller is working at a clock frequency of 70 MHz
    2. Code should be commented
    3. Calculations should be justified and written somewhere
    4. Apply engineering criteria if necessary and suitable (with its corresponding justification)

</details>

<details><summary>Solution</summary>

```c
/**
* @file e1-2024-ex1.c
* @brief Solution for the First Problem of the First 2024 Exam from Digital Electronics 3
* @author Ignacio Ledesma
* @license MIT
* @date 2024-11
*/

#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_systick.h"

#define SYSTICK_TIME_MS 100
#define SYSTICK_COUNTS_FOR_1SEC 10

#define INPUT 0
#define OUTPUT 1

#define RISING_EDGE 0
#define FALLING_EDGE 1

#define CONFIG_MODE 0
#define VALIDATION_MODE 1
#define VALIDATION_RETRY_SEC 1
#define OPEN_MODE 2
#define WAITING_MODE 3

// Pin definitions
#define CONFIG_SWITCH_PIN ((uint32_t)(1 << 10)) // P2.10: Config switch
#define TICKET_VALIDATOR_OUTPUT_PIN ((uint32_t)(1 << 5)) // P2.5: Ticket validator output
#define TICKET_VALIDATOR_VCC_PIN ((uint32_t)(1 << 4)) // P2.4: Ticket validator power
#define CAR_SENSOR_PIN ((uint32_t)(1 << 3)) // P2.3: Car sensor
#define INVALID_TICKET_LED_PIN ((uint32_t)(1 << 5)) // P1.5: Invalid ticket LED
#define BARRIER_MOTOR_PIN ((uint32_t)(1 << 15)) // P0.15: Barrier motor

uint16_t const SECOND = SYSTICK_COUNTS_FOR_1SEC;
uint32_t systick_counter = 3 * SECOND; // Initial configuration time: 3 seconds
uint8_t gate_timer_config = 0; // Configured barrier open time
uint8_t system_mode = CONFIG_MODE; // Initial system mode

// Function prototypes
void config_pins(void);
void config_exti(void);
void config_systick(void);
void EINT0_IRQHandler(void);
void try_validation(void);
void SysTick_Handler(void);
void EINT3_IRQHandler(void);

int main(void) {
    SystemInit();
    config_pins();
    config_exti();
    config_systick();

    while (1) {
        __WFI(); // Wait for interrupt
    }

    return 0;
}

void config_pins(void) {
    PINSEL_CFG_Type pin;

    // Config switch (P2.10)
    pin.Portnum = PINSEL_PORT_2;
    pin.Pinnum = PINSEL_PIN_10;
    pin.Funcnum = PINSEL_FUNC_1; // EXTI
    pin.Pinmode = PINSEL_PINMODE_PULLDOWN;
    pin.OpenDrain = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&pin);

    // Ticket validator pins (P2.4, P2.5)
    pin.Funcnum = PINSEL_FUNC_0; // GPIO
    pin.Pinnum = PINSEL_PIN_5;
    PINSEL_ConfigPin(&pin);
    pin.Pinnum = PINSEL_PIN_4;
    PINSEL_ConfigPin(&pin);

    // Car sensor pin (P2.3)
    pin.Pinnum = PINSEL_PIN_3;
    PINSEL_ConfigPin(&pin);

    // Invalid ticket LED (P1.5)
    pin.Portnum = PINSEL_PORT_1;
    pin.Pinnum = PINSEL_PIN_5;
    PINSEL_ConfigPin(&pin);

    // Barrier motor (P0.15)
    pin.Portnum = PINSEL_PORT_0;
    pin.Pinnum = PINSEL_PIN_15;
    PINSEL_ConfigPin(&pin);

    // Set pin directions
    GPIO_SetDir(PINSEL_PORT_2, TICKET_VALIDATOR_OUTPUT_PIN | CAR_SENSOR_PIN, INPUT);
    GPIO_SetDir(PINSEL_PORT_2, TICKET_VALIDATOR_VCC_PIN, OUTPUT);
    GPIO_SetDir(PINSEL_PORT_1, INVALID_TICKET_LED_PIN, OUTPUT);
    GPIO_SetDir(PINSEL_PORT_0, BARRIER_MOTOR_PIN, OUTPUT);

    // Enable GPIO interrupts for car sensor (P2.3)
    GPIO_IntCmd(PINSEL_PORT_2, CAR_SENSOR_PIN, FALLING_EDGE);
    GPIO_IntCmd(PINSEL_PORT_2, CAR_SENSOR_PIN, RISING_EDGE);

    NVIC_SetPriority(EINT3_IRQn, 0);
    NVIC_DisableIRQ(EINT3_IRQn); // Disable car sensor interrupts until configuration is done
}

void config_exti(void) {
    EXTI_InitTypeDef exti;

    exti.EXTI_Line = EXTI_EINT0; // Config switch (P2.10)
    exti.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
    exti.EXTI_polarity = EXTI_POLARITY_HIGH_ACTIVE_OR_RISING_EDGE;

    EXTI_Config(&exti);

    NVIC_SetPriority(EINT0_IRQn, 1);
    NVIC_EnableIRQ(EINT0_IRQn); // Enable for initial configuration
    EXTI_Init();
}

void config_systick(void) {
    SYSTICK_InternalInit(SYSTICK_TIME_MS);
    SYSTICK_IntCmd(ENABLE);
    SYSTICK_Cmd(ENABLE);
}

// Interrupt handler for config switch (P2.10)
void EINT0_IRQHandler(void) {
    gate_timer_config++;
    if (gate_timer_config == 4) {
        gate_timer_config = 0;
    }
    EXTI_ClearEXTIFlag(EXTI_EINT0);
}

// Try validating the ticket
void try_validation(void) {
    if (GPIO_ReadValue(PINSEL_PORT_2) & TICKET_VALIDATOR_OUTPUT_PIN) {
        system_mode = OPEN_MODE;
        switch (gate_timer_config) {
            case 0: systick_counter = 2 * SECOND; break;
            case 1: systick_counter = 4 * SECOND; break;
            case 2: systick_counter = 6 * SECOND; break;
            case 3: systick_counter = 8 * SECOND; break;
        }
        GPIO_SetValue(PINSEL_PORT_0, BARRIER_MOTOR_PIN); // Open the barrier
        GPIO_ClearValue(PINSEL_PORT_1, INVALID_TICKET_LED_PIN); // Turn off the LED
    } else {
        GPIO_SetValue(PINSEL_PORT_1, INVALID_TICKET_LED_PIN); // Turn on the LED
    }
}

// SysTick interrupt handler
void SysTick_Handler(void) {
    systick_counter--;
    if (systick_counter == 0) {
        switch (system_mode) {
            case CONFIG_MODE:
                NVIC_DisableIRQ(EINT0_IRQn);
                NVIC_EnableIRQ(EINT3_IRQn);
                SYSTICK_IntCmd(DISABLE);
                system_mode = WAITING_MODE;
                break;
            case VALIDATION_MODE:
                try_validation();
                systick_counter = VALIDATION_RETRY_SEC * SECOND;
                break;
            case OPEN_MODE:
                GPIO_ClearValue(PINSEL_PORT_0, BARRIER_MOTOR_PIN); // Close the barrier
                SYSTICK_IntCmd(DISABLE);
                break;
        }
    }
    SYSTICK_ClearCounterFlag();
}

// Car sensor interrupt handler (P2.3)
void EINT3_IRQHandler(void) {
    if (GPIO_GetIntStatus(PINSEL_PORT_2, CAR_SENSOR_PIN, RISING_EDGE)) {
        GPIO_SetValue(PINSEL_PORT_2, TICKET_VALIDATOR_VCC_PIN); // Activate ticket validator
        system_mode = VALIDATION_MODE;
        systick_counter = VALIDATION_RETRY_SEC * SECOND;
        SYSTICK_IntCmd(ENABLE);
    }
    if (GPIO_GetIntStatus(PINSEL_PORT_2, CAR_SENSOR_PIN, FALLING_EDGE)) {
        if (system_mode == OPEN_MODE) {
            SYSTICK_IntCmd(ENABLE);
        }
        GPIO_ClearValue(PINSEL_PORT_2, TICKET_VALIDATOR_VCC_PIN); // Deactivate ticket validator
        GPIO_ClearValue(PINSEL_PORT_1, INVALID_TICKET_LED_PIN); // Turn off the LED
        system_mode = WAITING_MODE;
    }
    GPIO_ClearInt(PINSEL_PORT_2, CAR_SENSOR_PIN);
    EXTI_ClearEXTIFlag(EXTI_EINT3);
}
```
</details>

## Second Problem

En  una  fábrica,  hay  un  sistema  de  alarma  utilizando  una  LPC1769  Rev. D trabajando a una frecuencia de CCLK a 100 [MHz], conectado a un sensor de puerta que se activa cuando la puerta se abre. El sensor está conectado al pin P0[6], el cual genera una interrupción externa (EINT) cuando se detecta una apertura (cambio de estado). Al detectar que la puerta se ha abierto, el sistema debe iniciar un temporizador utilizando el Systick para contar un período de 30 segundos. Durante estos 30 segundos, el usuario deberá introducir un código de desactivación mediante un DIP switch de 4 entradas conectado a los pines P2[0] - P2[3]. El código correcto es 0xAA (1010 en binario). El usuario tiene dos intentos para introducir el código correcto. Si después de dos intentos el código ingresado es  incorrecto,  la  alarma  se  activará,  encendiendo  un  buzzer conectado al pin P1[11]

<details><summary>Summary</summary>

Design an alarm system for a factory using the LPC1769. The system must:

    1. Monitor a door sensor connected to P2.10 (EINT0).
       - Detects when the door is opened using a rising edge.
    2. Start a 30-second countdown using SysTick when the door opens.
    3. Allow the user to input a 4-bit deactivation code using DIP switches connected to P2.0–P2.3.
       - The correct code is 0xAA (binary: 1010).
       - The user presses a button on P2.4 (GPIO Interruption) to confirm the input.
       - The user has 2 attempts to input the correct code.
    4. If the code is incorrect after 2 attempts or the timer runs out, trigger an alarm by activating a buzzer connected to P1.11.

Considering that:

    1. The microcontroller is working at a clock frequency of 100 MHz
    2. Code should be commented
    3. Calculations should be justified and written somewhere
    4. Apply engineering criteria if necessary and suitable (with its corresponding justification)

</details>

<details><summary>Solution</summary>

```c
/**
* @file e1-2024-ex2.c
* @brief Solution for the Second Problem of the First 2024 Exam from Digital Electronics 3
* @author Ignacio Ledesma
* @license MIT
* @date 2024-11
*/

#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_systick.h"

#define RISING_EDGE 0

#define INPUT 0
#define OUTPUT 1

#define SYSTICK_TIME_MS 100
#define SYSTICK_COUNTS_FOR_1SEC 10
#define ALARM_TIME_S 30
#define INCORRECT_PW_TRIES 2

#define PASSWORD 0xAA

// Pin definitions
#define DOOR_PIN ((uint32_t)(1 << 10)) // P2.10: Door sensor
#define PORT2_BIT_DIG0 0
#define PW_DIG0 ((uint32_t)(1 << 0)) // P2.0: First digit of the password
#define PW_DIG1 ((uint32_t)(1 << 1)) // P2.1: Second digit of the password
#define PW_DIG2 ((uint32_t)(1 << 2)) // P2.2: Third digit of the password
#define PW_DIG3 ((uint32_t)(1 << 3)) // P2.3: Fourth digit of the password
#define PW_BTN ((uint32_t)(1 << 4)) // P2.4: Button to confirm password
#define BUZZER_PIN ((uint32_t)(1 << 11)) // P1.11: Buzzer

uint8_t const SECOND = SYSTICK_COUNTS_FOR_1SEC; 
uint16_t systick_counter = ALARM_TIME_S * SECOND; 
uint8_t tries = INCORRECT_PW_TRIES; 

// Function prototypes
void config_pins(void);
void config_exti(void);
void config_systick(void);
void EINT0_IRQHandler(void);
void SYSTICK_IRQHandler(void);
void EINT3_IRQHandler(void);

int main(void) {
    SystemInit();
    config_pins();
    config_exti();
    config_systick();

    while (1) {
        __WFI(); // Wait for interrupt
    }
}

void config_pins(void) {
    PINSEL_CFG_Type pin;

    // Door sensor (P2.10)
    pin.Portnum = PINSEL_PORT_2;
    pin.Pinnum = PINSEL_PIN_10;
    pin.Funcnum = PINSEL_FUNC_1; // EXTI
    pin.Pinmode = PINSEL_PINMODE_PULLDOWN;
    pin.OpenDrain = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&pin); 

    // Password input pins (P2.0–P2.3) and password confirm button (P2.4)
    pin.Funcnum = PINSEL_FUNC_0; // GPIO
    for (uint32_t i = PINSEL_PIN_0; i <= PINSEL_PIN_4; i++) {
        pin.Pinnum = i;
        PINSEL_ConfigPin(&pin);
    }

    // Buzzer (P1.11)
    pin.Portnum = PINSEL_PORT_1;
    pin.Pinmode = PINSEL_PINMODE_PULLUP; // Negative logic with PNP transistor
    PINSEL_ConfigPin(&pin);

    // Set pin directions
    GPIO_SetDir(PINSEL_PORT_2, PW_DIG0 | PW_DIG1 | PW_DIG2 | PW_DIG3 | PW_BTN, INPUT);
    GPIO_SetDir(PINSEL_PORT_1, BUZZER_PIN, OUTPUT);

    // Enable GPIO interrupt for password button (P2.4)
    GPIO_IntCmd(PINSEL_PORT_2, PW_BTN, RISING_EDGE);
    NVIC_DisableIRQ(EINT3_IRQn); // Initially disable password button interrupt
    NVIC_SetPriority(EINT3_IRQn, 0);
}

void config_exti(void) {
    EXTI_InitTypeDef exti;

    // Door sensor (P2.10)
    exti.EXTI_Line = EXTI_EINT0;
    exti.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
    exti.EXTI_polarity = EXTI_POLARITY_HIGH_ACTIVE_OR_RISING_EDGE; // Rising edge

    EXTI_Config(&exti);

    NVIC_SetPriority(EINT0_IRQn, 2);
    NVIC_EnableIRQ(EINT0_IRQn); // Enable door sensor interrupt
    EXTI_Init();
}

void config_systick(void) {
    SYSTICK_InternalInit(SYSTICK_TIME_MS);
    SYSTICK_Cmd(ENABLE);
    SYSTICK_IntCmd(DISABLE); // Disabled initially
}

// Door sensor interrupt handler (P2.10)
void EINT0_IRQHandler(void) {
    tries = INCORRECT_PW_TRIES; // Reset password attempts
    systick_counter = ALARM_TIME_S * SECOND; // Start alarm countdown
    NVIC_DisableIRQ(EINT0_IRQn); // Disable door sensor interrupt
    NVIC_EnableIRQ(EINT3_IRQn); // Enable password button interrupt
    SYSTICK_IntCmd(ENABLE); // Enable SysTick
    EXTI_ClearEXTIFlag(EXTI_EINT0);
}

// SysTick interrupt handler
void SYSTICK_IRQHandler(void) {
    systick_counter--;
    if (systick_counter == 0) {
        GPIO_ClearValue(PINSEL_PORT_1, BUZZER_PIN); // Trigger alarm (0=active)
        SYSTICK_IntCmd(DISABLE); // Stop countdown
    }
    SYSTICK_ClearCounterFlag();
}

// Password button interrupt handler (P2.4)
void EINT3_IRQHandler(void) {
    tries--; // Decrease remaining attempts
    uint8_t input_password = (GPIO_ReadValue(PINSEL_PORT_2) & (PW_DIG0 | PW_DIG1 | PW_DIG2 | PW_DIG3)) >> PORT2_BIT_DIG0;

    if (input_password == PASSWORD) {
        GPIO_SetValue(PINSEL_PORT_1, BUZZER_PIN); // Deactivate alarm (1=inactive)
        NVIC_EnableIRQ(EINT0_IRQn); // Re-enable door sensor interrupt
        NVIC_DisableIRQ(EINT3_IRQn); // Disable password button interrupt
        SYSTICK_IntCmd(DISABLE); // Stop countdown
    } else if (tries == 0) {
        GPIO_ClearValue(PINSEL_PORT_1, BUZZER_PIN); // Trigger alarm (0=active)
        SYSTICK_IntCmd(DISABLE);
        tries = 1; // Prevent overflow and allow alarm deactivation
    }

    GPIO_ClearInt(PINSEL_PORT_2, PW_BTN);
    EXTI_ClearEXTIFlag(EXTI_EINT3);
}
```
</details>
