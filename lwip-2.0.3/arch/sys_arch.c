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
/* lwIP includes. */
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "sys_arch.h"
#include "string.h"
#include "task.h"

//����Ϣָ��Ϊ��ʱ,ָ��һ������pvNullPointer��ָ���ֵ.
//��UCOS�����OSQPost()�е�msg==NULL�᷵��һ��OS_ERR_POST_NULL
//����,����lwip�л����sys_mbox_post(mbox,NULL)����һ������Ϣ,����
//�ڱ������а�NULL���һ������ָ��0Xffffffff
//const void * const pvNullPointer = (mem_ptr_t*)0x01010101;
 
//ΪLWIP�ṩ��ʱ
u32_t sys_now(void){
	return xTaskGetTickCount();
} 



//����һ����Ϣ����
//*mbox:��Ϣ����
//size:�����С
//����ֵ:ERR_OK,�����ɹ�
//         ����,����ʧ��
err_t sys_mbox_new( sys_mbox_t *mbox, int size)
{
	(*mbox)= xQueueCreate(size,sizeof(void*)); 
	if((*mbox) == NULL)
		return ERR_MEM;
	return ERR_OK;
//	(*mbox)=mymalloc(SRAMIN,sizeof(TQ_DESCR));	//Ϊ��Ϣ���������ڴ�
//	mymemset((*mbox),0,sizeof(TQ_DESCR)); 		//���mbox���ڴ�
//	if(*mbox)//�ڴ����ɹ�
//	{
//		if(size>MAX_QUEUE_ENTRIES)size=MAX_QUEUE_ENTRIES;		//��Ϣ�����������MAX_QUEUE_ENTRIES��Ϣ��Ŀ 
// 		(*mbox)->pQ=OSQCreate(&((*mbox)->pvQEntries[0]),size);  //ʹ��UCOS����һ����Ϣ����
//		LWIP_ASSERT("OSQCreate",(*mbox)->pQ!=NULL); 
//		if((*mbox)->pQ!=NULL)return ERR_OK;  //����ERR_OK,��ʾ��Ϣ���д����ɹ� ERR_OK=0
//		else
//		{ 
//			myfree(SRAMIN,(*mbox));
//			return ERR_MEM;  		//��Ϣ���д�������
//		}
//	}else return ERR_MEM; 			//��Ϣ���д������� 
} 
//�ͷŲ�ɾ��һ����Ϣ����
//*mbox:Ҫɾ������Ϣ����
void sys_mbox_free(sys_mbox_t * mbox)
{
	//LWIP_ASSERT( "OSQDel ",uxQueueMessagesWaiting(*mbox) == 0 ); 	
	vQueueDelete(*mbox);
	*mbox=NULL;
//	u8_t ucErr;
//	sys_mbox_t m_box=*mbox;   
//	(void)OSQDel(m_box->pQ,OS_DEL_ALWAYS,&ucErr);
//	LWIP_ASSERT( "OSQDel ",ucErr == OS_ERR_NONE ); 
//	myfree(SRAMIN,m_box); 
//	*mbox=NULL;
}
//����Ϣ�����з���һ����Ϣ(���뷢�ͳɹ�)
//*mbox:��Ϣ����
//*msg:Ҫ���͵���Ϣ
void sys_mbox_post(sys_mbox_t *mbox,void *msg)
{  
	
	//if(&msg==NULL) msg =(void*)&pvNullPointer;//��msgΪ��ʱ msg����pvNullPointerָ���ֵ 
  while(xQueueSend(*mbox,&msg,1) != pdPASS);
	
//	if(msg==NULL)msg=(void*)&pvNullPointer;//��msgΪ��ʱ msg����pvNullPointerָ���ֵ 
//	while(OSQPost((*mbox)->pQ,msg)!=OS_ERR_NONE);//��ѭ���ȴ���Ϣ���ͳɹ� 
}
//������һ����Ϣ���䷢����Ϣ
//�˺��������sys_mbox_post����ֻ����һ����Ϣ��
//����ʧ�ܺ󲻻᳢�Եڶ��η���
//*mbox:��Ϣ����
//*msg:Ҫ���͵���Ϣ
//����ֵ:ERR_OK,����OK
// 	     ERR_MEM,����ʧ��
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{ 
	//if(msg==NULL)msg=(void*)&pvNullPointer;//��msgΪ��ʱ msg����pvNullPointerָ���ֵ 
  if(xQueueSend(*mbox,&msg,0) != pdPASS)
		return ERR_MEM;	
	return ERR_OK;
//	if(msg==NULL)msg=(void*)&pvNullPointer;//��msgΪ��ʱ msg����pvNullPointerָ���ֵ 
//	if((OSQPost((*mbox)->pQ, msg))!=OS_ERR_NONE)return ERR_MEM;
//	return ERR_OK;
}

