/**
*******************************************************************************
* @file      signaling.h
* @version   V1.0
* @date      2016.11.23
* @brief     外部信号指示、飞控状态提示
*******************************************************************************
* @copy
*
* @author	R&D @ AEE Enterprise
*
* XF600Z Flight Controler 
*******************************************************************************
*/

#ifndef __signaling_h
#define __signaling_h

#include "digital.h"

///*---------------------------- constant define ---------------------------------------*/
typedef enum
{
	LED_STATUS_1HZ = 0x00,
	LED_STATUS_2HZ = 0x01,
	LED_STATUS_5HZ = 0x02,
	LED_STATUS_DoubleFlash = 0x03,
	LED_STATUS_TribleFlash = 0x04,
	LED_STATUS_ON   = 0x05,
	LED_STATUS_OFF  = 0x06,
    LED_STATUS_END
} ledState_t;

typedef enum 
{
    SIG_EVENT_NONE = 0,
    // ongoing events
    SIG_EVENT_SELF_CHECK,
    SIG_EVENT_RC_LOST,
    SIG_EVENT_GPS_FIX_NONE,
    SIG_EVENT_GPS_FIX_2D,
    SIG_EVENT_GPS_FIX_3D,
    SIG_EVENT_GPS_FIX_3D_DGPS,
    SIG_EVENT_GPS_FIX_3D_RTK,
    SIG_EVENT_COMPASS_ERROR,
    SIG_EVENT_COMPASS_CALIB_1,
    SIG_EVENT_COMPASS_CALIB_2,
    SIG_EVENT_LOWBATT_L1,
    SIG_EVENT_LOWBATT_L2,
    SIG_EVENT_SD_LOST,
    // one-time events
    SIG_EVENT_GPS_NORTCM,
    SIG_EVENT_OT_DISARMING,
    SIG_EVENT_OT_HF_SET,
    SIG_EVENT_OT_WP_REACHED,
    SIG_EVENT_OT_WP_RECORDED,
    SIG_EVENT_OT_WP_CLEARED,
    SIG_EVENT_ENUM_END
} signalingEvent_T;

typedef struct {
    digitalPin_t debugLed1;
    digitalPin_t debugLed2;

    digitalPin_t ledPortR;
    digitalPin_t ledPortG;
    digitalPin_t ledPortB;

    uint8_t enabled;	      	 // flag indicating if any signaling is used (any ports are enabled)
    uint8_t patPos;	      		 // loop counter used in Led patterns
    uint8_t patLen;	      		 // number of positions in pattern per output device; 10 = 1Hz. Changing it will! affect the led pattern event
    uint8_t onGoingEvtTyp;
    uint8_t oneTimeEvtTyp;    	 // if set, a one-time event is signaled, overriding any other current events
    uint8_t oneTimeEvtStat;  	 // current one-time event stage: 0=not active; 1=event is done; 2=event is in progress
    
    uint8_t navCnt;
    
    ledState_t nav_status;
} sigStruct_t __attribute__((aligned));

extern sigStruct_t sigData;

///*---------------------------- function define ---------------------------------------*/
extern void signalingInit(void);
extern void signalingEvent(uint32_t loop);
extern void signalingOnGoingEventSet(int eventType);
extern signalingEvent_T signalingOnGoingEventGet(void);
extern void signalingOnetimeEvent(int eventTyp);
extern void signalingLEDsOn(void);
extern void signalingLEDsOff(void);

#endif /* __signaling_h */

/* end of file */
