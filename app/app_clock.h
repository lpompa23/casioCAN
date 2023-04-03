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
}APP_Clock_State;

void Clock_Init( void );
void Clock_Task( void );

#endif