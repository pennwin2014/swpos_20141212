#include "sp_transfer.h"

static int do_send_recv_by_cmd(sp_context* ctx, uint8 cmd, void* arg, uint16 time_out)
{
  int ret = 0;
  uint8 time_out_cnt = time_out / 100;
  ret = sp_send_request(ctx, cmd, arg, time_out);
  if(ret)
    return ret;
  if(ret)
  {
    sp_disp_error("send cmd=[%02x] ret=%04x", cmd, ret);
    return ret;
  }
  while(1)
  {
    ret = sp_prcocess_message(ctx, SP_SWITCH_ON);
    if(ret == SP_SUCCESS)
      break;
    if(time_out_cnt -- <= 0)
      return SP_R_WAIT_TIMEOUT;
    sp_sleep(100);
  }
  return 0;
}

#define GAP_SECONDS 12
static uint8 can_send_msg(sp_context* ctx)
{
  static SYSTIME old_time;
  SYSTIME new_time;
  static bool is_first = 1;
  uint32 gap_secs = 0;
  if(is_first)
  {
    is_first = 0;
    sp_get_hdtime(ctx, &old_time);
    gap_secs = GAP_SECONDS + 1;
  }
  else
  {
    sp_get_hdtime(ctx, &new_time);
    gap_secs = sp_calc_diff_rough_seconds(&new_time, &old_time);
  }
  if(gap_secs > GAP_SECONDS)
  {
    sp_get_hdtime(ctx, &old_time);
    return 1;
  }
  return 0;
}

int sp_send_trussdtl(sp_context* ctx)
{
  int ret = 0;
  sp_trussdtl read_trussdtl;
  int next_truss_addr = 0;
  if(memcmp(ctx->syspara.total_trussno, ctx->syspara.confirmed_trussno, 5)>0)
  {
    next_truss_addr = get_next_trussdtl_addr(ctx->syspara.confirmed_trussaddr);
    ret = sp_read_trussdtl(&read_trussdtl, next_truss_addr);
    if(ret)
      return ret;
    sp_sleep(1000);
    sp_send_request(ctx, SP_CMD_TRUSSDTL, &read_trussdtl, 3000);
  }
  return 0;
}


int sp_send_rt_transdtl(sp_context* ctx)
{
  int ret = 0;
  sp_transdtl read_transdtl;
  int next_trans_addr = 0;
  if(ctx->syspara.total_transno > ctx->syspara.confirmed_transno)
  {
    next_trans_addr = sp_get_next_transdtl_addr(ctx->syspara.confirmed_transaddr);
    ret = sp_read_transdtl(&read_transdtl, next_trans_addr);
    if(ret)
      return ret;
    sp_sleep(1000);
    sp_send_request(ctx, SP_CMD_RT_TRANSDTL, &read_transdtl, 3000);
  }
  return 0;
}

int sp_send_local_transdtl(sp_context* ctx)
{
  int ret = 0;
  sp_transdtl read_transdtl;
  while(1)
  {
    if(ctx->syspara.confirmed_transno >= ctx->syspara.total_transno)
    {
      break;
    }
    ret = sp_read_transdtl(&read_transdtl, ctx->syspara.confirmed_transaddr);
    if(ret)
      return ret;
    ret = do_send_recv_by_cmd(ctx, SP_CMD_RT_TRANSDTL, &read_transdtl, 3000);
    if(ret)
      return ret;
  }
  return 0;
}

int sp_download_feerate(sp_context* ctx)
{
  int ret = 0;
  ret = do_send_recv_by_cmd(ctx, SP_CMD_GET_FEERATE, NULL, 3000);
  if(ret)
    return ret;
  return 0;
}

int sp_download_syspara(sp_context* ctx)
{
  int ret = 0;
  ctx->svrpara.upd_flag = 0;
  ret = do_send_recv_by_cmd(ctx, SP_CMD_GET_SYSPARA, NULL, 3000);
  if(ret)
    return ret;
  if(ctx->svrpara.upd_flag)
  {
    ret = sp_write_syspara(ctx);
    if(ret)
      return ret;
  }
  return 0;
}

int sp_download_timepara(sp_context* ctx)
{
  return do_send_recv_by_cmd(ctx, SP_CMD_GET_TIMEPARA, NULL, 3000);
}

int sp_send_test(sp_context* ctx)
{
  return do_send_recv_by_cmd(ctx, SP_CMD_TEST, NULL, 500);
}

int sp_add_blacklist(sp_context* ctx)
{
  int ret = 0;
  ret = do_send_recv_by_cmd(ctx, SP_CMD_ADD_BLACKLIST, NULL, 5000);
  if(ret)
    return ret;
  if(memcmp(ctx->svrpara.blacklist_verno, ctx->syspara.blacklist_verno, 6) == 0)
    return SP_S_BLACKLIST_VERNO_NOT_CHANGE;
  //设置黑名单
  ret = sp_set_blacklist_by_cardno(ctx);
  if(ret)
    return ret;
  memcpy(ctx->syspara.blacklist_verno, ctx->svrpara.blacklist_verno, 6);
  ret = sp_write_syspara(ctx);
  if(ret)
    return ret;
  return 0;
}

