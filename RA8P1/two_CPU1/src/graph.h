#ifndef __GRAPH_H
#define __GRAPH_H

#include <stdint.h>

#define PI 3.14159265358979323846f
#define STEPS_PER_REV 3200
#define DISTANCE_MM 250.0f


// 

// 路径插值函数
uint32_t interpolate_path(int32_t points[][2], int point_count, int32_t generated_points[][2], float step_size);

// 圆形轨迹预计算函数
void precompute_circle(float radius_mm, int32_t generated_points[][2], uint16_t num_points);

// 正弦轨迹预计算函数
void precompute_sin(float amplitude_mm, float period_mm,int32_t generated_points[][2], uint16_t num_points);

// 绘制任意多边形
void precompute_polygon(int32_t points[][2], int point_count, int32_t generated_points[][2], float step_size);

// 绘制二次函数
void precompute_quadratic(int32_t low_x, int32_t high_x, int32_t generated_points[][2], uint16_t num_points);


#endif /* __GRAPH_H */
