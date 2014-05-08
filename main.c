#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "uart.h"
#include "config.h"
#include "astring.h"

static void initRegisters(void) {
    /* Setup Leds as outputs. */
    LEDDDR |= LEDDDRBITS;

    /* Set appropriate outputs to motors. */
    M1_DDR |= M1_DDRBITS;
    M2_DDR |= M2_DDRBITS;
}

static void initPwm(void) {
    /* Setups the timers for PWM.
     * The different PWMs are enabled through:
     *  TCCR0A |= (1<<COM0A1) for OC0A (M1_IN1)
     *  TCCR0A |= (1<<COM0B1) for OC0B (M1_IN2)
     *  TCCR1A |= (1<<COM1A1) for OC1A (M2_IN1)
     *  TCCR1A |= (1<<COM1B1) for OC1B (M2_IN2)
     *
     * Duty cycle is adjusted by setting:
     *  OCR0A for OC0A (M1_IN1)
     *  OCR0B for OC0B (M1_IN2)
     *  OCR1A for OC1A (M2_IN1)
     *  OCR1B for OC1B (M2_IN2)
     *
     * Note that the default duty cycle is 0%. */

    /* Init timer 0 assoicated with OC0A/B. */
    /* Mode 1 - Phase Correct PWM. */
    TCCR0A |= (1<<WGM00);
    /* Prescaler selection.
     * Our frequency is 20MHz.
     * Frequency of PWM should be ~10kHz.
     * Thus, a prescaler of 1024 will have to do => CS00=CS02=1. Ref p. 107 */
    TCCR0B |= ((1<<CS02) | (1<<CS00));

    /* Init Timer 1 associated with OC1A/B.
     * Same settings apply, even if timer 1 is a 16-bit timer.
     * Mode 1 - PWM, Phase Correct, 8-bit. */
    TCCR1A |= (1<<WGM10);
    /* Analogous to the timer 0 case, we set a prescaler of 1024.
     * Thus, CS10=CS12=1. */
    TCCR1B |= ((1<<CS10) | (1<<CS12));
    
    /* Set PWM ports as outputs. */
    M1_PWMDDR |= M1_PWMDDRBITS;
    M2_PWMDDR |= M2_PWMDDRBITS;
}    

static void initAdc(void) {
    /* Setups the ADC for use. 
     * Sets the ADC in Free Running Mode and enables ues of interrupts. */

    /* Select the 2.56V internal reference. */
    ADMUX |= ((1<<REFS1) | (1<<REFS0));

    /* Using the largest ADC prescaler (/128) since time is not an issue. */
    ADCSRA |= ((1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0));
 
    /* Enable the ADC and Interrupt Enable. */
    ADCSRA |= (1<<ADEN) | (1<<ADIE);
    
    /* Disable the digital part of the feedback pins. */
    DIDR0 |= (M1_FEEDBACK | M2_FEEDBACK);
    
    /* Start with Motor 1 Feedback for first conversion. */
    ADMUX |= M1_FEEDBACKADC;
    
    /* Start conversion. */
    ADCSRA |= (1<<ADSC);
}
static uint16_t getADCVal(void) {
    /* Function to fetch the value of the ADC register.*/

    uint8_t lsb = ADCL; /* Read ADC LSB. */
    uint8_t msb = ADCH;
    return (((uint16_t)(msb)<<8) | lsb);
}

uint8_t curAdc = M1_FEEDBACKADC;
uint16_t lastAdcValM1;
uint16_t lastAdcValM2;
ISR(ADC_vect) {
    if ((ADMUX & 0x1F) == M1_FEEDBACKADC) {
        lastAdcValM1 = getADCVal();
        curAdc = M2_FEEDBACKADC; /* Set M2 ADC for next conversion. */
    } else if ((ADMUX & 0x1F) == M2_FEEDBACKADC) {
        lastAdcValM2 = getADCVal();
        curAdc = M1_FEEDBACKADC; /* Set M1 ADC for next conversion. */
    }
    ADMUX &= ~0x1F; /* Set MUX4..0 to zero. */
    ADMUX |= (0x1F & curAdc); /* Set M2 ADC for next conversion. */
    ADCSRA |= (1<<ADSC); /* Start conversion. */
    LEDREG ^= LED2;

}

