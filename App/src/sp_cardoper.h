#ifndef _SP_CARDOPER_H_
#define _SP_CARDOPER_H_


//#include <assert.h>
//#include "config.h"
//#include "rfmod.h"
//#include "helper.h"
//#include "gnudes.h"
#include "sp_info.h"
#include "sp_pubfunc.h"
#include "sp_disp.h"

/**
* \brief request card
*/
extern int sp_request_card(uint8 cardphyid[4]);
extern int sp_request_card_poweron(sp_card* card);

/**
* \brief - read card info, read card base information , eg . cardno,
*          feetype, last trans record
*/
	
int sp_select_card_app(sp_context* ctx);

int sp_read_card(sp_context*  ctx, sp_card* card, uint8 read_level);
/**
* \brief - init for purchase
* \return -  0 : success , -1 : error, 1 : short of balance
*/
int sp_payinit(sp_context* ctx, sp_card* card, uint32 money);
/**
* \brief - debit for payment
*/
int sp_payment(sp_context* ctx, sp_card* card, uint32 money);
int sp_halt(sp_context* ctx, sp_card* card);
/**
*�жϽ����Ƿ�ɹ�

* @param  card
* @param  paycnt �жϵĽ��״���
* @return        ���� 0 ��ʾ�ý��׳ɹ� ���� card->tac �б��� tac, 1 ��ʾ�ý��ײ��ɹ�
*                   ���� -1 ��ʾ��������
*/
int sp_get_prove(sp_context* ctx, sp_card* card, uint16 paycnt);
int sp_set_card_loss(sp_context* ctx, sp_card* card);


/**
*��ʼ��psam��
*/
int	sp_init_sam_card(sp_context* ctx, uint8 samno[6]);
uint8  sp_rst_psam_card(uint8 *cData, uint8 *cLen);
int sp_enable_sam_card(sp_context* ctx);

/**
*  д��
*/
uint8 sp_write_card(sp_card* card, sp_context* ctx, uint8 write_level);



#endif

