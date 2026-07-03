#include "led.h"
#include "bsp_api.h"
#include "hal_data.h"

void LED_Init(void)
{
    fsp_err_t err = R_IOPORT_Open(&g_ioport_ctrl, g_ioport.p_cfg);
    if(err != FSP_SUCCESS) {
        return;
    }
    LED_Off();
}

void LED_On(void)
{
    g_ioport.p_api->pinWrite(&g_ioport_ctrl, LED_PIN, BSP_IO_LEVEL_LOW);
}

void LED_Off(void)
{
    g_ioport.p_api->pinWrite(&g_ioport_ctrl, LED_PIN, BSP_IO_LEVEL_HIGH);
}