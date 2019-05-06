#include "stm32_eth.h"
#include "drv_ethernet.h"
#include "ethernetif.h"
#include "lwip/ip_addr.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "netif/etharp.h"
#include "lwip/tcp.h"
#include "tcp_priv.h"
#include "lwip/dhcp.h"
#include "ethernetif.h" 
//#include "lwip/timers.h"
//#include "lwip/tcp_impl.h"
//#include "lwip/ip_frag.h"
#include "lwip/tcpip.h" 
#include <stdio.h>

/* ETHERNET errors */
#define  ETH_ERROR              ((uint32_t)0)
#define  ETH_SUCCESS            ((uint32_t)1)

u8 IP[]={192,168,45,223};
u8 NETMASK[]={255,255,255,0};
u8 GATEWAY[]={192,168,45,254};
u8 MACData[6] = {0xD4,0xBE,0xD9,0x99,0x85,0xBF};
struct netif lwip_netif;	
//����һ��ȫ�ֵ������
u32 TCPTimer=0;			//TCP��ѯ��ʱ��
u32 ARPTimer=0;			//ARP��ѯ��ʱ��
u32 lwip_localtime;		//lwip����ʱ�������,��λ:ms

#if LWIP_DHCP
u32 DHCPfineTimer=0;	//DHCP��ϸ�����ʱ��
u32 DHCPcoarseTimer=0;	//DHCP�ֲڴ����ʱ��
#endif
//��̫������io����
static void drv_ETH_GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIOs clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
		                RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

	/* ETHERNET pins configuration */
	/* AF Output Push Pull:
	- ETH_MII_MDIO / ETH_RMII_MDIO: PA2
	- ETH_MII_MDC / ETH_RMII_MDC: PC1
	- ETH_MII_TX_EN / ETH_RMII_TX_EN: PB11
	- ETH_MII_TXD0 / ETH_RMII_TXD0: PB12
	- ETH_MII_TXD1 / ETH_RMII_TXD1: PB13
	- ETH_MII_TXD2 / ETH_RMII_TXD2: PC2
	- ETH_MII_TXD3 / ETH_RMII_TXD3: PB8
								

	????????
	- ETH_MII_PPS_OUT / ETH_RMII_PPS_OUT: PB5
	- ETH_MII_TXD2: PC2
	- ETH_MII_TXD3: PB8 */
	//GPIO_PinRemapConfig(GPIO_Remap_ETH, ENABLE);
	/* Configure PA2 as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;//PA2_MII_MDIO
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure PC1, PC2 and PC3 as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;//ETH_MII_MDC @ ETH_MII_TXD2    | GPIO_Pin_2
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Configure PB5, PB8, PB11, PB12 and PB13 as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;//GPIO_Pin_8 | 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//ETH_MII_PPS_OUT @ ETH_MII_TXD3  @  ETH_MII_TX_EN
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/**************************************************************/
	/*               For Remapped Ethernet pins                   */
	/*************************************************************/
	/* Input (Reset Value):
	- ETH_MII_CRS CRS: PA0
	- ETH_MII_RX_CLK / ETH_RMII_REF_CLK: PA1
	- ETH_MII_RX_DV / ETH_RMII_CRS_DV: PD8   -->PA7
	- ETH_MII_RXD0 / ETH_RMII_RXD0: PD9  -->PC4
	- ETH_MII_RXD1 / ETH_RMII_RXD1: PD10 -->PC5
	- ETH_MII_RX_ER: PB10 

	- ETH_MII_COL: PA3
	- ETH_MII_RXD2: PB0
	- ETH_MII_RXD3: PB1
	- ETH_MII_TX_CLK: PC3
	???????
	- ETH_MII_TX_CLK: PC3
	- ETH_MII_RXD2: PD11
	- ETH_MII_RXD3: PD12
	- ETH_MII_COL: PA3*/

	/* ETHERNET pins remapp in STM3210C-EVAL board: RX_DV and RxD[3:0] */
	//GPIO_PinRemapConfig(GPIO_Remap_ETH, ENABLE);

	/* Configure PA0, PA1 and PA3 as input */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_7;//PA0_MII_CRS % PA1_PA8_MII_RX_CLK % PA3_MII_COL(??)   | GPIO_Pin_3 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure PB10 as input */
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_10;//PB10_MII_RX_ER   GPIO_Pin_0 | GPIO_Pin_1 | 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Configure PC3 as input */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;//PC3_MII_TX_CLK   GPIO_Pin_3 | 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* ADC Channel9 config --------------------------------------------------------*/
	/* Relative to STM3210D-EVAL Board   */
	/* Configure PB1(ADC Channel9) as analog input -------------------------*/
	//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	//  GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* MCO pin configuration------------------------------------------------- */
	/* Configure MCO (PA8) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;//PA1_PA8_MII_RX_CLK
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;          //????
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;         //????50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static void drv_ETH_Configuration(void)//��̫������Ӳ������
{
	ETH_InitTypeDef ETH_InitStructure;

	/* Enable ETHERNET clock  */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ETH_MAC | RCC_AHBPeriph_ETH_MAC_Tx | RCC_AHBPeriph_ETH_MAC_Rx, ENABLE);

	/* MII/RMII Media interface selection ------------------------------------------*/
