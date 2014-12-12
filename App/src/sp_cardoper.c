#include "sp_cardoper.h"


//static uint8 last_cardphyid[8] = {0};

static void uint16_to_pboc(uint16 value, uint8 data[2])
{
  data[0] = (value >> 8) & 0xFF;
  data[1] = value & 0xFF;
}
/*
static void uint32_to_pboc(uint32 value, uint8 data[4])
{
  data[0] = (value >> 24) & 0xFF;
  data[1] = (value >> 16) & 0xFF;
  data[2] = (value >> 8) & 0xFF;
  data[3] = value & 0xFF;
}
*/
static uint16 pboc_to_uint16(const uint8 data[2])
{
  uint16 r;
  r = data[0];
  r <<= 8;
  r |= data[1];
  return r;
}

static uint32 pboc_to_uint32(const uint8 data[4])
{
  uint32 r = 0;
  uint8 lhr[] = {24, 16, 8, 0};
  uint8 i;
  for(i = 0; i < 4; ++i)
  {
    r |= ((uint32)data[i]) << lhr[i];
  }
  return r;
}


/*
static int do_find_adf(uint8* data, uint16 datalen, uint8** cdd, uint8* cdd_len)
{
  uint16 i;
  for(i = 0; i < datalen; ++i)
  {
    if(memcmp(data + i, "\x9F\x0C", 2) == 0)
    {
      *cdd_len = data[i + 2];
      i += 3;
      *cdd = data + i;
      return 0;
    }
  }
  return -1;
}
*/
static int do_calc_cpu_safe_mac(sp_context* ctx, sp_card* card,
                                uint8* mac_cmd, uint8 mac_cmd_len,
                                uint8 mac[4])
{
  uint8 randomnum[8];
  adpu_cmd_t cmd;
  uint8 padlen = 0;
  int ret;

  /* get challenge 4 byte*/
  memset(&cmd, 0, sizeof cmd);
  if(ctx->sam_aid != SP_SAM_AID)
  {
    memcpy(cmd.send_buf, "\x00\xA4\x00\x00\x02", 5);
    cmd.send_len = 5;
    sp_uint16_to_buffer(SP_SAM_AID, cmd.send_buf + cmd.send_len);
    cmd.send_len += 2;
    ret = sp_sam_adpu_command(&cmd);
    if(ret)
    {
      //(("Select sam df03 error"));
      sp_disp_error("Select sam AID Error,%04x", ret);
      return ret;
    }
    if(cmd.sw != COS_R_SUCCESS)
    {
      //(("Select sam df03 error,rw =%04X", cmd.sw));
      sp_disp_error("Select sam AID Error,%04x", cmd.sw);
      return ret;
    }
    ctx->sam_aid = SP_SAM_AID;
  }


  memcpy(cmd.send_buf, "\x00\x84\x00\x00\x04", 5);
  cmd.send_len = 5;
  if(sp_cpu_adpu_command(&cmd))
  {
    sp_disp_error("Get challenge error");
    return -1;
  }
  if(cmd.sw != COS_R_SUCCESS)
  {
    sp_disp_error("Get challenge error, sw=%04X", cmd.sw);
    return -1;
  }
  memcpy(randomnum, cmd.recv_buf, 4);
  memset(randomnum + 4, 0, 4);

  /* sam init for des*/
  memcpy(cmd.send_buf, "\x80\x1A\x28\x01\x08", 5);
  cmd.send_len = 5;

  memcpy(cmd.send_buf + cmd.send_len, card->cardphyid, 4);
  memcpy(cmd.send_buf + cmd.send_len + 4, "\x80\x00\x00\x00", 4);
  cmd.send_len += 8;


  //((cmd.send_buf, cmd.send_len));
  if(sp_sam_adpu_command(&cmd))
  {
    sp_disp_error("Sam init for des error");
    return -1;
  }
  if(cmd.sw != COS_R_SUCCESS)
  {
    sp_disp_error("Sam init for des error,sw =%04X", cmd.sw);
    return -1;
  }

  /* sam des calc mac*/
  memcpy(cmd.send_buf, "\x80\xFA\x05\x00", 4);
  cmd.send_len = 4;

  if(mac_cmd_len % 8 != 0)
  {
    padlen = 8 - (mac_cmd_len % 8);
  }
  /* sam des command*/
  cmd.send_buf[cmd.send_len++] = 8 + mac_cmd_len + padlen;
  memcpy(cmd.send_buf + cmd.send_len, randomnum, 8);
  cmd.send_len += 8;
  memcpy(cmd.send_buf + cmd.send_len, mac_cmd, mac_cmd_len);
  cmd.send_len += mac_cmd_len;
  if(padlen > 0)
  {
    memcpy(cmd.send_buf + cmd.send_len, "\x80\x00\x00\x00\x00\x00\x00\x00", padlen);
    cmd.send_len += padlen;
  }
  //((cmd.send_buf, cmd.send_len));
  if(sp_sam_adpu_command(&cmd))
  {
    sp_disp_error("Sam des error");
    return -1;
  }
  if(cmd.sw != COS_R_SUCCESS)
  {
    sp_disp_error("Sam des error,sw =%04X", cmd.sw);
    return -1;
  }
  memcpy(mac, cmd.recv_buf, 4);
  return 0;
}