//�ȴ������е���Ϣ
//*mbox:��Ϣ����
//*msg:��Ϣ
//timeout:��ʱʱ�䣬���timeoutΪ0�Ļ�,��һֱ�ȴ�
//����ֵ:��timeout��Ϊ0ʱ����ɹ��Ļ��ͷ��صȴ���ʱ�䣬
//		ʧ�ܵĻ��ͷ��س�ʱSYS_ARCH_TIMEOUT
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{ 
	u32_t timeout_new,timeout_last;
	BaseType_t base_rslt;
	if(timeout!=0)
	{
		if(timeout<1)timeout=1;//����1������
	}else timeout = portMAX_DELAY;
	if(msg!=NULL)
	{
		timeout_last = xTaskGetTickCount();
		base_rslt = xQueueReceive(*mbox, msg, timeout);
		//if(msg==(void*)&pvNullPointer)*msg = NULL;   	//��Ϊlwip���Ϳ���Ϣ��ʱ������ʹ����pvNullPointerָ��,�����ж�pvNullPointerָ���ֵ
																										//�Ϳ�֪�����󵽵���Ϣ�Ƿ���Ч
	}
	if(base_rslt==errQUEUE_FULL) timeout = SYS_ARCH_TIMEOUT;  //����ʱ
	else
	{
		timeout_new=xTaskGetTickCount();
		if (timeout_new>=timeout_last) timeout = timeout_new - timeout_last;//���������Ϣ��ʹ�õ�ʱ��
		else timeout = 0xffffffff - timeout_last + timeout_new;  
	}
	return timeout; 
//	u8_t ucErr;
//	u32_t ucos_timeout,timeout_new;
//	void *temp;
//	sys_mbox_t m_box=*mbox;
//	if(timeout!=0)
//	{
//		ucos_timeout=(timeout*OS_TICKS_PER_SEC)/1000; //ת��Ϊ������,��ΪUCOS��ʱʹ�õ��ǽ�����,��LWIP����ms
//		if(ucos_timeout<1)ucos_timeout=1;//����1������
//	}else ucos_timeout = 0; 
//	timeout = OSTimeGet(); //��ȡϵͳʱ�� 
//	temp=OSQPend(m_box->pQ,(u16_t)ucos_timeout,&ucErr); //������Ϣ����,�ȴ�ʱ��Ϊucos_timeout
//	if(msg!=NULL)
//	{	
//		if(temp==(void*)&pvNullPointer)*msg = NULL;   	//��Ϊlwip���Ϳ���Ϣ��ʱ������ʹ����pvNullPointerָ��,�����ж�pvNullPointerָ���ֵ
// 		else *msg=temp;									//�Ϳ�֪�����󵽵���Ϣ�Ƿ���Ч
//	}    
//	if(ucErr==OS_ERR_TIMEOUT)timeout=SYS_ARCH_TIMEOUT;  //����ʱ
//	else
//	{
//		LWIP_ASSERT("OSQPend ",ucErr==OS_ERR_NONE); 
//		timeout_new=OSTimeGet();
//		if (timeout_new>timeout) timeout_new = timeout_new - timeout;//���������Ϣ��ʹ�õ�ʱ��
//		else timeout_new = 0xffffffff - timeout + timeout_new; 
//		timeout=timeout_new*1000/OS_TICKS_PER_SEC + 1; 
//	}
//	return timeout; 
}
//���Ի�ȡ��Ϣ
//*mbox:��Ϣ����
//*msg:��Ϣ
//����ֵ:�ȴ���Ϣ���õ�ʱ��/SYS_ARCH_TIMEOUT
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
	return sys_arch_mbox_fetch(mbox,msg,1);//���Ի�ȡһ����Ϣ
}

