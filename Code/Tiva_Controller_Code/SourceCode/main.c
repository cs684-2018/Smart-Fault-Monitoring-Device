
/*
*
 * Team Id: <6: Smart fault monitoring device>
 * Author List: Anil Kumar Garg, Abhishek Pal
 * Filename: main.c
 * Functions: ADC0handler, initADC, InitConsole, InitUART1, main, tier0init, Timer0intHandler
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/timer.h"
#include "DAQ.h"
#include "utils/uartstdio.h"


//* Global Variables: <List of global variables defined in this file, None if no global variables>

// define the peripheral/port/pins for the LEDs
#define LED_PERIF	SYSCTL_PERIPH_GPIOF
#define LED_PORT	GPIO_PORTF_BASE
#define LED_RED		GPIO_PIN_1
#define LED_GREEN	GPIO_PIN_3
#define LED_BLUE	GPIO_PIN_2
#define LEDS		(LED_RED|LED_GREEN|LED_BLUE)
#define	LED_SET(X)	 GPIOPinWrite(LED_PORT, LEDS, (X));
#define UART_Debug_enable 0

uint8_t Status=0;
uint8_t Current_High=0;
uint8_t Current_Low=0;
uint8_t Voltage1=0;
uint8_t Voltage2=0;

// this array must be big enough to hold the channels we convert
uint32_t rawtemp[4];
uint8_t ui8LED=0;

// this flag is set by the interrupt routine, and read by the wait loop
volatile uint32_t newtemp;
void timer0init(void);


//Testing array
uint16_t array[] = {
		1936,
		2268,
		2436,
		2512,
		2282,
		2084,
		1646,
		1203,
		858,
		641,
		444,
		370,
		596,
		863,
		1177,
		1518
};



/*
*
* Function Name: ADC0handler
* Input: NA
* Output: NA
* Logic: // this is the interrupt routine for the ADC
*
*/

void ADC0handler(void)
{
	// clear the interrupt flag, grab the data, set the 'done' flag
	ADCIntClear(ADC0_BASE, 1);
	ADCSequenceDataGet(ADC0_BASE, 1, rawtemp);
	g_iADCBuffer[ADC_CRNT]=rawtemp[0];
	g_iADCBuffer[ADC_VOLT1]=rawtemp[1];
	g_iADCBuffer[ADC_VOLT2]=rawtemp[2];

	PerSampleActivity();
	newtemp = 1;
}



/*
*
* Function Name: InitADC
* Input: NA
* Output: NA
* Logic: // init ADC
*
*/

void InitADC(void)
{
	// enable ADC0
	// set a slower conversion rate
	// use sequencer 1 (single sample), with manual trigger
	// seq 1, step 0, CH#, interrupt-enable, last-conversion
	// enable sequencer 1
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0); //Adc0
	SysCtlDelay(2);
	// Set ADC0 Sequencer, Priority , Ch#
	ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);

	ADCHardwareOversampleConfigure(ADC0_BASE, 16); // 64 32 16 8 4 2

	ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH7);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH6);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH5);
	ADCSequenceStepConfigure(ADC0_BASE,1,3,ADC_CTL_CH5|ADC_CTL_IE|ADC_CTL_END);
	ADCSequenceEnable(ADC0_BASE, 1);

	// make sure interrupt flag is clear
	// enable the interrupt for the module and for the sequence
	ADCIntClear(ADC0_BASE, 1);
	IntEnable(INT_ADC0SS1);
	ADCIntEnable(ADC0_BASE, 1);

}

/*
*
* Function Name: InitConsole
* Input: NA
* Output: NA
* Logic: // init UART0
*
*/

void InitConsole(void)
{
	// enable the port that has UART0 pins
	// configure the RX/TX port pins
	// enable the uart, set the clock, set the pins
	// set baud-rate
	 SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	 SysCtlDelay(2);
	 GPIOPinConfigure(GPIO_PA0_U0RX);
	 GPIOPinConfigure(GPIO_PA1_U0TX);
	 SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	 UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
	 GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	 UARTStdioConfig(0, 115200, 16000000);
}


/*
*
* Function Name: InitConsoleUART1
* Input: NA
* Output: NA
* Logic: // init UART1
*
*/
void InitConsoleUART1(void)
{
	// enable the port that has UART0 pins
	// configure the RX/TX port pins
	// enable the uart, set the clock, set the pins
	// set baud-rate
	 SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	 SysCtlDelay(2);
	 GPIOPinConfigure(GPIO_PB0_U1RX);
	 GPIOPinConfigure(GPIO_PB1_U1TX);
	 SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
	 UARTClockSourceSet(UART1_BASE, UART_CLOCK_PIOSC);
	 GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	 UARTStdioConfig(1, 9600, 16000000);
}


