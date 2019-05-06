#include "MQTTSNClientTopicMap.h"
#include <string.h>
topic_map_t         MY_TopicIDMap;
//sub_topic_map_t     MY_Sub_Topic;

const char* topiclist[MQTT_SN_CLIENT_LIST_MAX]={"FMSH_POWER_TEMP_702","FMSH_POWER_HUMI_702"};

/**
  * @brief   初始化主题list
  *
  * @retval  None
  */
void TopicInitConfig(void)
{
    uint8_t i; 
    MY_TopicIDMap.registerednum = 0;
    for(i=0;i<MQTT_SN_CLIENT_LIST_MAX;i++)
    {
        MY_TopicIDMap.toppiclist[i].status = unregister;
        MY_TopicIDMap.toppiclist[i].topic_id_lenth = strlen(topiclist[i]);
        memcpy(MY_TopicIDMap.toppiclist[i].topic_name,topiclist[i],strlen(topiclist[i]));
    }  
}
/**
  * @brief   将对应注意名称的主题项目状态切换到注册中
  *
  * @param   topic 主题名
  *
  * @retval  None
  */
uint8_t TopicidRegistering(const char* topic)
{
    uint8_t i; 
    for(i=0;i<MQTT_SN_CLIENT_LIST_MAX;i++)
    {
        if(strcmp(topic,MY_TopicIDMap.toppiclist[i].topic_name) == 0)
        {
            MY_TopicIDMap.toppiclist[i].status = registering;
            return 0;
        }
    }
    return 1;
}
/**
  * @brief   将对应注意名称的主题项目状态切换到未注册
  *
  * @param   topicid 主题id
  *
  * @retval  None
  */
uint8_t TopicidUnregister(uint16_t topicid)
{
    uint8_t i; 
    for(i=0;i<MQTT_SN_CLIENT_LIST_MAX;i++)
    {
        if(MY_TopicIDMap.toppiclist[i].topic_id == topicid)
        {
            MY_TopicIDMap.toppiclist[i].status = unregister;
            return 0;
        }
    }
    return 1;
}
/**
  * @brief   将对应注意名称的主题项目状态切换到已注册
  *
  * @param   topic 主题id
  *
  * @retval  None
  */
uint8_t TopicidRegistered(uint16_t topicid)
{
    uint8_t i; 
    for(i=0;i<MQTT_SN_CLIENT_LIST_MAX;i++)
    {
        if(MY_TopicIDMap.toppiclist[i].status == registering)
        {
            MY_TopicIDMap.toppiclist[i].status = registered;
            MY_TopicIDMap.toppiclist[i].topic_id = topicid;
            return 0;
        }
    }
    return 1;
}
/**
  * @brief   获取主题list的地址方便操作
  *
  * @param   None
  *
  * @retval  主题list的地址
  */
topic_map_t* GetTopicMapAddr(void)
{
    return (&MY_TopicIDMap);
}
/**
  * @brief   获取对应注意名称的主题ID
  *
  * @param   topic 主题id
  *
  * @retval  None
  */
int16_t GetTopicID(const char * topicid)
{
    uint8_t i; 
    for(i=0;i<MQTT_SN_CLIENT_LIST_MAX;i++)
    {
        if(strcmp(topicid,MY_TopicIDMap.toppiclist[i].topic_name) == 0&&MY_TopicIDMap.toppiclist[i].status == registered)
        {
            return MY_TopicIDMap.toppiclist[i].topic_id;
        }
    }
    return -1;

}
/**
  * @brief   已订阅主题列表初始化
  *
  * @param   None
  *
  * @retval  None
  */
void SubTopicInit(void)
{
//    uint8_t i; 
//    MY_Sub_Topic.subnum = 0;
//    for(i=0;i<MQTT_SN_CLIENT_LIST_MAX;i++)
//    {
//        MY_Sub_Topic.toppiclist[i].topic_id_lenth = 0;
//        MY_Sub_Topic.toppiclist[i].status = unsub;
//        memset(MY_TopicIDMap.toppiclist[i].topic_name,0,MQTT_SN_MAX_TOPIC_LENGTH);
//    }
}
/**
  * @brief   获取对应注意名称的主题ID
  *
  * @param   topic 主题id
  *
  * @retval  None
  */
uint8_t TopicidSubing(const char* topic)
{
//    uint8_t i; 
//    for(i=0;i<MQTT_SN_CLIENT_LIST_MAX;i++)
//    {
//        if(strcmp(topic,MY_Sub_Topic.toppiclist[i].topic_name) == 0)
//        {
//            MY_TopicIDMap.toppiclist[i].status = registering;
//            return 0;
//        }
//    }
//    return 1;
}
/**
  * @brief   获取对应注意名称的主题ID
  *
  * @param   topic 主题id
  *
  * @retval  None
  */
uint8_t TopicidSubed(uint16_t topicid)
{
//    uint8_t i; 
//    MY_Sub_Topic.subnum = 0;
//    for(i=0;i<MQTT_SN_CLIENT_LIST_MAX;i++)
//    {
//        MY_Sub_Topic.toppiclist[i].topic_id_lenth = 0;
//        MY_Sub_Topic.toppiclist[i].status = unsub;
//        memset(MY_TopicIDMap.toppiclist[i].topic_name,0,MQTT_SN_MAX_TOPIC_LENGTH);
//    }
}
/**
  * @brief   获取对应注意名称的主题ID
  *
  * @param   topic 主题id
  *
  * @retval  None
  */
uint8_t TopicidUnsub(uint16_t topicid)
{
//    uint8_t i; 
//    MY_Sub_Topic.subnum = 0;
//    for(i=0;i<MQTT_SN_CLIENT_LIST_MAX;i++)
//    {
//        MY_Sub_Topic.toppiclist[i].topic_id_lenth = 0;
//        MY_Sub_Topic.toppiclist[i].status = unsub;
//        memset(MY_TopicIDMap.toppiclist[i].topic_name,0,MQTT_SN_MAX_TOPIC_LENGTH);
//    }
}



