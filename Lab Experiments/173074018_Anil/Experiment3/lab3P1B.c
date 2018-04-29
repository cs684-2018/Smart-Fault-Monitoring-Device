
/* 

Name: Anil Kumar Garg : 173074018
Name: Abhishek Pal : 173074015

Course Name: CS684-2018
*/

/*
1. In Manual mode, user must be able to select any one of the color from the color
circle. For this intensity of any of the 3 LEDs must be controlled independently.


2. Mode 1 (Red LED control) - When SW2 is pressed continuously(long press) and
SW1 is pressed once controller goes to Manual Mode 1. In this mode, intensity
of Red LED can be controlled using SW1 and SW2.


3. Mode 2 (Blue LED control) - When SW2 is pressed continuously(long press) and
SW1 is pressed twice controller goes to Manual Mode 2. In this mode, intensity
of Blue LED can be controlled using SW1 and SW2.

4. Mode 3 (Green LED control) - When SW2 is pressed continuously(long press) and
SW1 is pressed three times, controller goes to Manual Mode 2. In this mode, intensity of Green LED can be
controlled using SW1 and SW2.


*/




#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "inc/hw_ints.h"

#define PWM_FREQUENCY 40	//Hz
#define DEBOUNCE_TIME 40	//in mS
#define LONG_PRESS_TIME_THR 2	//in Sec


#define KEY_IDLE_STATE 	0
#define KEY_PRESS_STATE 	1
#define KEY_RELEASE_STATE 	2
#define SWCLOSE	0
#define SWOPEN	1

volatile uint8_t uint8Key1State=0, uint8Key2State=0;
volatile uint8_t ucKey1Sets, ucKey1Flag=0, ucKey2Sets, ucKey2Flag=0;
volatile uint8_t ucSw2LongPressDetectFlag, ucSw1PressCount, ucKeyChkEnableFlag;
volatile uint32_t ulLongPressCount;

volatile uint8_t ucTransitionSpeed;
volatile uint8_t ucTimer2Flag,ucTempCounter =1;
volatile uint32_t ulTimer2LoadCnt;

volatile uint32_t ulPwmClockFreq, ucPwmPeriodCount, ulPwmPulseWidthCount;

uint8_t ucRbrightness, ucGbrightness, ucBbrightness;

#define AUTOMODE 0
#define MODE1	1
#define MODE2	2
#define MODE3	3

#define HighR 	0
#define LOWG	1
#define HIGHB	2

#define LOWR	3
#define HIGHG	4
#define LOWB	5

unsigned int operation_mode = AUTOMODE,led_state = HighR;

void AutoModeProg();
void mode_1_prog();
void mode_2_prog();
void mode_3_prog();
void AutoModeKeyFunc();
void Mode1KeyFunc();
void Mode2KeyFunc();
void Mode3KeyFunc();
/////////////////////////////////////////////////////////////////////////


