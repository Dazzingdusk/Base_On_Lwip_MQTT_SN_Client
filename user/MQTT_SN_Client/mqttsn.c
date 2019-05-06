#include "mqttsn.h"
#include "MY_BUFF.h"
#include "MQTTSNClient.h"
#include "MQTTSNClientTopicMap.h"

#if 1
MQTT_SN_GwList_t    GWlist;
MQTT_SN_GWInfor_t   gw; 
MQTT_SN_pub_infor_t publishinfor;

static void mqtt_sn_send_register_cb(void *arg);
static void mqtt_sn_client_publish_timeout_cb(void *arg);

/**
  * @brief  IPv4地址结构体格式化函数
  *   
  * @param  ip4_addr 必须是一个 ip4的结构体地址；
  *   
  * @param  abcd依次是IP地址的四个字节结果相当于a.b.c.d
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
/*******************************************************Callback API*********************************************************/
/**
  * @brief 处理网关信息报文回调函数，这里默认网关超时时间是15分钟
  * 
  * @param  arg 是网关的IP地址和端口信息的打包结构体的地址
  * 
  * @retval None
  */
void api_gwinfo_press(void *arg)
{
    uint8_t     freamlenth;
    uint16_t    keepalive = 900;
    uint8_t     inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    gwinfor_packet_t*   gwinformsg; 
    MQTT_SN_GWInfor_t*  GWinfor= arg;
    MQTT_SN_Client_t*   clientpcb = GetClientPCB();
    
    freamlenth = ReadClientImportindex(clientpcb);
    #ifdef  DEBUG_MQTTSN
    printf("api_gwinfo_press\r\n");
    #endif
    if(freamlenth <= MQTT_SN_MAX_PACKET_LENGTH)
    {
        if(ReadBUFF(inputdata,freamlenth) == ERROR_NONE)
        {
            gwinformsg = (gwinfor_packet_t*)inputdata;
            if(gwinformsg->length == freamlenth)
            {
                if(gwinformsg->type == MQTT_SN_TYPE_SEARCHGW )
                {
                    gwListappend(clientpcb->gwList,GWinfor->addr,GWinfor->port, gwinformsg->GWID,keepalive);
                }
            }
            #ifdef  DEBUG_MQTTSN
            else
            {
                while(1);
            }
            #endif
        }
    }

}
/**
  * @brief  处理网关广播信息报文回调函数 
  * 
  * @param  arg 是网关的IP地址和端口信息的打包结构体的地址
  * 
  * @retval None
  */
void api_advertise_press(void *arg)
{
    uint16_t keepalive;
    advertise_packet_t* adverrisemsg;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    MQTT_SN_GWInfor_t *GWinfor= arg;
    MQTT_SN_Client_t*   clientpcb = GetClientPCB();
    uint8_t freamlenth=ReadClientImportindex(clientpcb);
    
    #ifdef  DEBUG_MQTTSN
    printf("api_advertise_press\r\n");
    #endif
    if(freamlenth <= MQTT_SN_MAX_PACKET_LENGTH)
    {
        if(ReadBUFF(inputdata,freamlenth) == ERROR_NONE)
        {
            adverrisemsg =(advertise_packet_t*)inputdata; 
            if(adverrisemsg->length == freamlenth)
            {
                if(adverrisemsg->type == MQTT_SN_TYPE_ADVERTISE )
                {
                    keepalive = (adverrisemsg->duration[0]<<8)|adverrisemsg->duration[1];
                    gwListappend(clientpcb->gwList,GWinfor->addr,GWinfor->port, adverrisemsg->GWID,keepalive);
                    #ifdef DEBUG_GW_LIST
                    PrintfgwList(&GWlist);
                    #endif
                }
            }
            #ifdef  DEBUG_MQTTSN
            else
            {
                while(1);
            }
            #endif
        }
    }

}
/**
  * @brief  处理网关Connetack报文回调函数 
  * 
  * @param  arg 没用
  * 
  * @retval None
  */
