#ifndef __BSP_WIFI_ESP8266_H
#define __BSP_WIFI_ESP8266_H
#include "hal_data.h"
#include "cJSON.h"

/*宏定义调试信息*/
#define ESP8266_DEBUG   0

/* 使用 Debug_Printf 进行调试输出，支持 %d, %s 等格式化 */
#if     (ESP8266_DEBUG == 1)
#define     ESP8266_DEBUG_MSG(fmt, ... )        Debug_Printf ( fmt, ##__VA_ARGS__ )
#else
#define     ESP8266_DEBUG_MSG(fmt, ... )
#endif

#define   ID             "vivo S12"           //要连接的热点的名称
#define   PASSWORD       "123456789"           //要连接的热点的密钥

/*#define   CLIENT_ID      "d1"              //MQTT用户ID
#define   USER_NAME      "8o9v2bp2n7"             //MQTT用户名称
#define   USER_PASSWORD  "version=2018-10-31&res=products%2F8o9v2bp2n7%2Fdevices%2Fd1&et=1832067619&method=md5&sign=HgQSz%2BI4cw3EgcMdeFQjvA%3D%3D"             //MQTT用户密码

#define   MQTT_IP        "mqtts.heclouds.com"     //要连接的MQTT服务器IP
#define   MQTT_Port      "1883"               //要连接的MQTT服务器端口 (使用TCP)
*/
#define   CLIENT_ID      "d2"              //MQTT用户ID
#define   USER_NAME      "BOPTVcAZTj"             //MQTT用户名称
#define   USER_PASSWORD  "version=2018-10-31&res=products%2FBOPTVcAZTj%2Fdevices%2Fd2&et=1832067619&method=md5&sign=6D2H%2BJpnlgVpgbyvzUo%2Bjw%3D%3D"             //MQTT用户密码

#define   MQTT_IP        "mqtts.heclouds.com"     //要连接的MQTT服务器IP
#define   MQTT_Port      "1883"               //要连接的MQTT服务器端口 (使用TCP)
// OneNet标准通信主题 - 严格符合文档要求
/*#define   MQTT_TOPIC_PROPERTY_POST                "$sys/8o9v2bp2n7/d1/thing/property/post"             //设备属性上报请求（发布）
#define   MQTT_TOPIC_PROPERTY_POST_REPLY          "$sys/8o9v2bp2n7/d1/thing/property/post/reply"       //设备属性上报响应（订阅）
#define   MQTT_TOPIC_PROPERTY_SET                 "$sys/8o9v2bp2n7/d1/thing/property/set"              //设备属性设置请求（订阅）
#define   MQTT_TOPIC_PROPERTY_SET_REPLY           "$sys/8o9v2bp2n7/d1/thing/property/set_reply"         //设备属性设置响应（发布）
#define   MQTT_TOPIC_PROPERTY_DESIRED_GET          "$sys/8o9v2bp2n7/d1/thing/property/desired/get"      //设备获取属性期望值请求（发布）
#define   MQTT_TOPIC_PROPERTY_DESIRED_GET_REPLY    "$sys/8o9v2bp2n7/d1/thing/property/desired/get/reply"  //设备获取属性期望值响应（订阅）
#define   MQTT_TOPIC_PROPERTY_DESIRED_DELETE       "$sys/8o9v2bp2n7/d1/thing/property/desired/delete"   //设备清除属性期望值请求（发布）
#define   MQTT_TOPIC_PROPERTY_DESIRED_DELETE_REPLY "$sys/8o9v2bp2n7/d1/thing/property/desired/delete/reply" //设备清除属性期望值响应（订阅）
#define   MQTT_TOPIC_PROPERTY_GET                 "$sys/8o9v2bp2n7/d1/thing/property/get"              //设备属性获取请求（订阅）
#define   MQTT_TOPIC_PROPERTY_GET_REPLY           "$sys/8o9v2bp2n7/d1/thing/property/get_reply"         //设备属性获取响应（发布）
#define   MQTT_TOPIC_EVENT_POST                   "$sys/8o9v2bp2n7/d1/thing/event/post"                //设备事件上报请求（发布）
#define   MQTT_TOPIC_EVENT_POST_REPLY             "$sys/8o9v2bp2n7/d1/thing/event/post/reply"          //设备事件上报响应（订阅）
#define   TOPICS_DATA                             "{\"id\":\"123\",\"version\":\"1.0\",\"params\":{\"temp\":{\"value\":24}}}"    //要发送的数据 (符合OneNet JSON格式)
*/
#define   MQTT_TOPIC_PROPERTY_POST                "$sys/BOPTVcAZTj/d2/thing/property/post"             //设备属性上报请求（发布）
#define   MQTT_TOPIC_PROPERTY_POST_REPLY          "$sys/BOPTVcAZTj/d2/thing/property/post/reply"       //设备属性上报响应（订阅）
#define   MQTT_TOPIC_PROPERTY_SET                 "$sys/BOPTVcAZTj/d2/thing/property/set"              //设备属性设置请求（订阅）
#define   MQTT_TOPIC_PROPERTY_SET_REPLY           "$sys/BOPTVcAZTj/d2/thing/property/set_reply"         //设备属性设置响应（发布）
#define   MQTT_TOPIC_PROPERTY_DESIRED_GET          "$sys/BOPTVcAZTj/d2/thing/property/desired/get"      //设备获取属性期望值请求（发布）
#define   MQTT_TOPIC_PROPERTY_DESIRED_GET_REPLY    "$sys/BOPTVcAZTj/d2/thing/property/desired/get/reply"  //设备获取属性期望值响应（订阅）
#define   MQTT_TOPIC_PROPERTY_DESIRED_DELETE       "$sys/BOPTVcAZTj/d2/thing/property/desired/delete"   //设备清除属性期望值请求（发布）
#define   MQTT_TOPIC_PROPERTY_DESIRED_DELETE_REPLY "$sys/BOPTVcAZTj/d2/thing/property/desired/delete/reply" //设备清除属性期望值响应（订阅）
#define   MQTT_TOPIC_PROPERTY_GET                 "$sys/BOPTVcAZTj/d2/thing/property/get"              //设备属性获取请求（订阅）
#define   MQTT_TOPIC_PROPERTY_GET_REPLY           "$sys/BOPTVcAZTj/d2/thing/property/get_reply"         //设备属性获取响应（发布）
#define   MQTT_TOPIC_EVENT_POST                   "$sys/BOPTVcAZTj/d2/thing/event/post"                //设备事件上报请求（发布）
#define   MQTT_TOPIC_EVENT_POST_REPLY             "$sys/BOPTVcAZTj/d2/thing/event/post/reply"          //设备事件上报响应（订阅）
#define   TOPICS_DATA                             "{\"id\":\"123\",\"version\":\"1.0\",\"params\":{\"temp\":{\"value\":24}}}"    //要发送的数据 (符合OneNet JSON格式)

