#ifndef __SP_INFO_H
#define __SP_INFO_H
/*
**��������һЩ���õ�ͷ�ļ���ʵ�ֹ����йص�ͷ�ļ�����������
*/
#include "math.h"
#include "string.h"
#include "stdio.h"
#include "rfmod.h"
#include "config.h"
#include "M25PE16.h"
#include "sp_errdef.h"

//#define SP_DEBUG 0
//#define SP_DEBUG_MODE 0

typedef enum
{
  CPUCARD  = 0,
  M1CARD,
  SIMCARD,
  UNKNOWN_CARD
}
user_cardtype_t;
#define TRANSFLAG_UNCONFIRM 0x82
#define TRANSFLAG_LOSTCARD 0x13
/*
*  ����
*/
//���������ϴ��ɹ�
#define TRANSFLAG_OK_NO_PASSWD 0x89//��ͨ������
#define TRANSFLAG_OK_PASSWD 0x8D//��ͨ������
#define TRANSFLAG_OK_BOARD_NO_PASSWD 0xC9//������������
#define TRANSFLAG_OK_BOARD_PASSWD 0xCD//������������
#define TRANSFLAG_OK_DISCOUNT_NO_PASSWD 0xA9//���ۿ�������
#define TRANSFLAG_OK_DISCOUNT_PASSWD 0xAD//���ۿ�������
//��;�ο�
#define TRANSFLAG_HALFFWAY_NO_PASSWD 0x82//��;�ο�������
#define TRANSFLAG_HALFFWAY_PASSWD 0x86//��;�ο�������
#define TRANSFLAG_HALFFWAY_BOARD_NO_PASSWD 0xC2//��������;�ο�������
#define TRANSFLAG_HALFFWAY_BOARD_PASSWD 0xC6//��������;�ο�������
#define TRANSFLAG_HALFFWAY_DISCOUNT_NO_PASSWD 0xA2//���ۿ���;�ο�������
#define TRANSFLAG_HALFFWAY_DISCOUNT_PASSWD 0xA6//���ۿ���;�ο�������
//�ڿ�����
#define TRANSFLAG_BLACKCARD 0x10//00010000



#define CARD_SUPPORT_CPU (1<<7)//�൱�ڶ�������10000000
#define CARD_SUPPORT_M1 (1<<6)//�൱�ڶ�������1000000
#define CARD_SUPPORT_RFUIM (1<<5)//�൱�ڶ�������100000




#define MAX_FLASH_WRITE_CNT 20
#define TIMEOUT_CNT 200
#define TCP_WAIT_SLEEP 10


#define MAX_MENU_SCREEN_CNT 3  //��Ļһ�ο�����ʾ�Ĳ˵���Ŀ
//#define MAX_SCREEN_WORDS (16*3)
#define MAX_SCREEN_WORDS (16*4)
#define MAX_SCREEN_ROWS 16
#define SYSTEM_PASSWORD_LEN 6

//����ģʽ
#define SP_WORK_MODE_NORMAL 1
#define SP_WORK_MODE_FIXED_VALUE 2
//���¶�����̰���
#define SP_KEY_0 '0'
#define SP_KEY_1 '1'
#define SP_KEY_2 '2'
#define SP_KEY_3 '3'
#define SP_KEY_4 '4'
#define SP_KEY_5 '5'
#define SP_KEY_6 '6'
#define SP_KEY_7 '7'
#define SP_KEY_8 '8'
#define SP_KEY_9 '9'
#define SP_KEY_ADD '+'//0x0b
#define SP_KEY_MINUS '-'//0x0d
#define SP_KEY_MUL '*'
#define SP_KEY_DOT '.'
#define SP_KEY_EQUAL 0x0d
#define SP_KEY_CONFIRM 0xb5
#define SP_KEY_CLEAR 0xb6
#define SP_KEY_MODE 0xb3
#define SP_KEY_FUNC 0xb4
//������Ϊ�Ϸ��·��İ�ť
#define SP_KEY_NEXT SP_KEY_ADD
#define SP_KEY_PREV SP_KEY_MINUS