#ifdef MII_MODE /* Mode MII with STM3210C-EVAL  */
	GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_MII);

	/* Get HSE clock = 25MHz on PA8 pin (MCO) */
	RCC_MCOConfig(RCC_MCO_HSE);

#elif defined RMII_MODE  /* Mode RMII with STM3210C-EVAL */
	GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_RMII);

	/* Set PLL3 clock output to 50MHz (25MHz /5 *10 =50MHz) */
	RCC_PLL3Config(RCC_PLL3Mul_10);
	/* Enable PLL3 */
	RCC_PLL3Cmd(ENABLE);
	/* Wait till PLL3 is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_PLL3RDY) == RESET);

	/* Get PLL3 clock on PA8 pin (MCO) */
	RCC_MCOConfig(RCC_MCO_PLL3CLK);
#endif

	/* Reset ETHERNET on AHB Bus */
	ETH_DeInit();

	/* Software reset */
	ETH_SoftwareReset();

	/* Wait for software reset */
	while (ETH_GetSoftwareResetStatus() == SET);

	/* ETHERNET Configuration ------------------------------------------------------*/
	/* Call ETH_StructInit if you don't like to configure all ETH_InitStructure parameter */
	ETH_StructInit(&ETH_InitStructure);

	/* Fill ETH_InitStructure parametrs */
	/*------------------------   MAC   -----------------------------------*/
	ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable  ;
	ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
	ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
	ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
	ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;
	ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
	ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
	ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
	ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
#ifdef CHECKSUM_BY_HARDWARE
	ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;
#endif

	/*------------------------   DMA   -----------------------------------*/  

	/* When we use the Checksum offload feature, we need to enable the Store and Forward mode: 
	the store and forward guarantee that a whole frame is stored in the FIFO, so the MAC can insert/verify the checksum, 
	if the checksum is OK the DMA can handle the frame otherwise the frame is dropped */
	ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable; 
	ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;         
	ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;     

	ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;       
	ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;   
	ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;                                                          
	ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;      
	ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;                
	ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;          
	ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;                                                                 
	ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;

	/* Configure Ethernet */
	//ETH_Init(&ETH_InitStructure, PHY_ADDRESS);
	while(ETH_Init(&ETH_InitStructure, PHY_ADDRESS) != ETH_SUCCESS);
	/* Enable the Ethernet Rx Interrupt */
	ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R, ENABLE);

}

