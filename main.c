#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "uart.h"
#include "config.h"
#include "astring.h"
#include "motor.h"

static void initRegisters(void) {
    /* Setup Leds as outputs. */
    LEDDDR |= LEDDDRBITS;

    /* Set appropriate outputs to motors. */
    M1_DDR |= M1_DDRBITS;
    M2_DDR |= M2_DDRBITS;
}

static void initAdc(void) {
    /* Setups the ADC for use. 
     * Setups the ADC and enables use of interrupts. */

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