//��״̬
#define CARD_STATUS_NORMAL 0//����
#define CARD_STATUS_LOCK 1//����
#define CARD_STATUS_FREEZE 2//����
#define CARD_STATUS_LOST 3 //��ʧ
#define CARD_STATUS_LOGOUT 4 //ע��
#define CARD_STATUS_EXPIRE 5 // ����



/////////////////////////////////////////
#define SP_PROCESS_CARD_FIRST 0x01//�ȷſ�
#define SP_PROCESS_INPUT_FIRST 0x02//������


#define SP_FILE10 0x01//00000001B
#define SP_FILE12 0x02//00000010B
#define SP_FILE15 0x04//00000100B
#define SP_FILE16 0x08//00001000B
#define SP_FILE18 0x10//00010000B
#define SP_FILE19 0x20//00100000B
#define SP_READ_CARDBAL 0x40//01000000B

/////////////////////////////////////////
#define SP_STATUS_RESET_TIME 1//(1<<0) //00000001B
/*
#define SP_STATUS_RESET_TIME (1<<1) //00000010B
#define SP_STATUS_RESET_TIME (1<<2) //00000100B
#define SP_STATUS_RESET_TIME (1<<3) //00001000B
#define SP_STATUS_RESET_TIME (1<<4) //00010000B
#define SP_STATUS_RESET_TIME (1<<5) //00100000B
#define SP_STATUS_RESET_TIME (1<<6) //01000000B
#define SP_STATUS_RESET_TIME (1<<7) //10000000B
*/



#define SP_TRUSS_PERIOD0 "\x00\x00"


///////////////��С/////////////////////
#define SHOW_SYSTEM_INFO_COUNT 9

#define FLASH_PAGE_SIZE 256
#define MAX_SENDMSG_LEN 512 //����ͳ���
#define MAX_DATA_LEN 203
#define TCP_RECV_BUFFER_SIZE 1024 //  ���ջ�������С

#define SP_SWITCH_ON 1
#define SP_SWITCH_OFF 0

#define SP_TP_ADD 1
#define SP_TP_REDUCE 0

#define SAMSLOT 1
#define SAMBAND 9600
#define SP_SAM_AID ((uint16)0xDF03)

/////////////�ṹ��/////////////////

#pragma pack(push) //�������״̬
#pragma pack(1)

typedef struct
{
  uint32 last_trans_no;//���һ����ˮ��
  uint32 last_trans_addr;//���һ����ˮ��ַ
  uint8 date[3];//����yymmdd
  uint32 sum_amt;//�����ۼƽ��
  uint8 rev[3];//����
  uint8 crc[2];//crc
} sp_transno_unit;

typedef struct
{
  uint32 termseqno;//4pos���ļ�¼��ˮ��
  uint32 cardno;//4�û���ˮ�ţ������׿��ţ�����λ��ǰ��
  uint16 last_cnt;// 2 ��һ�ν��״���
  uint8 last_limit_amt[3];//��һ�ʵ�͸���޶�
  uint32 last_amt;//��һ�ʵĽ��׽��
  uint8 last_transflag;//��һ�ʵĽ��ױ�־
  uint8 last_termno[6];//��һ�ʵ��ն˱��
  uint8 last_datetime[6];//��һ�ʵ�����ʱ��yymmddhhmmss��bcd��
  uint32 cardbefbal;//4����ǰ������λ��ǰ��
  uint16 cardbefcnt;//2����ǰ����������λ��ǰ��
  uint32 amount;//4��ʵ���׽���λ��ǰ���������ۿۻ���ѽ��
  uint8 extra_amt[3];//3���ѻ��ۿ۽���λ��ǰ��
  uint8 transdatetime[6];//6��������ʱ�� YYMMDDHHMMSS
  uint8 samno[6];//6PSAM����
  uint8 tac[4];//4TAC��
  uint8 transflag;//1���ױ�־
  uint8 reserve[2];//2����
  uint8 crc[2];//2CRCУ��
} sp_transdtl;

typedef struct
{
  uint8 last_truss_no[5];//���һ���������κ�
  uint32 last_truss_addr;//���һ��������ˮ��ַ
  uint8 rev[5];//����
  uint8 crc[2];//crc
} sp_trussno_unit;


