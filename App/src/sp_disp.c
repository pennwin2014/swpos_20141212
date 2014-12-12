#include "sp_disp.h"


int sp_input_words(sp_context* ctx, char* title, char real_word[MAX_SCREEN_ROWS], uint16 max_word_len)
{
  char temp[MAX_SCREEN_ROWS];
  int start_pos = 0;
  int key = 0;
  uint8 tmp_len = 0;
  uint8 t;
  bool upd_flag = false;
  memset(real_word, 0, MAX_SCREEN_ROWS);
  SP_CLS_FULLSCREEN;
  SP_PRINT(0, 0, title);
  SP_PRINT(2, 6, "������ȷ��");
  SP_PRINT(3, 6, "������");
  while(1)
  {
    if(upd_flag == true)
    {
      start_pos = (MAX_SCREEN_ROWS - tmp_len) / 2 - 1;
      real_word[tmp_len] = 0;
      SP_CLS(1);
      if(tmp_len > 0)
        SP_PRINT(1, start_pos, real_word);
      upd_flag = false;
    }

    key = sp_get_key();
    if(SP_KEY_CLEAR == key)
    {
      if(tmp_len == 0)
        return SP_R_CARD_PASSWD_CANCEL;
      tmp_len--;
      upd_flag = true;
      continue;
    }
    if(SP_KEY_CONFIRM == key)
    {
      real_word[tmp_len] = 0;
      if(0 == strlen(real_word))
      {
        sp_disp_msg(5, "����Ϊ��!!");
        SP_CLS_FULLSCREEN;
        SP_PRINT(0, 0, title);
        SP_PRINT(2, 6, "������ȷ��");
        SP_PRINT(3, 6, "������");
        continue;
      }
      break;
    }
    if(!sp_is_number_key(key, &t))
    {
      if(t < 10)
      {
        if(tmp_len >= max_word_len)
          continue;
        tmp_len++;
        strcpy(temp, real_word);
        sprintf(real_word, "%s%d", temp, t);
        upd_flag = true;
      }
    }
  }
  return 0;
}

int sp_input_password(sp_context* ctx, char* title, char real_password[MAX_SCREEN_ROWS], uint16 max_passwd_len)
{
  char show_password[MAX_SCREEN_ROWS];
  char temp[MAX_SCREEN_ROWS];
  int start_pos = 0;
  int key = 0;
  uint8 tmp_len = 0;
  uint8 t;
  bool upd_flag = false;
  memset(show_password, 0, MAX_SCREEN_ROWS);
  memset(real_password, 0, MAX_SCREEN_ROWS);
  SP_CLS_FULLSCREEN;
  SP_PRINT(0, 0, title);
  SP_PRINT(2, 6, "������ȷ��");
  SP_PRINT(3, 6, "������");
  while(1)
  {
    if(upd_flag == true)
    {
      memset(show_password, 0, MAX_SCREEN_ROWS);
      memset(show_password, '*', tmp_len);
      start_pos = (MAX_SCREEN_ROWS - tmp_len) / 2 - 1;
      sp_print_row(FLAG_CLEAR, 1, start_pos, show_password);
      upd_flag = false;
    }

    key = sp_get_key();
    if(SP_KEY_CLEAR == key)
    {
      if(tmp_len == 0)
        return SP_R_CARD_PASSWD_CANCEL;
      tmp_len--;
      upd_flag = true;
      continue;
    }
    if(SP_KEY_CONFIRM == key)
    {
      //�Ƚ������Ƿ���ȷ
      /*
      if(tmp_len != 6)//���볤�ȴ���
        return SP_E_CARD_PASSWD;
      */
      real_password[tmp_len] = 0;
      break;
    }
    if(!sp_is_number_key(key, &t))
    {
      if(t < 10)
      {
        if(tmp_len >= max_passwd_len)
          continue;
        tmp_len++;
        strcpy(temp, real_password);
        sprintf(real_password, "%s%d", temp, t);
        upd_flag = true;
      }
    }
  }
  return 0;
}