static void do_cpu_parse_file10(sp_card* card, uint8* file10, uint8 len)
{
  uint8 offset = 0;
  uint32 d_sum_amt = 0;
  // assert(len >= 24);
  /*
  //需不需要按照他的方式校验crc
  //1-4  交易日期
  memcpy(card->last_trans_date, file10 + offset, 4);
  //5-7交易时间
  offset = 4;
  memcpy(card->last_trans_time, file10 + offset, 3);
  */
  //8-11日累计交易金额
  offset = 7;
  sp_buffer_to_uint32(file10 + offset, &(d_sum_amt));
  card->day_sum_amt = d_sum_amt;
  /*
  //sp_disp_debug("read day_sum_amt=%d", d_sum_amt);
  // memcpy(&(card->day_sum_amt), file10 + offset, 4);
  //12-17上次交易终端号
  offset = 11;
  memcpy(card->last_trans_dev, file10 + offset, 6);
  //18-19 上次交易前次数
  offset = 17;
  memcpy(card->last_trans_cnt, file10 + offset, 2);
  //20-23 上次交易金额
  offset = 19;
  memcpy(card->last_trans_amt, file10 + offset, 4);
  */
}
static void do_cpu_parse_file12(sp_card* card, uint8* file12, uint8 len)
{
  uint8 offset = 0;
  uint32 int_value = 0;
  // assert(len >= 16);
  // 1-3 单次消费限额  3
  sp_3bytes_to_uint32(file12 + offset, &int_value);
  card->once_limit_amt = int_value;
  // sp_disp_debug("read once_limit_amt=%d,raw data=[%02x%02x%02x]", card->once_limit_amt, file12[offset], file12[offset + 1], file12[offset + 2]);
  // 4-6 日累计消费限额  3
  offset = 3;
  sp_3bytes_to_uint32(file12 + offset, &int_value);
  card->day_sum_limit_amt = int_value;
  // 7-8 卡消费次数  2
  offset = 6;
  card->paycnt = pboc_to_uint16(file12 + offset);
  // 9-12  消费时间(DDHHMMSS)  4
  // 13-16 设备ID  4
}

static void do_cpu_parse_file15(sp_card* card, uint8* file15, uint8 len)
{
  char cardno[13];
  uint8 offset = 0;
  // assert(len >= 44);
  //应用序列号(卡号)
  // offset = 0;
  // sp_bcd_hex(file15 + offset, 10, cardno);
  offset = 6;
  sp_bcd_hex(file15 + offset, 4, cardno);
  card->cardno = atoi(cardno);
  //卡状态
  offset = 20;
  card->status = file15[offset];
  offset = 21;
  //黑名单版本号
  memcpy(card->cardverno, file15 + offset, 7);
  offset = 34;
  //sp_disp_debug("[%02x%02x%02x%02x%02x%02x%02x]", card->cardverno[0], card->cardverno[1], card->cardverno[2], card->cardverno[3], card->cardverno[4], card->cardverno[5], card->cardverno[6]);
  //卡收费类别
  card->feetype = file15[offset];
  //sp_disp_debug("card->feetype=%d", card->feetype);
  //卡有效期
  offset = 40;
  memcpy(card->expiredate, file15 + offset , 4);
}

static void do_cpu_parse_file16(sp_card* card, uint8* file16, uint8 len)
{
  uint8 offset = 0;
  // assert(len >= 112);
  memcpy(card->username, file16, 32);
  offset = 66;
  memcpy(card->stuempno, file16 + offset, 20);
}
static void do_cpu_parse_file18(sp_card* card, uint8* file18, uint8 len)
{
  uint8 offset = 0;
  uint16 trade_seqno = 0;
  uint32  last_overdraft_limit = 0;
  uint32 last_trans_amt = 0;
  // assert(len >= 23);
  //ED或EP联机或脱机交易序号2
  sp_buffer_to_uint16(file18 + offset, &trade_seqno);
  card->last_trans_seqno = trade_seqno;
  //透支限额3
  offset = 2;
  sp_3bytes_to_uint32(file18 + offset, &last_overdraft_limit);
  card->last_overdraft_limit = last_overdraft_limit;
  //交易金额4
  offset = 5;
  sp_buffer_to_uint32(file18 + offset, &last_trans_amt);
  card->last_trans_amt = last_trans_amt;
  //交易类型标志
  offset = 9;
  card->last_trans_flag = file18[offset];
  //终端机编号6
  offset = 10;
  memcpy(card->last_samno, file18 + offset, 6);
  //交易日期（终端）(YYYYMMDD)
  offset = 16;
  memcpy(card->last_datetime, file18 + offset, 4);
  //交易时间（终端）(HHMMSS)
  offset = 20;
  memcpy(card->last_datetime + 4, file18 + offset, 3);
}

