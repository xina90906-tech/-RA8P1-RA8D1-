#ifndef __PULSE_H
#define __PULSE_H

#include "hal_data.h"

enum Axis
{
    x_axis = 0,
    y_axis = 1,
    all_axis = 2,
};

void PULSE_Init(void);
void PULSE_SetXAxisPulse(uint32_t frequency);
void PULSE_SetYAxisPulse(uint32_t frequency);
void PULSE_Stop(enum Axis axis);    

uint8_t PULSE_GetFlag(enum Axis axis);

#endif /* __PULSE_H */
