#ifndef __SP_FLASH_IMPL_H__
#define __SP_FLASH_IMPL_H__

#include "sp_info.h"
#include "sp_flash_store.h"

#define SP_FLAG_MASTER true
#define SP_FLAG_SLAVE false
//////////////地址///////////////////
#define ADDR_BLACKLIST 0x00000
#define ADDR_TRANSNO_MASTER 0x40000
#define ADDR_TRANSNO_SLAVE 0x40100
#define ADDR_SYSINFO 0x40200 //一页256B存放基本信息
#define ADDR_FEERATE 0x40300 //一页256b存放费率表
#define ADDR_TIMEPARA 0x40400 //一页256b存放时间段参数版本号
#define ADDR_TRUSSNO_MASTER 0x40C00 //主扎帐批次号存储区
#define ADDR_TRUSSNO_SLAVE 0x40D00 //从扎帐批次号存储区
#define ADDR_TRUSSDTL_START 0x42400 //扎帐流水存储区起始地址
#define ADDR_TRUSSDTL_END 0x46000 //扎帐流水存储区结束地址

#define ADDR_TRANSDTL_START 0x46900//流水存储区起始地址
#define ADDR_TRANSDTL_END 0xE2D00//流水存储区结束地址
#define ADDR_HEX_FILE 0x10000//存放升级二进制文件地址
///////系统参数/////
#define OFFSET_WORK_MODE 0   //1byte,工作模式0：编号，1：定值，2：单价，
#define OFFSET_CONSUME_AMT 1  //4byte,消费金额 ，在定值模式下有效
#define OFFSET_CONSUME_GAP_SECONDS 5 //3byte,两次消费间隔秒,0为不限制， 低位在前
#define OFFSET_MAX_CARD_BAL 8 // 4byte, 最大卡余额
#define OFFSET_MAX_CONSUME_AMT 12 // 4byte, 最大消费金额， 0为不限制，低位在前
#define OFFSET_RESTART_TIME 16 // 8byte,定时重新启动（4段，每个时间段两个字节），
//FF表示结束，
//时间为HEX格式例如：13：30 表示为 0x0D1
#define OFFSET_RETURN_FLAG 24 // 1byte,;//退款功能开启与关闭
#define OFFSET_OFFLINE_FLAG 25 // 1byte,;//脱机消费时间限制
#define OFFSET_MIN_CARD_BAL 26 // 2byte, //最小卡余额
#define OFFSET_TIMEOUT 28 // 1byte ，超时时间
#define OFFSET_HEART_GAP 29 // 1byte,心跳间隔
#define OFFSET_SINGLE_CONSUME_LIMIT 30 // 4byte,单笔消费限额
#define OFFSET_DAY_SUM_COMSUME_LIMIT 34 // 4byte, /日累计消费限额
#define OFFSET_CARD_LIMIT_FLAG 38 // 1byte ,/卡限开关
#define OFFSET_TERMNO 39 // 4byte,本机设备号
#define OFFSET_HD_VERSION 43 // 5byte, 硬件版本号
#define OFFSET_SYSTEM_CAPACITY 48 // 4byte, 系统容量
#define OFFSET_ADDR_SAMNO 52 //6byte, SAM卡号
#define OFFSET_ADDR_KEY_INDEX 58//1byte, 消费密钥版本号


/*
*  流水相关
*/
uint8 sp_write_transno_unit(bool flag, sp_transno_unit* punit);
uint8 sp_read_transno_unit(bool flag, sp_transno_unit* punit);
uint32 sp_get_transno(void);
int32 sp_get_last_transaddr(void);
uint16 sp_get_transno_lastaddr(uint16 pageno);
uint32 sp_get_next_transdtl_addr(uint32 transdtl_addr);
/*
*读写流水
*/
uint8 sp_read_transdtl(sp_transdtl* ptransdtl, uint32 trans_addr);
uint8 sp_write_transdtl(sp_context* ctx, sp_transdtl* ptransdtl);
uint8 sp_update_transdtl_info(uint32 page_addr, sp_transdtl* ptransdtl);


/*
*扎帐批次号读写
*/
uint32 get_next_trussdtl_addr(uint32 trussdtl_addr);
uint8 sp_get_truss_index_by_hour(sp_context* ctx, uint8 hour);
void sp_get_trussno_unit(sp_trussno_unit* punit);
//transtime ----yymmddhhmmss
void sp_generate_trussno_by_transdatetime(sp_context* ctx, const uint8 transtime[6], uint8 trussno[5]);

uint8 sp_write_trussno_unit(bool flag, sp_trussno_unit* punit);
uint8 sp_read_trussno_unit(bool flag, sp_trussno_unit* punit);


/*
*读写扎帐流水
*/
uint8 sp_read_trussdtl(sp_trussdtl* trussdtl, uint32 truss_addr);
uint8 sp_write_trussdtl(sp_context* ctx, sp_trussdtl* trussdtl);


/*
*读写黑名单
*/
uint8 sp_read_blacklist(sp_context* ctx);
uint8 sp_write_blacklist(sp_context* ctx, uint16 offset_addr);


/*
*读写系统信息
*/
uint8 sp_write_syspara(sp_context* ctx);
uint8 sp_read_syspara(sp_context* ctx);


/*
*读写费率表
*/
uint8 sp_write_feerate_table(sp_context* ctx);
uint8 sp_read_feerate_table(sp_context* ctx);

/*
*读写时间段参数表
*/
uint8 sp_write_timepara_table(sp_context* ctx);
uint8 sp_read_timepara_table(sp_context* ctx);




/*
*其他
*/
void sp_disp_flash(int32 addr, int32 len);
/*
*  作用:根据卡号设置黑名单
*/
uint8 sp_set_blacklist_by_cardno(sp_context* ctx);


#endif


