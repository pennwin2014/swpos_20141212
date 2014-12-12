#include "sp_menu.h"
#include <time.h>

//////////////////////工具函数/////////////////
static int do_check_sys_passwd(sp_context* ctx, char input_password[MAX_SCREEN_ROWS])
{
  int ret = 0;
  while(1)
  {
    ret = sp_input_password(ctx, "请输入管理密码", input_password, SYSTEM_PASSWORD_LEN);
    if(ret)
      return SP_E_SYS_PASSWD;
    if(memcmp(ctx->syspara.password, input_password, 6) != 0)
    {
      sp_disp_error("管理密码错误,sys=[%02x%02x%02x%02x%02x%02x],in=[%02x%02x%02x%02x%02x%02x]", ctx->syspara.password[0],
                    ctx->syspara.password[1], ctx->syspara.password[2], ctx->syspara.password[3], ctx->syspara.password[4], ctx->syspara.password[5],
                    input_password[0], input_password[1], input_password[2], input_password[3], input_password[4], input_password[5]);
      continue;
    }
    break;
  }
  return 0;
}

//////////////////////菜单相关//////////////////////
static void do_set_password(sp_context* ctx)
{
  char input_password[MAX_SCREEN_ROWS];
  char renew_password[MAX_SCREEN_ROWS];
  int ret = 0;
  if(do_check_sys_passwd(ctx, input_password))
    return;
  while(1)
  {
    //请输入新密码
    while(1)
    {
      ret = sp_input_password(ctx, "请输入新密码", input_password, SYSTEM_PASSWORD_LEN);
      if(ret)
        return;
      if(strlen(input_password) != SYSTEM_PASSWORD_LEN)
      {
        sp_disp_msg(6, "密码长度错误");
        continue;
      }
      break;
    }
    //再次输入新密码
    ret = sp_input_password(ctx, "再次输入新密码", renew_password, SYSTEM_PASSWORD_LEN);
    if(ret)
      return;
    if(memcmp(renew_password, input_password, SYSTEM_PASSWORD_LEN) != 0)
    {
      sp_disp_msg(6, "两次密码不一致");
      continue;
    }
    break;
  }
  //修改密码成功
  memcpy(ctx->syspara.password, renew_password, 6);
  ret = sp_write_syspara(ctx);
  if(ret)
  {
    sp_disp_error("更新系统信息flash失败，ret=%04x", ret);
    return;
  }
  sp_disp_msg(6, "修改密码成功!! ");
}

static void do_set_termno(sp_context* ctx)
{
  int ret = 0;
  char sys_password[MAX_SCREEN_ROWS];
  if(do_check_sys_passwd(ctx, sys_password))
    return;
  ret = sp_input_termno(ctx);
  if(ret)
  {
    sp_disp_error("设置失败, ret=%04x", ret);
    return;
  }
  ret = sp_write_syspara(ctx);
  if(ret)
  {
    sp_disp_error("重写系统信息失败, ret=%04x", ret);
    return;
  }
  sp_disp_msg(8, "终端编号设置成功!! ");
}

static void do_set_communicate(sp_context* ctx)
{
  int ret = 0;
  char sys_password[MAX_SCREEN_ROWS];
  if(do_check_sys_passwd(ctx, sys_password))
    return;
  ret = sp_input_canid(ctx);
  if(ret)
  {
    sp_disp_error("设置失败, ret=%04x", ret);
    return;
  }
  ret = sp_write_syspara(ctx);
  if(ret)
  {
    sp_disp_error("重写系统信息失败, ret=%04x", ret);
    return;
  }
  sp_disp_msg(8, "通讯参数设置成功!! ");
}

static int do_check_unconfirm_transdtl(sp_context* ctx)
{
  uint32 unconfirm_transdtl_count = ctx->syspara.total_transno - ctx->syspara.confirmed_transno;
  if(unconfirm_transdtl_count > 0)
    return SP_R_TRANSDTL_UNCONFIRM;
  return 0;
}

