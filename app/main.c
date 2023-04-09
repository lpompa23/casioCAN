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

static uint32_t tickstart_heart;
static uint32_t tickstart_dog;
static WWDG_HandleTypeDef hwwdg;
static void heart_init( void );
static void heart_beat( void );
static void dog_init( void );
static void peth_the_dog( void );


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
     heart_init( );
     /* dog_init( ); */
     Serial_Init( );
     Clock_Init( ); 
       
     tickstart_heart = HAL_GetTick( );  
     tickstart_dog = HAL_GetTick( );

     for( ; ; )
     {    
  
          Serial_Task( );
          Clock_Task( );
          heart_beat( );
          /* peth_the_dog( ); */

     }
    return 0u;
}

static void heart_init(void)
{
     GPIO_InitTypeDef  GPIO_InitStruct;
     __HAL_RCC_GPIOC_CLK_ENABLE( );

     GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
     GPIO_InitStruct.Pull  = GPIO_NOPULL;
     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
     GPIO_InitStruct.Pin   = GPIO_PIN_0 ;
     HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );
}

static void heart_beat(void)
{
     if( (HAL_GetTick( ) - tickstart_heart ) >= 300u)
     {
          HAL_GPIO_TogglePin( GPIOC, GPIO_PIN_0 ); /*invertimos led*/
          tickstart_heart = HAL_GetTick( );
     }
}

static void dog_init(void)
{
     /* Para una ventana de 13ms y 20ms */
     hwwdg.Instance = WWDG;
     hwwdg.Init.Prescaler = WWDG_PRESCALER_4;
     hwwdg.Init.Window = 72u;
     hwwdg.Init.Counter = 84u;
     hwwdg.Init.EWIMode = WWDG_EWI_DISABLE;

     HAL_WWDG_Init(&hwwdg);     
}

static void peth_the_dog(void)
{
     if( (HAL_GetTick( ) - tickstart_dog ) >= 18)
     {
          HAL_WWDG_Refresh( &hwwdg );
          tickstart_dog = HAL_GetTick( );
     }
}
