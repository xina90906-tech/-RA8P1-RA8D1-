#include "servo_thread.h"
#include "servo.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ipc_msg_queue.h"


static void process_ipc_msg(ipc_msg_t *p_msg)
{
    if (p_msg == NULL) return;

    switch (p_msg->parts.msg_id)
    {
        case IPC_MSG_ID_VOICE_CMD:
        {
            uint8_t cmd = p_msg->parts.param1;
            switch (cmd)
            {
                case IPC_VOICE_START:
                    //__asm volatile ("nop");
                    servo_start();
                    break;

                case IPC_VOICE_GEAR_START:
                    //__asm volatile ("nop");
                    servo_start();
                    break;

                case IPC_VOICE_GEAR_STOP:
                    servo_stop();
                    break;

                default:
                    break;
            }
            break;
        }

        default:
            break;
    }
}

void servo_thread_entry(void *pvParameters)
{
    FSP_PARAMETER_NOT_USED(pvParameters);

    servo_init();

    QueueHandle_t ipc_queue = xQueueCreate(IPC_MSG_QUEUE_SIZE, sizeof(ipc_msg_t));
    if (ipc_queue != NULL)
    {
        IPC_Register_Queue(ipc_queue);
    }

    while (1)
    {
        ipc_msg_t ipc_msg;
        while (xQueueReceive(ipc_queue, &ipc_msg, 0) == pdTRUE)
        {
            process_ipc_msg(&ipc_msg);
        }

        uint32_t notify_value = 0;
        BaseType_t result = xTaskNotifyWait(
            pdFALSE,
            0xFFFFFFFF,
            &notify_value,
            pdMS_TO_TICKS(10)
        );

        if (result == pdTRUE)
        {
            if (notify_value == SERVO_CMD_START)
            {
                servo_start();
            }
            else if (notify_value == SERVO_CMD_STOP)
            {
                servo_stop();
            }
        }

        servo_process();
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
