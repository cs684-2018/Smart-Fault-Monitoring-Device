/*
Name: Anil Garg 173074018
Name: Abhishek Pal 173074015


1. Use sw1 to change the color of the led (R) G) B) R. . . .) where you should press
the switch just once instead of long press in Lab 1. Use switch debouncing mentioned
below in the procedure to diferentiate between switch bounce and actual key press.
2. Use sw2 to increment a global variable once for each button press. Check if the variable
always increments by one (adjust the time interval of 10 ms if you wish to)
*/

#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "inc/hw_gpio.h"


//variable declaration

uint32_t uiStatusSW1 = 0,uiStatusSW2 =0;
uint8_t KeyPressState = 0,uiTimeoutFlag=0,uiTimeoutCounter=0,uiTimerstartFlag=0,uiStatusSW1ReleaseLockFlag=0;
uint8_t Key2PressState = 0,uiStatusSW2ReleaseLockFlag=0;

uint8_t ucLEDState =0,ucSW2Counter=0;
void led_pin_config(void);
void SwitchStateMachine(void);
void timer0init(void);
void StateMachineLED(void);



/***************************/
unsigned char  Switch1StateMachine(void);
unsigned char  Switch2StateMachine(void);


#define DebounceDelay 	5

#define KeyIdle 	0
#define KeyPress 	1
#define KeyRelease 	2

#define R 0
#define G 1
#define B 2


void led_pin_config(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);

	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK)= GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK)= 0 ;

	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
}

void timer0init(void)
{
	uint32_t ui32Period;

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

	ui32Period = (SysCtlClockGet() / 100) / 2;

	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);

	IntEnable(INT_TIMER0A);

	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	IntMasterEnable();

	TimerEnable(TIMER0_BASE, TIMER_A);
}

unsigned char  Switch1StateMachine(void)
{
	unsigned char ucFlag=0;

//	uiStatusSW2 = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0);
//	uiStatusSW2 = (uiStatusSW2  & 0x01);

	uiStatusSW1 = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4);
	uiStatusSW1 = (uiStatusSW1  & 0x10)>>4;


	switch(KeyPressState)
	{
	case KeyIdle :
			if(uiStatusSW1==0)
			{
				KeyPressState = KeyPress;

			}
			break;

	case KeyPress :
				if(uiStatusSW1==0)
				{
					KeyPressState = KeyRelease;
					ucFlag = 1;				}
				else
				{
					KeyPressState = KeyIdle;
				}

		break;

	case KeyRelease :

		if(uiStatusSW1==1)
		{
			KeyPressState = KeyIdle;
			ucFlag =0;		}
		else
		{
			ucFlag = 1;
		}
		break;


	default			:
		KeyPressState = KeyIdle;
		ucFlag=0;
		break;

	}
return ucFlag;
}

unsigned char  Switch2StateMachine(void)
{
	unsigned char ucFlag=0;

	uiStatusSW2 = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0);
	uiStatusSW2 = (uiStatusSW2  & 0x01);

	//uiStatusSW1 = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4);
	//uiStatusSW1 = (uiStatusSW1  & 0x10)>>4;


	switch(Key2PressState)
	{
	case KeyIdle :
			if(uiStatusSW2==0)
			{
				Key2PressState = KeyPress;

			}
			break;

	case KeyPress :
				if(uiStatusSW2==0)
				{
					Key2PressState = KeyRelease;
					ucFlag = 1;				}
				else
				{
					Key2PressState = KeyIdle;
				}

		break;

	case KeyRelease :

		if(uiStatusSW2==1)
		{
			Key2PressState = KeyIdle;
			ucFlag =0;		}
		else
		{
			ucFlag = 1;
		}
		break;


	default			:
		Key2PressState = KeyIdle;
		ucFlag=0;
		break;

	}
return ucFlag;
}

int main(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

	led_pin_config();
	timer0init();
	KeyPressState = KeyIdle;

	while(1)
		{
		}

}

void Timer0IntHandler(void)
{
	// Clear the timer interrupt
		TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

		//StateMachineLED();

		if(Switch1StateMachine())
		{

			if(uiStatusSW1ReleaseLockFlag == 0)
			{
				uiStatusSW1ReleaseLockFlag = 1;
				ucLEDState++;
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x01<<(ucLEDState));

				if(ucLEDState >2)
					ucLEDState =0;
			}

		}
		else
		{
			uiStatusSW1ReleaseLockFlag = 0;
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x0);
		}
		/************Counter on Switch2 ****************************/

		if(Switch2StateMachine())
		{

			if(uiStatusSW2ReleaseLockFlag == 0)
			{
				uiStatusSW2ReleaseLockFlag = 1;
				ucSW2Counter++;

			}

		}

		else
		{
			uiStatusSW2ReleaseLockFlag = 0;

		}


}
