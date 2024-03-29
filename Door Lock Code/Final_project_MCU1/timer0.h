/*
 * timer0.h
 *
 *  Created on: Jul 13, 2021
 *      Author: ammar
 */

#ifndef TIMER0_H_
#define TIMER0_H_

#include "std_types.h"
#include "common_macros.h"
#include "micro_config.h"


typedef enum{
	NORMAL_MODE,PWM_PC,CTC,F_PWM
}TIMER0_MODE;

typedef enum
{
	NO_CLOCK,F_CPU_CLOCK,F_CPU_8,F_CPU_64,F_CPU_256,F_CPU_1024
}Timer0_Clock;

typedef struct{
	TIMER0_MODE mode;
	Timer0_Clock clock;
	uint8 OCR;
}Timer0_ConfigType;

//prototypes
void Timer0_init(const Timer0_ConfigType * Config_Ptr);
void ISR_COUNT(void);

void Set_callBack(void(*a_ptr)(void));

void timer0_DeInit(void);


#endif /* TIMER0_H_ */
