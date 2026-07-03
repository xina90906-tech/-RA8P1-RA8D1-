#ifndef __WIFI_THREAD_H__
#define __WIFI_THREAD_H__
#include "hal_data.h"

extern bool g_cloud_up;
extern bool g_cloud_down;
extern bool g_cloud_left;
extern bool g_cloud_right;

extern int g_mqtt_tab;
extern int g_mqtt_crack;
extern volatile bool g_mqtt_tab_changed;

void MQTT_Report_Data(void);

#endif