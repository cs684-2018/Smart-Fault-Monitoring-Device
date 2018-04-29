/*
* Author: Texas Instruments
* Submitted by: Abhishek Pal, 173074015, EE Department, IIT Bombay

* Description: This code is for Lab 1 (Part a,b,c)
* Filename: lab-1.c
* Functions: setup(), config(), main()
* Global Variables: sw2count
*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_ints.h"
#include <time.h>


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

/*

* Function Name: led_pin_config() & sw_pin_config()
* Input: none
* Output: none
* Description: Set Port F Pin 1, Pin 2, Pin 3 as output. On this pin Red, Blue and Green LEDs are connected.
			   Set Port F Pin 0 and 4 as input, enable pull up on both these pins.

* Example Call: pin_config();
*/

void led_pin_config(void)
{
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
}

void sw_pin_config(void)
{
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK)= GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01 ;
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK)= 0 ;

    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4,GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

uint8_t check_sw1(void)   //returns 1 if switch is closed, 0 if switch is open
{
    uint8_t sw1read    = 1;
    uint8_t gpiodata = 0;
    gpiodata = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4); // |GPIO_PIN_0);
    sw1read = (gpiodata & 0x10) >> 4;

    if(sw1read == 0) //check if pin is low, meaning switch is on
    {
        SysCtlDelay(670000); // 6700000 = 500ms
        gpiodata = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4); // |GPIO_PIN_0);
        sw1read    = (gpiodata & 0x10) >> 4;
        if(sw1read == 0)return(1);   //the switch is really closed
    }
return(0);  //the switch is open
}  /* end of function */

uint8_t check_sw2(void)   //returns 1 if switch is closed, 0 if switch is open
{
    uint8_t swread    = 1;
    uint8_t gpiodata = 0;
    gpiodata = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0); // |GPIO_PIN_0);
    swread = (gpiodata & 0x01);

    if(swread == 0) //check if pin is low, meaning switch is on
    {
        SysCtlDelay(670000); // 6700000 = 500ms
        gpiodata = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0); // |GPIO_PIN_0);
        swread    = (gpiodata & 0x01);
        if(swread == 0)return(1);   //the switch is really closed
    }
return(0);  //the switch is open
}  /* end of function */


uint8_t sw2count = 0; // global var

int main(void)
{
    uint8_t ui8LED = 2;
    uint32_t delay_cnt = 6700000/2; // 250ms
    uint8_t sw1status = 0;
    uint8_t sw2status = 0;

    setup();
    led_pin_config();
    sw_pin_config();

    while(1)
    {
// Sense SW1 & SW2
//      gpiostatus = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4); // |GPIO_PIN_0);
//      sw1status = (gpiostatus & 0x10) >> 4;
        sw1status = check_sw1();
        sw2status = check_sw2();

        if(sw2status == 0x01)
        {
            sw2count++; // count SW2
            // wait till SW is ON
            while(sw2status==0x01)
            {
                sw2status = check_sw2();
            }

            // cycle through delay value for toggle rate 0.5 , 1, 2 sec
            if (delay_cnt == 6700000*2)
            {
                delay_cnt = 6700000;
            }
            else
            {
                delay_cnt = delay_cnt*2;
            }
        }

        // Cycle through Red, Green and Blue LEDs
        if(sw1status == 0x01)
        {
            // wait till SW is ON
            while(sw1status==0x01)
            {
                sw1status = check_sw1();
            }
            // cycle through LED colors
            if (ui8LED == 8)
            {
                ui8LED = 2;
            }
            else
            {
                ui8LED = ui8LED*2;
            }
        }

        // Toggle LED ON/OFF
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ui8LED);
        SysCtlDelay(delay_cnt); // 6700000 = 500ms
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);
        SysCtlDelay(delay_cnt); // 500ms
//      SysCtlDelay(20000000);  //
  }
}