//���һ����Ϣ�����Ƿ���Ч
//*mbox:��Ϣ����
//����ֵ:1,��Ч.
//      0,��Ч
int sys_mbox_valid(sys_mbox_t *mbox)
{ 
	  if(*mbox != NULL)
		{
			return 1;
		}
		else
			return 0;
//	sys_mbox_t m_box=*mbox;
//	u8_t ucErr;
//	int ret;
//	OS_Q_DATA q_data;
//	memset(&q_data,0,sizeof(OS_Q_DATA));
//	ucErr=OSQQuery (m_box->pQ,&q_data);
//	ret=(ucErr<2&&(q_data.OSNMsgs<q_data.OSQSize))?1:0;
//	return ret; 
} 
//����һ����Ϣ����Ϊ��Ч
//*mbox:��Ϣ����
void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
	*mbox=NULL;
} 

//����һ���ź���
//*sem:�������ź���
//count:�ź���ֵ
//����ֵ:ERR_OK,����OK
// 	     ERR_MEM,����ʧ��
err_t sys_sem_new(sys_sem_t * sem, u8_t count)
{  
	*sem = xSemaphoreCreateCounting(100,count);
	if(*sem==NULL)
		return ERR_MEM; 
	else
	{ if(count!=0)
		{
			xSemaphoreGive(*sem);
		}
		return ERR_OK;
	}
//	u8_t err; 
//	*sem=OSSemCreate((u16_t)count);
//	if(*sem==NULL)return ERR_MEM; 
//	OSEventNameSet(*sem,"LWIP Sem",&err);
//	LWIP_ASSERT("OSSemCreate ",*sem != NULL );
//	return ERR_OK;
} 
//�ȴ�һ���ź���
//*sem:Ҫ�ȴ����ź���
//timeout:��ʱʱ��
//����ֵ:��timeout��Ϊ0ʱ����ɹ��Ļ��ͷ��صȴ���ʱ�䣬
//		ʧ�ܵĻ��ͷ��س�ʱSYS_ARCH_TIMEOUT
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{ 
	
	u32_t timeout_new,timeout_last;
	BaseType_t ret;
	if(	timeout!=0) 
	{
		if(timeout < 1)
			timeout = 1;
	}else timeout = portMAX_DELAY; 
	timeout_last = xTaskGetTickCount();
	ret = xSemaphoreTake(*sem, timeout);
	if(ret == pdFALSE)
		timeout=SYS_ARCH_TIMEOUT;
	else
	{
		timeout_new=xTaskGetTickCount();
		if (timeout_new>=timeout_last) timeout = timeout_new - timeout_last;//���������Ϣ��ʹ�õ�ʱ��
		else timeout = 0xffffffff - timeout_last + timeout_new;  
	}
	return timeout;
//	u8_t ucErr;
//	u32_t ucos_timeout, timeout_new; 
//	if(	timeout!=0) 
//	{
//		ucos_timeout = (timeout * OS_TICKS_PER_SEC) / 1000;//ת��Ϊ������,��ΪUCOS��ʱʹ�õ��ǽ�����,��LWIP����ms
//		if(ucos_timeout < 1)
//		ucos_timeout = 1;
//	}else ucos_timeout = 0; 
//	timeout = OSTimeGet();  
//	OSSemPend (*sem,(u16_t)ucos_timeout, (u8_t *)&ucErr);
// 	if(ucErr == OS_ERR_TIMEOUT)timeout=SYS_ARCH_TIMEOUT;//����ʱ	
//	else
//	{     
// 		timeout_new = OSTimeGet(); 
//		if (timeout_new>=timeout) timeout_new = timeout_new - timeout;
//		else timeout_new = 0xffffffff - timeout + timeout_new;
// 		timeout = (timeout_new*1000/OS_TICKS_PER_SEC + 1);//���������Ϣ��ʹ�õ�ʱ��(ms)
//	}
//	return timeout;
}
//����һ���ź���
//sem:�ź���ָ��
void sys_sem_signal(sys_sem_t *sem)
{
	xSemaphoreGive(*sem);
//	OSSemPost(*sem);
}
//�ͷŲ�ɾ��һ���ź���
//sem:�ź���ָ��
void sys_sem_free(sys_sem_t *sem)
{
	vSemaphoreDelete(*sem);
	*sem = NULL;
//	u8_t ucErr;
//	(void)OSSemDel(*sem,OS_DEL_ALWAYS,&ucErr );
//	if(ucErr!=OS_ERR_NONE)LWIP_ASSERT("OSSemDel ",ucErr==OS_ERR_NONE);
//	*sem = NULL;
} 

