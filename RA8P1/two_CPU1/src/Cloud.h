#ifndef __CLOUD_H
#define __CLOUD_H

#include "hal_data.h"
#include "PULSE.h"

#define min_vel_Acc 2

extern uint32_t X_pulse;
extern uint32_t Y_pulse;

/* 函数声明 */
/*脉冲坐标系*/
void Cloud_X_Move_speed(uint32_t speed,uint8_t direction);
void Cloud_Y_Move_speed(uint32_t speed,uint8_t direction);
void Cloud_X_Move_position(int32_t pulse, uint32_t speed);
void Cloud_Y_Move_position(int32_t pulse, uint32_t speed);
void Cloud_XY_Move_speed(uint32_t x_speed, uint32_t y_speed,uint8_t direction_x,uint8_t direction_y);
void Cloud_XY_Move_position(int32_t x_pulse, int32_t y_pulse, uint32_t speed_x, uint32_t speed_y);
void Cloud_Calculate_Sync(int32_t clk_x, int32_t clk_y, uint32_t base_vel_Acc,
                                    uint32_t *vel_Acc_x, uint32_t *vel_Acc_y);
void Cloud_XY_synMove_position(int32_t x_pulse, int32_t y_pulse, uint32_t base_speed);
void move_to_point(int32_t target_x, int32_t target_y , uint32_t speed_x,uint32_t speed_y);
int synmove_to_point(int32_t target_x, int32_t target_y , uint32_t base_speed);


// // 实际坐标系
// void Cloud_XY_Move_position_real(float x_mm, float y_mm, uint32_t speed_x, uint32_t speed_y);



void Cloud_Origin(uint32_t speed);
void Cloud_Stop(enum Axis axis);

#endif /* __CLOUD_H */
