#include "app_serial.h"



void Serial_Init( void )
{
    
}

void Serial_Task( void )
{
    static uint8_t state = SERIAL_STATE_IDLE;

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

}

static uint8_t CanTp_SingleFrameRx( uint8_t *data, uint8_t *size )
{
    return 0u;
}