static int do_wait_for_retkey(const int32 retkey, const char* wait_msg, bool wait_flag)
{
  int32 key = 0;
  int msg_len = 0, page_cnt = 0, page_index = 0, lastpage_len = 0, copy_len = 0;
  char tmp_msg[100];
  bool change_flag = false;
  msg_len = strlen(wait_msg);
  page_cnt = msg_len / MAX_SCREEN_WORDS;
  if(msg_len > page_cnt * MAX_SCREEN_WORDS)
  {
    lastpage_len = msg_len - page_cnt * MAX_SCREEN_WORDS;
    page_cnt ++;
  }
  memset(tmp_msg, 0, 100);
  memcpy(tmp_msg, wait_msg, sp_get_min(MAX_SCREEN_WORDS, msg_len));
  SP_CLS_FULLSCREEN;
  SP_PRINT(0, 0, tmp_msg);

  //������ʾ���ַ�������һҳ���������������÷�ҳ��ʾ��������+��-���������·�ҳ
  while(1)
  {
    change_flag = false;
    key = sp_get_key();
    if(wait_flag == true)
    {
      if(retkey == key)
        return 0;
    }
    else
    {
      if((key == SP_KEY_CLEAR) || (key == SP_KEY_CONFIRM) || (key == SP_KEY_MODE) || (key == SP_KEY_FUNC))
        return key;
    }
    if(SP_KEY_NEXT == key)
    {
      change_flag = true;
      page_index++;
    }
    else if(SP_KEY_PREV == key)
    {
      change_flag = true;
      page_index--;
    }
    if(page_index > page_cnt - 1)
    {
      change_flag = false;
      page_index = page_cnt - 1;
    }
    else if(page_index < 0)
    {
      change_flag = false;
      page_index = 0;
    }
    //��ʾ��page_indexҳ������
    if((change_flag == true) && (page_cnt > 1))
    {
      memset(tmp_msg, 0, 100);
      copy_len = MAX_SCREEN_WORDS;
      if(page_index == page_cnt - 1)
      {
        if(lastpage_len > 0)
        {
          copy_len = lastpage_len;
        }
      }
      memcpy(tmp_msg, wait_msg + page_index * MAX_SCREEN_WORDS, copy_len);
      // sp_disp_msg(tmp_msg, 3);
      SP_CLS_FULLSCREEN;
      SP_PRINT(0, 0, tmp_msg);
    }
  }
  //return 0;
}

void do_disp_money(const char* value)
{
  int len = 0;
  SP_CLS_FULLSCREEN;
  len = strlen(value);
  //assert(len < 16);
  //����len����
  sp_print_row(FLAG_DO_NOT_CLEAR, 1, (16 - len) / 2, "%s", value);
  SP_PRINT(2, 6, "������ȷ��");
  SP_PRINT(3, 6, "������");
  //SP_PRINT(3, 0, "ȷ��--ģʽ--���");
}

static void do_disp_request_consume_card(sp_context* ctx)
{
  char extra_msg[4];
  uint8 extra_cnt = 0;
  static uint8 old_extra_cnt = 0;
  static SYSTIME old_time;
  SYSTIME new_time;
  sp_get_hdtime(ctx, &new_time);
  if(sp_check_time(new_time, old_time) == 0)
    return;
  old_time = new_time;
  if(ctx->disp_type != SP_TP_REQUEST_CONSUME_CARD)
  {
    SP_CLS_FULLSCREEN;
    if(ctx->online_flag)
      SP_PRINT(0, 14, " ");
    else
      SP_PRINT(0, 14, "*");
    sp_print_row(FLAG_DO_NOT_CLEAR, 2, 2, "���׶� :%.2f", ctx->card.amount * 1.0 / 100.0);
  }
  extra_cnt = abs((new_time.chSecond % 4) - 2) + 1;
  //����Ҫ��ʾ��>���ӵ�ʱ�����Ҫˢ��
  if(extra_cnt < old_extra_cnt)
    SP_CLS(1);
  old_extra_cnt = extra_cnt;
  memset(extra_msg, 0, 4);
  memset(extra_msg, '>', extra_cnt);
  sp_print_row(FLAG_DO_NOT_CLEAR, 1, 4, "��ˢ��%s", extra_msg);
}

