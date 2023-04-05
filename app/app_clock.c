#include "app_clock.h"
#include "app_serial.h"
#include "app_bsp.h"

extern void initialise_monitor_handles(void);

extern APP_MsgTypeDef msgCasio;
RTC_HandleTypeDef hrtc;

static RTC_DateTypeDef sDate = {0};
static RTC_TimeTypeDef sTime = {0};
static RTC_AlarmTypeDef sAlarm = {0};
static uint8_t seg = 0;

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

    seg = sTime.Seconds;
}   

void Clock_Task( void )
{
    Clock_State_Machine(); 
}

static void Clock_State_Machine( void )
{
    static uint8_t state = CLOCK_STATE_IDLE;

    switch(state)
    {
        case CLOCK_STATE_IDLE:
            if ( elapsed1Seg() )
            {
                state = CLOCK_STATE_DISPLAY;
            }
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
        case CLOCK_STATE_DISPLAY:
            display( );
            state = CLOCK_STATE_IDLE;
            break;
    }
}

static void updateTime( void )
{
    /* Setting time in BCD format */
    sTime.Hours   = msgCasio.tm.tm_hour;
    sTime.Minutes = msgCasio.tm.tm_min;
    sTime.Seconds = msgCasio.tm.tm_sec;

    /*Set the time */
    HAL_RTC_SetTime( &hrtc, &sTime, RTC_FORMAT_BCD );

    seg = sTime.Seconds;
}

static void updateDate( void )
{
    /* Setting date in BCD format */
    sDate.Month = msgCasio.tm.tm_mon;
    sDate.Date = msgCasio.tm.tm_mday;
    sDate.Year = msgCasio.tm.tm_year;

    /*Set the date */
    HAL_RTC_SetDate( &hrtc, &sDate, RTC_FORMAT_BCD );
}

static void updateAlarm( void )
{
    /* Setting Alarm in BCD format */
    sAlarm.AlarmTime.Hours   = msgCasio.tm.tm_hour;
    sAlarm.AlarmTime.Minutes = msgCasio.tm.tm_min;
    
    /*Set the alarm */
    HAL_RTC_SetAlarm( &hrtc, &sAlarm, RTC_FORMAT_BCD );
}

static uint8_t elapsed1Seg( void )
{
    uint8_t elapsed = 0;

    HAL_RTC_GetTime( &hrtc, &sTime, RTC_FORMAT_BIN );
    HAL_RTC_GetDate( &hrtc, &sDate, RTC_FORMAT_BIN );

    if(sTime.Seconds == seg)
    {
        seg++;
        if(seg == 60) 
            seg = 0;
        elapsed = 1;   
    }

    return elapsed;
}

static void display( void )
{
    initialise_monitor_handles();

    printf("\n");
    printf("Time:  %02d:%02d:%02d\n\r", sTime.Hours, sTime.Minutes, sTime.Seconds);
    printf("Date:  %02d/%02d/%02d\n\r", sDate.Date, sDate.Month, sDate.Year);
    printf("Alarm: %02d:%02d\n\r", sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes);
}