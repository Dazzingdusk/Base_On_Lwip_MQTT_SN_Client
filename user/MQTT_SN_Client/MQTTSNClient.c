#include "lwip/tcp.h"
#include "lwip/memp.h"
#include "lwip/udp.h"
#include "lwip/opt.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include <string.h>
#include <stdio.h>

#include "MQTTSNTimes.h"
#include "MY_BUFF.h"
#include "MQTTSNClient.h"
#include "MQTTSNClientTopicMap.h"




MQTT_SN_Client_t    mqtt_sn_client;
MQTT_SN_GwList_t    GWlist;
MQTT_SN_GWInfor_t   gw; 
MQTT_SN_pub_infor_t publishinfor;
MQTT_SN_sub_infor_t subscribeInfor;


#define DEBUG_GW_LIST
#define DEBUG_CELIENT
//#define DEBUG_PACKET_INPUT

static void registeralltopic(void);
static void mqtt_sn_send_register_cb(void *arg);
static void mqtt_sn_client_publish_timeout_cb(void *arg);
void mqtt_sn_send_publishack(MQTT_SN_Client_t* mqttclient, uint16_t message_id,uint16_t topic_id,uint8_t return_code);

MQTT_SN_Client_t* GetClientPCB(void)
{
    return &mqtt_sn_client;
}
uint8_t getSpaceofpktinput(void)
{
    return GetSpaceofpktinput(mqtt_sn_client.rx_buffer.w_ptr, mqtt_sn_client.rx_buffer.r_ptr,mqtt_sn_client.rx_buffer.ring);
}
/**
  * @brief  IPv4��ַ�ṹ���ʽ������
  *   
  * @param  ip4_addr ������һ�� ip4�Ľṹ���ַ��
  *   
  * @param  abcd������IP��ַ���ĸ��ֽڽ���൱��a.b.c.d
  *   
  * @retval None
  */
void My_IP4_ADDR(struct ip4_addr *ipaddr, uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    ipaddr->addr = htonl(((u32_t)((a) & 0xff) << 24) | \
                         ((u32_t)((b) & 0xff) << 16) | \
                         ((u32_t)((c) & 0xff) << 8) | \
                         (u32_t)((d) & 0xff));
}
/******************************************************GW_LIST**********************************************************/
#ifdef DEBUG_GW_LIST
/**
  * @brief  �����б��״̬��ʾ����
  *   
  * @param  Gwlist �������б�ĵ�ַ��
  *    
  * @retval None
  */
void PrintfgwList(MQTT_SN_GwList_t* Gwlist)
{
    uint8_t i;
    printf("GW List len: %d\r\n",Gwlist->gwListAvalidNum);
    for(i=0;i<MQTT_GW_LISTNUM_MAX;i++)
    {
        /*����*/
        if(Gwlist->gwList[i].gw_usedflag)
        {
            printf("%d  GWID:%d   ",i,Gwlist->gwList[i].gw_ID);
            printf("IP:%d.%d.%d.%d   PORT:%d  ",*((uint8_t*)(&Gwlist->gwList[i].gw_addr)),*((uint8_t*)(&Gwlist->gwList[i].gw_addr)+1),\
            *((uint8_t*)(&Gwlist->gwList[i].gw_addr)+2),*((uint8_t*)(&Gwlist->gwList[i].gw_addr)+3),Gwlist->gwList[i].gw_port);
            printf("Keepalive:%d s  gw_Nadv:%d\r\n",Gwlist->gwList[i].gw_keepalive,Gwlist->gwList[i].gw_Nadv);
        }
    }
}
#endif
/**
  * @brief  �����б��ˢ�º�������Э��ջ�ڲ�����
  *   
  * @param  Gwlist �������б�ĵ�ַ
  * 
  * @param  addr ����������IP��ַ����Ϣ
  * 
  * @param  port ���������ض˿ڵ���Ϣ
  *
  * @param  gwid ����������ID����Ϣ
  *
  * @param  gwkeepalive ���������صĹ㲥ʱ����
  *
  * @retval 1 ʧ�ܿռ䲻��  0�ɹ�����
  */
uint8_t gwListappend(MQTT_SN_GwList_t *Gwlist,const struct ip4_addr addr,uint16_t port,uint8_t gwid,uint16_t gwkeepalive)
{
    uint8_t gwIndex,i;
    MQTT_SN_GwItem_t gwlist;
    
    gwlist.gw_addr = addr;
    gwlist.gw_ID = gwid; 
    gwlist.gw_keepalive = gwkeepalive; 
    gwlist.gw_Nadv = 0;
    gwlist.gw_port =  port;
    gwlist.gw_usedflag = 1;
    gwlist.gw_lasttime = mqtt_sn_client.cyclic_tick; 
    
    if(Gwlist->gwListAvalidNum == 0)
    {   
        memcpy(&(Gwlist->gwList[0]),&gwlist,sizeof(MQTT_SN_GwItem_t));
        Gwlist->gwListAvalidNum++;
        return 0;
    }
    if(Gwlist->gwListAvalidNum == MQTT_GW_LISTNUM_MAX)
    {    
         return 1;
    }
    for(i=0;i<MQTT_GW_LISTNUM_MAX;i++)
    {
        /*����*/
        if(gwid == Gwlist->gwList[i].gw_ID&&Gwlist->gwList[i].gw_usedflag)
        {
           memcpy(&(Gwlist->gwList[0]),&gwlist,sizeof(MQTT_SN_GwItem_t));
           return 0;                
        }
    }
    /*�ҵ�����λ��*/
    for(i=0;i<MQTT_GW_LISTNUM_MAX;i++)
    {
       if(Gwlist->gwList[i].gw_usedflag == 0)
        {
            gwIndex = i;
            break;
        } 
    }
    /*����*/ 
    memcpy(&(Gwlist->gwList[gwIndex]),&gwlist,sizeof(MQTT_SN_GwItem_t));
    Gwlist->gwListAvalidNum++;
    return 0;
    
}
/**
  * @brief  �����б��ɾ�������������س���ʱ�䳬ʱNadv�κ���ô˺���
  * ɾ����ʱ������Ŀ��  
  * @param  Gwlist �������б�ĵ�ַ
  * 
  * @param  index ����ɾ�����ص���������Ϣ
  * 
  * @retval 1 �����Ƿ�  2 GW�б�� 0�ɹ�ɾ��
  */
static uint8_t gwListdel(MQTT_SN_GwList_t* Gwlist,uint8_t index)
{
    if(index > MQTT_GW_LISTNUM_MAX){
        return 1;
    }
    if(Gwlist->gwListAvalidNum == 0){
        return 2;
    }
    /*ɾ��*/
    if(Gwlist->gwList[index].gw_usedflag)
    {
       Gwlist->gwList[index].gw_usedflag = 0;
       Gwlist->gwListAvalidNum--;
       #ifdef DEBUG_GW_LIST
       printf("Del Gwlist\r\n");
       PrintfgwList(Gwlist);
       #endif
       return 0;                
    }else{
     return 1;
    }
   
}
/**
  * @brief  �����б�ĳ�ʱά�����������ɿͻ�����ѭ�����ô˺����Լ��
  * �����Ƿ�ʱ����ʱ��ɾ�� 
  *
  * @param  client �ǿͻ��˿��ƿ��ַ
  *  
  * @retval None
  */
static void gwListupdata(MQTT_SN_Client_t *client)
{
    uint8_t i;
  
    if(client->gwList->gwListAvalidNum == 0)
    {   
        return ;
    }
    for(i=0;i<MQTT_GW_LISTNUM_MAX;i++)
    {
        /*����*/
        if(client->cyclic_tick > (client->gwList->gwList[i].gw_lasttime + client->gwList->gwList[i].gw_keepalive)\
            &&client->gwList->gwList[i].gw_usedflag)
        {
            client->gwList->gwList[i].gw_Nadv++;
            
           if(client->gwList->gwList[i].gw_Nadv > MQTT_SN_CILENT_NADV)
           {
               gwListdel(client->gwList,i);
           }
           else
           {
            client->gwList->gwList[i].gw_lasttime = client->cyclic_tick;
            #ifdef DEBUG_GW_LIST
            printf("one times \r\n");
            #endif
           }
        }
    }
    
    return ;
    
}
/**
  * @brief  �����б����Ŀ��ȡ������
  *   
  * @param  Gwlist �������б��ַ
  *  
  * @retval ���ص�һ����Ŀ��Ϣ����������DEBUG��
  */
