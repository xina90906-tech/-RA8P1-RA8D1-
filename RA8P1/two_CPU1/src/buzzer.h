#ifndef __BUZZER_H
#define __BUZZER_H

#include "hal_data.h"
#include <stdint.h>

#define BUZZER_PIN    BSP_IO_PORT_03_PIN_08

void Buzzer_On(void);
void Buzzer_Off(void);
void Buzzer_Init(void);

#endif
