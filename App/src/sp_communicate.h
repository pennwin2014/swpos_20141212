#ifndef __SP_CAN_H__
#define __SP_CAN_H__

#include "sp_info.h"
#include "sp_pubfunc.h"
#include "sp_disp.h"
#include "sp_flash_impl.h"
//#include "sp_syscontrol.h"
//#include "sp_cardoper.h"


#define SP_CMD_TEST 0x00 //测试
#define SP_CMD_RT_TRANSDTL 0xD2 //流水
#define SP_CMD_AUTH 0xD3//签到
#define SP_CMD_GET_BLACKLIST 0xDA//获取黑名单
#define SP_CMD_HEARTBEAT 0xD5//心跳
#define SP_CMD_GET_FEERATE 0xDC//获取费率参数
#define SP_CMD_GET_SYSPARA 0x06//获取系统参数
#define SP_CMD_GET_TIMEPARA 0x07//获取时间段参数
#define SP_CMD_ADD_BLACKLIST 0x08//更新一条黑名单
#define SP_CMD_TRUSSDTL 0x09 //扎帐流水




#define PROTOCOL_FUNC_PREPARE(x) static int x (sp_context* ctx, uint8* send_buffer, uint16 buffer_max_len, uint16* data_len, void* arg)
#define PROTOCOL_FUNC_RESP(x) static int x(sp_context* ctx, uint8* buffer, uint16 data_len)


#ifdef SP_GLOBALS
#define SP_EXT
#else
#define SP_EXT extern
#endif

#pragma pack(push) //保存对齐状态
#pragma pack(1)
typedef struct
{
  uint8 cmd;
  int (* prepare_req_data)(sp_context* , uint8* , uint16 , uint16* , void*);
  int (* resp_process_handler)(sp_context*, uint8* , uint16);
} protocol_process_handler, *pprotocol_process_handler;

typedef struct
{
  //接收缓冲区长度
  uint16 recv_len;
  //接收缓冲区
  uint8* recv_buffer;//[TCP_RECV_BUFFER_SIZE];
  //当前网络请求流水号
  uint32 current_tcp_seqno;
	//失效时间=当前时间+ 超时时间
	uint32 expire_tick;
} sp_com_buffer_handler;


typedef struct
{
	uint8 retcode;
	uint32 transno;
}
sp_recv_rt_transdtl;

typedef struct
{
	uint8 retcode;
	uint8 trussno[5];
}
sp_recv_trussdtl;

#pragma pack(pop)

/////////////////////发送网络请求函数/////////////////////////////
void sp_init_com(void);

int sp_send_request(sp_context* ctx, uint8 cmd, void* arg, uint16 time_out);

int sp_prcocess_message(sp_context* ctx, uint8 wait_flag);


#endif
