#include "sp_syscontrol.h"

///////////////////////////////////////私有函数/////////////////////////////////////////////

static int do_reset_trussdtl_data(sp_context* ctx)
{
  uint32 page_no = 0;
  uint32 tmp_addr = 0;
  uint8 ret = 0;
  sp_trussno_unit slave_unit;
  sp_trussdtl trussdtl;
  for(tmp_addr = ADDR_TRUSSDTL_START; tmp_addr < ADDR_TRUSSDTL_END; tmp_addr += FLASH_PAGE_SIZE)
  {
    page_no = tmp_addr / FLASH_PAGE_SIZE;
    ret = sp_flashpage_erase(page_no);
    if(ret)
    {
      sp_disp_error("擦除流水存储区失败");
      return SP_E_FLASH_ERASE;
    }
  }
  //在第一个位置写一笔0号流水
  memset(&trussdtl, 0, sizeof(sp_trussdtl));
  sp_get_trussno_unit(&slave_unit);
  memcpy(trussdtl.truss_no, slave_unit.last_truss_no, 5);
  trussdtl.end_transaddr = ADDR_TRANSDTL_START;
  sp_protocol_crc((uint8*)(&trussdtl), sizeof(sp_trussdtl) - 2, trussdtl.crc);
  page_no = ADDR_TRUSSDTL_START / FLASH_PAGE_SIZE;
  ret = sp_flash_write(page_no, 0, (uint8*)(&trussdtl), sizeof(sp_trussdtl), 0);
  if(ret)
    return SP_E_FLASH_WRITE;
  return SP_SUCCESS;
}

static int do_reset_transdtl_data(sp_context* ctx)
{
  uint32 page_no = 0;
  uint32 tmp_addr = 0;
  uint8 ret = 0;
  sp_transdtl transdtl;
  for(tmp_addr = ADDR_TRANSDTL_START; tmp_addr < ADDR_TRANSDTL_END; tmp_addr += FLASH_PAGE_SIZE)
  {
    page_no = tmp_addr / FLASH_PAGE_SIZE;
    ret = sp_flashpage_erase(page_no);
    if(ret)
    {
      sp_disp_error("擦除流水存储区失败");
      return SP_E_FLASH_ERASE;
    }
  }
  //在第一个位置写一笔0号流水
  memset(&transdtl, 0, sizeof(sp_transdtl));
  transdtl.termseqno = sp_get_transno();
  memcpy(transdtl.transdatetime, ctx->current_datetime+1,  6);
  sp_protocol_crc((uint8*)(&transdtl), sizeof(sp_transdtl) - 2, transdtl.crc);
  page_no = ADDR_TRANSDTL_START / FLASH_PAGE_SIZE;
  ret = sp_flash_write(page_no, 0, (uint8*)(&transdtl), sizeof(sp_transdtl), 0);
  if(ret)
    return SP_E_FLASH_WRITE;
  return SP_SUCCESS;
}

static int do_reset_transdtl(sp_context* ctx)
{
  uint8 ret = 0;
  uint32 page_no;
  sp_transno_unit transno_unit;
  //重置主存储区
  page_no = ADDR_TRANSNO_MASTER / FLASH_PAGE_SIZE;
  ret = sp_flashpage_erase(page_no);
  if(ret)
  {
    return SP_E_FLASH_ERASE;
  }
  //重置从存储区
  page_no = ADDR_TRANSNO_SLAVE / FLASH_PAGE_SIZE;
  ret = sp_flashpage_erase(page_no);
  if(ret)
  {
    return SP_E_FLASH_ERASE;
  }
  memset(&transno_unit, 0, sizeof(sp_transno_unit));
  transno_unit.last_trans_no = 0;
  transno_unit.last_trans_addr = ADDR_TRANSDTL_START;
  memcpy(transno_unit.date, ctx->current_datetime, 3);
  sp_protocol_crc((uint8*)&transno_unit, sizeof(sp_transno_unit) - 2, transno_unit.crc);
  //写主流水号存储区
  page_no = ADDR_TRANSNO_MASTER / FLASH_PAGE_SIZE;
  ret = sp_flash_write(page_no, 0, (uint8*)&transno_unit, sizeof(sp_transno_unit), 0);
  if(ret)
  {
    return SP_E_FLASH_WRITE;
  }
  //写从流水号存储区
  page_no = ADDR_TRANSNO_SLAVE / FLASH_PAGE_SIZE;
  ret = sp_flash_write(page_no, 0, (uint8*)&transno_unit, sizeof(sp_transno_unit), 0);
  if(ret)
  {
    return SP_E_FLASH_WRITE;
  }
  //再读一遍看看crc对不对?
  //sp_flash_read(uint32 page_no,uint32 offset_addr,uint8 * array,uint32 counter)
  ret = do_reset_transdtl_data(ctx);
  if(ret)
    return ret;
  return SP_SUCCESS;
}

