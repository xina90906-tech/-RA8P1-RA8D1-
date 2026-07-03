#include "Cloud.h"       // 云台控制头文件
#include "PULSE.h"       // 脉冲控制头文件
#include "GPIO.h"        // GPIO配置头文件
#include "hal_data.h"
#include <stdlib.h>      // 标准库函数（如abs）
#include <math.h>        // 数学函数（如atan2f、cosf、sinf）

// 全局方向变量，供PULSE.c访问
uint8_t Cloud_X_Direction = 0;  // X轴方向（0为CW顺时针，1为CCW逆时针）
uint8_t Cloud_Y_Direction = 0;  // Y轴方向（0为CW顺时针，1为CCW逆时针）
uint32_t X_pulse = 0;           // X轴需要发送的脉冲总数
uint32_t Y_pulse = 0;           // Y轴需要发送的脉冲总数
int32_t current_x_position = 0; // 当前X轴位置（脉冲数）
int32_t current_y_position = 0; // 当前Y轴位置（脉冲数）

/**
 * @brief X轴速度控制函数
 * @note 控制X轴以指定速度和方向持续运动
 * @param speed: 移动速度（脉冲频率，Hz）
 * @param direction: 运动方向（0为CW顺时针，1为CCW逆时针）
 * @retval 无
 */
void Cloud_X_Move_speed(uint32_t speed,uint8_t direction)
{
    // 确定运动方向 (0为CW，1为CCW)
    Cloud_X_Direction = direction;
    if (speed == 0)
    {
        PULSE_Stop(x_axis);  // 如果速度为0，停止X轴运动
        return;
    }
    // 设置X轴方向
    GPIO_SetXDirection(Cloud_X_Direction);
    // 设置脉冲频率（控制速度）
    PULSE_SetXAxisPulse(speed);
}

/**
 * @brief Y轴速度控制函数
 * @note 控制Y轴以指定速度和方向持续运动
 * @param speed: 移动速度（脉冲频率，Hz）
 * @param direction: 运动方向（0为CW顺时针，1为CCW逆时针）
 * @retval 无
 */
void Cloud_Y_Move_speed(uint32_t speed,uint8_t direction)
{
    // 确定运动方向 (0为CW，1为CCW)
    Cloud_Y_Direction = direction;
    if (speed == 0)
    {
        PULSE_Stop(y_axis);  // 如果速度为0，停止Y轴运动
        return;
    }
    // 设置Y轴方向
    GPIO_SetYDirection(Cloud_Y_Direction);
    // 设置脉冲频率（控制速度）
    PULSE_SetYAxisPulse(speed);
}

/**
 * @brief X轴位置移动函数
 * @note 控制X轴移动指定脉冲数，根据脉冲数的正负确定方向
 * @param pulse: 移动的脉冲数（正数为一个方向，负数为另一个方向）
 * @param speed: 移动速度（脉冲频率，Hz）
 * @retval 无
 */
void Cloud_X_Move_position(int32_t pulse,uint32_t speed)
{
    // 根据脉冲数的正负确定运动方向（正数为CCW，负数为CW）
    Cloud_X_Direction = (pulse > 0) ? 1 : 0;
    // 取脉冲数的绝对值
	if (Cloud_Y_Direction == 0)
	{
		X_pulse = (uint32_t)-pulse;
	}
    
    if (pulse == 0)
    {
        Cloud_Stop(x_axis);  // 如果不需要移动，停止X轴
        return;
    }
    
    // 启动X轴运动
    Cloud_X_Move_speed(speed, Cloud_X_Direction);
    
    // 等待移动完成
    uint8_t flag = 0;
    do
    {
        flag = PULSE_GetFlag(x_axis);  // 检查X轴是否到位
    } while (flag == 0);
}

/**
 * @brief Y轴位置移动函数
 * @note 控制Y轴移动指定脉冲数，根据脉冲数的正负确定方向
 * @param pulse: 移动的脉冲数（正数为一个方向，负数为另一个方向）
 * @param speed: 移动速度（脉冲频率，Hz）
 * @retval 无
 */
void Cloud_Y_Move_position(int32_t pulse,uint32_t speed)
{
    // 根据脉冲数的正负确定运动方向（正数为CCW，负数为CW）
    Cloud_Y_Direction = (pulse > 0) ? 1 : 0;
	// 取脉冲数的绝对值
	if (Cloud_Y_Direction == 0)
	{
		Y_pulse = (uint32_t)-pulse;
	}
    
    if (pulse == 0)
    {
        Cloud_Stop(y_axis);  // 如果不需要移动，停止Y轴
        return;
    }
    
    // 启动Y轴运动
    Cloud_Y_Move_speed(speed, Cloud_Y_Direction);
    
    // 等待移动完成
    uint8_t flag = 0;
    do
    {
        flag = PULSE_GetFlag(y_axis);  // 检查Y轴是否到位
    } while (flag == 0);
}

