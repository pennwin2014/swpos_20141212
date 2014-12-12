#ifndef __SP_SYSCONTROL_H__
#define __SP_SYSCONTROL_H__
#include "sp_info.h"
#include "sp_consume.h"

#define IS_DATE_INCREASE(x, y) (memcmp(x, y, 4)>0?1:0)

/*******************************************************
*** 函数名:   sp_pre_check
*** 函数功能: 消费前的检查流程
*** 参数:  全局结构体指针
*** 返回值:  0--成功  1--失败
*********************************************************/
uint8 sp_pre_check(sp_context* ctx);


/*
*  作用:检查并读取系统参数
*/
uint8 sp_read_system(sp_context* ctx);

/*
*  作用:恢复出厂设置
*  1、填上一些默认值
*  2、并强制要求用户输入终端编号
*/
uint8 sp_recover_device(sp_context* ctx);


/*
*  作用:检查系统时间是否变化
*/
int sp_check_date_change(sp_context* ctx);


/*
*  作用:获取脱机流水数量
*/
uint16 sp_get_offline_transdtl_amout(sp_context* ctx);



int sp_input_termno(sp_context* ctx);

int sp_input_canid(sp_context* ctx);

int sp_consume_normal(sp_context * ctx);

uint8 sp_consume_fixed_value(sp_context* ctx);

void sp_heartbeat(sp_context* ctx);

int sp_enable_device(sp_context* ctx);

int sp_check_deviceid(sp_context* ctx);

uint8 sp_recover_system(sp_context* ctx);

int sp_check_and_enable_device(sp_context* ctx);


#endif


