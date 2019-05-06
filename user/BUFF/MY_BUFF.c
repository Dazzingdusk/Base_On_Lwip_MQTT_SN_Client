#include "MY_BUFF.h"



#ifdef DEBUG_BUFF
#include <stdio.h>
#endif


loop_buff_t  BuffBody;

void BuffInit(void)
{
    memset(&BuffBody, 0, sizeof(loop_buff_t));
    BuffBody.buffInited = INITBUFFED;
}

uint16_t BuffSpaceget(void)
{
   return  (GetBuff_Free_DataLenth(BuffBody.w_ptr, BuffBody.r_ptr,BuffBody.ring));
}

/**
  * @brief  ������д�뺯��
  *          
  * @param  data: ��Ҫд��buff �����ݵ�ַ
*         	lenth: Ҫд���ַ�����ݳ�.
  * @retval 0 ��д��ɹ� ���������ɹ�					
  */
int8_t WriteBUFF(uint8_t *data, uint16_t lenth)
{
    uint16_t lastptr;
    uint16_t space_avail;
	/*ʹ��ǰδ��ʼ��*/
	if(BuffBody.buffInited != INITBUFFED)
	{
		return ERROR_NOT_INIT_BUFF;
	}
	/*�������ݲ��Ϸ�*/
    if ((lenth <= 0) || data == NULL)
    {
        return ERROR_DATA_ILLEGAL_OPERAT;
    }
	/*ʣ�೤�Ȳ���*/
	space_avail	= GetBuff_Free_DataLenth(BuffBody.w_ptr, BuffBody.r_ptr,BuffBody.ring);
	if(lenth > space_avail)
	{
		return ERROR_OVER_FLOW_WRITE;
	}
    #ifdef DEBUG_BUFF
    printf("W \r\n");
    printfStatus();
    #endif
    /*buff��h��W��R��*/
    if (BuffBody.w_ptr >= BuffBody.r_ptr)
    {
		/*�������ĺ��ι��ã�*/
        if ((MAX_BUFF_SIZE - BuffBody.w_ptr) >= lenth)
        {
            /*�������ݽ���*/
            memcpy(&BuffBody.buff[BuffBody.w_ptr], data, lenth);
            BuffBody.w_ptr += lenth;
						/*wָ��ֵ����߽���*/
            if (BuffBody.w_ptr == MAX_BUFF_SIZE)
            {
				BuffBody.w_ptr = 0;
				/*�����Ȧ*/
				if(BuffBody.w_ptr == BuffBody.r_ptr )
				{
					BuffBody.ring = 1;
				}  
            }
			return ERROR_NONE;
        }
		/*�������ĺ��β�����*/
        else
        {
			/*�������ĺ�����д��*/
            lastptr = MAX_BUFF_SIZE - BuffBody.w_ptr;
            memcpy(&BuffBody.buff[BuffBody.w_ptr], data, lastptr);
            BuffBody.w_ptr = 0;
			/*�������ĺ�����д��*/
            memcpy(&BuffBody.buff[BuffBody.w_ptr], &data[lastptr], lenth - lastptr);
            BuffBody.w_ptr += (lenth - lastptr);
			/*�����Ȧ*/
            if (BuffBody.w_ptr == BuffBody.r_ptr)
            {
                BuffBody.ring = 1;
            }
			return ERROR_NONE;
        }
    }
	/*buff�ǿ���R��W��*/
    else
    {
        memcpy(&BuffBody.buff[BuffBody.w_ptr], data, lenth);
        BuffBody.w_ptr += lenth;
		/*�����Ȧ*/
        if (BuffBody.w_ptr == BuffBody.r_ptr)
        {
            BuffBody.ring = 1;
        }
		return ERROR_NONE;
    }
}
/**
  * @brief  ��������ȡ����
  *          
  * @param  data: ��Ҫ����buff �����ݵĽ��յ�ַ 
*         	lenth: Ҫ������ַ�����ݳ�.
  * @retval 0 ����ȡ�ɹ� ���������ɹ�					
  */
int8_t ReadBUFF(uint8_t *data, uint16_t lenth)
{
    uint16_t lastptr;
    uint16_t avail_data_lenth ;
	/*ʹ��ǰδ��ʼ��*/
	if(BuffBody.buffInited != INITBUFFED)
	{
		return ERROR_NOT_INIT_BUFF;
	}
	/*�������ݲ��Ϸ�*/
    if ((lenth <= 0) || data == NULL)
    {
        return ERROR_DATA_ILLEGAL_OPERAT;
    }
		/*��ȡ���ݳ��ȴ����������ݳ�*/
		avail_data_lenth= GetBuff_Valid_DataLenth(BuffBody.w_ptr, BuffBody.r_ptr,BuffBody.ring);
    if (avail_data_lenth < lenth)
    {
        return ERROR_OVER_FLOW_READ;
    }
	/*buff������*/
	if (BuffBody.ring == 1)
	{
		BuffBody.ring = 0;
	}
		/*buff�ǿ���Wָ��λ���ں���*/
    if (BuffBody.w_ptr > BuffBody.r_ptr)
    {
        memcpy(data, &BuffBody.buff[BuffBody.r_ptr], lenth);
        BuffBody.r_ptr += lenth;
        #ifdef DEBUG_BUFF
        printf("R \r\n");
        printfStatus();
        #endif
		return ERROR_NONE;
    }
	/*buff�ǿ���r��w����*/
    else
    {
		/*��뻺��������*/
        if ((MAX_BUFF_SIZE - BuffBody.r_ptr) >= lenth)
        {
            memcpy(data, &BuffBody.buff[BuffBody.r_ptr], lenth);
            BuffBody.r_ptr += lenth;
            if (BuffBody.r_ptr == MAX_BUFF_SIZE)
            {
                BuffBody.r_ptr = 0;
            }
            #ifdef DEBUG_BUFF
            printf("R \r\n");
            printfStatus();
            #endif
			return ERROR_NONE;
        }
		/*��뻺����������*/
        else
        {
			/*�ȶ���뻺����*/
            lastptr = MAX_BUFF_SIZE - BuffBody.r_ptr;
            memcpy(data, &BuffBody.buff[BuffBody.r_ptr], lastptr);
            BuffBody.r_ptr = 0;
			/*ǰ�뻺������*/
            memcpy(&data[lastptr], &BuffBody.buff[BuffBody.r_ptr], lenth - lastptr);
            BuffBody.r_ptr += (lenth - lastptr);
            #ifdef DEBUG_BUFF
            printf("R \r\n");
            printfStatus();
            #endif
			return ERROR_NONE;
        }
		
    }
}
#ifdef DEBUG_BUFF
void printfStatus(void)
{
	printf("\r\n\r\n");
	printf("BUFF r_ptr:%d\r\n",BuffBody.r_ptr);
	printf("BUFF w_ptr:%d\r\n",BuffBody.w_ptr);
	printf("BUFF ring:%d\r\n",BuffBody.ring);
	printf("\r\n\r\n");
}
#endif