void api_connack_press(void *arg)
{
    uint8_t deltimeIndex;
    connack_packet_t returnmsg;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    MQTT_SN_Client_t*   clientpcb=arg;
    uint8_t freamlenth=ReadClientImportindex(clientpcb);
    #ifdef  DEBUG_MQTTSN
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
                    &&clientpcb->MqttSN_Status == MQTT_SN_CONNECTING)
                {
                    clientpcb->MqttSN_Status = MQTT_SN_CONNECTED;
                    #ifdef  DEBUG_MQTTSN
                    printf("connect success.\r\n");
                    mqtt_sn_getgateway_info(clientpcb,10000);
                    #endif  
                }
            }
            #ifdef  DEBUG_MQTTSN
            else
            {
                //异常情况复位
                while(1);
            }
            #endif
        }
    }
}
/**
  * @brief  处理网关请求托管主题报文回调函数 
  * 
  * @param  arg 没用
  * 
  * @retval None
  */
void api_willtopicreq_press(void *arg)
{
    uint8_t reloadtimeIndex;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    MQTT_SN_Client_t*   clientpcb=arg;
    uint8_t freamlenth=ReadClientImportindex(clientpcb);
    #ifdef  DEBUG_MQTTSN
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
                if(inputdata[1] == MQTT_SN_TYPE_WILLTOPICREQ && clientpcb->MqttSN_Status == MQTT_SN_CONNECTING)
                {
                   /*回应willtopic*/
                    mqtt_sn_send_willtopic(clientpcb,"lwip_test");
                }
            }
            #ifdef  DEBUG_MQTTSN
            else
            {
                //异常情况复位
                while(1);
            }
            #endif
        }
    } 
}
/**
  * @brief  处理网关请求托管消息报文回调函数 
  * 
  * @param  arg 没用
  * 
  * @retval None
  */
void api_willmsgreq(void *arg)
{
    uint8_t reloadtimeIndex;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    MQTT_SN_Client_t*   clientpcb=arg;
    
    uint8_t freamlenth=ReadClientImportindex(clientpcb);
    #ifdef  DEBUG_MQTTSN
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
                if(inputdata[1] == MQTT_SN_TYPE_WILLMSGREQ && clientpcb->MqttSN_Status == MQTT_SN_CONNECTING)
                {
                   /*回应willtopic*/
                    mqtt_sn_send_willMSG(clientpcb,"lwip_test");
                }
            }
            #ifdef  DEBUG_MQTTSN
            else
            {
                //异常情况复位
                while(1);
            }
            #endif
        }
    } 
       
}
/**
  * @brief  处理网关回应ping请求的pingresponse消息报文回调函数 
  * 
  * @param  arg 没用
  * 
  * @retval None
  */
void api_pingres_press(void *arg)
{
    uint8_t deltimeIndex;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    MQTT_SN_Client_t*   clientpcb=arg;
    
    uint8_t freamlenth=ReadClientImportindex(clientpcb);
    #ifdef  DEBUG_MQTTSN
    printf("api_pingres_press\r\n");
    #endif
    if(freamlenth <= MQTT_SN_MAX_PACKET_LENGTH)
    {
        if(ReadBUFF(inputdata,freamlenth) == ERROR_NONE)
        {
            if(inputdata[0] == freamlenth)
            {
                if(inputdata[1] == MQTT_SN_TYPE_PINGRESP && clientpcb->MqttSN_Status == MQTT_SN_CONNECTED)
                {
                    /*处理超时*/
                    clientpcb->server_watchdog = clientpcb->keep_alive ;
                    clientpcb->ping_req_res_lost_num = 0;
                    /*删除超时定时器*/
                    deltimeIndex = FindTimerIndexbyID(TIMER_ID_OF_PINGREQ);
                    if (deltimeIndex != 0xFF)
                    {
                        TimerStop(deltimeIndex);
                        TimerDel(deltimeIndex);
                    }
                }
            }
            #ifdef  DEBUG_MQTTSN
            else
            {
                //异常情况复位
                while(1);
            }
            #endif
        }
    } 
}
/**
  * @brief  处理网关ping请求报文回调函数(隐患只要是连接状态，谁ping都会回) 
  * 
  * @param  arg 没用
  * 
  * @retval None
  */
