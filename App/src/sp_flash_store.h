#ifndef __FLASH_STORE_H__
#define __FLASH_STORE_H__
#include "sp_info.h"
//#include "sp_pubfunc.h"
#include "sp_disp.h"





/////////////////////////////封装汇多提供的接口////////////////////////////////////
uint8 sp_flash_read(uint32 page_no, uint32 offset_addr, uint8 *array, uint32 counter);

/**************************************************************
//参数uIsErase:
   1-----先擦除原来的数据再写，肯定可以写上
   0-----直接写，有可能有的地方写不上
//返回值:1表示成功
****************************************************************/
uint8 sp_flash_write(uint32 page_no, uint32 offset_addr, uint8  *array, uint32 counter, uint8 uIsErase);

uint8 sp_flashpage_erase(uint32 page_no);







#endif
