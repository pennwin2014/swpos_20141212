#ifndef __SP_FLASH_IMPL_H__
#define __SP_FLASH_IMPL_H__

#include "sp_info.h"
#include "sp_flash_store.h"

#define SP_FLAG_MASTER true
#define SP_FLAG_SLAVE false
//////////////��ַ///////////////////
#define ADDR_BLACKLIST 0x00000
#define ADDR_TRANSNO_MASTER 0x40000
#define ADDR_TRANSNO_SLAVE 0x40100
#define ADDR_SYSINFO 0x40200 //һҳ256B��Ż�����Ϣ
#define ADDR_FEERATE 0x40300 //һҳ256b��ŷ��ʱ�
#define ADDR_TIMEPARA 0x40400 //һҳ256b���ʱ��β����汾��
#define ADDR_TRUSSNO_MASTER 0x40C00 //���������κŴ洢��
#define ADDR_TRUSSNO_SLAVE 0x40D00 //���������κŴ洢��
#define ADDR_TRUSSDTL_START 0x42400 //������ˮ�洢����ʼ��ַ
#define ADDR_TRUSSDTL_END 0x46000 //������ˮ�洢��������ַ

#define ADDR_TRANSDTL_START 0x46900//��ˮ�洢����ʼ��ַ
#define ADDR_TRANSDTL_END 0xE2D00//��ˮ�洢��������ַ
#define ADDR_HEX_FILE 0x10000//��������������ļ���ַ
///////ϵͳ����/////
#define OFFSET_WORK_MODE 0   //1byte,����ģʽ0����ţ�1����ֵ��2�����ۣ�
#define OFFSET_CONSUME_AMT 1  //4byte,���ѽ�� ���ڶ�ֵģʽ����Ч
#define OFFSET_CONSUME_GAP_SECONDS 5 //3byte,�������Ѽ����,0Ϊ�����ƣ� ��λ��ǰ
#define OFFSET_MAX_CARD_BAL 8 // 4byte, ������
#define OFFSET_MAX_CONSUME_AMT 12 // 4byte, ������ѽ� 0Ϊ�����ƣ���λ��ǰ
#define OFFSET_RESTART_TIME 16 // 8byte,��ʱ����������4�Σ�ÿ��ʱ��������ֽڣ���
//FF��ʾ������
//ʱ��ΪHEX��ʽ���磺13��30 ��ʾΪ 0x0D1
#define OFFSET_RETURN_FLAG 24 // 1byte,;//�˿�ܿ�����ر�
#define OFFSET_OFFLINE_FLAG 25 // 1byte,;//�ѻ�����ʱ������
#define OFFSET_MIN_CARD_BAL 26 // 2byte, //��С�����
#define OFFSET_TIMEOUT 28 // 1byte ����ʱʱ��
#define OFFSET_HEART_GAP 29 // 1byte,�������
#define OFFSET_SINGLE_CONSUME_LIMIT 30 // 4byte,���������޶�
#define OFFSET_DAY_SUM_COMSUME_LIMIT 34 // 4byte, /���ۼ������޶�
#define OFFSET_CARD_LIMIT_FLAG 38 // 1byte ,/���޿���
#define OFFSET_TERMNO 39 // 4byte,�����豸��
#define OFFSET_HD_VERSION 43 // 5byte, Ӳ���汾��
#define OFFSET_SYSTEM_CAPACITY 48 // 4byte, ϵͳ����
#define OFFSET_ADDR_SAMNO 52 //6byte, SAM����
#define OFFSET_ADDR_KEY_INDEX 58//1byte, ������Կ�汾��


/*
*  ��ˮ���
*/
uint8 sp_write_transno_unit(bool flag, sp_transno_unit* punit);
uint8 sp_read_transno_unit(bool flag, sp_transno_unit* punit);
uint32 sp_get_transno(void);
int32 sp_get_last_transaddr(void);
uint16 sp_get_transno_lastaddr(uint16 pageno);
uint32 sp_get_next_transdtl_addr(uint32 transdtl_addr);
/*
*��д��ˮ
*/
uint8 sp_read_transdtl(sp_transdtl* ptransdtl, uint32 trans_addr);
uint8 sp_write_transdtl(sp_context* ctx, sp_transdtl* ptransdtl);
uint8 sp_update_transdtl_info(uint32 page_addr, sp_transdtl* ptransdtl);


/*
*�������κŶ�д
*/
uint32 get_next_trussdtl_addr(uint32 trussdtl_addr);
uint8 sp_get_truss_index_by_hour(sp_context* ctx, uint8 hour);
void sp_get_trussno_unit(sp_trussno_unit* punit);
//transtime ----yymmddhhmmss
void sp_generate_trussno_by_transdatetime(sp_context* ctx, const uint8 transtime[6], uint8 trussno[5]);

uint8 sp_write_trussno_unit(bool flag, sp_trussno_unit* punit);
uint8 sp_read_trussno_unit(bool flag, sp_trussno_unit* punit);


/*
*��д������ˮ
*/
uint8 sp_read_trussdtl(sp_trussdtl* trussdtl, uint32 truss_addr);
uint8 sp_write_trussdtl(sp_context* ctx, sp_trussdtl* trussdtl);


/*
*��д������
*/
uint8 sp_read_blacklist(sp_context* ctx);
uint8 sp_write_blacklist(sp_context* ctx, uint16 offset_addr);


/*
*��дϵͳ��Ϣ
*/
uint8 sp_write_syspara(sp_context* ctx);
uint8 sp_read_syspara(sp_context* ctx);


/*
*��д���ʱ�
*/
uint8 sp_write_feerate_table(sp_context* ctx);
uint8 sp_read_feerate_table(sp_context* ctx);

/*
*��дʱ��β�����
*/
uint8 sp_write_timepara_table(sp_context* ctx);
uint8 sp_read_timepara_table(sp_context* ctx);




/*
*����
*/
void sp_disp_flash(int32 addr, int32 len);
/*
*  ����:���ݿ������ú�����
*/
uint8 sp_set_blacklist_by_cardno(sp_context* ctx);


#endif


