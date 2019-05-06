#ifndef MQTTSNCLIENT_H
#define MQTTSNCLIENT_H

#include <stdint.h>
#include "udp.h"
#include "MQTTSNTimes.h"

#ifndef FALSE
#define FALSE  (0)
#endif

#ifndef TRUE
#define TRUE  (1)
#endif

#define MQTT_SN_DEFAULT_PORT                (1883)
#define MQTT_SN_DEFAULT_TIMEOUT             (10)
#define MQTT_SN_DEFAULT_KEEP_ALIVE          (60)

#define MQTT_SN_MAX_PACKET_LENGTH           (255)
#define MQTT_SN_MAX_PAYLOAD_LENGTH          (MQTT_SN_MAX_PACKET_LENGTH-7)
#define MQTT_SN_MAX_TOPIC_LENGTH            (MQTT_SN_MAX_PACKET_LENGTH-6)
#define MQTT_SN_MAX_CLIENT_ID_LENGTH        (23)
#define MQTT_SN_MAX_WIRELESS_NODE_ID_LENGTH (252)

#define MQTT_SN_TYPE_ADVERTISE              (0x00)
#define MQTT_SN_TYPE_SEARCHGW               (0x01)
#define MQTT_SN_TYPE_GWINFO                 (0x02)
#define MQTT_SN_TYPE_CONNECT                (0x04)
#define MQTT_SN_TYPE_CONNACK                (0x05)
#define MQTT_SN_TYPE_WILLTOPICREQ           (0x06)
#define MQTT_SN_TYPE_WILLTOPIC              (0x07)
#define MQTT_SN_TYPE_WILLMSGREQ             (0x08)
#define MQTT_SN_TYPE_WILLMSG                (0x09)
#define MQTT_SN_TYPE_REGISTER               (0x0A)
#define MQTT_SN_TYPE_REGACK                 (0x0B)
#define MQTT_SN_TYPE_PUBLISH                (0x0C)
#define MQTT_SN_TYPE_PUBACK                 (0x0D)
#define MQTT_SN_TYPE_PUBCOMP                (0x0E)
#define MQTT_SN_TYPE_PUBREC                 (0x0F)
#define MQTT_SN_TYPE_PUBREL                 (0x10)
#define MQTT_SN_TYPE_SUBSCRIBE              (0x12)
#define MQTT_SN_TYPE_SUBACK                 (0x13)
#define MQTT_SN_TYPE_UNSUBSCRIBE            (0x14)
#define MQTT_SN_TYPE_UNSUBACK               (0x15)
#define MQTT_SN_TYPE_PINGREQ                (0x16)
#define MQTT_SN_TYPE_PINGRESP               (0x17)
#define MQTT_SN_TYPE_DISCONNECT             (0x18)
#define MQTT_SN_TYPE_WILLTOPICUPD           (0x1A)
#define MQTT_SN_TYPE_WILLTOPICRESP          (0x1B)
#define MQTT_SN_TYPE_WILLMSGUPD             (0x1C)
#define MQTT_SN_TYPE_WILLMSGRESP            (0x1D)
#define MQTT_SN_TYPE_FRWDENCAP              (0xFE)

#define MQTT_SN_ACCEPTED                    (0x00)
#define MQTT_SN_REJECTED_CONGESTION         (0x01)
#define MQTT_SN_REJECTED_INVALID            (0x02)
#define MQTT_SN_REJECTED_NOT_SUPPORTED      (0x03)

#define MQTT_SN_TOPIC_TYPE_NORMAL           (0x00)
#define MQTT_SN_TOPIC_TYPE_PREDEFINED       (0x01)
#define MQTT_SN_TOPIC_TYPE_SHORT            (0x02)


#define MQTT_SN_FLAG_DUP                    (0x1 << 7)
#define MQTT_SN_FLAG_QOS_0                  (0x0 << 5)
#define MQTT_SN_FLAG_QOS_1                  (0x1 << 5)
#define MQTT_SN_FLAG_QOS_2                  (0x2 << 5)
#define MQTT_SN_FLAG_QOS_N1                 (0x3 << 5)
#define MQTT_SN_FLAG_QOS_MASK               (0x3 << 5)
#define MQTT_SN_FLAG_RETAIN                 (0x1 << 4)
#define MQTT_SN_FLAG_WILL                   (0x1 << 3)
#define MQTT_SN_FLAG_CLEAN                  (0x1 << 2)

#define MQTT_SN_PROTOCOL_ID                 (0x01)

#define MQTT_SN_CONNET_FIXLENTH             (6)
#define MQTT_SN_REGISTER_FIXLENTH           (6)

