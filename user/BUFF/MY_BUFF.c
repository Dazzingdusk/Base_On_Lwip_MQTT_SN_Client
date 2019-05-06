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
  * @brief  缓冲区写入函数
  *          
  * @param  data: 将要写入buff 的数据地址
*         	lenth: 要写入地址的数据长.
  * @retval 0 ：写入成功 其他：不成功					
  */
int8_t WriteBUFF(uint8_t *data, uint16_t lenth)
{
    uint16_t lastptr;
    uint16_t space_avail;
	/*使用前未初始化*/
	if(BuffBody.buffInited != INITBUFFED)
	{
		return ERROR_NOT_INIT_BUFF;
	}
	/*操作数据不合法*/
    if ((lenth <= 0) || data == NULL)
    {
        return ERROR_DATA_ILLEGAL_OPERAT;
    }
	/*剩余长度不够*/
	space_avail	= GetBuff_Free_DataLenth(BuffBody.w_ptr, BuffBody.r_ptr,BuffBody.ring);
	if(lenth > space_avail)
	{
		return ERROR_OVER_FLOW_WRITE;
	}
    #ifdef DEBUG_BUFF
    printf("W \r\n");
    printfStatus();
    #endif
    /*buff空h或W在R后*/
    if (BuffBody.w_ptr >= BuffBody.r_ptr)
    {
		/*缓冲区的后半段够用？*/
        if ((MAX_BUFF_SIZE - BuffBody.w_ptr) >= lenth)
        {
            /*拷贝数据进来*/
            memcpy(&BuffBody.buff[BuffBody.w_ptr], data, lenth);
            BuffBody.w_ptr += lenth;
						/*w指针值到外边界了*/
            if (BuffBody.w_ptr == MAX_BUFF_SIZE)
            {
				BuffBody.w_ptr = 0;
				/*检测套圈*/
				if(BuffBody.w_ptr == BuffBody.r_ptr )
				{
					BuffBody.ring = 1;
				}  
            }
			return ERROR_NONE;
        }
		/*缓冲区的后半段不够用*/
        else
        {
			/*缓冲区的后半段先写满*/
            lastptr = MAX_BUFF_SIZE - BuffBody.w_ptr;
            memcpy(&BuffBody.buff[BuffBody.w_ptr], data, lastptr);
            BuffBody.w_ptr = 0;
			/*缓冲区的后半段先写入*/
            memcpy(&BuffBody.buff[BuffBody.w_ptr], &data[lastptr], lenth - lastptr);
            BuffBody.w_ptr += (lenth - lastptr);
			/*检测套圈*/
            if (BuffBody.w_ptr == BuffBody.r_ptr)
            {
                BuffBody.ring = 1;
            }
			return ERROR_NONE;
        }
    }
	/*buff非空且R在W后*/
    else
    {
        memcpy(&BuffBody.buff[BuffBody.w_ptr], data, lenth);
        BuffBody.w_ptr += lenth;
		/*检测套圈*/
        if (BuffBody.w_ptr == BuffBody.r_ptr)
        {
            BuffBody.ring = 1;
        }
		return ERROR_NONE;
    }
}
/**
  * @brief  缓冲区读取函数
  *          
  * @param  data: 将要读出buff 的数据的接收地址 
*         	lenth: 要读出地址的数据长.
  * @retval 0 ：读取成功 其他：不成功					
  */
int8_t ReadBUFF(uint8_t *data, uint16_t lenth)
{
    uint16_t lastptr;
    uint16_t avail_data_lenth ;
	/*使用前未初始化*/
	if(BuffBody.buffInited != INITBUFFED)
	{
		return ERROR_NOT_INIT_BUFF;
	}
	/*操作数据不合法*/
    if ((lenth <= 0) || data == NULL)
    {
        return ERROR_DATA_ILLEGAL_OPERAT;
    }
		/*读取数据长度大于现有数据长*/
		avail_data_lenth= GetBuff_Valid_DataLenth(BuffBody.w_ptr, BuffBody.r_ptr,BuffBody.ring);
    if (avail_data_lenth < lenth)
    {
        return ERROR_OVER_FLOW_READ;
    }
	/*buff是满的*/
	if (BuffBody.ring == 1)
	{
		BuffBody.ring = 0;
	}
		/*buff非空且W指针位置在后面*/
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
	/*buff非空且r在w后面*/
    else
    {
		/*后半缓冲区够读*/
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
		/*后半缓冲区不够读*/
        else
        {
			/*先读后半缓冲区*/
            lastptr = MAX_BUFF_SIZE - BuffBody.r_ptr;
            memcpy(data, &BuffBody.buff[BuffBody.r_ptr], lastptr);
            BuffBody.r_ptr = 0;
			/*前半缓冲区读*/
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
