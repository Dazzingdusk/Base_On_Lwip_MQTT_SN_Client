/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __SYS_ARCH_H__
#define __SYS_ARCH_H__ 
#include "arch/cc.h"
#include "arch.h"
#include "FreeRTOS.h"
#include "task.h"
//#include "timers.h"
#include "semphr.h"
 
// 
//#ifdef SYS_ARCH_GLOBALS
//#define SYS_ARCH_EXT
//#else
//#define SYS_ARCH_EXT extern
//#endif
 
#define MAX_QUEUES        		10	// ��Ϣ���������
#define MAX_QUEUE_ENTRIES 		20	// ÿ����Ϣ����Ĵ�С


//LWIP��Ϣ����ṹ��
typedef struct {
    void*   pvQEntries[MAX_QUEUE_ENTRIES];//��Ϣ���� MAX_QUEUE_ENTRIES��Ϣ�����������Ϣ��
} TQ_DESCR, *PQ_DESCR;



typedef SemaphoreHandle_t   	sys_sem_t;    //LWIPʹ�õ��ź���
typedef SemaphoreHandle_t 		sys_mutex_t; 	//LWIPʹ�õĻ����ź���
typedef QueueHandle_t  				sys_mbox_t;   //LWIPʹ�õ���Ϣ����,��ʵ������Ϣ����
typedef unsigned char 				sys_thread_t;   //�߳�ID,Ҳ�����������ȼ�









#define SYS_MBOX_NULL (xQueueHandle)0
#define SYS_SEM_NULL  (xSemaphoreHandle)0
#define SYS_DEFAULT_THREAD_STACK_DEPTH	configMINIMAL_STACK_SIZE



typedef struct _sys_arch_state_t
{
	// Task creation data.
	char cTaskName[configMAX_TASK_NAME_LEN];
	unsigned short nStackDepth;
	unsigned short nTaskCount;
} sys_arch_state_t;
#endif 

/* __SYS_RTXC_H__ */

