/**
 * @brief X轴和Y轴速度控制函数
 * @note 同时控制X轴和Y轴以各自的速度和方向运动
 * @param x_speed: X轴移动速度
 * @param y_speed: Y轴移动速度
 * @param direction_x: X轴运动方向
 * @param direction_y: Y轴运动方向
 * @retval 无
 */
void Cloud_XY_Move_speed(uint32_t x_speed, uint32_t y_speed,uint8_t direction_x,uint8_t direction_y)
{
    // 分别设置X轴和Y轴的运动参数
    Cloud_X_Move_speed(x_speed,direction_x);
    Cloud_Y_Move_speed(y_speed,direction_y);
}

/**
 * @brief 相对位置移动函数
 * @note 控制X轴和Y轴移动指定的相对脉冲数，两轴独立运动
 * @param x_pulse: X轴相对移动脉冲数
 * @param y_pulse: Y轴相对移动脉冲数
 * @param speed_x: X轴移动速度
 * @param speed_y: Y轴移动速度
 * @retval 无
 */
void Cloud_XY_Move_position(int32_t x_pulse, int32_t y_pulse, uint32_t speed_x, uint32_t speed_y)
{
    // 根据脉冲数的正负确定运动方向
    Cloud_X_Direction = (x_pulse > 0) ? 1 : 0;
    Cloud_Y_Direction = (y_pulse > 0) ? 1 : 0;

    // 取脉冲数的绝对值
	if (Cloud_X_Direction)
	{
		X_pulse = (uint32_t)x_pulse;
	}
    else
	{
		X_pulse = (uint32_t)-x_pulse;
	}
	// 取脉冲数的绝对值
	if (Cloud_Y_Direction)
	{
		Y_pulse = (uint32_t)y_pulse;
	}
    else
	{
		Y_pulse = (uint32_t)-y_pulse;
	}
	
    // 如果不需要移动，设置速度为0
    if (x_pulse == 0 )
    {
        speed_x=0;
    }
    if (y_pulse == 0 )
    {
        speed_y=0;
    }

    // 启动两轴运动
    Cloud_XY_Move_speed(speed_x, speed_y, Cloud_X_Direction, Cloud_Y_Direction);
	
    // 等待X轴移动完成（如果需要移动）
    uint8_t flag_x = 0, flag_y = 0;
    do
    {
        flag_x = PULSE_GetFlag(x_axis);
    } while (flag_x == 0 && x_pulse != 0);
	
    // 等待Y轴移动完成（如果需要移动）
    do
    {
        flag_y = PULSE_GetFlag(y_axis);
    } while (flag_y == 0 && y_pulse != 0);
}

/**
 * @brief 计算同步运动的速度分量
 * @note 根据目标轨迹角度计算X轴和Y轴的速度分量，确保两轴同时到达目标位置
 * @param clk_x: X轴需要移动的脉冲数
 * @param clk_y: Y轴需要移动的脉冲数
 * @param base_vel_Acc: 基础速度
 * @param vel_Acc_x: 输出参数，X轴速度分量
 * @param vel_Acc_y: 输出参数，Y轴速度分量
 * @retval 无
 */
void Cloud_Calculate_Sync(int32_t clk_x, int32_t clk_y, 
                                           uint32_t base_vel_Acc,
                                           uint32_t *vel_Acc_x, uint32_t *vel_Acc_y)
{
    // 取脉冲数的绝对值
    uint32_t abs_clk_x = (uint32_t)abs(clk_x);
    uint32_t abs_clk_y = (uint32_t)abs(clk_y);
    
    // 如果有一个轴不需要移动，直接使用基础速度
    if (abs_clk_x == 0 || abs_clk_y == 0) {
        *vel_Acc_x = base_vel_Acc;
        *vel_Acc_y = base_vel_Acc;
        return;
    }
    
    // 计算轨迹角度（使用反正切函数）
    float theta = atan2f((float)abs_clk_y, (float)abs_clk_x);
    
    // 计算余弦和正弦值
    float cos_theta = cosf(theta);
    float sin_theta = sinf(theta);
    
    // 根据三角函数计算各轴速度分量
    // 速度分量 = 基础速度 × 对应方向的三角函数值
    *vel_Acc_x = (uint16_t)(base_vel_Acc * cos_theta);
    *vel_Acc_y = (uint16_t)(base_vel_Acc * sin_theta);
    
    // 确保速度不低于最小值
    if (*vel_Acc_x < min_vel_Acc) *vel_Acc_x = min_vel_Acc;
    if (*vel_Acc_y < min_vel_Acc) *vel_Acc_y = min_vel_Acc;
}