int sp_download_blacklist(sp_context* ctx)
{
  int ret = 0;
  ctx->svrpara.end_flag = 0;
  ctx->svrpara.buffer_size = 0;
  ctx->svrpara.flash_offset = 0;
  ctx->svrpara.download_index = 0;
  while(!ctx->svrpara.end_flag)
  {
    ret = do_send_recv_by_cmd(ctx, SP_CMD_GET_BLACKLIST, NULL, 1500);
    if(ret)
      return ret;
    ctx->svrpara.download_index ++;
    if(ctx->svrpara.buffer_size >= FLASH_PAGE_SIZE)
    {
      ret = sp_write_blacklist(ctx, ctx->svrpara.flash_offset);
      if(ret)
        return ret;
      //拷贝一下
      ctx->svrpara.buffer_size -= FLASH_PAGE_SIZE;
      if(ctx->svrpara.buffer_size > 0)
      {
        memcpy(ctx->svrpara.tmp_blacklist_bitmap, ctx->svrpara.tmp_blacklist_bitmap + FLASH_PAGE_SIZE, ctx->svrpara.buffer_size);
      }
      ctx->svrpara.flash_offset += FLASH_PAGE_SIZE;
    }
  }
  return 0;
}

int sp_auth(sp_context* ctx)
{
  int ret = 0;
  uint8 need_upd_syspara = 0;
  ret = do_send_recv_by_cmd(ctx, SP_CMD_AUTH, NULL, 2000);
  if(ret)
    return ret;
  //系统参数版本号
  if(ctx->svrpara.syspara_verno != ctx->syspara.syspara_verno)
  {
    ret = sp_download_syspara(ctx);
    if(ret)
      return ret;
    ctx->syspara.syspara_verno = ctx->svrpara.syspara_verno;
    need_upd_syspara = 1;
  }
  //费率版本号
  if(ctx->svrpara.feepara_verno != ctx->syspara.feepara_verno)
  {
    ret  = sp_download_feerate(ctx);
    if(ret)
    {
      return ret;
    }
    ctx->syspara.feepara_verno = ctx->svrpara.feepara_verno;
    // DO:更新支持卡类别位图的flash中的存储
    need_upd_syspara = 1;
    ret = sp_write_feerate_table(ctx);
    if(ret)
      return ret;
    sp_sleep(100);//目的:两次写flash间隔一些时间
  }
  //时间段参数版本号
  if(ctx->svrpara.timepara_verno != ctx->syspara.timepara_verno)
  {
    ret = sp_download_timepara(ctx);
    if(ret)
      return ret;
    ctx->syspara.timepara_verno = ctx->svrpara.timepara_verno;
    need_upd_syspara = 1;
    ret = sp_write_timepara_table(ctx);
    if(ret)
      return ret;
    sp_sleep(100);//目的:两次写flash间隔一些时间
  }
  if(need_upd_syspara)
  {
    ret = sp_write_syspara(ctx);
    if(ret)
      return ret;
  }
  return 0;
}


int sp_send_heartbeat(sp_context* ctx)
{
  int ret = 0;
  uint8 need_upd_syspara = 0;
  if(!can_send_msg(ctx))
  {
    return SP_E_EXCESSIVE_CALL;
  }
  ret = do_send_recv_by_cmd(ctx, SP_CMD_HEARTBEAT, NULL, 500);
  if(ret)
    return ret;
  //比较黑名单版本号
  if(memcmp(ctx->syspara.blacklist_verno, ctx->svrpara.blacklist_verno, 6) != 0)
  {
    //更新黑名单
    ret = sp_add_blacklist(ctx);
    if(ret)
      return ret;
    memcpy(ctx->syspara.blacklist_verno, ctx->svrpara.blacklist_verno, 6);
    need_upd_syspara = 1;
  }
  //系统参数版本号
  if(ctx->svrpara.syspara_verno != ctx->syspara.syspara_verno)
  {
    ret = sp_download_syspara(ctx);
    if(ret)
      return ret;
    ctx->syspara.syspara_verno = ctx->svrpara.syspara_verno;
    need_upd_syspara = 1;
  }
  //费率版本号
  if(ctx->svrpara.feepara_verno != ctx->syspara.feepara_verno)
  {
    ret  = sp_download_feerate(ctx);
    if(ret)
    {
      return ret;
    }
    ctx->syspara.feepara_verno = ctx->svrpara.feepara_verno;
    // DO:更新支持卡类别位图的flash中的存储
    need_upd_syspara = 1;
    ret = sp_write_feerate_table(ctx);
    if(ret)
      return ret;
    sp_sleep(100);//目的:两次写flash间隔一些时间
  }
  //时间段参数版本号
  if(ctx->svrpara.timepara_verno != ctx->syspara.timepara_verno)
  {
    ret = sp_download_timepara(ctx);
    if(ret)
      return ret;
    ctx->syspara.timepara_verno = ctx->svrpara.timepara_verno;
    need_upd_syspara = 1;
    ret = sp_write_timepara_table(ctx);
    if(ret)
      return ret;
    sp_sleep(100);//目的:两次写flash间隔一些时间
  }
  if(need_upd_syspara)
  {
    ret = sp_write_syspara(ctx);
    if(ret)
      return ret;
  }
  return 0;
}




