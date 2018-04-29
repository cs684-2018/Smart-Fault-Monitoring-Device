/*

* Author: Texas Instruments 

* Editted by: Saurav Shandilya, Vishwanathan Iyer 
	      ERTS Lab, CSE Department, IIT Bombay

* Description: This code will test file to check software and hardware setup. This code will blink three colors of RGB LED present of Launchpad board in sequence.  

* Filename: lab-1.c

* Functions: setup(), led_pin_config(), main()  

* Global Variables: none

*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"


/* Global variable declaration*/
uint8_t uiStatusSW1 = 255,uiStatusSW1Final = 255,uicount=0,uicountSW2 = 0;
uint8_t ui8LED = 0;
uint16_t debounce = 0,debounceSW2 = 0;


/*

* Function Name: setup()

* Input: none

* Output: none

* Description: Set crystal frequency,enable GPIO Peripherals and unlock Port F pin 0 (PF0)

* Example Call: setup();

*/
void setup(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	//unlock PF0 based on requirement

}


/* Function Name: pin_config()

* Input: none

* Output: none

* Description: Set Port F Pin 1, Pin 2, Pin 3 as output. On this pin Red, Blue and Green LEDs are connected.
			   Set Port F Pin 0 and 4 as input, enable pull up on both these pins.

* Example Call: pin_config();

*/

void led_pin_config(void)
{
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);

	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK)= GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK)= 0 ;



	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);



}


int main(void)
{
	unsigned int temp = 0,Key_pressed=0,lock_flag = 0,uiStatusSW2Final,lock_flag2=0,SW2_pressed=0;
	setup();
	led_pin_config();

	while(1)
	{


		/*

Use switch SW2 and sw2Status (a variable). Your program should increment sw2Status
by one, every time switch is pressed. Note how the value of sw2Status changes on each
switch press. Use debugger and add sw2Status to \Watch Expression" window. Does
the value of sw2Status increment by one always? Show the result to TA. Note: Dene
sw2Status as a global variable and in debug perspective use continuous refresh option

Refresh Counter: uicountSW2
		 * */
		uiStatusSW2Final = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0);
		uiStatusSW2Final = (uiStatusSW2Final  & 0x01);
		if((uiStatusSW2Final==0)&(SW2_pressed == 0))
		{
			debounceSW2++;
			if(debounceSW2 >= 50000)
			{
				SW2_pressed =1;
				debounceSW2 = 0;
				uicountSW2++;
			}
		}
		else
		{
			if(uiStatusSW2Final!=0)
			{
			debounceSW2 = 0;
			SW2_pressed =0;
			}
		}

		/*
		 * Use switch SW1 to Turn on Red LED on First switch press, Green LED on second
switch press and Blue LED on third switch press. Repeat the same cycle next switch
press onwards. Note that LED should remain on for the duration switch is kept pressed
i.e. LED should turn o when switch is released.
		 */

		uiStatusSW1Final = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4);
		uiStatusSW1Final = (uiStatusSW1Final  & 0x10)>>4;
		if(uiStatusSW1Final==0)
		{
			debounce++;
			if(debounce >= 50000)
			{
				uiStatusSW1 = 0;
				Key_pressed = 1;
			}
		}
		else
		{
			debounce = 0;
			uiStatusSW1 = 1;
			Key_pressed = 0;
		}

		switch(uicount)
		{

		case 0 :

			if(uiStatusSW1==0)
			{
				temp = 1;
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 2); // first color

			}
			else
			{
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
				/* locking the past selection*/
				if(temp){
				uicount = 1;
				temp = 0;
				}
			}
			break;

		case 1:

			if(uiStatusSW1==0)
			{
				temp = 1;
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 4);  // Second color

			}
			else
			{
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);

				/* locking the past selection*/
				if(temp){
						uicount = 2;
						temp = 0;
						}
			}

			break;

		case 2 :

			if(uiStatusSW1==0)
			{
				temp = 1;
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 8); //// Third color

			}
			else
			{
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
				/* locking the past selection*/
				if(temp){
						uicount = 0;
						temp = 0;
						}
			}

			break;
		}

	}
}