MQTT_SN_GwItem_t* GetgwList(MQTT_SN_GwList_t* Gwlist)
{
    uint8_t i;
    
    for(i=0;i<MQTT_GW_LISTNUM_MAX;i++)
    {
        /*����*/
        if(Gwlist->gwList[i].gw_usedflag)
        {
           Gwlist->gwList[i].gw_usedflag = 0;
           Gwlist->gwListAvalidNum--;
           return 0;                
        }
    }
    return 0;
}
/****************************************************************************************************************/
/**
  * @brief  ���յ�UDP����ʱ������������д��buffͬʱ���ô˺������˴εı��ĳ���д��ͻ��˿��ƿ�Ĵ�
  * ������ϢFIFO�����ڶ�ȡbuff��Э��ջ�ڲ����á�
  *
  * @param  client �ǿͻ��˿��ƿ��ַ
  * 
  * @param  data ��д��buff�ı��ĳ�����Ϣ
  *
  * @retval 1 ʧ��-�ռ䲻��  0д��ɹ�
  */
uint8_t WriteClientImportindex(MQTT_SN_Client_t* client,uint8_t data )
{
    uint16_t space_avail;
    /*�������ݲ��Ϸ�*/
    if (data == NULL)
    {
        return 1;
    }
    /*ʣ�೤�Ȳ���*/
    space_avail	= GetSpaceofpktinput(client->rx_buffer.w_ptr, client->rx_buffer.r_ptr,client->rx_buffer.ring);
    #ifdef DEBUG_PACKET_INPUT
    printf("ClientImportindex space_avail_lenth: %d\r\n",space_avail);
    #endif
    if(1 > space_avail)
    {
        return 1;
    }

    client->rx_buffer.buff[client->rx_buffer.w_ptr++] = data; 
    #ifdef DEBUG_PACKET_INPUT
    printf("ClientImportindex Write w:%d r:%d\r\n",client->rx_buffer.w_ptr,client->rx_buffer.r_ptr);
    #endif
    if (client->rx_buffer.w_ptr == MQTT_SN_IMPORT_MAX)
    {
       client->rx_buffer.w_ptr = 0;
        
    }
    /*�����Ȧ*/
    if(client->rx_buffer.w_ptr == client->rx_buffer.r_ptr )
    {
        client->rx_buffer.ring = 1;
    }  
    return 0;
}
/**
  * @brief   ��ȡ�ͻ��˴�������Ϣ����FIFO���õ�Ҫ����ı��ĳ��ȣ����ڶ�ȡbuff 
  * 
  * @param   client �ǿͻ��˿��ƿ��ַ
  * 
  * @param   data ��д��buff�ı��ĳ�����Ϣ
  *
  * @retval  0 ��ȡʧ�� ���������ĳ���
  */
uint8_t ReadClientImportindex(MQTT_SN_Client_t* client)
{
    uint8_t data;
    uint16_t data_avail;
    /*ʣ�೤�Ȳ���*/
    data_avail	= GetAvalidofpktinput(client->rx_buffer.w_ptr, client->rx_buffer.r_ptr,client->rx_buffer.ring);
    #ifdef DEBUG_PACKET_INPUT
    printf("ClientImportindex data_avail_lenth: %d\r\n",data_avail);
    #endif
    if(1 > data_avail)
    {
        return 0;
    } 
    data=client->rx_buffer.buff[client->rx_buffer.r_ptr++];
    #ifdef DEBUG_PACKET_INPUT
    printf("ClientImportindex Read w:%d r:%d\r\n",client->rx_buffer.w_ptr,client->rx_buffer.r_ptr);
    #endif
    if (client->rx_buffer.r_ptr == MQTT_SN_IMPORT_MAX)
    {
       client->rx_buffer.r_ptr = 0;
        
    }
    /*�����Ȧ*/
    if(client->rx_buffer.ring )
    {
        client->rx_buffer.ring = 0;
    }
    return data;
}
/************************************************************************Sock********************************************************************/
/**
  * @brief   ͨ����ǰsock����UDP���ĵ�ָ����ַ��ָ���˿� 
  * 
  * @param   Sock �ǵ�ǰUDP���ƿ�
  * 
  * @param   addr Զ��Ŀ��IP��ַ
  *
  * @param   remote_port Զ��Ŀ�Ķ˿�
  * 
  * @param   datatosend Ҫ���͵�������
  *
  * @param   lenth �����͵�����������
  *
  * @retval  None
  */
static void SockSendDataStream(struct udp_pcb * Sock,const struct ip4_addr *addr,const uint16_t remote_port,uint8_t *datatosend,uint16_t lenth)
{
    struct pbuf* data_to_sent;
    
    data_to_sent = pbuf_alloc(PBUF_RAW, lenth , PBUF_RAM);
	data_to_sent->payload = (void*)datatosend;
    udp_sendto(Sock, data_to_sent, addr,remote_port);
    pbuf_free(data_to_sent);
    
}
/**
  * @brief   ͨ����ǰsock����UDP���ĵ���ǰ�ỰԶ�� 
  * 
  * @param   mqttclient �ǿͻ��˿��ƿ��ַ
  * 
  * @param   datatosend Ҫ���͵�������
  *
  * @param   lenth �����͵�����������
  *
  * @retval  None
  */
static void MQTTSNClientSendData(MQTT_SN_Client_t* mqttclient,uint8_t *datatosend,uint16_t lenth)
{
    struct pbuf* data_to_sent;
    
    data_to_sent = pbuf_alloc(PBUF_RAW, lenth , PBUF_RAM);
	data_to_sent->payload = (void*)datatosend;
    udp_send(mqttclient->Sock, data_to_sent);
    pbuf_free(data_to_sent);
    
}
/*******************************************************Callback API*********************************************************/
/**
  * @brief ����������Ϣ���Ļص�����������Ĭ�����س�ʱʱ����15����
  * 
  * @param  arg �����ص�IP��ַ�Ͷ˿���Ϣ�Ĵ���ṹ��ĵ�ַ
  * 
  * @retval None
  */
void api_gwinfo_press(void *arg)
{
    uint16_t keepalive = 900;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    MQTT_SN_GWInfor_t *GWinfor= arg;
    uint8_t freamlenth=ReadClientImportindex(&mqtt_sn_client);
    #ifdef  DEBUG_CELIENT
    printf("api_gwinfo_press\r\n");
    #endif
    if(freamlenth <= MQTT_SN_MAX_PACKET_LENGTH)
    {
        if(ReadBUFF(inputdata,freamlenth) == ERROR_NONE)
        {
            if(inputdata[0] == freamlenth)
            {
                if(inputdata[1] == MQTT_SN_TYPE_SEARCHGW )
                {
                    gwListappend(&GWlist,GWinfor->addr,GWinfor->port, inputdata[2],keepalive);
                }
            }
            #ifdef  DEBUG_CELIENT
            else
            {
                while(1);
            }
            #endif
        }
    }

}
/**
  * @brief  �������ع㲥��Ϣ���Ļص����� 
  * 
  * @param  arg �����ص�IP��ַ�Ͷ˿���Ϣ�Ĵ���ṹ��ĵ�ַ
  * 
  * @retval None
  */
void api_advertise_press(void *arg)
{
    uint16_t keepalive;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    MQTT_SN_GWInfor_t*  GWinfor= arg;
    advertise_packet_t* advertisemsg;
    uint8_t freamlenth=ReadClientImportindex(&mqtt_sn_client);
    
    #ifdef  DEBUG_CELIENT
    printf("api_advertise_press\r\n");
    #endif
    if(freamlenth <= MQTT_SN_MAX_PACKET_LENGTH)
    {
        if(ReadBUFF(inputdata,freamlenth) == ERROR_NONE)
        {
            advertisemsg = (advertise_packet_t*)inputdata;
            if(advertisemsg->length == freamlenth)
            {
                if(advertisemsg->type == MQTT_SN_TYPE_ADVERTISE )
                {
                    keepalive = advertisemsg->duration[0];
                    keepalive = keepalive>>8;
                    keepalive |= advertisemsg->duration[1];
                    gwListappend(&GWlist,GWinfor->addr,GWinfor->port, inputdata[2],keepalive);
                    #ifdef DEBUG_GW_LIST
                    PrintfgwList(&GWlist);
                    #endif
                }
            }
            #ifdef  DEBUG_CELIENT
            else
            {
                while(1);
            }
            #endif
        }
    }

}
/**
  * @brief  ��������Connetack���Ļص����� 
  * 
  * @param  arg û��
  * 
  * @retval None
  */
