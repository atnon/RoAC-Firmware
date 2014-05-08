#include <avr/io.h>
#include <avr/interrupt.h>
#include "config.h"
#include "adc.h"

void initAdc(void) {
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

uint16_t getADCVal(void) {
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

}