static uint8 get_truss_index_now(sp_context* ctx)
{
  if(sp_is_in_period(SP_TRUSS_PERIOD0, ctx->syspara.truss_period1, ctx->current_datetime + 4))
    return 4;
  if(sp_is_in_period(ctx->syspara.truss_period1, ctx->syspara.truss_period2, ctx->current_datetime + 4))
    return 1;
  else if(sp_is_in_period(ctx->syspara.truss_period2, ctx->syspara.truss_period3, ctx->current_datetime + 4))
    return 2;
  else if(sp_is_in_period(ctx->syspara.truss_period3, SP_TRUSS_PERIOD0, ctx->current_datetime + 4))
    return 3;
  else// 不可能出现
    return 0;
}

//datetime---yyyymmddhhmmss
static void generate_trussno_now(sp_context* ctx, uint8 trussno[5])
{
  memcpy(trussno, ctx->current_datetime, 4);
  trussno[4] = get_truss_index_now(ctx);
  //表示是上一天的第四个批次
  if(trussno[4] == 4)
  {
    //日期减1
    sp_date_minues(trussno, 1);
  }
}

static int do_reset_trussdtl(sp_context* ctx)
{
  uint8 ret = 0;
  uint32 page_no;
  sp_trussno_unit trussno_unit;
  //重置主存储区
  page_no = ADDR_TRUSSNO_MASTER / FLASH_PAGE_SIZE;
  ret = sp_flashpage_erase(page_no);
  if(ret)
  {
    return SP_E_FLASH_ERASE;
  }
  //重置从存储区
  page_no = ADDR_TRUSSNO_SLAVE / FLASH_PAGE_SIZE;
  ret = sp_flashpage_erase(page_no);
  if(ret)
  {
    return SP_E_FLASH_ERASE;
  }
  memset(&trussno_unit, 0, sizeof(sp_trussno_unit));
  trussno_unit.last_truss_addr = ADDR_TRUSSDTL_START;
  sp_get_time(ctx);
  memcpy(trussno_unit.last_truss_no, ctx->current_datetime, 4);
  trussno_unit.last_truss_no[4] = 0;//唯一一笔index=0的扎帐流水
  sp_protocol_crc((uint8*)&trussno_unit, sizeof(sp_trussno_unit) - 2, trussno_unit.crc);
  //写主批次号存储区
  page_no = ADDR_TRUSSNO_MASTER / FLASH_PAGE_SIZE;
  ret = sp_flash_write(page_no, 0, (uint8*)&trussno_unit, sizeof(sp_trussno_unit), 0);
  if(ret)
  {
    return SP_E_FLASH_WRITE;
  }
  //写从批次号存储区
  page_no = ADDR_TRUSSNO_SLAVE / FLASH_PAGE_SIZE;
  ret = sp_flash_write(page_no, 0, (uint8*)&trussno_unit, sizeof(sp_trussno_unit), 0);
  if(ret)
  {
    return SP_E_FLASH_WRITE;
  }
  ret = do_reset_trussdtl_data(ctx);
  if(ret)
    return ret;
  return SP_SUCCESS;
}

/*
*作用:检测扎帐存储区是否正常
*/
static int do_check_trussdtl(sp_context* ctx)
{
  sp_trussno_unit master_unit, slave_unit;
  sp_trussdtl trussdtl;
  uint8 ret = 0;
  //1、读取主扎帐号存储区
  ret = sp_read_trussno_unit(SP_FLAG_MASTER, &master_unit);
  if(ret)
  {
    return ret;
  }
  //2、读取从扎帐号存储区
  ret = sp_read_trussno_unit(SP_FLAG_SLAVE, &slave_unit);
  if(ret)
  {
    return ret;
  }
  if(master_unit.last_truss_addr != slave_unit.last_truss_addr)
  {
    return SP_E_FLASH_SEQNO_NOT_EQUAL;
  }
  //3、验证数据有效性
  ret = sp_read_trussdtl(&trussdtl, master_unit.last_truss_addr);
  if(ret)
  {
    sp_disp_error("读取扎帐流水失败,ret=%d", ret);
    return ret;
  }
  if(memcmp(trussdtl.truss_no, master_unit.last_truss_no, 5) != 0)
  {
    sp_disp_error("扎帐流水数据中的批次号不正确，扎帐存储区将重置");
    return SP_E_FLASH_SEQNO_NOT_EQUAL;
  }
  return SP_SUCCESS;
}