//��ѯһ���ź�����״̬,��Ч����Ч
//sem:�ź���ָ��
//����ֵ:1,��Ч.
//      0,��Ч
int sys_sem_valid(sys_sem_t *sem)
{
//	UBaseType_t ret;
//	if(*sem!= NULL)
//	ret = uxSemaphoreGetCount(*sem);
//	if(ret!=0)
//		return 1;
//	else
		if(*sem!= NULL) return 1;
	  else return 0;
//	OS_SEM_DATA  sem_data;
//	return (OSSemQuery (*sem,&sem_data) == OS_ERR_NONE )? 1:0;              
} 
//����һ���ź�����Ч
//sem:�ź���ָ��

void sys_sem_set_invalid(sys_sem_t *sem)
{
	*sem=NULL;
}

//arch��ʼ��
void sys_init(void)
{ 
    //����,�����ڸú���,�����κ�����
} 
//extern OS_STK * TCPIP_THREAD_TASK_STK;//TCP IP�ں������ջ,��lwip_comm��������
//����һ���½���
//*name:��������
//thred:����������
//*arg:�����������Ĳ���
//stacksize:��������Ķ�ջ��С
//prio:������������ȼ�
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
		BaseType_t  result;
	  result = xTaskCreate(thread,name,stacksize,arg,prio,NULL);
		if(result != pdPASS )
		{
			return 0;
		}
	return 1;
}

//#define sys_msleep
//lwip��ʱ����
//ms:Ҫ��ʱ��ms��
void sys_msleep(u32_t ms)
{
	vTaskDelay(ms);

}


/*
  This optional function does a "fast" critical region protection and returns
  the previous protection level. This function is only called during very short
  critical regions. An embedded system which supports ISR-based drivers might
  want to implement this function by disabling interrupts. Task-based systems
  might want to implement this by using a mutex or disabling tasking. This
  function should support recursive calls from the same task or interrupt. In
  other words, sys_arch_protect() could be called while already protected. In
  that case the return value indicates that it is already protected.

  sys_arch_protect() is only required if your port is supporting an operating
  system.
*/
sys_prot_t sys_arch_protect(void)
{
	vPortEnterCritical();
	return 1;
}

/*
  This optional function does a "fast" set of critical region protection to the
  value specified by pval. See the documentation for sys_arch_protect() for
  more information. This function is only required if your port is supporting
  an operating system.
*/
void sys_arch_unprotect(sys_prot_t pval)
{
	( void ) pval;
	vPortExitCritical();
}


