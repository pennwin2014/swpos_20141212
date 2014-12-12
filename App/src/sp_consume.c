#include "sp_consume.h"


static int is_blkcard(sp_context* ctx, uint32 cardno)
{
  uint32 page_no;
  uint8 blkbuffer[64], t;
  sp_bitmap_offset_t offset;
  uint8 page_offset = 0;
  sp_bitmap_transform(cardno, &offset);
  page_no = (ADDR_BLACKLIST + offset.byte_offset) / FLASH_PAGE_SIZE;
  page_offset = ADDR_BLACKLIST + offset.byte_offset - page_no * FLASH_PAGE_SIZE;
  sp_flash_read(page_no, page_offset, blkbuffer, 1);
  t = blkbuffer[0];//[offset.byte_offset - 0];
  return ((t & (1 << offset.bit_offset)) == 0) ? 1 : 0;
}

static int do_input_and_check_card_passwd(sp_context* ctx, sp_card* card)
{
  int ret = 0;
  char real_password[MAX_SCREEN_ROWS];
  ret = sp_input_password(ctx, "请输入卡密码:", real_password, 6);
  if(ret)
    return ret;
  if(memcmp(real_password, card->passwd, strlen(card->passwd)) != 0)
  {
    sp_disp_error("%02x%02x%02x%02x%02x%02x", real_password[0], real_password[1], real_password[2], real_password[3], real_password[4], real_password[5]);
    return SP_E_CARD_PASSWD;
  }
  return 0;
}

static void do_check_card_consume_date(sp_context* ctx, sp_card* card)
{
  //如果日期更换，则重置上一次交易日期
  //sp_disp_debug("lastdate=[%02x%02x%02x%02x]", the_card->last_trans_date[0], the_card->last_trans_date[1], the_card->last_trans_date[2], the_card->last_trans_date[3]);
  if(memcmp(card->last_datetime, ctx->today, 4) != 0)
  {
    memcpy(card->last_datetime, ctx->today, 4);
    card->day_sum_amt = 0;
    sp_disp_debug("today=[%02x%02x%02x%02x],reset date", ctx->today[0], ctx->today[1], ctx->today[2], ctx->today[3]);
  }
}

static void do_update_card_day_sum_amt(sp_context* ctx, sp_card* card)
{
  do_check_card_consume_date(ctx, card);
  card->day_sum_amt += card->payamt;
}

static int do_check_consume_limit(sp_context * ctx, sp_card* card)
{
  int ret = 0;
  uint8 cancel_flag = 0;
  card->use_passwd_flag = 0;
  if(ctx->syspara.limit_switch == SP_SWITCH_OFF)
    return 0;
  //卡上
  if(card->once_limit_amt > 0 && card->payamt >= card->once_limit_amt)
  {
    card->use_passwd_flag = 1;
  }
  else if(card->day_sum_limit_amt > 0 && (card->day_sum_amt + card->payamt) >= card->day_sum_limit_amt)
  {
    card->use_passwd_flag = 1;
  }
  //以卡为准，卡上没则按pos机来
  if(card->use_passwd_flag == 0)
  {
    //pos机上
    if(ctx->syspara.once_limit_amt > 0 && card->payamt >= ctx->syspara.once_limit_amt)
    {
      card->use_passwd_flag = 1;
    }
    else if(ctx->syspara.day_sum_limit_amt > 0 && (card->day_sum_amt + card->payamt) >= ctx->syspara.day_sum_limit_amt)
    {
      card->use_passwd_flag = 1;
    }
  }
  if(card->use_passwd_flag)
  {
    // 检查密码
    ret = sp_read_card(ctx, card, SP_FILE19);
    if(ret)
      return ret;
    while(1)
    {
      ret = do_input_and_check_card_passwd(ctx, card);
      if(SP_SUCCESS == ret)
        return SP_SUCCESS;
      else if(SP_R_CARD_PASSWD_CANCEL == ret)
      {
        sp_disp_by_type(SP_TP_IS_CANCEL_CONSUME, ctx, &cancel_flag);
        if(cancel_flag == SP_SWITCH_OFF)
        {
          continue;
        }
        else
          return ret;
      }
      else if(SP_E_CARD_PASSWD == ret)
      {
        sp_beep(1);
        sp_disp_by_type(SP_TP_PASSWORD_ERROR, ctx, NULL);
        continue;
      }
      return ret;
    }

  }
  return 0;
}