static int do_cpu_pay_init(sp_context* ctx, sp_card* card, uint32 money)
{
  int ret;
  adpu_cmd_t cmd;
  memcpy(cmd.send_buf, "\x80\x50\x01\x02\x0B\x01", 6);
  cmd.send_len = 6;
  sp_int_to_bigend(money, 4, cmd.send_buf + cmd.send_len);
  cmd.send_len += 4;
  memcpy(cmd.send_buf + cmd.send_len, ctx->syspara.samno, 6);
  cmd.send_len += 6;
  cmd.send_buf[cmd.send_len++] = 0x0F;
  ret = sp_cpu_adpu_command(&cmd);
  if(ret)
  {
    sp_disp_error("Initial for purchase error,%04x", ret);
    return ret;
  }
  if(cmd.sw != COS_R_SUCCESS)
  {
    sp_disp_debug("Initial for purchase error,sw=%04X", cmd.sw);
    if(cmd.sw == COS_R_INSUFFICIENT_BALANCE)
      return SP_E_INSUFFICIENT_BALANCE;
    return SP_E_INIT4PURCHASE;
  }
  card->cardbefbal = pboc_to_uint32(cmd.recv_buf);
  card->paycnt = pboc_to_uint16(cmd.recv_buf + 4);
  card->keyver = cmd.recv_buf[9];
  card->keyindex = cmd.recv_buf[10];
  memcpy(card->pay_random, cmd.recv_buf + 11, 4);
  return 0;
}


//作用:将random变为encrypt_random
static int calc_ext_auth_encrypt_for_sam(sp_context* ctx, const uint8 * cardphyid, uint8 keyidx,
                                         const uint8* random, uint8* encrypt_random)
{
  //1、选择df03
  adpu_cmd_t cmd;
  int ret = 0;
  uint8 mac_data_len = 0;
  // 假如不是df03目录下才执行
  if(ctx->sam_aid != SP_SAM_AID)
  {
    memset(&cmd, 0, sizeof(cmd));
    memcpy(cmd.send_buf, "\x00\xA4\x00\x00\x02\xDF\x03", 7);
    cmd.send_len = 7;
    ret = sp_sam_adpu_command(&cmd);
    if(ret)
    {
      sp_disp_error("SAM卡应用目录错误,ret=%04x", ret);
      return ret;
    }
    ctx->sam_aid = SP_SAM_AID;
  }
  //2、分散密钥
  memset(&cmd, 0, sizeof(cmd));
  memcpy(cmd.send_buf, "\x80\x1A\x27", 3);
  cmd.send_len = 3;
  cmd.send_buf[cmd.send_len ++] = keyidx;
  cmd.send_buf[cmd.send_len ++] = 0x08;
  memcpy(cmd.send_buf + cmd.send_len, cardphyid, 4);
  memcpy(cmd.send_buf + cmd.send_len + 4, "\x80\x00\x00\x00", 4);
  cmd.send_len += 8;
  ret = sp_sam_adpu_command(&cmd);
  if(ret)
  {
    sp_disp_error("分散密钥错误，ret=%04x", ret);
    return ret;
  }
  //3、加密数据
  memset(&cmd, 0, sizeof(cmd));
  memcpy(cmd.send_buf, "\x80\xFA\x00\x00", 4);
  cmd.send_len = 4;
  mac_data_len = 8;
  cmd.send_buf[cmd.send_len ++] = mac_data_len;
  memcpy(cmd.send_buf + cmd.send_len, random, mac_data_len);
  cmd.send_len += mac_data_len;
  ret = sp_sam_adpu_command(&cmd);
  if(ret)
  {
    sp_disp_error("加密错误,ret=%04x", ret);
    return ret;
  }
  memcpy(encrypt_random, cmd.recv_buf, 8);
  return 0;
}
static int get_random_num(uint8 random[8])
{
  int ret;
  adpu_cmd_t cmd;
  memset(&cmd, 0, sizeof(cmd));
  memcpy(cmd.send_buf, "\x00\x84\x00\x00\x04", 5);
  cmd.send_len = 5;
  ret = sp_cpu_adpu_command(&cmd);
  if(ret)
  {
    sp_disp_error("get random error,%04x", ret);
    return ret;
  }
  memset(random, 0, 8);
  memcpy(random, cmd.recv_buf, 4);//前八个字节为随机数，最后两个字节为\x90\x00
  return 0;
}

