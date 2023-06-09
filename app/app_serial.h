#ifndef APP_SERIAL_H
#define APP_SERIAL_H

#include "app_bsp.h"

/**
  * @brief   Enumeración para los distintos mensajes
  *          que se pueden recibir
  */
typedef enum 
{
    SERIAL_MSG_NONE = 0,
    SERIAL_MSG_TIME,
    SERIAL_MSG_DATE,
    SERIAL_MSG_ALARM
}APP_Messages;

/**
  * @brief   Enumeración que agrupa los estados de 
  *          la máquina de estados
  */
typedef enum
{
    SERIAL_STATE_IDLE = 0,
    SERIAL_STATE_MESSAGE,
    SERIAL_STATE_TIME,
    SERIAL_STATE_DATE,
    SERIAL_STATE_ALARM,
    SERIAL_STATE_OK,
    SERIAL_STATE_ERROR
}APP_States;

/**
  * @brief   Estructura para almacenar la fecha y hora 
  *         
  */
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

/**
  * @brief   Tipo de dato para los mensajes
  */
typedef struct _APP_MsgTypeDef
{
    APP_Messages msg;          /*!< Store the message type to send */
    APP_TmTypeDef tm;     /*!< time and date in stdlib tm format */
}APP_MsgTypeDef;
 
extern APP_MsgTypeDef Serial_Msg;
extern FDCAN_HandleTypeDef CANHandler;

void Serial_Init( void );
void Serial_Task( void );
void HAL_FDCAN_RxFifo0Callback( FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs );

#endif