void api_pingreq_press(void *arg)
{
  
    ping_respond_packet_t ping_respond_msg;
    ping_respond_packet_t *ping_request_msg;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    MQTT_SN_Client_t*   clientpcb=arg;
    
    uint8_t freamlenth=ReadClientImportindex(clientpcb);
    #ifdef  DEBUG_MQTTSN
    printf("api_pingreq_press\r\n");
    #endif
    if(freamlenth <= MQTT_SN_MAX_PACKET_LENGTH)
    {
        if(ReadBUFF(inputdata,freamlenth) == ERROR_NONE)
        {
            ping_request_msg = (ping_respond_packet_t*)inputdata;
            if(ping_request_msg->length == freamlenth)
            {
                if(ping_request_msg->type == MQTT_SN_TYPE_PINGREQ && clientpcb->MqttSN_Status == MQTT_SN_CONNECTED)
                {
                   ping_respond_msg.length = 2;
                   ping_respond_msg.type = MQTT_SN_TYPE_PINGRESP;
                   MQTTSNClientSendData(clientpcb,(uint8_t*)&ping_respond_msg,sizeof(ping_respond_packet_t));
                }
            }
            #ifdef  DEBUG_MQTTSN
            else
            {
                //异常情况复位
                while(1);
            }
            #endif
        }
    } 
}
/**
  * @brief  处理网关回应注册请求的regack消息报文回调函数 
  * 
  * @param  arg 没用
  * 
  * @retval None
  */
void api_regack_press(void *arg)
{
   
    regack_packet_t* regackmsg;
    uint8_t deltimeIndex;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    MQTT_SN_Client_t*   clientpcb=arg;
    
    uint8_t freamlenth=ReadClientImportindex(clientpcb);
    
    #ifdef  DEBUG_MQTTSN
    printf("api_regack_press\r\n");
    #endif
    if(freamlenth <= MQTT_SN_MAX_PACKET_LENGTH)
    {
        if(ReadBUFF(inputdata,freamlenth) == ERROR_NONE)
        {
            regackmsg = (regack_packet_t*)inputdata;
            if(regackmsg->length == freamlenth)
            {
                if(regackmsg->type == MQTT_SN_TYPE_REGACK && clientpcb->MqttSN_Status == MQTT_SN_CONNECTED\
                    &&regackmsg->message_id == clientpcb->inpub_pkt_id)
                {
                   /*删除超时定时器*/
                    deltimeIndex = FindTimerIndexbyID(TIMER_ID_OF_REGISTER); 
                    if (deltimeIndex != 0xFF)
                    {
                        TimerStop(deltimeIndex);
                        TimerDel(deltimeIndex);
                    } 
                    if(regackmsg->return_code == MQTT_SN_ACCEPTED){
                    /*注册确认,将注册ID加入注册map*/
                    TopicidRegistered(regackmsg->topic_id);
                    }
                    else if(regackmsg->return_code == MQTT_SN_REJECTED_CONGESTION){
                    TimerAdd(TIMER_MODE_SINGLE,5*60*1000,mqtt_sn_send_register_cb,clientpcb ,TIMER_ID_OF_REGISTER);    
                    /*注册拥塞*/
                    }
                    else{
                     TimerAdd(TIMER_MODE_SINGLE,60*1000,mqtt_sn_send_register_cb,clientpcb ,TIMER_ID_OF_REGISTER);   
                    }
                }
            }
            #ifdef  DEBUG_MQTTSN
            else
            {
                while(1);
            }
            #endif
        }
    }

}
/**
  * @brief  处理网关回应发布请求的puback消息报文回调函数 
  * 
  * @param  arg 没用
  * 
  * @retval None
  */