static int do_check_blkcard(sp_context * ctx, sp_card* card)
{
  if(memcmp(card->cardverno, ctx->syspara.blacklist_verno, 7) > 0)
  {
    return 0;
  }
  return is_blkcard(ctx, card->cardno);
}

static uint8 get_transflag_by_card(sp_card* card)
{
  uint8 transflag = 0;
  //前四位
  if(card->consume_flag == 2)//黑卡消费
  {
    sp_clear_bit(&transflag, 1);
    sp_clear_bit(&transflag, 2);
    sp_clear_bit(&transflag, 3);
    sp_set_bit(&transflag, 4);
  }
  else
  {
    if(card->consume_flag == 1)//冲正流水
    {
      sp_clear_bit(&transflag, 1);
    }
    else if(card->consume_flag == 0)//正常流水
    {
      sp_set_bit(&transflag, 1);
    }
    if(card->rate_flag == 0)//正常流水
    {
      sp_clear_bit(&transflag, 2);
      sp_clear_bit(&transflag, 3);
      sp_clear_bit(&transflag, 4);
    }
    else if(card->rate_flag == 1)//带搭伙费
    {
      sp_set_bit(&transflag, 2);
      sp_clear_bit(&transflag, 3);
      sp_clear_bit(&transflag, 4);

    }
    else//带折扣
    {
      sp_clear_bit(&transflag, 2);
      sp_set_bit(&transflag, 3);
      sp_clear_bit(&transflag, 4);
    }
  }
  //后四位
  sp_set_bit(&transflag, 5);//目前全部标记为已采集流水
  if(card->use_passwd_flag)
  {
    sp_set_bit(&transflag, 6);//已使用密码
  }
  else
  {
    sp_clear_bit(&transflag, 6);//未使用密码
  }
  if(card->pay_confirm_flag == 0)//写卡成功
  {
    sp_clear_bit(&transflag, 7);
    sp_set_bit(&transflag, 8);
  }
  else if(card->pay_confirm_flag == 1)//未写卡
  {
    sp_set_bit(&transflag, 7);
    sp_set_bit(&transflag, 8);
  }
  else if(card->pay_confirm_flag == 2)//中途拔卡
  {
    sp_set_bit(&transflag, 7);
    sp_clear_bit(&transflag, 8);
  }
  return transflag;
}

static int prepare_transdtl(sp_context * ctx, sp_card* card, sp_transdtl* transdtl)
{
  int extra_amount = 0;
  uint32 trans_no = 0;
  uint16 record_used_size;
  memset(transdtl, 0, sizeof(sp_transdtl));
  //根据各个标志为流水的标志赋值
  transdtl->transflag = get_transflag_by_card(card);
  //上一笔交易
  sp_uint32_to_3bytes(card->last_overdraft_limit, transdtl->last_limit_amt);
  transdtl->last_amt = card->last_trans_amt;
  transdtl->last_cnt = card->last_trans_seqno;
  transdtl->last_transflag = card->last_trans_flag;
  memcpy(transdtl->last_termno, card->last_samno, 6);
  memcpy(transdtl->last_datetime, card->last_datetime + 1, 6);
  //本次交易
  transdtl->amount = card->payamt;
  transdtl->cardbefbal = card->cardbefbal;
  transdtl->cardbefcnt = card->paycnt;
  transdtl->cardno = card->cardno;
  extra_amount = abs(card->payamt - card->amount);
  memcpy(transdtl->extra_amt, &extra_amount, 3);
  memset(transdtl->tac, 0 , sizeof transdtl->tac);
  memcpy(transdtl->tac, card->tac, 4);
  memcpy(transdtl->transdatetime, ctx->current_datetime + 1, 6);
  memcpy(transdtl->samno, ctx->syspara.samno, 6);
  //写流水的时候会用到这个流水号
  trans_no = sp_get_transno();
  transdtl->termseqno = trans_no + 1;
  //sp_disp_debug("termseqno=%d", transdtl->termseqno);
  //计算crc
  record_used_size = sizeof(sp_transdtl) - 2;//transdtl->crc - &transdtl->termseqno;
  //sp_disp_msg(20, "Record used size=%d", record_used_size);
  sp_protocol_crc((uint8*)transdtl, record_used_size, transdtl->crc);
  return  0;
}

