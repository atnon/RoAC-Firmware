#include <avr/io.h>
#include "motor.h"
#include "config.h"

void initPwm(void) {
    /* Setups the timers for PWM.
     * The different PWMs are enabled through:
     * TCCR0A |= (1<<COM0A1) for OC0A (M1_IN1)
     * TCCR0A |= (1<<COM0B1) for OC0B (M1_IN2)
     * TCCR1A |= (1<<COM1A1) for OC1A (M2_IN1)
     * TCCR1A |= (1<<COM1B1) for OC1B (M2_IN2)
     *
     * Duty cycle is adjusted by setting:
     * OCR0A for OC0A (M1_IN1)
     * OCR0B for OC0B (M1_IN2)
     * OCR1A for OC1A (M2_IN1)
     * OCR1B for OC1B (M2_IN2)
     *
     * Note that the default duty cycle is 0%. */

    /* Init timer 0 assoicated with OC0A/B. */
    /* Mode 1 - Phase Correct PWM. */
    TCCR0A |= (1<<WGM00);
    /* Prescaler selection.
     * Our frequency is 20MHz.
     * A prescaler of /256 gives a somewhat pleasant motor sound.
     * Clock selection ref p. 107 */
    TCCR0B |= (1<<CS02);

    /* Init Timer 1 associated with OC1A/B.
     * Same settings apply, even if timer 1 is a 16-bit timer.
     * Mode 1 - PWM, Phase Correct, 8-bit. */
    TCCR1A |= (1<<WGM10);
    /* Analogous to the timer 0 case, we set a prescaler of 1024.
     * Thus, CS10=CS12=1. */
    TCCR1B |= (1<<CS12);
    
    /* Set PWM ports as outputs. */
    M1_PWMDDR |= M1_PWMDDRBITS;
    M2_PWMDDR |= M2_PWMDDRBITS;

#if DISABLE_PWM
        /* The motor should be disabled at start. */
        M1_PWM_DC = 0xFF;
        M2_PWM_DC = 0xFF;
#endif /* DISABLE_PWM */
} 

void setSpeedM1(int8_t speed) {
    /* Function to set the speed and direction of M1.
     * Positive speed => Forward.
     * Negative speed => Reverse. 
     *
     * Note that the function maps the value ranges
     * 1:1:127 => 2:2:254
     * -1:1:-128 => 1:2:255
     * due to how the int8_t is represented. */

    if (speed > 0) {
        /* Forward. */

#if DISABLE_PWM
        /* Set M1_IN1 high, M1_IN2 low. */
        M1_REG |= M1_IN1;
        M1_PWMREG &= ~(M1_IN2);
        M1_PWM_DC = (0xFF - (speed<<1));
#else
        /* Set M1_IN1 to prefered duty cycle.
         * Set M1_IN2 to 0. */
        M1_IN1_DC = (speed<<1); /* Limit values 2:2:254*/
        M1_IN2_DC = 0x00;
#endif /* DISABLE_PWM */

    } else if (speed < 0) {
        /* Reverse. */
#if DISABLE_PWM
        /* Set M1_IN2 high, M1_IN1 low. */
        M1_REG &= ~(M1_IN1);
        M1_PWMREG |= M1_IN2;
        M1_PWM_DC = (0xFF - (((-speed)<<1)-1)); /* Limit values 1:2:255 */
#else
        /* Set M1_IN1 to 0.
         * Set M1_IN2 to prefered duty cycle. */
        M1_IN1_DC = 0x00;
        M1_IN2_DC = (((-speed)<<1)-1); /* Limit values 1:2:255 */
#endif /* DISABLE_PWM */

    } else {
        /* We're either at a speed of zero or out of bounds. */
#if DISABLE_PWM
        /* Simply disable the motor all of the time. */
        M1_PWM_DC = 0xFF;
#else
        /* Set M1_IN1 and M1_IN2 to zero. */
        M1_IN1_DC = 0x00;
        M1_IN2_DC = 0x00;
#endif /* DISABLE_PWM */

    }
}

void setSpeedM2(int8_t speed) {
    /* Function to set the speed and direction of M2 (rotated motor).
     * Positive speed => Forward.
     * Negative speed => Reverse. 
     *
     * Note that the function maps the value ranges
     * 1:1:127 => 2:2:254
     * -1:1:-128 => 1:2:255
     * due to how the int8_t is represented. */

    if (speed < 0) {
        /* Forward. */
#if DISABLE_PWM
        /* Set M1_IN2 high, M1_IN1 low. */
        M2_REG &= ~(M2_IN1);
        M2_PWMREG |= M2_IN2;
        M2_PWM_DC = (0xFF-((-speed)<<1)-1); /* Limit values 1:2:255 */
#else
        /* Set M2_IN1 to prefered duty cycle.
         * Set M2_IN2 to 0. */
        M2_IN1_DC = (((-speed)<<1)-1); /* Limit values 1:2:255 */
        M2_IN2_DC = 0x00;
#endif /* DISABLE_PWM */
    } else if (speed > 0) {
        /* Reverse. */
#if DISABLE_PWM
        /* Set M1_IN1 high, M1_IN2 low. */
        M2_REG |= M2_IN1;
        M2_PWMREG &= ~(M2_IN2);
        M2_PWM_DC = (0xFF-(speed<<1));
#else
        /* Set M2_IN1 to 0.
         * Set M2_IN2 to prefered duty cycle. */
        M2_IN2_DC = (speed<<1); /* Limit values 2:2:254*/
        M2_IN1_DC = 0x00;
#endif /* DISABLE_PWM */
    } else {
        /* We're either at a speed of zero or out of bounds. */
#if DISABLE_PWM
        /* Simply disable the motor all of the time. */
        M2_PWM_DC = 0xFF;
#else
        /* Set M2_IN1 and M2_IN2 to zero. */
        M2_IN1_DC = 0x00;
        M2_IN2_DC = 0x00;
#endif /* DISABLE_PWM */
    }
}

void setEnableM1(uint8_t state) {
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

void setEnableM2(uint8_t state) {
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

void setDisableM1(uint8_t state) {
#if DISABLE_PWM
#else
    /* Controls the state of the disable pin.
     * Setting the disable high will disable the H-bridge and 
     * put the outputs in Hi-Z mode. */
    if (state == 0) {
        M1_REG &= ~(M1_DISABLE);
    } else {
        M1_REG |= M1_DISABLE;
    }
#endif /* DISABLE_PWM */
}

void setDisableM2(uint8_t state) {
#if DISABLE_PWM
#else
    /* Controls the state of the disable pin.
     * Setting the disable high will disable the H-bridge and 
     * put the outputs in Hi-Z mode. */
    if (state == 0) {
        M2_REG &= ~(M2_DISABLE);
    } else {
        M2_REG |= M2_DISABLE;
    }
#endif /* DISABLE_PWM */
}