void api_puback_press(void *arg)
{
    puback_packet_t* pubackmsg;
    uint8_t deltimeIndex;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    MQTT_SN_Client_t*   clientpcb=arg;
    uint8_t freamlenth=ReadClientImportindex(clientpcb);
    
    #ifdef  DEBUG_MQTTSN
    printf("api_puback_press\r\n");
    #endif
    if(freamlenth <= MQTT_SN_MAX_PACKET_LENGTH)
    {
        if(ReadBUFF(inputdata,freamlenth) == ERROR_NONE)
        {
            pubackmsg = (puback_packet_t*)inputdata;
            if(pubackmsg->length == freamlenth)
            {
                if(pubackmsg->type == MQTT_SN_TYPE_PUBACK && clientpcb->MqttSN_Status == MQTT_SN_CONNECTED\
                    &&pubackmsg->message_id == clientpcb->inpub_pkt_id)
                {
                   /*删除超时定时器*/
                    deltimeIndex = FindTimerIndexbyID(TIMER_ID_OF_PUBLISH); 
                    if (deltimeIndex != 0xFF)
                    {
                        TimerStop(deltimeIndex);
                        TimerDel(deltimeIndex);
                    } 
                    if(pubackmsg->return_code == MQTT_SN_ACCEPTED){
                    /*注册确认,将注册ID加入注册map*/
                    #ifdef  DEBUG_MQTTSN
                    printf("Publish Sucessed\r\n");
                    #endif
                    }
                    else if(pubackmsg->return_code == MQTT_SN_REJECTED_CONGESTION){
                    TimerAdd(TIMER_MODE_SINGLE,5*60*1000, mqtt_sn_client_publish_timeout_cb,clientpcb ,TIMER_ID_OF_PUBLISH);    
                    /*拥塞*/
                    }
                    else if(pubackmsg->return_code == MQTT_SN_REJECTED_INVALID){  
                    /*不支持的ID，重新注册*/
                    TopicidUnregister(pubackmsg->topic_id);
                    }
                    else{
                        #ifdef  DEBUG_MQTTSN
                        printf("Publish Unsupport\r\n");
                        #endif
                    }
                }
            }
            #ifdef  DEBUG_MQTTSN
            else
            {
                while(1);
            }
            #endif
        }
    }
}
/**
  * @brief  处理网关回应ping请求的pingresponse消息报文回调函数 
  * 
  * @param  arg 没用
  * 
  * @retval None
  */
void api_pubcomp_press(void *arg)
{
    #ifdef  DEBUG_MQTTSN
    printf("api_pubcomp_press\r\n");
    #endif  
}
/**
  * @brief  处理网关回应ping请求的pingresponse消息报文回调函数 
  * 
  * @param  arg 没用
  * 
  * @retval None
  */
void api_pubrec_press(void *arg)
{
    #ifdef  DEBUG_MQTTSN
    printf("api_pubrec_press\r\n");
    #endif
}
/**
  * @brief  处理网关回应ping请求的pingresponse消息报文回调函数 
  * 
  * @param  arg 没用
  * 
  * @retval None
  */
void api_pubrel_press(void *arg)
{
    #ifdef  DEBUG_MQTTSN
    printf("api_pubrel_press\r\n");
    #endif
}
/**
  * @brief  处理网关回应订阅请求的suback消息报文回调函数 
  * 
  * @param  arg 没用
  * 
  * @retval None
  */
void api_suback_press(void *arg)
{
    disconnect_packet_t* disconnetmsg;
    uint8_t deltimeIndex;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    MQTT_SN_Client_t*   clientpcb=arg;
    uint8_t freamlenth=ReadClientImportindex(clientpcb);
    
    #ifdef  DEBUG_MQTTSN
    printf("api_suback_press\r\n");
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
                   /*删除超时定时器*/
                    deltimeIndex = FindTimerIndexbyID(TIMER_ID_OF_DISCONNCET); 
                    if (deltimeIndex != 0xFF)
                    {
                        TimerStop(deltimeIndex);
                        TimerDel(deltimeIndex);
                    }
                    clientpcb->MqttSN_Status = MQTT_SN_NOTCONNECT;
                }
            }
            #ifdef  DEBUG_MQTTSN
            else
            {
                while(1);
            }
            #endif
        }
    }

}
/**
  * @brief  处理网关回应取消订阅请求的unsub消息报文回调函数 
  * 
  * @param  arg 没用
  * 
  * @retval None
  */