static int do_ext_auth(sp_context* ctx, const uint8* cardphyid, uint8 keyidx)
{
  uint8 ucRandom[9];
  uint8 encrypt_random[9];
  int nRet;
  adpu_cmd_t cmd;
  nRet = get_random_num(ucRandom);
  if(nRet)
    return nRet;
  memset(&cmd, 0, sizeof(cmd));
  nRet = calc_ext_auth_encrypt_for_sam(ctx, cardphyid, keyidx, ucRandom, encrypt_random);
  if(nRet)
  {
    return nRet;
  }
  memset(&cmd, 0, sizeof(cmd));
  memcpy(cmd.send_buf, "\x00\x82\x00", 3);
  cmd.send_len = 3;
  cmd.send_buf[cmd.send_len++] = keyidx;
  cmd.send_buf[cmd.send_len++] = 0x08;
  memcpy(cmd.send_buf + cmd.send_len, encrypt_random, 8);
  cmd.send_len += 8;
  nRet = sp_cpu_adpu_command(&cmd);
  if(nRet)
  {
    if(cmd.sw)
    {
      return cmd.sw;
    }
    else
    {
      return nRet;
    }
  }
  return 0;
}

int sp_select_card_app(sp_context* ctx)
{
  int ret = 0;
  adpu_cmd_t cmd;
  //选择df03
  memset(&cmd, 0, sizeof cmd);
  memcpy(cmd.send_buf, "\x00\xA4\x04\x00\x0F\xD1\x56\x00\x00\x01\xBD\xF0\xCA\xCB\xB4\xEF\xD6\xA7\xB8\xB6\x00", 21);
  cmd.send_len = 21;
  ret = sp_cpu_adpu_command(&cmd);
  if(ret)
  {
    sp_disp_error("Select DF03 Error,sw=%04X", cmd.sw);
    return ret;
  }
  ctx->card.aid = 0xdf03;
  return  0;
}


static int do_cpucard_read_card(sp_context* ctx, sp_card* card, uint8 read_level)
{
  int ret;
  adpu_cmd_t cmd;
  uint8 mac[4];
  ret = sp_select_card_app(ctx);
  if(ret)
  {
    sp_disp_error("读卡前,select df03 error, ret=%04x", ret);
    return ret;
  }
  if(read_level & SP_FILE10)
  {
    //解析file10文件第一条
    //sp_disp_msg(10, "读file10文件");
    memset(&cmd, 0, sizeof(cmd));
    memcpy(cmd.send_buf, "\x00\xB2\x01\x84\x00", 5);
    cmd.send_len = 5;
    ret = sp_cpu_adpu_command(&cmd);
    if(ret)
    {
      sp_disp_error("read file10 error,sw=%04X", cmd.sw);
      return ret;
    }
    else
    {
      do_cpu_parse_file10(card, cmd.recv_buf, cmd.recv_len);
    }
  }
  if(read_level & SP_FILE12)
  {
    //sp_disp_msg(10, "读file12文件");
    memset(&cmd, 0, sizeof(cmd));
    memcpy(cmd.send_buf, "\x00\xB0\x92\x00\x10", 5);
    cmd.send_len = 5;
    ret = sp_cpu_adpu_command(&cmd);
    if(ret)
    {
      sp_disp_error("read file12 error,sw=%04X", cmd.sw);
      return ret;
    }
    else
    {
      do_cpu_parse_file12(card, cmd.recv_buf, cmd.recv_len);
    }
  }
  if(read_level & SP_FILE15)
  {
    //sp_disp_msg(10, "读file15文件");
    //读file15文件
    memset(&cmd, 0, sizeof(cmd));
    memcpy(cmd.send_buf, "\x00\xB0\x95\x00\x38", 5);
    cmd.send_len = 5;
    ret = sp_cpu_adpu_command(&cmd);
    if(ret)
    {
      sp_disp_error("read file15 error,sw=%04X", cmd.sw);
      return ret;
    }
    else
    {
      do_cpu_parse_file15(card, cmd.recv_buf, cmd.recv_len);
    }
  }
  if(read_level & SP_FILE16)
  {
    //sp_disp_msg(10, "读file16文件");
    //读file16文件
    memset(&cmd, 0, sizeof(cmd));
    memcpy(cmd.send_buf, "\x00\xB0\x96\x00\x70", 5);
    cmd.send_len = 5;
    ret = sp_cpu_adpu_command(&cmd);
    if(ret)
    {
      sp_disp_error("read file16 error,sw=%04X", cmd.sw);
      return ret;
    }
    else
    {
      do_cpu_parse_file16(card, cmd.recv_buf, cmd.recv_len);
    }
    //选择df03
    ret = sp_select_card_app(ctx);
    if(ret)
    {
      sp_disp_error("读完16文件后，选df03失败");
      return ret;
    }
  }
  if(read_level & SP_FILE18)
  {
    //读file18文件里的第一条记录
    memset(&cmd, 0, sizeof(cmd));
    memcpy(cmd.send_buf, "\x00\xB2\x01\xC4\x00", 5);
    cmd.send_len = 5;
    ret = sp_cpu_adpu_command(&cmd);
    if(ret)
    {
      sp_disp_error("read file18 error,sw=%04X", cmd.sw);
      return ret;
    }
    else
    {
      do_cpu_parse_file18(card, cmd.recv_buf, cmd.recv_len);
    }
  }
  if(read_level & SP_FILE19)
  {
    //sp_disp_msg(10, "读file19文件");
    //读file19文件
    //先进行外部认证
    ret = do_ext_auth(ctx, card->cardphyid, 1);
    if(ret)
    {
      sp_disp_error("外部认证失败,ret=%4x", ret);
      return ret;
    }
    memset(&cmd, 0, sizeof(cmd));
    memcpy(cmd.send_buf, "\x04\xB0\x99\x00\x04", 5);
    cmd.send_len = 5;
    if(do_calc_cpu_safe_mac(ctx, card, cmd.send_buf, cmd.send_len, mac))
    {
      return SP_E_CALC_MAC;
    }
    memcpy(cmd.send_buf + cmd.send_len, mac, 4);
    cmd.send_len += 4;
    //cmd.send_buf[cmd.send_len++] = 0x00;
    /*
    memset(disp_msg, 0, 256);
    for(i = 0; i < cmd.send_len; i++)
    {
      sprintf(disp_msg + i * 2, "%02x", cmd.send_buf[i]);
    }
    sp_wait_for_retkey(SP_KEY_CONFIRM, __FUNCTION__, __LINE__, disp_msg);
    */
    ret = sp_cpu_adpu_command(&cmd);
    //
    /*
      memset(disp_msg, 0, 256);
      for(i = 0; i < cmd.recv_len; i++)
      {
        sprintf(disp_msg + i * 2, "%02x", cmd.recv_buf[i]);
      }
      sp_wait_for_retkey(SP_KEY_CONFIRM, __FUNCTION__, __LINE__, disp_msg);
      */
    if(ret)
    {
      sp_disp_error("read file19 error,sw=%04X", cmd.sw);
      return ret;
    }
    else
    {
      memcpy(card->passwd, cmd.recv_buf, cmd.recv_len);
    }
  }
  if(read_level & SP_READ_CARDBAL)
  {
    //发一次消费初始化指令读取余额
    ret = do_cpu_pay_init(ctx, card, 0);
    if(ret)
    {
      if(SP_E_INIT4PURCHASE == ret)
        return SP_E_INIT4PURCHASE;
      sp_disp_error("read balance error,sw=%04X", cmd.sw);
      return ret;
    }
    //ret = sp_cpu_reset_card();
    //sp_disp_error("after readcard resetcard ret=%04x", ret);
  }
  return 0;
}

