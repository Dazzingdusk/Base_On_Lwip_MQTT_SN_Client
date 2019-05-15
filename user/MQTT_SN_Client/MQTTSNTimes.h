#ifndef MQTTSNTIMERS_H
#define MQTTSNTIMERS_H
#include <stdint.h>

#define         MAX_TIMER_NUM               10

#define         TIMER_STATUS_RUNING         1
#define         TIMER_STATUS_STOP           0

#define         TIMER_MODE_LOOP             1
#define         TIMER_MODE_SINGLE           !TIMER_MODE_LOOP

#define         ERROR_TIMER_NONE            (0)
#define         ERROR_TIMER_ADD_EMPTY       (-1)
#define         ERROR_TIMER_DEL_EMPTY       (-2)
#define         ERROR_TIMER_DEL_RUNING      (-3)
#define         ERROR_TIMER_UNUSED          (-4)
#define         ERROR_TIMER_INDEX           (-5)

/*预定义的超时操作ID*/
#define TIMER_ID_OF_PINGREQ                 (1)
#define TIMER_ID_OF_CONNCET                 (2)
#define TIMER_ID_OF_REGISTER                (3)
#define TIMER_ID_OF_PUBLISH                 (4)
#define TIMER_ID_OF_DISCONNCET              (5)
#define TIMER_ID_OF_SUBSCRIBE               (6)
//#define         DEBUG

typedef void (* FUN_TIMER_CALLBACK_T)(void*arg);

typedef struct {
    uint8_t loop;
    uint8_t Runing;
    uint32_t PrevTick;
    uint32_t TimeOut;
    uint8_t  TimerID;
    void * arg;
    FUN_TIMER_CALLBACK_T timer_callback;
} SW_TIMER_T;

typedef struct {
   uint8_t Free; 
   uint8_t Unused[MAX_TIMER_NUM];
   SW_TIMER_T TIMER_T[MAX_TIMER_NUM];
}TIMER;
 

void  TimerInit(void );
void  TimerCheck(uint32_t nowtimetick );


int32_t TimerAdd(uint8_t mode ,uint32_t timeout,FUN_TIMER_CALLBACK_T   cb ,void* arg,uint8_t TimerID);
int32_t TimerDel(uint8_t timerindex);
int32_t TimerReload(uint8_t timerindex);
int32_t TimerStop(uint8_t timerindex);
int32_t FindTimerIndexbyID(uint8_t TimerID );
#ifdef DEBUG

void DebugPrint(uint32_t timerindex);
int32_t GetTimerFreeNum(void);
#endif

#endif

