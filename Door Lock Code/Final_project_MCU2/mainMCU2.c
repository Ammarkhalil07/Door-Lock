/*
 * mainMCU2.c
 *
 *  Created on: Jul 22, 2021
 *      Author: ammar
 */

#include"i2c.h"
#include"external_EEPROM.h"
#include"uart.h"
#include "timer0.h"
#include "motor.h"

#define M1_READY 0x20
#define M2_READY 0x10
#define NUMBER_OF_OVERFLOWS_PER_SECOND 32

/*timer global variables*/
uint8 volatile g_tick = 0;
uint8 volatile g_sec_counted = 0;
uint8 volatile g_seconds_needed=0;


/*global variable determines in which step the code will be */

uint8 volatile g_step=0;



/*function checks the two passwords are matched*/

uint8 check_pass(uint8 *ptr1,uint8 *ptr2)
{
	for(int i=0 ; i<5 ; i++)
	{
		/*checking each element of array is matched*/
		if(ptr1[i]!=ptr2[i])
		{
			return ERROR;
		}
	}
	return SUCCESS;
}


/*function checks the entered password matched with the saved password in EEPROM*/

uint8 check_with_EEPROM(uint8 *ptr1,uint8 *ptr2)
{
	for(int i =0;i<5;i++)
	{
		/*reads the saved value of address 0x0311 and next 4 addresses in EEPROM and check sit with the input password */
		EEPROM_readByte(0x0311+i, &ptr2[i]);
		if(ptr1[i]!=ptr2[i]){
			return ERROR;
		}
	}
	return SUCCESS;

}

void ISR_COUNT(void){

	g_tick++;			/*increases the global tick each time the timer interrupt*/

	if(g_tick == NUMBER_OF_OVERFLOWS_PER_SECOND){		/*checks if the timer counted one second*/
		g_tick = 0; 									/*clear the tick counter again to count a new second*/
		g_sec_counted++;								/*increases seconds counter*/
		if (g_sec_counted==g_seconds_needed){
			g_sec_counted=0;
			Motor_stop();
			CLEAR_BIT(PORTC,PC2);
			timer0_DeInit();
			g_step=2;
		}
	}
}