/**
 * @brief 同步移动位置函数
 * @note 控制X轴和Y轴同步移动到目标相对位置，确保两轴同时到达
 * @param x_pulse: X轴相对移动脉冲数
 * @param y_pulse: Y轴相对移动脉冲数
 * @param base_speed: 基础移动速度
 * @retval 无
 */
void Cloud_XY_synMove_position(int32_t x_pulse, int32_t y_pulse, uint32_t base_speed)
{
    // 根据脉冲数的正负确定运动方向
    Cloud_X_Direction = (x_pulse > 0) ? 1 : 0;
    Cloud_Y_Direction = (y_pulse > 0) ? 1 : 0;

    // 取脉冲数的绝对值
	if (Cloud_X_Direction)
	{
		X_pulse = (uint32_t)x_pulse;
	}
    else
	{
		X_pulse = (uint32_t)-x_pulse;
	}
	// 取脉冲数的绝对值
	if (Cloud_Y_Direction)
	{
		Y_pulse = (uint32_t)y_pulse;
	}
    else
	{
		Y_pulse = (uint32_t)-y_pulse;
	}
	
    // 计算同步运动的速度分量
    uint32_t speed_x = 0, speed_y = 0;
    Cloud_Calculate_Sync(x_pulse, y_pulse, base_speed, &speed_x, &speed_y); 
    
    // 如果不需要移动，设置速度为0
    if (x_pulse == 0 )
    {
        speed_x = 0;
    }
    if (y_pulse == 0 )
    {
        speed_y = 0;
    }

    // 启动两轴同步运动
    Cloud_XY_Move_speed(speed_x, speed_y, Cloud_X_Direction, Cloud_Y_Direction);
	
    // 等待X轴移动完成（如果需要移动）
    uint8_t flag_x = 0, flag_y = 0;
    do
    {
        flag_x = PULSE_GetFlag(x_axis);
    } while (flag_x == 0 && x_pulse != 0);
    
    // 等待Y轴移动完成（如果需要移动）
    do
    {
        flag_y = PULSE_GetFlag(y_axis);
    } while (flag_y == 0 && y_pulse != 0);
}

/**
 * @brief 坐标点移动函数
 * @note 从当前位置移动到指定坐标点，两轴独立运动
 * @param target_x: 目标X坐标（脉冲数）
 * @param target_y: 目标Y坐标（脉冲数）
 * @param speed_x: X轴移动速度
 * @param speed_y: Y轴移动速度
 * @retval 无
 */
void move_to_point(int32_t target_x, int32_t target_y, uint32_t speed_x, uint32_t speed_y)
{
    // 计算两轴需要移动的脉冲数
    int32_t dx = target_x - current_x_position;
    int32_t dy = target_y - current_y_position;

    // 移动到目标位置
    Cloud_XY_Move_position(dx, dy, speed_x, speed_y);
    
    // 更新当前位置
    current_x_position = target_x;
    current_y_position = target_y;
}

/**
 * @brief 同步坐标点移动函数
 * @note 从当前位置同步移动到指定坐标点，确保两轴同时到达
 * @param target_x: 目标X坐标（脉冲数）
 * @param target_y: 目标Y坐标（脉冲数）
 * @param base_speed: 基础移动速度
 * @retval 无
 */
int synmove_to_point(int32_t target_x, int32_t target_y, uint32_t base_speed)
{
    // 计算两轴需要移动的脉冲数
    int32_t dx = target_x ;//- current_x_position;
    int32_t dy = target_y ;//- current_y_position;

    // 同步移动到目标位置
    Cloud_XY_synMove_position(dx, dy, base_speed);
    
    // 更新当前位置
    //current_x_position = target_x;
    //current_y_position = target_y;
	return 1;
}

/**
 * @brief 原点复位函数
 * @note 将云台移动回原点位置（0,0）
 * @param speed: 移动速度
 * @retval 无
 */
void Cloud_Origin(uint32_t speed)
{
    // 移动回原点位置
    Cloud_XY_Move_position(-current_x_position, -current_y_position, speed, speed);
    // 重置当前位置为原点
    current_x_position = 0;
    current_y_position = 0;
}

/**
 * @brief 停止函数
 * @note 停止指定轴或所有轴的运动
 * @param axis: 停止的轴（x_axis、y_axis 或 all_axis）
 * @retval 无
 */
void Cloud_Stop(enum Axis axis)
{
    // 调用PULSE模块的停止函数
    PULSE_Stop(axis);
}