static int do_cpu_payment(sp_context* ctx, sp_card* card, uint32 money)
{
  int ret;
  uint8 mac1[4];
  adpu_cmd_t cmd;
  memset(&cmd, 0 , sizeof cmd);
  if(ctx->sam_aid != 0xdf03)
  {
    memcpy(cmd.send_buf, "\x00\xA4\x00\x00\x02\xDF\x03", 7);
    cmd.send_len = 7;
    ret = sp_sam_adpu_command(&cmd);
    if(ret)
    {
      sp_disp_error("Select sam df03 error,%04x", ret);
      return -1;
    }
    if(cmd.sw != COS_R_SUCCESS)
    {
      sp_disp_error("Select sam df03 error,rw =%04X", cmd.sw);
      return -1;
    }
    ctx->sam_aid = 0xDf03;
  }

  memcpy(cmd.send_buf, "\x80\x70\x00\x00\x1C", 5);
  cmd.send_len = 5;
  /*random*/
  memcpy(cmd.send_buf + cmd.send_len , card->pay_random, 4);
  cmd.send_len += 4;
  /* pay cnt*/
  uint16_to_pboc(card->paycnt, cmd.send_buf + cmd.send_len);
  cmd.send_len += 2;
  /*trans amt*/
  sp_uint32_to_buffer(money, cmd.send_buf + cmd.send_len);
  cmd.send_len += 4;
  /*交易类型标志。2byte*/
  //cmd.send_buf[cmd.send_len++] = 0x00;
  cmd.send_buf[cmd.send_len++] = 0x06;
  /*trans datetime*/
  memcpy(cmd.send_buf + cmd.send_len, ctx->current_datetime, 7);
  cmd.send_len += 7;
  /* 消费密钥版本号*/
  //sp_disp_msg(12, "消费密钥版本号=%d", ctx->syspara.key_index);
  cmd.send_buf[cmd.send_len++] = ctx->syspara.key_index;
  /* 消费密钥算法标志*/
  cmd.send_buf[cmd.send_len++] = 0x00;
  /* 用户卡应用序列号*/
  if(card->cardtype == CPUCARD)
  {
    memcpy(cmd.send_buf + cmd.send_len, card->cardphyid, 4);
    memcpy(cmd.send_buf + cmd.send_len + 4, "\x80\x00\x00\x00", 4);
    cmd.send_len += 8;
  }
  else if(card->cardtype == SIMCARD)
  {
    memcpy(cmd.send_buf + cmd.send_len, card->cardphyid, 8);
    cmd.send_len += 8;
  }
  //cmd.send_buf[cmd.send_len++] = 0x08;
  //(("PSAM init4purchase"));
  //((cmd.send_buf, cmd.send_len));
  ret = sp_sam_adpu_command(&cmd);
  if(ret)
  {
    sp_disp_error("PSAM init4purchase error,ret=%d", ret);
    return ret;
  }
  if(cmd.sw != COS_R_SUCCESS)
  {
    sp_disp_error("PSAM init4purchase error, sw=%04X", cmd.sw);
    return -1;
  }
  card->sam_seqno = pboc_to_uint32(cmd.recv_buf);
  memcpy(mac1, cmd.recv_buf + 4, 4);
  /*card*/
  memcpy(cmd.send_buf, "\x80\x54\x01\x00\x0F", 5);
  cmd.send_len = 5;
  /* sam seqno*/
  memcpy(cmd.send_buf + cmd.send_len, cmd.recv_buf, 4);
  cmd.send_len += 4;
  memcpy(cmd.send_buf + cmd.send_len, ctx->current_datetime, 7);
  cmd.send_len += 7;
  memcpy(cmd.send_buf + cmd.send_len, mac1, 4);
  cmd.send_len += 4;

  //(("Card debit4purchase"));
  //((cmd.send_buf, cmd.send_len));
#ifdef _DEBUG
  /* delayxms(500);*/
#endif
  ret = sp_cpu_adpu_command(&cmd);
  if(ret)
  {
    sp_disp_error("Card debit4purchase error,ret=%04x", ret);
    return ret;
  }
  if(cmd.sw != COS_R_SUCCESS)
  {
    sp_disp_error("Card debit4purchase error, sw=%04X", cmd.sw);
    return -1;
  }
  memcpy(card->tac, cmd.recv_buf, 4);
  //写卡成功的tac
  memcpy(ctx->record.tac, cmd.recv_buf, 4);
  return 0;
}

