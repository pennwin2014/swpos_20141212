#include "config.h"
#include "Mifare.h"
#include <stdio.h>
#include "sp_communicate.h"
#include "sp_disp.h"
#include "sp_menu.h"


//#define TEST_MODE 0
#ifdef TEST_MODE
static void test_card(sp_context * ctx)
{
  SYSTIME new_time;
  int ret = 0;
  while(1)
  {
    //1、寻卡
    ret = sp_request_card_poweron(&ctx->card);
    SP_WAIT_AND_DISPMSG(SP_KEY_CONFIRM, "寻卡ret=%04x,按确定键继续", ret);
    //2、读卡
    ret = sp_read_card(ctx, &ctx->card, SP_FILE18);
    SP_WAIT_AND_DISPMSG(SP_KEY_CONFIRM, "读卡ret=%04x,按确定键继续", ret);
    //3、显示时间
    ret = sp_get_hdtime(ctx, &new_time);
    sp_disp_error("%02d/%02d %02d:%02d:%02d", new_time.chMonth, new_time.chDay, new_time.chHour,
                  new_time.chMinute, new_time.chSecond);
    //4、重设时间
    //sp_reset_hdtime(INTER_RTC);
    sp_reset_hdtime(EXTER_RTC);
    sp_disp_error("重设时间");
    //5、显示时间
    ret = sp_get_hdtime(ctx, &new_time);
    sp_disp_error("%02d/%02d %02d:%02d:%02d", new_time.chMonth, new_time.chDay, new_time.chHour,
                  new_time.chMinute, new_time.chSecond);
  }
}
#endif


#define HD_POS HD_SC
static int do_init_hd_stuff(sp_context* ctx)
{
  InitBoard();
  //看门狗初始化
#ifdef OPEN_WDT
  InitWatchDog();
#endif
  //串口设置
  sp_init_com();
  //
  sp_reset_hdtime(EXTER_RTC);
  return 0;

}

static void do_auth(sp_context* ctx)
{
  int ret = 0;
  sp_print_row(FLAG_CLEAR, 1, 2, "正在签到...");
  ret = sp_auth(ctx);
  if(ret)
  {
    sp_disp_error("签到失败!!ret=%04x", ret);
    return;
  }
  sp_print_row(FLAG_CLEAR, 1, 2, "签到成功!! ");
  sp_sleep(2000);
}

int main()
{
  sp_context *main_ctx = (sp_context*)malloc(sizeof(sp_context));
  int ret = 0;
  ret = do_init_hd_stuff(main_ctx);
  if(ret)
  {
    SP_WAIT_AND_DISPMSG(SP_KEY_CONFIRM, "初始化汇多相关失败!!按确认键退出程序,ret=%04x", ret);
    return 1;
  }
  ret = sp_read_system(main_ctx);
  if(ret)
  {
    ret = sp_recover_system(main_ctx);
    if(ret)
    {
      ret = sp_write_syspara(main_ctx);
      if(ret)
      {
        sp_handle_fatal_error(main_ctx, ret);
      }
      return SP_R_RECOVER_FAIL;
    }
  }

  //已经init的需要重新启用设备
  if(main_ctx->syspara.init_flag)
  {
    if(main_ctx->syspara.enable_staus == SP_S_DEVICEID_NOT_MATCH)
    {
      //就要重新初始化
      ret = sp_recover_system(main_ctx);
      if(ret)
        return ret;
    }
    else if(main_ctx->syspara.enable_staus == SP_SUCCESS)
    {
      //签到即使失败也可以进入系统
      do_auth(main_ctx);
    }
    else
    {
      do
      {
        ret = sp_check_and_enable_device(main_ctx);
        if(SP_R_DEVICE_NOT_ENABLE == ret)
          sp_disp_msg(5, "设备未启用, 不能正常工作");
      }
      while(ret);
      /*
        //启用设备
        ret = sp_enable_device(main_ctx);
        if(ret)
        {
          sp_disp_error("启用设备失败!!,ret=%04x", ret);
          main_ctx->syspara.enable_staus = ret;
          ret = sp_write_syspara(main_ctx);
          return ret;
        }
        ret = sp_write_syspara(main_ctx);
        if(ret)
          return ret;
          */
    }
  }
  ret = sp_enable_sam_card(main_ctx);
  if(ret)
  {
    SP_WAIT_AND_DISPMSG(SP_KEY_CONFIRM, "初始化psam卡失败,ret=%04x", ret);
    return 1;
  }
  // 检查日期是否变化
  ret = sp_check_date_change(main_ctx);
  if(ret)
  {
    //sp_get_time(main_ctx);
    SP_WAIT_AND_DISPMSG(SP_KEY_CONFIRM, "检查日期变化失败,ret=%04x", ret);
  }

#ifdef TEST_MODE
  test_card(main_ctx);
#endif
  //消费菜单
  sp_menu_consume(main_ctx);
  free(main_ctx);
  return 0;
}