void api_unsuback_press(void *arg)
{
    #ifdef  DEBUG_MQTTSN
    printf("api_unsuback_press\r\n");
    #endif
}
/**
  * @brief  处理网关回应断开连接请求的disconnect消息报文回调函数 
  * 
  * @param  arg 没用
  * 
  * @retval None
  */
void api_disconnet_press(void *arg)
{
    disconnect_packet_t* disconnetmsg;
    uint8_t deltimeIndex;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    MQTT_SN_Client_t*   clientpcb=arg;
    uint8_t freamlenth=ReadClientImportindex(clientpcb);
    
    #ifdef  DEBUG_MQTTSN
    printf("api_puback_press\r\n");
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
                   /*删除超时定时器*/
                    deltimeIndex = FindTimerIndexbyID(TIMER_ID_OF_DISCONNCET); 
                    if (deltimeIndex != 0xFF)
                    {
                        TimerStop(deltimeIndex);
                        TimerDel(deltimeIndex);
                    }
                    clientpcb->MqttSN_Status = MQTT_SN_NOTCONNECT;
                }
            }
            #ifdef  DEBUG_MQTTSN
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
    
};

/**
  * @brief  由命令码返回处理回调函数 
  * 
  * @param  cmd MQTT_SN命令
  * 
  * @retval 回调函数
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
}/**
  * @brief  判断命令码类型创建回调函数 
  * 
  * @param  cmd MQTT_SN命令
  * 
  * @retval 1 传入网关信息 0 传入客户端控制块
  */
uint8_t getcmdtype(uint8_t cmd)
{
    uint8_t n=0;
    if( MQTT_SN_TYPE_GWINFO == cmd ||MQTT_SN_TYPE_ADVERTISE== cmd){
            return 1;
    }
    return 0;
}
/******************************************************************************************************************************/
/**
  * @brief   收到UDP报文的回调函数，主要是将报文写入buff并启动一个回调去处理
  * 
  * @param   mqttclient 是客户端控制块地址
  * 
  * @param   addr 远端IP地址
  *
  * @param   port 远端端口
  * 
  * @retval  None
  */
static void mqtt_sn_socket_incomm_cb(MQTT_SN_Client_t* mqttclient, const struct ip4_addr *addr, uint16_t port,uint8_t cmd)
{
  
    FUN_TIMER_CALLBACK_T cb;
    uint8_t inputdata[MQTT_SN_MAX_PACKET_LENGTH];
    
    /*此处处理用于网关信息*/
    gw.addr = *addr;
    gw.port = port;
    /*ping response*/
    
    cb = getAPIFUN(cmd);
    if(cb !=NULL){
        if(getcmdtype(cmd)){
        TimerAdd(TIMER_MODE_SINGLE,100,cb,(void*)&gw,NULL);
        }else{
        TimerAdd(TIMER_MODE_SINGLE,100,cb,mqttclient,NULL);
        }
     }else{
        ReadBUFF(inputdata,ReadClientImportindex(mqttclient));
     }
}
/**
  * @brief   判断收到UDP报文是否是MQTT-SN有用命令报文
  * 
  * @param   cmd 命令
  *
  * @retval  0 是mqtt命令报文  1 不是
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
  * @brief   收到UDP报文的回调函数，此函数由LWIP协议栈回调
  * 
  * @param   arg 参数
  * 
  * @param   pcb ucp控制块
  *
  * @param   p 数据包地址
  * 
  * @param   addr 远端地址
  *
  * @param   port 远端端口
  *
  * @retval  None
  */
