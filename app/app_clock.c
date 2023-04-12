/**
 * @file    app_clock.c
 * @brief   This file provides firmware functions to manage the 
 *          functionalities of one clock.
 *          
 *
 * Write a detail description of your driver, what it does and how and how to work with its interfaces
 * Use as many lines as you need
 *
 * @note    If there is something that needs to be take into account beyond its normal
 *          utilization, write right here
 */

#include "app_clock.h"
#include "app_serial.h"
#include "app_bsp.h"
#include <stdio.h>      /* cppcheck-suppress misra-c2012-21.6 ; the header file es only for testint pourpose */

static void updateTime( void );
static void updateDate( void );
static void updateAlarm( void );
static uint8_t elapsed1Seg( void );
static uint32_t tickstar;
static void display( void );

extern void initialise_monitor_handles(void);

/**
 * @brief  TypeDef para el RTC
 */
static RTC_HandleTypeDef hrtc;
/**
 * @brief  TypeDef para la fecha
 */
static RTC_DateTypeDef sDate = {0};
/**
 * @brief  TypeDef para la hora
 */
static RTC_TimeTypeDef sTime = {0};
/**
 * @brief  TypeDef para la alarma
 */
static RTC_AlarmTypeDef sAlarm = {0};

/**
 * @brief  Initialize the Clock and regular group according to
 *         parameters.
 *
 * @param   hrtc [in/out] RTC handler
 * @param   sTime [in/out] RCT time
 * @param   sDate [in/out] RTC date
 *
 * @retval  None
 */
void Clock_Init( void )
{
    /*configuration*/
    hrtc.Instance             = RTC;
    hrtc.Init.HourFormat      = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv    = 127;//RTC_ASYNCH_PREDIV;
    hrtc.Init.SynchPrediv     = 255;//RTC_SYNCH_PREDIV;
    hrtc.Init.OutPut          = RTC_OUTPUT_DISABLE;
    /* initilize the RTC with 24 hour format and no output signal enble */
    HAL_RTC_Init( &hrtc );  

    sTime.Hours   = 0x12;
    sTime.Minutes = 0x00;
    sTime.Seconds = 0x00;
    sTime.SubSeconds = 0x00;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    /*Set the time to 12:00:00*/
    HAL_RTC_SetTime( &hrtc, &sTime, RTC_FORMAT_BCD );

    sDate.WeekDay = RTC_WEEKDAY_THURSDAY;
    sDate.Month = RTC_MONTH_MARCH;
    sDate.Date = 0x04;
    sDate.Year = 0x23;
    /*Set date to Thursday, march 4, 2023*/
    HAL_RTC_SetDate( &hrtc, &sDate, RTC_FORMAT_BCD );

    tickstar = HAL_GetTick( );
}   

/**
 * @brief  Ejecuta la máquina de estados del reloj
 *
 * @param   Serial_Msg [in] Structura de mensaje
 *
 * @retval  None
 */
void Clock_Task( void )
{
    static APP_Clock_State state = CLOCK_STATE_IDLE;

    switch(state)
    {
        case CLOCK_STATE_IDLE:
            if( elapsed1Seg() == 1u)
            {
                state = CLOCK_STATE_DISPLAY;
            }
            else if ( Serial_Msg.msg != SERIAL_MSG_NONE )
            {
                state = CLOCK_STATE_MESSAGE;
            }
            else
            {}         
            break;

        case CLOCK_STATE_MESSAGE:
            switch(Serial_Msg.msg)
            {
                case SERIAL_MSG_TIME:
                    state = CLOCK_STATE_TIME;
                    Serial_Msg.msg = SERIAL_MSG_NONE;
                    break;

                case SERIAL_MSG_DATE:
                    state = CLOCK_STATE_DATE;
                    Serial_Msg.msg = SERIAL_MSG_NONE;
                    break;

                case SERIAL_MSG_ALARM:
                    state = CLOCK_STATE_ALARM;
                    Serial_Msg.msg = SERIAL_MSG_NONE;
                    break;
                default:
                    break;
            }
            break;

        case CLOCK_STATE_TIME:
            /* Get the RTC current Time */
            updateTime();
            state = CLOCK_STATE_DISPLAY;  
            break;

        case CLOCK_STATE_DATE:
            /* Get the RTC current date */
            updateDate();
            state = CLOCK_STATE_DISPLAY;
            break;

        case CLOCK_STATE_ALARM:
            /* Get the RTC current time and date */
            updateAlarm();
            state = CLOCK_STATE_DISPLAY; 
            break;
        case CLOCK_STATE_DISPLAY:
            display( );
            state = CLOCK_STATE_IDLE;
            break;
        default:
            break;
    } 
}

