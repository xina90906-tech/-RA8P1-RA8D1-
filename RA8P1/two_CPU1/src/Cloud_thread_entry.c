#include "Cloud_thread.h"
#include "GPIO.h"
#include "PULSE.h"
#include "Cloud.h"
#include "uart.h"
#include "buzzer.h"
#include "led.h"
#include "Wifi_thread.h"
#include "bsp_debug_uart.h"
#include "bsp_wifi_esp8266.h"
#include "ipc_msg_queue.h"

bool g_agt0_timeout_flag;
bool find_value;
bool pulse_reset_flag;
uint16_t pulse_accumulator;

void g_timer_AGT0_Callback(timer_callback_args_t * p_args)
{
	if(p_args->event == TIMER_EVENT_CYCLE_END){
		g_agt0_timeout_flag = true;
	}
}

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
                case IPC_VOICE_PTZ_UP:
                    g_cloud_up = true;
                    g_cloud_down = false;
                    g_cloud_left = false;
                    g_cloud_right = false;
                    break;

                case IPC_VOICE_PTZ_DOWN:
                    g_cloud_up = false;
                    g_cloud_down = true;
                    g_cloud_left = false;
                    g_cloud_right = false;
                    break;

                case IPC_VOICE_PTZ_LEFT:
                    g_cloud_up = false;
                    g_cloud_down = false;
                    g_cloud_left = true;
                    g_cloud_right = false;
                    break;

                case IPC_VOICE_PTZ_RIGHT:
                    g_cloud_up = false;
                    g_cloud_down = false;
                    g_cloud_left = false;
                    g_cloud_right = true;
                    break;

                case IPC_VOICE_PTZ_STOP:
                    g_cloud_up = false;
                    g_cloud_down = false;
                    g_cloud_left = false;
                    g_cloud_right = false;
                    break;

                case IPC_VOICE_CRACK_START:
                    find_value = true;
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

void Cloud_thread_entry(void *pvParameters) {
	FSP_PARAMETER_NOT_USED(pvParameters);

	PULSE_Init();
	GPIO_DirectionInit();
	Buzzer_Init();

    QueueHandle_t ipc_queue = xQueueCreate(IPC_MSG_QUEUE_SIZE, sizeof(ipc_msg_t));
    if (ipc_queue != NULL)
    {
        IPC_Register_Queue(ipc_queue);
    }

	while (1) {
		ipc_msg_t ipc_msg;
		while (xQueueReceive(ipc_queue, &ipc_msg, 0) == pdTRUE)
		{
			process_ipc_msg(&ipc_msg);
		}

		if (g_cloud_up)
		{
			synmove_to_point(0, 75, 300);
		}
		else if (g_cloud_down)
		{
			synmove_to_point(0, -75, 300);
		}
		else if (g_cloud_left)
		{
			synmove_to_point(-75, 0, 300);
		}
		else if (g_cloud_right)
		{
			synmove_to_point(75, 0, 300);
		}

		//synmove_to_point(500, 500, 200);
		vTaskDelay(300);
	}
}