/*
*作用:检测流水存储区是否正常
*/
static int do_check_transdtl(sp_context* ctx)
{
  sp_transno_unit master_unit, slave_unit;
  sp_transdtl transdtl;
  uint8 ret = 0;
  //1、读取主流水号存储区
  ret = sp_read_transno_unit(SP_FLAG_MASTER, &master_unit);
  if(ret)
  {
    return ret;
  }
  //2、读取从流水号存储区
  ret = sp_read_transno_unit(SP_FLAG_SLAVE, &slave_unit);
  if(ret)
  {
    return ret;
  }
  if(master_unit.last_trans_no != slave_unit.last_trans_no)
  {
    return SP_E_FLASH_SEQNO_NOT_EQUAL;
  }
  //3、验证数据有效性
  ret = sp_read_transdtl(&transdtl, master_unit.last_trans_addr);
  if(ret)
  {
    sp_disp_error("读取流水失败,ret=%d", ret);
    return ret;
  }
  sp_disp_debug("termseqno=%d,last_trans_no=%d", transdtl.termseqno, master_unit.last_trans_no);
  if(transdtl.termseqno != master_unit.last_trans_no)
  {
    sp_disp_error("流水数据中的流水号与流水号存储区的流水号不同，流水以及流水号存储区将重置");
    return SP_E_FLASH_SEQNO_NOT_EQUAL;
  }
  return SP_SUCCESS;
}

static int do_init_blacklist(sp_context* ctx)
{
  return SP_SUCCESS;
}

static int do_init_syspara(sp_context* ctx)
{
  int ret = 0;
  memset(&ctx->syspara, 0, sizeof(sp_syspara));
  //其他
  ctx->syspara.work_mode = SP_WORK_MODE_NORMAL;
  ctx->syspara.max_cardbal = 100;
  ctx->syspara.return_flag = 0;
  ctx->syspara.offline_flag = 0;
  ctx->syspara.min_cardbal = 1000;//十块钱
  ctx->syspara.oper_timeout = 20;
  ctx->syspara.heartgap = 60;
  ctx->syspara.once_limit_amt = 0;
  ctx->syspara.day_sum_limit_amt = 50000;//五百元
  ctx->syspara.limit_switch = 1;
  memcpy(ctx->syspara.hd_verno, "\x09\x09\x09\x09\x09", 5);
  memcpy(ctx->syspara.soft_verno, "\x00\x00\x01", 3);
  ctx->syspara.key_index = '\x01';
  ctx->syspara.yesterday_total_amt = 0;
  ctx->syspara.yesterday_total_cnt = 0;
  ctx->syspara.today_total_amt = 0;
  ctx->syspara.today_total_cnt = 0;
  memcpy(ctx->syspara.password, "\x31\x31\x31\x31\x31\x31", 6);
  ctx->syspara.max_pay_cnt = 5000;
  ctx->syspara.offline_flag = 1;
  ctx->syspara.offline_days = 0;
  ctx->syspara.max_offline_days = 10;
  //流水统计
  ctx->syspara.total_transaddr = ADDR_TRANSDTL_START;
  ctx->syspara.total_transno = 0;
  ctx->syspara.confirmed_transaddr = ADDR_TRANSDTL_START;
  ctx->syspara.confirmed_transno = 0;
  //扎帐统计
  ctx->syspara.total_trussaddr = ADDR_TRUSSDTL_START;
  //ctx->syspara.total_trussno = 0;
  ctx->syspara.confirmed_trussaddr = ADDR_TRUSSDTL_START;
  //ctx->syspara.confirmed_trussno = 0;
  ctx->syspara.enable_staus = SP_R_DEVICE_NOT_ENABLE;
  //扎帐
  memcpy(ctx->syspara.truss_period1, "\x07\x30", 2);
  memcpy(ctx->syspara.truss_period2, "\x13\x00", 2);
  memcpy(ctx->syspara.truss_period3, "\x19\x00", 2);
  //影响硬件的
  memcpy(ctx->syspara.blacklist_verno, "\x14\x11\x07\x15\x05\x09\x11", 7);
  ctx->syspara.syspara_verno = 0;
  ctx->syspara.feepara_verno = 0;
  memcpy(ctx->syspara.today_date, "\x20\x13\x12\x08", 4);
  //提示用户输入设备编号和设备机号
  ret = sp_input_termno(ctx);
  if(ret)
    return ret;
  ret = sp_input_canid(ctx);
  if(ret)
    return ret;
  ctx->syspara.init_flag = 1;
  ret = sp_write_syspara(ctx);
  if(ret)
    return ret;
  return SP_SUCCESS;
}