static void do_disp_request_card(sp_context* ctx)
{
  char extra_msg[4];
  uint8 extra_cnt = 0;
  static uint8 old_extra_cnt = 0;
  static SYSTIME old_time;
  SYSTIME new_time;
  sp_get_hdtime(ctx, &new_time);
  if(sp_check_time(new_time, old_time) == 0)
    return;
  old_time = new_time;
  if(ctx->disp_type != SP_TP_REQUEST_CARD)
  {
    SP_CLS_FULLSCREEN;
    if(!ctx->online_flag)
      SP_PRINT(0, 15, "*");
  }
  extra_cnt = abs((new_time.chSecond % 4) - 2) + 1;
  //����Ҫ��ʾ��>���ӵ�ʱ�����Ҫˢ��
  if(extra_cnt < old_extra_cnt)
    SP_CLS(1);
  old_extra_cnt = extra_cnt;
  memset(extra_msg, 0, 4);
  memset(extra_msg, '>', extra_cnt);
  sp_print_row(FLAG_DO_NOT_CLEAR, 1, 4, "��ˢ��%s", extra_msg);
  sp_print_row(FLAG_DO_NOT_CLEAR, 3, 2, "%02d/%02d %02d:%02d:%02d", new_time.chMonth, new_time.chDay, new_time.chHour,
               new_time.chMinute, new_time.chSecond);
}

static void do_disp_user_balance(sp_context* ctx)
{
  SP_CLS_FULLSCREEN;
  sp_print_row(FLAG_DO_NOT_CLEAR, 1, 0, "���� :%s", ctx->card.username);
  sp_print_row(FLAG_DO_NOT_CLEAR, 2, 0, "ѧ�� :%s", ctx->card.stuempno);
  sp_print_row(FLAG_DO_NOT_CLEAR, 3, 0, "��� :%.2f", ctx->card.cardbefbal * 1.0 / 100.0);
}

static void do_disp_cancel_consume(sp_context* ctx)
{
  uint32 money;
  SP_CLS_FULLSCREEN;
  sp_print_row(FLAG_DO_NOT_CLEAR, 0, 2, "%02x/%02x %02x:%02x:%02x", ctx->card.last_datetime[2], ctx->card.last_datetime[3],
               ctx->card.last_datetime[0], ctx->card.last_datetime[1], ctx->card.last_datetime[2]);
  sp_print_row(FLAG_DO_NOT_CLEAR, 1, 0, "���� :%s", ctx->card.username);
  money =  ctx->card.last_trans_amt;
  //sp_buffer_to_uint32(ctx->card.last_trans_amt, &money);
  sp_print_row(FLAG_DO_NOT_CLEAR, 2, 0, "���׶� :%.2f", money * 1.0 / 100.0);
  sp_print_row(FLAG_DO_NOT_CLEAR, 3, 6, "������ȷ��");
}

static void do_disp_consume_success(sp_context* ctx)
{
  SP_CLS_FULLSCREEN;
  sp_print_row(FLAG_DO_NOT_CLEAR, 1, 2, "���ѳɹ� !");
  sp_print_row(FLAG_DO_NOT_CLEAR, 2, 0, "���׶� :%.2f", ctx->card.amount * 1.0 / 100.0);
  sp_print_row(FLAG_DO_NOT_CLEAR, 3, 0, "����� :%.2f", (ctx->card.cardbefbal - ctx->card.amount) * 1.0 / 100.0);
  sp_sleep(2500);
}

void sp_wait_seconds(int wait_sec)
{
  int key = 0;
  while(wait_sec --)
  {
    key = sp_get_key();
    if(SP_KEY_CLEAR == key)
      return;
    sp_print_row(FLAG_CLEAR, 3, 0, "%d", wait_sec);
    sp_sleep(1000);
  }
}