static int do_transdtl_blackcard(sp_context* ctx, sp_card* card)
{
  int ret = 0;
  // black card
  sp_disp_msg(100, "该卡是黑卡");
  sp_set_card_loss(ctx, card);
  card->consume_flag = 2;//黑卡
  card->pay_confirm_flag = 0;
  ret = prepare_transdtl(ctx, card, &(ctx->exception_record));
  if(ret)
    return ret;
  ret = sp_write_transdtl(ctx, &(ctx->exception_record));//记一笔黑卡流水
  if(ret)
  {
    sp_disp_error("保存流水失败,ret=%d", ret);
    return ret;
  }
  return 0;
}

static int do_transdtl_halfway(sp_context* ctx, sp_card* card)
{
  int ret = 0;
  card->pay_confirm_flag = 2;//中途拔卡
  card->consume_flag = 0;//正常消费
  ret = prepare_transdtl(ctx, card, &(ctx->exception_record));
  if(ret)
    return ret;
  ret = sp_write_transdtl(ctx, &(ctx->exception_record));//记一笔中途拔卡流水
  if(ret)
  {
    sp_disp_error("保存流水失败,ret=%d", ret);
    return ret;
  }
  return 0;
}

static int do_transdtl_unconfirm(sp_context* ctx, sp_card* card)
{
  int ret = 0;
  card->consume_flag = 0;
  card->pay_confirm_flag = 1;
  ret = prepare_transdtl(ctx, card, &ctx->record);
  if(ret)
    return ret;
  ret = sp_write_transdtl(ctx, &ctx->record);
  if(ret)
  {
    sp_disp_error("保存流水失败,ret=%d", ret);
    return ret;
  }
  return 0;
}

static int upd_transdtl_confirm(sp_context* ctx, sp_card* card, int last_transdtl_addr)
{
  int ret = 0;
  card->pay_confirm_flag = 0;
  card->consume_flag = 0;
  ctx->record.transflag = get_transflag_by_card(card);
  memcpy(ctx->record.tac, card->tac, 4);
  ret = sp_update_transdtl_info(last_transdtl_addr, &(ctx->record));
  if(ret)
  {
    sp_disp_error("更新流水状态失败，ret=%04x", ret);
    return ret;
  }
  return 0;
}

static uint8 do_check_date_change(sp_context* ctx)
{
  int diff_days = 0;
  sp_get_time(ctx);
  diff_days = sp_calc_diff_days(ctx->today, ctx->syspara.today_date);
  if(diff_days > ctx->syspara.max_offline_days)
    return SP_R_OFFLINE_DAYS_EXCEED;
  if(diff_days > 1)
  {
    ctx->syspara.yesterday_total_cnt = 0;
    ctx->syspara.yesterday_total_amt = 0;
    ctx->syspara.today_total_cnt = 0;
    ctx->syspara.today_total_amt = 0;
    memcpy(ctx->syspara.today_date, ctx->today, 4);
    return SP_R_DATE_CHANGE;
  }
  else if(diff_days == 1)
  {
    ctx->syspara.yesterday_total_cnt = ctx->syspara.today_total_cnt;
    ctx->syspara.yesterday_total_amt = ctx->syspara.today_total_amt;
    ctx->syspara.today_total_cnt = 0;
    ctx->syspara.today_total_amt = 0;
    memcpy(ctx->syspara.today_date, ctx->today, 4);
    return SP_R_DATE_CHANGE;
  }
  else if(diff_days < 0)
  {
    sp_disp_error("致命错误,时间倒退,diffdays=%d", diff_days);
    memcpy(ctx->syspara.today_date, ctx->today, 4);
    return SP_E_DATE_REVERSE;
  }
  return 0;
}

