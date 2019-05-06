/*************************************************************************
                Copyright (c), 1998-2015, 上海复旦微电子集团股份有限公司 
                                      电力电子事业部 

    文件名称： drv_timer.c  计时模块
    文件描述:  计时模块
    注意事项:
    修订记录： 
            1、作者:  孟祥晨
               时间： 2015-11-02
               内容:  创建文件

**************************************************************************/


#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_it.h"


void WDG_Init( void )
{
    u32 psc = 0;
    u32 reload = 0;
	u16 Psc_Vel[8]={4,8,16,32,64,128,256,256};//对应分频真值
	u32 WDG_SetTimes=2000;//单位ms
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);/*允许访问IWDG_PR和IWDG_RLR寄存器*/
    
    /*调用预分频系数和重装载计数器数值计算函数*/
    psc = 5;
    reload = 40*WDG_SetTimes /Psc_Vel[psc];
    IWDG_SetPrescaler(psc);/*设置预分频系数*/
    IWDG_SetReload(reload);/*设置重装载寄存器值，当前这种定时约为500ms*/
    IWDG_ReloadCounter();/*设置重装载寄存器*/
    IWDG_Enable();/*启动看门狗*/    
}


void WDG_Kick( void )
{
    IWDG_ReloadCounter();/*设置重装载寄存器*/
}


