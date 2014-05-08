#ifndef CMD_H_
#define CMD_H_

static char *cmdList[];

static char *cmdPropList[];

void cmdParser(uint8_t *bufPtr);

void cmdSet(uint8_t *bufPtr);

void cmdGet(uint8_t *bufPtr);

void setProperty(uint8_t propIndex, int8_t value);

void uartPutHex(uint16_t num);
#endif /* CMD_H_ */