static void setSpeedM1(int8_t speed) {
    /* Function to set the speed and direction of M1.
     * Positive speed => Forward.
     * Negative speed => Reverse. 
     *
     * Note that the function maps the value ranges
     * 1:1:127 => 2:2:254
     * -1:1:-128 => 1:2:255
     * due to how the int8_t is represented. */

    if (speed > 0) {
        /* Forward.
         * Set M1_IN1 to prefered duty cycle.
         * Set M1_IN2 to 0. */
        M1_IN1_DC = (speed<<1); /* Limit values 2:2:254*/
        M1_IN2_DC = 0x00;
    } else if (speed < 0) {
        /* Reverse.
         * Set M1_IN1 to 0.
         * Set M1_IN2 to prefered duty cycle. */
        M1_IN1_DC = 0x00;
        M1_IN2_DC = (((-speed)<<1)-1); /* Limit values 1:2:255 */
    } else {
        /* We're either at a speed of zero or out of bounds.
         * Set M1_IN1 and M1_IN2 to zero. */
        M1_IN1_DC = 0x00;
        M1_IN2_DC = 0x00;
    }
}

static void setSpeedM2(int8_t speed) {
    /* Function to set the speed and direction of M2 (rotated motor).
     * Positive speed => Forward.
     * Negative speed => Reverse. 
     *
     * Note that the function maps the value ranges
     * 1:1:127 => 2:2:254
     * -1:1:-128 => 1:2:255
     * due to how the int8_t is represented. */

    if (speed < 0) {
        /* Forward.
         * Set M2_IN1 to prefered duty cycle.
         * Set M2_IN2 to 0. */
        M2_IN1_DC = (((-speed)<<1)-1); /* Limit values 1:2:255 */
        M2_IN2_DC = 0x00;
    } else if (speed > 0) {
        /* Reverse.
         * Set M2_IN1 to 0.
         * Set M2_IN2 to prefered duty cycle. */
        M2_IN2_DC = (speed<<1); /* Limit values 2:2:254*/
        M2_IN1_DC = 0x00;
    } else {
        /* We're either at a speed of zero or out of bounds.
         * Set M2_IN1 and M2_IN2 to zero. */
        M2_IN1_DC = 0x00;
        M2_IN2_DC = 0x00;
    }
}

static void setEnableM1(uint8_t state) {
    /* This function controls the enable of the H-bridge.
     * If enable is set to zero, the device will enter sleep mode.
     * The function also controls weather the PWM is active or not. */
    if (state == 0) {
        M1_REG &= ~(M1_ENABLE);
        TCCR0A &= ~((1<<COM0A1) | (1<<COM0B1)); /* Disable OC0A,OC0B. */
    } else {
        M1_REG |= M1_ENABLE;
        TCCR0A |= ((1<<COM0A1) | (1<<COM0B1)); /* Enable OC0A, OC0B. */
    }
}

static void setEnableM2(uint8_t state) {
    /* This function controls the enable of the H-bridge.
     * If enable is set to zero, the device will enter sleep mode.
     * The function also controls weather the PWM is active or not. */
    if (state == 0) {
        M2_REG &= ~(M2_ENABLE);
        TCCR1A &= ~((1<<COM1A1) | (1<<COM1B1)); /* Disable OC1A,OC1B. */
    } else {
        M2_REG |= M2_ENABLE;
        TCCR1A |= ((1<<COM1A1) | (1<<COM1B1)); /* Enable OC1A, OC1B. */
    }
}

