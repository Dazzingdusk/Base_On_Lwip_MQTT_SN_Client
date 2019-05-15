#include "MQTTSNTimes.h"
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"

#define _TIME_SUB_ABS(PrevTimeTick,NowTimeTick)             ((NowTimeTick)>=(PrevTimeTick)?(NowTimeTick)-(PrevTimeTick):(0xFFFFFFFF-(PrevTimeTick)+(NowTimeTick)))       
#define _IS_TIME_OUT(PrevTimeTick,NowTimeTick,TimeOut)      ((_TIME_SUB_ABS((PrevTimeTick),(NowTimeTick)))>(TimeOut))?(1):(0)                                           \
                                                               
TIMER Systime;

void TimerInit(void )
{
   memset(&Systime,0,sizeof(TIMER));
   memset(&Systime.Unused,1,MAX_TIMER_NUM); 
   Systime.Free = MAX_TIMER_NUM;
}

int32_t TimerAdd(uint8_t mode ,uint32_t timeout,FUN_TIMER_CALLBACK_T   cb,void* arg,uint8_t TimerID )
{
    uint32_t num;
    
    if(Systime.Free <= 0)
    {
        return ERROR_TIMER_ADD_EMPTY;
    }
    for(num=0;num < MAX_TIMER_NUM;num++)
    {
        if(Systime.Unused[num])
        {
            Systime.TIMER_T[num].loop = mode;
            Systime.TIMER_T[num].TimeOut = timeout;
            Systime.TIMER_T[num].PrevTick = xTaskGetTickCount();
            Systime.TIMER_T[num].Runing = TIMER_STATUS_RUNING;
            Systime.TIMER_T[num].timer_callback =  cb;
            Systime.TIMER_T[num].arg = arg;
            Systime.TIMER_T[num].TimerID = TimerID;
            Systime.Unused[num] = 0;
            Systime.Free--;
            #ifdef DEBUG
            DebugPrint(num);
            GetTimerFreeNum();
            #endif
            return num;
        }
    }   
}
int32_t FindTimerIndexbyID(uint8_t TimerID )
{
    uint32_t num;

    for(num=0;num < MAX_TIMER_NUM;num++)
    {
        if(Systime.TIMER_T[num].TimerID ==TimerID )
        {
            return num;
        }
    } 
    return 0xFF;    
}
int32_t TimerDel(uint8_t timerindex)
{
    if (timerindex >= MAX_TIMER_NUM)
    {
        return ERROR_TIMER_INDEX;
    }
    if(Systime.Free >= MAX_TIMER_NUM)
    {
        return ERROR_TIMER_DEL_EMPTY;
    }
    if(Systime.Unused[timerindex])
    {
        return ERROR_TIMER_DEL_EMPTY;
    }
    if(Systime.TIMER_T[timerindex].Runing == TIMER_STATUS_RUNING)
    {
        return ERROR_TIMER_DEL_RUNING;
    }
    else
    {
        Systime.Unused[timerindex] = 1;
        Systime.Free++;
    }
    return ERROR_TIMER_NONE;
}

int32_t TimerReload(uint8_t timerindex)
{
    if (timerindex >= MAX_TIMER_NUM)
    {
        return ERROR_TIMER_INDEX;
    }
    if (Systime.TIMER_T[timerindex].Runing != TIMER_STATUS_RUNING)
    {
        return  ERROR_TIMER_UNUSED;
    }
    if (Systime.Unused[timerindex])
    {
        return  ERROR_TIMER_UNUSED;
    }

    Systime.TIMER_T[timerindex].PrevTick = xTaskGetTickCount();
    #ifdef DEBUG
    printf("Time is Reload\r\n");
    DebugPrint(timerindex);
    #endif
    return ERROR_TIMER_NONE;
}
int32_t TimerStop(uint8_t timerindex)
{
    if (timerindex >= MAX_TIMER_NUM)
    {
        return ERROR_TIMER_INDEX;
    }
    if (Systime.TIMER_T[timerindex].Runing == TIMER_STATUS_STOP)
    {
        return  ERROR_TIMER_NONE;
    }
    if (Systime.Unused[timerindex])
    {
        return  ERROR_TIMER_UNUSED;
    }

    Systime.TIMER_T[timerindex].Runing = TIMER_STATUS_STOP;

    return ERROR_TIMER_NONE;
}


#ifdef DEBUG

int32_t GetTimerFreeNum(void)
{
    printf("The Free Time Num is %u\r\n",Systime.Free);
    return Systime.Free;
}

void DebugPrint(uint32_t timerindex)
{
    printf("Timer PrevTick is: %u\r\n",Systime.TIMER_T[timerindex].PrevTick);
    printf("Timer TimeOut is:%u\r\n",Systime.TIMER_T[timerindex].TimeOut );
    printf("Timer Runing is:%u\r\n",Systime.TIMER_T[timerindex].Runing);
    printf("Timer loop is:%u\r\n",Systime.TIMER_T[timerindex].loop);
}
#endif

void TimerCheck(uint32_t nowtimetick )
{
    uint16_t num;
    #ifdef FREERTOS
    taskENTER_CRITICAL();
    #endif    
    
    for(num=0;num < MAX_TIMER_NUM;num++)
    {
        if(Systime.TIMER_T[num].Runing == TIMER_STATUS_RUNING)
        {
           #ifdef DEBUG 
            //printf("Runing Time index is:%d\r\n",num);
           #endif
            /**/
            if( _IS_TIME_OUT(Systime.TIMER_T[num].PrevTick,nowtimetick,Systime.TIMER_T[num].TimeOut))
            {
                Systime.TIMER_T[num].timer_callback(Systime.TIMER_T[num].arg);
                
                if(Systime.TIMER_T[num].loop == 1)
                {
                    #ifdef DEBUG 
                    printf("Runing Time Reload is:%u\r\n",TimerReload(num));
                    #else
                    TimerReload(num);
                    #endif
                    
                    
                }
                else
                {
                    TimerStop(num);
                    TimerDel(num);
                }
            }
        }
    }
    #ifdef FREERTOS
    taskEXIT_CRITICAL();
    #endif
}

