#include "bsp_debug_uart.h"
#include <bsp_wifi_esp8266.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "hal_data.h"
#include "cJSON.h"
#include "FreeRTOS.h"
#include "task.h"


_Bool               Uart6_Send_Flag = false;
_Bool               Uart6_Show_Flag = false;
extern bool people;

char                At_Rx_Buff[512];
uint16_t            Uart6_Num = 0;

void g_uart6_Callback(uart_callback_args_t * p_args)
{
    if (p_args->event == UART_EVENT_RX_CHAR)
    {
        if (Uart6_Num < sizeof(At_Rx_Buff) - 1)
        {
            At_Rx_Buff[Uart6_Num++] = (char)p_args->data;
            At_Rx_Buff[Uart6_Num] = '\0';
        }
    }
    else if (p_args->event == UART_EVENT_TX_COMPLETE)
    {
        Uart6_Send_Flag = true;
    }
}

int g_socket_id = 0;

void ESP8266_MQTT_Test(DataPoint_t *data_points, int count)
{
    ESP8266_DEBUG_MSG("\r\n=======================================================\r\n");
    ESP8266_DEBUG_MSG("                ESP-12F MQTT连接测试\r\n");
    ESP8266_DEBUG_MSG("=======================================================\r\n");

    ESP8266_DEBUG_MSG("\r\n[1/5] 初始化硬件...\r\n");
    ESP8266_DEBUG_MSG("  ESP-12F模块初始化...\r\n");
    vTaskDelay(1000);

    ESP8266_DEBUG_MSG("\r\n[2/5] 初始化ESP-12F模块...\r\n");

    Clear_Buff();
    ESP8266_AT_Send("AT\r\n");
    vTaskDelay(500);

    if (strstr(At_Rx_Buff, "OK"))
    {
        ESP8266_DEBUG_MSG("  ✅ ESP-12F模块响应正常\r\n");
    }
    else
    {
        ESP8266_DEBUG_MSG("  ⚠️ ESP-12F模块无响应，继续尝试...\r\n");
    }
    Clear_Buff();

    /* 检查 ESP8266 固件版本 - MQTT AT 指令需要 2.0.0 或更高版本 */
    ESP8266_DEBUG_MSG("  检查固件版本...\r\n");
    Clear_Buff();
    ESP8266_AT_Send("AT+GMR\r\n");
    vTaskDelay(1000);
    
    /* 打印固件版本信息 */
    if (strlen(At_Rx_Buff) > 0)
    {
        do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  固件版本信息: %s\r\n", At_Rx_Buff); Debug_Print(_dbg_buf); } while(0);
    }
    else
    {
        ESP8266_DEBUG_MSG("  ⚠️ 无法获取固件版本\r\n");
    }
    Clear_Buff();

    ESP8266_DEBUG_MSG("  执行软复位...\r\n");
    Clear_Buff();
    ESP8266_AT_Send("AT+RST\r\n");

    for (int i = 1; i <= 3; i++)
    {
        vTaskDelay(1000);
        do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  等待重启完成... (%d秒)\r\n", i); Debug_Print(_dbg_buf); } while(0);
    }

    Clear_Buff();
    ESP8266_AT_Send("AT\r\n");
    vTaskDelay(500);

    if (strstr(At_Rx_Buff, "OK"))
    {
        ESP8266_DEBUG_MSG("  ✅ ESP-12F模块响应正常\r\n");
    }
    else
    {
        ESP8266_DEBUG_MSG("  ❌ ESP-12F重启超时\r\n");
        //return;
    }
    Clear_Buff();

    ESP8266_DEBUG_MSG("\r\n[3/5] 配置网络模式...\r\n");
    ESP8266_STA();

    ESP8266_DEBUG_MSG("  关闭回显...\r\n");
    Clear_Buff();
    ESP8266_AT_Send("ATE0\r\n");
    vTaskDelay(500);
    Clear_Buff();

    ESP8266_DEBUG_MSG("  启用详细错误码...\r\n");
    Clear_Buff();
    ESP8266_AT_Send("AT+CMEE=1\r\n");
    vTaskDelay(500);
    Clear_Buff();

    ESP8266_DEBUG_MSG("\r\n[4/5] 连接WiFi网络...\r\n");
    do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  SSID: %s\r\n", ID); Debug_Print(_dbg_buf); } while(0);

    int wifi_result = ESP8266_STA_JoinAP((char *)ID, (char *)PASSWORD, 30);
    if (wifi_result == 0)
    {
        ESP8266_DEBUG_MSG("  ✅ WiFi连接成功\r\n");
    }
    else
    {
        do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  ❌ WiFi连接失败，错误代码: %d\r\n", wifi_result); Debug_Print(_dbg_buf); } while(0);
        return;
    }

    ESP8266_DEBUG_MSG("\r\n[5/5] 配置MQTT连接参数...\r\n");
    MQTT_SetUserProperty((char *)CLIENT_ID, (char *)USER_NAME, (char *)USER_PASSWORD);

    ESP8266_DEBUG_MSG("\r\n[6/6] 连接MQTT服务器...\r\n");
    bool mqtt_result = Connect_MQTT((char *)MQTT_IP, (char *)MQTT_Port, 255);
    if (mqtt_result)
    {
        ESP8266_DEBUG_MSG("  ✅ MQTT连接成功\r\n");

        ESP8266_DEBUG_MSG("\r\n[7/7] 订阅主题...\r\n");
        Subscribes_Topics((char *)MQTT_TOPIC_PROPERTY_POST_REPLY);
        ESP8266_DEBUG_MSG("  ✅ 主题订阅完成\r\n");

        ESP8266_DEBUG_MSG("\r\n[8/8] 发送属性上报数据...\r\n");
        char mqtt_data[1024];
        Generate_MQTT_Data_Multiple(mqtt_data, data_points, count);
        do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  生成的MQTT数据: %s\r\n", mqtt_data); Debug_Print(_dbg_buf); } while(0);
        Send_Data((char *)MQTT_TOPIC_PROPERTY_POST, mqtt_data);
        ESP8266_DEBUG_MSG("  ✅ 属性上报数据发送完成\r\n");

        ESP8266_DEBUG_MSG("  等待属性上报回复...\r\n");
        // 不要清空缓冲区，保留之前的数据供响应检查

        bool success_received = false;
        for (int i = 0; i < 20; i++)
        {
            if (Uart6_Num > 0)
            {
                if (strstr(At_Rx_Buff, "success"))
                {
                    ESP8266_DEBUG_MSG("  ✅ 收到属性上报成功响应\r\n");
                    success_received = true;
                    break;
                }
                else if (strstr(At_Rx_Buff, "error"))
                {
                    do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  ❌ 属性上报失败: %s\r\n", At_Rx_Buff); Debug_Print(_dbg_buf); } while(0);
                    break;
                }
            }
            vTaskDelay(200);
        }

        if (success_received)
        {
            ESP8266_DEBUG_MSG("  ✅ 属性上报成功\r\n");
        }
        else
        {
            ESP8266_DEBUG_MSG("  ⚠️ 未收到属性上报成功响应\r\n");
        }

        ESP8266_DEBUG_MSG("\r\n=======================================================\r\n");
        ESP8266_DEBUG_MSG("             ESP-12F MQTT连接测试完成\r\n");
        ESP8266_DEBUG_MSG("=======================================================\r\n");
    }
    else
    {
        ESP8266_DEBUG_MSG("  ❌ MQTT连接失败\r\n");
    }
}


