#ifndef __MY_MEM_H__
#define __MY_MEM_H__
#include <stdint.h>
#include <string.h>

//config
#define MAX_BUFF_SIZE    														(512)

//err
#define ERROR_NONE																	(0)
#define ERROR_OVER_FLOW_WRITE												(-1)
#define ERROR_OVER_FLOW_READ												(-2)
#define ERROR_NOT_INIT_BUFF													(-3)
#define ERROR_DATA_ILLEGAL_OPERAT										(-4)
//
#define  GetBuff_Valid_DataLenth(w_ptr,r_ptr,ring)  ((ring)?(MAX_BUFF_SIZE):(w_ptr==r_ptr)?(0):((w_ptr>r_ptr)?(w_ptr-r_ptr):(MAX_BUFF_SIZE-r_ptr+w_ptr)))
#define  GetBuff_Free_DataLenth(w_ptr,r_ptr,ring)   (MAX_BUFF_SIZE - (GetBuff_Valid_DataLenth(w_ptr,r_ptr,ring)))
#define  INITBUFFED																	(1)
//

//#define  DEBUG_BUFF

typedef struct
{
    uint16_t 	r_ptr;//���ݶ�����
    uint16_t 	w_ptr;//����д����
	uint8_t  	ring;//��Ȧ
	uint8_t  	buffInited;//��ʼ����
    uint8_t 	buff[MAX_BUFF_SIZE];//���ݻ�����

}loop_buff_t;
//
void BuffInit(void);
int8_t WriteBUFF(uint8_t *data, uint16_t lenth);
int8_t ReadBUFF(uint8_t *data, uint16_t lenth);
uint16_t BuffSpaceget(void);

#ifdef DEBUG_BUFF
void printfStatus(void);
#endif

#endif