static void do_set_recover(sp_context* ctx)
{
  int ret = 0;
  uint8 choice = 0;
  char input_password[MAX_SCREEN_ROWS];
  if(do_check_sys_passwd(ctx, input_password))
    return;
  // DO: 检查一下是否有未上传的流水
  if(do_check_unconfirm_transdtl(ctx))
  {
    sp_disp_by_type(SP_TP_DISP_UNCONFIM_TRANSDTL, ctx, &choice);
    if(choice)
      return;
  }
  SP_CLS_FULLSCREEN;
  SP_PRINT(1, 2, "正在恢复... ");
  ret = sp_recover_system(ctx);
  if(ret)
  {
    ret = sp_write_syspara(ctx);
    if(ret)
    {
      sp_handle_fatal_error(ctx, ret);
    }
    return;
  }
  sp_print_row(FLAG_CLEAR, 1, 2, "恢复成功!! ");
  sp_sleep(3000);
}

static void do_set_clear_local_transdtl(sp_context* ctx)
{
  uint8 choice = 0;
  char input_password[MAX_SCREEN_ROWS];
  if(do_check_sys_passwd(ctx, input_password))
    return;
  if(do_check_unconfirm_transdtl(ctx))
  {
    sp_disp_by_type(SP_TP_DISP_UNCONFIM_TRANSDTL, ctx, &choice);
    if(choice)
      return;
  }
  sp_disp_msg(8, "清空本地流水");
}

static void do_set_clear_blacklist(sp_context* ctx)
{
  sp_disp_msg(8, "清空黑名单");
}

static void do_menu_sign(sp_context* ctx)
{
  int ret = 0;
  SP_CLS_FULLSCREEN;
  SP_PRINT(1, 2, "正在签到...");
  ret = sp_auth(ctx);
  if(ret)
  {
    sp_disp_msg(3, "签到失败, ret=%04x", ret);
    return;
  }
  sp_print_row(FLAG_CLEAR, 1, 2, "签到成功!!");
  sp_sleep(2000);
}

static void do_menu_show_sysinfo(sp_context* ctx)
{
  uint8 upd_flag = 1;
  uint8 index = 0;
  int key = 0;
  uint8 page_count = SHOW_SYSTEM_INFO_COUNT;
  ctx->oper_upd_flag = 1;
  while(1)
  {
    if(sp_check_oper_timeout(ctx))
      return;
    if(upd_flag)
    {
      sp_disp_by_type(SP_TP_SHOW_SYSTEM_INFO, ctx, &index);
      upd_flag = 0;
    }
    key = sp_get_key();
    switch(key)
    {
      case SP_KEY_CLEAR:
        return;
      case SP_KEY_NEXT:
        if(index < page_count - 1)
        {
          ++index;
          upd_flag = 1;
        }
        break;
      case SP_KEY_PREV:
        if(index > 0)
        {
          --index;
          upd_flag = 1;
        }
        break;
    }

  }
}
   /*
static void do_menu_disp_transdtl(sp_context * ctx)
{
  uint8 ret = 0;
  int i = 0;
  int start_pos = 0;
  sp_transno_unit transno_unt;
  ret = sp_read_transno_unit(SP_FLAG_MASTER, &transno_unt);
  if(ret)
  {
    sp_disp_msg(999, "读取主流水号失败");
    return;
  }
  sp_disp_msg(999, "last_addr=%06x,last_no=%d", transno_unt.last_trans_addr, transno_unt.last_trans_no);
  if(transno_unt.last_trans_no > 2)
    start_pos = -2;
  for(i = start_pos; i < start_pos + 5; i++)
    sp_disp_flash(transno_unt.last_trans_addr + i * 64, 64); //打印五笔流水的数据
}
*/
static void do_menu_disp_trussdtl(sp_context * ctx)
{
  uint8 ret = 0;
  int i = 0;
  sp_trussno_unit trussno_unt;
  ret = sp_read_trussno_unit(SP_FLAG_MASTER, &trussno_unt);
  if(ret)
  {
    sp_disp_error("读取主扎帐批次号失败");
    return;
  }
  sp_disp_msg(999, "last_addr=%06x,last_no=%02x", trussno_unt.last_truss_addr, trussno_unt.last_truss_no[4]);
  for(i = 0; i < 5; i++)
    sp_disp_flash(ADDR_TRUSSDTL_START + i * 64, 64); //打印五笔流水的数据
}