void api_connack_press(void *arg)
{
    uint8_t deltimeIndex;
    connack_packet_t returnmsg;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    uint8_t freamlenth=ReadClientImportindex(&mqtt_sn_client);
    #ifdef  DEBUG_CELIENT
    printf("api_connack_press\r\n");
    #endif
    deltimeIndex = FindTimerIndexbyID(TIMER_ID_OF_CONNCET); 
    if (deltimeIndex != 0xFF)
    {
        TimerStop(deltimeIndex);
        TimerDel(deltimeIndex);
    }
    if(freamlenth <= MQTT_SN_MAX_PACKET_LENGTH)
    {
        if(ReadBUFF(inputdata,freamlenth) == ERROR_NONE && freamlenth == sizeof(connack_packet_t))
        {
            memcpy(&returnmsg,inputdata,sizeof(connack_packet_t)); 
            if(returnmsg.length== freamlenth)
            {
                if(returnmsg.type == MQTT_SN_TYPE_CONNACK && (returnmsg.return_code == 0x00)
                    &&mqtt_sn_client.MqttSN_Status == MQTT_SN_CONNECTING)
                {
                    mqtt_sn_client.MqttSN_Status = MQTT_SN_CONNECTED;
                    #ifdef  DEBUG_CELIENT
                    printf("connect success.\r\n");
                    mqtt_sn_getgateway_info(&mqtt_sn_client,10000);
                    #endif  
                }
            }
            #ifdef  DEBUG_CELIENT
            else
            {
                //�쳣�����λ
                while(1);
            }
            #endif
        }
    }
}
/**
  * @brief  �������������й����ⱨ�Ļص����� 
  * 
  * @param  arg û��
  * 
  * @retval None
  */
void api_willtopicreq_press(void *arg)
{
    uint8_t reloadtimeIndex;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    uint8_t freamlenth=ReadClientImportindex(&mqtt_sn_client);
    #ifdef  DEBUG_CELIENT
    printf("api_willtopicreq_press\r\n");
    #endif
    reloadtimeIndex = FindTimerIndexbyID(TIMER_ID_OF_CONNCET); 
    if (reloadtimeIndex != 0xFF)
    {
        TimerReload(reloadtimeIndex);
    }
    if(freamlenth <= MQTT_SN_MAX_PACKET_LENGTH)
    {
        if(ReadBUFF(inputdata,freamlenth) == ERROR_NONE)
        {
            if(inputdata[0] == freamlenth)
            {
                if(inputdata[1] == MQTT_SN_TYPE_WILLTOPICREQ && mqtt_sn_client.MqttSN_Status == MQTT_SN_CONNECTING)
                {
                   /*��Ӧwilltopic*/
                    mqtt_sn_send_willtopic(&mqtt_sn_client,"lwip_test");
                }
            }
            #ifdef  DEBUG_CELIENT
            else
            {
                //�쳣�����λ
                while(1);
            }
            #endif
        }
    } 
}
/**
  * @brief  �������������й���Ϣ���Ļص����� 
  * 
  * @param  arg û��
  * 
  * @retval None
  */
void api_willmsgreq(void *arg)
{
    uint8_t reloadtimeIndex;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    uint8_t freamlenth=ReadClientImportindex(&mqtt_sn_client);
    #ifdef  DEBUG_CELIENT
    printf("api_willmsgreq\r\n");
    #endif
    reloadtimeIndex = FindTimerIndexbyID(TIMER_ID_OF_CONNCET); 
    if (reloadtimeIndex != 0xFF)
    {
        TimerReload(reloadtimeIndex);
    }
    if(freamlenth <= MQTT_SN_MAX_PACKET_LENGTH)
    {
        if(ReadBUFF(inputdata,freamlenth) == ERROR_NONE)
        {
            if(inputdata[0] == freamlenth)
            {
                if(inputdata[1] == MQTT_SN_TYPE_WILLMSGREQ && mqtt_sn_client.MqttSN_Status == MQTT_SN_CONNECTING)
                {
                   /*��Ӧwilltopic*/
                    mqtt_sn_send_willMSG(&mqtt_sn_client,"lwip_test");
                }
            }
            #ifdef  DEBUG_CELIENT
            else
            {
                //�쳣�����λ
                while(1);
            }
            #endif
        }
    } 
       
}
/**
  * @brief  �������ػ�Ӧping�����pingresponse��Ϣ���Ļص����� 
  * 
  * @param  arg û��
  * 
  * @retval None
  */
void api_pingres_press(void *arg)
{
    uint8_t deltimeIndex;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    uint8_t freamlenth=ReadClientImportindex(&mqtt_sn_client);
    #ifdef  DEBUG_CELIENT
    printf("api_pingres_press\r\n");
    #endif
    if(freamlenth <= MQTT_SN_MAX_PACKET_LENGTH)
    {
        if(ReadBUFF(inputdata,freamlenth) == ERROR_NONE)
        {
            if(inputdata[0] == freamlenth)
            {
                if(inputdata[1] == MQTT_SN_TYPE_PINGRESP && mqtt_sn_client.MqttSN_Status == MQTT_SN_CONNECTED)
                {
                    /*����ʱ*/
                    mqtt_sn_client.server_watchdog = mqtt_sn_client.keep_alive ;
                    mqtt_sn_client.ping_req_res_lost_num = 0;
                    /*ɾ����ʱ��ʱ��*/
                    deltimeIndex = FindTimerIndexbyID(TIMER_ID_OF_PINGREQ);
                    if (deltimeIndex != 0xFF)
                    {
                        TimerStop(deltimeIndex);
                        TimerDel(deltimeIndex);
                    }
                }
            }
            #ifdef  DEBUG_CELIENT
            else
            {
                //�쳣�����λ
                while(1);
            }
            #endif
        }
    } 
}
/**
  * @brief  ��������ping�����Ļص�����(����ֻҪ������״̬��˭ping�����) 
  * 
  * @param  arg û��
  * 
  * @retval None
  */
void api_pingreq_press(void *arg)
{
  
    ping_respond_packet_t ping_respond_msg;
    ping_respond_packet_t *ping_request_msg;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    
    uint8_t freamlenth=ReadClientImportindex(&mqtt_sn_client);
    #ifdef  DEBUG_CELIENT
    printf("api_pingreq_press\r\n");
    #endif
    if(freamlenth <= MQTT_SN_MAX_PACKET_LENGTH)
    {
        if(ReadBUFF(inputdata,freamlenth) == ERROR_NONE)
        {
            ping_request_msg = (ping_respond_packet_t*)inputdata;
            if(ping_request_msg->length == freamlenth)
            {
                if(ping_request_msg->type == MQTT_SN_TYPE_PINGREQ && mqtt_sn_client.MqttSN_Status == MQTT_SN_CONNECTED)
                {
                   ping_respond_msg.length = 2;
                   ping_respond_msg.type = MQTT_SN_TYPE_PINGRESP;
                   MQTTSNClientSendData(&mqtt_sn_client,(uint8_t*)&ping_respond_msg,sizeof(ping_respond_packet_t));
                }
            }
            #ifdef  DEBUG_CELIENT
            else
            {
                //�쳣�����λ
                while(1);
            }
            #endif
        }
    } 
}
/**
  * @brief  �������ػ�Ӧע�������regack��Ϣ���Ļص����� 
  * 
  * @param  arg û��
  * 
  * @retval None
  */