static int do_init_feerate_table(sp_context* ctx)
{
  int ret = 0;
  ret = sp_write_feerate_table(ctx);
  if(ret)
    return ret;
  return SP_SUCCESS;
}

static int do_init_timepara_table(sp_context* ctx)
{
  int ret = 0;
  ret = sp_write_timepara_table(ctx);
  if(ret)
    return ret;
  return SP_SUCCESS;
}

static int check_heart_beat_time(sp_context* ctx)
{
  static SYSTIME old_tm;
  static uint8 is_first = 1;
  SYSTIME new_tm;
  uint32 gapsec = 0;
  if(is_first)
  {
    sp_get_hdtime(ctx, &old_tm);
    is_first = 0;
    return 0;
  }
  sp_get_hdtime(ctx, &new_tm);
  gapsec = sp_calc_diff_rough_seconds(&new_tm, &old_tm);
  if(gapsec > ctx->syspara.heartgap)
  {
    old_tm = new_tm;
    return 1;
  }
  return 0;
}


static void react_by_type(sp_context* ctx, uint8 err_code)
{
  switch(err_code)
  {
    case SP_E_CARD_TYPE_NOT_SUPPORT:
      sp_disp_msg(3, "该卡类别不支持");
      ctx->disp_type = 0;
      break;
    case SP_NO_NUMBER_KEY_PRESSED:
    case SP_R_CANCEL_CONSUME:
    case SP_R_REQUEST_SAME_CARD:
    case SP_E_INIT4PURCHASE:
      break;
    case SP_E_FLASH_ERASE:
    case SP_E_FLASH_READ:
    case SP_E_FLASH_WRITE:
    case SP_FAIL:
      sp_handle_fatal_error(ctx, err_code);
      break;
    case SP_E_INSUFFICIENT_BALANCE:
      //显示余额不足
      sp_disp_by_type(SP_TP_INSUFFICIENT_BALANCE, ctx, NULL);
      break;
    case SP_E_DATE_REVERSE:
      sp_set_bit(&ctx->global_status, SP_STATUS_RESET_TIME);
      //sp_reset_hdtime(EXTER_RTC);
      break;
    case SP_R_OFFLINE_DAYS_EXCEED:
      sp_disp_error("脱机超过最大脱机时间有效期，或者日期异常导致");
      break;
    default:
      sp_disp_error("消费错误, 错误码=%04x", err_code);
      ctx->disp_type = 0;
      break;
  }
}
static void do_reset_consume_para(sp_context* ctx)
{
  ctx->disp_type = 0;
  memset(ctx->last_cardphyid, 0, 8);
}

/***********************************************************************************
********************************  扎帐相关*************************************
**************************************************************************************/
static uint8 is_success_by_flag(uint8 transflag)
{
  return 0;
}

static void do_add_one_transdtl_to_trussdtl(sp_transdtl* transdtl, sp_trussdtl* trussdtl)
{
  if(is_success_by_flag(transdtl->transflag))
  {
    trussdtl->success_amount += transdtl->amount;
    trussdtl->success_count ++;
  }
  else
  {
    trussdtl->fail_amount += transdtl->amount;
    trussdtl->fail_count ++;
  }
}



//把扎帐数据补完，都用0来赋值
static void complete_trussdtls_by_index(sp_context* ctx, uint8 index_start, uint8 index_end, uint8 date[4], uint32 end_trans_addr)
{
  int ret = 0;
  uint8 i = 0;
  sp_trussdtl trussdtl;
  if(index_start == 0)
    index_start = 1;
  if(index_end > 4)
    index_end = 4;
  if(index_start > index_end)
    return;
  for(i = index_start; i <= index_end; i++)
  {
    //memset
    memset(&trussdtl, 0, sizeof(sp_trussdtl));
    memcpy(trussdtl.truss_no, date, 4);
    trussdtl.truss_no[4] = i;
    trussdtl.end_transaddr = end_trans_addr;
    sp_protocol_crc((uint8*)(&trussdtl), sizeof(trussdtl) - 2, trussdtl.crc);
    ret = sp_write_trussdtl(ctx, &trussdtl);
    if(ret)
    {
      sp_handle_fatal_error(ctx, ret);
    };
    sp_sleep(10);//写flash之间留一点时间
  }
}