static void do_menu_disp_transno(sp_context * ctx)
{
  int i = 0;
  int unit_len = sizeof(sp_transno_unit);
  sp_disp_msg(999, "主流水号存储区数据");
  for(i = 0; i < FLASH_PAGE_SIZE / unit_len; i++)
    sp_disp_flash(ADDR_TRANSNO_MASTER + i * unit_len, unit_len);
  sp_disp_msg(999, "从流水号存储区数据");
  for(i = 0; i < FLASH_PAGE_SIZE / unit_len; i++)
    sp_disp_flash(ADDR_TRANSNO_SLAVE + i * unit_len, unit_len);
}

static int do_cancel_last_trans(sp_card * card)
{
  sp_disp_msg(20, "撤销上一笔消费");
  return 0;
}

static void do_menu_cancel(sp_context * ctx)
{
  int key = 0;
  int ret = 0;
  //char phyid_hex[33] = {0};
  sp_card this_card;
  sp_disp_by_type(SP_TP_REQUEST_CARD, ctx, NULL);
  while(1)
  {
    key = sp_get_key();
    if(SP_KEY_CLEAR == key)
      return;
    ret = sp_request_card(this_card.cardphyid);
    if(ret == 0)
      break;
  }
  if(CPUCARD == this_card.cardtype)
  {
    sp_disp_msg(6, "不支持消费撤销");
    return;
  }
  //重新寻卡
  ret = sp_request_card_poweron(&this_card);
  if(ret)
  {
    sp_disp_error("sp_request_card_purchase,ret=%04x", ret);
    return;
  }
  ret = sp_read_card(ctx, &this_card, SP_FILE10 | SP_FILE15 | SP_FILE16 | SP_READ_CARDBAL);
  if(ret)
  {
    sp_disp_error("sp_read_card,ret=%04x", ret);
    return;
  }
  sp_disp_by_type(SP_TP_CANCEL_CONSUME, ctx, NULL);
  while(1)
  {
    key = sp_get_key();
    if(SP_KEY_CLEAR == key)
      return;
    if(SP_KEY_CONFIRM == key)
      do_cancel_last_trans(&this_card);
  }
}

static void do_disp_revenue(sp_context* ctx, uint8* init_flag)
{
  int key = 0;
  int show_cnt = 0;
  int show_amt = 0;
  uint8 upd_flag = 1;
  uint8 cur_opp_index = 0;
  if(*init_flag == 1)
  {
    show_cnt = ctx->syspara.today_total_cnt;
    show_amt = sp_d2i((ctx->syspara.today_total_amt * 1.0) / 100.0);
    cur_opp_index = 2;
  }
  else
  {
    show_cnt = ctx->syspara.yesterday_total_cnt;
    show_amt = sp_d2i((ctx->syspara.yesterday_total_amt * 1.0) / 100.0);
    cur_opp_index = 1;
  }
  ctx->oper_upd_flag = 1;
  while(1)
  {
    if(sp_check_oper_timeout(ctx))
      return;
    if(upd_flag)
    {
      SP_CLS_FULLSCREEN;
      if(cur_opp_index == 2)
      {
        SP_PRINT(0, 2, "当日营业额");
      }
      else
      {
        SP_PRINT(0, 2, "昨日营业额");
      }
      sp_print_row(FLAG_DO_NOT_CLEAR, 1, 0, "总笔数 :%d", show_cnt);
      sp_print_row(FLAG_DO_NOT_CLEAR, 2, 0, "总金额 :%d", show_amt);
      // sp_print_row(FLAG_DO_NOT_CLEAR,2, 0, "继续查询请按 :%d", cur_opp_index);
      //SP_PRINT(3, 0, "退出查询请按确认");
      upd_flag = 0;
    }
    key = sp_get_key();
    if(SP_KEY_1 == key)
    {
      if(cur_opp_index == 2)
        continue;
      show_cnt = ctx->syspara.today_total_cnt;
      show_amt = sp_d2i((ctx->syspara.today_total_amt * 1.0) / 100.0);
      cur_opp_index = 2;
      upd_flag = 1;
    }
    else if(SP_KEY_2 == key)
    {
      if(cur_opp_index == 1)
        continue;
      show_cnt = ctx->syspara.yesterday_total_cnt;
      show_amt = sp_d2i((ctx->syspara.yesterday_total_amt * 1.0) / 100.0);
      cur_opp_index = 1;
      upd_flag = 1;
    }
    else if(SP_KEY_CLEAR == key)
    {
      return;
    }
  }
}

