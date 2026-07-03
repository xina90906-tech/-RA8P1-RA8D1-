#include "new_thread0.h"
#include "ipc_msg_common.h"
#include "voice.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "hal_data.h"

static uint32_t g_send_count = 0U;
static QueueHandle_t g_voice_cmd_queue = NULL;

// 语音命令状态保持机制
static volatile voice_cmd_t g_pending_voice_cmd = VOICE_CMD_NONE;

#define VOICE_CMD_QUEUE_SIZE    (32U)

static fsp_err_t ipc_send_msg(uint8_t msg_id, uint8_t param1, uint8_t param2, uint8_t param3)
{
    ipc_msg_t msg;
    msg.parts.msg_id = msg_id;
    msg.parts.param1 = param1;
    msg.parts.param2 = param2;
    msg.parts.param3 = param3;

    return g_ipc0.p_api->messageSend(g_ipc0.p_ctrl, msg.word);
}

static void ipc_send_ping(void)
{
    g_send_count++;
    fsp_err_t err = ipc_send_msg(IPC_MSG_ID_PING, (uint8_t)(g_send_count >> 8), (uint8_t)g_send_count, 0U);
    (void)err;
}

static fsp_err_t ipc_send_voice_cmd(uint8_t cmd)
{
    fsp_err_t err = ipc_send_msg(IPC_MSG_ID_VOICE_CMD, cmd, 0U, 0U);
    return err;
}

static void VoiceIpcSenderTask(void *pvParameters)
{
    FSP_PARAMETER_NOT_USED(pvParameters);
    uint8_t cmd;

    while (1)
    {
        if (xQueueReceive(g_voice_cmd_queue, &cmd, portMAX_DELAY) == pdPASS)
        {
            fsp_err_t err = ipc_send_voice_cmd(cmd);
            if (err == FSP_SUCCESS)
            {
                // IPC发送成功后，清除待处理命令
                g_pending_voice_cmd = VOICE_CMD_NONE;
            }
        }
    }
}

void new_thread0_entry(void *pvParameters)
{
    Voice_Init();
    FSP_PARAMETER_NOT_USED(pvParameters);

    fsp_err_t err;

    g_voice_cmd_queue = xQueueCreate(VOICE_CMD_QUEUE_SIZE, sizeof(uint8_t));
    if (NULL == g_voice_cmd_queue)
    {
        while (1)
        {
            __NOP();
        }
    }

    err = g_ipc0.p_api->open(g_ipc0.p_ctrl, g_ipc0.p_cfg);
    if (FSP_SUCCESS != err)
    {
        while (1)
        {
            __NOP();
        }
    }

    xTaskCreate(VoiceIpcSenderTask, "VoiceSend", 256, NULL, 3, NULL);

    R_BSP_SecondaryCoreStart();

    vTaskDelay(pdMS_TO_TICKS(100));

    ipc_send_ping();

    while (1)
    {
        Voice_Process();

        // 状态保持模式：如果有待处理命令，先处理它
        if (g_pending_voice_cmd == VOICE_CMD_NONE)
        {
            // 没有待处理命令，获取新命令
            voice_cmd_t cmd = Voice_GetCommand();
            if (cmd != VOICE_CMD_NONE)
            {
                g_pending_voice_cmd = cmd;
            }
        }

        // 如果有待处理命令，发送到IPC队列
        if (g_pending_voice_cmd != VOICE_CMD_NONE)
        {
            uint8_t ipc_cmd = 0;

            switch (g_pending_voice_cmd)
            {
                case VOICE_CMD_START:
                    ipc_cmd = IPC_VOICE_START;
                    break;
                case VOICE_CMD_PILT_UP:
                    ipc_cmd = IPC_VOICE_PTZ_UP;
                    break;
                case VOICE_CMD_PILT_DOWN:
                    ipc_cmd = IPC_VOICE_PTZ_DOWN;
                    break;
                case VOICE_CMD_PILT_LEFT:
                    ipc_cmd = IPC_VOICE_PTZ_LEFT;
                    break;
                case VOICE_CMD_PILT_RIGHT:
                    ipc_cmd = IPC_VOICE_PTZ_RIGHT;
                    break;
                case VOICE_CMD_PILT_STOP:
                    ipc_cmd = IPC_VOICE_PTZ_STOP;
                    break;
                case VOICE_CMD_GEAR_RUN:
                    ipc_cmd = IPC_VOICE_GEAR_START;
                    break;
                case VOICE_CMD_GEAR_STOP:
                    ipc_cmd = IPC_VOICE_GEAR_STOP;
                    break;
                case VOICE_CMD_SEND_CLOUD:
                    ipc_cmd = IPC_VOICE_REPORT_DATA;
                    break;
                case VOICE_CMD_IDENTIFY_TAG:
                    ipc_cmd = IPC_VOICE_TAG_START;
                    break;
                case VOICE_CMD_FIND_CRACK:
                    ipc_cmd = IPC_VOICE_CRACK_START;
                    break;
                case VOICE_CMD_WAKEUP:
                default:
                    // 唤醒命令和未知命令直接清除
                    g_pending_voice_cmd = VOICE_CMD_NONE;
                    break;
            }

            // 发送到IPC发送队列
            if (g_pending_voice_cmd != VOICE_CMD_NONE)
            {
                if (xQueueSend(g_voice_cmd_queue, &ipc_cmd, 0) == pdPASS)
                {
                    // 发送到队列成功后，清除本地待处理标志
                    // (等待VoiceIpcSenderTask确认IPC发送成功后再清除g_pending_voice_cmd)
                    // 为避免队列堆积，这里直接清除
                    g_pending_voice_cmd = VOICE_CMD_NONE;
                }
                // 如果队列满，保持g_pending_voice_cmd，下次循环继续尝试
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
