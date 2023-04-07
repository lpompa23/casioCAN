#include "app_serial.h"

/* Declaration of structure type variables for USER CAN initialization */
FDCAN_HandleTypeDef CANHandler;
FDCAN_TxHeaderTypeDef CANTxHeader;
FDCAN_RxHeaderTypeDef CANRxHeader;
FDCAN_FilterTypeDef CANFilter;

static uint8_t flag = 0u;

//extern void initialise_monitor_handles(void);
APP_MsgTypeDef msgCasio;


void Serial_Init( void )
{
    /* Declaramos las opciones para configurar el modulo FDCAN1 para transmitir al bus CAN a 100Kbps
     y sample point de 75% */
    CANHandler.Instance                 = FDCAN1;
    CANHandler.Init.Mode                = FDCAN_MODE_NORMAL;
    CANHandler.Init.FrameFormat         = FDCAN_FRAME_CLASSIC;
    CANHandler.Init.ClockDivider        = FDCAN_CLOCK_DIV1;
    CANHandler.Init.TxFifoQueueMode     = FDCAN_TX_FIFO_OPERATION;
    CANHandler.Init.AutoRetransmission  = DISABLE;
    CANHandler.Init.TransmitPause       = DISABLE;
    CANHandler.Init.ProtocolException   = DISABLE;
    CANHandler.Init.ExtFiltersNbr       = 0;
    CANHandler.Init.StdFiltersNbr       = 1; /* indicamos que vamos a utilizar filtros */
    CANHandler.Init.NominalPrescaler    = 10;
    CANHandler.Init.NominalSyncJumpWidth = 1;
    CANHandler.Init.NominalTimeSeg1     = 11;
    CANHandler.Init.NominalTimeSeg2     = 4;
    HAL_FDCAN_Init( &CANHandler);

    /* Declaramos las opciones para configurar los parametros de transmision CAN */
    CANTxHeader.IdType      = FDCAN_STANDARD_ID;
    CANTxHeader.FDFormat    = FDCAN_CLASSIC_CAN;
    CANTxHeader.TxFrameType = FDCAN_DATA_FRAME;
    CANTxHeader.Identifier  = 0x111;    /* según requerimiento */
    CANTxHeader.DataLength  = FDCAN_DLC_BYTES_8;

    /* Configure reception filter to Rx FIFO 0, este filtro solo aceptara mensajes con el ID 0x1FE */
    CANFilter.IdType = FDCAN_STANDARD_ID;
    CANFilter.FilterIndex = 0;
    CANFilter.FilterType = FDCAN_FILTER_MASK;
    CANFilter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    CANFilter.FilterID1 = 0x111; /* segun requerimiento */
    CANFilter.FilterID2 = 0x7FF;
    HAL_FDCAN_ConfigFilter( &CANHandler, &CANFilter );
    /*indicamos que los mensajes que no vengan con el filtro indicado sean rechazados*/
    HAL_FDCAN_ConfigGlobalFilter(&CANHandler, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE);
    
    /* Change FDCAN instance from initialization mode to normal mode */
    HAL_FDCAN_Start( &CANHandler);
    /*activamos la interrupcion por recepcion en el fifo0 cuando llega algun mensaje*/
    HAL_FDCAN_ActivateNotification( &CANHandler, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0 );

    /* Inicializando la estructura de mensajes */
    msgCasio.msg = SERIAL_MSG_NONE;
}

void Serial_Task( void )
{
    //initialise_monitor_handles();
    static uint8_t state = SERIAL_STATE_IDLE;
    static uint8_t messageTx[8] = {0x07}; /*  (0 to 7) Single frame data length*/
    static uint8_t messageRx[8] = {0};

    switch(state)
    {
        case SERIAL_STATE_IDLE:
            if( CanTp_SingleFrameRx( messageRx,8u ) )
            {
                state = SERIAL_STATE_MESSAGE;
            }
            break;

        case SERIAL_STATE_MESSAGE:
           //printf("MESSAGE\n\r");
            switch(messageRx[1])
            {
                case SERIAL_MSG_TIME:
                    state = SERIAL_STATE_TIME;
                    break;

                case SERIAL_MSG_DATE:
                    state = SERIAL_STATE_DATE;
                    break;

                case SERIAL_MSG_ALARM:
                    state = SERIAL_STATE_ALARM;
                    break;
                default:
                    state = SERIAL_STATE_ERROR;
            }
            break;

        case SERIAL_STATE_TIME:
            //printf("TIME\n\r");
            if( validateTime( messageRx ) )
            {
                
              //  printf("%0.2d:%0.2d:%0.2d Hrs\n\r",msgCasio.tm.tm_hour, msgCasio.tm.tm_min, msgCasio.tm.tm_sec);
                state = SERIAL_STATE_OK;
            }
            else
            {
                state = SERIAL_STATE_ERROR;
            }        
            break;

        case SERIAL_STATE_DATE:
            //printf("DATE\n\r");
            if( validateDate( messageRx ) )
            {
                //printf("%0.2d/%0.2d/%0.2d \n\r",msgCasio.tm.tm_mday, msgCasio.tm.tm_mon, msgCasio.tm.tm_year);
                state = SERIAL_STATE_OK;
            }
            else
            {
                state = SERIAL_STATE_ERROR;
            }
            break;

        case SERIAL_STATE_ALARM:
            //printf("ALARMA\n\r");
            if( validateAlarm( messageRx ) )
            {
                //printf("%0.2d:%0.2d Hrs\n\r",msgCasio.tm.tm_hour, msgCasio.tm.tm_min);
                state = SERIAL_STATE_OK;
            }
            else
            {
                state = SERIAL_STATE_ERROR;
            }     
            break;

        case SERIAL_STATE_OK:
            //printf("OK\n\r");
            updateMessageCAN( messageRx );
            messageTx[1] = 0x55;
            CanTp_SingleFrameTx(messageTx,8);
            state = SERIAL_STATE_IDLE;
            //msgCasio.msg = SERIAL_MSG_NONE;
            break;

        case SERIAL_STATE_ERROR:
            //printf("ERROR\n\r");
            messageTx[1] = 0xAA;
            CanTp_SingleFrameTx(messageTx,8);
            state = SERIAL_STATE_IDLE;
           // msgCasio.msg = SERIAL_MSG_NONE;
            break;
    }
}

