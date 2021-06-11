/**
  ******************************************************************************
  * @file    Applications/signaling.c 
  * @author  Jason Lee
  * @version V0.0.1
  * @date    16-October-2018
  * @brief   LED signal handle.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2018 AEE Technology</center></h2>
  *
  * All rights reserved.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "signaling.h"
#include "digital.h"
#include "string.h"

sigStruct_t sigData;


const uint8_t sig_pattern[SIG_EVENT_ENUM_END][31] = {
//         led-r                 led-g                 led-b        lenth
//  0 1 2 3 4 5 6 7 8 9   0 1 2 3 4 5 6 7 8 9   0 1 2 3 4 5 6 7 8 9
// ongoing events
  { 0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0, 10 }, 	// 0  (All-Off)
  { 1,1,1,1,0,0,0,0,0,0,  0,0,1,1,1,1,0,0,0,0,  1,1,0,0,0,0,1,1,0,0, 10 }, 	// 1  (Self check)	//¬Ã¿∂ΩªÃÊ…¡À∏
  { 1,0,1,0,1,0,1,0,1,0,  1,0,1,0,1,0,1,0,1,0,  0,0,0,0,0,0,0,0,0,0, 10 }, 	// 2  (RC Lost)		//ª∆µ∆5HZ…¡À∏	
  { 1,1,1,1,1,0,0,0,0,0,  1,1,1,1,1,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0, 10 }, 	// 3  (GPS no fixed)  //ª∆µ∆1HZ…¡À∏
  { 0,0,0,0,0,0,0,0,0,0,  1,1,1,1,1,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0, 10 }, 	// 4  (GPS fixed 2d)  //¬Ãµ∆1HZ…¡À∏
  { 0,0,0,0,0,0,0,0,0,0,  1,1,1,0,0,1,1,1,0,0,  0,0,0,0,0,0,0,0,0,0, 10 }, 	// 5  (GPS fixed 3d)  //¬Ãµ∆2HZ…¡À∏
  { 0,0,0,0,0,0,0,0,0,0,  1,0,1,0,1,0,1,0,1,0,  0,0,0,0,0,0,0,0,0,0, 10 }, 	// 6  (GPS fixed 3d-dgps) //¬Ãµ∆5HZ…¡À∏
  { 0,0,0,0,0,0,0,0,0,0,  1,1,1,1,1,1,1,1,1,1,  0,0,0,0,0,0,0,0,0,0, 10 }, 	// 7  (GPS fixed 3d-rtk)  //¬Ãµ∆≥£¡¡S
  { 1,1,1,1,1,1,1,1,1,1,  1,1,1,1,1,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0, 10 }, 	// 8  (Compass error)    //ª∆∫ÏΩªÃÊ1HZ…¡À∏
  { 0,0,0,0,0,0,0,0,0,0,  1,1,1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,1,1, 10 }, 	// 9  (Compass calib #1)	//«‡…´≥£¡¡
  { 1,1,1,1,1,1,1,1,1,1,  0,0,0,0,0,0,0,0,0,0,  1,1,1,1,1,1,1,1,1,1, 10 }, 	// 10 (Compass calib #2)	//—Û∫Ï≥£¡¡
  { 1,1,1,1,1,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0, 10 },	// 11 (Low batt 1)          //∫Ïµ∆1HZ…¡À∏
  { 1,1,1,0,0,1,1,1,0,0,  0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0, 10 },	// 12 (Low batt 2)			//∫Ïµ∆2HZ…¡À∏
  { 1,1,1,0,0,0,0,0,0,0,  0,0,0,0,0,1,1,1,0,0,  0,0,0,0,0,0,0,0,0,0, 10 },  // 13 (sd lost)				//∫Ï¬ÃΩªÃÊ1HZ…¡À∏
      
// one-time events                              
  { 0,0,0,0,0,0,0,0,0,0,  1,1,1,0,0,0,0,0,0,0,  0,0,0,0,0,1,1,1,0,0, 10 }, 	// 14 (GPS without RTCM)   //¬Ã¿∂ΩªÃÊ1HZ…¡À∏
  { 0,0,0,0,0,0,0,0,0,0,  1,0,0,1,0,0,1,0,0,1,  0,0,0,0,0,0,0,0,0,0, 10 }, 	// 15 (Arming)			//¬Ãµ∆4HZ…¡À∏
  { 1,1,0,0,1,1,0,0,1,1,  0,0,1,1,0,0,1,1,0,0,  1,1,1,1,1,1,1,1,1,1, 10 }, 	// 16 (Disarming)		//¿∂«‡ΩªÃÊ2HZ…¡À∏
  { 0,0,1,1,0,0,1,1,0,0,  0,0,1,1,0,0,1,1,0,0,  1,1,0,0,1,1,0,0,1,1, 10 }, 	// 17 (Waypoint reached)	L1 & L2 simultaneous 200ms flashes @ 3Hz; 200ms beeps @ 3Hz
  { 0,0,0,1,1,1,1,0,0,0,  0,0,0,1,1,1,1,0,0,0,  1,1,1,0,0,0,0,1,1,1, 10 }, 	// 18 (Waypoint recorded)	L1 & L2 simultaneous 400ms flash @ 1Hz; 2x300ms beeps
  { 1,0,1,0,1,0,1,0,1,0,  1,0,1,0,1,0,1,0,1,0,  1,0,1,0,1,0,1,0,1,0, 10 }, 	// 19 (Waypoints cleared)	L1 & L2 simultaneous 100ms flashes @ 5hz; 100ms beeps @ 5hz
};

/* “‘100msŒ™ ±º‰º‰∏Ù£¨2sŒ™—≠ª∑÷‹∆⁄ */
const uint8_t led_state[][20] = {
	{1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},	/* 1Hz…¡À∏ */
	{1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0},	/* 2Hz…¡À∏ */
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},	/* 5Hz…¡À∏ */
	{1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	/* À´…¡ */
	{1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	/* »˝…¡ */
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},	/* ≥§¡¡ */
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	/* œ®√ */
};



