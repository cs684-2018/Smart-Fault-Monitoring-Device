/*
*
 * Team Id: <6: Smart fault monitoring device>
 * Author List: Anil Kumar Garg, Abhishek Pal
 * Filename: DAQ.c
 * Functions:
	void init_MeasurementTask(void);
	void PerSampleActivity(void);
	void DAQ_20msTask();
 *
 */

#include "DAQ.h"
#include "math.h"

//* Global Variables: <List of global variables defined in this file, None if no global variables>

uint32_t CTest=0,V1Test=0,V2Test=0;
uint32_t g_ulDaq_CalibrationMS[DAQ_ARRY_SIZE];
uint16_t	g_iADCBuffer[MAX_ADC_CHNL];	// Variable to store ADC count as read from external ADC
MetAdcTaskVar	g_stADCTaskVar;	 // Structure to store intermediate calculation variables for DAQ
stDaq	g_stDaqVar[ DAQ_ARRY_SIZE ]; // Structure for DAQ variables
stMeas	g_stMeasVar; // Structure for Measuremet Variables
stMet	g_stMetVar; // Structure for Metering Variables
stMet	g_stTrippedMetVar; // Structure for Metering Variables
stMet	g_stMaximumMetVar; // Structure for Metering Variables
Flags g_stFlags;	// Structure to store various status flags
uint16_t uiCounter = 0;
void init_MeasurementTask(void);
void PerSampleActivity(void);
void DAQ_20msTask();
//End


/*
*
* Function Name: PerSampleActivity
* Input: g_iADCBuffer
* Output: g_stMeasVar
* Logic: Capture and buffer ADC samples in every 1.25msec
* Example Call: call in adc interrupt
*
*/

void PerSampleActivity(void)
{
	g_stADCTaskVar.m_iSample = (g_iADCBuffer[ ADC_CRNT ] - g_stDaqVar[ADC_CRNT].m_iAverage);
	g_stDaqVar[ADC_CRNT].m_lSumSamples += g_iADCBuffer[ ADC_CRNT ];
	g_stDaqVar[ADC_CRNT].m_iSampleBuff[g_stADCTaskVar.m_uiSampleCount] = g_stADCTaskVar.m_iSample;
	g_stDaqVar[ADC_CRNT].m_ullSumSqr += (uint64_t)((int32_t)g_stADCTaskVar.m_iSample * (int32_t)g_stADCTaskVar.m_iSample);

	g_stADCTaskVar.m_iSample = (g_iADCBuffer[ ADC_VOLT1 ] - g_stDaqVar[ADC_VOLT1].m_iAverage);
	g_stDaqVar[ADC_VOLT1].m_lSumSamples += g_iADCBuffer[ ADC_VOLT1 ];
	g_stDaqVar[ADC_VOLT1].m_iSampleBuff[g_stADCTaskVar.m_uiSampleCount] = g_stADCTaskVar.m_iSample;
	g_stDaqVar[ADC_VOLT1].m_ullSumSqr += (uint64_t)((int32_t)g_stADCTaskVar.m_iSample * (int32_t)g_stADCTaskVar.m_iSample);

	g_stADCTaskVar.m_iSample = (g_iADCBuffer[ ADC_VOLT2 ] - g_stDaqVar[ADC_VOLT2].m_iAverage);
	g_stDaqVar[ADC_VOLT2].m_lSumSamples += g_iADCBuffer[ ADC_VOLT2 ];
	g_stDaqVar[ADC_VOLT2].m_iSampleBuff[g_stADCTaskVar.m_uiSampleCount] = g_stADCTaskVar.m_iSample;
	g_stDaqVar[ADC_VOLT2].m_ullSumSqr += (uint64_t)((int32_t)g_stADCTaskVar.m_iSample * (int32_t)g_stADCTaskVar.m_iSample);

	g_stADCTaskVar.m_uiSampleCount++;

	if(g_stADCTaskVar.m_uiSampleCount == SAMPLES_PER_CYCLE)
	{
		g_stADCTaskVar.m_uiSampleCount = 0;
		g_stDaqVar[ADC_CRNT].m_iAverage= g_stDaqVar[ADC_CRNT].m_lSumSamples >>4;
		g_stDaqVar[ADC_VOLT1].m_iAverage=g_stDaqVar[ADC_VOLT1].m_lSumSamples >>4;
		g_stDaqVar[ADC_VOLT2].m_iAverage=g_stDaqVar[ADC_VOLT2].m_lSumSamples >>4;

		g_stMeasVar.m_ullSumSqr[ADC_VOLT2] = g_stDaqVar[ADC_VOLT2].m_ullSumSqr;
		g_stMeasVar.m_ullSumSqr[ADC_VOLT1] = g_stDaqVar[ADC_VOLT1].m_ullSumSqr;
		g_stMeasVar.m_ullSumSqr[ADC_CRNT] = g_stDaqVar[ADC_CRNT].m_ullSumSqr;

		g_stDaqVar[ADC_VOLT2].m_ullSumSqr=0;
		g_stDaqVar[ADC_VOLT1].m_ullSumSqr=0;
		g_stDaqVar[ADC_CRNT].m_ullSumSqr = 0;

		g_stDaqVar[ADC_CRNT].m_lSumSamples =0;
		g_stDaqVar[ADC_VOLT1].m_lSumSamples =0;
		g_stDaqVar[ADC_VOLT2].m_lSumSamples =0;

		g_stFlags.bCycleOver = 1;


	}
}

