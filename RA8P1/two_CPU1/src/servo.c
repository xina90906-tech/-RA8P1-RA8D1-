#include "servo.h"
#include "hal_data.h"
#include "FreeRTOS.h"
#include "task.h"

typedef enum e_servo_state
{
    SERVO_STATE_STOPPED,
    SERVO_STATE_ROTATING_FORWARD,
    SERVO_STATE_ROTATING_BACKWARD,
} servo_state_t;

static struct
{
    servo_state_t state;
    uint8_t       current_angle;
    uint8_t       target_angle;
    int8_t        direction;
    uint32_t      step_count;
    uint32_t      total_steps;
} g_servo;

#define GPT_FREQUENCY       (250000000UL)
#define PWM_PERIOD_MS       (20UL)
#define PWM_PERIOD_COUNTS   (5000000UL)

#define COMPARE_0DEG        (125000UL)   // 0.5ms → 0°
#define COMPARE_135DEG      (375000UL)   // 1.5ms → 135°
#define COMPARE_270DEG      (625000UL)   // 2.5ms → 270°

#define COMPARE_RANGE_270   (COMPARE_270DEG - COMPARE_0DEG)

static uint32_t angle_to_compare(uint8_t angle)
{
    if (angle > 270) angle = 270;
    if (angle < SERVO_ANGLE_MIN) angle = SERVO_ANGLE_MIN;
    
    return COMPARE_0DEG + ((uint64_t)angle * COMPARE_RANGE_270) / 270UL;
}

static void servo_set_angle_raw(uint8_t angle)
{
    if (angle > 270) angle = 270;
    if (angle < SERVO_ANGLE_MIN) angle = SERVO_ANGLE_MIN;

    g_servo.current_angle = angle;
    uint32_t compare = angle_to_compare(angle);
    R_GPT_DutyCycleSet(&g_timer5_ctrl, compare, GPT_IO_PIN_GTIOCA);
}

void servo_init(void)
{
    g_servo.state = SERVO_STATE_STOPPED;
    g_servo.current_angle = SERVO_ANGLE_MIN;
    g_servo.target_angle = SERVO_ANGLE_MIN;
    g_servo.direction = 0;
    g_servo.step_count = 0;
    g_servo.total_steps = 0;

    R_GPT_Open(&g_timer5_ctrl, &g_timer5_cfg);
    servo_set_angle_raw(SERVO_ANGLE_MIN);
    R_GPT_Start(&g_timer5_ctrl);
}

void servo_start(void)
{
    if (g_servo.current_angle <= SERVO_ANGLE_MIN)
    {
        g_servo.state = SERVO_STATE_ROTATING_FORWARD;
        g_servo.direction = 1;
    }
    else if (g_servo.current_angle >= 270)
    {
        g_servo.state = SERVO_STATE_ROTATING_BACKWARD;
        g_servo.direction = -1;
    }
    else
    {
        g_servo.state = SERVO_STATE_ROTATING_FORWARD;
        g_servo.direction = 1;
    }
}

void servo_stop(void)
{
    g_servo.state = SERVO_STATE_STOPPED;
    g_servo.direction = 0;
    servo_set_angle_raw(SERVO_ANGLE_MIN);
}

void servo_process(void)
{
    if (g_servo.state == SERVO_STATE_STOPPED)
    {
        return;
    }

    if (g_servo.state == SERVO_STATE_ROTATING_FORWARD)
    {
        if (g_servo.current_angle >= 270)
        {
            g_servo.state = SERVO_STATE_ROTATING_BACKWARD;
            g_servo.direction = -1;
        }
        else
        {
            g_servo.current_angle++;
            servo_set_angle_raw(g_servo.current_angle);
        }
    }
    else if (g_servo.state == SERVO_STATE_ROTATING_BACKWARD)
    {
        if (g_servo.current_angle <= SERVO_ANGLE_MIN)
        {
            g_servo.state = SERVO_STATE_ROTATING_FORWARD;
            g_servo.direction = 1;
        }
        else
        {
            g_servo.current_angle--;
            servo_set_angle_raw(g_servo.current_angle);
        }
    }
}

uint8_t servo_get_angle(void)
{
    return g_servo.current_angle;
}