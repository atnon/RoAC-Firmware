#ifndef ADC_H_
#define ADC_H_

void initAdc(void);

uint16_t getADCVal(void);

uint8_t curAdc;
uint16_t lastAdcValM1;
uint16_t lastAdcValM2;

#endif /* ADC_H_ */