typedef struct
{
  uint8 truss_no[5];//�������κ�20141209+01
  uint8 last_trussno[5];//��һ���������κ�
  uint32 success_count;//�ɹ�����
  uint32 success_amount;//�ɹ����
  uint32 fail_count;//ʧ�ܱ���
  uint32 fail_amount;//ʧ�ܽ��
  uint32 start_termseqno;//��ʼ��ˮ��
  uint32 end_termseqno;//��ֹ��ˮ��
  uint8 end_truss_time[3];//���һ��ʱ��
  uint32 end_transaddr;//���һ����ˮ��ַ
  uint8 reserve[21];//����
  uint8 crc[2];
} sp_trussdtl;

typedef struct
{
  //uint8 reset_helper;
  /*
  * �������õĲ���
  */
  uint8 termno[4];//�ն˱��
  uint32 canid;//֡id���豸id
  /*
  * ���ص�ͳ��
  */
  uint8 enable_staus;//�Ƿ����õı�־���ߺ�̨���صĴ�����
  uint8 init_flag;//�Ƿ��ʼ����־
  uint32 today_total_amt;  // ���������ܶ�
  uint32 today_total_cnt;  //�������ѱ���
  uint32 yesterday_total_amt;  // �������ѽ��
  uint32 yesterday_total_cnt;  //�������ѱ���
  uint8 samno[6];//sam����
  uint8 today_date[4];  //��������
  uint16 offline_days;//�ѻ���ʹ������
  /*
      ��Ҫ�ӷ�������ȡ�Ĳ���
  */
  uint8 work_mode;//����ģʽ0����ţ�1����ֵ��2�����ۣ�
  uint32 max_cardbal;//������
  uint8 return_flag;//�˿�ܿ�����ر�
  uint8 offline_flag;//�ѻ�����ʱ������
  uint16 min_cardbal;//��С�����
  uint8 oper_timeout;//������ʱʱ�䣬��λ:��
  uint8 heartgap;//�������
  uint8 limit_switch;//���޿��أ��Ƿ���Ҫ�ж������޶�
  uint8 key_index;  //������Կ�汾��
  uint8 password[6];  //��������
  uint32 max_pay_cnt;  //�����ʹ�ô���
  uint16 max_offline_days; //pos���ѻ�ʹ���������
  uint8 hd_verno[5];//Ӳ���汾
  uint8 soft_verno[3];//����汾
  uint32 timepara_verno;//ʱ��β����汾��
  uint32 feepara_verno;//�շѲ����汾��
  uint32 syspara_verno;//�������汾��
  uint8 blacklist_verno[7]; // ��ǰ�������汾
  uint8 feetype_bitmap[32];  // ����ʹ�ÿ����1-255��
  uint32 once_limit_amt;//���������޶�
  uint32 day_sum_limit_amt;//���ۼ������޶�
  /*
  *���ʵ��ĸ����ʵ�
  */
  uint8 truss_period1[2];
  uint8 truss_period2[2];
  uint8 truss_period3[2];
//  uint8 truss_period4;
  /*
    * ��ˮͳ����Ϣ
    */
  uint32 confirmed_transaddr;
  uint32 confirmed_transno;
  uint32 total_transaddr;
  uint32 total_transno;
  /*
  *����ͳ����Ϣ
  */
  uint32 confirmed_trussaddr;
  uint8 confirmed_trussno[5];
  uint32 total_trussaddr;
  uint8 total_trussno[5];
  uint8 crc[2];  //2�ֽ�У��λ
} sp_syspara;

