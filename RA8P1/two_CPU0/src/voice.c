#include "voice.h"
#include "hal_data.h"
#include <string.h>
#include <stdio.h>

// 使用正确的FSP函数名
#include "r_sci_b_uart.h"

/**
  * 语音指令缓存
  */
static voice_cmd_t g_voice_cmd = VOICE_CMD_NONE;
volatile uint8_t g_uart_rx_data = 0;
volatile uint16_t g_uart_queue_count = 0;
volatile bool g_uart_callback_triggered = false;

/**
  * 串口数据缓存
  */
#define VOICE_PACKET_LEN    1
static uint8_t g_packet_buffer[VOICE_PACKET_LEN];
static uint8_t g_packet_index = 0;
extern bool data_flag;

/**
  * UART0专用环形队列
  */
#define UART_QUEUE_SIZE    256
typedef struct {
    uint8_t data[UART_QUEUE_SIZE];
    uint16_t head;
    uint16_t tail;
} UART_Queue_t;
static UART_Queue_t g_uart_queue;

// 函数前向声明
static void Voice_ParseCommand(uint8_t data);
static void Voice_ExecuteCommand(void);
static void UART_SendSafe(uint8_t *data, size_t len);
static void UART_Queue_Init(void);
static bool UART_Queue_Write(uint8_t data);
static bool UART_Queue_Read(uint8_t *data);
static uint16_t UART_Queue_Count(void);

// 串口发送完成标志
volatile bool uart_send_complete_flag = false;

/**
  * 语音模块初始化
  */
void Voice_Init(void)
{
    UART_Init();
    UART_Queue_Init();
    g_voice_cmd = VOICE_CMD_NONE;
    g_packet_index = 0;
    uart_send_complete_flag = true;
}

/**
  * 获取当前语音指令
  */
voice_cmd_t Voice_GetCommand(void)
{
    voice_cmd_t cmd = g_voice_cmd;
    g_voice_cmd = VOICE_CMD_NONE;
    return cmd;
}

/**
  * 解析语音指令
  */
static void Voice_ParseCommand(uint8_t data)
{
    g_packet_buffer[0] = data;
    g_packet_index = 1;

    if (g_packet_index >= VOICE_PACKET_LEN)
    {
        Voice_ExecuteCommand();
        g_packet_index = 0;
    }
}

/**
  * 安全串口发送（等待完成）
  */
static void UART_SendSafe(uint8_t *data, size_t len)
{
    if(len == 0) return;
    uart_send_complete_flag = false;
    R_SCI_B_UART_Write(&g_uart1_ctrl, data, len);
    // 等待发送完成
    while(uart_send_complete_flag == false);
}

/**
  * 执行语音指令
  */
static void Voice_ExecuteCommand(void)
{
    uint8_t cmd = g_packet_buffer[0];
    g_packet_buffer[0] = 0x00;

    switch (cmd)
    {
        case 0x00: g_voice_cmd = VOICE_CMD_WAKEUP;      break;
        case 0x01: g_voice_cmd = VOICE_CMD_START;       break;
        case 0x02: g_voice_cmd = VOICE_CMD_PILT_UP;     break;
        case 0x03: g_voice_cmd = VOICE_CMD_PILT_DOWN;   break;
        case 0x04: g_voice_cmd = VOICE_CMD_PILT_LEFT;   break;
        case 0x05: g_voice_cmd = VOICE_CMD_PILT_RIGHT;  break;
        case 0x06: g_voice_cmd = VOICE_CMD_PILT_STOP;   break;
        case 0x07: g_voice_cmd = VOICE_CMD_GEAR_RUN;    break;
        case 0x08: g_voice_cmd = VOICE_CMD_GEAR_STOP;   break;
        case 0x09: g_voice_cmd = VOICE_CMD_SEND_CLOUD;  break;
        case 0x0A: g_voice_cmd = VOICE_CMD_IDENTIFY_TAG;break;
        case 0x0B: g_voice_cmd = VOICE_CMD_FIND_CRACK;  break;
        default:
            break;
    }
}

/**
  * 清空UART队列
  */
static void UART_Queue_Clear(void)
{
    memset(&g_uart_queue, 0, sizeof(UART_Queue_t));
}

/**
  * 语音识别处理（必须在主循环调用）
  */
void Voice_Process(void)
{
    uint16_t queue_len = UART_Queue_Count();
    if (queue_len > 0)
    {
        // 处理队列中的所有数据
        while (queue_len > 0)
        {
            uint8_t data;
            if (UART_Queue_Read(&data))
            {
                Voice_ParseCommand(data);
            }
            queue_len = UART_Queue_Count();
        }
        
        // 处理完命令后清空队列
        UART_Queue_Clear();
    }
}

/**
  * UART初始化
  */
void UART_Init(void)
{
    R_SCI_B_UART_Open(&g_uart1_ctrl, &g_uart1_cfg);
    R_SCI_B_UART_Read(&g_uart1_ctrl, g_packet_buffer, VOICE_PACKET_LEN);
}

/**
  * UART回调函数：仅入队
  */
void g_uart0_Callback(uart_callback_args_t * p_args)
{
    g_uart_callback_triggered = true;
    switch (p_args->event)
    {
        case UART_EVENT_RX_CHAR:
            g_uart_rx_data = (uint8_t)p_args->data;
            UART_Queue_Write((uint8_t)p_args->data);
            g_uart_queue_count = UART_Queue_Count();
            R_SCI_B_UART_Read(&g_uart1_ctrl, g_packet_buffer, VOICE_PACKET_LEN);
            break;

        case UART_EVENT_RX_COMPLETE:
            break;

        case UART_EVENT_TX_COMPLETE:
            uart_send_complete_flag = true;
            break;

        default:
            break;
    }
}

/**
  * 初始化UART队列
  */
static void UART_Queue_Init(void)
{
    memset(&g_uart_queue, 0, sizeof(UART_Queue_t));
}

/**
  * 向UART队列写入数据
  */
static bool UART_Queue_Write(uint8_t data)
{
    uint16_t next_tail = (g_uart_queue.tail + 1) % UART_QUEUE_SIZE;
    if (next_tail == g_uart_queue.head) return false;
    g_uart_queue.data[g_uart_queue.tail] = data;
    g_uart_queue.tail = next_tail;
    return true;
}

/**
  * 从UART队列读取数据
  */
static bool UART_Queue_Read(uint8_t *data)
{
    if (g_uart_queue.head == g_uart_queue.tail) return false;
    *data = g_uart_queue.data[g_uart_queue.head];
    g_uart_queue.head = (g_uart_queue.head + 1) % UART_QUEUE_SIZE;
    return true;
}

/**
  * 获取UART队列数据数量
  */
static uint16_t UART_Queue_Count(void)
{
    return (g_uart_queue.tail - g_uart_queue.head + UART_QUEUE_SIZE) % UART_QUEUE_SIZE;
}
