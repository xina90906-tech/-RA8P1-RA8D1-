#ifndef __GPIO_H
#define __GPIO_H

#include "hal_data.h"

/* 硬件引脚定义 */
// X轴方向引脚定义
#define X_DIR_PORT &g_ioport_ctrl
#define X_DIR_PIN  BSP_IO_PORT_01_PIN_05

// Y轴方向引脚定义
#define Y_DIR_PORT &g_ioport_ctrl
#define Y_DIR_PIN  BSP_IO_PORT_13_PIN_06

/* 方向定义 */
//#define DIR_SHUN  0 // 顺时针
//#define DIR_NI    1 // 逆时针

/* 函数声明 */
void GPIO_DirectionInit(void);
void GPIO_SetXDirection(uint8_t direction);
void GPIO_SetYDirection(uint8_t direction);

#endif /* __GPIO_H */