static void do_disp_insufficient_balance()
{
  SP_CLS_FULLSCREEN;
  SP_PRINT(1, 2, "����ʧ�� !");
  SP_PRINT(2, 0, "���Ŀ�����");
  sp_sleep(2000);
  sp_wait_seconds(3);
}

static void is_cancel_consume(uint8* cancel_flag)
{
  int key = 0;
  SP_CLS_FULLSCREEN;
  SP_PRINT(1, 0, "�Ƿ�ȡ������ ?");
  SP_PRINT(2, 6, "ȡ����ȷ��");
  SP_PRINT(3, 6, "���ذ����");
  while(1)
  {
    key = sp_get_key();
    if(SP_KEY_CLEAR == key)
    {
      *cancel_flag = SP_SWITCH_OFF;
      return;
    }
    else if(SP_KEY_CONFIRM == key)
    {
      *cancel_flag = SP_SWITCH_ON;
      return;
    }
  }
}

static void show_passwd_error()
{
  SP_CLS_FULLSCREEN;
  SP_PRINT(1, 2, "�������!!");
  sp_wait_seconds(3);
}

void do_show_system_info(sp_context* ctx, int index)
{
  sp_syspara* para = &ctx->syspara;
  uint16 offline_transdtl_amt = 0;
  offline_transdtl_amt = ctx->syspara.total_transno - ctx->syspara.confirmed_transno;
  SP_CLS_FULLSCREEN;
  switch(index)
  {
    case 0:
      SP_PRINT(0, 0, "�ն˺�: ");
      sp_print_row(FLAG_DO_NOT_CLEAR, 1, 4, "%02x%02x%02x%02x", para->termno[0], para->termno[1], para->termno[2], para->termno[3]);
      break;
    case 1:
      SP_PRINT(0, 0, "SAM ����: ");
      sp_print_row(FLAG_DO_NOT_CLEAR, 1, 2, "%02x%02x%02x%02x%02x%02x", para->samno[0], para->samno[1], para->samno[2], para->samno[3], para->samno[4], para->samno[5]);
      break;
    case 2:
      SP_PRINT(0, 0, "����汾��: ");
      sp_print_row(FLAG_DO_NOT_CLEAR, 1, 4, "%02d.%02d.%02d", para->soft_verno[0], para->soft_verno[1], para->soft_verno[2]);
      break;
    case 3:
      SP_PRINT(0, 0, "�շѲ����汾��: ");
      sp_print_row(FLAG_DO_NOT_CLEAR, 1, 4, "%08d", para->feepara_verno);
      break;
    case 4:
      SP_PRINT(0, 0, "�������汾��: ");
      sp_print_row(FLAG_DO_NOT_CLEAR, 1, 4, "%08d", para->syspara_verno);
      break;
    case 5:
      SP_PRINT(0, 0, "�������汾��: ");
      sp_print_row(FLAG_DO_NOT_CLEAR, 1, 1, "%02x%02x%02x%02x%02x%02x%02x", para->blacklist_verno[0], para->blacklist_verno[1],
                   para->blacklist_verno[2], para->blacklist_verno[3], para->blacklist_verno[4], para->blacklist_verno[5], para->blacklist_verno[6]);
      break;
    case 6:
      SP_PRINT(0, 0, "���ؽ�����ˮ��: ");
      sp_print_row(FLAG_DO_NOT_CLEAR, 1, 5, "%06d", ctx->syspara.total_transno);
      break;
    case 7:
      SP_PRINT(0, 0, "δ�ϴ���ˮ����: ");
      sp_print_row(FLAG_DO_NOT_CLEAR, 1, 4, "%08d", offline_transdtl_amt);
      break;
    case 8:
      SP_PRINT(0, 0, "�ն˻���: ");
      sp_print_row(FLAG_DO_NOT_CLEAR, 1, 4, "%08d", ctx->syspara.canid);
      break;
  }
  sp_print_row(FLAG_DO_NOT_CLEAR, 3, 3, "[%d/%d] +/- ��ҳ", index + 1, SHOW_SYSTEM_INFO_COUNT);
}