void api_regack_press(void *arg)
{
   
    regack_packet_t* regackmsg;
    uint8_t deltimeIndex;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    uint8_t freamlenth=ReadClientImportindex(&mqtt_sn_client);
    
    #ifdef  DEBUG_CELIENT
    printf("api_regack_press\r\n");
    #endif
    if(freamlenth <= MQTT_SN_MAX_PACKET_LENGTH)
    {
        if(ReadBUFF(inputdata,freamlenth) == ERROR_NONE)
        {
            regackmsg = (regack_packet_t*)inputdata;
            if(regackmsg->length == freamlenth)
            {
                
                if(regackmsg->type == MQTT_SN_TYPE_REGACK && mqtt_sn_client.MqttSN_Status == MQTT_SN_CONNECTED\
                    &&regackmsg->message_id == mqtt_sn_client.input_pack_id.inpub_pkt_id[REGGISTER])
                {
                    
                    /*ɾ����ʱ��ʱ��*/
                    deltimeIndex = FindTimerIndexbyID(TIMER_ID_OF_REGISTER); 
                    if (deltimeIndex != 0xFF)
                    {
                        TimerStop(deltimeIndex);
                        TimerDel(deltimeIndex);
                    } 
                    if(regackmsg->return_code == MQTT_SN_ACCEPTED){
                    /*ע��ȷ��,��ע��ID����ע��map*/
                    TopicidRegistered(regackmsg->topic_id);
                    }
                    else if(regackmsg->return_code == MQTT_SN_REJECTED_CONGESTION){
                    TimerAdd(TIMER_MODE_SINGLE,5*60*1000,mqtt_sn_send_register_cb,&mqtt_sn_client ,TIMER_ID_OF_REGISTER);    
                    /*ע��ӵ��*/
                    }
                    else{
                     TimerAdd(TIMER_MODE_SINGLE,60*1000,mqtt_sn_send_register_cb,&mqtt_sn_client ,TIMER_ID_OF_REGISTER);   
                    }
                }
            }
            #ifdef  DEBUG_CELIENT
            else
            {
                while(1);
            }
            #endif
        }
    }

}
/**
  * @brief  �������ػ�Ӧ���������puback��Ϣ���Ļص����� 
  * 
  * @param  arg û��
  * 
  * @retval None
  */
void api_puback_press(void *arg)
{
    puback_packet_t* pubackmsg;
    uint8_t deltimeIndex;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    uint8_t freamlenth=ReadClientImportindex(&mqtt_sn_client);
    
    #ifdef  DEBUG_CELIENT
    printf("api_puback_press\r\n");
    #endif
    if(freamlenth <= MQTT_SN_MAX_PACKET_LENGTH)
    {
        if(ReadBUFF(inputdata,freamlenth) == ERROR_NONE)
        {
            pubackmsg = (puback_packet_t*)inputdata;
            if(pubackmsg->length == freamlenth)
            {
                if(pubackmsg->type == MQTT_SN_TYPE_PUBACK && mqtt_sn_client.MqttSN_Status == MQTT_SN_CONNECTED\
                    &&pubackmsg->message_id == mqtt_sn_client.input_pack_id.inpub_pkt_id[PUBLISH])
                {
                    /*��������Ϣ�ĵȴ�pack IDɾ������־��û�����ڷ�����������Ϣ*/
                    mqtt_sn_client.input_pack_id.inpub_pkt_id[PUBLISH] = 0;
                   /*ɾ����ʱ��ʱ��*/
                    deltimeIndex = FindTimerIndexbyID(TIMER_ID_OF_PUBLISH); 
                    if (deltimeIndex != 0xFF)
                    {
                        TimerStop(deltimeIndex);
                        TimerDel(deltimeIndex);
                    } 
                    if(pubackmsg->return_code == MQTT_SN_ACCEPTED){
                    /*�����ɹ�ȷ��*/
                    #ifdef  DEBUG_CELIENT
                    printf("Publish Sucessed\r\n");
                    #endif
                    }
                    else if(pubackmsg->return_code == MQTT_SN_REJECTED_CONGESTION){
                    TimerAdd(TIMER_MODE_SINGLE,5*60*1000, mqtt_sn_client_publish_timeout_cb,&mqtt_sn_client ,TIMER_ID_OF_PUBLISH);    
                    /*ӵ��*/
                    }
                    else if(pubackmsg->return_code == MQTT_SN_REJECTED_INVALID){  
                    /*��֧�ֵ�ID������ע��*/
                    TopicidUnregister(pubackmsg->topic_id);
                    }
                    else{
                        #ifdef  DEBUG_CELIENT
                        printf("Publish Unsupport\r\n");
                        #endif
                    }
                }
            }
            #ifdef  DEBUG_CELIENT
            else
            {
                while(1);
            }
            #endif
        }
    }
}
/**
  * @brief  �������ػ�Ӧ���������puback��Ϣ���Ļص����� 
  * 
  * @param  arg û��
  * 
  * @retval None
  */
void api_publish_press(void *arg)
{
    publish_packet_t* publishmsg;
    uint8_t deltimeIndex;
    #ifdef  DEBUG_CELIENT
    uint8_t i;
    #endif
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    uint8_t freamlenth=ReadClientImportindex(&mqtt_sn_client);
    
    #ifdef  DEBUG_CELIENT
    printf("api_publish_press\r\n");
    #endif
    if(freamlenth <= MQTT_SN_MAX_PACKET_LENGTH)
    {
        if(ReadBUFF(inputdata,freamlenth) == ERROR_NONE)
        {
            publishmsg = (publish_packet_t*)inputdata;
            if(publishmsg->length == freamlenth)
            {
                if(publishmsg->type == MQTT_SN_TYPE_PUBLISH )
                {
                    if(publishmsg->flags&MQTT_SN_FLAG_QOS_1){
                        /*����ACK*/
                        mqtt_sn_send_publishack(&mqtt_sn_client,publishmsg->message_id,publishmsg->topic_id,0);
                    }
                    #ifdef  DEBUG_CELIENT
                    printf("publish_msg:");
                    for(i=0;i<(publishmsg->length-7);i++)
                       printf("%c",publishmsg->data[i]); 
                    printf("\r\n");
                    #endif
                }
            }
            #ifdef  DEBUG_CELIENT
            else
            {
                while(1);
            }
            #endif
        }
    }
}
/**
  * @brief  �������ػ�Ӧping�����pingresponse��Ϣ���Ļص����� 
  * 
  * @param  arg û��
  * 
  * @retval None
  */
void api_pubcomp_press(void *arg)
{
    #ifdef  DEBUG_CELIENT
    printf("api_pubcomp_press\r\n");
    #endif  
}
/**
  * @brief  �������ػ�Ӧping�����pingresponse��Ϣ���Ļص����� 
  * 
  * @param  arg û��
  * 
  * @retval None
  */
void api_pubrec_press(void *arg)
{
    #ifdef  DEBUG_CELIENT
    printf("api_pubrec_press\r\n");
    #endif
}
/**
  * @brief  �������ػ�Ӧping�����pingresponse��Ϣ���Ļص����� 
  * 
  * @param  arg û��
  * 
  * @retval None
  */
void api_pubrel_press(void *arg)
{
    #ifdef  DEBUG_CELIENT
    printf("api_pubrel_press\r\n");
    #endif
}
/**
  * @brief  �������ػ�Ӧ���������suback��Ϣ���Ļص����� 
  * 
  * @param  arg û��
  * 
  * @retval None
  */
void api_suback_press(void *arg)
{
    suback_packet_t* subackmsg;
    uint8_t deltimeIndex;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    uint8_t freamlenth=ReadClientImportindex(&mqtt_sn_client);
    
    #ifdef  DEBUG_CELIENT
    printf("api_suback_press\r\n");
    #endif
    if(freamlenth <= MQTT_SN_MAX_PACKET_LENGTH)
    {
        if(ReadBUFF(inputdata,freamlenth) == ERROR_NONE)
        {
            subackmsg = (suback_packet_t*)inputdata;
            if(subackmsg->length == freamlenth)
            {
                if(subackmsg->type == MQTT_SN_TYPE_SUBACK&&subackmsg->message_id == mqtt_sn_client.input_pack_id.inpub_pkt_id[SUBSCARIBE])
                {
                   /*ɾ����ʱ��ʱ��*/
                    deltimeIndex = FindTimerIndexbyID(TIMER_ID_OF_SUBSCRIBE); 
                    if (deltimeIndex != 0xFF)
                    {
                        TimerStop(deltimeIndex);
                        TimerDel(deltimeIndex);
                    }
                }
            }
            #ifdef  DEBUG_CELIENT
            else
            {
                while(1);
            }
            #endif
        }
    }

}
/**
  * @brief  �������ػ�Ӧȡ�����������unsub��Ϣ���Ļص����� 
  * 
  * @param  arg û��
  * 
  * @retval None
  */