static int do_cpu_get_prove(sp_context* ctx, sp_card* card, uint16 paycnt)
{
  adpu_cmd_t cmd;
  // char disp_msg[256];
  int ret = 0;
  //int i = 0;
  memcpy(cmd.send_buf, "\x80\x5A\x00\x06\x02", 5);
  cmd.send_len = 5;
  uint16_to_pboc(paycnt, cmd.send_buf + cmd.send_len);
  cmd.send_len += 2;
  cmd.send_buf[cmd.send_len++] = 0x08;

  ret = sp_cpu_adpu_command(&cmd);
  sp_disp_debug("getprove,recvlen=%d", cmd.recv_len);
  /*
  memset(disp_msg, 0, 256);
  for(i = 0; i < cmd.send_len; i++)
  {
    sprintf(disp_msg + i * 2, "%02x", cmd.send_buf[i]);
  }
  sp_wait_for_retkey(SP_KEY_CONFIRM, __FUNCTION__, __LINE__, disp_msg);
  memset(disp_msg, 0, 256);
  for(i = 0; i < cmd.recv_len; i++)
  {
    sprintf(disp_msg + i * 2, "%02x", cmd.recv_buf[i]);
  }
  sp_wait_for_retkey(SP_KEY_CONFIRM, __FUNCTION__, __LINE__, disp_msg);
  */
  if(ret)
  {
    sp_disp_error("Get trans prove error,%04x", ret);
    return -1;
  }

  if(cmd.sw != COS_R_SUCCESS)
  {
    sp_disp_error("Get trans prove error,sw=%04X", cmd.sw);
    return 1;
  }
  sp_disp_debug("Get transaction prove, paycnt=%d", paycnt);
  memcpy(card->tac, cmd.recv_buf + 4, 4);
  return 0;
}


static int do_cpu_set_card_loss(sp_context* ctx, sp_card* card)
{

  uint8 mac[4];
  adpu_cmd_t cmd;
  memset(&cmd, 0, sizeof cmd);
  /* update binary file*/
  memcpy(cmd.send_buf, "\x04\xD6\x95\x14\x0B\x03", 6);
  cmd.send_len = 6;
  memcpy(cmd.send_buf + cmd.send_len, ctx->syspara.blacklist_verno, 7);
  cmd.send_len += 7;
  if(do_calc_cpu_safe_mac(ctx, card, cmd.send_buf, cmd.send_len, mac))
  {
    return -1;
  }
  memcpy(cmd.send_buf + cmd.send_len, mac, 4);
  cmd.send_len += 4;
  //((cmd.send_buf, cmd.send_len));
  /*
  if(sp_cpu_adpu_command(&cmd))
  {
    sp_disp_error("Update card binary error");
    return -1;
  }
  */
  if(cmd.sw != COS_R_SUCCESS)
  {
    sp_disp_error("Update card binary error,sw=%04X", cmd.sw);
    return -1;
  }
  sp_disp_msg(10, "set loss card success");
  return 0;
}
/*
#define RFUIM_CMD_SW(cmd, cmdlen) ((uint16)((uint16)cmd[cmdlen -2] << 8) | cmd[cmdlen - 1])


enum {TRANS_BEGIN = 1, TRANS_END};
enum { TRANS_CONSUME = 0x01, TRANS_DEPOSIT = 0x88 , TRANS_LOCK = 0x99 };
enum {RFUIM_BASEINFO_SECT = 6, RFUIM_TRANSINFO_SECT = 9, RFUIM_RECORD_SECT = 2,
      RFUIM_PURSE_SECT = 1
     };

typedef struct
{
  uint8 recidx;
  uint8 paycnt[2];
  uint8 transflag;
  uint8 rfu1[2];
  uint8 status;
  uint8 cardverno[6];
  uint8 dpscnt[2];
  uint8 crc;
} rfuim_trans_t;

typedef struct
{
  uint8 transdate[4];
  uint8 cardbefbal[4];
  uint8 amount[3];
  uint8 transtype;
  uint8 deviceid[4];
} rfuim_trans_record_t;
*/
/*
static uint8 check_m1_card()
{
  return 0;
  //HL_Active
}
*/