//统计某个批次号的，如果发现批次变化了就退出
static void do_trussdtl_by_trussno(sp_context* ctx, uint8 trussno[5], uint32 trans_addr)
{
  uint32 next_addr = 0;
  int ret = 0;
  sp_transdtl transdtl;
  sp_trussdtl trussdtl;
  uint8 trans_trussno[5];
  next_addr = trans_addr;
  memset(&trussdtl, 0, sizeof(sp_trussdtl));
  memcpy(trussdtl.truss_no, trussno, 5);
  trussdtl.end_transaddr = trans_addr;
  while(1)
  {
    ret = sp_read_transdtl(&transdtl, next_addr);
    if(ret)
    {
      sp_handle_fatal_error(ctx, ret);
    }
    sp_generate_trussno_by_transdatetime(ctx, transdtl.transdatetime, trans_trussno);
    if(memcmp(trans_trussno, trussno, 5) != 0)
      break;
    do_add_one_transdtl_to_trussdtl(&transdtl, &trussdtl);
    //下一笔流水
    next_addr = sp_get_next_transdtl_addr(next_addr);
  }
  sp_protocol_crc((uint8*)&trussdtl, sizeof(sp_trussdtl) - 2, trussdtl.crc);
  //可以保证有一笔该批次的流水
  ret = sp_write_trussdtl(ctx, &trussdtl);
  if(ret)
  {
    sp_handle_fatal_error(ctx, ret);
  }
}

static int do_trussdtl_until_des_trussno(sp_context* ctx, uint8 start_trussno[5], uint8 end_trussno[5], uint32 end_trans_addr)
{
  int next_addr = 0;
  sp_transdtl next_transdtl;
  int ret = 0;
  int cmp_ret = 0;
  uint8 new_trussno[5];

  next_addr = sp_get_next_transdtl_addr(end_trans_addr);//下一笔流水的地址
  ret = sp_read_transdtl(&next_transdtl, next_addr);
  if(ret)
  {
    return ret;
  }
  //根据下一笔流水时间产生一个新的扎帐批次号
  sp_generate_trussno_by_transdatetime(ctx, next_transdtl.transdatetime, new_trussno);
  cmp_ret = memcmp(start_trussno, new_trussno, 4);
  if(cmp_ret > 0) //startdate>transdate
  {
    //除非最后一笔扎帐流水的endtransdtladdr记录的有问题
    //否则不可能发生
    return SP_R_LOST_TRANSDTL;
  }
  else if(cmp_ret < 0) //startdate<transdate
  {
    //startdate后面几笔补充完
    complete_trussdtls_by_index(ctx, start_trussno[4] + 1, 4, start_trussno, end_trans_addr);
    cmp_ret = memcmp(new_trussno, end_trussno, 4);
    if(cmp_ret > 0) //transdate>enddate
      return SP_E_DATE_REVERSE;
    else if(cmp_ret == 0) //startdate<transdate=enddate
    {
      complete_trussdtls_by_index(ctx, 1, end_trussno[4] - 1, end_trussno, end_trans_addr);
      do_trussdtl_by_trussno(ctx, end_trussno, end_trans_addr);
    }
    else//startdate<transdate<enddate
    {
      //把该笔流水时间对应的扎帐点之前的扎帐流水补完(比如index=3那么要把1和2的批次补完)
      complete_trussdtls_by_index(ctx, 1, new_trussno[4] - 1, new_trussno, end_trans_addr);
      //从该流水对应的批次号开始一直到当前
      return do_trussdtl_until_des_trussno(ctx, new_trussno, end_trussno, end_trans_addr);
    }
  }
  else//startdate=transdate
  {
    cmp_ret = memcmp(new_trussno, end_trussno, 4);
    if(cmp_ret > 0) //transdate>enddate
      return SP_E_DATE_REVERSE;
    else if(cmp_ret == 0) //transdate=enddate
    {
      //把startdate->trussno到end_trussno之间的补完(如:1到4的话就要补上2和3)
      complete_trussdtls_by_index(ctx, start_trussno[4] + 1, end_trussno[4] - 1, end_trussno, end_trans_addr);
      do_trussdtl_by_trussno(ctx, end_trussno, end_trans_addr);
    }
    else//transdate<enddate
    {
      //把startdate->trussno到end_trussno之间的补完(如:1到4的话就要补上2和3)
      complete_trussdtls_by_index(ctx, start_trussno[4] + 1, new_trussno[4] - 1, new_trussno, end_trans_addr);
      //从该流水对应的批次号开始一直到当前
      return do_trussdtl_until_des_trussno(ctx, new_trussno, end_trussno, end_trans_addr);
    }
  }
  return 0;
}

