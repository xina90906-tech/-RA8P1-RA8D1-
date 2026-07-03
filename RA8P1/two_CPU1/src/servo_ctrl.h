#ifndef __SERVO_CTRL_H__
#define __SERVO_CTRL_H__

#include "servo_thread.h"
#include "servo.h"
#include <stdint.h>

/**
  * 函    数：发送舵机控制命令（通过任务通知量）
  * 参    数：cmd 命令值 (0=暂停, 1=开始)
  * 返 回 值：无
  * 说    明：此函数可从任何任务调用，用于控制舵机启动/停止
  */
static inline void servo_ctrl_send_cmd(uint32_t cmd)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xTaskNotifyFromISR(
        servo_thread,           // servo_thread任务句柄
        cmd,                     // 通知值
        eSetValueWithOverwrite,  // 覆盖模式
        &xHigherPriorityTaskWoken
    );

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
  * 简化的控制宏
  */
#define SERVO_START()    servo_ctrl_send_cmd(SERVO_CMD_START)
#define SERVO_STOP()     servo_ctrl_send_cmd(SERVO_CMD_STOP)

#endif
