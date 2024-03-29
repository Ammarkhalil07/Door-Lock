/*
 * timer0.c
 *
 *  Created on: Jul 13, 2021
 *      Author: ammar
 */

#include "timer0.h"
#define NULL_PTR ((void*)0)

static volatile void (*g_callBackPtr)(void) = NULL_PTR;

ISR(TIMER0_OVF_vect)
{
	if(g_callBackPtr != NULL_PTR)
	{
		/* Call the Call Back function in the application after the edge is detected */
		(*g_callBackPtr)(); /* another method to call the function using pointer to function g_callBackPtr(); */
	}
}
ISR(TIMER0_COMP_vect)
{
	if(g_callBackPtr != NULL_PTR)
	{
		/* Call the Call Back function in the application after the edge is detected */
		(*g_callBackPtr)(); /* another method to call the function using pointer to function g_callBackPtr(); */
	}
}

void Set_callBack(void(*a_ptr)(void)){
	g_callBackPtr=a_ptr;
}


void Timer0_init(const Timer0_ConfigType * Config_Ptr){

	TCNT0=0;

	if ((Config_Ptr->mode==NORMAL_MODE)){
			SET_BIT(TIMSK,TOIE0);
		}else {
			SET_BIT(TIMSK,OCIE0);
		}


	if ((Config_Ptr->mode==NORMAL_MODE)||(Config_Ptr->mode==CTC)){
		SET_BIT(TCCR0,FOC0);
	}else{
		CLEAR_BIT(TCCR0,FOC0);
	}
	//configue clock
	TCCR0= (TCCR0 & 0xF8) | (Config_Ptr->clock);

	//configure mode
	TCCR0 = (TCCR0 & 0xB7) | (((Config_Ptr->mode)&0x01)<<6);
	TCCR0 = (TCCR0 & 0xB7) | (((Config_Ptr->mode)&0x02)<<3);

	//configure OCR
	OCR0=Config_Ptr->OCR;
}


void timer0_DeInit(){
	TCCR0=0;
	TCNT0=0;
	CLEAR_BIT(TIMSK,OCIE0);
	CLEAR_BIT(TIMSK,TOIE0);

}

