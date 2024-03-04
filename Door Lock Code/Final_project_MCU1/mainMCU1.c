/*
 * mainMCU1.c
 *
 *  Created on: Jul 13, 2021
 *      Author: ammar
 */

#include"keypad.h"
#include"lcd.h"
#include"uart.h"
#include "timer0.h"

#define M1_READY 0x20
#define M2_READY 0x10
#define NUMBER_OF_OVERFLOWS_PER_SECOND 32

/*timer global variables*/
uint8 volatile g_tick = 0;
uint8 volatile g_sec_counted = 0;
uint8 volatile g_seconds_needed=0;
uint8 volatile g_step=0;

void ISR_COUNT(void)
{
	g_tick++;		/*increases the global tick each time the timer interrupt*/
	if(g_tick == NUMBER_OF_OVERFLOWS_PER_SECOND){		/*checks if the timer counted one second*/
		g_tick = 0; 									/*clear the tick counter again to count a new second*/
		g_sec_counted++;								/*increases seconds counter*/
		if (g_sec_counted==g_seconds_needed)
		{
			g_sec_counted=0;
			timer0_DeInit();
			g_step=2;
		}
	}
}

int main(){

	/*initialize LCD and UART */
	LCD_init();
	UART_init();
	uint8 key[5];
	uint8 check=0;
	uint8 send_key=0;

	UART_sendByte(M1_READY);			/*sends to MCU2 that MCU1 is ready to receive data*/
	g_step=UART_recieveByte();			/*receives the step which the code run*/

	while(1)
	{
		switch( g_step ){
		/*in step zero you open the project for the first time and there is no password saved in EEPROM or when changing password */
		case 0:
			LCD_displayString("Enter New Pass");	/*display string on LCD for 0.5 second*/
			_delay_ms(500);
			LCD_goToRowColumn(1,0);
			for(int i=0 ;i<5;i++ ){
				/* if any switch pressed for more than 500 ms it counts more than one press */
				key[i] = KeyPad_getPressedKey(); /* get the pressed key number */
				LCD_displayCharacter('*');
				/*extra addition
				 * button ON/C in keyboard clears the password entered without count any wrong trials
				 */
				if(key[i]==13){
					/*resets the LCD screen*/
					LCD_clearScreen();
					LCD_displayString("Enter New Pass");
					LCD_goToRowColumn(1,0);
					i=-1;
				}
				_delay_ms(500); /* Press time */
			}
			LCD_clearScreen();

			/*repeats entering the key until user presses = sign */
			do
			{
				LCD_displayString("Press = to send!");//Statements
				send_key=KeyPad_getPressedKey();
			}
			while(send_key!= '=');

			/*in case the user pressed send button the MCU1 sends the password to MCU2*/
			for (int i=0;i<5;i++)
			{
				UART_sendByte(key[i]);
			}
			LCD_clearScreen();
			LCD_displayString("Password1 sent");
			_delay_ms(500);
			LCD_clearScreen();

			/* reentering the new password to make sure that the entered password is the wanted one*/
			LCD_displayString("Reenter the New pass");
			LCD_goToRowColumn(1,0);
			for(int i=0 ;i<5;i++ )
			{
				/* if any switch pressed for more than 500 ms it counts more than one press */
				key[i] = KeyPad_getPressedKey(); /* get the pressed key number */
				LCD_displayCharacter('*');
				/*extra addition
				 * button ON/C in keyboard clears the password entered without count any wrong trials
				 */
				if(key[i]==13)
				{
					/*reset the LCD*/
					LCD_clearScreen();
					LCD_displayString("Reenter New pass");
					LCD_goToRowColumn(1,0);
					/*the counter is set to -1 so the next loop will be resets to zero as i increases in each loop by one*/
					i=-1;
				}
				_delay_ms(500); /* Press time */
			}
			LCD_clearScreen();

			/*also repeats entering the key until user presses = sign */
			do
			{
				LCD_displayString("press = to send!");//Statements
				send_key=KeyPad_getPressedKey();
			}while(send_key!= '=');

			/*in case the user pressed send button the MCU1 sends the password to MCU2*/
			for (int i=0;i<5;i++)
			{
				UART_sendByte(key[i]);
			}
			LCD_clearScreen();
			LCD_displayString("Password2 sent");
			_delay_ms(500);

			/*receiving byte which determines if the two passwords are matched it will go to next step
			 * either the two passwords are unmatched so it repeats the current step until the two passwords are matched
			 */
			g_step=UART_recieveByte();
			LCD_clearScreen();
			if (g_step==0)
			{
				LCD_displayString("Unmatched!");
				_delay_ms(500);
				LCD_clearScreen();
			}
			else if(g_step==1)
			{
				LCD_displayString("Matched!");
				LCD_displayStringRowColumn(1,0,"Pass Saved!");
				_delay_ms(500);
				g_step=2;
			}
			break;


		case 1:
			/*this steps displays that the two passwords sent are matched and saved in EEPROM*/
			LCD_displayString("Matched!");
			LCD_displayStringRowColumn(1,0,"Pass saved!");
			_delay_ms(500);
			g_step=2;
			break;


		case 2:

			/*this step is the main step as the MCU1 asks the user to choose between
			 * opening or closing the door or changing the saved password
			 */
			while(UART_recieveByte()!=M2_READY);		/*waits until MCU2 is ready*/
			LCD_clearScreen();

			/*displays multiple options for the user*/
			LCD_displayString("+:open -:close");
			LCD_displayStringRowColumn(1,0,"*:change pass");

			/*repeats until the user choose one of three options */
			do
			{
				send_key=KeyPad_getPressedKey();
			}while((send_key!= '*')&&(send_key!= '+')&&(send_key!= '-'));

			/*sends to MCU2 the option which the user chose*/
			UART_sendByte(send_key);
			LCD_clearScreen();
			LCD_displayCharacter(send_key);
			_delay_ms(500);

			/*the chosen option makes the code go to defined step*/
			if (send_key=='+')
			{
				g_step=3;
			}
			else if(send_key=='*')
			{
				g_step=4;
			}
			else if(send_key=='-')
			{
				g_step=6;
			}
			break;

		case 3:

			/*This step in code makes the motor open the door for 15 seconds
			 * there is trial 3 times to enter the password correct or the code go to step 5 and buzzer turned on
			 */
			LCD_displayString("enter password");
			LCD_goToRowColumn(1,0);
			/*enters the password*/
			for(int i=0 ;i<5;i++ )
			{
				/* if any switch pressed for more than 500 ms it counts more than one press */
				key[i] = KeyPad_getPressedKey(); /* get the pressed key number */
				LCD_displayCharacter('*');
				/*extra addition
				 * button ON/C in keyboard clears the password entered without count any wrong trials
				 */
				if(key[i]==13)
				{
					/*reset the screen*/
					LCD_clearScreen();
					LCD_displayString("enter password ");
					LCD_goToRowColumn(1,0);
					i=-1;				/*the counter is set to -1 so the next loop will be resets to zero as i increases in each loop by one*/
				}
				_delay_ms(500); /* Press time */
			}

			LCD_clearScreen();
			/*repeats entering the key until user presses = sign */
			do
			{
				LCD_displayString("press = to send!");//Statements
				send_key=KeyPad_getPressedKey();
			}while(send_key!= '=');

			/*sends the password to MCU2*/
			for (int i=0;i<5;i++)
			{
				UART_sendByte(key[i]);
			}
			LCD_clearScreen();

			/*receive byte checks the password with the saved in EEPROM*/
			check=UART_recieveByte();


			if(check==0)
			{
				/*in case the user didn't enter the correct password the lcd displays unmatched and receive step the code will goto*/
				LCD_displayString("unmatched");
				_delay_ms(1000);
				LCD_clearScreen();
				g_step=UART_recieveByte();
			}
			else if(check==1)
			{
				/*in case the password matches displays matched in LCD and counts 15 seconds while displaying that the door is opening*/
				LCD_displayString("matched");
				_delay_ms(1000);
				LCD_clearScreen();
				while(UART_recieveByte()!=M2_READY);		/*waits until MCU2 is ready*/
				g_seconds_needed=15;						/*set the required time*/
				SREG |= (1<<7);								/*enable I-bit*/
				Set_callBack(ISR_COUNT);					/* Set the Call back function pointer in the timer0 driver*/
				Timer0_ConfigType timer0_config = {CTC,F_CPU_1024,245};	/*create the configuration parameters of timer0*/
				LCD_displayString("door is opening!");			/*displays that the door is opening*/
				Timer0_init(&timer0_config);		/*initialize the timer with selected configurations*/
				/*this line prevents the code to repeat step3 and get stuck in UART receiving byte
				 * the code will wait in infinite loop until the counter finishes and reset the step to main
				 */
				while(g_step!=2);
			}

			break;

		case 4:
			/*this step resets the password by entering the old one and change it
			 * the are three trials for entering the password right or the code will jump to step5
			 */
			LCD_displayString("Enter Password");
			LCD_goToRowColumn(1,0);
			for(int i=0 ;i<5;i++ )
			{
				/* if any switch pressed for more than 500 ms it counts more than one press */
				key[i] = KeyPad_getPressedKey(); /* get the pressed key number */
				LCD_displayCharacter('*');
				/*extra addition
				 * button ON/C in keyboard clears the password entered without count any wrong trials
				 */
				if(key[i]==13)
				{
					/*reset the LCD screen*/
					LCD_clearScreen();
					LCD_displayString("Enter Password ");
					LCD_goToRowColumn(1,0);
					i=-1;							/*the counter is set to -1 so the next loop will be resets to zero as i increases in each loop by one*/
				}
				_delay_ms(500); /* Press time */
			}

			LCD_clearScreen();

			/*repeats entering the key until user presses = sign */
			do
			{
				LCD_displayString("press = to send!");//Statements
				send_key=KeyPad_getPressedKey();
			}while(send_key!= '=');

			/*send password and receives the check from MCU2*/
			for (int i=0;i<5;i++){
				UART_sendByte(key[i]);
			}
			LCD_clearScreen();

			check=UART_recieveByte();
			/*in case the password didn't match the saved in EEPROM the MCU2 sends that the two passes aren't same
			 * and repeat the current step, and in case they matched the code goto step 1 where user can change password
			 */
			if(check==0){
				LCD_displayString("unmatched");
				_delay_ms(1000);
				LCD_clearScreen();
				g_step=UART_recieveByte();
			}else if(check==1){
				LCD_displayString("matched");
				_delay_ms(1000);
				LCD_clearScreen();
				g_step=UART_recieveByte();
			}
			break;

			/*the code will go to this step in step the user entered wrong password for 3 times*/
		case 5:
			LCD_displayString("There is a thief!");			/*displays a warning message*/
			g_seconds_needed=60;							/*select required time*/
			SREG |= (1<<7);									/*enable I-bit*/
			Set_callBack(ISR_COUNT);						/* Set the Call back function pointer in the timer0 driver*/
			Timer0_ConfigType timer0_config = {CTC,F_CPU_1024,245};	/*create the configuration parameters of timer0*/
			LCD_displayString("door is closing!");				/*displays that the door is closing*/
			Timer0_init(&timer0_config);						/*initialize the timer with selected configurations*/
			/*this line prevents the code to repeat step3 and get stuck in UART receiving byte
			 * the code will wait in infinite loop until the counter finishes and reset the step to main
			 */
			while(g_step!=2);
			break;


			/* step 6 is like step 3 exactly but the motor will run CCW*/
		case 6:
			LCD_displayString("enter password");
			LCD_goToRowColumn(1,0);
			/*enter the password*/
			for(int i=0 ;i<5;i++ ){
				/* if any switch pressed for more than 500 ms it counts more than one press */
				key[i] = KeyPad_getPressedKey(); /* get the pressed key number */
				LCD_displayCharacter('*');
				/*extra addition
				 * button ON/C in keyboard clears the password entered without count any wrong trials
				 */
				if(key[i]==13){
					/*reset the LCD screen*/
					LCD_clearScreen();
					LCD_displayString("enter password ");
					LCD_goToRowColumn(1,0);
					i=-1;						/*the counter is set to -1 so the next loop will be resets to zero as i increases in each loop by one*/
				}
				_delay_ms(500); /* Press time */

			}

			LCD_clearScreen();
			/*repeats until the user presses = sign to send*/
			do
			{
				LCD_displayString("press = to send!");//Statements
				send_key=KeyPad_getPressedKey();
			}while(send_key!= '=');

			/*sending the password to MCU2 and receive indication if the password matches or not*/
			for (int i=0;i<5;i++){
				UART_sendByte(key[i]);
			}
			LCD_clearScreen();
			check=UART_recieveByte();

			/*in case the passwords don't match repeat the same step
			 * and in case it match it close the door
			 */

			if(check==0){
				LCD_displayString("unmatched");
				_delay_ms(1000);
				LCD_clearScreen();
				g_step=UART_recieveByte();
			}else if(check==1){
				LCD_displayString("matched");
				_delay_ms(1000);
				LCD_clearScreen();
				while(UART_recieveByte()!=M2_READY);		/*waits until MCU2 is ready*/
				g_seconds_needed=15;			/*select required time*/
				SREG |= (1<<7);					/*enable the I-bit*/
				Set_callBack(ISR_COUNT);		/* Set the Call back function pointer in the timer0 driver*/
				Timer0_ConfigType timer0_config = {CTC,F_CPU_1024,245};/*create the configuration parameters of timer0*/
				LCD_displayString("door is closing!");
				Timer0_init(&timer0_config);	/*initialize the timer with selected configurations*/
				/*this line prevents the code to repeat step3 and get stuck in UART receiving byte
				 * the code will wait in infinite loop until the counter finishes and reset the step to main
				 */
				while(g_step!=2);
			}
			break;
		}
	}
}