void ESP8266_UART6_Init(void)
{
    fsp_err_t err = FSP_SUCCESS;
    err = R_SCI_B_UART_Open(&g_uart6_ctrl, &g_uart6_cfg);
    assert(FSP_SUCCESS == err);
}

void ESP8266_AT_Send(char * cmd )
{
    uint8_t len = (uint8_t)strlen(cmd);
    Uart6_Send_Flag = false;
    R_SCI_B_UART_Write(&g_uart6_ctrl, (uint8_t *)cmd, len );
    while(Uart6_Send_Flag == true);
}

void ESP8266_Rst(void)
{
    ESP8266_DEBUG_MSG("  软复位ESP-12F模块...\r\n");

    Clear_Buff();
    ESP8266_AT_Send("AT+RST\r\n");

    for (int i = 1; i <= 3; i++)
    {
        vTaskDelay(1000);
        do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  等待重启完成... (%d秒)\r\n", i); Debug_Print(_dbg_buf); } while(0);
    }

    Clear_Buff();
    ESP8266_AT_Send("AT\r\n");
    vTaskDelay(500);

    if (strstr(At_Rx_Buff, "OK"))
    {
        ESP8266_DEBUG_MSG("  ✅ ESP-12F模块响应正常\r\n");
        ESP8266_DEBUG_MSG("  ESP-12F已重启\r\n");
    }
    else
    {
        ESP8266_DEBUG_MSG("  ❌ ESP-12F重启超时\r\n");
    }
    Clear_Buff();
}

