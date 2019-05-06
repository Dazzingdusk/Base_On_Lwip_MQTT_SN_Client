#include "MQTTSNClientTopicMap.h"
#include <string.h>
topic_map_t         MY_TopicIDMap;
//sub_topic_map_t     MY_Sub_Topic;

const char* topiclist[MQTT_SN_CLIENT_LIST_MAX]={"FMSH_POWER_TEMP_702","FMSH_POWER_HUMI_702"};

/**
  * @brief   ��ʼ������list
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
  * @brief   ����Ӧע�����Ƶ�������Ŀ״̬�л���ע����
  *
  * @param   topic ������
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
  * @brief   ����Ӧע�����Ƶ�������Ŀ״̬�л���δע��
  *
  * @param   topicid ����id
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
  * @brief   ����Ӧע�����Ƶ�������Ŀ״̬�л�����ע��
  *
  * @param   topic ����id
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
  * @brief   ��ȡ����list�ĵ�ַ�������
  *
  * @param   None
  *
  * @retval  ����list�ĵ�ַ
  */
topic_map_t* GetTopicMapAddr(void)
{
    return (&MY_TopicIDMap);
}
/**
  * @brief   ��ȡ��Ӧע�����Ƶ�����ID
  *
  * @param   topic ����id
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
  * @brief   �Ѷ��������б��ʼ��
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
  * @brief   ��ȡ��Ӧע�����Ƶ�����ID
  *
  * @param   topic ����id
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
  * @brief   ��ȡ��Ӧע�����Ƶ�����ID
  *
  * @param   topic ����id
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
  * @brief   ��ȡ��Ӧע�����Ƶ�����ID
  *
  * @param   topic ����id
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



