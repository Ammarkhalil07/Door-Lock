/*
 * motor.c
 *
 *  Created on: Jul 13, 2021
 *      Author:  ammar
 */

#include "motor.h"
void Motor_init(void){

	SET_BIT(MOTOR_CTRL_PORT_DIR,PC3);
	SET_BIT(MOTOR_CTRL_PORT_DIR,PC4);
	CLEAR_BIT(MOTOR_CTRL_PORT,PC3);
	CLEAR_BIT(MOTOR_CTRL_PORT,PC4);

}

void Motor_CW(void){
	SET_BIT(MOTOR_CTRL_PORT,PC3);
	CLEAR_BIT(MOTOR_CTRL_PORT,PC4);

}


void Motor_CCW(void){
	SET_BIT(MOTOR_CTRL_PORT,PC4);
	CLEAR_BIT(MOTOR_CTRL_PORT,PC3);

}

void Motor_stop(void){
	CLEAR_BIT(MOTOR_CTRL_PORT,PC3);
	CLEAR_BIT(MOTOR_CTRL_PORT,PC4);


}