static void do_update_sum_amt(sp_context* ctx, uint32 pay_amt)
{
  uint8 ret = 0;
  ret = do_check_date_change(ctx);
  if(ret == SP_E_DATE_REVERSE)
  {
    return;
  }
  ctx->syspara.today_total_amt += pay_amt;
  ++ctx->syspara.today_total_cnt;
  ret = sp_write_syspara(ctx);
  if(ret)
  {
    sp_handle_fatal_error(ctx, ret);
  }
}






/////////////////////////////外部函数//////////////////////////////////////////////
void sp_check_record_rom(sp_context* ctx)
{
  ctx->rec_flash_full = false;
}

static int do_input_money(sp_context* ctx, uint32* money, const uint8* cardphyid, char* value)
{
  uint8 upd = 1;
  uint32 key;
  int32 ret;
  uint8 req_card_phyid[4];
  char temp[20];
  uint8 t;
  while(1)
  {
    if(upd)
    {
      //更新一下金额
      sp_disp_by_type(SP_TP_DISP_MONEY, ctx, value);
      upd = 0;
    }
    ret = sp_request_card(req_card_phyid);
    if(ret)
    {
      //sp_disp_error(3, "==寻卡失败==");
      return SP_R_CARD_NOT_FOUND;
    }

    if(memcmp(req_card_phyid, cardphyid, 4) != 0)
    {
      sp_disp_debug("phyid=[%02x%02x%02x%02x]", req_card_phyid[0], req_card_phyid[1], req_card_phyid[2], req_card_phyid[3]);
      return SP_E_NOT_SAME_CARD;
    }

    key = sp_get_key();
    if(!sp_is_number_key(key, &t))
    {
      if(strlen(value) >= 6)
        continue;
      strcpy(temp, value);
      if(t == SP_KEY_DOT)
      {
        //第一个不可以是点
        if(strlen(value) == 0)
        {
          memcpy(value, "0.", 2);
        }
        else
        {
          //假如已经存在点了就不覆盖
          if(strchr(temp, '.') != NULL)
            continue;
          sprintf(value, "%s%c", temp, '.');
        }
      }
      else
      {
        sprintf(value, "%s%d", temp, t);
      }
      upd = 1;
    }
    /*
    else if(key == SP_KEY_MODE)
    {
      return -1;
    }
    */
    else if(key == SP_KEY_CONFIRM)
    {
      //*money = atoi(value) * 100;
      *money = sp_atof_mul100(value);
      return 0;
    }
    else if(key == SP_KEY_CLEAR)
    {
      uint8 vl = strlen(value);
      if(vl == 0)
        return SP_R_CANCEL_CONSUME;
      if(vl > 0)
        value[vl - 1] = 0;
      upd = 1;
    }
  }
}

static int do_input_money_without_card(sp_context* ctx, uint32* money, uint8 t)
{
  uint8 upd = 1;
  uint32 key;
  char temp[20];
  char value[20] = {0};
  //uint8 t;
  //输入金额
  if(t == SP_KEY_DOT)//输入的是.就变成0.
  {
    memcpy(value, "0.", 2);
  }
  else
  {
    strcpy(temp, value);
    sprintf(value, "%s%d", temp, t);
  }
  while(1)
  {
    if(upd)
    {
      //更新一下金额
      sp_disp_by_type(SP_TP_DISP_MONEY, ctx, value);
      upd = 0;
    }
    key = sp_get_key();
    if(!sp_is_number_key(key, &t))
    {
      if(strlen(value) >= 6)
        continue;
      strcpy(temp, value);
      if(t == SP_KEY_DOT)
      {
        //第一个不可以是点
        if(strlen(value) == 0)
        {
          memcpy(value, "0.", 2);
        }
        else
        {
          //假如已经存在点了就不覆盖
          if(strchr(temp, '.') != NULL)
            continue;
          sprintf(value, "%s%c", temp, '.');
        }
      }
      else
      {
        sprintf(value, "%s%d", temp, t);
      }
      upd = 1;
    }
    else if(key == SP_KEY_CONFIRM)
    {
      //*money = atoi(value) * 100;
      *money = sp_atof_mul100(value);
      return 0;
    }
    else if(key == SP_KEY_CLEAR)
    {
      uint8 vl = strlen(value);
      if(vl == 0)
        return 1;
      if(vl > 0)
        value[vl - 1] = 0;
      upd = 1;
    }
  }
}