void UdpServer_recv_cb(void *arg, struct udp_pcb *pcb, struct pbuf *p, const struct ip4_addr *addr, uint16_t port)
{
    uint8_t cmd;
    MQTT_SN_Client_t* mqttclientpcb = arg;
    uint8_t *pdata = p->payload;
    
    pcb->remote_port = port;
    pcb->remote_ip = *addr;
    if(*pdata == 0x01){
        cmd = *(pdata+3);
    }else{
        cmd = *(pdata+1);
    }
    if (CmdConfirm(cmd)){
        /*丢弃数据包*/
        pbuf_free(p);
        return ;
    }
    //保证写入成功
    if (getSpaceofpktinput()>0 && BuffSpaceget() > (p->tot_len))
    {
        //广播数据
        if(WriteClientImportindex(mqttclientpcb,p->tot_len)==0)
        {
            if (WriteBUFF(p->payload,p->tot_len) == 0)
                mqtt_sn_socket_incomm_cb(mqttclientpcb,addr,port,cmd);
        } 
    }
    /*释放该udp段*/
    pbuf_free(p);
      
}
/**
  * @brief   创建一个UDP sockt 并绑定到客户端地址上 
  *          初始化消息buff
  * @param   mqttclient 是客户端控制块地址
  * 
  * @retval  None
  */
void mqtt_sn_create_socket(struct udp_pcb* pcb)
{
	pcb = udp_new();/* 建立通信的UDP控制块(pcb) */
	udp_bind(pcb, IP_ADDR_ANY, MQTT_SN_DEFAULT_PORT);/* 绑定本地IP地址和端口号（作为udp服务器） */
	udp_recv(pcb, UdpServer_recv_cb, GetClientPCB());/* 设置UDP段到时的回调函数 */
}

/**
  * @brief   读取客户端待处理消息队列FIFO，得到要处理的报文长度，用于读取buff 
  * 
  * @param   client 是客户端控制块地址
  * 
  * @param   data 新写入buff的报文长度信息
  *
  * @retval  0 读取失败 其他：报文长度
  */
void mqtt_sn_getgateway_info(struct udp_pcb* pcb,const uint16_t remote_port)
{
    struct ip4_addr destAddr;
    uint8_t searchinfo[3] = {0x02,0x01,0x00};
	My_IP4_ADDR(&destAddr,255,255,255,255);
    SockSendDataStream(pcb,&destAddr,remote_port,searchinfo,3);
}
/**
  * @brief   读取客户端待处理消息队列FIFO，得到要处理的报文长度，用于读取buff 
  * 
  * @param   client 是客户端控制块地址
  * 
  * @param   data 新写入buff的报文长度信息
  *
  * @retval  0 读取失败 其他：报文长度
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
  * @brief   客户端连接网关 
  * 
  * @param   client 是客户端控制块地址
  * 
  * @param   data 新写入buff的报文长度信息
  *
  * @retval  0 读取失败 其他：报文长度
  */
void mqtt_sn_send_connect(struct udp_pcb* pcb,struct ip4_addr* svrip,uint16_t dstport ,const char * client_id, uint16_t keepalive,uint8_t flags)
{
    connect_packet_t  ConnetMssage;
    memset(&ConnetMssage,0,sizeof(connect_packet_t));
    ConnetMssage.flags = 0;
    memcpy(ConnetMssage.client_id,client_id,strlen(client_id));
    
    
    ConnetMssage.length = strlen(client_id)+MQTT_SN_CONNET_FIXLENTH;
//    ConnetMssage.duration[0] = 0xFF&(keepalive>>8);
//    ConnetMssage.duration[1] = 0xFF&keepalive;
    ConnetMssage.duration = keepalive;     
    ConnetMssage.protocol_id = MQTT_SN_PROTOCOL_ID;
    ConnetMssage.type = MQTT_SN_TYPE_CONNECT;
    ConnetMssage.flags |=  flags;
    SockSendDataStream(pcb,svrip,dstport,(uint8_t*)&ConnetMssage,ConnetMssage.length); 
}

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
  * @brief   客户端注册主题ID操作 
  * 
  * @param   mqttclient 是客户端控制块地址
  * 
  * @param   topic_name 注册的主题名
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
    mqttclient->inpub_pkt_id = RegisterMssage.message_id;
    RegisterMssage.topic_id = 0x0000; 
    memcpy(&RegisterMssage.topic_name,topic_name,strlen(topic_name)); 
    
    MQTTSNClientSendData(mqttclient,(uint8_t*)&RegisterMssage,RegisterMssage.length);
    
    TimerAdd(TIMER_MODE_SINGLE,4000,mqtt_sn_send_register_cb,mqttclient ,TIMER_ID_OF_REGISTER);
      
}
/**
  * @brief   读取客户端待处理消息队列FIFO，得到要处理的报文长度，用于读取buff 
  * 
  * @param   client 是客户端控制块地址
  * 
  * @param   data 新写入buff的报文长度信息
  *
  * @retval  0 读取失败 其他：报文长度
  */
