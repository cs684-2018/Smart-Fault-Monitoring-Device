

/*
 * Lab 5 : Animation using motion image load
 * Submitted By: Abhishek Pal: Roll# 173074015
 *			  Anil Garg : Roll# 173074018
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/debug.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"
#include "driverlib/adc.h"

unsigned long logo_delay;
void ADC_init(void);
uint32_t ui32ADC0Value[4];
uint32_t ui32ADC1Value[4];

volatile uint32_t ui32Avg0;
volatile uint32_t copy0;

volatile uint32_t ui32Avg1;
volatile uint32_t copy1;

void check_adc(void);

void glcd_logodisplay(unsigned char*);
extern  unsigned char one[];
extern  unsigned char two[];
extern  unsigned char three[];
extern  unsigned char four[];
extern  unsigned char five[];
extern  unsigned char six[];
extern  unsigned char seven[];
extern  unsigned char eight[];



void sendImage(void );
void glcd_cleardisplay();
void glcd_setcolumn(unsigned char );
void glcd_init();
void glcd_data(unsigned char );
void glcd_cmd(unsigned char );
void glcd_setpage(unsigned char );
void Timer0IntHandler(void);



/*To display image include an array with hex values and index it accordingly*/
#include "part1/mickey.h"
#include "part1/logo.h"


/* void glcd_cmd(cmd)
 * This function sends commands to the GLCD.
 * Value of RS is 0
 * Command is written on data lines.
 * Enable is made 1 for a short duration.
 */
void glcd_cmd(unsigned char cmd)
{
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,0x00);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7,0x00);

	/* RS = 0 */
	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6,0x00);

	/* Put command on data lines */
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,cmd);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7,cmd);

	/* Generate a high to low pulse on enable */
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0,0x01);
	SysCtlDelay(100);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0,0x00);

}


/* void glcd_data(data)
 * This function sends data to the GLCD.
 * Value of RS is 1
 * Data is written on data lines.
 * Enable is made 1 for a short duration.
 */
void glcd_data(unsigned char data)
{
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,0x00);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7,0x00);

	/* RS = 1 */
	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6,0x40);

	/* Put command on data lines */
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,data);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7,data);

	/* Generate a high to low pulse on enable */
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0,0x01);
	SysCtlDelay(100);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0,0x00);
}


/* void glcd_init()
 * This function initializes the GLCD.
 * Always call this function at the beginning of main program after configuring the port pins.
 */
void glcd_init()
{
	SysCtlDelay(6700000/50);                            // creates ~10ms delay - TivaWare fxn
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, 0x00);    //cbi(GPORTC_2,GLCD_RST);
	SysCtlDelay(6700000/50);
	/* Set RST */
	GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5,0x20);

	/* Set CS1 (CS1=1 and CS2=0) The right side is selected(column>64) */
	GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_3,0x00);

	/* Select the start line */
	glcd_cmd(0xC0);
	//SysCtlDelay(6700);

	/* Send the page */
	glcd_cmd(0xB8);
	//  SysCtlDelay(6700);

	/*Send the column */
	glcd_cmd(0x40);
	//SysCtlDelay(6700);

	/* Send glcd on command */
	glcd_cmd(0x3F);


	/* Initialize the right side of GLCD */
	/* Set CS2 (CS2=1 and CS1=0) The right side is selected(column>64) */
	GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_3,0x08);

	/* Select the start line */
	glcd_cmd(0xC0);
	SysCtlDelay(6700);

	/* Send the page */
	glcd_cmd(0xB8);
	//  SysCtlDelay(6700);

	/*Send the column */
	glcd_cmd(0x40);
	//  SysCtlDelay(6700);

	/* Send glcd on command */
	glcd_cmd(0x3F);
	//  SysCtlDelay(6700);
}




/* void glcd_setpage(page)
 * This function selects page number on GLCD.
 * Depending on the value of column number CS1 or CS2 is made high.
 */
void glcd_setpage(unsigned char page)
{
	/* Set CS1 (CS1=1 and CS2=0) The right side is selected(column>64) */
	GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_3,0x00);
	glcd_cmd(0xB8 | page);
	SysCtlDelay(100);

	/* Set CS2 (CS2=1 and CS1=0) The right side is selected(column>64) */
	GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_3,0x08);
	glcd_cmd(0xB8 | page);
	SysCtlDelay(100);
}




/* void glcd_setcolumn(column)
 * This function selects column number on GLCD.
 * Depending on the value of column number CS1 or CS2 is made high.
 */
void glcd_setcolumn(unsigned char column)
{
	if(column < 64)
	{
		/* Set CS1 (CS1=1 and CS2=0) The right side is selected(column>64) */
		GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_3,0x00);
		glcd_cmd(0x40 | column);
		SysCtlDelay(100);
	}
	else
	{
		/* Set CS2 (CS2=1 and CS1=0) The right side is selected(column>64) */
		GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_3,0x08);
		glcd_cmd(0x40 | (column-64));
		SysCtlDelay(100);
	}

}


