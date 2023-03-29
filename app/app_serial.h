#ifndef __APP_SERIAL_H__
#define __APP_SERIAL_H__

#include "app_bsp.h"

typedef enum
{
    SERIAL_MSG_NONE = 0,
    SERIAL_MSG_TIME,
    SERIAL_MSG_DATE,
    SERIAL_MSG_ALARM
}APP_Messages;

typedef enum
{
    SERIAL_STATE_IDLE = 0,
    SERIAL_STATE_MESSAGE,
    SERIAL_STATE_ERROR,
    SERIAL_STATE_TIME,
    SERIAL_STATE_DATE,
    SERIAL_STATE_ALARM,
    SERIAL_STATE_OK
}APP_States;

typedef struct _APP_TmTypeDef 
{
    uint32_t tm_sec;         /* seconds,  range 0 to 59          */
    uint32_t tm_min;         /* minutes, range 0 to 59           */
    uint32_t tm_hour;        /* hours, range 0 to 23             */
    uint32_t tm_mday;        /* day of the month, range 1 to 31  */
    uint32_t tm_mon;         /* month, range 0 to 11             */
    uint32_t tm_year;        /* years in rage 1900 2100          */
    uint32_t tm_wday;        /* day of the week, range 0 to 6    */
    uint32_t tm_yday;        /* day in the year, range 0 to 365  */
    uint32_t tm_isdst;       /* daylight saving time             */
}APP_TmTypeDef;

typedef struct _APP_MsgTypeDef
{
    uint8_t msg;          /*!< Store the message type to send */
    APP_TmTypeDef tm;     /*!< time and date in stdlib tm format */
}APP_MsgTypeDef;



void Serial_Init( void );
void Serial_Task( void );
void Serial_State_Machine( void );
static void CanTp_SingleFrameTx( uint8_t *data, uint8_t size );
static uint8_t CanTp_SingleFrameRx( uint8_t *data, uint8_t *size );


#endif