void api_unsuback_press(void *arg)
{
    unsuback_packet_t* unsubackmsg;
    uint8_t deltimeIndex;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    uint8_t freamlenth=ReadClientImportindex(&mqtt_sn_client);
    
    #ifdef  DEBUG_CELIENT
    printf("api_unsuback_press\r\n");
    #endif
    if(freamlenth <= MQTT_SN_MAX_PACKET_LENGTH)
    {
        if(ReadBUFF(inputdata,freamlenth) == ERROR_NONE)
        {
            unsubackmsg = (unsuback_packet_t*)inputdata;
            if(unsubackmsg->length == freamlenth)
            {
                if(unsubackmsg->type == MQTT_SN_TYPE_UNSUBACK&&unsubackmsg->message_id == mqtt_sn_client.input_pack_id.inpub_pkt_id[UNSUBSCARIBE])
                {
                   /*ɾ����ʱ��ʱ��*/
                   printf("unsuback_press is OK\r\n");
                }
            }
            #ifdef  DEBUG_CELIENT
            else
            {
                while(1);
            }
            #endif
        }
    }
}
/**
  * @brief  �������ػ�Ӧ�Ͽ����������disconnect��Ϣ���Ļص����� 
  * 
  * @param  arg û��
  * 
  * @retval None
  */
void api_disconnet_press(void *arg)
{
    disconnect_packet_t* disconnetmsg;
    uint8_t deltimeIndex;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    uint8_t freamlenth=ReadClientImportindex(&mqtt_sn_client);
    
    #ifdef  DEBUG_CELIENT
    printf("api_disconnet_press\r\n");
    #endif
    if(freamlenth <= MQTT_SN_MAX_PACKET_LENGTH)
    {
        if(ReadBUFF(inputdata,freamlenth) == ERROR_NONE)
        {
            disconnetmsg = (disconnect_packet_t*)inputdata;
            if(disconnetmsg->length == freamlenth)
            {
                if(disconnetmsg->type == MQTT_SN_TYPE_DISCONNECT)
                {
                   /*ɾ����ʱ��ʱ��*/
                    deltimeIndex = FindTimerIndexbyID(TIMER_ID_OF_DISCONNCET); 
                    if (deltimeIndex != 0xFF)
                    {
                        TimerStop(deltimeIndex);
                        TimerDel(deltimeIndex);
                    }
                    mqtt_sn_client.MqttSN_Status = MQTT_SN_NOTCONNECT;
                }
            }
            #ifdef  DEBUG_CELIENT
            else
            {
                while(1);
            }
            #endif
        }
    }
}

const API_TAB_T g_api_table[API_NUM] = 
{
    {MQTT_SN_TYPE_ADVERTISE          ,      api_advertise_press       },
    {MQTT_SN_TYPE_GWINFO             ,      api_gwinfo_press          },
    {MQTT_SN_TYPE_CONNACK            ,      api_connack_press         },
    {MQTT_SN_TYPE_WILLTOPICREQ       ,      api_willtopicreq_press    },
    {MQTT_SN_TYPE_WILLMSGREQ         ,      api_willmsgreq            },
    {MQTT_SN_TYPE_REGACK             ,      api_regack_press          },
    {MQTT_SN_TYPE_PUBACK             ,      api_puback_press          },
    {MQTT_SN_TYPE_PUBCOMP            ,      api_pubcomp_press         },
    {MQTT_SN_TYPE_PUBREC             ,      api_pubrec_press          },
    {MQTT_SN_TYPE_PUBREL             ,      api_pubrel_press          },
    {MQTT_SN_TYPE_SUBACK             ,      api_suback_press          },
    {MQTT_SN_TYPE_UNSUBACK           ,      api_unsuback_press        },
    {MQTT_SN_TYPE_DISCONNECT         ,      api_disconnet_press       },
    {MQTT_SN_TYPE_PINGRESP           ,      api_pingres_press         },
    {MQTT_SN_TYPE_PINGREQ            ,      api_pingreq_press         },
    {MQTT_SN_TYPE_PUBLISH            ,      api_publish_press         },
    
    
};
/**
  * @brief  �������뷵�ش���ص����� 
  * 
  * @param  cmd MQTT_SN����
  * 
  * @retval �ص�����
  */
FUN_TIMER_CALLBACK_T getAPIFUN(uint8_t cmd)
{
    uint8_t n=0;
    for(n=0;n<API_NUM;n++)
    {
        if(g_api_table[n].cmd_id == cmd)
            return g_api_table[n].api_func;
        
    }
    return NULL;
}
/******************************************************************************************************************************/
/**
  * @brief   �յ�UDP���ĵĻص���������Ҫ�ǽ�����д��buff������һ���ص�ȥ����
  * 
  * @param   mqttclient �ǿͻ��˿��ƿ��ַ
  * 
  * @param   addr Զ��IP��ַ
  *
  * @param   port Զ�˶˿�
  * 
  * @retval  None
  */
static void mqtt_sn_socket_incomm_cb(MQTT_SN_Client_t* mqttclient, const struct ip4_addr *addr, uint16_t port,uint8_t cmd)
{
  
    FUN_TIMER_CALLBACK_T cb;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    
    /*�˴���������������Ϣ*/
    gw.addr = *addr;
    gw.port = port;
    /*ping response*/
    
    cb = getAPIFUN(cmd);
    if(cb !=NULL){
        TimerAdd(TIMER_MODE_SINGLE,100,cb,(void*)&gw,NULL);
     }else{
        /*��������*/
        ReadBUFF(inputdata,ReadClientImportindex(mqttclient));
     }
}
/**
  * @brief   �ж��յ�UDP�����Ƿ���MQTT-SN���������
  * 
  * @param   cmd ����
  *
  * @retval  0 ��mqtt�����  1 ����
  */
static uint8_t CmdConfirm(uint8_t cmd)
{
    if ((0x00 <= cmd)&&(cmd <= 0x1D))
    {
        if (cmd != 0x03 &&cmd != 0x11 &&cmd != 0x19)
            return 0;
    }
    return 1;
}
/**
  * @brief   �յ�UDP���ĵĻص��������˺�����LWIPЭ��ջ�ص�
  * 
  * @param   arg ����
  * 
  * @param   pcb ucp���ƿ�
  *
  * @param   p ���ݰ���ַ
  * 
  * @param   addr Զ�˵�ַ
  *
  * @param   port Զ�˶˿�
  *
  * @retval  None
  */
void UdpServer_recv_cb(void *arg, struct udp_pcb *pcb, struct pbuf *p, const struct ip4_addr *addr, uint16_t port)
{
    uint8_t *pdata = p->payload;
    uint8_t cmd;
    pcb->remote_port = port;
    pcb->remote_ip = *addr;
    if(*pdata == 0x01){
        cmd = *(pdata+3);
    }else{
        cmd = *(pdata+1);
    }
    if (CmdConfirm(cmd)){
        /*�������ݰ�*/
        pbuf_free(p);
        return ;
    }
    //��֤д��ɹ�
    if (GetSpaceofpktinput(mqtt_sn_client.rx_buffer.w_ptr, mqtt_sn_client.rx_buffer.r_ptr,mqtt_sn_client.rx_buffer.ring)&&\
        BuffSpaceget() > (p->tot_len))
    {
        //�㲥����
        if(WriteClientImportindex(&mqtt_sn_client,p->tot_len)==0)
        {
            if (WriteBUFF(p->payload,p->tot_len) == 0)
                mqtt_sn_socket_incomm_cb(&mqtt_sn_client,addr,port,cmd);
        } 
    }
    /*�ͷŸ�udp��*/
    pbuf_free(p);
      
}
/**
  * @brief   ����һ��UDP sockt ���󶨵��ͻ��˵�ַ�� 
  *          ��ʼ����Ϣbuff
  * @param   mqttclient �ǿͻ��˿��ƿ��ַ
  * 
  * @retval  None
  */
void mqtt_sn_create_socket(MQTT_SN_Client_t* mqttclient)
{
	mqttclient->Sock  = udp_new();/* ����ͨ�ŵ�UDP���ƿ�(pcb) */
	udp_bind(mqttclient->Sock, IP_ADDR_ANY, MQTT_SN_DEFAULT_PORT);/* �󶨱���IP��ַ�Ͷ˿ںţ���Ϊudp�������� */
	udp_recv(mqttclient->Sock, UdpServer_recv_cb, NULL);/* ����UDP�ε�ʱ�Ļص����� */
    BuffInit();
}
/**
  * @brief   ����publishȷ����Ϣ������
  * 
  * @param   client �ǿͻ��˿��ƿ��ַ
  * 
  * @param   message_id ���յ��ķ�����Ϣ��ΨһID
  * 
  * @param   topic_id ���յ��ķ�����Ϣ������ID
  * 
  * @param   return_code ���յ���״̬
  *
  * @retval  None
  */
