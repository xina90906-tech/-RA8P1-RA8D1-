#ifndef IPC_MSG_QUEUE_H_
#define IPC_MSG_QUEUE_H_

#include "ipc_msg_common.h"
#include "FreeRTOS.h"
#include "queue.h"

#define IPC_MSG_QUEUE_SIZE    (16U)
#define MAX_IPC_QUEUES        (8U)

bool IPC_Register_Queue(QueueHandle_t queue);
bool IPC_Receive_Message(ipc_msg_t *p_msg, TickType_t timeout);

#endif /* IPC_MSG_QUEUE_H_ */