void mqtt_sn_send_willtopic(MQTT_SN_Client_t* mqttclient,const char* topic_name)
{
    uint8_t willtopic[30]={0x00,MQTT_SN_TYPE_WILLTOPIC,00};
    willtopic[0] = 3+strlen(topic_name);
    memcpy(&willtopic[3],topic_name,strlen(topic_name));
    MQTTSNClientSendData(mqttclient,willtopic,willtopic[0]);
      
}
/**
  * @brief   读取客户端待处理消息队列FIFO，得到要处理的报文长度，用于读取buff 
  * 
  * @param   client 是客户端控制块地址
  * 
  * @param   data 新写入buff的报文长度信息
  *
  * @retval  0 读取失败 其他：报文长度
  */
void mqtt_sn_send_willMSG(MQTT_SN_Client_t* mqttclient,const char* topic_name)
{
    uint8_t willtopic[30]={0x00,MQTT_SN_TYPE_WILLMSG};
    willtopic[0] = 2+strlen(topic_name);
    memcpy(&willtopic[2],topic_name,strlen(topic_name));
    MQTTSNClientSendData(mqttclient,willtopic,willtopic[0]);
      
}
/**
  * @brief   读取客户端待处理消息队列FIFO，得到要处理的报文长度，用于读取buff 
  * 
  * @param   client 是客户端控制块地址
  * 
  * @param   data 新写入buff的报文长度信息
  *
  * @retval  0 读取失败 其他：报文长度
  */
void mqtt_sn_send_disconnect(MQTT_SN_Client_t* mqttclient,uint16_t duration)
{
    disconnect_packet_t disconnet_msg;
    disconnet_msg.type = MQTT_SN_TYPE_DISCONNECT;
    disconnet_msg.duration = duration ;
    disconnet_msg.length = 4;
    MQTTSNClientSendData(mqttclient,(uint8_t*)&disconnet_msg,disconnet_msg.length);
    #ifdef  DEBUG_MQTTSN
    printf("mqtt_sn_disconnect\r\n");
    #endif
    mqttclient->MqttSN_Status = MQTT_SN_DISCONNECTING;
}
/**
  * @brief   读取客户端待处理消息队列FIFO，得到要处理的报文长度，用于读取buff 
  * 
  * @param   client 是客户端控制块地址
  * 
  * @param   data 新写入buff的报文长度信息
  *
  * @retval  0 读取失败 其他：报文长度
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
        /*永久断开*/
        mqtt_sn_send_disconnect(&mqtt_sn_client,0);
     }
}
/**
  * @brief   读取客户端待处理消息队列FIFO，得到要处理的报文长度，用于读取buff 
  * 
  * @param   client 是客户端控制块地址
  * 
  * @param   data 新写入buff的报文长度信息
  *
  * @retval  0 读取失败 其他：报文长度
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

static void mqtt_sn_client_publish_timeout_cb(void *arg)
{
    MQTT_SN_pub_infor_t*  punlishmsg = arg;
    mqtt_sn_send_publish(GetClientPCB(),punlishmsg->topic_id,punlishmsg->topic_type,\
    punlishmsg->data,punlishmsg->data_lenth,punlishmsg->qos,punlishmsg->retain);
}
/**
  * @brief   读取客户端发布消息 
  * 
  * @param   mqttclient 是客户端控制块地址
  * 
  * @param   topic_id 发布消息的长主题的短ID
  *
  * @param   topic_type 发布消息的主题类型
  * 
  * @param   data 发布的数据载荷
  *
  * @param   data_len 发布的数据长度
  * 
  * @param   qos 发布qos等级
  *
  * @param   retain 是否保留消息
  * 
  * @retval  None
  */
