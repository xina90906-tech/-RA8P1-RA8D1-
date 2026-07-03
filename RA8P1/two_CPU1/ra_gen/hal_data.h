/* generated HAL header file - do not edit */
#ifndef HAL_DATA_H_
#define HAL_DATA_H_
#include <stdint.h>
#include "bsp_api.h"
#include "common_data.h"
#include "r_gpt.h"
#include "r_timer_api.h"
#include "r_ipc.h"
#include "r_agt.h"
#include "r_timer_api.h"
#include "r_sci_b_uart.h"
#include "r_uart_api.h"
FSP_HEADER
/** Timer on GPT Instance. */
extern const timer_instance_t g_timer5;

/** Access the GPT instance using these structures when calling API functions directly (::p_api is not used). */
extern gpt_instance_ctrl_t g_timer5_ctrl;
extern const timer_cfg_t g_timer5_cfg;

#ifndef NULL
void NULL(timer_callback_args_t *p_args);
#endif
/** IPC Instance. */
extern const ipc_instance_t g_ipc0;

/** Access the IPC instance using these structures when calling API functions directly
 (::p_api is not used). */
extern ipc_instance_ctrl_t g_ipc0_ctrl;
extern const ipc_cfg_t g_ipc0_cfg;

#ifndef g_ipc0_Callback
void g_ipc0_Callback(ipc_callback_args_t *p_args);
#endif
/** AGT Timer Instance */
extern const timer_instance_t g_timer_AGT0;

/** Access the AGT instance using these structures when calling API functions directly (::p_api is not used). */
extern agt_instance_ctrl_t g_timer_AGT0_ctrl;
extern const timer_cfg_t g_timer_AGT0_cfg;

#ifndef g_timer_AGT0_Callback
void g_timer_AGT0_Callback(timer_callback_args_t *p_args);
#endif
/** UART on SCI Instance. */
extern const uart_instance_t g_uart0;

/** Access the UART instance using these structures when calling API functions directly (::p_api is not used). */
extern sci_b_uart_instance_ctrl_t g_uart0_ctrl;
extern const uart_cfg_t g_uart0_cfg;
extern const sci_b_uart_extended_cfg_t g_uart0_cfg_extend;

#ifndef g_uart0_Callback
void g_uart0_Callback(uart_callback_args_t *p_args);
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t g_timer2;

/** Access the GPT instance using these structures when calling API functions directly (::p_api is not used). */
extern gpt_instance_ctrl_t g_timer2_ctrl;
extern const timer_cfg_t g_timer2_cfg;

#ifndef g_timer2_Callback
void g_timer2_Callback(timer_callback_args_t *p_args);
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t g_timer1;

/** Access the GPT instance using these structures when calling API functions directly (::p_api is not used). */
extern gpt_instance_ctrl_t g_timer1_ctrl;
extern const timer_cfg_t g_timer1_cfg;

#ifndef g_timer1_Callback
void g_timer1_Callback(timer_callback_args_t *p_args);
#endif
/** UART on SCI Instance. */
extern const uart_instance_t g_uart6;

/** Access the UART instance using these structures when calling API functions directly (::p_api is not used). */
extern sci_b_uart_instance_ctrl_t g_uart6_ctrl;
extern const uart_cfg_t g_uart6_cfg;
extern const sci_b_uart_extended_cfg_t g_uart6_cfg_extend;

#ifndef g_uart6_Callback
void g_uart6_Callback(uart_callback_args_t *p_args);
#endif
/** UART on SCI Instance. */
extern const uart_instance_t g_uart9;

/** Access the UART instance using these structures when calling API functions directly (::p_api is not used). */
extern sci_b_uart_instance_ctrl_t g_uart9_ctrl;
extern const uart_cfg_t g_uart9_cfg;
extern const sci_b_uart_extended_cfg_t g_uart9_cfg_extend;

#ifndef g_uart9_Callback
void g_uart9_Callback(uart_callback_args_t *p_args);
#endif
void hal_entry(void);
void g_hal_init(void);
FSP_FOOTER
#endif /* HAL_DATA_H_ */