void signalingWriteOutput(int eventType)
{
    if (sigData.ledPortR)
    {
        digitalSet(sigData.ledPortR, sig_pattern[eventType][sigData.patPos]);
    }
    
    if (sigData.ledPortG)
    {
		digitalSet(sigData.ledPortG, sig_pattern[eventType][sigData.patPos + 10]);
    }
    
    if (sigData.ledPortB)
    {
		digitalSet(sigData.ledPortB, sig_pattern[eventType][sigData.patPos + 20]);
    }
}



/**
 *******************************************************************************
 * @brief      void signalingOnetimeEvent(int eventType)
 * @param[in]  None
 * @param[out] None
 * @retval     None
 *
 * @par Description  µ±«∞–≈∫≈◊¥Ã¨÷∏ æ
 * @details    initiate a one-time event, eg. arming/disarming
 *			   eventTyp should be one of signalingEventTypes
 *******************************************************************************
 */
void signalingOnetimeEvent(int eventType)
{
    if (eventType < 0 || eventType >= SIG_EVENT_ENUM_END)
		return;

    // shut down any current event signals
    // TODO: set up a signaling event queue
    if (sigData.oneTimeEvtTyp)
    {
		signalingWriteOutput(SIG_EVENT_NONE);
    }
    sigData.oneTimeEvtStat = 0;
    sigData.oneTimeEvtTyp = eventType;
}

void signalingOnGoingEventSet(int eventType)
{
    if (eventType != sigData.onGoingEvtTyp)
    {
        sigData.onGoingEvtTyp = eventType;
        sigData.patPos = 0;
        sigData.patLen = sig_pattern[sigData.oneTimeEvtTyp][30];
    }
}

signalingEvent_T signalingOnGoingEventGet(void)
{
    return (signalingEvent_T)sigData.onGoingEvtTyp;
}

void signalingEvent(uint32_t loop)
{
    // watch for one-time event being triggered
    if (sigData.oneTimeEvtTyp)
    {
		switch (sigData.oneTimeEvtStat)
        {
		case 0:	// start the event
			sigData.oneTimeEvtStat = 2;
			sigData.patPos = 0;
            sigData.patLen = sig_pattern[sigData.oneTimeEvtTyp][30];
			break;
            
		case 1: // event has finished
			sigData.oneTimeEvtStat = 0;
			sigData.oneTimeEvtTyp = SIG_EVENT_NONE;
			sigData.patPos = 0;
			// make sure everything is off
			signalingWriteOutput(SIG_EVENT_NONE);
			break;
            
		case 2: // event is in progress
			signalingWriteOutput(sigData.oneTimeEvtTyp);
			// see if we're done  TODO: allow events longer than one pattern length
			if (++sigData.patPos >= sigData.patLen)
            {
                sigData.oneTimeEvtStat = 1;
            }
			break;
		}
		return;
    }
    else
    {
        signalingWriteOutput(sigData.onGoingEvtTyp);
        
        if (++sigData.patPos >= 10)
        {
            sigData.patPos = 0;
        }
    }
    
	
	/*∞Â‘ÿLEDµ∆øÿ÷∆*/
    if (sigData.nav_status < LED_STATUS_END)
    {
		digitalSet(sigData.debugLed1,(led_state[sigData.nav_status][sigData.navCnt]));
		//digitalSet(sigData.debugLed2,(led_state[sigData.nav_status][sigData.navCnt]));
		sigData.navCnt++;
		if(sigData.navCnt >= 20)
			sigData.navCnt = 0;
	}
	
}



/**
 *******************************************************************************
 * @brief      void signalingInit(void)
 * @param[in]  None
 * @param[out] None
 * @retval     None
 *
 * @par Description
 * @details    Õ‚≤ø÷∏ æ–≈∫≈≥ı ºªØ£¨∞¸¿®∞Â‘ÿLedµ∆º∞±€µ∆
 *******************************************************************************
 */
void signalingInit(void)
{
    memset((void *)&sigData, 0, sizeof(sigData));
    
	/*∞Â‘ÿµ˜ ‘µ∆*/
    sigData.debugLed1 = digitalInit(GPIOF, GPIO_Pin_9, 0);
	sigData.debugLed2 = digitalInit(GPIOF , GPIO_Pin_10, 0);
    
	/*RGBµ∆*/
    sigData.ledPortR = digitalInit(GPIOE, GPIO_Pin_1, 0);
    sigData.ledPortG = digitalInit(GPIOE, GPIO_Pin_3, 0);
    sigData.ledPortB = digitalInit(GPIOE, GPIO_Pin_5, 0);
    
    sigData.onGoingEvtTyp = 1;
	sigData.nav_status = LED_STATUS_1HZ;
}
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************** (C) COPYRIGHT AEE Technology ******END OF FILE*****/