int main(void)
{
	uint32_t debounce_time_count;
	uint8_t sw2_key_chk;
	operation_mode = AUTOMODE; 	//default mode
	led_state = HighR;
	ucRbrightness = ucGbrightness = ucBbrightness = 2;
	ucTransitionSpeed = 50;
	ucTimer2Flag=0;
	ucSw2LongPressDetectFlag =0;
	ucSw1PressCount=0;

	//setup();
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
	GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	//	timer0init();
	ucKeyChkEnableFlag=0;
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	debounce_time_count = (SysCtlClockGet() * DEBOUNCE_TIME / 1000) / 2;
	TimerLoadSet(TIMER0_BASE, TIMER_A, debounce_time_count - 1);
	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();
	TimerEnable(TIMER0_BASE, TIMER_A);


	//timer1init();
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
	TimerConfigure(TIMER1_BASE, TIMER_CFG_ONE_SHOT);
	ulLongPressCount = (SysCtlClockGet() * LONG_PRESS_TIME_THR);
	TimerLoadSet(TIMER1_BASE, TIMER_A, ulLongPressCount - 1);
	IntEnable(INT_TIMER1A);
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();


	//timer2init();
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
	TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);
	ulTimer2LoadCnt = (SysCtlClockGet() / ucTransitionSpeed) / 2;
	TimerLoadSet(TIMER2_BASE, TIMER_A, ulTimer2LoadCnt);
	IntEnable(INT_TIMER2A);
	TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();
	TimerEnable(TIMER2_BASE, TIMER_A);


	//PWM_init();

	SysCtlPWMClockSet(SYSCTL_PWMDIV_64);	//PWM clock frequency input is system freq/64

	//Calculation of load count for PWM
	ulPwmClockFreq = SysCtlClockGet() / 64;
	ucPwmPeriodCount = (ulPwmClockFreq / (PWM_FREQUENCY)) - 1; //ucPwmPeriodCount correspons to the Time period of the output PWM

	//Port pin PF1 configure for PWM; RED LED
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);
	GPIOPinConfigure(GPIO_PF1_M1PWM5);
	PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, ucPwmPeriodCount);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ucRbrightness * ucPwmPeriodCount / 256);
	PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, true);
	PWMGenEnable(PWM1_BASE, PWM_GEN_2);

	//Port pin PF2 configure for PWM; Blue LED
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2);
	GPIOPinConfigure(GPIO_PF2_M1PWM6);
	PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, ucPwmPeriodCount);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ucBbrightness * ucPwmPeriodCount / 256);
	PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, true);
	PWMGenEnable(PWM1_BASE, PWM_GEN_3);

	//Port pin PF2 configure for PWM; Green LED
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_3);
	GPIOPinConfigure(GPIO_PF3_M1PWM7);
	PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, ucPwmPeriodCount);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ucGbrightness * ucPwmPeriodCount / 256);
	PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, true);
	PWMGenEnable(PWM1_BASE, PWM_GEN_3);


	while(1)
	{
		while(!ucSw2LongPressDetectFlag)
		{
			if(ucTimer2Flag)
			{
				ucTimer2Flag=0;
				switch(operation_mode)
				{
					case AUTOMODE:
						AutoModeProg();
						break;
					case MODE1:	//Control RED LED brightness
						//mode_1_prog();
						//ucGbrightness = 1;
						//ucBbrightness = 1;
						PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, true);		//Enable R LED PWM output
						PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, false);	//Disableable B LED PWM output
						PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, false);	//Disable G LED PWM output
						break;
					case MODE2:	//Control Blue LED brightness
						//mode_2_prog();
						//ucGbrightness = 1;
						//ucRbrightness = 1;
						PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, false);	//Disable R LED PWM output
						PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, true);		//Enable B LED PWM output
						PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, false);	//Disable G LED PWM output
						break;
					case MODE3:	////Control Green LED brightness
						//mode_3_prog();
						//ucRbrightness = 1;
						//ucBbrightness = 1;
						PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, false);	//Disable R LED PWM output
						PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, false);	//Disable B LED PWM output
						PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, true);		//Enable G LED PWM output
				}

				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ucRbrightness * ucPwmPeriodCount / 256);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ucBbrightness * ucPwmPeriodCount / 256);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ucGbrightness * ucPwmPeriodCount / 256);
			}
			switch(operation_mode)
			{
				case AUTOMODE:
					AutoModeKeyFunc();
					break;
				case MODE1:
					Mode1KeyFunc();
					break;
				case MODE2:
					Mode2KeyFunc();
					break;
				case MODE3:
					Mode3KeyFunc();
			}
		}

		//Indicate long press detect by put off all LEDs
		PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, false);
		PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, false);
		PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, false);

		do
		{
			sw2_key_chk = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0);
			sw2_key_chk = (sw2_key_chk & 0x01);
		}while(!sw2_key_chk);

		switch(ucSw1PressCount)
		{
			case 1:
				operation_mode = MODE1;
				break;
			case 2:
				operation_mode = MODE2;
				break;
			case 3:
				operation_mode = MODE3;
		}
		ucSw1PressCount =0;
		ucSw2LongPressDetectFlag =0;

		//Enable all LEDs output
		PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, true);
		PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, true);
		PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, true);
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////
void AutoModeProg()
{
	switch(led_state)
	{
		case HighR :
			ucRbrightness++;
			if(ucRbrightness>=254)
			{
				led_state  = LOWG;
			}
			break;

		case LOWG :
			ucGbrightness--;
			if(ucGbrightness <= 1)
			{
				led_state  = HIGHB;
			}
			break;

		case HIGHB :
			ucBbrightness++;
			if(ucBbrightness>=254)
			{
				led_state  = LOWR;
			}
			break;

		case LOWR :
			ucRbrightness--;
			if(ucRbrightness <= 1)
			{
				led_state  = HIGHG;
			}
			break;

		case HIGHG :
			ucGbrightness++;
			if(ucGbrightness>=254)
			{
				led_state  = LOWB;
			}
			break;

		case LOWB :
			ucBbrightness--;
			if(ucBbrightness <= 1)
			{
				led_state  = HighR;
			}
		}
}
//////////////////////////////////////////////////////////////////
void AutoModeKeyFunc()
{
	if(ucKeyChkEnableFlag)
	{
		ucKeyChkEnableFlag=0;
		if(ucKey1Flag)
		{
			ucTransitionSpeed--;
			if (ucTransitionSpeed < 10)
			{
				ucTransitionSpeed = 10;//10
			}
			ulTimer2LoadCnt = (SysCtlClockGet() / ucTransitionSpeed) / 2;
			TimerLoadSet(TIMER2_BASE, TIMER_A, ulTimer2LoadCnt);
		}

		if(ucKey2Flag)
		{
			ucTransitionSpeed++;
			if (ucTransitionSpeed > 250)
			{
				ucTransitionSpeed = 250;//111
			}
			ulTimer2LoadCnt = (SysCtlClockGet() / ucTransitionSpeed) / 2;
			TimerLoadSet(TIMER2_BASE, TIMER_A, ulTimer2LoadCnt);
		}
	}
}
//////////////////////////////////////////////////////////////////
void Mode1KeyFunc()
{
	if(ucKeyChkEnableFlag)
	{
		ucKeyChkEnableFlag=0;
		if(ucKey1Flag)
		{
			ucRbrightness++;
			if(ucRbrightness>=254)
			{
				ucRbrightness=254;
			}
		}

		if(ucKey2Flag)
		{
			ucRbrightness--;
			if(ucRbrightness<=2)
			{
				ucRbrightness=2;
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void Mode2KeyFunc()
{
	if(ucKeyChkEnableFlag)
	{
		ucKeyChkEnableFlag=0;
		if(ucKey1Flag)
		{
			ucBbrightness++;
			if(ucBbrightness>=254)
			{
				ucBbrightness=254;
			}
		}

		if(ucKey2Flag)
		{
			ucBbrightness--;
			if(ucBbrightness<=2)
			{
				ucBbrightness=2;
			}
		}
	}
}

//////////////////////////////////////////////////////
void Mode3KeyFunc()
{
	if(ucKeyChkEnableFlag)
	{
		ucKeyChkEnableFlag=0;
		if(ucKey1Flag)
		{
			ucGbrightness++;
			if(ucGbrightness>=254)
			{
				ucGbrightness=254;
			}
		}

		if(ucKey2Flag)
		{
			ucGbrightness--;
			if(ucGbrightness<=2)
			{
				ucGbrightness=2;
			}
		}
	}
}

void detectKeyPress_1(void)
{
	ucKey1Sets = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4);
	ucKey1Sets = (ucKey1Sets & 0x10)>>4;

	switch(uint8Key1State)
	{
	case KEY_IDLE_STATE :
		if(ucKey1Sets==SWCLOSE)
		{
			uint8Key1State = KEY_PRESS_STATE;
		}
		break;

	case KEY_PRESS_STATE :
		if(ucKey1Sets==SWCLOSE)
		{
			uint8Key1State = KEY_RELEASE_STATE;
			ucKey1Flag = 1;
			if(ucSw2LongPressDetectFlag)
			{
				ucSw1PressCount++;
			}
		}
		else
		{
			uint8Key1State = KEY_IDLE_STATE;
		}
		break;

	case KEY_RELEASE_STATE :

		if(ucKey1Sets==SWOPEN)
		{
			uint8Key1State = KEY_IDLE_STATE;
			ucKey1Flag = 0;
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////

void detectKeyPress_2(void)
{
	ucKey2Sets = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0);
	ucKey2Sets = (ucKey2Sets & 0x01);

	switch(uint8Key2State)
	{
	case KEY_IDLE_STATE :
		if(ucKey2Sets==SWCLOSE)
		{
			uint8Key2State = KEY_PRESS_STATE;
		}
		break;

	case KEY_PRESS_STATE :
		if(ucKey2Sets==SWCLOSE)
		{
			uint8Key2State = KEY_RELEASE_STATE;
			ucKey2Flag = 1;
			TimerLoadSet(TIMER1_BASE, TIMER_A, ulLongPressCount);
			TimerEnable(TIMER1_BASE, TIMER_A);
		}
		else
		{
			uint8Key2State = KEY_IDLE_STATE;
		}
		break;

	case KEY_RELEASE_STATE :

		if(ucKey2Sets==SWOPEN)
		{
			uint8Key2State = KEY_IDLE_STATE;
			ucKey2Flag = 0;
			TimerDisable(TIMER1_BASE, TIMER_A);
		}
	}
}
////////////////////////////////////////////////////////////////////////////////
void Timer0IntHandler(void)
{
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	detectKeyPress_1();
	detectKeyPress_2();
	ucKeyChkEnableFlag=1;
}
///////////////////////////////////////////////////////////////////////////////
void Timer1IntHandler(void)
{
	TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	ucSw2LongPressDetectFlag = 1;

}
///////////////////////////////////////////////////////////////////////////////
void Timer2IntHandler(void)
{
	TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

	ucTimer2Flag =1;

}
//////////////////////////////////////////////////////////////////////////////

