/**
 * @file    main.c
 * @brief   **Template Application entry point**
 *
 * The main file is the entry point of the application or any user code, please provide the 
 * proper description of this file according to your own implementation
 *
 * CasioCAN first real challenge
 * 
 * It is time for you to face a real challenge, during this phase you gonna develop an application that even
 * though it is really simple its develop is pretty close to what you gonna face in a real job. Be ready to
 * learn the most advanced programming techniques like reusable driver development, state machines, 
 * schedulers, and more. Step by step you will have to increase the complexity and the features requested to 
 * the point to reach a little more than 2000 lines of code written
 * 
 *
 * @note    Only the files inside folder app will be take them into account when the 
 *          doxygen runs by typing "make docs", index page is generated in
 *          Build/doxigen/html/index.html
 */
#include "app_bsp.h"
#include "app_serial.h"
#include "app_clock.h"

static uint32_t tickstart;
void heart_init( void );
void heart_beat( void );


/**
 * @brief   **Application entry point**
 *
 * Ptovide the proper description for function main according to your own
 * implementation
 * @retval  None
 */
int main( void )
{
     HAL_Init();
     heart_init();
     Serial_Init();
     Clock_Init();
 
     tickstart = HAL_GetTick( );
     for( ; ;)
     {
          Serial_Task();
          Clock_Task();
          heart_beat();
     }
    return 0u;
}

void heart_init(void)
{
     GPIO_InitTypeDef  GPIO_InitStruct;
     __HAL_RCC_GPIOC_CLK_ENABLE( );

     GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
     GPIO_InitStruct.Pull  = GPIO_NOPULL;
     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
     GPIO_InitStruct.Pin   = GPIO_PIN_0 ;
     HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );
}

void heart_beat(void)
{
     if( (HAL_GetTick( ) - tickstart ) >= 300)
     {
          HAL_GPIO_TogglePin( GPIOC, GPIO_PIN_0 ); /*invertimos led*/
          tickstart = HAL_GetTick( );
     }
}
