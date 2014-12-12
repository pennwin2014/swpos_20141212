#ifndef __SP_CONSUME_H__
#define __SP_CONSUME_H__

#include "sp_info.h"
#include "sp_cardoper.h"
#include "sp_transfer.h"
#include "sp_disp.h"
#include "sp_flash_impl.h"

#define CONSUME_SUCCESS_BEEP 1
#define CONSUME_FAIL_BEEP 3

/*******************************************************
*** ������:   sp_consume_loop
*** ��������: ��������
*** ����:  ȫ�ֽṹ��ָ��
*** ����ֵ:  0--�ɹ�  1--ʧ��
*********************************************************/
int sp_consume_loop(sp_context* ctx);









/*******************************************************
*** ������:   sp_check_record_rom
*** ��������:�����ˮ�洢���Ƿ����㹻�Ŀռ�
*** ����:  ȫ�ֽṹ��ָ��
*** ����ֵ:  0--�ɹ�  1--ʧ��
*********************************************************/
void sp_check_record_rom(sp_context* ctx);






#endif