static void do_disp_is_init_device(sp_context* ctx, uint8* choice)
{
  int key = 0;
  SP_CLS_FULLSCREEN;
  SP_PRINT(0, 0, "�Ƿ��ʼ��? ");
  SP_PRINT(2, 8, "�ǰ�ȷ��");
  SP_PRINT(3, 8, "�����");
  while(1)
  {
    key = sp_get_key();
    if(SP_KEY_CONFIRM == key)
    {
      *choice = 0;
      break;
    }
    else if(SP_KEY_CLEAR == key)
    {
      *choice = 1;
      break;
    }
    sp_sleep(10);
  }
}

static void do_disp_unconfirm_transdtl(sp_context* ctx, uint8* choice)
{
  int key = 0;
  SP_CLS_FULLSCREEN;
  SP_PRINT(0, 0, "����ˮδ�ϴ�");
  SP_PRINT(2, 6, "������ȷ��");
  SP_PRINT(3, 6, "���ذ����");
  while(1)
  {
    key = sp_get_key();
    if(SP_KEY_CLEAR == key)
    {
      *choice = 1;
      return;
    }
    else if(SP_KEY_CONFIRM == key)
    {
      *choice = 0;
      return;
    }
  }
}

static void do_disp_is_enable_device(sp_context* ctx, uint8* choice)
{
  int key = 0;
  SP_CLS_FULLSCREEN;
  SP_PRINT(0, 0, "�豸δ����");
  SP_PRINT(1, 0, "�Ƿ������豸? ");
  SP_PRINT(2, 8, "�ǰ�ȷ��");
  SP_PRINT(3, 8, "�����");
  while(1)
  {
    key = sp_get_key();
    if(SP_KEY_CLEAR == key)
    {
      *choice = 1;
      return;
    }
    else if(SP_KEY_CONFIRM == key)
    {
      *choice = 0;
      return;
    }
  }
}

void do_disp_trussdtl(sp_context* ctx)
{
  SP_CLS_FULLSCREEN;
  SP_PRINT(1, 0, "ϵͳ������...");
  SP_PRINT(2, 0, "ҵ����ͣ");
}


void sp_disp_by_type(uint8 disp_type, sp_context* ctx, void* arg)
{
  switch(disp_type)
  {
    case SP_TP_REQUEST_CARD:
      do_disp_request_card(ctx);
      break;
    case SP_TP_USER_BALANCE:
      do_disp_user_balance(ctx);
      break;
    case SP_TP_CANCEL_CONSUME:
      do_disp_cancel_consume(ctx);
      break;
    case SP_TP_CONSUME_SUCCESS:
      do_disp_consume_success(ctx);
      break;
    case SP_TP_INSUFFICIENT_BALANCE:
      do_disp_insufficient_balance();
      break;
    case SP_TP_REQUEST_CONSUME_CARD:
      do_disp_request_consume_card(ctx);
      break;
    case SP_TP_DISP_MONEY:
      do_disp_money(arg);
      break;
    case SP_TP_IS_CANCEL_CONSUME:
      is_cancel_consume(arg);
      break;
    case SP_TP_PASSWORD_ERROR:
      show_passwd_error();
      break;
      /*
      case SP_TP_REVENUE_QUERY:
      do_disp_revenue(ctx, arg);
      break;
      */
    case SP_TP_SHOW_SYSTEM_INFO:
      do_show_system_info(ctx, *((int*)arg));
      break;
    case SP_TP_IS_INIT_DEVICE:
      do_disp_is_init_device(ctx, arg);
      break;
    case SP_TP_DISP_UNCONFIM_TRANSDTL:
      do_disp_unconfirm_transdtl(ctx, arg);
      break;
    case SP_TP_IS_ENABLE_DEVICE:
      do_disp_is_enable_device(ctx, arg);
      break;
    case SP_TP_TRUSSDTL:
      do_disp_trussdtl(ctx);
      break;
  }
  ctx->disp_type = disp_type;
}

