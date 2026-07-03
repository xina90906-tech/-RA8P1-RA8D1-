#include "bsp_debug_uart.h"
#include "bsp_wifi_esp8266.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdarg.h>

/* LLVM tinystdio 需要的 stdio 符号定义 */
__attribute__((used)) static FILE _stdout_obj = { 0 };
__attribute__((used)) static FILE _stdin_obj  = { 0 };
__attribute__((used)) static FILE _stderr_obj = { 0 };

FILE *const stdout = &_stdout_obj;
FILE *const stdin  = &_stdin_obj;
FILE *const stderr = &_stderr_obj;

/* 发送完成标志 */
volatile bool uart_send_complete_flag1 = false;
volatile bool uart9_rx_data_ready = false;
volatile uint8_t uart9_rx_data = 0;

void vApplicationMallocFailedHook(void)
{
    /* 栈溢出或内存分配失败时停在这里 */
    while(1);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    while(1);
}

/* 调试串口 UART9 初始化 */
void Debug_UART9_Init(void)
{
    fsp_err_t err = FSP_SUCCESS;

    err = R_SCI_B_UART_Open (&g_uart9_ctrl, &g_uart9_cfg);
    assert(FSP_SUCCESS == err);
}


/* 串口中断回调 */
void g_uart9_Callback (uart_callback_args_t * p_args)
{
    switch (p_args->event)
    {
        case UART_EVENT_RX_CHAR:
        {
            /* 只保存接收到的数据，不在中断中发送 */
            uart9_rx_data = (uint8_t)p_args->data;
            uart9_rx_data_ready = true;
            break;
        }
        case UART_EVENT_TX_COMPLETE:
        {
            uart_send_complete_flag1 = true;
            break;
        }
        default:
            break;
    }
}


/* 重定向 printf 输出 - 使用 fputc 方式（最可靠） */
int fputc(int ch, FILE *f)
{
    (void)f;

    /* 检查 UART9 是否已初始化 */
    if (R_SCI_B_UART_Write(&g_uart9_ctrl, (uint8_t *)&ch, 1) != FSP_SUCCESS)
    {
        return -1;
    }

    /* 等待发送完成（带超时保护）*/
    uint32_t timeout = 0;
    while(uart_send_complete_flag1 == false)
    {
        timeout++;
        if (timeout > 100000)
        {
            break;  /* 防止死循环 */
        }
    }
    uart_send_complete_flag1 = false;

    return ch;
}

/* 同时提供 _write 实现以兼容 GCC */
int _write(int fd, char *pBuffer, int size)
{
    (void)fd;
    for (int i = 0; i < size; i++)
    {
        fputc(pBuffer[i], NULL);
    }
    return size;
}

/* 格式化调试打印函数 - 支持 %d, %s, %x 等格式化 */
void Debug_Printf(const char *fmt, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    Debug_Print(buffer);
}

/* 轻量级调试打印函数 - 直接通过 UART 发送，不使用 printf
   避免 printf 的栈消耗问题 */
void Debug_Print(const char *str)
{
    if (str == NULL) return;

    uint16_t len = 0;
    while (str[len] != '\0' && len < 256)
    {
        len++;
    }

    if (len == 0) return;

    R_SCI_B_UART_Write(&g_uart9_ctrl, (uint8_t *)str, len);

    /* 等待发送完成 */
    uint32_t timeout = 0;
    while(uart_send_complete_flag1 == false)
    {
        timeout++;
        if (timeout > 100000)
        {
            break;
        }
    }
    uart_send_complete_flag1 = false;
}