void ESP8266_STA ( void )
{
    uint8_t counter = 0;
    uint8_t timeout = 30;

    ESP8266_DEBUG_MSG("  设置STA模式...\r\n");

    do
    {
        Clear_Buff();
        ESP8266_AT_Send("AT+CWMODE=1\r\n");
        vTaskDelay(500);

        if (strstr(At_Rx_Buff, "OK"))
        {
            ESP8266_DEBUG_MSG("  ✅ ESP-12F已切换为STA模式\r\n");
            Clear_Buff();
            break;
        }
        if (strstr(At_Rx_Buff, "ERROR") || strstr(At_Rx_Buff, "ERR"))
        {
            ESP8266_DEBUG_MSG("\r\n设置STA模式失败\r\n");
            while(1);
        }
        if (counter >= timeout)
        {
            ESP8266_DEBUG_MSG("\r\n设置STA模式超时\r\n");
            while(1);
        }
        counter++;
        vTaskDelay(100);
    } while (1);

    ESP8266_Rst();
}

void ESP8266_AP ( void )
{
    uint8_t counter = 0;
    uint8_t timeout = 30;

    do
    {
        Clear_Buff();
        ESP8266_AT_Send("AT+CWMODE=2\r\n");
        vTaskDelay(500);

        if (strstr(At_Rx_Buff, "OK"))
        {
            ESP8266_DEBUG_MSG("\r\nESP8266已切换为AP模式\r\n");
            Clear_Buff();
            break;
        }
        if (strstr(At_Rx_Buff, "ERROR") || strstr(At_Rx_Buff, "ERR"))
        {
            ESP8266_DEBUG_MSG("\r\n设置AP模式失败\r\n");
            while(1);
        }
        if (counter >= timeout)
        {
            ESP8266_DEBUG_MSG("\r\n设置AP模式超时\r\n");
            while(1);
        }
        counter++;
        vTaskDelay(100);
    } while (1);

    ESP8266_Rst();
}

void ESP8266_STA_AP ( void )
{
    uint8_t counter = 0;
    uint8_t timeout = 30;

    do
    {
        Clear_Buff();
        ESP8266_AT_Send("AT+CWMODE=3\r\n");
        vTaskDelay(500);

        if (strstr(At_Rx_Buff, "OK"))
        {
            ESP8266_DEBUG_MSG("\r\nESP8266已切换为STA+AP模式\r\n");
            Clear_Buff();
            break;
        }
        if (strstr(At_Rx_Buff, "ERROR") || strstr(At_Rx_Buff, "ERR"))
        {
            ESP8266_DEBUG_MSG("\r\n设置STA+AP模式失败\r\n");
            while(1);
        }
        if (counter >= timeout)
        {
            ESP8266_DEBUG_MSG("\r\n设置STA+AP模式超时\r\n");
            while(1);
        }
        counter++;
        vTaskDelay(100);
    } while (1);

    ESP8266_Rst();
}

char *g_client_id = NULL;
char *g_user_name = NULL;
char *g_user_password = NULL;

void MQTT_SetUserProperty( char * client_id , char * user_name, char * user_password )
{
    g_client_id = client_id;
    g_user_name = user_name;
    g_user_password = user_password;

    ESP8266_DEBUG_MSG("  MQTT连接参数:\r\n");
    do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  Client ID: %s\r\n", g_client_id); Debug_Print(_dbg_buf); } while(0);
    do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  Username: %s\r\n", g_user_name); Debug_Print(_dbg_buf); } while(0);
    do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  Password length: %d\r\n", strlen(g_user_password)); Debug_Print(_dbg_buf); } while(0);
}

