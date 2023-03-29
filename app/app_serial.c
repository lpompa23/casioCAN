#include "app_serial.h"

/* Declaration of structure type variables for USER CAN initialization */
FDCAN_HandleTypeDef CANHandler;
FDCAN_TxHeaderTypeDef CANTxHeader;
FDCAN_RxHeaderTypeDef CANRxHeader;
FDCAN_FilterTypeDef CANFilter;
uint8_t messageTx[8];
uint8_t messageRx[8];
uint8_t flag = 0u;
static uint8_t state = SERIAL_STATE_IDLE;

GPIO_InitTypeDef GPIO_InitStruct;
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
    CANHandler.Init.StdFiltersNbr       = 0;
    CANHandler.Init.NominalPrescaler    = 10;
    CANHandler.Init.NominalSyncJumpWidth = 1;
    CANHandler.Init.NominalTimeSeg1     = 11;
    CANHandler.Init.NominalTimeSeg2     = 4;
    HAL_FDCAN_Init( &CANHandler);

    /* Declaramos las opciones para configurar los parametros de transmision CAN */
    CANTxHeader.IdType      = FDCAN_STANDARD_ID;
    CANTxHeader.FDFormat    = FDCAN_CLASSIC_CAN;
    CANTxHeader.TxFrameType = FDCAN_DATA_FRAME;
    CANTxHeader.Identifier  = 0x111;
    CANTxHeader.DataLength  = FDCAN_DLC_BYTES_8;

    /* Configure reception filter to Rx FIFO 0, este filtro solo aceptara mensajes con el ID 0x1FE */
    CANFilter.IdType = FDCAN_STANDARD_ID;
    CANFilter.FilterIndex = 0;
    CANFilter.FilterType = FDCAN_FILTER_MASK;
    CANFilter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    CANFilter.FilterID1 = 0x111;
    CANFilter.FilterID2 = 0x7FF;
    HAL_FDCAN_ConfigFilter( &CANHandler, &CANFilter );
    /*indicamos que los mensajes que no vengan con el filtro indicado sean rechazados*/
    HAL_FDCAN_ConfigGlobalFilter(&CANHandler, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE);
    
    /* Change FDCAN instance from initialization mode to normal mode */
    HAL_FDCAN_Start( &CANHandler);
    /*activamos la interrupcion por recepcion en el fifo0 cuando llega algun mensaje*/
    HAL_FDCAN_ActivateNotification( &CANHandler, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0 );
}

void Serial_Task( void )
{
    if( CanTp_SingleFrameRx(messageRx,8) )
    {
        state = SERIAL_STATE_MESSAGE;
    }
    Serial_State_Machine();
}

void Serial_State_Machine( void )
{
   
    switch(state)
    {
        case SERIAL_STATE_IDLE:
        break;

        case SERIAL_STATE_MESSAGE:
        break;

        case SERIAL_STATE_TIME:
        break;

        case SERIAL_STATE_DATE:
        break;

        case SERIAL_STATE_ALARM:
        break;

        case SERIAL_STATE_OK:
        break;

        case SERIAL_STATE_ERROR:
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