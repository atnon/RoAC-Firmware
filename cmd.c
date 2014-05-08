#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "config.h"
#include "astring.h"
#include "cmd.h"
#include "motor.h"
#include "adc.h"
#include "uart.h"

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
#if DISABLE_PWM
            uartPutHex(M1_PWM_DC);
#else
            uartPutHex(M1_IN1_DC | M1_IN2_DC);
#endif /* DISABLE_PWM */
            break;
        case 2: 
            /* m2speed */
#if DISABLE_PWM
            uartPutHex(M2_PWM_DC);
#else
            uartPutHex(M2_IN1_DC | M2_IN2_DC);
#endif /* DISABLE_PWM */
            break;
        case 3: 
            /* m1disable */
#if DISABLE_PWM
            uart_puts_P("Error: Not implemented\r\n");
#else
            uartPutHex(0x1&(M1_PIN>>PA1));
#endif /* DISABLE_PWM */
            break;
        case 4: 
            /* m2disable */
#if DISABLE_PWM
            uart_puts_P("Error: Not implemented\r\n");
#else
            uartPutHex(0x1&(M2_PIN>>PA5));
#endif /* DISABLE_PWM */
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

