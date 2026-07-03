#ifndef VOICE_H
#define VOICE_H

#include <stdint.h>
#include <stdbool.h>

// 语音指令枚举
typedef enum {
    VOICE_CMD_NONE = 0,
    VOICE_CMD_WAKEUP,
    VOICE_CMD_START,
    VOICE_CMD_PILT_UP,
    VOICE_CMD_PILT_DOWN,
    VOICE_CMD_PILT_LEFT,
    VOICE_CMD_PILT_RIGHT,
    VOICE_CMD_PILT_STOP,
    VOICE_CMD_GEAR_RUN,
    VOICE_CMD_GEAR_STOP,
    VOICE_CMD_SEND_CLOUD,
    VOICE_CMD_IDENTIFY_TAG,
    VOICE_CMD_FIND_CRACK
} voice_cmd_t;

// 调试变量声明
extern volatile uint8_t g_uart_rx_data;
extern volatile uint16_t g_uart_queue_count;
extern volatile bool g_uart_callback_triggered;

// 函数声明
void Voice_Init(void);
void Voice_Process(void);
voice_cmd_t Voice_GetCommand(void);
void UART_Init(void);

#endif // VOICE_H