void mqtt_sn_send_publishack(MQTT_SN_Client_t* mqttclient, uint16_t message_id,uint16_t topic_id,uint8_t return_code)
{
    puback_packet_t  pubackmsg;
    
    pubackmsg.message_id = message_id;
    pubackmsg.topic_id = topic_id;
    pubackmsg.type = MQTT_SN_TYPE_SUBACK;
    pubackmsg.return_code = return_code;
    
    pubackmsg.length = 7;
    
    
    MQTTSNClientSendData(mqttclient,(uint8_t*)&pubackmsg,pubackmsg.length);
}
/**
  * @brief   ��������������Ϣ�Ĺ㲥����
  * 
  * @param   client �ǿͻ��˿��ƿ��ַ
  * 
  * @param   remote_port �㲥����Զ�˶˿�
  *
  * @retval  None
  */
void mqtt_sn_getgateway_info(MQTT_SN_Client_t* mqttclient,const uint16_t remote_port)
{
    struct ip4_addr destAddr;
    uint8_t searchinfo[3] = {0x02,0x01,0x00};
	My_IP4_ADDR(&destAddr,255,255,255,255);
    SockSendDataStream(mqttclient->Sock,&destAddr,remote_port,searchinfo,3);
}
/**
  * @brief   �ͻ����������س�ʱ�ص�����  
  * 
  * @param   arg δʹ��
  *
  * @retval  0 ��ȡʧ�� ���������ĳ���
  */
void mqtt_sn_send_connect_cb(void *arg)
{
    MQTT_SN_Client_t* mqttclient = arg;
    uint8_t flag = flag|MQTT_SN_FLAG_CLEAN;
    if(mqttclient->MqttSN_Status == MQTT_SN_CONNECTED)
    {
        printf("connect success\r\n");
    }else{
        printf("connect return %d\r\n",0xFF);
        mqtt_sn_send_connect(&mqtt_sn_client,"lwip_test",60,flag);
    }
}
/**
  * @brief   �ͻ����������� 
  * 
  * @param   client_id �ǿͻ���id
  * 
  * @param   keepalive �ͻ��˱��ʱ�����ʱ��
  * 
  * @param   flags �������ӱ�־
  *
  * @retval  0 ��ȡʧ�� ���������ĳ���
  */
void mqtt_sn_send_connect(MQTT_SN_Client_t* mqttclient,const char * client_id, uint16_t keepalive,uint8_t flags)
{
    connect_packet_t  ConnetMssage;
    struct ip4_addr   testaddr;
    
    memset(&ConnetMssage,0,sizeof(connect_packet_t));
    
    mqttclient->keep_alive = keepalive;
    mqttclient->server_watchdog = keepalive;
    
    ConnetMssage.flags = 0;
    memcpy(mqttclient->clientID,client_id,strlen(client_id)+1);
    
    memcpy(ConnetMssage.client_id,client_id,strlen(client_id));
    
    
    ConnetMssage.length = strlen(client_id)+MQTT_SN_CONNET_FIXLENTH;
    ConnetMssage.duration = keepalive;
    ConnetMssage.protocol_id = MQTT_SN_PROTOCOL_ID;
    ConnetMssage.type = MQTT_SN_TYPE_CONNECT;
    ConnetMssage.flags |=  flags;
    
    My_IP4_ADDR(&testaddr,192,168,45,119);
    if(mqttclient->gwList->gwListAvalidNum >= 0)
    {
        SockSendDataStream(mqttclient->Sock,&testaddr,10000,(uint8_t*)&ConnetMssage,ConnetMssage.length);
        mqttclient->MqttSN_Status = MQTT_SN_CONNECTING;
        TimerAdd(TIMER_MODE_SINGLE,4000,mqtt_sn_send_connect_cb,mqttclient,TIMER_ID_OF_CONNCET);
    }
    else
    {
        mqtt_sn_getgateway_info(mqttclient,10000);
    }   
}
/**
  * @brief   �ͻ���ע�����ⳬʱ�ص����� 
  * 
  * @param   arg �ǿͻ��˿��ƿ��ַ
  *
  * @retval  0 ��ȡʧ�� ���������ĳ���
  */
static void mqtt_sn_send_register_cb(void *arg)
{
    uint8_t i;
    MQTT_SN_Client_t* client = arg;
    topic_map_t* p_map = GetTopicMapAddr();
    for(i =0;i<MQTT_SN_CLIENT_LIST_MAX;i++)
    {
        if(p_map->toppiclist[i].status == registering)
        {
            mqtt_sn_send_register(client,topiclist[i]);
        }
    }
}
/**
  * @brief   �ͻ���ע������ID���� 
  * 
  * @param   mqttclient �ǿͻ��˿��ƿ��ַ
  * 
  * @param   topic_name ע���������
  *
  * @retval  None
  */
void mqtt_sn_send_register(MQTT_SN_Client_t* mqttclient,const char* topic_name)
{
    register_packet_t RegisterMssage;
    memset(&RegisterMssage,0,sizeof(register_packet_t));
    
    RegisterMssage.length = strlen(topic_name) + MQTT_SN_REGISTER_FIXLENTH;
    RegisterMssage.type = MQTT_SN_TYPE_REGISTER;
    
    RegisterMssage.message_id = mqttclient->pkt_id++;
    mqttclient->input_pack_id.inpub_pkt_id[REGGISTER] = RegisterMssage.message_id;
    
    RegisterMssage.topic_id = 0x0000; 
    memcpy(&RegisterMssage.topic_name,topic_name,strlen(topic_name)); 
    
    MQTTSNClientSendData(mqttclient,(uint8_t*)&RegisterMssage,RegisterMssage.length);
    
    TimerAdd(TIMER_MODE_SINGLE,10000,mqtt_sn_send_register_cb,mqttclient ,TIMER_ID_OF_REGISTER);
      
}
/**
  * @brief   �����й���������
  * 
  * @param   client �ǿͻ��˿��ƿ��ַ
  * 
  * @param   topic_name ������
  *
  * @retval  None
  */
void mqtt_sn_send_willtopic(MQTT_SN_Client_t* mqttclient,const char* topic_name)
{
    uint8_t willtopic[30]={0x00,MQTT_SN_TYPE_WILLTOPIC,00};
    willtopic[0] = 3+strlen(topic_name);
    memcpy(&willtopic[3],topic_name,strlen(topic_name));
    MQTTSNClientSendData(mqttclient,willtopic,willtopic[0]);
      
}
/**
  * @brief   �����й���Ϣ���� 
  * 
  * @param   client �ǿͻ��˿��ƿ��ַ
  * 
  * @param   topic_name ��Ϣ��
  *
  * @retval  None
  */
void mqtt_sn_send_willMSG(MQTT_SN_Client_t* mqttclient,const char* topic_name)
{
    uint8_t willtopic[30]={0x00,MQTT_SN_TYPE_WILLMSG};
    willtopic[0] = 2+strlen(topic_name);
    memcpy(&willtopic[2],topic_name,strlen(topic_name));
    MQTTSNClientSendData(mqttclient,willtopic,willtopic[0]);
      
}
/**
  * @brief   ���ͶϿ���������
  * 
  * @param   client �ǿͻ��˿��ƿ��ַ
  * 
  * @param   duration �Ͽ�ʱ�䵥λs
  *
  * @retval  ��
  */
void mqtt_sn_send_disconnect(MQTT_SN_Client_t* mqttclient,uint16_t duration)
{
    disconnect_packet_t disconnet_msg;
    disconnet_msg.type = MQTT_SN_TYPE_DISCONNECT;
    disconnet_msg.duration = duration ;
    disconnet_msg.length = 4;
    MQTTSNClientSendData(mqttclient,(uint8_t*)&disconnet_msg,disconnet_msg.length);
    #ifdef  DEBUG_CELIENT
    printf("mqtt_sn_disconnect\r\n");
    #endif
    mqttclient->MqttSN_Status = MQTT_SN_DISCONNECTING;
}
/**
  * @brief   ping������Ϣ��ʱ���� 
  * 
  * @param   arg �ͻ��˿��ƿ��ַ
  *
  * @retval  0 ��ȡʧ�� ���������ĳ���
  */  