void sp_disp_msg(uint32 nsec, const char* fmt, ...)
{
  char msg[256];
  int len;
  int key = 0, left_sec = 0;
  va_list arg;
  memset(msg, 0, 256);
  va_start(arg, fmt);
  len = vsnprintf(msg, sizeof(msg) - 1, fmt, arg);
  va_end(arg);
  if(len > 0)
  {
    SP_CLS_FULLSCREEN;
    SP_PRINT(0, 0, msg);
  }
  left_sec = nsec;
  while(left_sec > 0)
  {
    sp_print_row(FLAG_DO_NOT_CLEAR, 3, 0, " %d ", left_sec--);
    key = sp_get_key();
    if(SP_KEY_CLEAR == key)
      return;
    sp_sleep(1000);
  }
}


void sp_wait_for_retkey(const int32 retkey, const char* func_name,
                        const int lineno, const char* fmt, ...)
{
  char msg[256];
  char final_msg[256];
  int len;
  va_list arg;
  va_start(arg, fmt);
  len = vsnprintf(msg, sizeof(msg) - 1, fmt, arg);
  va_end(arg);
  if(len > 0)
  {
    memset(final_msg, 0, 256);
    memcpy(final_msg, msg, strlen(msg));
#ifdef SP_DEBUG
    sprintf(final_msg + strlen(msg), " (func=%s,line=%d)<press 0X%x to return>", func_name, lineno, retkey);
#endif
    sprintf(final_msg + strlen(msg), ",len=%d", strlen(msg));
    do_wait_for_retkey(retkey, final_msg, true);
  }
}

#define SP_ERROR_DEBUG 0
void sp_disp_error(const char* fmt, ...)
{
#ifdef SP_ERROR_DEBUG
  char msg[256];
  int len;
  int key = 0, left_sec = 0;
  va_list arg;
  memset(msg, 0, sizeof(msg));
  va_start(arg, fmt);
  len = vsnprintf(msg, sizeof(msg) - 1, fmt, arg);
  va_end(arg);
  if(len > 0)
  {
    SP_CLS_FULLSCREEN;
    SP_PRINT(0, 0, msg);
  }
  left_sec = 999;
  while(left_sec > 0)
  {
    sp_print_row(FLAG_CLEAR, 3, 0, "%d", left_sec--);
    sp_sleep(1000);
    key = sp_get_key();
    if(SP_KEY_CLEAR == key)
      return;
  }
#endif
}

//#define SP_DEBUG_MODE 0
void sp_disp_debug(const char* fmt, ...)
{
#ifdef SP_DEBUG_MODE
  char msg[256];
  int len;
  int key = 0, left_sec = 0;
  va_list arg;
  memset(msg, 0, 256);
  va_start(arg, fmt);
  len = vsnprintf(msg, sizeof(msg) - 1, fmt, arg);
  va_end(arg);

  if(len > 0)
  {
    SP_CLS_FULLSCREEN;
    SP_PRINT(0, 0, msg);
  }

  left_sec = 999;
  while(left_sec > 0)
  {
    sp_print_row(FLAG_CLEAR, 3, 0, "%d", left_sec--);
    sp_sleep(1000);
    key = sp_get_key();
    if(SP_KEY_CLEAR == key)
      return;
  }

#endif
}

void sp_print_row(uint8 flag, uint8 row, uint8 col, const char* fmt, ...)
{
  char msg[20];
  int len;
  va_list arg;
  memset(msg, 0, sizeof msg);
  va_start(arg, fmt);
  len = vsnprintf(msg, sizeof(msg) - 1, fmt, arg);
  va_end(arg);

  if(len > 0)
  {
    if(flag == FLAG_CLEAR)
      SP_CLS(row);
    SP_PRINT(row, col, msg);
  }
}

void sp_handle_fatal_error(sp_context* ctx,  uint8 ret)
{
  //����������ϵ����Ա
  sp_disp_error("������������ϵ����Ա��������%04x", ret);
  sp_sleep(9999999);
}