static int do_purchase_wirte_card(sp_context* ctx, sp_card* card)
{
  int ret;
  ret = sp_payinit(ctx, card, card->payamt);
  if(ret)
  {
    sp_disp_error("pay init fail, ret=%04x", ret);
    return ret;
  }
  //sp_disp_debug("finish pay init=%d",1);
  // pay debit
  ret = sp_payment(ctx, card, card->payamt);
  if(ret)
  {
    //记一笔中途拔卡流水
    card->is_halfway = 1;
    return ret;
  }
  return 0;
}

static int do_wait_reload_card(sp_context* ctx, sp_card* card)
{
  sp_card this_card;
  int ret = 0;
  while(sp_request_card_poweron(&this_card))
  {
    sp_beep(1);
    SP_CLS(0);
    if(SP_KEY_CLEAR == sp_get_key())
      return SP_R_CANCEL_CONSUME;
  }
  //寻到卡了判断下是不是同一张卡
  if(!SP_IS_SAME_CARDPHYID(this_card.cardphyid, card->cardphyid))
  {
    return SP_E_NOT_SAME_CARD;
  }
  ret = sp_select_card_app(ctx);
  if(ret)
  {
    sp_disp_error("select card app error, ret=%04x", ret);
    return ret;
  }
  return 0;
}

static int do_wait_and_prove_card(sp_context* ctx, sp_card* card)
{
  int ret = 0;
  ret = do_wait_reload_card(ctx, card);
  if(ret)
    return ret;
  ret = sp_get_prove(ctx, card, ++card->paycnt);
  if(ret)
  {
    return SP_E_NOT_GET_PROVE;
  }
  return 0;
}

static int do_purchase(sp_context* ctx, sp_card* card)
{
  int ret;
  bool has_save_transdtl = false;
  while(1)
  {
    ret = do_purchase_wirte_card(ctx, card);
    if(ret)
    {
      //余额不足的情况不需要
      if(SP_E_INSUFFICIENT_BALANCE == ret)
        return ret;
      //记录一笔中途拔卡流水
      if(has_save_transdtl == false)
      {
        if(card->is_halfway)
        {
          //记一笔中途拔卡流水
          ret = do_transdtl_halfway(ctx, card);
          if(!ret)
          {
            has_save_transdtl = true;
          }
        }
      }
      sp_disp_msg(30, "请重新放卡");
      ret = do_wait_and_prove_card(ctx, card);
      // success,  记录成功消费流水一笔
      if(SP_SUCCESS == ret)
        break;
      sp_disp_debug("do_wait_and_prove_card,ret=%04x", ret);
      // 用户取消， return
      if(SP_R_CANCEL_CONSUME == ret)
      {
        sp_disp_msg(30, "用户取消");
        return ret;
      }
      //卡未扣成功，补扣
      if(SP_E_NOT_GET_PROVE == ret)
        continue;
    }
    else
      break;
  }
  //  记录成功消费流水一笔
  return 0;
}

static int check_and_input_money(sp_context* ctx, uint32* money, sp_card* card, char value[20], uint8 clear_value_flag, uint8* press_flag)
{
  int32 key = 0;
  //char value[20] = {0};
  char temp[20];
  uint8 req_card_phyid[4];
  uint8 t;
  if(clear_value_flag)
  {
    memset(value, 0, 20);
    while(1)
    {
      // 寻不到卡也要return
      if(sp_request_card(req_card_phyid))
      {
        return SP_R_CANCEL_CONSUME;//没有输过金额卡拿走也认为是用户取消
      }
      // 判断是不是同一张卡
      if(!SP_IS_SAME_CARDPHYID(req_card_phyid, card->cardphyid))
      {
        return SP_E_NOT_SAME_CARD;
      }
      key = sp_get_key();
      if(!sp_is_number_key(key, &t))
      {
        if(t == SP_KEY_DOT)
        {
          memcpy(value, "0.", 2);
        }
        else
        {
          strcpy(temp, value);
          sprintf(value, "%s%d", temp, t);
        }
        *press_flag = 0;
        break;
      }
    }
  }
  //输入金额
  return do_input_money(ctx, money, card->cardphyid, value);
}

