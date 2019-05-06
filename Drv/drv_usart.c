#include "drv_usart.h"

DRV_UART_CFG_T Drv_Debug_UART =
{
    
    UART_HOST,// UARTNum
    GPIOC,//UART_TX_Port
    GPIO_Pin_12,//UART_TX_Pi
    GPIO_Speed_50MHz,//UART_TX_Pin_speed
    GPIO_Mode_AF_PP,//UART_TX_Pin_mode


    GPIOD,//UART_RX_PORT
    GPIO_Pin_2,//UART_RX_PIN
    GPIO_Speed_50MHz,//UART_RX_Pin_speed
    GPIO_Mode_IPU,//UART_RX_Pin_mode


    UART_HOST_BAUD_RATE,// UART_BaudRate
    USART_WordLength_9b,// UART_WordLength
    USART_StopBits_1,// UART_StopBits
    USART_Parity_Even,// UART_Parity
    USART_Mode_Rx | USART_Mode_Tx,// UART_Mode
    USART_HardwareFlowControl_None,// UART_HardwareFlowControl
    ENABLE,// UARTIntEnable
    //NULL,// UART_IT_RXNE | UART_IT_TXE,
    UART_HOST_IRQn,// NVIC_IRQChannel
    6,// NVIC_IRQChannelPreemptionPriority
    0,// NVIC_IRQChannelSubPriority
    ENABLE// NVIC_IRQChannelCmd
};

//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
int _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((UART5->SR&0X40)==0);//循环发送,直到发送完毕   
    UART5->DR = (u8) ch;      
	return ch;
}

static void uart_init_one_port(DRV_UART_CFG_T *Drv_UartCfgPtr)
{
    //uart相应io初始化
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);


    if (Drv_UartCfgPtr->UARTx_Num == UART_HOST)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    }
   

    GPIO_Init(Drv_UartCfgPtr->UARTx_TX_Port, &Drv_UartCfgPtr->UARTx_TX_Pin_InitStruct);
    GPIO_Init(Drv_UartCfgPtr->UARTx_RX_Port, &Drv_UartCfgPtr->UARTx_RX_Pin_InitStruct);

    /* Initialize UART parameters */
    USART_Init(Drv_UartCfgPtr->UARTx_Num, &Drv_UartCfgPtr->UARTx_InitStruct);

    /* NVIC control */
    if (Drv_UartCfgPtr->UARTx_IntEnable == ENABLE)
    {
        NVIC_Init(&Drv_UartCfgPtr->UARTx_Nvic);
    }
}
/* Initialize Iot_Debug_UART and Iot_Host_UART */
void uart_init(void)
{
    //u32 temp_baudrate;

    RCC_APB2PeriphClockCmd(USART1_RCC, DISABLE);
    RCC_APB1PeriphClockCmd(USART2_RCC, DISABLE);
    RCC_APB1PeriphClockCmd(USART3_RCC, DISABLE);
    RCC_APB1PeriphClockCmd(UART4_RCC, DISABLE);
    RCC_APB1PeriphClockCmd(UART5_RCC, DISABLE);

    RCC_APB2PeriphClockCmd(USART1_RCC, DISABLE);
    RCC_APB1PeriphClockCmd(USART2_RCC, DISABLE);
    RCC_APB1PeriphClockCmd(USART3_RCC, DISABLE);
    RCC_APB1PeriphClockCmd(UART4_RCC, ENABLE);
    RCC_APB1PeriphClockCmd(UART5_RCC, ENABLE);
    uart_init_one_port(&Drv_Debug_UART);

    /*start the UART debug*/
    USART_Cmd(UART_HOST, ENABLE);

}