/**
 * @brief  Actualiza la hora del reloj
 *
 * @param   hrtc [in] RTC handler
 * @param   sTime [in/out] RCT time
 *
 * @retval  None
 */
static void updateTime( void )
{
    /* Setting time in BCD format */
    sTime.Hours   = Serial_Msg.tm.tm_hour;
    sTime.Minutes = Serial_Msg.tm.tm_min;
    sTime.Seconds = Serial_Msg.tm.tm_sec;

    /*Set the time */
    HAL_RTC_SetTime( &hrtc, &sTime, RTC_FORMAT_BIN );

}

/**
 * @brief  Actualiza la fecha del reloj
 *
 * @param   hrtc [in] RTC handler
 * @param   sDate [in/out] RTC date
 *
 * @retval  None
 */
static void updateDate( void )
{
    /* Setting date in BCD format */
    sDate.Month = Serial_Msg.tm.tm_mon;
    sDate.Date = Serial_Msg.tm.tm_mday;
    sDate.Year = ( ( Serial_Msg.tm.tm_year % 1000u ) % 100u );

    /*Set the date */
    HAL_RTC_SetDate( &hrtc, &sDate, RTC_FORMAT_BIN );
}

/**
 * @brief  Actualiza la hora de la alarma
 *
 * @param   hrtc [in] RTC handler
 * @param   sAlarm [in/out] Alarma del RTC
 *
 * @retval  None
 */
static void updateAlarm( void )
{
    /* Setting Alarm in BCD format */
    sAlarm.AlarmTime.Hours   = Serial_Msg.tm.tm_hour;
    sAlarm.AlarmTime.Minutes = Serial_Msg.tm.tm_min;
    
    /*Set the alarm */
    HAL_RTC_SetAlarm( &hrtc, &sAlarm, RTC_FORMAT_BIN );
}

/**
 * @brief  Verifica si ya trasncurrio un segundo
 *
 * @param   tickstar [in/out] tiempo transcurrido
 *
 * @retval  un verdadero si transcurrio un segundo, falto en caso contrario
 */
static uint8_t elapsed1Seg( void )
{
    uint8_t timeOver = 0u; 

    if( ( HAL_GetTick( ) - tickstar ) >= 1000u )
    {
        tickstar = HAL_GetTick( );
        timeOver = 1;
    }

    return timeOver;
}

/**
 * @brief  Muestra por el semihost la hora, fecha y alarma actual
 *         parameters
 *
 * @param   hrtc [in] RTC handler
 * @param   sTime [in/out] RCT time
 * @param   sDate [in/out] RTC date
 *
 * @retval  None
 */
static void display( void )
{
    initialise_monitor_handles();

    HAL_RTC_GetTime( &hrtc, &sTime, RTC_FORMAT_BIN );
    HAL_RTC_GetDate( &hrtc, &sDate, RTC_FORMAT_BIN );

    printf("\n"); /* cppcheck-suppress misra-c2012-17.7 ; the function es only for testint pourpose */
    printf("Time:  %02d:%02d:%02d\n\r", sTime.Hours, sTime.Minutes, sTime.Seconds); /* cppcheck-suppress misra-c2012-17.7 ; the function es only for testint pourpose */
    printf("Date:  %02d/%02d/%02d\n\r", sDate.Date, sDate.Month, sDate.Year); /* cppcheck-suppress misra-c2012-17.7 ; the function es only for testint pourpose */
    printf("Alarm: %02d:%02d\n\r", sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes); /* cppcheck-suppress misra-c2012-17.7 ; the function es only for testint pourpose */
}