static int do_check_card_validity(sp_context* ctx, sp_card* card)
{
  int ret = 0;
  //检查日期是否变化
  ret = do_check_date_change(ctx);
  if(ret)
  {
    if(ret == SP_E_DATE_REVERSE)
      return ret;
    ret = sp_write_syspara(ctx);
    if(ret)
    {
      sp_handle_fatal_error(ctx, ret);
    }
  }
  if(memcmp(ctx->today, card->expiredate, 4) > 0)
  {
    sp_disp_error("t[%02x%02x%02x%02x]->e[%02x%02x%02x%02x]",
                  ctx->today[0], ctx->today[1], ctx->today[2], ctx->today[3],
                  card->expiredate[0], card->expiredate[1], card->expiredate[2], card->expiredate[3]);
    return SP_E_CARD_DATE_EXPIRE;
  }
  if(card->paycnt >= ctx->syspara.max_pay_cnt)
  {
    return SP_E_CARD_PAYCNT_LIMIT;
  }
  if(sp_check_bitmap(card->feetype, ctx->syspara.feetype_bitmap, sizeof(ctx->syspara.feetype_bitmap)))
  {
    //sp_disp_error("card feetype=%02x", card->feetype);
    return SP_E_CARD_TYPE_NOT_SUPPORT;
  }
  // TODO: 脱机使用天数(可以挪到其他地方检查)
  // TODO : 判断黑卡前移到读卡，假如是黑卡马上记一笔黑卡消费的流水并返回
  if(do_check_blkcard(ctx, card))
  {
    ret = do_transdtl_blackcard(ctx, card);
    if(ret)
    {
      sp_handle_fatal_error(ctx, ret);
    }
    return SP_R_BLACKCARD_CONSUME;
  }
  //检查消费日期是否变化
  do_check_card_consume_date(ctx, card);
  return 0;
}

static int do_prepare_pay_with_card(sp_context* ctx, sp_card* the_card)
{
  int ret;
  uint32 amount;
  uint8 clear_value_flag = 1;
  uint8 press_num_flag = 1;
  char value[20] = {0};
  //  uint8 is_first_time = 1;
  //如果超时也清空一下上一次卡号
  if(sp_check_oper_timeout(ctx))
  {
    memset(ctx->last_cardphyid, 0, 8);
  }
  //寻到的是用一张卡就不进入下面的流程
  if(memcmp(ctx->last_cardphyid, ctx->card.cardphyid, 8) == 0)
  {
    return SP_R_REQUEST_SAME_CARD;
  }
  ret = sp_read_card(ctx, the_card, SP_FILE10 | SP_FILE12 | SP_FILE15 | SP_FILE16 | SP_FILE18 | SP_READ_CARDBAL);
  if(ret)
  {
    sp_halt(ctx, the_card);
    return ret;
  }
  ret = do_check_card_validity(ctx, the_card);
  if(ret)
    return ret;
  while(1)
  {
    //显示卡余额，用户姓名，学号
    sp_disp_by_type(SP_TP_USER_BALANCE, ctx, NULL);
    ret = check_and_input_money(ctx, &amount, the_card, value, clear_value_flag, &press_num_flag);
    if(ret)
    {
      if(SP_R_CANCEL_CONSUME == ret)
        return ret;
      //没输过数字键的情况,换了一张卡也相当于取消交易了
      if((press_num_flag) && (SP_E_NOT_SAME_CARD == ret))
        return SP_R_CANCEL_CONSUME;
      while(1)
      {
        SP_CLS_FULLSCREEN;
        if(SP_E_NOT_SAME_CARD == ret)
        {
          sp_print_row(FLAG_DO_NOT_CLEAR, 0, 0, "不是同一张卡");
          clear_value_flag = 1;
        }
        else
        {
          clear_value_flag = 0;
        }
        sp_print_row(FLAG_DO_NOT_CLEAR, 1, 0, "请重新刷卡>>");
        ret = do_wait_reload_card(ctx, &ctx->card);
        if(SP_R_CANCEL_CONSUME == ret)
          return SP_R_CANCEL_CONSUME;
        else if(SP_SUCCESS == ret)
          break;
      }
    }
    else
      break;
  }
  //sp_disp_debug("%d,%8x", money, money);
  if(amount <= 0)
  {
    sp_disp_msg(10, "输入金额为0 ,交易撤销");
    return SP_R_CANCEL_CONSUME;
  }
  the_card->amount = amount;
  return 0;
}

