#include <avr/io.h>

/* Leds */
#define LEDREG	        PORTC
#define LEDDDR	        DDRC
#define LED1	        (1<<PC5)
#define LED2	        (1<<PC4)
#define LED3	        (1<<PC7)
#define LED4	        (1<<PC6)
#define LEDDDRBITS      (LED1 | LED2 | LED3 | LED4)

/* Buttons */
#define BTNREG	        PORTB
#define BTNDDR	        DDRB
#define BTN1	        (1<<PB0)	/* T0, PCINT8 */
#define BTN2	        (1<<PB1)	/* T1, PCINT9 */

/* Motor 1 */
#define M1_REG			PORTA
#define M1_DDR			DDRA
#define M1_ENABLE		(1<<PA0)
#define M1_DISABLE		(1<<PA1)
#define M1_FAULT		(1<<PA2)	/* PCINT2 */
#define M1_FEEDBACK		(1<<PA3)	/* ADC3 */
#define M1_DDRBITS      (M1_ENABLE | M1_DISABLE)

/* Motor 1 PWM */
#define M1_PWMREG		PORTB
#define M1_PWMDDR	    DDRB
#define M1_IN1			(1<<PB3)	/* OC0A */
#define M1_IN2			(1<<PB4)	/* OC0B */
#define M1_PWMDDRBITS   (M1_IN1 | M2_IN2)

/* Motor 2 */
#define M2_REG			PORTA
#define M2_DDR			DDRA
#define M2_ENABLE		(1<<PA4)
#define M2_DISABLE		(1<<PA5)
#define M2_FAULT		(1<<PA6)	/* PCINT6 */
#define M2_FEEDBACK		(1<<PA7)	/* ADC7 */
#define M2_DDRBITS      (M2_ENABLE | M2_DISABLE) 

/* Motor 2 PWM */
#define M2_PWMREG		PORTD
#define M2_PWMDDR	    DDRD
#define M2_IN1			(1<<PD5)	/* OC1A */
#define M2_IN2			(1<<PD4)	/* OC1B */
#define M2_PWMDDRBITS   (M2_IN1 | M2_IN2)   

/* Bit functions */
static inline void setBit(uint8_t *port, uint8_t bits) {
	*port |= bits;
}

static inline void clearBit(uint8_t *port, uint8_t bits) {
	*port &= ~(bits);
}

static inline void toggleBit(uint8_t *port, uint8_t bits) {
	*port ^= bits;
}

static void initRegisters(void) {
    /* Setup Leds as outputs. */
    setBit(LEDDDR, LEDDDRBITS);

    /* Set appropriate outputs to motors. */
    setBit(M1_DDR, M1_DDRBITS);
    setBit(M2_DDR, M2_DDRBITS);
}

static void initPwm(void) {
    /* Setups the timers for PWM.
     * The different PWMs are enabled through:
     *  TCCR0A |= (1<<COM0A1) for OC0A
     *  TCCR0A |= (1<<COM0B1) for OC0B
     *  TCCR1A |= (1<<COM1A1) for OC1A
     *  TCCR1A |= (1<<COM1B1) for OC1B
     *
     * Duty cycle is adjusted by setting:
     *  OCR0A for OC0A
     *  OCR0B for OC0B
     *  OCR1A for OC1A
     *  OCR1B for OC1B
     *
     * Note that the default duty cycle is 0%. */

    /* Init timer 0 assoicated with OC0A/B. */
    /* Mode 1 - Phase Correct PWM. */
    TCCR0A |= (1<<WGM00);
    /* Prescaler selection.
     * Our frequency is 20e6/8=2.5MHz.
     * Frequency of PWM should be ~10kHz.
     * Thus, a prescaler of 256 should do => CS02=1. Ref p. 107 */
    TCCR0B |= (1<<CS02);

    /* Init Timer 1 associated with OC1A/B.
     * Same settings apply, even if timer 1 is a 16-bit timer.
     * Mode 1 - PWM, Phase Correct, 8-bit. */
    TCCR1A |= (1<<WGM10);
    /* Analogous to the timer 0 case, we set a prescaler of 256.
     * Thus, CS12=1. */
    TCCR1B |= (1<<CS12);
    
    /* Set PWM ports as outputs. */
    setBit(M1_PWMDDR, M1_PWMDDRBITS);
    setBit(M2_PWMDDR, M2_PWMDDRBITS);
}                

static void setSpeedM1(int8_t speed) {
    /* Function to set the speed and direction of M1.
     * Positive speed => Forward.
     * Negative speed => Reverse. */

    if (speed == 0) {
        /* Set M1_IN1 and M1_IN2 to zero. */
    } else if (speed < 0) {
        /* Reverse.
         * Set M1_IN1 to 0.
         * Set M1_IN2 to prefered duty cycle. */
    } else if (speed > 0) {
        /* Forward.
         * Set M1_IN1 to prefered duty cycle.
         * Set M1_IN2 to 0. */
    } else {
        /* Really shouldn't go here... */
    }
}

int main(void)
{
    while(1)
    {
        //TODO:: Please write your application code 
    }
}
