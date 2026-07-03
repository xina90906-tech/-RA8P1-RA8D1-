#include "graph.h"              // 图形生成头文件
#include <math.h>               // 数学函数库，用于三角函数和平方根计算
#include <stdlib.h>             // 标准库，提供常用工具函数

/**
 * @brief 计算两个整数中的最大值
 * @param a 第一个整数
 * @param b 第二个整数
 * @return 两个数中的较大值
 */
int max(int a, int b) {
    return (a > b) ? a : b;
}

/**
 * @brief 在给定点集之间进行线性插补，生成连续的路径点
 * @param points 输入的原始点集，每个点包含X和Y坐标
 * @param point_count 原始点集的点数
 * @param generated_points 输出的生成点集，存储插补后的路径点
 * @param step_size 插补步长（未实际使用，而是使用了最大坐标差值作为步数）
 * @return 生成的路径点总数
 */
uint32_t interpolate_path(int32_t points[][2], int point_count, int32_t generated_points[][2], float step_size)
{
    // 检查是否有足够的点进行插补（至少需要2个点）
    if (point_count < 2)
    {
        return 0;  // 点数不足，无法进行插补
    }
    
    int total_generated = 0;  // 记录生成的路径点总数

    // 遍历每一对连续的点，在它们之间进行插补
    for (int i = 0; i < point_count - 1; i++)
    {
        // 计算当前点与下一点之间的X和Y方向的差值
        float dx = points[i+1][0] - points[i][0];
        float dy = points[i+1][1] - points[i][1];
        // 计算两点之间的欧几里得距离
        float distance = sqrt(dx * dx + dy * dy);
        
        // 计算需要插补的步数
        // 注意：这里使用了X和Y方向上绝对值较大的值作为步数，而非按step_size计算
        // 这样可以确保在斜线上也能正确插补，避免步进电机失步
        int num_steps = max(abs((int)dx), abs((int)dy));
        if (num_steps < 1) num_steps = 1;  // 确保至少有1步
        
        // 在当前点与下一点之间生成插补点
        for (int step = 0; step <= num_steps; step++)
        {
            // 计算当前步骤在总步数中的比例
            float ratio = (float)step / num_steps;
            
            // 使用线性插值计算当前步骤的X和Y坐标，并四舍五入提高精度
            int32_t x = (int32_t)round(points[i][0] + dx * ratio);
            int32_t y = (int32_t)round(points[i][1] + dy * ratio);
            
            // 将计算得到的点添加到生成路径点数组
            generated_points[total_generated][0] = x;
            generated_points[total_generated][1] = y;
            total_generated++;  // 更新生成点计数
            
            // 注意：以下代码存在逻辑问题，可能导致重复存储点
            // 由于前面已经存储了点，这里又进行了一次重复的条件判断和存储
            // 建议：删除此部分代码以避免路径点重复和数组越界风险
            if (total_generated == 0 || 
                generated_points[total_generated-1][0] != x || 
                generated_points[total_generated-1][1] != y)
            {
                generated_points[total_generated][0] = x;
                generated_points[total_generated][1] = y;
                total_generated++;
            }
            
        }
    }
    total_generated++;  // 额外增加计数，可能用于为结束标记预留位置
    return total_generated;  // 返回生成的路径点总数

}

/**
 * @brief 预计算圆形轨迹的路径点
 * @param radius_mm 圆的半径（毫米）
 * @param generated_points 输出的生成点集，存储圆形轨迹的路径点
 * @param num_points 要生成的路径点数量
 */
void precompute_circle(float radius_mm, int32_t generated_points[][2], uint16_t num_points)
{
    // 计算每弧度对应的步进电机步数
    // STEPS_PER_REV：步进电机每转的步数，PI：圆周率
    const float steps_per_radian = STEPS_PER_REV / (2.0f * PI);  // 每弧度步数
    
    // 生成圆形轨迹的每个点
    for(int i = 0; i < num_points; i++)
    {
        // 计算当前点对应的角度（从0到2π，等分整个圆）
        float angle = 2.0f * PI * i / num_points;
        
        // 使用极坐标到直角坐标的转换，计算点的物理坐标（毫米）
        float x_mm = radius_mm * cos(angle);
        float y_mm = radius_mm * sin(angle);
        
        // 将物理坐标转换为云台X轴和Y轴需要转动的角度（弧度）
        // 使用反正切函数计算角度，DISTANCE_MM是云台到目标平面的距离
        float angle_x_rad = atan2f(x_mm, DISTANCE_MM);
        float angle_y_rad = atan2f(y_mm, DISTANCE_MM);
        
        // 将角度转换为步进电机的步数，并四舍五入取整
        generated_points[i][0] = (int32_t)round(angle_x_rad * steps_per_radian);
        generated_points[i][1] = (int32_t)round(angle_y_rad * steps_per_radian);

        // 在最后一个点设置结束标记（-1，-1），表示轨迹结束
        if(i == num_points - 1)
        {
            generated_points[i][0] = -1;
            generated_points[i][1] = -1;
        }
    }
}