static void setDisableM1(uint8_t state) {
    /* Controls the state of the disable pin.
     * Setting the disable high will disable the H-bridge and 
     * put the outputs in Hi-Z mode. */
    if (state == 0) {
        M1_REG &= ~(M1_DISABLE);
    } else {
        M1_REG |= M1_DISABLE;
    }
}

static void setDisableM2(uint8_t state) {
    /* Controls the state of the disable pin.
     * Setting the disable high will disable the H-bridge and 
     * put the outputs in Hi-Z mode. */
    if (state == 0) {
        M2_REG &= ~(M2_DISABLE);
    } else {
        M2_REG |= M2_DISABLE;
    }
}

uint8_t uartWorker(void) {
    uint16_t character = uart_getc();
    if (character & UART_NO_DATA) {
        /* Nothing to see here. */
        return 0;
    } else {
        /* Data! */
        uart_putc((uint8_t)character); /* Loopback */
        if((uint8_t)character == '\r') uart_putc('\n'); /* New line if return. */
        return (uint8_t)character;
    }
}

static char *cmdList[] = {
    "set",
    "get",
    '\0'
};

static char *cmdPropList[] = {
    "m1speed",
    "m2speed",
    "m1disable",
    "m2disable",
    "led1",
    "led2",
    "led3",
    "led4",
    "m1current",
    "m2current",
    '\0'
};

void cmdParser(uint8_t *bufPtr) {
    uint8_t *strPtr = bufPtr;

    uint8_t len = getEndOfPart(strPtr);
    char result = compareStrs(strPtr, cmdList, len, 1);
    strPtr += len;
    switch(result) {
        case 1: cmdSet(strPtr); break;
        case 2: cmdGet(strPtr); break;
        default: uart_puts_P("Invalid command\r\n");
    }
}

void cmdSet(uint8_t *bufPtr) {
    uint8_t *strPtr = bufPtr;
    int8_t value;

    /* Make sure another parameter is coming. */
    if(strPtr[0] != 0x20) { uart_puts_P("Error: Set requires at least 2 parameters.\r\n"); return; }
    strPtr++; /* Jump across the space. */
    
    /* Get the next parameter / property. */
    uint8_t len = getEndOfPart(strPtr);
    uint8_t result = (uint8_t)compareStrs(strPtr, cmdPropList, len, 1);
    //strPtr++; /* Jump across the space to the value. */
    strPtr += len;
    if(strPtr[0] != 0x20) { uart_puts_P("Error: Set requires at least 2 parameters.\r\n"); return; }
    strPtr++; /* Jump across the space. */

    len = getEndOfPart(strPtr);
    if(strPtr[0] == 0x2D) { /* ASCII 2D = - (dash). */
        /* Let's handle negative values.
         * This would be better to handle in astring.c. */
        strPtr++; /* Jump the pesky dash. */
        value = getInteger(strPtr, len-1);
        if(value == -1) { uart_puts_P("Error: Expected ninteger.\r\n"); return;}
        value = -value;
    } else {
        value = getInteger(strPtr, len);
        if(value == -1) { uart_puts_P("Error: Expected integer.\r\n"); return;}
    }

    setProperty(result, value);
}

