#include "MQTT_SN_GateWay.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_it.h"
#include "stm32f10x.h"
#include "lwip/tcp.h"
#include "tcp_priv.h"
#include "lwip/memp.h"
#include "lwip/udp.h"
#include "misc.h"
#include "lwip/opt.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include <string.h>

unsigned char search_rsp_buf[100] = {0};//搜索回应数据区

void drv_UdpServer_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const struct ip4_addr *addr, u16_t port)
{
	struct ip4_addr destAddr = *addr; /* 发送IP */
	u32 temp_reg_readout;
    struct pbuf* search_rsp_p;
    
	u8* pdata;//udp数据指针
	pcb->remote_ip = *addr;
	pcb->remote_port = port;
	//广播数据
    pdata = p->payload;
    memcpy(search_rsp_buf,pdata,p->tot_len);
    memset(&search_rsp_buf[p->tot_len],'\0',1);
//    printf("Remoteport is %d\tUDP Data is: %s\r\n",pcb->remote_port,search_rsp_buf);
    search_rsp_p = pbuf_alloc(PBUF_RAW, p->tot_len+1, PBUF_RAM);
					search_rsp_p->payload = (void*)search_rsp_buf;
    
    udp_sendto(pcb, search_rsp_p, addr, pcb->remote_port);
    pbuf_free(search_rsp_p);
	pbuf_free(p);//释放该udp段
}
void drv_UdpServer_Init(void)
{
	struct udp_pcb* pcb;
	u32 temp_udp_port = 1883;

	pcb = udp_new();/* 建立通信的UDP控制块(pcb) */
	udp_bind(pcb, IP_ADDR_ANY, temp_udp_port);/* 绑定本地IP地址和端口号（作为udp服务器） */
	udp_recv(pcb, drv_UdpServer_recv, NULL);/* 设置UDP段到时的回调函数 */
} 
