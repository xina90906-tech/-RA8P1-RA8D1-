#ifndef __BSP_DEBUG_UART_H
#define	__BSP_DEBUG_UART_H
#include "hal_data.h"
#include "stdio.h"


void Debug_UART9_Init(void);

/* 轻量级调试打印函数 - 不使用 printf，节省栈空间 */
void Debug_Print(const char *str);

/* 格式化调试打印函数 - 支持 %d, %s, %x 等格式化 */
void Debug_Printf(const char *fmt, ...);

/* UART9 接收数据相关变量 */
extern volatile bool uart9_rx_data_ready;
extern volatile uint8_t uart9_rx_data;


#endif
