#include "hal_data.h"
#include "bsp_debug_uart.h"
#include "bsp_wifi_esp8266.h"
#include "ipc_msg_queue.h"
#include "ipc_msg_common.h"

static QueueHandle_t g_ipc_queues[MAX_IPC_QUEUES] = {NULL};
static uint32_t g_queue_count = 0U;

static uint32_t g_recv_count = 0U;

bool IPC_Register_Queue(QueueHandle_t queue)
{
    if (queue == NULL || g_queue_count >= MAX_IPC_QUEUES)
    {
        return false;
    }

    for (uint32_t i = 0; i < g_queue_count; i++)
    {
        if (g_ipc_queues[i] == queue)
        {
            return true;
        }
    }

    g_ipc_queues[g_queue_count++] = queue;
    return true;
}

bool IPC_Receive_Message(ipc_msg_t *p_msg, TickType_t timeout)
{
    if (p_msg == NULL)
    {
        return false;
    }

    QueueHandle_t queue = xQueueCreate(IPC_MSG_QUEUE_SIZE, sizeof(ipc_msg_t));
    if (queue == NULL)
    {
        return false;
    }

    if (!IPC_Register_Queue(queue))
    {
        vQueueDelete(queue);
        return false;
    }

    bool result = (xQueueReceive(queue, p_msg, timeout) == pdPASS);

    return result;
}

void g_ipc0_Callback(ipc_callback_args_t *p_args)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (IPC_EVENT_MESSAGE_RECEIVED == p_args->event)
    {
        ipc_msg_t msg;
        msg.word = p_args->message;

        for (uint32_t i = 0; i < g_queue_count; i++)
        {
            if (NULL != g_ipc_queues[i])
            {
                xQueueSendFromISR(g_ipc_queues[i], &msg, &xHigherPriorityTaskWoken);
            }
        }
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void handle_ping_msg(ipc_msg_t *p_msg)
{
    uint16_t count = ((uint16_t)p_msg->parts.param1 << 8) | p_msg->parts.param2;
    g_recv_count++;
    (void)count;
}

static void handle_led_ctrl_msg(ipc_msg_t *p_msg)
{
    uint8_t led_index = p_msg->parts.param1;
    uint8_t cmd       = p_msg->parts.param2;

    switch (cmd)
    {
        case IPC_LED_ON:
            break;

        case IPC_LED_OFF:
            break;

        case IPC_LED_TOGGLE:
            break;

        default:
            break;
    }
    (void)led_index;
}

static void handle_voice_cmd(ipc_msg_t *p_msg)
{
    uint8_t cmd = p_msg->parts.param1;
    ESP8266_DEBUG_MSG("[IPC] 收到语音命令: cmd=0x%02X\r\n", cmd);
}

static void ipc_msg_dispatch(ipc_msg_t *p_msg)
{
    switch (p_msg->parts.msg_id)
    {
        case IPC_MSG_ID_PING:
            handle_ping_msg(p_msg);
            break;

        case IPC_MSG_ID_LED_CTRL:
            handle_led_ctrl_msg(p_msg);
            break;

        case IPC_MSG_ID_VOICE_CMD:
            handle_voice_cmd(p_msg);
            break;

        case IPC_MSG_ID_DATA_SYNC:
            break;

        case IPC_MSG_ID_STATUS_GET:
            break;

        default:
            break;
    }
}

void IPC_thread_entry(void *pvParameters)
{
    FSP_PARAMETER_NOT_USED(pvParameters);

    fsp_err_t err;
    ipc_msg_t msg;

    err = g_ipc0.p_api->open(g_ipc0.p_ctrl, g_ipc0.p_cfg);
    if (FSP_SUCCESS != err)
    {
        while (1)
        {
            vTaskDelay(portMAX_DELAY);
        }
    }

    QueueHandle_t queue = xQueueCreate(IPC_MSG_QUEUE_SIZE, sizeof(ipc_msg_t));
    if (queue != NULL)
    {
        IPC_Register_Queue(queue);
    }

    while (1)
    {
        if (pdPASS == xQueueReceive(queue, &msg, portMAX_DELAY))
        {
            ipc_msg_dispatch(&msg);
        }
    }
}
