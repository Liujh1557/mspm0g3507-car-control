#ifndef CONTROL_MATH_H
#define CONTROL_MATH_H

#include <stdint.h>

/* Incremental PI update in signed floating-point space before PWM conversion. */
static inline uint16_t MotorPid_NextDuty(uint16_t duty, float error,
                                         float previous_error, float kp,
                                         float ki, uint16_t maximum)
{
    float next = (float)duty + kp * (error - previous_error) + ki * error;
    if (next <= 0.0f) {
        return 0U;
    }
    if (next >= (float)maximum) {
        return maximum;
    }
    return (uint16_t)next;
}

#endif