/* void glcd_cleardisplay()
 * This function clears the data on GLCD by writing 0 on all pixels.
 */
void glcd_cleardisplay()
{
	unsigned char i,j;
	for(i=0;i<8;i++)
	{
		glcd_setpage(i);
		for(j=0;j<128;j++)
		{
			glcd_setcolumn(j);
			glcd_data(0x00);
		}
	}
}

int main()
{
	/* Enable all the peripherals */
	/* PORTS A,E,F,C,D,B */
	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

	ADC_init();

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

	/* Unlock pin PF0 */
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK)= GPIO_LOCK_KEY;    // unlocking sw2 switch
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK)= 0;

	/* Configure Enable pin as output */
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);



	/*This ensures buzzer remains OFF, since PC4 when logic 0 turns ON buzzer */
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_4);
	GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_4,16);

	/* Configure PE5 (RST), PE0 to PE3 (D0 to D3) and PB4 to PB7(D4 to D7) as output pins */
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_5 | GPIO_PIN_3| GPIO_PIN_2 | GPIO_PIN_1 |GPIO_PIN_0);
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_7| GPIO_PIN_6 | GPIO_PIN_5 |GPIO_PIN_4);

	/* Configure RS as output : PC6 */
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_6 );

	/* Configure CS1 or CS2 as output PD3 */
	GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_3 );

	/*initialize glcd*/
	glcd_init();

	/* Select a page and display lines on it from column 0 to 127 */

	//sendImage(&mickey);
//	sendImage(&logo);

	logo_delay = 6700000/5;//67000000/5;

	while(1)
	{
		check_adc();
		sendImage();
	}
}


/*
* Initialize the Adc Channels to read up/down & left/right
* analog position of Joystick
*/
void ADC_init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	 SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);

	// ADCHardwareOversampleConfigure(ADC0_BASE, 64);

	 ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
	 ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH6);
	 ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH6);
	 ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH6);
	 ADCSequenceStepConfigure(ADC0_BASE,1,3,ADC_CTL_CH6|ADC_CTL_IE|ADC_CTL_END);
	 ADCSequenceEnable(ADC0_BASE, 1);

	 ADCSequenceConfigure(ADC1_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
	 ADCSequenceStepConfigure(ADC1_BASE, 1, 0, ADC_CTL_CH7);
	 ADCSequenceStepConfigure(ADC1_BASE, 1, 1, ADC_CTL_CH7);
	 ADCSequenceStepConfigure(ADC1_BASE, 1, 2, ADC_CTL_CH7);
	 ADCSequenceStepConfigure(ADC1_BASE,1,3,ADC_CTL_CH7|ADC_CTL_IE|ADC_CTL_END);
	 ADCSequenceEnable(ADC1_BASE, 1);


}

/*
Read Adc data from both channels
*/

void check_adc(void)
{
	 ADCIntClear(ADC0_BASE, 1);
	 ADCProcessorTrigger(ADC0_BASE, 1);
	 ADCIntClear(ADC1_BASE, 1);
	 ADCProcessorTrigger(ADC1_BASE, 1);

	while(!ADCIntStatus(ADC0_BASE, 1, false))
	{
	}
	while(!ADCIntStatus(ADC1_BASE, 1, false))
	{
	}
	 ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);
	 ADCSequenceDataGet(ADC1_BASE, 1, ui32ADC1Value);

	ui32Avg0 = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2)/4;
	copy0=ui32Avg0;
	ui32Avg1 = (ui32ADC1Value[0] + ui32ADC1Value[1] + ui32ADC1Value[2] + ui32ADC1Value[3] + 2)/4;
	copy1=ui32Avg1;
	logo_delay = (670000/2)+ (copy0*1000);
	
}

void glcd_logodisplay(unsigned char* start_address)
{
    unsigned char i,j;
    for(i=0;i<8;i++)
    {
        glcd_setpage(i);
        for(j=0;j<128;j++)
        {
            glcd_setcolumn(j);
            glcd_data(~start_address[(i*128)+j]);
        }
    }
}

/*
* Send image data in a from 8 images one by one
* delay between load is controlled by 'logo_delay'
* 'logo_delay'is adjusted using left/right Joystick
*/
void sendImage(void )
{
    	glcd_logodisplay(&one[0]);
    	SysCtlDelay(logo_delay);

       	glcd_logodisplay(&two[0]);
    	SysCtlDelay(logo_delay);

       	glcd_logodisplay(&three[0]);
    	SysCtlDelay(logo_delay);

       	glcd_logodisplay(&four[0]);
    	SysCtlDelay(logo_delay);

       	glcd_logodisplay(&five[0]);
    	SysCtlDelay(logo_delay);

       	glcd_logodisplay(&six[0]);
    	SysCtlDelay(logo_delay);

       	glcd_logodisplay(&seven[0]);
    	SysCtlDelay(logo_delay);

       	glcd_logodisplay(&eight[0]);
    	SysCtlDelay(logo_delay);
}

void Timer0IntHandler(void)
{

}