int sp_request_card_poweron(sp_card* card)
{
  int ret;
  ret = sp_request_card(card->cardphyid);
  if(ret)
    return ret;

  //判断是哪种类型的卡
  ret  = sp_cpu_reset_card();//用于区分是不是cpu卡
  if(ret)
  {
    card->cardtype = M1CARD;
    return 2;//暂时不处理
  }
  else
  {
    card->cardtype = CPUCARD;
  }
  /*
  if(card->cardtype == CPUCARD)
  {
    ret = sp_cpucard_poweron();
    if(ret)
    {
      sp_disp_error("CPU 卡上电复位失败");
      return -1;
    }
  }
  */
  return 0;
}

int sp_request_card(uint8 cardphyid[4])
{
  int ret = 0;
  uint8 uid[4];
  memset(uid, 0, sizeof uid);
  sp_cpu_deselect_card();//取消选择
  //64E3E3F6
  ret = sp_read_uid(uid);//用于区分有没有卡
  if(ret)
    return ret;
  cardphyid[0] = uid[3];
  cardphyid[1] = uid[2];
  cardphyid[2] = uid[1];
  cardphyid[3] = uid[0];
  return 0;
}


int sp_read_card(sp_context* ctx, sp_card* card, uint8 read_level)
{
  int ret = 0;
  switch(card->cardtype)
  {
    case CPUCARD:
      ret = do_cpucard_read_card(ctx, card, read_level);
      sp_set_bit(&ctx->global_status, SP_STATUS_RESET_TIME);//读完卡以后需要重设时间
      return ret;
    case M1CARD:
      return 0;
    case SIMCARD:
      return 0;
    default:
      return -1;
  }
}
int sp_payinit(sp_context* ctx, sp_card* card, uint32 money)
{
  switch(card->cardtype)
  {
    case CPUCARD:
      return do_cpu_pay_init(ctx, card, money);
    case M1CARD:
      return 0;
    case SIMCARD:
      return 0;
    default:
      return -1;
  }
}

int sp_payment(sp_context* ctx, sp_card* card, uint32 money)
{
  switch(card->cardtype)
  {
    case CPUCARD:
      return do_cpu_payment(ctx, card, money);
    case M1CARD:
      return 0;
    case SIMCARD:
      return 0;
    default:
      return -1;
  }
}
int sp_get_prove(sp_context* ctx, sp_card* card, uint16 paycnt)
{
  switch(card->cardtype)
  {
    case CPUCARD:
      return do_cpu_get_prove(ctx, card, paycnt);
    case M1CARD:
      return 0;
    case SIMCARD:
      return 0;
    default:
      return -1;
  }
}

int sp_set_card_loss(sp_context* ctx, sp_card* card)
{
  switch(card->cardtype)
  {
    case CPUCARD:
      return do_cpu_set_card_loss(ctx, card);
    case M1CARD:
      return 0;
    case SIMCARD:
      return 0;
    default:
      return -1;
  }
}

int sp_halt(sp_context* ctx, sp_card* card)
{
  switch(card->cardtype)
  {
    case CPUCARD:
      return 0;
    case M1CARD:
      //
      return 0;
    case SIMCARD:
      return 0;
    default:
      return -1;
  }
}




uint8  sp_rst_psam_card(uint8 *cData, uint8 *cLen)
{
  return (Rst_Psam_Card(cData, cLen) == 1) ? 0 : 1;
}

