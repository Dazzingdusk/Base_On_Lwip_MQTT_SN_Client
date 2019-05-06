#ifndef __DRV_ETHERNET_H__
#define __DRV_ETHERNET_H__


#define PHY_ADDRESS       0x01 /* Relative to STM3210C-EVAL Board */

//#define MII_MODE          /* MII mode for STM3210C-EVAL Board (MB784) (check jumpers setting) */
#define RMII_MODE       /* RMII mode for STM3210C-EVAL Board (MB784) (check jumpers setting) */

void ethernet_init(void);
void lwip_periodic_handle(void);
u8 lwip_comm_init(void);
#endif

