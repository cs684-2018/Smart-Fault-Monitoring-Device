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

/* variabe declaartion */
	uint8_t uiStatusSW1 = 255,uiStatusSW1Final = 255,uicount=0,uicountSW2 = 0;
	uint8_t ui8LED = 0;
	uint16_t debounce = 0;
	unsigned long  val = 0;


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

/*
 *
 Configure SW1 and SW2 such that:
Every time SW1 is pressed toggle delay of LED should cycle through approximately
0.5s, 1s, 2s (Of any one color).
Every time SW2 is pressed color of LED should cycle through Red, Green and Blue.
 */

int main(void)
{
	unsigned int temp = 0,Key_pressed=0,lock_flag = 0,uiStatusSW2Final,lock_flag2=0,lock=0;

	val = 6700000/2;  // 50% duty cycle for ON and OFF

	setup();
	led_pin_config();

	while(1)
	{

		//Switch-2 for LED selection
		uiStatusSW2Final = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0);
		uiStatusSW2Final = (uiStatusSW2Final  & 0x01);
		if((uiStatusSW2Final == 0)&&(lock_flag2==0))
		{
			lock_flag2 =1;
			uicountSW2++;
			if(uicountSW2>2)
				uicountSW2 = 0;

		}

		if(uiStatusSW2Final == 1)
		lock_flag2 = 0;


		//Switch-1 for delay selection
		uiStatusSW1Final = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4);
		uiStatusSW1Final = (uiStatusSW1Final  & 0x10)>>4;
		if(uiStatusSW1Final==0)
		{
			debounce++;
			if(debounce >= 2)
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


//delay selection
		switch(uicount)
		{

		case 0 :

			if(uiStatusSW1==0)
			{
				temp = 1;
				val = 6700000/2;

			}
			else
			{
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
				val = 6700000;

			}
			else
			{
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
				val = 6700000*2;

			}
			else
			{
				if(temp){
						uicount = 0;
						temp = 0;
						}
			}

			break;
		}


#if 1
// LED selection
		if(uicountSW2==0)
		{
			if(lock)
			{
				lock =0;
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 2);
			}
			else
			{
				lock =1;
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);

			}
		}


		if(uicountSW2==1)
		{
			if(lock)
			{
				lock =0;
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 4);
			}
			else
			{
				lock =1;
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);

			}
		}

		if(uicountSW2==2)
		{
			if(lock)
			{
				lock =0;
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 8);
			}
			else
			{
				lock =1;
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);

			}
		}
		SysCtlDelay(val);

	}
#endif
}