// 数据类型定义
typedef enum {
    DATA_TYPE_INT8,
    DATA_TYPE_BOOL
} DataType_t;

// 数据点结构体
typedef struct {
    char *identifier;
    DataType_t data_type;
    union {
        int int_value;
        bool bool_value;
    } value;
} DataPoint_t;


/*清除UART6数据缓冲区函数*/
extern char At_Rx_Buff[512];
extern uint16_t Uart6_Num;
#define   Clear_Buff()   memset( At_Rx_Buff , 0 , sizeof(At_Rx_Buff) ); \
                         Uart6_Num = 0;


void ESP8266_MQTT_Test(DataPoint_t *data_points, int count);
void ESP8266_UART6_Init(void);
void ESP8266_AT_Send(char * cmd );
void ESP8266_Rst(void);
void ESP8266_STA ( void );
void ESP8266_AP ( void );
void ESP8266_STA_AP ( void );
int ESP8266_STA_JoinAP( char * id ,  char * password , uint8_t timeout );
void MQTT_SetUserProperty( char * client_id , char * user_name, char * user_password );
bool Connect_MQTT( char * mqtt_ip , char * mqtt_port , uint8_t timeout );
void Subscribes_Topics( char * topics );
void Send_Data( char * topics , char * data );
void Generate_MQTT_Data(char *buffer, const char *identifier, int value);
void Generate_MQTT_Data_Bool(char *buffer, const char *identifier, bool value);
void Generate_MQTT_Data_Multiple(char *buffer, DataPoint_t *data_points, int count);
bool Subscribe_Property_Set_Topic(void);
bool Receive_MQTT_Message(char *buffer, uint16_t max_size, uint16_t *actual_size);
bool Parse_Find_Value(const char *json_data, bool *find_value);
bool Monitor_MQTT_Find_Value(bool *find_value);
void Send_People_Status(void);
bool MQTT_Send_Property_Data(DataPoint_t *data_points, int count);
// 简化版本，用于定时器中断回调
bool Monitor_MQTT_Find_Value_Simple(bool *find_value);
bool Receive_MQTT_Message_Simple(char *buffer, uint16_t max_size, uint16_t *actual_size);
bool Check_MQTT_Connection(void);

#endif