static void drv_ETH_NVIC_Configuration(void)//��̫�������ж�����
{
	NVIC_InitTypeDef   NVIC_InitStructure;
    /* Enable the Ethernet global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = ETH_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);   
}

void ethernet_init(void)
{
	drv_ETH_GPIO_Configuration();
	drv_ETH_NVIC_Configuration();
	drv_ETH_Configuration();
}

//LWIP��ʼ��(LWIP������ʱ��ʹ��)
//����ֵ:0,�ɹ�
//      1,����
u8 lwip_comm_init(void)
{
	struct netif *Netif_Init_Flag;		//����netif_add()����ʱ�ķ���ֵ,�����ж������ʼ���Ƿ�ɹ�
	struct ip4_addr ipaddr;  			//ip��ַ
	struct ip4_addr netmask; 			//��������
	struct ip4_addr gw;      			//Ĭ������ 
	ethernet_init();					//��ʼ��DP83848ʧ�� 								
	tcpip_init(NULL,NULL);//��ʼ��LWIP�ں�

#if LWIP_DHCP		//ʹ�ö�̬IP
	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr = 0;
#else				//ʹ�þ�̬IP
	IP4_ADDR(&ipaddr,IP[0],IP[1], IP[2], IP[3]);
	IP4_ADDR(&netmask,NETMASK[0], NETMASK[1] , NETMASK[2], NETMASK[3]);
	IP4_ADDR(&gw, GATEWAY[0], GATEWAY[1], GATEWAY[2], GATEWAY[3]);
	Set_MAC_Address(MACData);
#endif
	Netif_Init_Flag=netif_add(&lwip_netif,&ipaddr,&netmask,&gw,NULL,&ethernetif_init,&ethernet_input);//�������б������һ������
	
#if LWIP_DHCP			//���ʹ��DHCP�Ļ�
	dhcpstatus=0;	//DHCP���Ϊ0
	dhcp_start(&lwip_netif);	//����DHCP����
#endif
	
	if(Netif_Init_Flag==NULL)return 3;//�������ʧ�� 
	else//������ӳɹ���,����netifΪĬ��ֵ,���Ҵ�netif����
	{
		netif_set_default(&lwip_netif); //����netifΪĬ������
		netif_set_up(&lwip_netif);		//��netif����
	}
	return 0;//����OK.
}   

//�����յ����ݺ���� 
void lwip_pkt_handle(void)
{
  //�����绺�����ж�ȡ���յ������ݰ������䷢�͸�LWIP���� 
 ethernetif_input(&lwip_netif);
}

//LWIP��ѯ����
void lwip_periodic_handle(void)
{
#if LWIP_TCP
	//ÿ250ms����һ��tcp_tmr()����
  if (lwip_localtime - TCPTimer >= TCP_TMR_INTERVAL)
  {
    TCPTimer =  lwip_localtime;
    tcp_tmr();
  }
#endif
  //ARPÿ5s�����Ե���һ��
  if ((lwip_localtime - ARPTimer) >= ARP_TMR_INTERVAL)
  {
    ARPTimer =  lwip_localtime;
    etharp_tmr();
  }

#if LWIP_DHCP //���ʹ��DHCP�Ļ�
  //ÿ500ms����һ��dhcp_fine_tmr()
  if (lwip_localtime - DHCPfineTimer >= DHCP_FINE_TIMER_MSECS)
  {
    DHCPfineTimer =  lwip_localtime;
    dhcp_fine_tmr();
    if (( dhcpstatus != 2)&&( dhcpstatus != 0XFF))
    { 
      lwip_dhcp_process_handle();  //DHCP����
    }
  }

  //ÿ60sִ��һ��DHCP�ֲڴ���
  if (lwip_localtime - DHCPcoarseTimer >= DHCP_COARSE_TIMER_MSECS)
  {
    DHCPcoarseTimer =  lwip_localtime;
    dhcp_coarse_tmr();
  }  
#endif
}


//���ʹ����DHCP
#if LWIP_DHCP
u8  dhcpstatus=0;
//DHCP��������
void lwip_dhcp_process_handle(void)
{
	u32 ip=0,netmask=0,gw=0;
	switch( dhcpstatus)
	{
		case 0: 	//����DHCP
			dhcp_start(&lwip_netif);
			dhcpstatus = 1;		//�ȴ�ͨ��DHCP��ȡ���ĵ�ַ
		case 1:		//�ȴ���ȡ��IP��ַ
		{
			ip=lwip_netif.ip_addr.addr;		//��ȡ��IP��ַ
			netmask=lwip_netif.netmask.addr;//��ȡ��������
			gw=lwip_netif.gw.addr;			//��ȡĬ������ 
			
			if(ip!=0)			//��ȷ��ȡ��IP��ַ��ʱ��
			{
				 dhcpstatus=2;	//DHCP�ɹ�
			}else if(lwip_netif.dhcp->tries>LWIP_MAX_DHCP_TRIES) //ͨ��DHCP�����ȡIP��ַʧ��,�ҳ�������Դ���
			{
				dhcpstatus=0XFF;//DHCP��ʱʧ��.
				//ʹ�þ�̬IP��ַ
				IP4_ADDR(&(lwip_netif.ip_addr), IP[0], IP[1], IP[2], IP[3]);
				IP4_ADDR(&(lwip_netif.NETMASK), NETMASK[0], NETMASK[1], NETMASK[2], NETMASK[3]);
				IP4_ADDR(&(lwip_netif.gw), GATEWAY[0], GATEWAY[1], GATEWAY[2], GATEWAY[3]);
			}
		}
		break;
		default : break;
	}
}
#endif
