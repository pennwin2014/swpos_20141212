#ifndef __SP_SYSCONTROL_H__
#define __SP_SYSCONTROL_H__
#include "sp_info.h"
#include "sp_consume.h"

#define IS_DATE_INCREASE(x, y) (memcmp(x, y, 4)>0?1:0)

/*******************************************************
*** ������:   sp_pre_check
*** ��������: ����ǰ�ļ������
*** ����:  ȫ�ֽṹ��ָ��
*** ����ֵ:  0--�ɹ�  1--ʧ��
*********************************************************/
uint8 sp_pre_check(sp_context* ctx);


/*
*  ����:��鲢��ȡϵͳ����
*/
uint8 sp_read_system(sp_context* ctx);

/*
*  ����:�ָ���������
*  1������һЩĬ��ֵ
*  2����ǿ��Ҫ���û������ն˱��
*/
uint8 sp_recover_device(sp_context* ctx);


/*
*  ����:���ϵͳʱ���Ƿ�仯
*/
int sp_check_date_change(sp_context* ctx);


/*
*  ����:��ȡ�ѻ���ˮ����
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