static int check_number_key(uint8* t)
{
  int key;
  key = sp_get_key();
  if(key == SP_KEY_FUNC)
    return SP_R_PRESS_FUNC;
  //sp_disp_debug("key=%02x,0=%02x,9=%02x", key, SP_KEY_0, SP_KEY_9);
  //if(sp_is_number_key(key, &t))
  if(sp_is_number_key(key, t))
    return SP_NO_NUMBER_KEY_PRESSED;
  else
    return 0;
}

static int do_prepare_pay_without_card(sp_context* ctx, sp_card* the_card)
{
  int ret = 0;
  uint8 t;
  uint32 amount = 0;
  //假如没有寻到卡就清空上一次物理卡号
  memset(ctx->last_cardphyid, 0, 8);
  ret = check_number_key(&t);
  if(ret)
    return ret;
  //输入金额
  if(do_input_money_without_card(ctx, &amount, t))
    return SP_R_CANCEL_CONSUME;
  if(amount <= 0)
  {
    sp_disp_msg(5, "输入金额为0 ,交易撤销");
    return SP_R_CANCEL_CONSUME;
  }

  the_card->amount = amount;
  //除非按撤销键，否则一直寻卡
  while(sp_request_card_poweron(the_card))
  {
    sp_disp_by_type(SP_TP_REQUEST_CONSUME_CARD, ctx, NULL);
    if(SP_KEY_CLEAR == sp_get_key())
      return SP_R_CANCEL_CONSUME;
  }
  ret = sp_read_card(ctx, the_card, SP_FILE10 | SP_FILE12 | SP_FILE15 | SP_FILE18 | SP_READ_CARDBAL);
  if(ret)
  {
    sp_halt(ctx, the_card);
    return ret;
  }
  ret = do_check_card_validity(ctx, the_card);
  if(ret)
    return ret;
  //sp_request_card(the_card->cardphyid);
  return 0;
}

/*
*  函数执行步骤:
*   1、先记一笔未写卡的流水
*   2、消费初始化已经消费确认，消费确认会返回tac码
*   3、更新流水状态为已写卡且填上tac码
*/
static int do_purchase_and_write_transdtl(sp_context * ctx, sp_card* card)
{
  int ret = 0;
  int last_transdtl_addr = 0;
  //1、先记一笔未写卡流水
  ret = do_transdtl_unconfirm(ctx, card);
  if(ret)
    return ret;
  last_transdtl_addr = sp_get_last_transaddr();//本次流水的地址
  //2、消费(有中途拔卡的就记一笔中途拔卡流水)
  ret = do_purchase(ctx, card);
  if(ret)
  {
    sp_beep(CONSUME_FAIL_BEEP);
    return ret;
  }
  //3、更新流水状态为已写卡
  ret = upd_transdtl_confirm(ctx, card, last_transdtl_addr);
  if(ret)
    return ret;
  return 0;
}

static int do_calc_paymoney(sp_context* ctx, sp_card* card)
{
  int rate = 0;
  rate = ctx->feerate_table[card->feetype - 1];
  if(rate > 100)
    card->rate_flag = 2;
  else if(rate < 100)
    card->rate_flag = 1;
  else
    card->rate_flag = 0;
  card->payamt = sp_d2i((card->amount * rate) * 1.0 / 100.0);
  return 0;
}

