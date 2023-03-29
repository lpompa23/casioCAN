/**------------------------------------------------------------------------------------------------
 * Archivo con la funciones de las incilaizaciones auxiliares de la libreria
-------------------------------------------------------------------------------------------------*/
#include "app_bsp.h"

void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef *hfdcan)
{
    GPIO_InitTypeDef GPIOCanStruct;

    /* Habilitamos los relojes de los perifericos GPIO y CAN */
    __HAL_RCC_FDCAN_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /* configuramos pin 0(rx) y pin 1(tx) en modo alterno para FDCAN1 */
    GPIOCanStruct.Mode = GPIO_MODE_AF_PP;
    GPIOCanStruct.Alternate = GPIO_AF3_FDCAN1;
    GPIOCanStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIOCanStruct.Pull = GPIO_NOPULL;
    GPIOCanStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init( GPIOD, &GPIOCanStruct );
}

void HAL_MspInit( void )
{
    
}
