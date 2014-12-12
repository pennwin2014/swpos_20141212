#include "sp_flash_store.h"




/////////////////////////////////////��װ����ṩ�Ľӿ�/////////////////////////////////////////////////////
uint8 sp_flash_read(uint32 page_no, uint32 offset_addr, uint8 *array, uint32 counter)
{
  return (SF_Read(page_no, offset_addr, array, counter) == 1) ? 0 : 1;
}
//uIsErase 0---ֱ��д��1---��������д
uint8 sp_flash_write(uint32 page_no, uint32 offset_addr, uint8  *array, uint32 counter, uint8 uIsErase)
{
  return (SF_Write(page_no, offset_addr, array, counter, uIsErase) == 1) ? 0 : 1;
}

uint8 sp_flashpage_erase(uint32 page_no)
{
  return (SF_ErasePage(page_no) == 1) ? 0 : 1;
}











