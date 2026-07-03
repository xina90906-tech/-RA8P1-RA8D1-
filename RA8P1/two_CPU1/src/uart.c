#include "uart.h"
#include "hal_data.h"
#include <stdio.h>
#include "Wifi_thread.h"


void UART_Init(void)
{
	R_SCI_B_UART_Open(&g_uart0_ctrl, &g_uart0_cfg);
}
//定义环形队列结构体变量
Circular_queue_t Circular_queue;

/**
 * @brief 初始化队列
 * @param circular_queue: 要使用的队列
 * @retval 若初始化成功返回true,反之false
 */
bool Queue_Init(Circular_queue_t *circular_queue)
{
    if (!circular_queue)
        return false;
    memset (circular_queue, 0, sizeof(Circular_queue_t));
    return true;
}

/**
 * @brief 判断队列是否为空
 * @param circular_queue: 要使用的队列
 * @retval 若队列为空返回true,反之或指针非法则false
 */
bool Queue_isEmpty(Circular_queue_t *circular_queue)
{
    if (!circular_queue)
        return false;

    if (circular_queue->head == circular_queue->tail) //头指针等于尾指针则为空
        return true;
    else
        return false;
}

/**
 * @brief 判断队列是否已满
 * @param circular_queue: 要使用的队列
 * @return 若队列已满返回true,反之或指针非法则false
 */
bool Queue_isFull(Circular_queue_t *circular_queue)
{
    if (!circular_queue)
        return false;

    if ((circular_queue->tail + 1) % DATA_LEN == circular_queue->head)
        return true;
    else
        return false;
}

/**
 * @brief 返回队列中数据的长度
 * @param circular_queue
 * @return len_haduse
 */
uint16_t Queue_HadUse(Circular_queue_t *circular_queue)
{
    if (!circular_queue)
        return false;
    return (uint16_t)(circular_queue->tail - circular_queue->head + DATA_LEN) % DATA_LEN;
}

/**
 * @brief 返回队列未使用数据长度
 * @param circular_queue
 * @return len_NoUse
 */
uint16_t Queue_NoUse(Circular_queue_t *circular_queue)
{
    if (!circular_queue)
        return false;
    return (uint16_t)(DATA_LEN - (circular_queue->tail -
           circular_queue->head + DATA_LEN) % DATA_LEN -1); //保留一个Byte数据用来区分队列的满和空状态
}

/**
 * @brief 给队列写数据
 * @param circular_queue: 要使用的队列
 * @param string: 要写入的数据
 * @param len: 数据长度
 * @return 写入成功则返回true，反之或指针非法则false
 */
bool Queue_Wirte( Circular_queue_t *circular_queue, uint8_t *string, uint16_t len)
{
    if (!circular_queue)
        return false;

    if(Queue_NoUse(circular_queue)<len)
        return false;
    for(int i=0;i<len;i++)
    {
        circular_queue->data[circular_queue->tail]=string[i];
        circular_queue->tail = (uint16_t)(circular_queue->tail+1)%DATA_LEN; //防止越界
    }
    return true;
}

/**
 * @brief 读队列的数据
 * @param circular_queue
 * @param string
 * @param len
 * @return 读数据成功则返回true，反之或指针非法则false
 */
bool Queue_Read(Circular_queue_t *circular_queue, uint8_t *string, uint16_t len)
{
    if (!circular_queue)
            return false;

    if(Queue_HadUse(circular_queue)<len)
        return false;
    for(int i=0;i<len;i++)
    {
        string[i]=circular_queue->data[circular_queue->head];
        circular_queue->head = (uint16_t)(circular_queue->head+1)%DATA_LEN; //防止越界
    }
    return true;
}

/* 发送完成标志 */
volatile bool uart_send_complete_flag = false;


/* 串口中断回调 */
void g_uart0_Callback(uart_callback_args_t * p_args)
{
    switch (p_args->event)
    {
        case UART_EVENT_RX_CHAR:
        {
            /* 接收到数据后马上写入队列中 */
            Queue_Wirte(&Circular_queue, (uint8_t*) &p_args->data, 1);
			//printf("%d",(int) &p_args->data);
            break;
        }
        case UART_EVENT_RX_COMPLETE:
        {
            uart_send_complete_flag = true;
            break;
        }
        default:
            break;
    }
}

//int fputc(int ch, FILE *f)
//{
//    (void)f;
//    R_SCI_UART_Write(g_uart9.p_ctrl, (uint8_t *)&ch, 1);
//    while(uart_send_complete_flag == false);
//    uart_send_complete_flag = false;

//    return ch;
//}

/**
 * @brief 处理接收到的数据包
 * 协议格式: [0xAA][0xAA][tab][crack][checksum][0xFF][0xFF]
 * 校验位 = tab + crack
 * @retval 若处理成功返回true，反之false
 */
bool UART_ProcessPacket(void)
{
    uint8_t buffer[7];
    uint16_t queue_length = Queue_HadUse(&Circular_queue);
    
    if (queue_length < 7)
    {
        return false;
    }
    
    uint8_t temp_buffer[DATA_LEN];
    uint16_t temp_len = Queue_HadUse(&Circular_queue);
    Queue_Read(&Circular_queue, temp_buffer, temp_len);
    
    uint16_t start_index = 0;
    bool found = false;
    
    for (uint16_t i = 0; i < temp_len - 1; i++)
    {
        if (temp_buffer[i] == 0xAA && temp_buffer[i+1] == 0xAA)
        {
            start_index = i;
            found = true;
            break;
        }
    }
    
    if (!found)
    {
        Circular_queue.head = Circular_queue.tail;
        return false;
    }
    
    if (start_index + 7 > temp_len)
    {
        Queue_Wirte(&Circular_queue, temp_buffer, temp_len);
        return false;
    }
    
    for (uint16_t i = 0; i < 7; i++)
    {
        buffer[i] = temp_buffer[start_index + i];
    }
    
    if (buffer[5] != 0xFF || buffer[6] != 0xFF)
    {
        Circular_queue.head = Circular_queue.tail;
        return false;
    }
    
    uint8_t checksum = buffer[2] + buffer[3];
    if (buffer[4] != checksum)
    {
        Circular_queue.head = Circular_queue.tail;
        return false;
    }
    
    static int last_mqtt_tab = -1;
    
    g_mqtt_tab = buffer[2];
    g_mqtt_crack ++;
    
    if (g_mqtt_tab != last_mqtt_tab)
    {
        last_mqtt_tab = g_mqtt_tab;
        g_mqtt_tab_changed = true;
    }
    
    Circular_queue.head = Circular_queue.tail;
    
    return true;
}



/**
 * @brief 通过串口9发送0xFF
 * @param count 发送次数
 * @retval 无
 */
void UART0_Send_FF(uint16_t count)
{
    uint8_t data = 0xFF;
    for (uint16_t i = 0; i < count; i++)
    {
    	R_SCI_B_UART_Write(&g_uart0_ctrl, &data, 1);
        // 等待发送完成
        uart_send_complete_flag = false;
        while(!uart_send_complete_flag)
        {
            // 等待发送完成
        }
    }
}

