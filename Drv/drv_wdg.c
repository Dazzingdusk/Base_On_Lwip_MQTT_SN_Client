/*************************************************************************
                Copyright (c), 1998-2015, �Ϻ�����΢���Ӽ��Źɷ����޹�˾ 
                                      ����������ҵ�� 

    �ļ����ƣ� drv_timer.c  ��ʱģ��
    �ļ�����:  ��ʱģ��
    ע������:
    �޶���¼�� 
            1������:  ���鳿
               ʱ�䣺 2015-11-02
               ����:  �����ļ�

**************************************************************************/


#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_it.h"


void WDG_Init( void )
{
    u32 psc = 0;
    u32 reload = 0;
	u16 Psc_Vel[8]={4,8,16,32,64,128,256,256};//��Ӧ��Ƶ��ֵ
	u32 WDG_SetTimes=2000;//��λms
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);/*�������IWDG_PR��IWDG_RLR�Ĵ���*/
    
    /*����Ԥ��Ƶϵ������װ�ؼ�������ֵ���㺯��*/
    psc = 5;
    reload = 40*WDG_SetTimes /Psc_Vel[psc];
    IWDG_SetPrescaler(psc);/*����Ԥ��Ƶϵ��*/
    IWDG_SetReload(reload);/*������װ�ؼĴ���ֵ����ǰ���ֶ�ʱԼΪ500ms*/
    IWDG_ReloadCounter();/*������װ�ؼĴ���*/
    IWDG_Enable();/*�������Ź�*/    
}


void WDG_Kick( void )
{
    IWDG_ReloadCounter();/*������װ�ؼĴ���*/
}