static void do_menu_query_revenue(sp_context * ctx)
{
  int key = 0;
  uint8 init_disp_flag = 0;
  uint8 upd_flag = 1;
  ctx->oper_upd_flag = 1;
  while(1)
  {
    if(sp_check_oper_timeout(ctx))
      return;
    if(upd_flag)
    {
      SP_CLS_FULLSCREEN;
      SP_PRINT(1, 0, "1.当日营业额查询");
      SP_PRINT(2, 0, "2.昨日营业额查询");
      upd_flag = 0;
    }
    key = sp_get_key();
    switch(key)
    {
      case SP_KEY_1:
        init_disp_flag = 1;
        do_disp_revenue(ctx, &init_disp_flag);
        // sp_disp_by_type(SP_TP_REVENUE_QUERY, ctx, &init_disp_flag);
        upd_flag = 1;
        break;
      case SP_KEY_2:
        init_disp_flag = 2;
        do_disp_revenue(ctx, &init_disp_flag);
        // sp_disp_by_type(SP_TP_REVENUE_QUERY, ctx, &init_disp_flag);
        upd_flag = 1;
        break;
      case SP_KEY_CLEAR:
        return;
    }
  }
}

static void do_menu_update_blacklist(sp_context * ctx)
{
  int ret = 0;
  SP_CLS_FULLSCREEN;
  SP_PRINT(1, 0, "正在更新黑名单..");
  ret = sp_add_blacklist(ctx);
  if(ret)
  {
    if(SP_S_BLACKLIST_VERNO_NOT_CHANGE == ret)
    {
      sp_disp_msg(3, "黑名单无需更新!! ");
      return;
    }
    sp_disp_msg(6, "更新黑名单失败，请稍后重试!! ret=%04x", ret);
    return;
  }
  sp_print_row(FLAG_CLEAR, 1, 3, "更新成功!! ");
  sp_sleep(1500);
}

static void do_menu_upload_transdtl(sp_context * ctx)
{
  int ret = 0;
  SP_CLS_FULLSCREEN;
  SP_PRINT(1, 2, "正在上传..");
  ret = sp_send_local_transdtl(ctx);
  if(ret)
  {
    sp_disp_msg(6, "上传流水失败，请稍候重试");
    return;
  }
  sp_sleep(1000);
  SP_PRINT(1, 2, "上传成功!!");
  sp_sleep(1500);
}

static void do_menu_test_net(sp_context * ctx)
{
  int ret = 0;
  SP_CLS_FULLSCREEN;
  SP_PRINT(1, 2, "正在检测...");
  ret = sp_send_test(ctx);
  if(ret)
  {
    ctx->online_flag = false;
    sp_disp_error("链路检测失败, ret=%04x", ret);
    return;
  }
  ctx->online_flag = true;
  sp_print_row(FLAG_CLEAR, 1, 2, "通讯成功!!");
  sp_sleep(3000);
}

