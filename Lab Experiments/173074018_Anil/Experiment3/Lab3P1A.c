/*

Name: Anil Kumar Garg : 173074018
Name: Abhishek Pal : 173074015

Course Name: CS684-2018

Problem: 
1. In Auto mode color of the RGB LED follows a pattern in a cycle.
2. The pattern must follow the color circle as shown in Figure 1.
3. In Auto mode SW1 will increase the speed of color transition and SW2 will
decrease the speed.

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


#define PWM_FREQUENCY 55

#define RB 0
#define BY 1
#define YR 2

void timer0init(void);
uint32_t ui32Period;
uint16_t timedelay =0;
uint8_t counter=1,TempCounter =1;

int main(void)
{
		unsigned int RedVal=255,BlueVal=0,GreenVal =0,state =  0;
		volatile uint32_t ui32Load;
		volatile uint32_t ui32PWMClock;
		volatile uint8_t ui8Adjust;
		ui8Adjust = 254;

		SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
		SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

		timer0init();


		/************GPIO F-1configuration ****************/
		/*******Start*********/
		SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
		GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);
		GPIOPinConfigure(GPIO_PF1_M1PWM5);
		HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
		HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
		HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
		GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);

		GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
		ui32PWMClock = SysCtlClockGet() / 64;
		ui32Load = (ui32PWMClock / (PWM_FREQUENCY)) - 1;
		PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN);
		PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, ui32Load);
		PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, true);
		PWMGenEnable(PWM1_BASE, PWM_GEN_2);
		/******End*********/


		/************GPIO F-2configuration ****************/
		/*******Start*********/
		GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2);
		GPIOPinConfigure(GPIO_PF2_M1PWM6);
		PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN);
		PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, ui32Load);
		PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, true);
		PWMGenEnable(PWM1_BASE, PWM_GEN_3);
		/******End*********/

		/************GPIO F-3configuration ****************/
		/*******Start*********/
		SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
		GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_3);
		GPIOPinConfigure(GPIO_PF3_M1PWM7);
		PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN);
		PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, ui32Load);
		PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, true);
		PWMGenEnable(PWM1_BASE, PWM_GEN_3);
			/******End*********/
	while(1)
	{
	switch(state)
	{
		case RB :
			RedVal = 255-counter;
			BlueVal =counter;
			GreenVal =1;
			if(BlueVal>250)
			{
			counter =1;
			state  = BY;
			}
			break;

		case BY :

			BlueVal= 255-counter;
			GreenVal =counter;
			RedVal =1;
			if(GreenVal>250)
			{
				state  = YR;
				counter =1;
				}
			break;

		case YR :
			GreenVal= 255-counter;
			RedVal =counter;
			BlueVal =1;
			if(RedVal>250)
			{
			state  = RB;
			counter =1;
			}
			break;
		}

	if(TempCounter >2)
	{
	TempCounter =0;
		PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, GreenVal * ui32Load / 1000);

		PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, BlueVal * ui32Load / 1000);

		PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, RedVal * ui32Load / 1000);

	}
		if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)
		{
			timedelay--;
			if (timedelay < 10)
			{
				timedelay = 10;//10
			}
			ui32Period = (SysCtlClockGet() / timedelay) / 2;
			TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);
		}

		if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00)
		{
			timedelay++;
			if (timedelay > 200)
			{
				timedelay = 200;//111
			}
			ui32Period = (SysCtlClockGet() / timedelay) / 2;
			TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);
		}

		SysCtlDelay(100000);
	}

}



void Timer0IntHandler(void)
{

	// Clear the timer interrupt
		TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

		TempCounter++;

		counter++;

		if(counter>254)
			counter =1;
}


void timer0init(void)
{
	uint32_t ui32Period;

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

	timedelay = 50;
	ui32Period = (SysCtlClockGet() / timedelay) / 2;

	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);

	IntEnable(INT_TIMER0A);

	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	IntMasterEnable();

	TimerEnable(TIMER0_BASE, TIMER_A);
}

void Timer1IntHandler(void)
{

}


void Timer2IntHandler(void)
{

}
