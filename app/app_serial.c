#include "app_serial.h"

/* Declaration of structure type variables for USER CAN initialization */
FDCAN_HandleTypeDef CANHandler;
static FDCAN_TxHeaderTypeDef CANTxHeader;

static uint8_t flag = 0u;

//extern void initialise_monitor_handles(void);
APP_MsgTypeDef Serial_Msg;


void Serial_Init( void )
{
    FDCAN_FilterTypeDef CANFilter;

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
    Serial_Msg.msg = SERIAL_MSG_NONE;
}

void Serial_Task( void )
{
    //initialise_monitor_handles();
    static APP_States state = SERIAL_STATE_IDLE;
    static uint8_t messageTx[8] = {0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; /*  (0 to 7) Single frame data length*/
    static uint8_t messageRx[8] = {0};

    switch(state)
    {
        case SERIAL_STATE_IDLE:
            if( CanTp_SingleFrameRx( messageRx,8u ) == 1u )
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
                    break;
            }
            break;

        case SERIAL_STATE_TIME:
            //printf("TIME\n\r");
            if( validateTime( messageRx ) == 1u )
            {
                
              //  printf("%0.2d:%0.2d:%0.2d Hrs\n\r",Serial_Msg.tm.tm_hour, Serial_Msg.tm.tm_min, Serial_Msg.tm.tm_sec);
                state = SERIAL_STATE_OK;
            }
            else
            {
                state = SERIAL_STATE_ERROR;
            }        
            break;

        case SERIAL_STATE_DATE:
            //printf("DATE\n\r");
            if( validateDate( messageRx ) == 1u )
            {
                //printf("%0.2d/%0.2d/%0.2d \n\r",Serial_Msg.tm.tm_mday, Serial_Msg.tm.tm_mon, Serial_Msg.tm.tm_year);
                state = SERIAL_STATE_OK;
            }
            else
            {
                state = SERIAL_STATE_ERROR;
            }
            break;

        case SERIAL_STATE_ALARM:
            //printf("ALARMA\n\r");
            if( validateAlarm( messageRx ) == 1u )
            {
                //printf("%0.2d:%0.2d Hrs\n\r",Serial_Msg.tm.tm_hour, Serial_Msg.tm.tm_min);
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
            //Serial_Msg.msg = SERIAL_MSG_NONE;
            break;

        case SERIAL_STATE_ERROR:
            //printf("ERROR\n\r");
            messageTx[1] = 0xAA;
            CanTp_SingleFrameTx(messageTx,8);
            state = SERIAL_STATE_IDLE;
           // Serial_Msg.msg = SERIAL_MSG_NONE;
            break;
        default:
    }
}

void CanTp_SingleFrameTx( uint8_t *data, uint8_t size )
{
    /*Colocanmos el mensaje en el buffer de salida y activamos el envio*/
    HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, data );
    uint8_t tmp = size;
}

uint8_t CanTp_SingleFrameRx( uint8_t *data, uint8_t *size )
{
    static FDCAN_RxHeaderTypeDef CANRxHeader;
    uint8_t status = 0;

    if( flag == 1u)
    {
         /* Retrieve Rx messages from RX FIFO0 */
        HAL_FDCAN_GetRxMessage( &CANHandler, FDCAN_RX_FIFO0, &CANRxHeader, data );
        status = 1u;
        flag = 0u; 
        *size = 0;
    }
    return status;
}

void HAL_FDCAN_RxFifo0Callback( FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs )
{
    if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0u)
    {
        /* Retrieve Rx messages from RX FIFO0 */
       
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

    day = ( data[2] >> (unsigned char)4 ) * (unsigned char)10 + ( data[2] & 15u );
    month = ( data[3] >> (unsigned char)4 ) * (unsigned char)10 + ( data[3] & 15u );
    year = ( ( data[4] >> 4u ) * 1000u ) + ( ( data[4] & 15u ) * 100u ) + ( ( data[5] >> 4u ) * 10u ) + ( data[5] & 15u );

    if( (month >= (unsigned char)1 ) && ( month <= (unsigned char)12 ) )
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
                if( ( ( year % 4u ) == 0u ) && ( year % 100u ) != ( year % 400u ) )
                {
                    maxDay = 29u;
                }
                else 
                {
                    maxDay = 28u;
                }
                break;
            default:
        }    
        if( ( day >= 1u ) && ( day <= maxDay ) && ( year >= 1901u ) && ( year <= 2099u ) )
        {
            success = 1u;
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
   
   hour = ( ( data[2] >> 4u ) * 10u ) + ( data[2] & 15u );
   min = ( ( data[3] >> 4u ) * 10u ) + ( data[3] & 15u );
   sec = ( ( data[4] >> 4u ) * 10u ) + ( data[4] & 15u );
   
   if( ( hour >= 0u )  && ( hour <= 23u ) && ( min >= 0u ) && ( min <= 59u )  && ( sec >= 0u ) && ( sec <= 59u ) )
   {
        success = 1u;
   }
   
   return success;
}

static uint8_t validateAlarm(uint8_t *data)
{
   uint32_t hour;
   uint32_t min;
   uint8_t success = 0u;
   
   hour = ( ( data[2] >> 4u ) * 10u ) + ( data[2] & 15u );
   min = ( ( data[3] >> 4u ) * 10u ) + ( data[3] & 15u );
    
   if( ( hour >= 0u ) && ( hour <= 23u) && ( min >= 0u ) && ( min <= 59u )  )
   {
        success = 1u;
   }
   
   return success;
}

static void updateMessageCAN( uint8_t *data )
{
    Serial_Msg.msg = data[1];
    switch(Serial_Msg.msg)
    {
        case SERIAL_MSG_TIME:
            Serial_Msg.tm.tm_hour = ( ( data[2] >> 4u ) * 10u ) + ( data[2] & 15u );
            Serial_Msg.tm.tm_min = ( ( data[3] >> 4u ) * 10u ) + ( data[3] & 15u );
            Serial_Msg.tm.tm_sec = ( ( data[4] >> 4u ) * 10u ) + ( data[4] & 15u );
            break;
        case SERIAL_MSG_DATE:
            Serial_Msg.tm.tm_mday = ( ( data[2] >> 4u ) * 10u ) + ( data[2] & 15u );
            Serial_Msg.tm.tm_mon = ( ( data[3] >> 4u ) * 10u ) + ( data[3] & 15u );
            Serial_Msg.tm.tm_year = ( ( data[4] >> 4u ) * 1000u ) + ( ( data[4] & 15u ) * 100u ) + ( ( data[5] >> 4u ) * 10u ) + ( data[5] & 15u );
            break;
        case SERIAL_MSG_ALARM:
            Serial_Msg.tm.tm_hour = ( ( data[2] >> 4u ) * 10u ) + ( data[2] & 15u );
            Serial_Msg.tm.tm_min = ( ( data[3] >> 4u ) * 10u ) + ( data[3] & 15u );
            break;
        default:
    }
}