static int do_truss_account(sp_context* ctx, uint8 start_trussno[5], uint32 end_transaddr)
{
  uint8 des_trussno[5];
  //根据当前时间产生扎帐批次号
  generate_trussno_now(ctx, des_trussno);
  return do_trussdtl_until_des_trussno(ctx, start_trussno, des_trussno, end_transaddr);
}


/////////////////////////////////////外部函数///////////////////////////////////////////
void sp_heartbeat(sp_context* ctx)
{
  int ret = 0;
  if(check_heart_beat_time(ctx))
  {
    //发送心跳
    ret = sp_send_heartbeat(ctx);
    if(ret)
    {
      ctx->online_flag = false;
      if(ctx->syspara.offline_days <= 0)
      {
        ctx->syspara.offline_days = 1;
      }
    }
    else
    {
      ctx->online_flag = true;
      ctx->syspara.offline_days = 0;
    }
  }
}

uint8 sp_consume_fixed_value(sp_context* ctx)
{
  sp_disp_msg(1000, "使用定值消费");
  return 0;
}

uint8 sp_pre_check(sp_context* ctx)
{
  sp_check_record_rom(ctx);
  if(ctx->rec_flash_full)
  {
    sp_disp_error("流水存储已满，请采集");
    return SP_E_FLASH_TRANSDTL_FULL;
  }
  return SP_SUCCESS;
}

static uint8 need_do_trussdtl(sp_context* ctx, uint8 start_trussno[5], uint32* end_transaddr)
{
  uint8 now_trussno[5];
  uint32 next_addr = 0;
  sp_transdtl next_transdtl;
  int ret = 0;
  sp_trussno_unit commit_trussno_unit;
  sp_trussdtl commit_trussdtl;

  generate_trussno_now(ctx, now_trussno);
  sp_get_trussno_unit(&commit_trussno_unit);
  //看看是否已经扎帐过该批次
  if(memcmp(commit_trussno_unit.last_truss_no, now_trussno, 5) >= 0)
    return 0;
  //尝试读一下流水
  sp_read_trussdtl(&commit_trussdtl, commit_trussno_unit.last_truss_addr);
  next_addr = sp_get_next_transdtl_addr(commit_trussdtl.end_transaddr);//下一笔流水的地址
  ret = sp_read_transdtl(&next_transdtl, next_addr);
  if(ret)
  {
    //表示没有流水，无需扎帐
    return 0;
  }
  //赋值
  (*end_transaddr) = commit_trussdtl.end_transaddr;
  memcpy(start_trussno, commit_trussdtl.truss_no, 5);

  return 1;
}

int sp_consume_normal(sp_context * ctx)
{
  int ret = 0;
  int32 key = 0;
  uint8 start_trussno[5];
  uint32 end_transaddr = 0;
  do_reset_consume_para(ctx);
  SP_CLS_FULLSCREEN;
  while(1)
  {
    sp_sleep(3);
    ret = sp_check_and_enable_device(ctx);
    if(ret)
      return ret;
    if(ctx->online_flag == true)
    {
      sp_send_rt_transdtl(ctx);
      //上传扎帐流水
      sp_send_trussdtl(ctx);
    }
    key = sp_get_key();
    if(SP_KEY_FUNC == key)
      return 0;
    //检查是否有存储空间
    ret = sp_pre_check(ctx);
    if(ret)
    {
      sp_disp_msg(10, "存储空间不足");
      continue;
    }
    ret = sp_check_date_change(ctx);
    if(ret)
    {
      if(SP_R_OFFLINE_DAYS_EXCEED == ret)
      {
        sp_disp_msg(5, "超过最大脱机天数，不允许使用");
      }
      sp_disp_error("致命错误, ret=%04x,[%02x%02x%02x%02x]->[%02x%02x%02x%02x%02x%02x%02x]", ret,
                    ctx->syspara.today_date[0], ctx->syspara.today_date[1], ctx->syspara.today_date[2], ctx->syspara.today_date[3],
                    ctx->today[0], ctx->today[1], ctx->today[2], ctx->today[3], ctx->current_datetime[4], ctx->current_datetime[5], ctx->current_datetime[6]);
      sp_set_bit(&ctx->global_status, SP_STATUS_RESET_TIME);
      continue;
      //return ret;
    }
    if(need_do_trussdtl(ctx, start_trussno, &end_transaddr))
    {
      sp_disp_by_type(SP_TP_TRUSSDTL, ctx, NULL);
      do_truss_account(ctx, start_trussno, end_transaddr);
      continue;
    }
    ret = sp_prcocess_message(ctx, SP_SWITCH_OFF);
    if(ret)
    {
      sp_disp_debug("proc msg,ret=%04x", ret);
    }
    ret = sp_consume_loop(ctx);
    if(SP_R_PRESS_FUNC == ret)
    {
      return 0;
    }
    if(ret)
    {
      react_by_type(ctx, ret);
    }
    Reset_Reader_TimeOut();
    //500ms或1S刷新一次 从Flash读取点阵需要时间可放定时器回调函数处理
    //喂狗定义防止溢出程序复位
    //  KillWatchDog();
    sp_heartbeat(ctx);
  }
}

