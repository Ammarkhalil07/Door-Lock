/*
 * motor.h
 *
 *   Created on: Jul 13, 2021
 *      Author:  ammar
 */

#ifndef MOTOR_H_
#define MOTOR_H_
#include "micro_config.h"
#include "std_types.h"
#include "common_macros.h"

#define MOTOR_CTRL_PORT_DIR DDRC
#define MOTOR_CTRL_PORT PORTC


// prototypes
void Motor_init(void);
void Motor_CW(void);
void Motor_CCW(void);
void Motor_stop(void);

#endif /* MOTOR_H_ */
