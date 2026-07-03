#include "buzzer.h"
#include "bsp_api.h"
#include "hal_data.h"

void Buzzer_On(void)
{
    g_ioport.p_api->pinWrite(&g_ioport_ctrl, BUZZER_PIN, 0);
}

void Buzzer_Off(void)
{
    g_ioport.p_api->pinWrite(&g_ioport_ctrl, BUZZER_PIN, 1);
}

void Buzzer_Init(void)
{
    // 初始化蜂鸣器引脚为输出模式
    fsp_err_t err = R_IOPORT_Open(&g_ioport_ctrl, g_ioport.p_cfg);
    if(err != FSP_SUCCESS) {
        return;
    }

    // 初始状态为关闭
    Buzzer_Off();
}