void cmdGet(uint8_t *bufPtr) {
    /* Command to fetch values of various properties.
     * Implement actual procedures to get values.*/
    uint8_t *strPtr = bufPtr;
    
    /* Make sure another parameter is coming. */
    if(strPtr[0] != 0x20) { uart_puts_P("Error: Get requires a property to fetch.\r\n"); return; }
    strPtr++; /* Jump space. */
    
    uint8_t len = getEndOfPart(strPtr);
    uint8_t result = (uint8_t)compareStrs(strPtr, cmdPropList, len, 1);
    
    switch(result) {
        case 1: 
            /* m1speed */
            uartPutHex(M1_IN1_DC | M1_IN2_DC);
            break;
        case 2: 
            /* m2speed */
            uartPutHex(M2_IN1_DC | M2_IN2_DC);
            break;
        case 3: 
            /* m1disable */
            uartPutHex(0x1&(M1_PIN>>PA1));
            break;
        case 4: 
            /* m2disable */
            uartPutHex(0x1&(M2_PIN>>PA5));
            break;
        case 5:
            /* led1 */
            uart_puts_P("Error: Not Implemented.\r\n"); /* TODO */
            break;
        case 6: 
            /* led2 */
            uart_puts_P("Error: Not Implemented.\r\n"); /* TODO */
            break;
        case 7: 
            /* led3 */
            uart_puts_P("Error: Not Implemented.\r\n"); /* TODO */
            break;
        case 8: 
            /* led4 */
            uart_puts_P("Error: Not Implemented.\r\n"); /* TODO */
            break;
        case 9:
            /* m1current */
            uartPutHex(lastAdcValM1);
            break;
        case 10:
            /* m2current */
            uartPutHex(lastAdcValM2);
            break;
        default:
            /* Invalid command. */
            uart_puts_P("Error: Invalid property.\r\n");
    }
}

void setProperty(uint8_t propIndex, int8_t value) {
    switch(propIndex) {
        case 1: 
            /* m1speed */
            setSpeedM1(value);
            break;
        case 2: 
            /* m2speed */
            setSpeedM2(value);
            break;
        case 3: 
            /* m1disable */
            setDisableM1(value);
            break;
        case 4: 
            /* m2disable */
            setDisableM2(value);
            break;
        case 5:
            /* led1 */
            uart_puts_P("Error: Not Implemented.\r\n"); /* TODO */
            break;
        case 6: 
            /* led2 */
            uart_puts_P("Error: Not Implemented.\r\n"); /* TODO */
            break;
        case 7: 
            /* led3 */
            uart_puts_P("Error: Not Implemented.\r\n"); /* TODO */
            break;
        case 8: 
            /* led4 */
            uart_puts_P("Error: Not Implemented.\r\n"); /* TODO */
            break;
        case 9:
            /* m1current */
            uart_puts_P("Error: Non-valid Action.\r\n");
            break;
        case 10:
            /* m2current */
            uart_puts_P("Error: Non-valid Action.\r\n");
            break;
        default: 
            /* Invalid command. */
            uart_puts_P("Error: Invalid property.\r\n");
    }
}

void uartPutHex(uint16_t num) {
    uint8_t buf[4];
    sprintf(buf, "%X", num);
    uart_puts_P("0x");
    uart_puts(buf);
    uart_puts_P("\r\n");
}


int main(void)
{
    initRegisters();
    initPwm();
    uart_init((UART_BAUD_SELECT((SERIAL_BAUDRATE), F_CPU)));
    sei();
    initAdc();
    uart_puts_P("Welcome to the Robot of Awesome Controller terminal\r\n");
    uart_puts_P("# ");
    LEDREG |= LED1;
    uint8_t character;
    
    setEnableM1(1);
    setEnableM2(1);
    uint8_t buffer[127];
    uint8_t bufLength = 128;
    uint8_t bufHead = 0;
    while(1)
    {
        character = uartWorker();
        if((character == '\n') || (character == '\r')) { 
            /* Set null terminator for detection of end of string. */
            buffer[bufHead] = '\0';
            /* Go to beginning of buffer. */
            bufHead = 0;
            /* Process command. */
            cmdParser(buffer);
            uart_puts_P("# ");
            /*motorOn += 1;
            LEDREG ^= LED2;
            setSpeedM1(motorOn);*/
        } else if ((bufHead < bufLength) && ((character > 0x1F) && (character < 0x7F))){
            /* Buffer is not full and character is valid.
             * Put character in buffer. */
            buffer[bufHead] = character;
            bufHead++;
        }

    }
}
