#include "system_stm32f10x.h"
#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "string.h"
//
#include "lwip/netif.h"
#include "lwipopts.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/ip_addr.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/init.h"
//
#include "ethernet.h"
#include "drv_ethernet.h"
#include "drv_ethernet.h"
#include "drv_usart.h"
#include "drv_wdg.h"
//
#include "MQTTClient.h"
#include "MQTT_SN_GateWay.h"
#include "MQTTSNTimes.h"
#include "MQTTSNClient.h"
#include "MQTTSNClientTopicMap.h"
//为LWIP提供计时
extern char buildConnetSuccess;

#if LWIP_DHCP
extern u8  dhcpstatus;
#endif


void TASK_MQTT_Client(void* pvParameters);
void TASK_TIMER_Test(void* pvParameters);

xTaskHandle x_Handle_MQTT_Client;
xTaskHandle x_Handle_TIMER_Test;


int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); 
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//关闭JTAG调试接口打开ＳＷＤ调试接口
    uart_init();
    TimerInit();
    TopicInitConfig();
    #if LWIP_DHCP
    while(dhcpstatus!=2&&dhcpstatus!=0xFF);
    {
        lwip_periodic_handle();
    }
    #endif
    WDG_Init();
    //xTaskCreate(TASK_MQTT_Client, "Main", 500, NULL, 3, &x_Handle_MQTT_Client);//创建任务
    xTaskCreate(TASK_TIMER_Test, "Test", 200, NULL, 3, &x_Handle_TIMER_Test);//创建任务
    vTaskStartScheduler();
  
        
    return 0;																						
}

//void TASK_MQTT_Client(void* pvParameters)
//{
//    u32 lwip_localtime,last_localtime;
//    mqtt_client_t *client = 0;
//    while(lwip_comm_init());
//    printf("CommInitFinished!\r\n");
////	client = mqtt_client_new();
////    if(client != NULL) {
////    example_do_connect(client);
////    }
//    drv_UdpServer_Init();
//    while(1)
//    {
////        while(buildConnetSuccess==0);
//        WDG_Kick();
//        lwip_localtime = xTaskGetTickCount();
////        if (lwip_localtime%2000==0 && last_localtime!=lwip_localtime)
////        {
////            last_localtime = lwip_localtime;
////            example_publish(client,0);
////        }
// 
//    }
//}


void mqtt_sn_client_cyclic_cb2(void*arg)
{
    printf("Timr test\r\n");
    TimerAdd(TIMER_MODE_SINGLE,1000,mqtt_sn_client_cyclic_cb2,0,0);
}

void TASK_TIMER_Test(void* pvParameters)
{
    uint32_t systick,last,flag = 0;
    char *msg="Hello FMSH Power everyone";
    MQTT_SN_Client_t    *mqtt_sn_client_pcb;
    while(lwip_comm_init());
    printf("CommInitFinished!\r\n");
    MQTTClientInit();
    mqtt_sn_client_pcb=GetClientPCB();
    //TimerAdd(TIMER_MODE_SINGLE,1000,mqtt_sn_client_cyclic_cb2,0,0);
    while(1)
    {
        WDG_Kick();
        systick=xTaskGetTickCount();
        TimerCheck(systick);       
        if((systick%10000 == 0)&&registertopicfinsh())
        {
            if(flag==0)
            {
                mqtt_sn_send_subscribe_topic_name(mqtt_sn_client_pcb,topiclist[1],0,0);
                flag = 1;
            }
            if(flag < 15)
            {
                mqtt_sn_send_publish(mqtt_sn_client_pcb,GetTopicID(topiclist[0]),MQTT_SN_TOPIC_TYPE_NORMAL,msg,strlen(msg),1,1,0);
                flag++;
                if(flag==10)
                {
                    mqtt_sn_send_unsubscribe_topic_name(mqtt_sn_client_pcb,topiclist[1]);
                }
            }
            else if(flag == 15)
            {
                flag++;
                mqtt_sn_send_disconnect_topic_id(mqtt_sn_client_pcb,0);
            }
        }
        vTaskDelayUntil ( &systick, 1 ); //1ms检查1次
    }

}