bool Connect_MQTT( char * mqtt_ip , char * mqtt_port , uint8_t timeout )
{
    (void)timeout;

    ESP8266_DEBUG_MSG("  === 尝试连接MQTT服务器 ===\r\n");

    ESP8266_DEBUG_MSG("  关闭现有MQTT连接...\r\n");
    Clear_Buff();
    ESP8266_AT_Send("AT+MQTTCLEAN=0\r\n");
    vTaskDelay(1000);
    Clear_Buff();

    ESP8266_DEBUG_MSG("  检查MQTT指令支持...\r\n");
    Clear_Buff();
    ESP8266_AT_Send("AT+MQTTUSERCFG=?\r\n");
    vTaskDelay(1000);
    
    if (strlen(At_Rx_Buff) > 0 && strstr(At_Rx_Buff, "+MQTTUSERCFG:"))
    {
        ESP8266_DEBUG_MSG("  ✅ MQTT指令支持\r\n");
    }
    else
    {
        do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  ❌ MQTT指令不支持，响应: %s\r\n", At_Rx_Buff); Debug_Print(_dbg_buf); } while(0);
        ESP8266_DEBUG_MSG("  ⚠️ 需要升级ESP8266固件或改用TCP直连方式\r\n");
    }
    Clear_Buff();

    ESP8266_DEBUG_MSG("  配置MQTT用户属性...\r\n");
    
    ESP8266_DEBUG_MSG("  配置MQTT基础属性...\r\n");
    char usercfg_cmd[128];
    sprintf(usercfg_cmd, "AT+MQTTUSERCFG=0,1,\"%s\",\"\",\"\",0,0,\"\"\r\n", g_client_id);
    do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  命令: %s\r\n", usercfg_cmd); Debug_Print(_dbg_buf); } while(0);
    
    Clear_Buff();
    ESP8266_AT_Send(usercfg_cmd);
    vTaskDelay(1000);
    
    if (strstr(At_Rx_Buff, "OK"))
    {
        ESP8266_DEBUG_MSG("  ✅ MQTT基础属性配置成功\r\n");
    }
    else
    {
        do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  ❌ MQTT基础属性配置失败: %s\r\n", At_Rx_Buff); Debug_Print(_dbg_buf); } while(0);
        return false;
    }
    Clear_Buff();
    
    ESP8266_DEBUG_MSG("  配置MQTT用户名...\r\n");
    char username_cmd[128];
    sprintf(username_cmd, "AT+MQTTUSERNAME=0,\"%s\"\r\n", g_user_name);
    do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  命令: %s\r\n", username_cmd); Debug_Print(_dbg_buf); } while(0);
    
    Clear_Buff();
    ESP8266_AT_Send(username_cmd);
    vTaskDelay(1000);
    
    if (strstr(At_Rx_Buff, "OK"))
    {
        ESP8266_DEBUG_MSG("  ✅ 用户名配置成功\r\n");
    }
    else
    {
        do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  ❌ 用户名配置失败: %s\r\n", At_Rx_Buff); Debug_Print(_dbg_buf); } while(0);
        return false;
    }
    Clear_Buff();
    
    ESP8266_DEBUG_MSG("  配置MQTT密码...\r\n");
    do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  密码长度: %d\r\n", strlen(g_user_password)); Debug_Print(_dbg_buf); } while(0);
    
    char password_cmd[512];
    sprintf(password_cmd, "AT+MQTTPASSWORD=0,\"%s\"\r\n", g_user_password);
    do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  命令: %s\r\n", password_cmd); Debug_Print(_dbg_buf); } while(0);
    
    Clear_Buff();
    ESP8266_AT_Send(password_cmd);
    vTaskDelay(2000);
    
    if (strstr(At_Rx_Buff, "OK"))
    {
        ESP8266_DEBUG_MSG("  ✅ 密码配置成功\r\n");
    }
    else
    {
        do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  ❌ 密码配置失败: %s\r\n", At_Rx_Buff); Debug_Print(_dbg_buf); } while(0);
        return false;
    }
    Clear_Buff();

    ESP8266_DEBUG_MSG("  配置MQTT连接属性...\r\n");
    ESP8266_AT_Send("AT+MQTTCONNCFG=0,60,0,\"\",\"\",0,0\r\n");
    vTaskDelay(1000);

    if (strstr(At_Rx_Buff, "OK"))
    {
        ESP8266_DEBUG_MSG("  ✅ MQTT连接属性配置成功\r\n");
    }
    else
    {
        do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  ❌ MQTT连接属性配置失败: %s\r\n", At_Rx_Buff); Debug_Print(_dbg_buf); } while(0);
        return false;
    }
    Clear_Buff();

    do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  连接MQTT服务器: %s:%s\r\n", mqtt_ip, mqtt_port); Debug_Print(_dbg_buf); } while(0);
    char conn_cmd[128];
    sprintf(conn_cmd, "AT+MQTTCONN=0,\"%s\",%s,0\r\n", mqtt_ip, mqtt_port);
    do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  命令: %s\r\n", conn_cmd); Debug_Print(_dbg_buf); } while(0);

    Clear_Buff();
    ESP8266_AT_Send(conn_cmd);

    uint32_t conn_timeout = 15000;
    while (conn_timeout > 0)
    {
        if (strstr(At_Rx_Buff, "+MQTTCONNECTED"))
        {
            ESP8266_DEBUG_MSG("  ✅ MQTT连接成功\r\n");
            g_socket_id = 1;
            return true;
        }
        if (strstr(At_Rx_Buff, "+MQTTDISCONNECTED"))
        {
            do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  ❌ MQTT连接失败: %s\r\n", At_Rx_Buff); Debug_Print(_dbg_buf); } while(0);
            return false;
        }
        if (strstr(At_Rx_Buff, "ERROR"))
        {
            do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  ❌ MQTT连接命令失败: %s\r\n", At_Rx_Buff); Debug_Print(_dbg_buf); } while(0);
            return false;
        }
        vTaskDelay(100);
        conn_timeout -= 100;
    }

    ESP8266_DEBUG_MSG("  ❌ MQTT连接超时\r\n");
    return false;
}

