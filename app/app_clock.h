#ifndef __APP_CLOCK_H__
#define __APP_CLOCK_H__

#include "app_bsp.h"

typedef enum
{
    CLOCK_STATE_IDLE = 0,
    CLOCK_STATE_MESSAGE,
    CLOCK_STATE_TIME,
    CLOCK_STATE_DATE,
    CLOCK_STATE_ALARM,
    CLOCK_STATE_DISPLAY
}APP_Clock_State;

void Clock_Init( void );
void Clock_Task( void );
static void Clock_State_Machine( void );
static void updateTime( void );
static void updateDate( void );
static void updateAlarm( void );
static uint8_t elapsed1Seg( void );
static void display( void );
#endif