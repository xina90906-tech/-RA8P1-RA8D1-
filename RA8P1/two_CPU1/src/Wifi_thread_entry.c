#include "Wifi_thread.h"
#include "hal_data.h"
#include "bsp_debug_uart.h"
#include "bsp_wifi_esp8266.h"
#include "ipc_msg_queue.h"
#include "cJSON.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include "FreeRTOS.h"
#include "task.h"

bool g_cloud_up = false;
bool g_cloud_down = false;
bool g_cloud_left = false;
bool g_cloud_right = false;

int g_mqtt_tab = 0;
int g_mqtt_crack = 0;
volatile bool g_mqtt_tab_changed = false;

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
                case IPC_VOICE_REPORT_DATA:
                    ESP8266_DEBUG_MSG("[Wifi] IPC命令: 上报数据\r\n");
                    MQTT_Report_Data();
                    break;

                case IPC_VOICE_TAG_START:
                    ESP8266_DEBUG_MSG("[Wifi] IPC命令: 开始识别标签\r\n");
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

void Wifi_thread_entry(void *pvParameters) {
	FSP_PARAMETER_NOT_USED(pvParameters);
	
	Debug_UART9_Init();
	ESP8266_UART6_Init();
	
	DataPoint_t data_points[] = {
			{ .identifier = "tab", .data_type = DATA_TYPE_INT8, .value.int_value = 0 },
			{ .identifier = "crack", .data_type = DATA_TYPE_INT8, .value.int_value = 0 },
			{ .identifier = "people", .data_type = DATA_TYPE_BOOL, .value.bool_value = false }
		};
	ESP8266_MQTT_Test(data_points, 3);
	bool mqtt_connected = true;
	bool topic_subscribed = false;

    QueueHandle_t ipc_queue = xQueueCreate(IPC_MSG_QUEUE_SIZE, sizeof(ipc_msg_t));
    if (ipc_queue != NULL)
    {
        IPC_Register_Queue(ipc_queue);
    }

	for (;;) {
		ipc_msg_t ipc_msg;
		while (xQueueReceive(ipc_queue, &ipc_msg, 0) == pdTRUE)
		{
			process_ipc_msg(&ipc_msg);
		}

		if (!mqtt_connected)
		{
			vTaskDelay(100);
			
			ESP8266_DEBUG_MSG("\r\n========== 尝试连接MQTT ==========\r\n");
			
			ESP8266_Rst();
			vTaskDelay(2000);
			
			ESP8266_STA();
			vTaskDelay(500);
			
			if (ESP8266_STA_JoinAP((char *)ID, (char *)PASSWORD, 5) == 0)
			{
				vTaskDelay(1000);
				
				ESP8266_MQTT_Test(data_points, sizeof(data_points) / sizeof(data_points[0]));
				
				if (Check_MQTT_Connection())
				{
					mqtt_connected = true;
					topic_subscribed = false;
					ESP8266_DEBUG_MSG("\r\n========== MQTT连接成功 ==========\r\n");
				}
				else
				{
					ESP8266_DEBUG_MSG("\r\n========== MQTT连接失败，稍后重试 ==========\r\n");
				}
			}
			else
			{
				ESP8266_DEBUG_MSG("\r\n========== WiFi连接失败，稍后重试 ==========\r\n");
			}
		}
		else
		{
			if (!topic_subscribed)
			{
				ESP8266_DEBUG_MSG("订阅主题: %s\r\n", MQTT_TOPIC_PROPERTY_SET);
				Subscribes_Topics((char *)MQTT_TOPIC_PROPERTY_SET);
				topic_subscribed = true;
			}

			char mqtt_rx_buffer[512];
			uint16_t rx_len = 0;

			if (Receive_MQTT_Message(mqtt_rx_buffer, sizeof(mqtt_rx_buffer), &rx_len))
			{
				ESP8266_DEBUG_MSG("MQTT接收: %s\r\n", mqtt_rx_buffer);

				cJSON *root = cJSON_Parse(mqtt_rx_buffer);
				if (root != NULL)
				{
					cJSON *params = cJSON_GetObjectItem(root, "params");
					if (params == NULL)
					{
						params = root;
					}

					cJSON *up_item = cJSON_GetObjectItem(params, "up");
					cJSON *down_item = cJSON_GetObjectItem(params, "down");
					cJSON *left_item = cJSON_GetObjectItem(params, "left");
					cJSON *right_item = cJSON_GetObjectItem(params, "right");

					if (up_item != NULL && (up_item->type == cJSON_True || up_item->type == cJSON_False))
					{
						g_cloud_up = (up_item->type == cJSON_True);
						ESP8266_DEBUG_MSG("up = %s\r\n", g_cloud_up ? "true" : "false");
					}
					if (down_item != NULL && (down_item->type == cJSON_True || down_item->type == cJSON_False))
					{
						g_cloud_down = (down_item->type == cJSON_True);
						ESP8266_DEBUG_MSG("down = %s\r\n", g_cloud_down ? "true" : "false");
					}
					if (left_item != NULL && (left_item->type == cJSON_True || left_item->type == cJSON_False))
					{
						g_cloud_left = (left_item->type == cJSON_True);
						ESP8266_DEBUG_MSG("left = %s\r\n", g_cloud_left ? "true" : "false");
					}
					if (right_item != NULL && (right_item->type == cJSON_True || right_item->type == cJSON_False))
					{
						g_cloud_right = (right_item->type == cJSON_True);
						ESP8266_DEBUG_MSG("right = %s\r\n", g_cloud_right ? "true" : "false");
					}

					cJSON_Delete(root);
				}

				Clear_Buff();
			}

			static uint32_t check_counter = 0;
			check_counter++;
			if (check_counter >= 50)
			{
				check_counter = 0;
				if (!Check_MQTT_Connection())
				{
					ESP8266_DEBUG_MSG("MQTT断联，尝试重连\r\n");
					mqtt_connected = false;
				}
			}
		}
		
		if (g_mqtt_tab_changed)
		{
			g_mqtt_tab_changed = false;
			MQTT_Report_Data();
		}
		
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

void MQTT_Report_Data(void)
{
	DataPoint_t data_points[] = {
		{ .identifier = "tab", .data_type = DATA_TYPE_INT8, .value.int_value = g_mqtt_tab },
		{ .identifier = "crack", .data_type = DATA_TYPE_INT8, .value.int_value = g_mqtt_crack },
		{ .identifier = "people", .data_type = DATA_TYPE_BOOL, .value.bool_value = true }
	};
	
	char mqtt_data[256];
	Generate_MQTT_Data_Multiple(mqtt_data, data_points, 3);
	
	ESP8266_DEBUG_MSG("上报数据: %s\r\n", mqtt_data);
	Send_Data((char *)MQTT_TOPIC_PROPERTY_POST, mqtt_data);
}