#define MQTT_SN_WD_ADDR_LEHTH               (6)



#define MQTT_SN_CILENT_TADV                 (900)
#define MQTT_SN_CILENT_NADV                 (3)
#define MQTT_GW_LISTNUM_MAX                 (3)
#define MQTT_SN_IMPORT_MAX                  (10)

#define INITBUFFED							(1)
#define API_NUM                             (16)
#define WEAKUP                              (1)


#define  GetAvalidofpktinput(w_ptr,r_ptr,ring)  ((ring)?(MQTT_SN_IMPORT_MAX):(w_ptr==r_ptr)?(0):((w_ptr>r_ptr)?(w_ptr-r_ptr):(MQTT_SN_IMPORT_MAX-r_ptr+w_ptr)))
#define  GetSpaceofpktinput(w_ptr,r_ptr,ring)   (MQTT_SN_IMPORT_MAX - (GetBuff_Valid_DataLenth(w_ptr,r_ptr,ring)))


typedef enum  { QOS0, QOS1, QOS2,QOSN1 }QoS;
typedef enum  {MQTT_SN_NOTCONNECT, MQTT_SN_CONNECTING,MQTT_SN_CONNECTED,MQTT_SN_DISCONNECTING,MQTT_SN_DISCONNECTED}MQTTSNSTATUS_T;

//禁止编译器自动对齐https://blog.csdn.net/21aspnet/article/details/6729724
//http://www.openedv.com/forum.php?mod=viewthread&tid=268646&page=1&extra=
#pragma pack(1)
typedef struct  
{
    uint8_t length;
    uint8_t type;
    uint8_t flags;
    uint8_t protocol_id;
    uint16_t duration;
    char client_id[MQTT_SN_MAX_CLIENT_ID_LENGTH];
} connect_packet_t;

typedef struct  
{
    uint8_t length;
    uint8_t type;
    uint8_t GWID;
    uint8_t gwaddr[MQTT_SN_MAX_CLIENT_ID_LENGTH];
} gwinfor_packet_t;

typedef struct 
{
    uint8_t length;
    uint8_t type;
    uint8_t GWID;
    uint8_t duration[2];
} advertise_packet_t;

typedef struct   
{
    uint8_t length;
    uint8_t type;
    uint8_t return_code;
} connack_packet_t;

typedef struct  
{
    uint8_t  length;
    uint8_t  type;
    uint16_t topic_id;
    uint16_t message_id;
    char topic_name[MQTT_SN_MAX_TOPIC_LENGTH];
} register_packet_t;

typedef struct   
{
    uint8_t length;
    uint8_t type;
    uint16_t topic_id;
    uint16_t message_id;
    uint8_t return_code;
} regack_packet_t;


typedef struct  __attribute__((packed))
{
    uint8_t length;
    uint8_t type;
    uint8_t flags;
    uint16_t topic_id;
    uint16_t message_id;
    char data[MQTT_SN_MAX_PAYLOAD_LENGTH];
}publish_packet_t;

typedef struct  
{
    uint8_t length;
    uint8_t type;
    uint16_t topic_id;
    uint16_t message_id;
    uint8_t return_code;
}puback_packet_t;

typedef union 
{
    uint16_t topic_id;
    char topic_name[MQTT_SN_MAX_TOPIC_LENGTH];
}subscribe_t;

typedef struct   
{
    uint8_t length;
    uint8_t type;
    uint8_t flags;
    uint16_t message_id;
    subscribe_t topic_id_name;

}subscribe_packet_t;

typedef struct  
{
    uint8_t length;
    uint8_t type;
    uint8_t flags;
    uint16_t topic_id;
    uint16_t message_id;
    uint8_t return_code;
}suback_packet_t;

typedef struct  
{
    uint8_t length;
    uint8_t type;
    uint16_t message_id;
}unsuback_packet_t;

typedef struct  
{
    uint8_t length;
    uint8_t type;
    uint16_t duration;
} disconnect_packet_t;


typedef struct  
{
    uint8_t length;
    uint8_t type;
} ping_respond_packet_t;


typedef struct 
{
    uint8_t         gw_ID;
    uint8_t         gw_Nadv;
    uint16_t        gw_port;
    uint16_t        gw_keepalive;
    struct ip4_addr gw_addr;
    uint8_t         gw_usedflag;
    uint32_t        gw_lasttime;
}MQTT_SN_GwItem_t;
#pragma pack()
typedef struct 
{
    MQTT_SN_GwItem_t gwList[MQTT_GW_LISTNUM_MAX];
    uint8_t          gwListAvalidNum;
}MQTT_SN_GwList_t;
/*消息长度FIFO*/
typedef struct  
{
    uint8_t 	r_ptr;
    uint8_t 	w_ptr;
	uint8_t  	ring;
    uint8_t 	buff[MQTT_SN_IMPORT_MAX];

}Import_buff_t;

