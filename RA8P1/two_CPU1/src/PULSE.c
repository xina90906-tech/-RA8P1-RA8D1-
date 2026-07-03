#include "hal_data.h"
#include "Cloud.h"       // 云台控制头文件
#include "PULSE.h"       // 脉冲控制头文件

//系统时钟频率
uint32_t frequency_ = 250000000;

// 脉冲数计数器 - 记录已经发送的脉冲数
uint32_t Pulsex_count = 0;  // X轴脉冲计数器
uint32_t Pulsey_count = 0;  // Y轴脉冲计数器

// 到位标志位 - 表示轴是否到达目标位置
uint8_t X_Arrive_Flag = 0;  // X轴到位标志
uint8_t Y_Arrive_Flag = 0;  // Y轴到位标志

/**
 * @brief 初始化脉冲控制模块
 * @note 配置TIM5和TIM6定时器用于产生步进电机所需的脉冲信号
 *       TIM5用于X轴，TIM6用于Y轴，均配置为PWM模式
 *       NVIC配置TIM5高于TIM6
 *       频率设置为100K,无法用库函数配置预分频器
 * @param 无
 * @retval 无
 */
void PULSE_Init(void)
{
	
	// TIM5配置 - 用于X轴脉冲输出，使用PA1 (TIM2_CH2)
	R_GPT_Open(&g_timer1_ctrl,&g_timer1_cfg);
	// 配置TIM6时基参数，与TIM2相同
	R_GPT_Open(&g_timer2_ctrl,&g_timer2_cfg);
	R_GPT_Stop(&g_timer1_ctrl);
	R_GPT_Stop(&g_timer2_ctrl);
}

/**
 * @brief 设置X轴脉冲频率
 * @note 根据给定频率配置TIM5的PWM参数，控制X轴步进电机转速
 * @param frequency: 脉冲频率值（Hz）
 * @retval 无
 */
void PULSE_SetXAxisPulse(uint32_t frequency)
{
	R_GPT_Stop(&g_timer1_ctrl);  // 先禁用TIM5

	// 计算自动重载值，根据系统时钟和预分频器计算得到所需频率
	uint32_t period = (uint32_t)(frequency_ / frequency) - 1;
	
	R_GPT_PeriodSet(&g_timer1_ctrl,period);  // 设置自动重载值
	// 设置占空比为50%的PWM输出
	uint32_t compare = (uint32_t)((period + 1) * 50.0 / 100);
	
	R_GPT_DutyCycleSet(&g_timer1_ctrl,compare,GPT_IO_PIN_GTIOCB | GPT_BUFFER_FORCE_PUSH);  // 设置比较值 (使用B口)
	R_GPT_Start(&g_timer1_ctrl);  // 启用TIM5
}


/**
 * @brief 设置Y轴脉冲频率
 * @note 根据给定频率配置TIM3的PWM参数，控制Y轴步进电机转速
 * @param frequency: 脉冲频率值（Hz）
 * @retval 无
 */
void PULSE_SetYAxisPulse(uint32_t frequency)
{
	R_GPT_Stop(&g_timer2_ctrl);  // 先禁用TIM6

	// 计算自动重载值，根据系统时钟和预分频器计算得到所需频率
	uint32_t period = (uint32_t)(frequency_ / frequency) - 1;
	
	R_GPT_PeriodSet(&g_timer2_ctrl,period);  // 设置自动重载值
	// 设置占空比为50%的PWM输出
	uint32_t compare = (uint32_t)((period + 1) * 50.0 / 100);
	
	R_GPT_DutyCycleSet(&g_timer2_ctrl,compare,GPT_IO_PIN_GTIOCB | GPT_BUFFER_FORCE_PUSH);  // 设置比较值
	R_GPT_Start(&g_timer2_ctrl);  // 启用TIM6
}

/**
 * @brief TIM2中断处理函数
 * @note X轴脉冲计数和到位检测
 *       每次TIM2更新中断时，X轴脉冲计数器加1
 *       当达到目标脉冲数时，禁用TIM2并设置到位标志
 * @param 无
 * @retval 无
 */
void g_timer1_Callback(timer_callback_args_t * p_args)
{
	if(p_args->event == TIMER_EVENT_CYCLE_END)
	{
		Pulsex_count++;  // X轴脉冲计数加1
		if(Pulsex_count >= X_pulse)  // 检查是否达到目标脉冲数
		{
			Pulsex_count = 0;  // 重置计数器
			R_GPT_Stop(&g_timer1_ctrl);  // 禁用TIM6
			X_Arrive_Flag = 1;  // 设置X轴到位标志
		}
		
	}
}

void g_timer2_Callback(timer_callback_args_t * p_args)
{
	if(p_args->event == TIMER_EVENT_CYCLE_END)
	{
		Pulsey_count++;  // Y轴脉冲计数加1
		if(Pulsey_count >= Y_pulse)  // 检查是否达到目标脉冲数
		{
			Pulsey_count = 0;  // 重置计数器
			R_GPT_Stop(&g_timer2_ctrl);  // 禁用TIM6
			Y_Arrive_Flag = 1;  // 设置Y轴到位标志
		}
		
	}
}


/**
 * @brief 获取指定轴的到位标志
 * @note 读取指定轴的到位标志并清零
 * @param axis: 指定轴 (x_axis 或 y_axis)
 * @retval uint8_t: 1表示到位，0表示未到位
 */
uint8_t PULSE_GetFlag(enum Axis axis)
{
	if(axis == x_axis)  // 如果是X轴
    {
		if(X_Arrive_Flag)  // 检查X轴到位标志
        {
            X_Arrive_Flag = 0;  // 清零标志
            return 1;  // 返回到位状态
        }
		else
		{
			return 0;  // 返回未到位状态
		}
    }
	
	 if(axis == y_axis)  // 如果是Y轴
    {
		if(Y_Arrive_Flag)  // 检查Y轴到位标志
        {
            Y_Arrive_Flag = 0;  // 清零标志
            return 1;  // 返回到位状态
        }
		else
		{
			return 0;  // 返回未到位状态
		}
    }
    
    return 0;  // 默认返回未到位
}

/**
 * @brief 停止指定轴的脉冲输出
 * @note 禁用指定轴对应的定时器，停止脉冲输出
 * @param axis: 指定轴 (x_axis, y_axis 或 all_axis)
 * @retval 无
 */
void PULSE_Stop(enum Axis axis)
{
    if(axis == x_axis)  // 如果是X轴
    {
        R_GPT_Stop(&g_timer1_ctrl);  // 禁用TIM5
    }
    else if(axis == y_axis)  // 如果是Y轴
    {
        R_GPT_Stop(&g_timer2_ctrl);  // 禁用TIM6
    }
    else if(axis == all_axis)  // 如果是全部轴
    {
        R_GPT_Stop(&g_timer1_ctrl);  // 禁用TIM5
        R_GPT_Stop(&g_timer2_ctrl);  // 禁用TIM6
    }
}
