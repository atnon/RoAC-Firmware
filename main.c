#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "uart.h"
#include "config.h"
#include "motor.h"
#include "cmd.h"
#include "adc.h"

static void initRegisters(void) {
    /* Setup Leds as outputs. */
    LEDDDR |= LEDDDRBITS;

    /* Set appropriate outputs to motors. */
    M1_DDR |= M1_DDRBITS;
    M2_DDR |= M2_DDRBITS;
}


uint8_t uartWorker(uint16_t getChar, uint8_t localEcho) {
    uint16_t character = getChar;
    if (character & UART_NO_DATA) {
        /* Nothing to see here. */
        return 0;
    } else {
        /* Data! */
        if (localEcho) {
            uart1_putc((uint8_t)character); /* Loopback */
            if((uint8_t)character == '\r') uart1_putc('\n'); /* New line if return. */
        }
        return (uint8_t)character;
    }
}

typedef struct cmdBuffer_ {
    uint8_t buffer[127];
    uint8_t length;
    uint8_t head;
} cmdBuffer;

const cmdBuffer CMD_BUFFER_DEFAULTS = { "", 128, 0 };

void uartParser(uint8_t uartChar, cmdBuffer *buf) {
    if((uartChar == '\n') || (uartChar == '\r')) { 
        /* Set null terminator for detection of end of string. */
        buf->buffer[buf->head] = '\0';
        /* Go to beginning of buffer. */
        buf->head = 0;
        /* Process command. */
        cmdParser(buf->buffer);
        /* uart_puts_P("# "); */
    } else if ((buf->head < buf->length) && ((uartChar > 0x1F) && (uartChar < 0x7F))){
        /* Buffer is not full and character is valid.
         * Put character in buffer. */
        buf->buffer[buf->head] = uartChar;
        buf->head++;
    }
}

int main(void)
{
    initRegisters();
    initPwm();

    /* UART0 connected to FT312. */
    uart_init((UART_BAUD_SELECT((SERIAL_BAUDRATE), F_CPU)));
    cmdBuffer uart0Buffer = CMD_BUFFER_DEFAULTS;
    uint8_t uartChar;    
    /* UART1 connected to FT230. */
    uart1_init((UART_BAUD_SELECT((SERIAL_BAUDRATE), F_CPU)));
    cmdBuffer uart1Buffer = CMD_BUFFER_DEFAULTS;
    uint8_t uart1Char;
    
    sei(); /* Enable interrupts. */

    initAdc();
    uart1_puts_P("Welcome to the Robot of Awesome Controller terminal\r\n");
    uart1_puts_P("# ");
    LEDREG |= LED1;
    
    setEnableM1(1);
    setEnableM2(1);


    while(1)
    {
        uartChar = uartWorker(uart_getc(), 0);
        uartParser(uartChar, &uart0Buffer);
        uart1Char = uartWorker(uart1_getc(), 1);
        uartParser(uart1Char, &uart1Buffer);

    }
}
