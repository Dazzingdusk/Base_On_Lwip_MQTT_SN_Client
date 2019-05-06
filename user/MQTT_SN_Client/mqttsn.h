#ifndef MQTTSN_H
#define MQTTSN_H
#include "stdint.h"
#include "udp.h"

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

typedef enum  { QOS0, QOS1, QOS2,QOSN1 }QoS;
typedef enum  {MQTT_SN_NOTCONNECT, MQTT_SN_CONNECTING,MQTT_SN_CONNECTED,MQTT_SN_DISCONNECTING,MQTT_SN_DISCONNECTED}MQTTSNSTATUS_T;



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
//½ûÖ¹±àÒëÆ÷×Ô¶¯¶ÔÆë
typedef struct  
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
    char topic_name[MQTT_SN_MAX_TOPIC_LENGTH];
    uint16_t topic_id;
}subscribe_t;

typedef struct   
{
    uint8_t length;
    uint8_t type;
    uint8_t flags;
    uint16_t message_id;
    subscribe_t topic_id;

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
    uint16_t duration;
} disconnect_packet_t;


typedef struct  
{
    uint8_t length;
    uint8_t type;
} ping_respond_packet_t;

struct mqtt_connect_client_info_t {
  /** Client identifier, must be set by caller */
  const char *client_id;
  /** User name and password, set to NULL if not used */
  const char* client_user;
  const char* client_pass;
  /** keep alive time in seconds, 0 to disable keep alive functionality*/
  uint16_t keep_alive;
  /** will topic, set to NULL if will is not to be used,
      will_msg, will_qos and will retain are then ignored */
  const char* will_topic;
  const char* will_msg;
  uint8_t will_qos;
  uint8_t will_retain;
};


void mqtt_sn_create_socket(struct udp_pcb* pcb);

void mqtt_sn_send_connect(struct udp_pcb* pcb,struct ip4_addr* svrip,uint16_t dstport ,const char * client_id, uint16_t keepalive,uint8_t flags);
void mqtt_sn_send_register(struct udp_pcb* pcb,const char* topic_name);
void mqtt_sn_send_willtopic(struct udp_pcb* pcb,const char* topic_name);
void mqtt_sn_send_willMSG(struct udp_pcb* pcb,const char* topic_name);                          
void mqtt_sn_send_pingreq(struct udp_pcb* pcb,uint8_t mode);                         
void mqtt_sn_send_disconnect(struct udp_pcb* pcb,uint16_t duration);                           
void mqtt_sn_send_register(struct udp_pcb* pcb,const char* topic_name); 
void mqtt_sn_getgateway_info(struct udp_pcb* pcb,const uint16_t remote_port);
                          
void mqtt_sn_send_publish(struct udp_pcb* pcb, uint16_t topic_id, uint8_t topic_type, const void* data, uint16_t data_len, int8_t qos, uint8_t retain);
void mqtt_sn_send_subscribe_topic_name(struct udp_pcb* pcb, const char* topic_name, uint8_t qos);
void mqtt_sn_send_subscribe_topic_id(struct udp_pcb* pcb, uint16_t topic_id, uint8_t qos);

void mqtt_sn_send_unsubscribe_topic_name(struct udp_pcb* pcb, const char* topic_name, uint8_t qos);
void mqtt_sn_send_unsubscribe_topic_id(struct udp_pcb* pcb, uint16_t topic_id, uint8_t qos);


#endif

