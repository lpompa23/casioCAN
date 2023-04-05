#include "app_clock.h"
#include "app_serial.h"
#include "app_bsp.h"

extern void initialise_monitor_handles(void);

extern APP_MsgTypeDef msgCasio;
RTC_HandleTypeDef hrtc;

static RTC_DateTypeDef sDate = {0};
static RTC_TimeTypeDef sTime = {0};
static RTC_AlarmTypeDef sAlarm = {0};

void Clock_Init( void )
{
    /*configuration
    hrtc.Instance             = RTC;
    hrtc.Init.HourFormat      = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv    = 127; //RTC_ASYNCH_PREDIV;
    hrtc.Init.SynchPrediv     = 255; //RTC_SYNCH_PREDIV;
    hrtc.Init.OutPut          = RTC_OUTPUT_DISABLE;

    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType     = RTC_OUTPUT_TYPE_OPENDRAIN;
     initilize the RTC with 24 hour format and no output signal enable 
    HAL_RTC_Init( &hrtc );

     Setting time and date in BCD format 
    sTime.Hours   = 0x12;
    sTime.Minutes = 0x00;
    sTime.Seconds = 0x01;
    sTime.SubSeconds = 0x00;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    Set the time to 12:00:00
    HAL_RTC_SetTime( &hrtc, &sTime, RTC_FORMAT_BCD );

    sDate.WeekDay = RTC_WEEKDAY_MONDAY;
    sDate.Month = RTC_MONTH_MARCH;
    sDate.Date = 0x03;
    sDate.Year = 0x23;
    Set date to Monday, March 3, 2023
    HAL_RTC_SetDate( &hrtc, &sDate, RTC_FORMAT_BCD ); */
    /*configuration*/
    hrtc.Instance             = RTC;
    hrtc.Init.HourFormat      = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv    = 127;//RTC_ASYNCH_PREDIV;
    hrtc.Init.SynchPrediv     = 255;//RTC_SYNCH_PREDIV;
    hrtc.Init.OutPut          = RTC_OUTPUT_DISABLE;
    /* initilize the RTC with 24 hour format and no output signal enble */
    HAL_RTC_Init( &hrtc );  

    sTime.Hours   = 0x02;
    sTime.Minutes = 0x00;
    sTime.Seconds = 0x00;
    sTime.SubSeconds = 0x00;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    /*Set the time to 2:00:00*/
    HAL_RTC_SetTime( &hrtc, &sTime, RTC_FORMAT_BCD );

    sDate.WeekDay = RTC_WEEKDAY_MONDAY;
    sDate.Month = RTC_MONTH_APRIL;
    sDate.Date = 0x16;
    sDate.Year = 0x18;
    /*Set date to Monday, April 16, 2018*/
    HAL_RTC_SetDate( &hrtc, &sDate, RTC_FORMAT_BCD );

}   

void Clock_Task( void )
{
    static uint8_t seg = 0;
    initialise_monitor_handles();
    //Clock_State_Machine();
    HAL_RTC_GetTime( &hrtc, &sTime, RTC_FORMAT_BIN );
    HAL_RTC_GetDate( &hrtc, &sDate, RTC_FORMAT_BIN );
    
    if(sTime.Seconds == seg)
    {
        printf("%02d:%02d:%02d\n\r", sTime.Hours, sTime.Minutes, sTime.Seconds);
        seg++;
        if(seg == 60) 
            seg = 0;
    
    }
}

void Clock_State_Machine( void )
{
    static uint8_t state = CLOCK_STATE_IDLE;

    switch(state)
    {
        case CLOCK_STATE_IDLE:
            if( msgCasio.msg != SERIAL_MSG_NONE)
            {
                state = CLOCK_STATE_MESSAGE;
            }
            break;

        case CLOCK_STATE_MESSAGE:
            switch(msgCasio.msg)
            {
                case SERIAL_MSG_TIME:
                    state = CLOCK_STATE_TIME;
                    break;

                case SERIAL_MSG_DATE:
                    state = CLOCK_STATE_DATE;
                    break;

                case SERIAL_MSG_ALARM:
                    state = CLOCK_STATE_ALARM;
                    break;
            }
            break;

        case CLOCK_STATE_TIME:
            /* Get the RTC current Time */
            updateTime();
            state = CLOCK_STATE_IDLE;  
            break;

        case CLOCK_STATE_DATE:
            /* Get the RTC current date */
            updateDate();
            state = CLOCK_STATE_IDLE;
            break;

        case CLOCK_STATE_ALARM:
            /* Get the RTC current time and date */
            updateAlarm();
            state = CLOCK_STATE_IDLE; 
            break;
    }
}

void updateTime( void )
{
    /* Setting time in BCD format */
    sTime.Hours   = msgCasio.tm.tm_hour;
    sTime.Minutes = msgCasio.tm.tm_min;
    sTime.Seconds = msgCasio.tm.tm_sec;

    /*Set the time */
    HAL_RTC_SetTime( &hrtc, &sTime, RTC_FORMAT_BCD );
}

void updateDate( void )
{
    /* Setting date in BCD format */
    sDate.Month = msgCasio.tm.tm_mon;
    sDate.Date = msgCasio.tm.tm_mday;
    sDate.Year = msgCasio.tm.tm_year;

    /*Set the date */
    HAL_RTC_SetDate( &hrtc, &sDate, RTC_FORMAT_BCD );
}

void updateAlarm( void )
{
    /* Setting Alarm in BCD format */
    sAlarm.AlarmTime.Hours   = msgCasio.tm.tm_hour;
    sAlarm.AlarmTime.Minutes = msgCasio.tm.tm_min;
    
    /*Set the alarm */
    HAL_RTC_SetAlarm( &hrtc, &sAlarm, RTC_FORMAT_BCD );
}