static void do_modify_termno(char termno[8], char modify_termno[8])
{
  uint16 len = 0;
  len = strlen(termno);
  memset(modify_termno, '\x30', 8);
  memcpy(modify_termno + 8 - len, termno, len);
}

int sp_input_termno(sp_context* ctx)
{
  int ret = 0;
  char termno[MAX_SCREEN_ROWS];
  char final_termno[MAX_SCREEN_ROWS];
  ret = sp_input_words(ctx, "终端编号: ", termno, 8);
  if(ret)
    return ret;
  sp_disp_debug("%02x%02x%02x%02x%02x%02x%02x%02x", termno[0], termno[1], termno[2], termno[3], termno[4],
                termno[5], termno[6], termno[7]);
  //\x31\x00\x00\x00\x00\x00\x00\x00
  do_modify_termno(termno, final_termno);
  sp_string_to_bcd(final_termno, ctx->syspara.termno, 4);

  return 0;
}

int sp_input_canid(sp_context* ctx)
{
  int ret = 0;
  char canid[MAX_SCREEN_ROWS];
  ret = sp_input_words(ctx, "终端机号: ", canid, 8);
  if(ret)
    return ret;
  ctx->syspara.canid = atoi(canid);
  return 0;
}

uint8 sp_read_system(sp_context* ctx)
{
  int ret = 0;
  memset(ctx, 0, sizeof(sp_context));
  //读取黑名单
  ret = sp_read_blacklist(ctx);
  if(ret)
    return ret;
  //读取系统参数
  ret = sp_read_syspara(ctx);
  if(ret)
    return ret;
  //读取费率
  ret = sp_read_feerate_table(ctx);
  if(ret)
    return ret;
  //读取时间段
  ret = sp_read_timepara_table(ctx);
  if(ret)
    return ret;
  //检查流水存储区
  ret = do_check_transdtl(ctx);
  if(ret)
    return ret;
  //检查扎帐存储区
  ret = do_check_trussdtl(ctx);
  if(ret)
    return ret;
  return SP_SUCCESS;
}

uint8 sp_recover_device(sp_context* ctx)
{
  int ret = 0;
  SP_CLS_FULLSCREEN;
  SP_PRINT(1, 0, "正在初始化系统参数...");
  ret = do_init_syspara(ctx);
  if(ret)
    return ret;
  SP_CLS_FULLSCREEN;
  SP_PRINT(1, 0, "正在初始化黑名单...");
  ret = do_init_blacklist(ctx);
  if(ret)
    return ret;
  SP_PRINT(1, 0, "正在初始化费率表...");
  ret = do_init_feerate_table(ctx);
  if(ret)
    return ret;
  SP_PRINT(1, 0, "正在初始化时间段表...");
  ret = do_init_timepara_table(ctx);
  if(ret)
    return ret;
  sp_print_row(FLAG_CLEAR, 1, 0, "正在初始化流水区 ...");
  ret = do_reset_transdtl(ctx);
  if(ret)
    return ret;
  sp_print_row(FLAG_CLEAR, 1, 0, "正在初始化扎帐区 ...");
  ret = do_reset_trussdtl(ctx);
  if(ret)
    return ret;
  sp_print_row(FLAG_CLEAR, 1, 0, "初始化成功!!");
  return SP_SUCCESS;
}