void mqtt_sn_client_pingreq_timeout_cb(void*arg)
{
     MQTT_SN_Client_t* client = arg;
     
     client->ping_req_res_lost_num ++;
     if(client->ping_req_res_lost_num<= MQTT_SN_CILENT_NADV)
     {
        mqtt_sn_send_pingreq(&mqtt_sn_client,WEAKUP);
     }else
     {
        /*���öϿ�*/
        mqtt_sn_send_disconnect(&mqtt_sn_client,0);
     }
}
/**
  * @brief   �ͻ��˷���ping���� 
  * 
  * @param   client �ǿͻ��˿��ƿ��ַ
  * 
  * @param   mode ������˯�ߺ��ѵĵ�һ��pingreq����������ping����
  *
  * @retval  0 ��ȡʧ�� ���������ĳ���
  */
void mqtt_sn_send_pingreq(MQTT_SN_Client_t* mqttclient,uint8_t mode)
{
    uint8_t pingreq[40]={0x00,MQTT_SN_TYPE_PINGREQ};
    
    if(mode == WEAKUP)
    {
       memcpy(&pingreq[2],mqttclient->clientID,strlen((const char*)mqttclient->clientID));
       pingreq[0] = 2 + strlen((const char*)mqttclient->clientID);
    }
    else
    {
        pingreq[0] = 2 ;
    }
    
    MQTTSNClientSendData(mqttclient,pingreq,pingreq[0]);
    TimerAdd(TIMER_MODE_SINGLE,10000,mqtt_sn_client_pingreq_timeout_cb,mqttclient,TIMER_ID_OF_PINGREQ);
      
}
/**
  * @brief   �ͻ��˷�����Ϣqos�ȼ���Ϊ0ʱ�ĳ�ʱ�ص����� 
  * 
  * @param   arg �ͻ��˷�����Ϣ�����ַ
  * 
  * @retval  None
  */
static void mqtt_sn_client_publish_timeout_cb(void *arg)
{
    MQTT_SN_pub_infor_t*  pub_infor = arg;
    printf("publish_timeout\r\n");
    mqtt_sn_send_publish(GetClientPCB(),pub_infor->topic_id,pub_infor->topic_type,\
    pub_infor->data,pub_infor->data_lenth,pub_infor->qos,pub_infor->retain,pub_infor->dup);
}
/**
  * @brief   ��ȡ�ͻ��˷�����Ϣ 
  * 
  * @param   mqttclient �ǿͻ��˿��ƿ��ַ
  * 
  * @param   topic_id ������Ϣ�ĳ�����Ķ�ID
  *
  * @param   topic_type ������Ϣ����������
  * 
  * @param   data �����������غ�
  *
  * @param   data_len ���������ݳ���
  * 
  * @param   qos ����qos�ȼ�
  *
  * @param   retain �Ƿ�����Ϣ
  * 
  * @retval  None
  */
void mqtt_sn_send_publish(MQTT_SN_Client_t* mqttclient, uint16_t topic_id, uint8_t topic_type, const void* data, \
                        uint16_t data_len, int8_t qos, uint8_t retain,uint8_t DUP)
{
    publish_packet_t publishmsg;
    
    
    memset(&publishmsg,0,sizeof(publish_packet_t));
    publishmsg.type  = MQTT_SN_TYPE_PUBLISH;
    publishmsg.topic_id = topic_id;
    memcpy(publishmsg.data,data,data_len);
    publishmsg.length = data_len + 7;
    if(qos==0){
        publishmsg.flags |=  MQTT_SN_FLAG_QOS_0;
    }
    else if(qos == 1){
       publishmsg.flags |= MQTT_SN_FLAG_QOS_1;
    }
    else if(qos == 2)
    {
       publishmsg.flags |= MQTT_SN_FLAG_QOS_2;
    }
    else{
       publishmsg.flags |= MQTT_SN_FLAG_QOS_N1;   
    }
    
    publishmsg.message_id = mqttclient->pkt_id++;
    mqttclient->input_pack_id.inpub_pkt_id[PUBLISH] = publishmsg.message_id;
    
    if(retain){
        publishmsg.flags |= MQTT_SN_FLAG_RETAIN;
    }
    if(DUP){
      publishmsg.flags|=MQTT_SN_FLAG_DUP;  
    }   
    publishmsg.flags|=topic_type;
    MQTTSNClientSendData(mqttclient,(uint8_t*)&publishmsg,publishmsg.length);
    if(qos == 1){
     publishinfor.data = data;
     publishinfor.data_lenth = data_len;
     publishinfor.qos = qos ;
     publishinfor.dup = 1 ;
     publishinfor.retain = retain;
     publishinfor.topic_id = topic_id;
     TimerAdd(TIMER_MODE_SINGLE,10000,mqtt_sn_client_publish_timeout_cb,&publishinfor,TIMER_ID_OF_PUBLISH);   
    }else if(qos == 2){
        
    }
    else if (qos == -1){
           
    }
}
/**
  * @brief   ͨ���������������ⳬʱ���� 
  * 
  * @param   arg ������Ϣ�Ļ���
  * 
  * @retval  None
  */
static void mqtt_sn_client_subscribe_timeout_cb(void *arg)
{
    MQTT_SN_sub_infor_t*  subscribeInfor = arg;
    printf("subscribe_timeout\r\n");
    mqtt_sn_send_subscribe_topic_name(GetClientPCB(),subscribeInfor->subscri.topic_name,subscribeInfor->qos,subscribeInfor->dup);
}
/**
  * @brief   ͨ������������һ������ 
  * 
  * @param   mqttclient �ǿͻ��˿��ƿ��ַ
  * 
  * @param   topic_topic_name ������Ϣ�ĳ�����
  * 
  * @param   qos ������Ϣ��qos�ȼ�
  * 
  * @retval  None
  */
void mqtt_sn_send_subscribe_topic_name(MQTT_SN_Client_t* mqttclient, const char* topic_name, int8_t qos, uint8_t dup)
{
    subscribe_packet_t submsg;
    
    memset(&submsg,0,sizeof(subscribe_packet_t));
    
    submsg.type  = MQTT_SN_TYPE_SUBSCRIBE;
    memcpy(submsg.topic_id_name.topic_name,topic_name,strlen(topic_name));
    submsg.message_id = mqttclient->pkt_id++;
    mqttclient->input_pack_id.inpub_pkt_id[SUBSCARIBE] = submsg.message_id;
    submsg.flags |= MQTT_SN_TOPIC_TYPE_NORMAL;
    submsg.length = strlen(topic_name) + 5;
    if(qos==0){
        submsg.flags |=  MQTT_SN_FLAG_QOS_0;
    }
    else if(qos == 1){
       submsg.flags |= MQTT_SN_FLAG_QOS_1;
    }
    else if(qos == 2)
    {
       submsg.flags |= MQTT_SN_FLAG_QOS_2;
    }
    else{
       submsg.flags |= MQTT_SN_FLAG_QOS_N1;   
    }
    if(dup){
      submsg.flags|=MQTT_SN_FLAG_DUP;  
    }   
    MQTTSNClientSendData(mqttclient,(uint8_t*)&submsg,submsg.length);
    
    if(qos == 1){
        memcpy(subscribeInfor.subscri.topic_name,topic_name,strlen(topic_name)+1);
        subscribeInfor.qos = qos ;
        subscribeInfor.dup = 1 ;
     TimerAdd(TIMER_MODE_SINGLE,10000,mqtt_sn_client_subscribe_timeout_cb,&subscribeInfor,TIMER_ID_OF_PUBLISH);   
    }else if(qos == 2){
        
    }
    else if (qos == -1){
           
    }
}
/**
  * @brief   ͨ������ID�������ⳬʱ���� 
  * 
  * @param   arg ������Ϣ�Ļ���
  * 
  * @retval  None
  */
static void mqtt_sn_client_subscribe_timeout_id_cb(void *arg)
{
    MQTT_SN_sub_infor_t*  subscribeInfor = arg;
    printf("subscribe_timeout\r\n");
    mqtt_sn_send_subscribe_topic_id(GetClientPCB(),subscribeInfor->subscri.topic_id,subscribeInfor->qos,subscribeInfor->dup);
}
/**
  * @brief   ͨ������ID����һ������ 
  * 
  * @param   mqttclient �ǿͻ��˿��ƿ��ַ
  * 
  * @param   topic_topic_name ������Ϣ�ĳ�����
  * 
  * @param   qos ����qos�ȼ�
  * 
  * @retval  None
  */
