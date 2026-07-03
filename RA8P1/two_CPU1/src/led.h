#ifndef LED_LED_H_
#define LED_LED_H_

#include "hal_data.h"
#include <stdint.h>

#define LED_PIN    BSP_IO_PORT_03_PIN_12

void LED_Init(void);
void LED_On(void);
void LED_Off(void);

#endif /* LED_LED_H_ */
