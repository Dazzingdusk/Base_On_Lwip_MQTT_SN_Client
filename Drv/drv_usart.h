#ifndef __DRV_USART_H__
#define __DRV_USART_H__

#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include <stdio.h>



//----------------定义外设时钟------------------------
#define USART1_RCC (RCC_APB2Periph_USART1)
#define USART2_RCC (RCC_APB1Periph_USART2)
#define USART3_RCC (RCC_APB1Periph_USART3)
#define UART4_RCC (RCC_APB1Periph_UART4)
#define UART5_RCC (RCC_APB1Periph_UART5)


#define UART_HOST UART5
#define UART_HOST_IRQn UART5_IRQn
#define UART_HOST_BAUD_RATE 115200



typedef struct
{
    USART_TypeDef *UARTx_Num;

    GPIO_TypeDef *UARTx_TX_Port;
    GPIO_InitTypeDef UARTx_TX_Pin_InitStruct;

    GPIO_TypeDef *UARTx_RX_Port;
    GPIO_InitTypeDef UARTx_RX_Pin_InitStruct;

    USART_InitTypeDef UARTx_InitStruct;
    u8 UARTx_IntEnable;
    //u16 UARTx_InitType;
    NVIC_InitTypeDef UARTx_Nvic;
} DRV_UART_CFG_T;

void uart_init(void);
void host_uart_init(void);


#endif