static void CanTp_SingleFrameTx( uint8_t *data, uint8_t size )
{
    /*Colocanmos el mensaje en el buffer de salida y activamos el envio*/
    HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, data );
}

static uint8_t CanTp_SingleFrameRx( uint8_t *data, uint8_t *size )
{
    uint8_t status = 0;

    if( flag == 1u)
    {
         /* Retrieve Rx messages from RX FIFO0 */
        HAL_FDCAN_GetRxMessage( &CANHandler, FDCAN_RX_FIFO0, &CANRxHeader, data );
        status = 1;
        flag = 0;
    }
    return status;
}

void HAL_FDCAN_RxFifo0Callback( FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs )
{
    if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0)
    {
       // /* Retrieve Rx messages from RX FIFO0 */
       // HAL_FDCAN_GetRxMessage( hfdcan, FDCAN_RX_FIFO0, &CANRxHeader, messageRx );
        flag = 1u;
    }
}

static uint8_t validateDate(uint8_t *data)
{
    uint8_t day;
    uint8_t month;
    uint32_t year;
    uint8_t success = 0;
    uint32_t maxDay;

    day = ( data[2] >> 4 ) * 10 + ( data[2] & 0x0F );
    month = ( data[3] >> 4 ) * 10 + ( data[3] & 0x0F );
    year = ( data[4] >> 4 ) * 1000 + ( data[4] & 0x0F ) * 100 + ( data[5] >> 4 ) * 10 + ( data[5] & 0x0F );

    if(month >= 1 && month <= 12)
    {
        switch(month)
        {
            case 1:
            case 3:
            case 5:
            case 7:
            case 8:
            case 10:
            case 12:
                maxDay = 31;
                break;
            case 4:
            case 6:
            case 9:
            case 11:
                maxDay = 30;
                break;
            case 2:
                if( year % 4 == 0 && year % 100 != year % 400 )
                {
                    maxDay = 29;
                }
                else 
                {
                    maxDay = 28;
                }
                break;
        }    
        if( (day >= 1 && day <= maxDay) && (year >= 1901 && year <= 2099) )
        {
            success = 1;
        }
    }
    return success; 
            
}

static uint8_t validateTime(uint8_t *data)
{
   uint32_t hour;
   uint32_t min;
   uint32_t sec;
   uint8_t success = 0;
   
   hour = ( data[2] >> 4 ) * 10 + ( data[2] & 0x0F );
   min = ( data[3] >> 4 ) * 10 + ( data[3] & 0x0F );
   sec = ( data[4] >> 4 ) * 10 + ( data[4] & 0x0F );
   
   if( (hour >= 0 && hour <= 23) && (min >= 0 && min <= 59) && (sec >= 0 && sec <= 59) )
   {
        success = 1;
   }
   
   return success;
}

static uint8_t validateAlarm(uint8_t *data)
{
   uint32_t hour;
   uint32_t min;
   uint8_t success = 0;
   
   hour = ( data[2] >> 4 ) * 10 + ( data[2] & 0x0F );
   min = ( data[3] >> 4 ) * 10 + ( data[3] & 0x0F );
    
   if( (hour >= 0 && hour <= 23) && (min >= 0 && min <= 59)  )
   {
        success = 1;
   }
   
   return success;
}

static void updateMessageCAN( uint8_t *data )
{
    msgCasio.msg = data[1];
    switch(msgCasio.msg)
    {
        case SERIAL_MSG_TIME:
            msgCasio.tm.tm_hour = ( data[2] >> 4 ) * 10 + ( data[2] & 0x0F );
            msgCasio.tm.tm_min = ( data[3] >> 4 ) * 10 + ( data[3] & 0x0F );
            msgCasio.tm.tm_sec = ( data[4] >> 4 ) * 10 + ( data[4] & 0x0F );
            break;
        case SERIAL_MSG_DATE:
            msgCasio.tm.tm_mday = ( data[2] >> 4 ) * 10 + ( data[2] & 0x0F );
            msgCasio.tm.tm_mon = ( data[3] >> 4 ) * 10 + ( data[3] & 0x0F );
            msgCasio.tm.tm_year = ( data[4] >> 4 ) * 1000 + ( data[4] & 0x0F ) * 100 + ( data[5] >> 4 ) * 10 + ( data[5] & 0x0F );
            break;
        case SERIAL_MSG_ALARM:
            msgCasio.tm.tm_hour = ( data[2] >> 4 ) * 10 + ( data[2] & 0x0F );
            msgCasio.tm.tm_min = ( data[3] >> 4 ) * 10 + ( data[3] & 0x0F );
            break;
    }
}