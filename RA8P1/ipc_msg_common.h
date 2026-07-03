#ifndef IPC_MSG_COMMON_H_
#define IPC_MSG_COMMON_H_

#include "bsp_api.h"

/* IPC消息ID定义 */
typedef enum e_ipc_msg_id
{
    IPC_MSG_ID_PING       = 0x01U,
    IPC_MSG_ID_LED_CTRL   = 0x02U,
    IPC_MSG_ID_DATA_SYNC  = 0x03U,
    IPC_MSG_ID_STATUS_GET = 0x04U,
    IPC_MSG_ID_USER_MAX   = 0xFFU
} ipc_msg_id_t;

/* IPC消息结构体（32位，通过FIFO传递）
 * bit[31:24]  msg_id   消息ID
 * bit[23:16]  param1   参数1
 * bit[15:8]   param2   参数2
 * bit[7:0]    param3   参数3
 */
typedef union u_ipc_msg
{
    uint32_t word;
    struct
    {
        uint8_t param3;
        uint8_t param2;
        uint8_t param1;
        uint8_t msg_id;
    } parts;
} ipc_msg_t;

/* LED控制参数定义 */
#define IPC_LED_ON     (1U)
#define IPC_LED_OFF    (0U)
#define IPC_LED_TOGGLE (2U)

/* IPC通道定义 */
#define IPC_CHANNEL_CPU0_TO_CPU1    (0U)
#define IPC_CHANNEL_CPU1_TO_CPU0    (1U)

#endif /* IPC_MSG_COMMON_H_ */