int sp_init_sam_card(sp_context* ctx, uint8 samno[6])
{
  int ret;
  unsigned char rxbuf[256];
  uint8 len = 0;
  adpu_cmd_t cmd;
  Init_Psam();
  ctx->sam_ok = 0;
  ret = sp_rst_psam_card(rxbuf, &len);
  if(ret)
  {
    sp_disp_error("SAM卡复位错误");
    return -1;
  }
  memset(&cmd, 0, sizeof cmd);
  // memcpy(cmd.send_buf, "\x00\xA4\x00\x00\x02\x3F\x00", 7);
  memcpy(cmd.send_buf, "\x00\xA4\x00\x00\x02\x3F\x00", 7);
  cmd.send_len = 7;
  ret = sp_sam_adpu_command(&cmd);
  if(ret)
  {
    sp_disp_error("读取PSAM卡信息错误,但继续。。");
    //return -1;
  }

  memset(&cmd, 0, sizeof cmd);
  memcpy(cmd.send_buf, "\x00\xB0\x96\x00\x06", 5);
  cmd.send_len = 5;
  ret = sp_sam_adpu_command(&cmd);
  if(ret)
  {
    sp_disp_error("无法读取SAM号");
    return -1;
  }
  if(cmd.sw != COS_R_SUCCESS)
  {
    sp_disp_error("读取SAM卡号错误, sw=%04X", cmd.sw);
    return -1;
  }
  memcpy(samno, cmd.recv_buf, 6);

  memcpy(cmd.send_buf, "\x00\xA4\x00\x00\x02\xDF\x03", 7);
  cmd.send_len = 7;
  ret = sp_sam_adpu_command(&cmd);
  if(ret)
  {
    sp_disp_error("SAM卡应用目录错误, ret=%04x", ret);
    return -1;
  }
  if(cmd.sw != COS_R_SUCCESS)
  {
    sp_disp_error("Select sam df03 error,rw =%04X", cmd.sw);
    return -1;
  }
  ctx->sam_aid = 0xDf03;
  ctx->sam_ok = 1;
  return 0;
}

int sp_enable_sam_card(sp_context* ctx)
{
  int ret = 0;
  uint8 samno[6];
  //初始化psam卡
  ret = sp_init_sam_card(ctx, samno);
  if(ret)
    return ret;
  if(memcmp(samno, ctx->syspara.samno, 6) != 0)
  {
    memcpy(ctx->syspara.samno, samno, 6);
    /*
    ret = sp_write_syspara(ctx);
    if(ret)
    {
      sp_disp_error("samno write fail,samno=%s", samno);
      return ret;
    }
    */
  }
  return 0;
}


uint8 sp_write_card(sp_card* card, sp_context* ctx, uint8 write_level)
{
  //写file10文件
  adpu_cmd_t cmd;
  int ret = 0;
  char disp_msg[500];
  int i = 0;
  if(write_level & SP_FILE10)
  {
    memset(&cmd, 0, sizeof cmd);
    /* update binary file*/
    memcpy(cmd.send_buf, "\x00\xDC\x01\x84\x18", 5);
    cmd.send_len = 5;
    //交易日期    4byte
    //交易时间  3byte
    memcpy(cmd.send_buf + cmd.send_len, ctx->current_datetime, 7);
    cmd.send_len += 4;
    cmd.send_len += 3;
    //  8-11  日累计交易金额  4 B
    sp_uint32_to_buffer(card->day_sum_amt, cmd.send_buf + cmd.send_len);
    //memcpy(cmd.send_buf + cmd.send_len, &(), 4);
    cmd.send_len += 4;
    //sp_disp_debug("write last_dev_no=[%02x%02x%02x%02x%02x%02x]", ctx->syspara.samno[0], ctx->syspara.samno[1],
    //ctx->syspara.samno[2], ctx->syspara.samno[3], ctx->syspara.samno[4], ctx->syspara.samno[5]);
    //12-17 上次交易终端号  6 b
    memcpy(cmd.send_buf + cmd.send_len, ctx->syspara.samno, 6);
    cmd.send_len += 6;
    //18-19 上次交易前次数  2 B
    uint16_to_pboc(card->last_trans_seqno, cmd.send_buf + cmd.send_len);
    cmd.send_len += 2;
    //20-23 上次交易金额  4
    sp_int_to_bigend(card->amount, 4, cmd.send_buf + cmd.send_len);
    cmd.send_len += 4;
    //根据新的crc算法算出crc，存在最后一个字节
    cmd.send_buf[cmd.send_len++] = sp_get_crc(cmd.send_buf + 6);
    //02000000080800000000000000000000000000000000000A
    ret = sp_cpu_adpu_command(&cmd);
    if(ret)
    {
      memset(disp_msg, 0, 500);
      for(i = 0; i < cmd.send_len; i++)
      {
        sprintf(disp_msg + i * 3, "%02x ", cmd.send_buf[i]);
      }
      sp_wait_for_retkey(SP_KEY_0, __FUNCTION__, __LINE__, "senddata=%s", disp_msg);
      sp_disp_msg(20, "写卡file10,ret=%04x,recvlen=%d", ret, cmd.recv_len);
      memset(disp_msg, 0, 500);
      for(i = 0; i < cmd.recv_len; i++)
      {
        sprintf(disp_msg + i * 3, "%02x ", cmd.recv_buf[i]);
      }
      sp_wait_for_retkey(SP_KEY_0, __FUNCTION__, __LINE__, "recvdata=%s", disp_msg);
      return ret;
    }
  }
  return 0;
}