/*
*
* Function Name: main
* Input: NA
* Output: NA
* Logic: // init peripheral and send data to ESP8266
*
*/

int main(void)	{
	uint32_t i;

	// clock at 8 MHz from the crystal
	SysCtlClockSet(SYSCTL_SYSDIV_2 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlDelay(2);

	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

	init_MeasurementTask();

#if UART_Debug_enable
	InitConsole();
#else
	InitConsoleUART1();
#endif

	InitADC();
	timer0init();

#if UART_Debug_enable
	UARTprintf("\n\nAnalog Input Signal\n");
#endif

	// allow interrupts
	IntMasterEnable();
	i = 0;

	while ( 1 ) {
		// delay to slow down the loop to something around 1KHz
		for(i=0;i<2000;i++)
		{
			SysCtlDelay( SysCtlClockGet()*3/8000);
		}

	if(g_stFlags.bCycleOver)
	DAQ_20msTask();

	if(g_stFlags.bTripFlag==1)
	{
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 2);
		Status=1;
	#if UART_Debug_enable
		UARTprintf("- MCB Tripped!");
	#endif

	}
	else if(g_stFlags.bTripFlag==2)
	{
	#if UART_Debug_enable
		UARTprintf("- NO Power!  ");
	#endif
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 4);
		g_stFlags.bSendmsgFlag=1;
		Status=2;
	}
	else
	{
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 8);
	#if UART_Debug_enable
		UARTprintf("- Power OK!  ");
	#endif

		Status=0;
		g_stFlags.bSendmsgFlag=1;
	}
	Current_High=(uint8_t)(g_stTrippedMetVar.m_ulRMS[ADC_CRNT]>>8);
	Current_Low=(uint8_t)g_stTrippedMetVar.m_ulRMS[ADC_CRNT];
	Voltage1=(uint8_t)g_stTrippedMetVar.m_ulRMS[ADC_VOLT1];
	Voltage2=(uint8_t)g_stTrippedMetVar.m_ulRMS[ADC_VOLT2];


#if UART_Debug_enable
	UARTprintf("\n\n Vp$%4d V Vs$%4d V  I$%3d mA ",
	g_stTrippedMetVar.m_ulRMS[ADC_VOLT1], g_stTrippedMetVar.m_ulRMS[ADC_VOLT2], g_stTrippedMetVar.m_ulRMS[ADC_CRNT]);

#else

	UARTCharPut(UART1_BASE, Status);

	SysCtlDelay( SysCtlClockGet()*3/8000);
	SysCtlDelay( SysCtlClockGet()*3/8000);
	SysCtlDelay( SysCtlClockGet()*3/8000);

	UARTCharPut(UART1_BASE, Current_High);

	SysCtlDelay( SysCtlClockGet()*3/8000);
	SysCtlDelay( SysCtlClockGet()*3/8000);
	SysCtlDelay( SysCtlClockGet()*3/8000);

	UARTCharPut(UART1_BASE, Current_Low);

	SysCtlDelay( SysCtlClockGet()*3/8000);
	SysCtlDelay( SysCtlClockGet()*3/8000);
	SysCtlDelay( SysCtlClockGet()*3/8000);

	UARTCharPut(UART1_BASE, Voltage1);

	SysCtlDelay( SysCtlClockGet()*3/8000);
	SysCtlDelay( SysCtlClockGet()*3/8000);
	SysCtlDelay( SysCtlClockGet()*3/8000);

	UARTCharPut(UART1_BASE, Voltage2);

	SysCtlDelay( SysCtlClockGet()*3/8000);
	SysCtlDelay( SysCtlClockGet()*3/8000);
	SysCtlDelay( SysCtlClockGet()*3/8000);
	UARTCharPut(UART1_BASE, '\n');

#endif

	}
}


void Timer0IntHandler(void)
{
// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	// start a conversion
	 ADCProcessorTrigger(ADC0_BASE, 1);

	 uiCounter++;// 4000 for 5 sec
	 if(uiCounter > 4000)
	 {
		 g_stFlags.bTimeoutFlag =1;
		 uiCounter =0;
	 }
}


/*
*
* Function Name: timer0init
* Input: NA
* Output: NA
* Logic: // init timer0init
*
*/

void timer0init(void)
{
	uint32_t ui32Period;

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

	ui32Period = (SysCtlClockGet() / 400) / 2; // Freq = 50*16 Hz

	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);

	IntEnable(INT_TIMER0A);

	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	IntMasterEnable();

	TimerEnable(TIMER0_BASE, TIMER_A);
}
