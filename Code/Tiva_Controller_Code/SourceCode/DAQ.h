
/*
*
 * Team Id: <6: Smart fault monitoring device>
 * Author List: Anil Kumar Garg, Abhishek Pal
 * Filename: DAQ.h
 * Functions: Header files
 *
 */
#ifndef LAB4_OLD_DAQ_H_
#define LAB4_OLD_DAQ_H_

#include <stdint.h>


#define	SAMPLES_PER_CYCLE	16 		 // No. of samples taken per cycle (360 deg)
#define MAX_AVG_COUNTER		16		// using in counter of 20ms*32 sec



#define Byte(x) ((uint16_t *)&x)
#define YES				1
#define NO				0


/* Changes the current ranges below 95mA = zero*/
#define NOLOAD_CURR_RMS  50
#define NOLOAD_VOLT		  10
/*********************/

// Index numbers of all external ADC channels

#define	ADC_CRNT		0
#define	ADC_VOLT1		1
#define	ADC_VOLT2		2

#define	MAX_ADC_CHNL ADC_VOLT2+1

#define	DAQ_ARRY_SIZE	MAX_ADC_CHNL
#define	MEAS_ARY_SIZE 	MAX_ADC_CHNL



typedef struct BitField
{
	uint8_t		bCycleOver      			:1;	// Indicate if all samples of one cycle recieved, ie. one cycle is complete
	uint8_t 	bPowerUpFlag				:1; // for reset max min current value
	uint8_t 	bTripFlag					:2; // for reset max min current value
	uint8_t 	bSendmsgFlag				:1; // for reset max min current value
	uint8_t		bTimeoutFlag				:1;
	uint8_t		unused						:2;

}Flags;
extern Flags g_stFlags;


typedef struct SystemSettings
{
	//Not using
	uint16_t m_deviceID	    	;//SEF
	uint16_t m_baudrate;		//STAR/DELTA
	uint16_t m_CheckSum;

}BL_stSystemSettings;
extern BL_stSystemSettings g_stBLSystemSettings;

extern uint16_t	g_iADCBuffer[MAX_ADC_CHNL];	// Variable to store ADC count as read from external ADC

typedef struct MetData_STR
{
	int16_t	m_iSample[SAMPLES_PER_CYCLE];	// Variable to store all derived samples from last one cycle for all ADC channel

}MetData;
extern MetData	g_stMetData[MEAS_ARY_SIZE]; // Structure to store intermediate calculation variables for DAQ


typedef struct g_stADCTaskVariables
{
	int16_t	m_iSample; 	// Variable to hold sample temporarily for manipulation
	uint16_t	 m_uiSampleCount; // Variable to store number of samples taken in present cycle
}MetAdcTaskVar;
extern MetAdcTaskVar	g_stADCTaskVar; // Structure to store intermediate calculation variables for DAQ

// Structure to store all DAQ variables
typedef struct DAQ_STR
{
	int16_t	m_iSampleBuff[SAMPLES_PER_CYCLE];	// Variable to store all samples from last one cycle for all ADC channel and derived variables
	int16_t	m_iAverage;						// Average of all samples taken per cycle -> equates to Vref + offset
	int32_t	m_lSumSamples;			 // Accumulated ADC count for finding m_iAverage
	uint64_t	m_ullSumSqr;		// Variable to store accumulated sum of square of sample values in present cycle
}stDaq;
extern stDaq	g_stDaqVar[DAQ_ARRY_SIZE];

// Structure to store all measurement variables
typedef struct MEAS_STR
{
	uint64_t	m_ullSumSqr[MEAS_ARY_SIZE];		// Variable to store Gain calibrated accumulated sum of square of sample values in present cycle
	uint32_t 	m_ulMeanSqr[MEAS_ARY_SIZE];		// Variable for Mean Suqare value of last completed cycle : in term of nominal current
}stMeas;
extern stMeas	g_stMeasVar;

// Structure to store all metering parameters
typedef struct MET_STR
{
	uint32_t	m_ulRMS[MEAS_ARY_SIZE];			// RMS values

}stMet;
extern stMet	g_stMetVar;



/******************************
extern Functions/variables  declaration
******************************/

extern uint32_t g_ulDaq_CalibrationMS[DAQ_ARRY_SIZE];
extern uint64_t ulMeanSqrBuffer[DAQ_ARRY_SIZE];
extern uint32_t uiCurrentDCOffset,uiVoltageDCOffset ;
extern uint16_t uiCounter;
extern stMet	g_stTrippedMetVar; // Structure for Metering Variables
extern stMet	g_stMaximumMetVar; // Structure for Metering Variables

////////////////////////////////
extern void init_MeasurementTask(void);
extern void PerSampleActivity(void);
extern void DAQ_20msTask();




#endif /* LAB4_OLD_DAQ_H_ */