void Subscribes_Topics( char * topics )
{
    if (g_socket_id == 0)
    {
        ESP8266_DEBUG_MSG("❌ 无有效MQTT连接，无法订阅主题\r\n");
        return;
    }

    do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  订阅主题: %s\r\n", topics); Debug_Print(_dbg_buf); } while(0);
    char sub_cmd[256];
    sprintf(sub_cmd, "AT+MQTTSUB=0,\"%s\",0\r\n", topics);
    do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  命令: %s\r\n", sub_cmd); Debug_Print(_dbg_buf); } while(0);

    Clear_Buff();
    ESP8266_AT_Send(sub_cmd);
    vTaskDelay(2000);

    if (strstr(At_Rx_Buff, "OK"))
    {
        ESP8266_DEBUG_MSG("  ✅ 订阅主题成功\r\n");
    }
    else if (strstr(At_Rx_Buff, "+MQTTSUB:OK"))
    {
        ESP8266_DEBUG_MSG("  ✅ 订阅主题成功\r\n");
    }
    else
    {
        do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  ❌ 订阅主题失败: %s\r\n", At_Rx_Buff); Debug_Print(_dbg_buf); } while(0);
    }

    Clear_Buff();
}

void Send_Data( char * topics , char * data )
{
    if (g_socket_id == 0)
    {
        ESP8266_DEBUG_MSG("❌ 无有效MQTT连接，无法发送数据\r\n");
        return;
    }

    uint16_t data_len = (uint16_t)strlen(data);

    do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  发送数据到主题: %s\r\n", topics); Debug_Print(_dbg_buf); } while(0);
    do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  数据长度: %d字节，使用AT+MQTTPUBRAW\r\n", data_len); Debug_Print(_dbg_buf); } while(0);

    char pubraw_cmd[256];
    sprintf(pubraw_cmd, "AT+MQTTPUBRAW=0,\"%s\",%d,0,0\r\n", topics, data_len);
    
    do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  命令: %s\r\n", pubraw_cmd); Debug_Print(_dbg_buf); } while(0);

    Clear_Buff();
    ESP8266_AT_Send(pubraw_cmd);
    vTaskDelay(1000);

    if (strstr(At_Rx_Buff, ">"))
    {
        ESP8266_DEBUG_MSG("  AT准备接收数据，发送原始数据...\r\n");
        
        Clear_Buff();
        ESP8266_AT_Send(data);
        
        uint32_t pub_timeout = 5000;
        while (pub_timeout > 0)
        {
            if (strstr(At_Rx_Buff, "+MQTTPUB:OK"))
            {
                ESP8266_DEBUG_MSG("  ✅ 数据发送成功\r\n");
                break;
            }
            else if (strstr(At_Rx_Buff, "+MQTTPUB:FAIL"))
            {
                ESP8266_DEBUG_MSG("  ❌ 数据发送失败\r\n");
                break;
            }
            vTaskDelay(100);
            pub_timeout -= 100;
        }
        
        if (pub_timeout == 0)
        {
            ESP8266_DEBUG_MSG("  ⚠️ 数据发送超时\r\n");
        }
    }
    else
    {
        do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  ❌ 未收到准备接收数据响应: %s\r\n", At_Rx_Buff); Debug_Print(_dbg_buf); } while(0);
    }
}