static int do_purchase_with_card(sp_context* ctx, sp_card* card, uint8 card_reset_flag)
{
  int ret;
  sp_card current_card;
  memset(&current_card, 0, sizeof current_card);
  /*
  //扣费部分(需要重新走一遍流程)
  ret = sp_request_card_poweron(&current_card);
  if(ret)
  {
    sp_disp_error("重新寻卡失败");
    return SP_R_CARD_NOT_FOUND;
  }
  if(!SP_IS_SAME_CARDPHYID(the_card->cardphyid, current_card.cardphyid))
  {
    return SP_E_NOT_SAME_CARD;
  }
  */
  if(card_reset_flag)
  {
    ret = sp_cpu_reset_card();
    if(ret)
    {
      sp_disp_error("sp_cpu_reset_card error,ret=%04x", ret);
      return ret;
    }
    SP_CLS(0);
    SP_PRINT(0, 0, "消费按键成功!!");
  }

  if(card->cardbefbal <= ctx->syspara.min_cardbal)
  {
    //提示该卡余额不足
    sp_disp_by_type(SP_TP_INSUFFICIENT_BALANCE, ctx, NULL);
    return SP_E_LESS_THAN_MIN_CARD_BALANCE;
  }
  ret = sp_select_card_app(ctx);
  if(ret)
    return ret;
  //判断卡状态
  if(CARD_STATUS_NORMAL != card->status)
  {
    return SP_E_CARD_STATUS;
  }
  // DO:计算折扣后的真实消费金额
  ret = do_calc_paymoney(ctx, card);
  if(ret)
  {
    sp_disp_error("计算折扣失败, ret=%04x", ret);
    return ret;
  }

  // DO:  判断消费限额
  ret = do_check_consume_limit(ctx, card);
  if(ret)
  {
    sp_disp_error("消费限额，ret=%04x", ret);
    return ret;
  }

  //消费并记流水
  ret = do_purchase_and_write_transdtl(ctx, card);
  if(ret)
  {
    sp_disp_error("write card error,ret= %04x", ret);
    return ret;
  }
  // DO:更新卡里的信息
  do_update_card_day_sum_amt(ctx, card);
  ret = sp_write_card(card, ctx, SP_FILE10);
  if(ret)
  {
    Beep(100);//长鸣
    sp_disp_msg(30, "长鸣后，请重新刷卡");
    // DO: 等待用户刷卡，寻卡成功后写卡里的0x10文件的信息
    ret = do_wait_reload_card(ctx, card);
  }
  else
  {
    do_update_sum_amt(ctx, ctx->record.amount);
  }
  sp_disp_by_type(SP_TP_CONSUME_SUCCESS, ctx, NULL);
  sp_beep(CONSUME_SUCCESS_BEEP);
  return 0;
}

int sp_consume_loop(sp_context* ctx)
{
  int ret = 0;
  uint8 card_reset_flag = 0;
  memset(&ctx->card, 0 , sizeof(ctx->card));
  sp_disp_by_type(SP_TP_REQUEST_CARD, ctx, NULL);
  //是否寻卡或者输入数字
  ret = sp_request_card_poweron(&ctx->card);
  if(ret)
  {
    ret = do_prepare_pay_without_card(ctx, &ctx->card);
    card_reset_flag = 0;
  }
  else
  {
    ret = do_prepare_pay_with_card(ctx, &ctx->card);
    card_reset_flag = 1;
  }
  if(ret)
  {
    return ret;
  }
  ret = do_purchase_with_card(ctx, &ctx->card, card_reset_flag);
  if(ret)
    return ret;
  //重置内存中的流水存储区
  memset(&ctx->record, 0, sizeof(sp_transdtl));
  memset(&ctx->exception_record, 0, sizeof(sp_transdtl));
  memcpy(ctx->last_cardphyid, ctx->card.cardphyid, 8);
  ctx->oper_upd_flag = 1;
  return 0;
}