typedef enum 
{
    REGGISTER=0,
    PUBLISH,
    SUBSCARIBE,
    UNSUBSCARIBE,
    MAX_PENDING_NUM
}PACK_PENDING_LIST;

typedef struct
{
    uint16_t   inpub_pkt_id[MAX_PENDING_NUM];
}inpub_pkt_id_list;

typedef struct  
{
    uint32_t            cyclic_tick;
    uint16_t            keep_alive;
    uint16_t            server_watchdog;
    uint8_t             ping_req_res_lost_num;
    uint8_t             clientID[MQTT_SN_MAX_CLIENT_ID_LENGTH];
    /*packet id generator*/
    uint16_t            pkt_id;
    /*pending packet id*/
    inpub_pkt_id_list   input_pack_id;
    MQTTSNSTATUS_T      MqttSN_Status;
    MQTT_SN_GwList_t    *gwList;
    struct udp_pcb*     Sock;
    /** Input */
    Import_buff_t       rx_buffer;
    
}MQTT_SN_Client_t;


typedef struct  
{
    uint8_t 	cmd_id;
    FUN_TIMER_CALLBACK_T api_func;
} API_TAB_T;

typedef struct 
{
	struct ip4_addr addr;
	uint16_t port;
} MQTT_SN_GWInfor_t;

typedef struct  
{
	const void  *data;
	uint16_t    data_lenth;
    uint16_t    topic_id; 
    uint8_t     topic_type; 
    int8_t      qos;
    int8_t      dup;
    uint8_t     retain;
    
} MQTT_SN_pub_infor_t;
typedef struct  
{
	subscribe_t subscri; 
    int8_t      qos;
    int8_t      dup;
    
} MQTT_SN_sub_infor_t;

void MQTTClientInit(void);



MQTT_SN_Client_t* GetClientPCB(void);
uint8_t registertopicfinsh(void);
uint8_t getSpaceofpktinput(void);

uint8_t ReadClientImportindex(MQTT_SN_Client_t* client);
uint8_t WriteClientImportindex(MQTT_SN_Client_t* client,uint8_t data );
uint8_t gwListappend(MQTT_SN_GwList_t *Gwlist,const struct ip4_addr addr,uint16_t port,uint8_t gwid,uint16_t gwkeepalive);
void mqtt_sn_create_socket(MQTT_SN_Client_t* client);

//void mqtt_sn_send_connect(MQTT_SN_Client_t* client,struct ip4_addr* svrip,uint16_t dstport ,const char * client_id, uint16_t keepalive,uint8_t flags);
void mqtt_sn_send_connect(MQTT_SN_Client_t* mqttclient,const char * client_id, uint16_t keepalive,uint8_t flags);
void mqtt_sn_send_register(MQTT_SN_Client_t* client,const char* topic_name);
void mqtt_sn_send_willtopic(MQTT_SN_Client_t* client,const char* topic_name);
void mqtt_sn_send_willMSG(MQTT_SN_Client_t* client,const char* topic_name);                          
void mqtt_sn_send_pingreq(MQTT_SN_Client_t* client,uint8_t mode);                         
void mqtt_sn_send_disconnect(MQTT_SN_Client_t* client,uint16_t duration);                           
void mqtt_sn_send_register(MQTT_SN_Client_t* client,const char* topic_name); 
void mqtt_sn_getgateway_info(MQTT_SN_Client_t* client,const uint16_t remote_port);
                          
void mqtt_sn_send_publish(MQTT_SN_Client_t* client, uint16_t topic_id, uint8_t topic_type, const void* data, uint16_t data_len,\
    int8_t qos, uint8_t retain,uint8_t DUP);
void mqtt_sn_send_subscribe_topic_name(MQTT_SN_Client_t* mqttclient, const char* topic_name, int8_t qos, uint8_t dup);
void mqtt_sn_send_subscribe_topic_id(MQTT_SN_Client_t* client, uint16_t topic_id, int8_t qos , uint8_t dup);

void mqtt_sn_send_unsubscribe_topic_name(MQTT_SN_Client_t* client, const char* topic_name);
void mqtt_sn_send_unsubscribe_topic_id(MQTT_SN_Client_t* client, uint16_t topic_id);
void mqtt_sn_send_disconnect_topic_id(MQTT_SN_Client_t* mqttclient, uint16_t duration);
#endif