typedef struct
{
  uint16 aid;
  uint8 cardphyid[8];
  char username[32];
  char stuempno[20];
  uint32 cardno;
  uint8 cardtype;
  uint8 status;
  uint8 cardverno[7];
  uint8 feetype;
  uint8 expiredate[4];
  uint32 cardbefbal;
  uint16 paycnt;
  uint32 amount;
  uint32 payamt;
  uint8 keyver;
  uint8 keyindex;
  uint8 pay_random[4];
  uint8 tac[4];
  uint32 sam_seqno;
  uint32 day_sum_limit_amt;
  uint32 once_limit_amt;
  //0x18�ļ�
  uint16 last_trans_seqno;//ED��EP�������ѻ��������
  uint32 last_overdraft_limit;//��һ�ʵ�͸���޶�
  uint32 last_trans_amt;//���׽��
  uint8 last_trans_flag;//�������ͱ�־
  uint8 last_samno[6];//�ն˻����
  uint8 last_datetime[7];//��һ�ʽ�������4+��һ�ʽ���ʱ��3
  //0x10�ļ�
  uint32 day_sum_amt;//���ۼƽ��׽��
  /*
  uint8 last_trans_dev[6];
  uint8 last_trans_date[4];
  uint8 last_trans_time[3];
  uint8 last_trans_amt[4];
  uint8 last_trans_cnt[2];
  uint8 last_tac[4];
  */
  //���
  uint8 is_halfway; // �Ƿ�����;�ο�
  uint8 use_passwd_flag;//�Ƿ�ʹ�����룬0δʹ�ã�1ʹ��
  uint8 pay_confirm_flag;//�Ƿ�д��0���ɹ���1��δ�ɹ���2����;�ο�
  uint8 rate_flag;// 0������ 1���ۿ� 2������
  uint8 consume_flag;//0.������1������  2,�ڿ�
  char passwd[16];//������
  // m1 card package block data
  uint8 m1_package_block[16];
} sp_card;

#define MAX_CARNNO_BUFFER_SIZE 4*10
typedef struct
{
  uint32 feepara_verno;
  uint32 timepara_verno;
  uint8 blacklist_verno[6];
  uint32 syspara_verno;
  uint8 upd_flag;
  //�������������
  uint8 end_flag;
  uint16 download_index;
  uint16 flash_offset;
  uint16 buffer_size;
  uint8 tmp_blacklist_bitmap[2 * FLASH_PAGE_SIZE];
  //�������º�����������
  uint8 cardno_buffer[MAX_CARNNO_BUFFER_SIZE];
} sp_svrpara;

typedef struct
{
  uint8 period[4];
  uint8 use_times;
  uint8 forbidden_bitmap[32];
} sp_timepara;

typedef struct
{
  //������־
  uint8 oper_upd_flag;
  //״̬��־
  uint8 global_status;
  //��һ�ʽ��׳ɹ���������
  uint8 last_cardphyid[8];
  //��ʾ������
  uint8 disp_type;
  // ��ѧ���Ʊ���
  char school_name[17];
  //�Ƿ�����
  bool online_flag;
  // ��ǰ����ͳ������
  uint8 today[4];
  // ��ǰʱ��
  uint8 current_datetime[7];//yy-yy-mm-dd-hh-mi-ss
  // ����Կ
  uint8 card_key[16];
  // ��ǰ��ˮ flash �Ƿ��Ѿ�����
  uint8 rec_flash_full;
  //sam���Ƿ���Ч
  uint8 sam_ok;
  //��ǰsam����Ӧ��id
  uint16 sam_aid;
  // ��ǰ��ˮ
  sp_transdtl record;
  //�쳣��ˮ
  sp_transdtl exception_record;
  // ��ǰ��
  sp_card card;
  //���ص�ʱ���ݴ�ķ������ϵ�ֵ
  sp_svrpara svrpara;
  //ϵͳ����
  sp_syspara syspara;
  // ���ʱ�
  uint8 feerate_table[256];
  //ʱ��β����汾��
  sp_timepara timepara_table[4];
} sp_context;

////////////////////////////////��������ͨѶ�ṹ��////////////////////////////////////////////
typedef struct
{
  uint8 guide_code[3];//������
  uint8 cmd_code;//������
  uint16 data_len;//���ݳ���
  uint8 deviceno[2];//������ַ
  uint32 seqno;
} sp_tcp_header;

typedef struct
{
  sp_tcp_header header;
  uint8 data[MAX_DATA_LEN];
  uint8 check_sum;
} sp_tcp_trans_unit;
#pragma pack(pop)//�ָ�����״̬



#endif