/*
*
* Function Name: DAQ_20msTask
* Input: g_stMeasVar
* Output: g_stMetVar
* Logic: Calculate RMS and buffer tripped value  in every 20msec
*
*/

void DAQ_20msTask(void)
{
	g_stFlags.bCycleOver = 0;
	g_stMeasVar.m_ulMeanSqr[ADC_CRNT]= (uint32_t)((g_stMeasVar.m_ullSumSqr[ADC_CRNT] * (uint64_t)g_ulDaq_CalibrationMS[ADC_CRNT])>>10);
	g_stMeasVar.m_ulMeanSqr[ADC_VOLT1]= (uint32_t)((g_stMeasVar.m_ullSumSqr[ADC_VOLT1] * (uint64_t)g_ulDaq_CalibrationMS[ADC_VOLT1])>>15);
	g_stMeasVar.m_ulMeanSqr[ADC_VOLT2]= (uint32_t)((g_stMeasVar.m_ullSumSqr[ADC_VOLT2] * (uint64_t)g_ulDaq_CalibrationMS[ADC_VOLT2])>>15);

	g_stMetVar.m_ulRMS[ADC_CRNT] = sqrt(g_stMeasVar.m_ulMeanSqr[ADC_CRNT]);
	g_stMetVar.m_ulRMS[ADC_VOLT1] = sqrt(g_stMeasVar.m_ulMeanSqr[ADC_VOLT1]);
	g_stMetVar.m_ulRMS[ADC_VOLT2] = sqrt(g_stMeasVar.m_ulMeanSqr[ADC_VOLT2]);

	if(g_stMetVar.m_ulRMS[ADC_CRNT] <= NOLOAD_CURR_RMS)
	{
		g_stMetVar.m_ulRMS[ADC_CRNT] =0;
	}
	if(g_stMetVar.m_ulRMS[ADC_VOLT1] < NOLOAD_VOLT)
	{
		g_stMetVar.m_ulRMS[ADC_VOLT1] =0;
	}
	if(g_stMetVar.m_ulRMS[ADC_VOLT2] < NOLOAD_VOLT)
	{
		g_stMetVar.m_ulRMS[ADC_VOLT2] =0;
	}

	if(g_stMaximumMetVar.m_ulRMS[ADC_CRNT] <= g_stMetVar.m_ulRMS[ADC_CRNT])
		g_stMaximumMetVar = g_stMetVar;
	if((g_stMetVar.m_ulRMS[ADC_VOLT1] >200) & (g_stMetVar.m_ulRMS[ADC_VOLT2] <150))
	{
		g_stTrippedMetVar = g_stMaximumMetVar;
		g_stFlags.bTripFlag = 1;
	}
	else if((g_stMetVar.m_ulRMS[ADC_VOLT1] <50) )
	{
		g_stTrippedMetVar = g_stMaximumMetVar;
		g_stFlags.bTripFlag = 2;
	}
	else
	{
		g_stTrippedMetVar = g_stMetVar;
		g_stFlags.bTripFlag = 0;
		if(g_stFlags.bTimeoutFlag)
		{
			g_stFlags.bTimeoutFlag = 0;
			g_stMaximumMetVar.m_ulRMS[ADC_CRNT] = g_stMetVar.m_ulRMS[ADC_CRNT];
			g_stMaximumMetVar.m_ulRMS[ADC_VOLT1] = g_stMetVar.m_ulRMS[ADC_VOLT1];
			g_stMaximumMetVar.m_ulRMS[ADC_VOLT2] = g_stMetVar.m_ulRMS[ADC_VOLT2];

		}
	}

}


/*
*
* Function Name: init_MeasurementTask
* Input: NA
* Output: NA
* Logic: Initialization RAM variables on power up
*
*/

void init_MeasurementTask(void)
{
	g_ulDaq_CalibrationMS[ADC_CRNT]= 29402;
	g_ulDaq_CalibrationMS[ADC_VOLT1]= 113;
	g_ulDaq_CalibrationMS[ADC_VOLT2]= 113;

	g_stDaqVar[ADC_CRNT].m_iAverage= 2045;
	g_stDaqVar[ADC_VOLT1].m_iAverage= 2045;
	g_stDaqVar[ADC_VOLT2].m_iAverage= 2045;

	g_stADCTaskVar.m_uiSampleCount = 0;
	g_stDaqVar[ADC_VOLT2].m_ullSumSqr=0;
	g_stDaqVar[ADC_VOLT1].m_ullSumSqr=0;
	g_stDaqVar[ADC_CRNT].m_ullSumSqr = 0;

	g_stDaqVar[ADC_CRNT].m_lSumSamples =0;
	g_stDaqVar[ADC_VOLT1].m_lSumSamples =0;
	g_stDaqVar[ADC_VOLT2].m_lSumSamples =0;

	g_stMeasVar.m_ullSumSqr[ADC_VOLT2] = 0;
	g_stMeasVar.m_ullSumSqr[ADC_VOLT1] = 0;
	g_stMeasVar.m_ullSumSqr[ADC_CRNT] = 0;

}
