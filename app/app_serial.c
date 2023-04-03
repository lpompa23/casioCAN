#include "app_serial.h"

/* Declaration of structure type variables for USER CAN initialization */
FDCAN_HandleTypeDef CANHandler;
FDCAN_TxHeaderTypeDef CANTxHeader;
FDCAN_RxHeaderTypeDef CANRxHeader;
FDCAN_FilterTypeDef CANFilter;
uint8_t messageTx[8] = {0x07}; /*  (0 to 7) Single frame data length*/
uint8_t messageRx[8] = {0};
uint8_t flag = 0u;


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
    Serial_State_Machine();
}

void Serial_State_Machine( void )
{
    initialise_monitor_handles();
    static uint8_t state = SERIAL_STATE_IDLE;

    switch(state)
    {
        case SERIAL_STATE_IDLE:
            //if( CanTp_SingleFrameRx( messageRx,8 ) )
            //{
            //    state = SERIAL_STATE_MESSAGE;
            //}
            if( msgCasio.msg != SERIAL_MSG_NONE)
            {
                state = SERIAL_STATE_MESSAGE;
            }
            break;

        case SERIAL_STATE_MESSAGE:
            printf("MESSAGE\n\r");
            switch(msgCasio.msg)
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
            printf("TIME\n\r");
            if( validateTime( msgCasio.tm.tm_hour, msgCasio.tm.tm_min, msgCasio.tm.tm_sec ) )
            {
                printf("%0.2d:%0.2d:%0.2d Hrs\n\r",msgCasio.tm.tm_hour, msgCasio.tm.tm_min, msgCasio.tm.tm_sec);
                state = SERIAL_STATE_OK;
            }
            else
            {
                state = SERIAL_STATE_ERROR;
            }        
            break;

        case SERIAL_STATE_DATE:
            printf("DATE\n\r");
            if( validateDate( msgCasio.tm.tm_mday, msgCasio.tm.tm_mon, msgCasio.tm.tm_year) )
            {
                printf("%0.2d//%0.2d//%0.2d \n\r",msgCasio.tm.tm_mday, msgCasio.tm.tm_mon, msgCasio.tm.tm_year);
                state = SERIAL_STATE_IDLE;
            }
            else
            {
                state = SERIAL_STATE_ERROR;
            }
            break;

        case SERIAL_STATE_ALARM:
            printf("ALARMA\n\r");
            if( validateTime( msgCasio.tm.tm_hour, msgCasio.tm.tm_min, 0 ) )
            {
                printf("%0.2d:%0.2d Hrs\n\r",msgCasio.tm.tm_hour, msgCasio.tm.tm_min);
                state = SERIAL_STATE_OK;
            }
            else
            {
                state = SERIAL_STATE_ERROR;
            }     
            break;

        case SERIAL_STATE_OK:
            printf("OK\n\r");
            messageTx[1] = 0x55;
            CanTp_SingleFrameTx(messageTx,8);
            state = SERIAL_STATE_IDLE;
            break;

        case SERIAL_STATE_ERROR:
            printf("ERROR\n\r");
            messageTx[1] = 0xAA;
            CanTp_SingleFrameTx(messageTx,8);
            state = SERIAL_STATE_IDLE;
            break;
    }
}

static void CanTp_SingleFrameTx( uint8_t *data, uint8_t size )
{
    /*Colocanmos el mensaje en el buffer de salida y activamos el envio*/
    HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, messageTx );
}

static uint8_t CanTp_SingleFrameRx( uint8_t *data, uint8_t *size )
{
    uint8_t status = 0;

    if( flag == 1u)
    {
        msgCasio.msg = data[1];
        switch(msgCasio.msg)
        {
            case SERIAL_MSG_TIME:
                msgCasio.tm.tm_hour = data[2];
                msgCasio.tm.tm_min = data[3];
                msgCasio.tm.tm_sec = data[4];
                break;
            case SERIAL_MSG_DATE:
                msgCasio.tm.tm_mday = data[2];
                msgCasio.tm.tm_mon = data[3];
                uint32_t year = data[4] << 8;
                msgCasio.tm.tm_year = year + data[5] ;
                break;
            case SERIAL_MSG_ALARM:
                msgCasio.tm.tm_hour = data[2];
                msgCasio.tm.tm_min = data[3];
                break;
            default:
                msgCasio.msg = SERIAL_MSG_NONE;
        }
        flag = 0;
        status = 1;
    }
    return status;   
}

void HAL_FDCAN_RxFifo0Callback( FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs )
{
    if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0)
    {
        /* Retrieve Rx messages from RX FIFO0 */
        HAL_FDCAN_GetRxMessage( hfdcan, FDCAN_RX_FIFO0, &CANRxHeader, messageRx );
        flag = 1u;
    }
}

uint8_t validateDate(uint32_t day, uint32_t month, uint32_t year)
{
    uint8_t success = 0;
    uint32_t maxDay;

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

uint8_t validateTime(uint32_t hour, uint32_t min, uint32_t seg)
{
    uint8_t success = 0;

    if( (hour >= 0 && hour <= 23) && (min >= 0 && min <= 59) && (seg >= 0 && seg <= 59) )
    { 
        success = 1;
    }
    return success;
}