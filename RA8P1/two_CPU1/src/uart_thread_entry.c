#include "uart_thread.h"
#include "uart.h"
#include "Wifi_thread.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ipc_msg_queue.h"
#include "buzzer.h"
#include "led.h"

static bool crack_detecting = false;
static bool beep_active = false;
static uint32_t beep_timer = 0;
static bool led_blink = false;
static bool led_state = false;
static uint32_t led_timer = 0;

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
                case IPC_VOICE_CRACK_START:
                    crack_detecting = true;
                    break;

                //case IPC_VOICE_CRACK_STOP:
                //    crack_detecting = false;
                //    break;

                default:
                    break;
            }
            break;
        }

        default:
            break;
    }
}

void uart_thread_entry(void *pvParameters) {
	FSP_PARAMETER_NOT_USED(pvParameters);

	Buzzer_Init();

	UART_Init();

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

		if (beep_active)
		{
			beep_timer++;
			if (beep_timer >= 100)
			{
				Buzzer_Off();
				beep_active = false;
				beep_timer = 0;
			}
		}

		if (led_blink)
		{
			led_timer++;
			if (led_timer >= 100)
			{
				if (led_state)
				{
					LED_Off();
					led_state = false;
				}
				else
				{
					LED_On();
					led_state = true;
				}
				led_timer = 0;
			}
		}

		if (crack_detecting)
		{
			UART0_Send_FF(1);
			if (UART_ProcessPacket())
			{
				if (!beep_active)
				{
					crack_detecting = false;
					Buzzer_On();
					beep_active = true;
					beep_timer = 0;
				}
				if (!led_blink)
				{
					led_blink = true;
					led_timer = 0;
					LED_On();
					led_state = true;
				}
			}
		}
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}