static void do_show_menu(const menu_info_t * menu_info, int menu_count, void * arg)
{
  sp_context* ctx = arg;
  int ret = 0;
  int32 index = 0;
  int32 page_index = 0;
  int32 page_cnt = 0;
  int32 key = 0;
  int32 start = 0;
  bool need_upd = true;//是否需要刷新
  int32 i = 0, cnt = 0, last = 0;
  bool no_key_pressed = false;
  bool is_quit = false;
  page_cnt = ceil(menu_count * 1.0 / MAX_MENU_SCREEN_CNT);

  while(1)
  {
    sp_heartbeat(ctx);
    ret = sp_check_and_enable_device(ctx);
    if(ret)
      return;

    if(need_upd)
    {
      SP_CLS_FULLSCREEN;
      start = page_index * MAX_MENU_SCREEN_CNT;
      cnt = 0;
      last = start + MAX_MENU_SCREEN_CNT;
      if(last > menu_count)
        last = menu_count;
      for(i = start; i < last; i++)
      {
        SP_PRINT(cnt++, 0, (char*)menu_info[i].menu_desc);
      }
      SP_PRINT(3, 2, "按+/ - 上下翻页");
      need_upd = false;
    }
    is_quit = false;
    no_key_pressed = true;
    key = sp_get_key();
    switch(key)
    {
      case SP_KEY_CLEAR:
        no_key_pressed = false;
        is_quit = true;
        return;
        // break;
      case SP_KEY_NEXT:
        if(page_index < page_cnt - 1)
        {
          page_index ++;
          need_upd = true;
        }
        break;
      case SP_KEY_PREV:
        if(page_index > 0)
        {
          page_index--;
          need_upd = true;
        }
        break;
      case SP_KEY_1:
        no_key_pressed = false;
        index = 1;
        break;
      case SP_KEY_2:
        no_key_pressed = false;
        index = 2;
        break;
      case SP_KEY_3:
        no_key_pressed = false;
        index = 3;
        break;
      case SP_KEY_4:
        no_key_pressed = false;
        index = 4;
        break;
      case SP_KEY_5:
        no_key_pressed = false;
        index = 5;
        break;
      case SP_KEY_6:
        no_key_pressed = false;
        index = 6;
        break;
      case SP_KEY_7:
        no_key_pressed = false;
        index = 7;
        break;
      case SP_KEY_8:
        no_key_pressed = false;
        index = 8;
        break;
      case SP_KEY_9:
        no_key_pressed = false;
        index = 9;
        break;
      case SP_KEY_0:
        no_key_pressed = false;
        index = 10;
        break;
      default:
        break;
    }
    if(no_key_pressed == false)
    {
      if((index > 0) && (index <= menu_count))
      {
        menu_info[index - 1].menu_func(arg);
        need_upd = true;
      }
      if(is_quit)
        break;
    }
  }
}

static void do_menu_set(sp_context* ctx)
{
  menu_info_t menus[] =
  {
    {"1. 管理密码修改", (menu_func_t)do_set_password},
    {"2. 终端编号设置", (menu_func_t)do_set_termno},
    {"3. 通讯参数设置", (menu_func_t)do_set_communicate},
    {"4. 恢复出厂参数", (menu_func_t)do_set_recover},
    {"5. 清空本地流水", (menu_func_t)do_set_clear_local_transdtl},
    {"6. 清空黑名单", (menu_func_t)do_set_clear_blacklist}
  };
  do_show_menu(menus, sizeof(menus) / sizeof(menus[0]), ctx);
}

static void do_menu_main(sp_context * ctx)
{
  menu_info_t menus[] =
  {
    {"1. 消费撤销", (menu_func_t)do_menu_cancel},
    {"2. 营业额查询", (menu_func_t)do_menu_query_revenue},
    {"3. 手工更新黑名单", (menu_func_t)do_menu_update_blacklist},
    {"4. 手动上传流水", (menu_func_t)do_menu_upload_transdtl},
    {"5. 链路检测", (menu_func_t)do_menu_test_net},
    {"6. 手工签到", (menu_func_t)do_menu_sign},
    {"7. 终端参数查询", (menu_func_t)do_menu_show_sysinfo},
    {"8. 管理功能", (menu_func_t)do_menu_set},
    {"9. 流水号存储区", (menu_func_t)do_menu_disp_transno},
    {"0. 流水存储区", (menu_func_t)do_menu_disp_trussdtl}
  };
  do_show_menu(menus, sizeof(menus) / sizeof(menus[0]), ctx);
}

void sp_menu_consume(sp_context * ctx)
{
  int ret = 0;
  while(1)
  {
    //sp_disp_error("work_mode=%02x", ctx->syspara.work_mode);
    if(ctx->syspara.work_mode == SP_WORK_MODE_NORMAL)
      ret = sp_consume_normal(ctx);
    else if(ctx->syspara.work_mode == SP_WORK_MODE_FIXED_VALUE)
      ret = sp_consume_fixed_value(ctx);
    if(ret)
      return;
    do_menu_main(ctx);
    //KillWatchDog();
  }
}






