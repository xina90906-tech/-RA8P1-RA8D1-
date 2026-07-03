/* generated thread header file - do not edit */
#ifndef IPC_THREAD_H_
#define IPC_THREAD_H_
#include "bsp_api.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "hal_data.h"
#ifdef __cplusplus
                extern "C" void IPC_thread_entry(void * pvParameters);
                #else
extern void IPC_thread_entry(void *pvParameters);
#endif
FSP_HEADER
FSP_FOOTER
#endif /* IPC_THREAD_H_ */
