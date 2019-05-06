#ifndef MQTTSNCLIENTTOPICMAP_H
#define MQTTSNCLIENTTOPICMAP_H
#include <stdint.h>
#include "MQTTSNClient.h"

#define MQTT_SN_CLIENT_LIST_MAX             (2)

typedef enum { unregister, registering, registered }TOPIC_STATUS;

typedef struct  
{
    uint16_t topic_id;
    uint8_t topic_id_lenth;
    char topic_name[MQTT_SN_MAX_TOPIC_LENGTH];
    TOPIC_STATUS status;
} topic_map_item_t;

typedef struct  
{
    uint8_t registerednum;
    topic_map_item_t toppiclist[MQTT_SN_CLIENT_LIST_MAX];
    
} topic_map_t;


typedef enum { unsub, subing, subed }SUB_TOPIC_STATUS;
typedef struct  
{
    uint8_t topic_id_lenth;
    subscribe_t topic_name;
    SUB_TOPIC_STATUS status;
} sub_topic_map_item_t;
typedef struct  
{
    uint8_t subnum;
    sub_topic_map_item_t toppiclist[MQTT_SN_CLIENT_LIST_MAX];
    
} sub_topic_map_t;



extern const char* topiclist[MQTT_SN_CLIENT_LIST_MAX];


/*Topic Register Manager*/
void TopicInitConfig(void);

topic_map_t* GetTopicMapAddr(void);
uint8_t TopicidRegistering(const char* topic);
uint8_t TopicidRegistered(uint16_t topicid);
uint8_t TopicidUnregister(uint16_t topicid);
int16_t GetTopicID(const char * topicid);

/*SubTopic Register Manager*/
void SubTopicInit(void);

uint8_t TopicidSubing(const char* topic);
uint8_t TopicidSubed(uint16_t topicid);
uint8_t TopicidUnsub(uint16_t topicid);


#endif

