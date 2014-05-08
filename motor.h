#ifndef MOTOR_H_
#define MOTOR_H_

/* Function to setup the proper PWM channels. */
void initPwm(void);

/* Function to set the speed and direction of M1.
 * Positive speed => Forward.
 * Negative speed => Reverse. 
 *
 * Note that the function maps the value ranges
 * 1:1:127 => 2:2:254
 * -1:1:-128 => 1:2:255
 * due to how the int8_t is represented. */
void setSpeedM1(int8_t speed);

/* Function to set the speed and direction of M2 (rotated motor).
 * Positive speed => Forward.
 * Negative speed => Reverse. 
 *
 * Note that the function maps the value ranges
 * 1:1:127 => 2:2:254
 * -1:1:-128 => 1:2:255
 * due to how the int8_t is represented. */
void setSpeedM2(int8_t speed);

/* This function controls the enable of the H-bridge.
 * If enable is set to zero, the device will enter sleep mode.
 * The function also controls weather the PWM is active or not. */
void setEnableM1(uint8_t state);

/* This function controls the enable of the H-bridge.
 * If enable is set to zero, the device will enter sleep mode.
 * The function also controls weather the PWM is active or not. */
void setEnableM2(uint8_t state);

/* Controls the state of the disable pin.
 * Setting the disable high will disable the H-bridge and 
 * put the outputs in Hi-Z mode. */
void setDisableM1(uint8_t state);

/* Controls the state of the disable pin.
 * Setting the disable high will disable the H-bridge and 
 * put the outputs in Hi-Z mode. */
void setDisableM2(uint8_t state);
#endif /* MOTOR_H_ */
