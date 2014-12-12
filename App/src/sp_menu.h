#ifndef __MENU_H__
#define __MENU_H__
#include "sp_syscontrol.h"



typedef void (* menu_func_t)(void* arg);
typedef struct
{
  const char* menu_desc;//菜单名称
  menu_func_t menu_func;//菜单函数
} menu_info_t;

////////////////////菜单处理函数////////////////////////////
void sp_menu_consume(sp_context* ctx);

//////////////////////////////////////



#endif