void Generate_MQTT_Data(char *buffer, const char *identifier, int value)
{
    sprintf(buffer, "{\"id\":\"123\",\"version\":\"1.0\",\"params\":{\"%s\":{\"value\":%d}}}", identifier, value);
}

void Generate_MQTT_Data_Bool(char *buffer, const char *identifier, bool value)
{
    sprintf(buffer, "{\"id\":\"123\",\"version\":\"1.0\",\"params\":{\"%s\":{\"value\":%s}}}", identifier, value ? "true" : "false");
}

void Generate_MQTT_Data_Multiple(char *buffer, DataPoint_t *data_points, int count)
{
    int pos = 0;
    pos += sprintf(buffer + pos, "{\"id\":\"123\",\"version\":\"1.0\",\"params\":{");

    for (int i = 0; i < count; i++)
    {
        if (i > 0)
        {
            pos += sprintf(buffer + pos, ",");
        }

        pos += sprintf(buffer + pos, "\"%s\":{", data_points[i].identifier);

        if (data_points[i].data_type == DATA_TYPE_INT8)
        {
            pos += sprintf(buffer + pos, "\"value\":%d", data_points[i].value.int_value);
        }
        else if (data_points[i].data_type == DATA_TYPE_BOOL)
        {
            pos += sprintf(buffer + pos, "\"value\":%s", data_points[i].value.bool_value ? "true" : "false");
        }

        pos += sprintf(buffer + pos, "}");
    }

    pos += sprintf(buffer + pos, "}}");
}

