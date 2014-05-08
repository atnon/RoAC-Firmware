#ifndef CONFIG_H_
#define CONFIG_H_

    /* UART */
    #define SERIAL_BAUDRATE 57600
    #define SERIAL_RX_BUFFER_SIZE 32
    #define SERIAL_TX_BUFFER_SIZE 64
   
    /* Leds */
    #define LEDREG          PORTC
    #define LEDDDR          DDRC
    #define LED1            (1<<PC5)
    #define LED2            (1<<PC4)
    #define LED3            (1<<PC7)
    #define LED4            (1<<PC6)
    #define LEDDDRBITS      (LED1 | LED2 | LED3 | LED4)
   
    /* Buttons */
    #define BTNREG          PORTB
    #define BTNDDR          DDRB
    #define BTN1            (1<<PB0)    /* T0, PCINT8 */
    #define BTN2            (1<<PB1)    /* T1, PCINT9 */
   
    /* Motor 1 */
    #define M1_REG          PORTA
    #define M1_DDR          DDRA
    #define M1_PIN          PINA
    #define M1_ENABLE       (1<<PA0)
    #define M1_DISABLE      (1<<PA1)
    #define M1_FAULT        (1<<PA2)    /* PCINT2 */
    #define M1_FEEDBACK     (1<<PA3)    /* ADC3 */
    #define M1_FEEDBACKADC  3
    #define M1_DDRBITS      (M1_ENABLE | M1_DISABLE)
   
    /* Motor 1 PWM */
    #define M1_PWMREG       PORTB
    #define M1_PWMDDR       DDRB
    #define M1_IN1          (1<<PB3)    /* OC0A */
    #define M1_IN2          (1<<PB4)    /* OC0B */
    #define M1_IN1_DC       OCR0A       /* Duty Cycle */
    #define M1_IN2_DC       OCR0B       /* Duty Cycle */
    #define M1_PWMDDRBITS   (M1_IN1 | M1_IN2)
   
    /* Motor 2 */
    #define M2_REG          PORTA
    #define M2_DDR          DDRA
    #define M2_PIN          PINA
    #define M2_ENABLE       (1<<PA4)
    #define M2_DISABLE      (1<<PA5)
    #define M2_FAULT        (1<<PA6)    /* PCINT6 */
    #define M2_FEEDBACK     (1<<PA7)    /* ADC7 */
    #define M2_FEEDBACKADC  7
    #define M2_DDRBITS      (M2_ENABLE | M2_DISABLE) 
   
    /* Motor 2 PWM */
    #define M2_PWMREG       PORTD
    #define M2_PWMDDR       DDRD
    #define M2_IN1          (1<<PD5)    /* OC1A */
    #define M2_IN2          (1<<PD4)    /* OC1B */
    #define M2_IN1_DC       OCR1A       /* Duty Cycle */
    #define M2_IN2_DC       OCR1B       /* Duty Cycle */
    #define M2_PWMDDRBITS   (M2_IN1 | M2_IN2)   

#endif /* CONFIG_H_ */