int sp_check_date_change(sp_context* ctx)
{
  int diff_days = 0;
  sp_get_time(ctx);
  //检查脱机使用天数
  if(ctx->syspara.offline_flag)
  {
    if(ctx->syspara.offline_days >= ctx->syspara.max_offline_days)
      return SP_R_OFFLINE_DAYS_EXCEED;
  }
  diff_days = sp_calc_diff_days(ctx->today, ctx->syspara.today_date);
  if(diff_days > ctx->syspara.max_offline_days)
    return SP_R_OFFLINE_DAYS_EXCEED;
  //查看日期是否有切换,并且是切换了1天
  if(diff_days > 1)
  {
    ctx->syspara.yesterday_total_cnt = 0;
    ctx->syspara.yesterday_total_amt = 0;
    ctx->syspara.today_total_cnt = 0;
    ctx->syspara.today_total_amt = 0;
    memcpy(ctx->syspara.today_date, ctx->today, 4);
    return sp_write_syspara(ctx);
  }
  else if(diff_days == 1)
  {
    ctx->syspara.yesterday_total_cnt = ctx->syspara.today_total_cnt;
    ctx->syspara.yesterday_total_amt = ctx->syspara.today_total_amt;
    ctx->syspara.today_total_cnt = 0;
    ctx->syspara.today_total_amt = 0;
    memcpy(ctx->syspara.today_date, ctx->today, 4);
    return sp_write_syspara(ctx);
  }
  else if(diff_days < 0)
  {
    //日期倒退
    //sp_disp_error("日期倒退，联系管理员");
    memcpy(ctx->syspara.today_date, ctx->today, 4);
    sp_write_syspara(ctx);
    return SP_E_DATE_REVERSE;
  }
  return 0;
}

uint16 sp_get_offline_transdtl_amout(sp_context* ctx)
{
  return 0;
}

int sp_check_deviceid(sp_context* ctx)
{
  //ctx->syspara.enable_staus = SP_SUCCESS;//SP_S_DEVICEID_NOT_MATCH;
  return 0;
}

uint8 sp_recover_system(sp_context* ctx)
{
  uint8 choice = 0;
  int ret = 0;
  //提示是否初始化设备
  sp_disp_by_type(SP_TP_IS_INIT_DEVICE, ctx, &choice);
  if(choice)
  {
    sp_disp_msg(999, "已取消初始化设备,设备可能会不可使用");
    return SP_R_CANCEL_CONSUME;
  }
  //恢复出厂设置
  ret = sp_recover_device(ctx);
  if(ret)
  {
    sp_disp_error("恢复出厂设置失败!!,ret=%04x", ret);
    return ret;
  }
  //启用设备
  ret = sp_enable_device(ctx);
  if(ret)
  {
    sp_disp_error("启用设备失败!!,ret=%04x", ret);
    ctx->syspara.enable_staus = ret;
    return ret;
  }
  return 0;
}

int sp_check_and_enable_device(sp_context* ctx)
{
  int ret = 0, ret1 = 0;
  uint8 choice = 0;
  if(SP_SUCCESS == ctx->syspara.enable_staus)
    return 0;
  sp_disp_by_type(SP_TP_IS_ENABLE_DEVICE, ctx, &choice);
  if(choice)
    return SP_R_DEVICE_NOT_ENABLE;
  ret = sp_enable_device(ctx);
  if(ret)
  {
    ret1 = sp_write_syspara(ctx);
    if(ret1)
    {
      sp_handle_fatal_error(ctx, ret1);
    }
    sp_disp_error("启用设备失败, ret=%04x", ret);
    return ret;
  }
  ret = sp_write_syspara(ctx);
  if(ret)
    sp_handle_fatal_error(ctx, ret);
  sp_sleep(1000);
  return 0;
}


int sp_enable_device(sp_context* ctx)
{
  int ret = 0;
  SP_CLS_FULLSCREEN;
  SP_PRINT(1, 0, "正在启用设备...");
  ret = sp_check_deviceid(ctx);
  if(ret)
    return ret;
  ret = sp_download_syspara(ctx);
  if(ret)
    return ret;
  ret = sp_download_feerate(ctx);
  if(ret)
    return ret;
  ret = sp_download_blacklist(ctx);
  if(ret)
    return ret;
  ret = sp_auth(ctx);
  if(ret)
    return ret;
  //启用psam卡
  ret = sp_enable_sam_card(ctx);
  if(ret)
  {
    return ret;
  }
  SP_PRINT(1, 0, "启用设备成功!! ");
  ctx->syspara.enable_staus = SP_SUCCESS;
  return 0;
}




