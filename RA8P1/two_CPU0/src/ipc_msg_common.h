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
    IPC_MSG_ID_VOICE_CMD  = 0x05U,
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

/* 语音控制命令值定义 (param1字段) */
#define IPC_VOICE_START         (0x01U)  // 启动
#define IPC_VOICE_PTZ_UP        (0x02U)  // 云台向上运动
#define IPC_VOICE_PTZ_DOWN      (0x03U)  // 云台向下运动
#define IPC_VOICE_PTZ_LEFT      (0x04U)  // 云台向左运动
#define IPC_VOICE_PTZ_RIGHT     (0x05U)  // 云台向右运动
#define IPC_VOICE_PTZ_STOP      (0x06U)  // 云台停止转动
#define IPC_VOICE_GEAR_START    (0x07U)  // 开始转动齿轮
#define IPC_VOICE_GEAR_STOP     (0x08U)  // 停止转动齿轮
#define IPC_VOICE_REPORT_DATA   (0x09U)  // 上报数据
#define IPC_VOICE_TAG_START     (0x0AU)  // 开始识别标签
#define IPC_VOICE_CRACK_START   (0x0BU)  // 开始寻找裂痕

/* IPC通道定义 */
#define IPC_CHANNEL_CPU0_TO_CPU1    (0U)
#define IPC_CHANNEL_CPU1_TO_CPU0    (1U)

#endif /* IPC_MSG_COMMON_H_ */