int ESP8266_STA_JoinAP( char * id ,  char * password , uint8_t timeout )
{
    int ret = -1;

    ESP8266_DEBUG_MSG("  开始ESP-12F WIFI连接流程...\r\n");

    Clear_Buff();
    ESP8266_AT_Send("AT+CWMODE=1\r\n");
    vTaskDelay(500);
    Clear_Buff();

    Clear_Buff();
    ESP8266_AT_Send("AT+CWDHCP=1,1\r\n");
    vTaskDelay(500);
    Clear_Buff();

    Clear_Buff();
    ESP8266_AT_Send("AT+CWQAP\r\n");
    vTaskDelay(1000);
    Clear_Buff();

    do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  连接AP: %s\r\n", id); Debug_Print(_dbg_buf); } while(0);
    char connect_cmd[128];
    sprintf(connect_cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", id, password);

    uint8_t counter = 0;
    while (counter < timeout)
    {
        Clear_Buff();
        ESP8266_AT_Send(connect_cmd);
        vTaskDelay(3000);

        if (strstr(At_Rx_Buff, "WIFI GOT IP"))
        {
            ESP8266_DEBUG_MSG("  ✅ WiFi连接成功，获取到IP地址\r\n");
            ret = 0;
            break;
        }
        else if (strstr(At_Rx_Buff, "OK"))
        {
            ESP8266_DEBUG_MSG("  ✅ WiFi连接成功\r\n");
            ret = 0;
            break;
        }
        else if (strstr(At_Rx_Buff, "FAIL"))
        {
            ESP8266_DEBUG_MSG("  ❌ WiFi连接失败: 密码错误或超时\r\n");
            break;
        }
        else if (strstr(At_Rx_Buff, "ERROR"))
        {
            do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  ❌ WiFi连接失败: %s\r\n", At_Rx_Buff); Debug_Print(_dbg_buf); } while(0);
            break;
        }

        counter++;
        do { char _dbg_buf[256]; snprintf(_dbg_buf, sizeof(_dbg_buf), "  连接中... (%d/%d)\r\n", counter, timeout); Debug_Print(_dbg_buf); } while(0);
    }

    if (counter >= timeout && ret != 0)
    {
        ESP8266_DEBUG_MSG("  ❌ WiFi连接超时\r\n");
    }

    Clear_Buff();
    ESP8266_AT_Send("AT+CWJAP?\r\n");
    vTaskDelay(1000);

    if (strstr(At_Rx_Buff, "No AP"))
    {
        ESP8266_DEBUG_MSG("  ❌ 未连接到任何AP\r\n");
        ret = -1;
    }
    else if (strstr(At_Rx_Buff, "+CWJAP:"))
    {
        ESP8266_DEBUG_MSG("  ✅ 已连接到AP\r\n");
        if (ret != 0) ret = 0;
    }

    return ret;
}

bool Check_MQTT_Connection(void)
{
    Clear_Buff();
    ESP8266_AT_Send("AT+MQTTCONN?\r\n");
    vTaskDelay(1000);

    if (strstr(At_Rx_Buff, "+MQTTCONN:"))
    {
        if (strstr(At_Rx_Buff, ",4,") || strstr(At_Rx_Buff, ",5,") || strstr(At_Rx_Buff, ",6,"))
        {
            ESP8266_DEBUG_MSG("  ✅ MQTT已连接\r\n");
            return true;
        }
        else if (strstr(At_Rx_Buff, ",0,") || strstr(At_Rx_Buff, ",1,") || strstr(At_Rx_Buff, ",2,") || strstr(At_Rx_Buff, ",3,"))
        {
            ESP8266_DEBUG_MSG("  ❌ MQTT未连接或连接已断开\r\n");
            return false;
        }
    }

    ESP8266_DEBUG_MSG("  ⚠️ MQTT状态未知\r\n");
    return false;
}

/**
  * 函    数：接收MQTT消息
  * 参    数：buffer 接收缓冲区
  * 参    数：max_size 缓冲区最大长度
  * 参    数：actual_size 实际接收长度
  * 返 回 值：true 接收到有效消息，false 无消息
  * 说    明：检查UART6缓冲区中是否有MQTT订阅消息（+MQTTSUBRECV:）
  */
bool Receive_MQTT_Message(char *buffer, uint16_t max_size, uint16_t *actual_size)
{
    if (buffer == NULL || max_size == 0)
    {
        ESP8266_DEBUG_MSG("[Receive_MQTT] 参数错误\r\n");
        return false;
    }

    if (Uart6_Num == 0)
    {
        ESP8266_DEBUG_MSG("[Receive_MQTT] 缓冲区为空\r\n");
        return false;
    }

    ESP8266_DEBUG_MSG("[Receive_MQTT] 缓冲区有%d字节数据\r\n", Uart6_Num);

    // 查找 +MQTTSUBRECV: 标志
    char *recv_start = strstr(At_Rx_Buff, "+MQTTSUBRECV:");
    if (recv_start == NULL)
    {
        ESP8266_DEBUG_MSG("[Receive_MQTT] 未找到+MQTTSUBRECV: 标志\r\n");
        return false;
    }

    ESP8266_DEBUG_MSG("[Receive_MQTT] ✅ 找到+MQTTSUBRECV: 标志\r\n");

    // 跳过 "+MQTTSUBRECV:" 和 LinkID、topic、length 字段
    // 格式: +MQTTSUBRECV:<LinkID>,<"topic">,<data_length>,data
    // 跳过第一个 ',' 后的 LinkID
    char *p = strchr(recv_start, ',');
    if (p == NULL) return false;
    p++;

    // 跳过 topic（被双引号包围）
    if (*p != '"') return false;
    p++;
    char *topic_end = strchr(p, '"');
    if (topic_end == NULL) return false;
    p = topic_end + 1;

    // 跳过 topic 后的 ','
    if (*p != ',') return false;
    p++;

    // 解析 data_length
    int data_length = 0;
    while (*p >= '0' && *p <= '9')
    {
        data_length = data_length * 10 + (*p - '0');
        p++;
    }

    // 跳过 length 后的 ','
    if (*p != ',') return false;
    p++;

    // 拷贝数据
    if (data_length <= 0 || (uint16_t)data_length >= max_size)
    {
        // 数据长度异常，清空缓冲区
        Clear_Buff();
        return false;
    }

    memcpy(buffer, p, data_length);
    buffer[data_length] = '\0';

    if (actual_size != NULL)
    {
        *actual_size = (uint16_t)data_length;
    }

    // 清空缓冲区
    Clear_Buff();

    return true;
}