void mqtt_sn_send_subscribe_topic_id(MQTT_SN_Client_t* mqttclient, uint16_t topic_id, int8_t qos, uint8_t dup)
{
    subscribe_packet_t submsg;
    
    memset(&submsg,0,sizeof(subscribe_packet_t));
    
    submsg.type  = MQTT_SN_TYPE_SUBSCRIBE;
    submsg.topic_id_name.topic_id = topic_id;
    submsg.message_id = mqttclient->pkt_id++;
    mqttclient->input_pack_id.inpub_pkt_id[SUBSCARIBE] = submsg.message_id;
    submsg.flags|=MQTT_SN_TOPIC_TYPE_PREDEFINED;
    
    submsg.length =  7;
    if(qos==0){
        submsg.flags |=  MQTT_SN_FLAG_QOS_0;
    }
    else if(qos == 1){
       submsg.flags |= MQTT_SN_FLAG_QOS_1;
    }
    else if(qos == 2)
    {
       submsg.flags |= MQTT_SN_FLAG_QOS_2;
    }
    else{
       submsg.flags |= MQTT_SN_FLAG_QOS_N1;   
    }
    if(dup){
      submsg.flags|=MQTT_SN_FLAG_DUP;  
    }   
    
    
    MQTTSNClientSendData(mqttclient,(uint8_t*)&submsg,submsg.length);
    
    if(qos == 1){
        subscribeInfor.subscri.topic_id=topic_id;
        subscribeInfor.qos = qos ;
        subscribeInfor.dup = 1 ;
     TimerAdd(TIMER_MODE_SINGLE,10000,mqtt_sn_client_subscribe_timeout_id_cb,&subscribeInfor,TIMER_ID_OF_PUBLISH);   
    }else if(qos == 2){
        
    }
    else if (qos == -1){
           
    }
}
/**
  * @brief   ͨ���������˶�һ������ 
  * 
  * @param   mqttclient �ǿͻ��˿��ƿ��ַ
  * 
  * @param   topic_topic_name ������Ϣ�ĳ�����
  * 
  * @param   qos ����qos�ȼ�
  * 
  * @retval  None
  */
void mqtt_sn_send_unsubscribe_topic_name(MQTT_SN_Client_t* mqttclient, const char* topic_name)
{
    subscribe_packet_t unsubmsg;
    
    memset(&unsubmsg,0,sizeof(subscribe_packet_t));
    
    unsubmsg.type  = MQTT_SN_TYPE_UNSUBSCRIBE;
    memcpy(unsubmsg.topic_id_name.topic_name,topic_name,strlen(topic_name));
    unsubmsg.message_id = mqttclient->pkt_id++;
    mqttclient->input_pack_id.inpub_pkt_id[UNSUBSCARIBE] = unsubmsg.message_id;
    unsubmsg.flags |= MQTT_SN_TOPIC_TYPE_NORMAL;
    unsubmsg.length = strlen(topic_name) + 5;
    
    MQTTSNClientSendData(mqttclient,(uint8_t*)&unsubmsg,unsubmsg.length);
    
}
/**
  * @brief   ͨ������ID�˶�һ������ 
  * 
  * @param   mqttclient �ǿͻ��˿��ƿ��ַ
  * 
  * @param   topic_topic_name ������Ϣ�ĳ�����
  * 
  * @param   qos ����qos�ȼ�
  * 
  * @retval  None
  */
void mqtt_sn_send_unsubscribe_topic_id(MQTT_SN_Client_t* mqttclient, uint16_t topic_id)
{
    subscribe_packet_t unsubmsg;
    
    memset(&unsubmsg,0,sizeof(subscribe_packet_t));
    
    unsubmsg.type  = MQTT_SN_TYPE_UNSUBSCRIBE;
    unsubmsg.topic_id_name.topic_id = topic_id;
    unsubmsg.message_id = mqttclient->pkt_id++;
    mqttclient->input_pack_id.inpub_pkt_id[UNSUBSCARIBE] = unsubmsg.message_id;
    unsubmsg.flags|=MQTT_SN_TOPIC_TYPE_PREDEFINED;
    unsubmsg.length =  7;
    
    MQTTSNClientSendData(mqttclient,(uint8_t*)&unsubmsg,unsubmsg.length);
     
}
/**
  * @brief   �Ͽ������ص����� 
  * 
  * @param   mqttclient �ǿͻ��˿��ƿ��ַ
  * 
  * @param   duration �Ͽ�˯��ʱ��
  * 
  * @retval  None
  */
void mqtt_sn_send_disconnect_topic_id(MQTT_SN_Client_t* mqttclient, uint16_t duration)
{
    disconnect_packet_t disconnectbmsg;
    
    memset(&disconnectbmsg,0,sizeof(disconnect_packet_t));
    
    disconnectbmsg.type  = MQTT_SN_TYPE_DISCONNECT;
    disconnectbmsg.duration = duration;
    if(duration != 0){
    disconnectbmsg.length = 4;
    }else{
    disconnectbmsg.length = 2;
    }
    MQTTSNClientSendData(mqttclient,(uint8_t*)&disconnectbmsg,disconnectbmsg.length);
}

/**
  * @brief   ����ע���б�������Ƿ�ȫ��ע�����  
  *
  * @retval  0 δ��� ���������
  */
uint8_t registertopicfinsh(void)
{
    uint8_t i;
    topic_map_t* p_map = GetTopicMapAddr();
    for(i =0;i<MQTT_SN_CLIENT_LIST_MAX;i++)
    {
        if(p_map->toppiclist[i].status == unregister)
        {
            return 0;
        }
    }
    return 1;
}
/**
  * @brief   ѭ��ע�������б���δע������� 
  *
  * @retval  None
  */
void registeralltopic(void)
{
    uint8_t i;
    topic_map_t* p_map = GetTopicMapAddr();
    for(i =0;i<MQTT_SN_CLIENT_LIST_MAX;i++)
    {
        if(p_map->toppiclist[i].status == registering)
        {
            return ;
        }
    }
    for(i =0;i<MQTT_SN_CLIENT_LIST_MAX;i++)
    {
        if(p_map->toppiclist[i].status == unregister)
        {
            mqtt_sn_send_register(&mqtt_sn_client,topiclist[i]);
            TopicidRegistering(topiclist[i]);
            return ;
        }
    }
}
/**
  * @brief   mqttc�ͻ��˵���ѭ������
  * 
  * @param   client �ǿͻ��˿��ƿ��ַ
  *
  * @retval  None
  */
void mqtt_sn_client_cyclic(MQTT_SN_Client_t* mqttclient)
{
    mqttclient->cyclic_tick++;
    /*�����б�ά��*/
    gwListupdata(mqttclient);
    /*PING*/
    if(mqttclient->MqttSN_Status == MQTT_SN_CONNECTED)
    {
        registeralltopic();
        mqttclient->server_watchdog--;
        if(mqttclient->server_watchdog == 0)
        {
            mqtt_sn_send_pingreq(mqttclient,WEAKUP);
        } 
        
    }
    
}
/**
  * @brief   mqtt�ͻ��˵���ѭ�������Ļص���װ 
  * 
  * @param   arg δʹ��
  *
  * @retval  None
  */
void mqtt_sn_client_cyclic_cb(void*arg)
{
    mqtt_sn_client_cyclic(&mqtt_sn_client);
}
/**
  * @brief   �ͻ��˳�ʼ��
  * 
  * @param   None��ַ
  *
  * @retval  None
  */
void MQTTClientInit(void)
{
    uint8_t flag = 0;
    MQTT_SN_Client_t* MqttClient_pcb = GetClientPCB();
    memset(MqttClient_pcb,0,sizeof(MQTT_SN_Client_t));
    MqttClient_pcb->pkt_id = 1;
    MqttClient_pcb->gwList = &GWlist;
    
    mqtt_sn_create_socket(MqttClient_pcb);
    mqtt_sn_send_connect(MqttClient_pcb,"lwip_test",60,flag|MQTT_SN_FLAG_CLEAN);
    TimerAdd(TIMER_MODE_LOOP,1000,mqtt_sn_client_cyclic_cb,NULL,NULL);
}