// 幅值，周期，步数
/**
 * @brief 预计算正弦曲线轨迹的路径点
 * @param amplitude_mm 正弦波的振幅（毫米）
 * @param period_mm 正弦波的周期（毫米）
 * @param generated_points 输出的生成点集，存储正弦曲线轨迹的路径点
 * @param num_points 要生成的路径点数量
 */
void precompute_sin(float amplitude_mm, float period_mm, int32_t generated_points[][2], uint16_t num_points)
{
    // 计算每弧度对应的步进电机步数
    const float steps_per_radian = STEPS_PER_REV / (2.0f * PI);  // 每弧度步数
    // 计算角频率 W = 2π/周期
    float W = 2.0f * PI / period_mm;
    
    // 生成正弦曲线轨迹的每个点
    for(int i = 0; i < num_points; i++)
    {
        // 计算X坐标（线性变化）
        float x_mm = period_mm * (i+1.0f) / num_points;
        // 根据正弦函数计算Y坐标
        float y_mm = amplitude_mm * sin(W * x_mm);

        // 将物理坐标转换为云台X轴和Y轴需要转动的角度（弧度）
        float angle_x_rad = atan2f(x_mm, DISTANCE_MM);
        float angle_y_rad = atan2f(y_mm, DISTANCE_MM);

        // 将角度转换为步进电机的步数，并四舍五入取整
        generated_points[i][0] = (int32_t)round(angle_x_rad * steps_per_radian);
        generated_points[i][1] = (int32_t)round(angle_y_rad * steps_per_radian);
        // 添加最后一个点（-1，-1）
        if(i == num_points - 1)
        {
            generated_points[i][0] = -1;
            generated_points[i][1] = -1;
        }
    }
}

/**
 * @brief 预计算多边形轨迹的路径点
 * @param points 多边形顶点坐标数组
 * @param point_count 多边形顶点数量
 * @param generated_points 输出的生成点集，存储多边形轨迹的路径点
 * @param step_size 路径插补的步长
 */
void precompute_polygon(int32_t points[][2], int point_count, int32_t generated_points[][2], float step_size)
{  
    // 首先调用interpolate_path函数在多边形顶点之间进行插补
    uint32_t total_generated = interpolate_path(points, point_count, generated_points, step_size);

    // 计算每弧度对应的步进电机步数
    const float steps_per_radian = STEPS_PER_REV / (2.0f * PI);  // 每弧度步数

    // 遍历所有生成的路径点，将其转换为步进电机步数
    for(int i = 0; i < total_generated; i++)
    {
        // 将物理坐标转换为云台X轴和Y轴需要转动的角度（弧度）
        float angle_x_rad = atan2f(generated_points[i][0], DISTANCE_MM);
        float angle_y_rad = atan2f(generated_points[i][1], DISTANCE_MM);
        
        // 将角度转换为步进电机的步数，并四舍五入取整
        generated_points[i][0] = (int32_t)round(angle_x_rad * steps_per_radian);
        generated_points[i][1] = (int32_t)round(angle_y_rad * steps_per_radian);
        // 添加最后一个点（-1，-1）
        if(i == total_generated - 1)
        {
            generated_points[i][0] = -1;
            generated_points[i][1] = -1;
        }

    }  
}

/**
 * @brief 预计算二次曲线轨迹的路径点
 * @param low_x X坐标的最小值（毫米）
 * @param high_x X坐标的最大值（毫米）
 * @param generated_points 输出的生成点集，存储二次曲线轨迹的路径点
 * @param num_points 要生成的路径点数量
 */
void precompute_quadratic(int32_t low_x, int32_t high_x, int32_t generated_points[][2], uint16_t num_points)
{
    // 计算每弧度对应的步进电机步数
    const float steps_per_radian = STEPS_PER_REV / (2.0f * PI);  // 每弧度步数
    
    // 生成二次曲线轨迹的每个点
    for(int i = 0; i < num_points; i++)
    {
        // 在指定范围内线性分配X坐标
        float x_mm = low_x + (high_x - low_x) * i / (num_points - 1);
        // 根据二次函数 y = x²/16 计算Y坐标
        float y_mm = x_mm * x_mm / 16;

        // 将物理坐标转换为云台X轴和Y轴需要转动的角度（弧度）
        float angle_x_rad = atan2f(x_mm, DISTANCE_MM);      
        float angle_y_rad = atan2f(y_mm, DISTANCE_MM);

        // 将角度转换为步进电机的步数，并四舍五入取整
        generated_points[i][0] = (int32_t)round(angle_x_rad * steps_per_radian);
        generated_points[i][1] = (int32_t)round(angle_y_rad * steps_per_radian);
        // 添加最后一个点（-1，-1）
        if(i == num_points - 1)
        {
            generated_points[i][0] = -1;
            generated_points[i][1] = -1;
        }
    }

}
