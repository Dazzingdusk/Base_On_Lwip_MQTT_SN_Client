/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#include "tcpecho.h"

#include "lwip/opt.h"

#if LWIP_NETCONN

#include "lwip/sys.h"
#include "lwip/api.h"
#include "string.h"
#include "stm32f10x.h"
/*-----------------------------------------------------------------------------------*/
static void 
tcpecho_thread(void *arg)
{
  struct netconn *conn, *newconn;
  err_t err;
  LWIP_UNUSED_ARG(arg);

  /* Create a new connection identifier. */
  /* Bind connection to well known port number 7. */
#if LWIP_IPV6
  conn = netconn_new(NETCONN_TCP_IPV6);
  netconn_bind(conn, IP6_ADDR_ANY, 7);
#else /* LWIP_IPV6 */
  conn = netconn_new(NETCONN_TCP);
  netconn_bind(conn, IP_ADDR_ANY, 7);
#endif /* LWIP_IPV6 */
  LWIP_ERROR("tcpecho: invalid conn", (conn != NULL), return;);

  /* Tell connection to go into listening mode. */
  netconn_listen(conn);

  while (1) {

    /* Grab new connection. */
    err = netconn_accept(conn, &newconn);
    /*printf("accepted new connection %p\n", newconn);*/
    /* Process the new connection. */
    if (err == ERR_OK) {
      struct netbuf *buf;
      void *data;
      u16_t len;
      
      while ((err = netconn_recv(newconn, &buf)) == ERR_OK) {
        /*printf("Recved\n");*/
        do {
             netbuf_data(buf, &data, &len);
             err = netconn_write(newconn, data, len, NETCONN_COPY);
#if 0
            if (err != ERR_OK) {
              printf("tcpecho: netconn_write: error \"%s\"\n", lwip_strerr(err));
            }
#endif
        } while (netbuf_next(buf) >= 0);
        netbuf_delete(buf);
      }
      /*printf("Got EOF, looping\n");*/ 
      /* Close connection and discard connection identifier. */
      netconn_close(newconn);
      netconn_delete(newconn);
    }
  }
}
//extern u8 tcp_server_recvbuf[1000];
//extern u8 *tcp_server_sendbuf;
////tcp����������
//static void tcp_server_thread(void *arg)
//{
//	unsigned int data_len = 0;
//	struct pbuf *q;
//	err_t err,recv_err;
//	struct netconn *conn, *newconn;
//	static ip_addr_t ipaddr;
//	static u16_t 			port;
//	
//	LWIP_UNUSED_ARG(arg);

//	conn = netconn_new(NETCONN_TCP);  //����һ��TCP����
//	netconn_bind(conn,IP_ADDR_ANY,7);  //�󶨶˿� 8�Ŷ˿�
//	netconn_listen(conn);  		//�������ģʽ
//	//conn->recv_timeout = 10;  	//��ֹ�����߳� �ȴ�10ms
//	while (1) 
//	{
//		err = netconn_accept(conn,&newconn);  //������������
//		if(err==ERR_OK)newconn->recv_timeout = 10;

//		if (err == ERR_OK)    //���������ӵ�����
//		{ 
//			struct netbuf *recvbuf;

//			netconn_getaddr(newconn,&ipaddr,&port,0); //��ȡԶ��IP��ַ�Ͷ˿ں�
//			while(1)
//			{
//				err = netconn_write(newconn ,tcp_server_sendbuf,strlen((char*)tcp_server_sendbuf),NETCONN_COPY); //����tcp_server_sendbuf�е�����
//				if(err != ERR_OK)
//				{
//				//	printf("����ʧ��\r\n");
//				}
//				if((recv_err = netconn_recv(newconn,&recvbuf)) == ERR_OK)  	//���յ�����
//				{		
//					memset(tcp_server_recvbuf,0,1000);  //���ݽ��ջ���������
//					for(q=recvbuf->p;q!=NULL;q=q->next)  //����������pbuf����
//					{
//						//�ж�Ҫ������TCP_SERVER_RX_BUFSIZE�е������Ƿ����TCP_SERVER_RX_BUFSIZE��ʣ��ռ䣬�������
//						//�Ļ���ֻ����TCP_SERVER_RX_BUFSIZE��ʣ�೤�ȵ����ݣ�����Ļ��Ϳ������е�����
//						if(q->len > (1000-data_len)) memcpy(tcp_server_recvbuf+data_len,q->payload,(1000-data_len));//��������
//						else memcpy(tcp_server_recvbuf+data_len,q->payload,q->len);
//						data_len += q->len;  	
//						if(data_len > 1000) break; //����TCP�ͻ��˽�������,����	
//					}
//					data_len=0;  //������ɺ�data_lenҪ���㡣	
//					netbuf_delete(recvbuf);
//				}else if(recv_err == ERR_CLSD)  //�ر�����
//				{
//					netconn_close(newconn);
//					netconn_delete(newconn);
//					break;
//				}
//			}
//		}
//	}
//}

/*-----------------------------------------------------------------------------------*/
void
tcpecho_init(void)
{
  sys_thread_new("tcpecho_thread", tcpecho_thread, NULL, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
}
/*-----------------------------------------------------------------------------------*/

#endif /* LWIP_NETCONN */