int main(void)
{
	/*initialize LCD,UART and Motor*/
	UART_init();
	Motor_init();

	/*initialize buzzer*/
	SET_BIT(DDRC,PC2);
	CLEAR_BIT(PORTC,PC2);

	uint8 pass1[5];
	uint8 pass2[5];
	uint8 checkpass;
	uint8 counter_trial=0;
	uint8 result=0;

	while(UART_recieveByte()!=M1_READY);		/*wait until the MCU1 is ready*/

	/*the following lines checks if there is saved password in EEPROM or not
	 * passing variable checkpass by address and checks its value
	 * the initial value of EEPROM is 0xFF
	 * the value of check pass determines which step the code runs*/

	EEPROM_readByte(0x0311, &checkpass);
	if (checkpass !=0xFF)
	{
		g_step=2;
		UART_sendByte(g_step);
	}
	else
	{
		g_step=0;
		UART_sendByte(g_step);
	}

	while(1)
	{
		switch( g_step ){

		/*in step zero you open the project for the first time and there is no password saved in EEPROM or when changing password*/
		case 0:
			/*receiving the first password from MCU1*/
			for(int i=0;i<5;i++)
			{
				pass1[i]= UART_recieveByte();
				_delay_ms(1);
			}

			/*receiving the second password from MCU1*/
			for(int i=0;i<5;i++){
				pass2[i]= UART_recieveByte();
				_delay_ms(1);
			}

			/*checking the two passwords are matched or not
			 * if they are matched the code will go to step 1 and if not the code will repeat step 0 until they match
			 */
			checkpass=check_pass(pass1,pass2);
			UART_sendByte(checkpass);
			g_step=checkpass;
			break;

		case 1:
			/*the time taken to write in eeprom is 3.3msec */
			for (int i =0 ;i<5;i++){
				EEPROM_writeByte(0x0311+i, pass1[i]);
				_delay_ms(10);
			}
			/*after writing in eeprom we go to step 2 (main)*/
			g_step=2;
			break;
			/*this step is the main step which appears by default after each step*/

		case 2:
			/*sends that MCU2 is ready to MCU1*/
			UART_sendByte(M2_READY);
			/*receive the symbol from MCU1 determines in which step the code go */
			uint8 sym=UART_recieveByte();

			if (sym=='+'){
				g_step=3;

			}else if(sym=='*'){
				g_step=4;

			}else if(sym=='-'){
				g_step=6;
			}
			break;

		case 3:

			/*This step in code makes the motor open the door for 15 seconds
			 * there is trial 3 times to enter the password correct or the code go to step 5 and buzzer turned on
			 */

			/*receiving password from MCU1*/
			for(int i =0;i<5;i++){
				pass1[i]=UART_recieveByte();
			}
			/*checks if the sent password matches the saved one or not and send back the result*/
		    result=check_with_EEPROM(pass1,pass2);
			UART_sendByte(result);


			/*in case the user entered wrong password the program counts trials*/
			if(result==0){
				counter_trial++;
				/*the user run out the trials and go to step 5 */
				if(counter_trial==3){
					counter_trial=0;
					g_step=5;
				}

				UART_sendByte(g_step);

				/*in case the user entered right password*/
			}else if(result ==1){
				counter_trial=0;
				UART_sendByte(M2_READY);			/*sends to mcu1 that MCU2 is ready*/
				g_seconds_needed=15;				/*the timer will count 15 seconds */
				SREG |= (1<<7);						/*enables the I-bit*/
				Set_callBack(ISR_COUNT);			/* Set the Call back function pointer in the timer0 driver*/
				Timer0_ConfigType timer0_config = {CTC,F_CPU_1024,245};		/*create the configuration parameters of timer0*/
				Motor_CW();							/*enable the motor mn cw direction*/
				Timer0_init(&timer0_config);		/*initialize the timer with selected configurations*/

				/*this line prevents the code to repeat step3 and get stuck in UART receiving byte
				 * the code will wait in infinite loop until the counter finishes and reset the step to main
				 */
				while(g_step!=2);

			}

			break;


			/*this step resets the password by entering the old one and change it
			 * the are three trials for entering the password right or the code will jump to step5
			 */
		case 4:
			/*receiving the password from MCU1 and checks it with the saved in EEPROM then send back the result*/
			for(int i =0;i<5;i++){
				pass1[i]=UART_recieveByte();
				_delay_ms(1);
			}
			result=check_with_EEPROM(pass1,pass2);
			UART_sendByte(result);


			/*in case the user entered wrong password the program counts trials*/
			if(result==0){
				counter_trial++;

				/*the user run out the trials and go to step 5 */
				if(counter_trial==3){
					counter_trial=0;
					g_step=5;
				}
				UART_sendByte(g_step);

				/*in case the user entered right password the code will jump to step 0 and reset the password*/
			}else if(result ==1){
				counter_trial=0;
				g_step=0;
				UART_sendByte(g_step);

			}
			break;


			/*the code will jump to this step in case the user entered wrong password for 3 times*/

		case 5:

			g_seconds_needed= 60;		/*the timer will count 60 seconds */
			SREG |= (1<<7);				/*enables I-bit*/
			Set_callBack(ISR_COUNT);	/* Set the Call back function pointer in the timer0 driver*/
			Timer0_ConfigType timer0_config = {CTC,F_CPU_1024,245}; 	/*create the configuration parameters of timer0*/
			SET_BIT(PORTC,PC2);				/*sets the buzzer on for 60 seconds*/
			Timer0_init(&timer0_config);	/*initialize the timer with selected configurations*/

			/*this line prevents the code to repeat step3 and get stuck in UART receiving byte
			 * the code will wait in infinite loop until the counter finishes and reset the step to main
			*/
			while(g_step!=2);
			break;


			/* step 6 is like step 3 exactly but the motor will run CCW*/
		case 6:
			/*receiving the password from MCU1 and checks it with the saved in EEPROM then send back the result*/
			for(int i =0;i<5;i++){
				pass1[i]=UART_recieveByte();
			}
			result=check_with_EEPROM(pass1,pass2);
			UART_sendByte(result);


			/*in case the user entered wrong password the program counts trials*/
			if(result==0){
				counter_trial++;
				/*the user run out the trials and go to step 5 */
				if(counter_trial==3){
					counter_trial=0;
					g_step=5;
				}
				UART_sendByte(g_step);
			}else if(result ==1){
				counter_trial=0;

				UART_sendByte(M2_READY);

				g_seconds_needed=15;
				SREG |= (1<<7);
				Set_callBack(ISR_COUNT);
				Timer0_ConfigType timer0_config = {CTC,F_CPU_1024,245};
				Motor_CCW();
				Timer0_init(&timer0_config);

				/*this line prevents the code to repeat step3 and get stuck in UART receiving byte
				* the code will wait in infinite loop until the counter finishes and reset the step to main
				*/
				while(g_step!=2);

			}
			break;
		}

	}
	return 0;
}
