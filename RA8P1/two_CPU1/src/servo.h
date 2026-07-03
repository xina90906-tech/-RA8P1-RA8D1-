#ifndef __SERVO_H
#define __SERVO_H

#include "hal_data.h"
#include <stdint.h>

#define SERVO_ANGLE_MIN     0U    // 最小角度 0度
#define SERVO_ANGLE_MAX     270U   // 最大角度 270度

/* 舵机一个完整周期时间（0°→180°→0°），单位ms，可修改 */
#define SERVO_CYCLE_MS      20000U  // 默认10秒一个来回

/* 任务通知值定义 */
#define SERVO_CMD_STOP       0U    // 暂停
#define SERVO_CMD_START      1U    // 开始

void servo_init(void);
void servo_start(void);
void servo_stop(void);
void servo_process(void);
uint8_t servo_get_angle(void);

#endif
