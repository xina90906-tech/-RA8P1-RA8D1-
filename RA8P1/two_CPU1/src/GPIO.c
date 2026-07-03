#include "GPIO.h"

/**
 * @brief 初始化云台方向控制引脚
 * @note 配置X轴和Y轴的方向控制GPIO引脚为推挽输出模式
 *       这些引脚用于控制步进电机的旋转方向
 * @param 无
 * @retval 无
 */
void GPIO_DirectionInit(void)
{
    //通过图形化配置进行初始化
	
    // 初始化方向状态（默认设置为0，表示初始方向）
    g_ioport.p_api->pinWrite(X_DIR_PORT, X_DIR_PIN,0);  // X轴方向引脚初始化为低电平
    g_ioport.p_api->pinWrite(Y_DIR_PORT, Y_DIR_PIN,0);  // Y轴方向引脚初始化为低电平
}

/**
 * @brief 设置X轴步进电机方向
 * @param direction: 方向控制值 (0: 顺时针, 1: 逆时针)
 * @retval 无
 */
void GPIO_SetXDirection(uint8_t direction)
{
    // 根据方向参数设置X轴方向引脚的电平
    g_ioport.p_api->pinWrite(X_DIR_PORT, X_DIR_PIN, direction ? 1 : 0);
	R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MICROSECONDS);
}

/**
 * @brief 设置Y轴步进电机方向
 * @param direction: 方向控制值 (0: 顺时针, 1: 逆时针)
 * @retval 无
 */
void GPIO_SetYDirection(uint8_t direction)
{
    // 根据方向参数设置Y轴方向引脚的电平
    g_ioport.p_api->pinWrite(Y_DIR_PORT, Y_DIR_PIN, direction ? 1 : 0);
	R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MICROSECONDS);
}
