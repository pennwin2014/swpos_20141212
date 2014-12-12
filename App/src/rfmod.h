#ifndef _SP_RFMOD_H_
#define _SP_RFMOD_H_
/**
*
*   ����: ��Ƶģ��
*
**/
#include "config.h"
#include "sp_errdef.h"
//#include "sp_cardoper.h"
//#include "sp_info.h"
//#include "sp_disp.h"

typedef struct
{
  uint8 send_len;
  // �ǽӴ����ײ�ָ���ֵ����㲻��ʹ��
  uint8 send_dev_instruction;
  uint8 send_buf[256];
  uint8 recv_len;
  // �ǽӴ����ײ�ָ���ֵ����㲻��ʹ��
  uint8 recv_dev_instruction;
  uint8 recv_buf[256];
  uint16 sw;
} adpu_cmd_t;

////////////////////////////////////���ײ㺯����һ��////////////////////////////////////
/*
*  ��ȡ��Ƭ��uid
*/
extern int sp_read_uid(uint8* uid);

/*
*  ��λ�ǽ�cpu��
*/
extern int sp_cpu_reset_card(void);

/*
*  ȡ����Ƭѡ��
*/
extern int sp_cpu_deselect_card(void);

/*
*  ѡ��df03
*/
extern int sp_cpu_select_df03(void);


/*
*  ��Ƭ����(��Ѱ��\����ͻ\ѡ��\������֤�Ȳ���)
*  Block_Adr :���ַ
*  hKey: KEYA(0x10) �� KEYB(0x20)��֤
*  cKey:����ֵ����
*/
extern uint8 sp_hl_active(uint8 Block_Adr, uint8 hKey, uint8 const *cKey);


/*
*******************************************************************************
** �������� ����������
** ��ڲ��� ��Adr:���
** ���ڲ��� ��buff:����������
*******************************************************************************
*/
extern uint8 sp_m1_readblock(uint8 *buff, uint8 Block_Adr);

/*
** �������� ����������(�Ѻ������)
** ��ڲ��� ��cBlock : �������ݣ�
: hKey ����������֤��ʽ��KEYA_MODE ��ʾ Keya,
KEYB_MODE ��ʾ Keyb
: cKey �˿��Ӧ��������Կ
: pData �����Ŀ�����
: cTry ����ʧ�ܺ��ظ�����
*******************************************************************************
*/
extern bool sp_m1_readcard(uint8 cBlock, uint8 hKey, uint8 const *cKey, uint8 *pData, uint8 cTry);

/*
****************************************************
����: request  card
���: mode    ����ģʽ REQUEST_STD ��ʾ��׼Ѱ��  REQUEST_ALL��ʾѰ���п�
����: ����ֵ0:�ɹ�����0:ʧ��
˵��:ĿǰREQUEST_ALL����������û�зſ�����ʱ���ܸ�sp_read_uid�ص�
*****************************************************
*/
extern uint8 sp_request(uint8 mode);



extern int sp_cpu_select_df03(void);


extern int sp_cpu_adpu_command(adpu_cmd_t* cmd);

extern int sp_sam_adpu_command(adpu_cmd_t* cmd);

extern int sp_cpucard_poweron(void);

extern int sp_cpucard_deselect(void);

////////////////////////////////////////////////////////////////////////////////////




#endif // _SP_RFMOD_H_


