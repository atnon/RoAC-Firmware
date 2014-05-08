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