void mqtt_sn_send_publish(MQTT_SN_Client_t* mqttclient, uint16_t topic_id, uint8_t topic_type, const void* data, \
                        uint16_t data_len, int8_t qos, uint8_t retain)
{
    publish_packet_t publishmsg;
    memset(&publishmsg,0,sizeof(publish_packet_t));
    if(qos==0){
        publishmsg.flags |=  MQTT_SN_FLAG_QOS_0;
        publishmsg.message_id = 0x0000;
    }else{
        if(qos == 1){
            publishmsg.flags |= MQTT_SN_FLAG_QOS_1;
        }else if(qos == 2){
            publishmsg.flags |= MQTT_SN_FLAG_QOS_2;
        }
        else{
            publishmsg.flags |= MQTT_SN_FLAG_QOS_N1;   
        }
        publishmsg.message_id = mqttclient->pkt_id++;
        mqttclient->inpub_pkt_id = publishmsg.message_id;
    }
    if(retain){
        publishmsg.flags |= MQTT_SN_FLAG_RETAIN;
    }
    publishmsg.flags|=topic_type;
    
    publishmsg.type  = MQTT_SN_TYPE_PUBLISH;
    publishmsg.topic_id = topic_id;
    memcpy(publishmsg.data,data,data_len);
    publishmsg.length = data_len + 7;
    
    MQTTSNClientSendData(mqttclient,(uint8_t*)&publishmsg,publishmsg.length);
    if(qos == 1){
     publishinfor.data = data;
     publishinfor.data_lenth = data_len;
     publishinfor.qos = qos|MQTT_SN_FLAG_DUP;
     publishinfor.retain = retain;
     publishinfor.topic_id = topic_id;
     TimerAdd(TIMER_MODE_SINGLE,60000,mqtt_sn_client_publish_timeout_cb,&publishinfor,TIMER_ID_OF_PUBLISH);   
    }else if(qos == 2){
        
    }
    else if (qos == -1){
           
    }
}
/**
  * @brief   通过主题名订阅一个主题 
  * 
  * @param   mqttclient 是客户端控制块地址
  * 
  * @param   topic_topic_name 订阅消息的长主题
  * 
  * @param   qos 发布qos等级
  * 
  * @retval  None
  */
void mqtt_sn_send_subscribe_topic_name(MQTT_SN_Client_t* mqttclient, const char* topic_name, uint8_t qos)
{
}
/**
  * @brief   通过主题ID订阅一个主题 
  * 
  * @param   mqttclient 是客户端控制块地址
  * 
  * @param   topic_topic_name 订阅消息的长主题
  * 
  * @param   qos 发布qos等级
  * 
  * @retval  None
  */
void mqtt_sn_send_subscribe_topic_id(MQTT_SN_Client_t* mqttclient, uint16_t topic_id, uint8_t qos)
{
}
/**
  * @brief   通过主题名退订一个主题 
  * 
  * @param   mqttclient 是客户端控制块地址
  * 
  * @param   topic_topic_name 订阅消息的长主题
  * 
  * @param   qos 发布qos等级
  * 
  * @retval  None
  */
void mqtt_sn_send_unsubscribe_topic_name(MQTT_SN_Client_t* mqttclient, const char* topic_name, uint8_t qos)
{

}
/**
  * @brief   通过主题ID退订一个主题 
  * 
  * @param   mqttclient 是客户端控制块地址
  * 
  * @param   topic_topic_name 订阅消息的长主题
  * 
  * @param   qos 发布qos等级
  * 
  * @retval  None
  */
void mqtt_sn_send_unsubscribe_topic_id(MQTT_SN_Client_t* mqttclient, uint16_t topic_id, uint8_t qos)
{

}

#endif
