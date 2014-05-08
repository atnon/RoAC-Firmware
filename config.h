#ifndef CONFIG_H_
#define CONFIG_H_
    
    /* The DISABLE_PWM setting selects the desired
     * way of using the motor controller.
     * 
     * The need of this feature was realized after the construction
     * of the controller card. Unfortunately this makes the precompiler
     * code a bit less readable.
     * Thus: Hic sunt dracones.
     *
     * DISABLE_PWM = 0:
     *     PWM the IN1 and IN2 inputs on the motor controller.
     *     This makes the controller brake the motor during the non-active
     *     part of the dutycycle. Makes for a lot of vibrations, but stops
     *     the motor immediately when setting the speed to zero.
     *
     * DISABLE_PWM = 1:
     *     PWM the DISABLE pin of the motor controller.
     *     This makes the controller set the outputs to high impedance mode
     *     during the non-active part of the dutycycle.
     *     In this mode, the direction of the motor is set through
     *     the IN1 and IN2 bits of the motor controller.
     */
    #define DISABLE_PWM 1

    /* UART */
    #define SERIAL_BAUDRATE 57600
    #define UART_RX_BUFFER_SIZE 32
    #define UART_TX_BUFFER_SIZE 64
   
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
    #define M1_FAULT        (1<<PA2)    /* PCINT2 */
    #define M1_FEEDBACK     (1<<PA3)    /* ADC3 */
    #define M1_FEEDBACKADC  3
    #if DISABLE_PWM
        /* See note at top of file. */
        #define M1_IN1          (1<<PA1)
        #define M1_DDRBITS      (M1_ENABLE | M1_IN1)
    #else
        #define M1_DISABLE      (1<<PA1)
        #define M1_DDRBITS      (M1_ENABLE | M1_DISABLE)
    #endif /* DISABLE_PWM */

    /* Motor 1 PWM */
    #define M1_PWMREG       PORTB
    #define M1_PWMDDR       DDRB
    #if DISABLE_PWM
        /* See note at top of file. */
        #define M1_PWM      (1<<PB3)    /* OC0A */
        #define M1_IN2      (1<<PB4)
        #define M1_PWM_DC   OCR0A
        #define M1_PWMDDRBITS   (M1_PWM | M1_IN2)
    #else
        #define M1_IN1          (1<<PB3)    /* OC0A */
        #define M1_IN2          (1<<PB4)    /* OC0B */
        #define M1_IN1_DC       OCR0A       /* Duty Cycle */
        #define M1_IN2_DC       OCR0B       /* Duty Cycle */
        #define M1_PWMDDRBITS   (M1_IN1 | M1_IN2)
    #endif /* DISABLE_PWM */

    /* Motor 2 */
    #define M2_REG          PORTA
    #define M2_DDR          DDRA
    #define M2_PIN          PINA
    #define M2_ENABLE       (1<<PA4)
    #define M2_FAULT        (1<<PA6)    /* PCINT6 */
    #define M2_FEEDBACK     (1<<PA7)    /* ADC7 */
    #define M2_FEEDBACKADC  7
    #if DISABLE_PWM
        /* See note at top of file. */
        #define M2_IN1          (1<<PA5)
        #define M2_DDRBITS      (M2_ENABLE | M2_IN1)
    #else
        #define M2_DISABLE      (1<<PA5)
        #define M2_DDRBITS      (M2_ENABLE | M2_DISABLE) 
    #endif /* DISABLE_PWM */

    /* Motor 2 PWM */
    #define M2_PWMREG       PORTD
    #define M2_PWMDDR       DDRD
    #if DISABLE_PWM
        /* See note at top of file. */
        #define M2_PWM      (1<<PB5)    /* OC1A */
        #define M2_IN2      (1<<PB4)
        #define M2_PWM_DC   OCR1A
        #define M2_PWMDDRBITS   (M2_PWM | M2_IN2)
    #else
        #define M2_IN1          (1<<PD5)    /* OC1A */
        #define M2_IN2          (1<<PD4)    /* OC1B */
        #define M2_IN1_DC       OCR1A       /* Duty Cycle */
        #define M2_IN2_DC       OCR1B       /* Duty Cycle */
        #define M2_PWMDDRBITS   (M2_IN1 | M2_IN2)   
    #endif /* DISABLE_PWM */

#endif /* CONFIG_H_ */
