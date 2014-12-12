#define SP_GLOBALS

#include "sp_communicate.h"

//��̬ȫ�ֱ��������������ص���������
static DCB sDCB;
static uint8 global_send_buffer[MAX_SENDMSG_LEN];
static sp_com_buffer_handler global_tcp_handler;
//////////////////////////////���ߺ���/////////////////////////////////////
static void get_std_ctl_len(uint8* ctl_len, uint16 msg_len)
{
  memcpy(ctl_len, &msg_len, 2);
}

static uint16 get_data_len(void * data_len_buffer)
{
  uint16 data_len = 0;
  memcpy(&data_len, data_len_buffer, 2);
  return data_len;
}

static int compare_tcp_checksum(sp_context* ctx, uint8* data, uint16 data_len)
{
  uint8 crc1, crc2;
  crc1 = data[data_len - 1];
  crc2 = sp_calc_crc8(data, data_len - 1);
  return (crc1 == crc2) ? 0 : 1;
}

static uint8 do_com_sendbuf(uint8 uComm, uint8 *pcSendBuf, int iLen)
{
  return (0 == COM_SendBuf(uComm, pcSendBuf, iLen)) ? 1 : 0;
}

static uint8 do_send_msg(const uint8 * msg, uint16 msg_len)
{
  uint8* send_msg = global_send_buffer;//[MAX_SENDMSG_LEN] = {0};
  uint8 ctl_len[2] = {0};
  int send_len = 0;
  if(msg_len > MAX_SENDMSG_LEN)
    return 2;
  send_len = msg_len + 2;
  get_std_ctl_len(ctl_len, msg_len);
  memcpy(send_msg, ctl_len, 2);
  memcpy(send_msg + 2, msg, msg_len);
  return do_com_sendbuf(COM1, send_msg, send_len);
}


///////////////////////////�ڲ�����///////////////////////////////////
static void do_init_tcp_handler()
{
  global_tcp_handler.recv_buffer = (uint8*)malloc(TCP_RECV_BUFFER_SIZE);
  memset(global_tcp_handler.recv_buffer, 0, TCP_RECV_BUFFER_SIZE);
  global_tcp_handler.recv_len = 0;
  global_tcp_handler.current_tcp_seqno = 0;
}
static void clear_recv_buffer()
{
  //��ջ�����
  memset(global_tcp_handler.recv_buffer, 0 , TCP_RECV_BUFFER_SIZE);
  global_tcp_handler.recv_len = 0;
}

static void do_parse_resp(sp_context* ctx, uint8* recv_buffer, uint16* buffer_len)
{
  uint16 data_len = 0;
  data_len = get_data_len(global_tcp_handler.recv_buffer);
  if(global_tcp_handler.recv_len > 0)
  {
    if(data_len > 0)
    {
      if(global_tcp_handler.recv_len - 2 >= data_len)
      {
        //crcһ�¾���Ϊ���յ�����������Ϣ
        if(!compare_tcp_checksum(ctx, global_tcp_handler.recv_buffer + 2, data_len))
        {
          global_tcp_handler.recv_len -= 2;
          (*buffer_len) = data_len;
          memcpy(recv_buffer, global_tcp_handler.recv_buffer + 2, data_len);
          //���½��ջ���������
          global_tcp_handler.recv_len -= data_len;
          memcpy(global_tcp_handler.recv_buffer, global_tcp_handler.recv_buffer + 2 + data_len, global_tcp_handler.recv_len);
        }
      }
    }
  }

}


/*
* ����:�����������ݵĻص�����
*/
static void do_receive_data(void *pBuf, uint16 nLen)
{
  //�ͼ�����ջ�����
  if(global_tcp_handler.recv_len + nLen > TCP_RECV_BUFFER_SIZE)
  {
    clear_recv_buffer();
    sp_print_row(FLAG_CLEAR, 2, 6, "cls buf");
    sp_sleep(10000);
  }
  //���뻺����
  memcpy(global_tcp_handler.recv_buffer + global_tcp_handler.recv_len, pBuf, nLen);
  global_tcp_handler.recv_len += nLen;
  /*
    //���ǻ�����
    global_tcp_handler.recv_len = nLen;
    memcpy(global_tcp_handler.recv_buffer, pBuf, nLen);
    */
}



///////////////////////////////������////////////////////////////////////////////////
static void prepare_header_by_cmd(uint8 cmd, sp_tcp_header* header)
{
  memcpy(header->guide_code, "HST", 3);
  header->cmd_code = cmd;
  memcpy(header->deviceno, "\x00\x01", 2);
  header->seqno = global_tcp_handler.current_tcp_seqno;
}

static int prepare_output_parameter(uint8* send_buffer, uint16 buffer_max_len, uint16* data_len, sp_tcp_trans_unit* record)
{
  uint16 offset = 0;
  (*data_len) = sizeof(sp_tcp_header) + record->header.data_len;
  if(*data_len > buffer_max_len - 1)
    return SP_E_BUFFER_SIZE_OUT_OF_RANGE;
  memcpy(send_buffer, &record->header, sizeof(sp_tcp_header));
  offset = sizeof(sp_tcp_header);
  memcpy(send_buffer + offset, record->data, record->header.data_len);
  return 0;
}

static void calc_send_buffer_checksum(uint8* send_buffer, uint16* data_len)
{
  send_buffer[*data_len] = sp_calc_crc8(send_buffer, *data_len - 1);
  *data_len = *data_len + 1;
}

PROTOCOL_FUNC_PREPARE(do_prepare_auth)
{
  sp_tcp_trans_unit tcp_struct;
  int ret = 0;
  int offset = 0;
  memset(&tcp_struct, 0, sizeof(sp_tcp_trans_unit));
  //header
  prepare_header_by_cmd(SP_CMD_AUTH, &tcp_struct.header);
  //��data����
  //�豸��������
  memcpy(tcp_struct.data, ctx->syspara.termno, 4);
  offset += 4;
  //�豸����汾��
  memcpy(tcp_struct.data + offset, ctx->syspara.soft_verno, 3);
  offset += 3;
  // ʱ��
  memcpy(tcp_struct.data + offset, ctx->current_datetime, 7);
  offset += 7;
  //���ʰ汾��
  sp_uint32_to_buffer(ctx->syspara.feepara_verno, tcp_struct.data + offset);
  offset += 4;
  //�������汾��
  sp_uint32_to_buffer(ctx->syspara.syspara_verno, tcp_struct.data + offset);
  offset += 4;
  tcp_struct.header.data_len = offset;
  //��������
  ret = prepare_output_parameter(send_buffer, buffer_max_len, data_len, &tcp_struct);
  if(ret)
    return ret;
  //����check_sum
  calc_send_buffer_checksum(send_buffer, data_len);
  return 0;
}

PROTOCOL_FUNC_RESP(auth_resp_handler)
{
  int offset;
  int ret = 0;
  uint8 need_upd_time = 0;
  uint32 feepara_verno = 0;
  uint32 syspara_verno = 0;
  uint32 timepara_verno = 0;
  SYSTIME svr_tm;
  sp_print_row(FLAG_CLEAR, 2, 0, "%02x%02x%02x%02x%02x%02x%02x%02x", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5],
               buffer[6], buffer[7]);
  offset = 0;
  need_upd_time = buffer[offset++];
  if(need_upd_time)
  {
    ret = sp_get_hdtime(ctx, &svr_tm);
    sp_timestr_to_systime(buffer + offset, &svr_tm);
    //sp_print_row(FLAG_CLEAR, 0, 0, "gap=%d", svr_tm - local_tm);
    ret = sp_set_hdtime(&svr_tm);
    if(ret)
    {
      return ret;
    }
  }
  offset += 7;
  //���ʰ汾��
  sp_buffer_to_uint32(buffer + offset, &feepara_verno);
  ctx->svrpara.feepara_verno = feepara_verno;
  offset += 4;
  //ϵͳ�����汾��
  sp_buffer_to_uint32(buffer + offset, &syspara_verno);
  ctx->svrpara.syspara_verno = syspara_verno;
  offset += 4;
  //ʱ��β����汾��
  sp_buffer_to_uint32(buffer + offset, &timepara_verno);
  ctx->svrpara.timepara_verno = timepara_verno;
  offset += 4;
  return 0;
}

PROTOCOL_FUNC_PREPARE(do_prepare_transdtl)
{
  sp_tcp_trans_unit tcp_struct;
  int ret = 0;
  memset(&tcp_struct, 0, sizeof(sp_tcp_trans_unit));
  //header
  prepare_header_by_cmd(SP_CMD_RT_TRANSDTL, &tcp_struct.header);
  //data
  tcp_struct.header.data_len = sizeof(sp_transdtl);
  memcpy(tcp_struct.data, arg, tcp_struct.header.data_len);
  //��������
  ret = prepare_output_parameter(send_buffer, buffer_max_len, data_len, &tcp_struct);
  if(ret)
    return ret;
  //����check_sum
  calc_send_buffer_checksum(send_buffer, data_len);
  return 0;
}

PROTOCOL_FUNC_RESP(transdtl_resp_handler)
{
  //������ˮָ��
  sp_recv_rt_transdtl* rt_record = NULL;
  sp_transdtl read_trans;
  int ret = 0;
  uint32 next_transaddr = 0;
  //sp_print_row(FLAG_CLEAR, 0, 0, "rt,len=%d", data_len);
  rt_record = (sp_recv_rt_transdtl*) buffer;
  //��ʾ��ˮ�Ѿ��ϴ��ɹ�
  if(rt_record->retcode == 0)
  {
    if(ctx->syspara.confirmed_transno < rt_record->transno)
    {
      if(ctx->syspara.confirmed_transaddr < ctx->syspara.total_transaddr)
      {
        next_transaddr = sp_get_next_transdtl_addr(ctx->syspara.confirmed_transaddr);
        ret = sp_read_transdtl(&read_trans, next_transaddr);
        if(ret)
          return ret;
        ctx->syspara.confirmed_transaddr = next_transaddr;
        ctx->syspara.confirmed_transno = read_trans.termseqno;
        ret = sp_write_syspara(ctx);
        if(ret)
          return ret;
      }
    }
  }
  return 0;
}

PROTOCOL_FUNC_PREPARE(do_prepare_blacklist)
{
  sp_tcp_trans_unit tcp_struct;
  int ret = 0;
  memset(&tcp_struct, 0, sizeof(sp_tcp_trans_unit));
  //header
  prepare_header_by_cmd(SP_CMD_GET_BLACKLIST, &tcp_struct.header);
  //data
  tcp_struct.header.data_len = 2;
  sp_uint16_to_buffer(ctx->svrpara.download_index, tcp_struct.data);
  //��������
  ret = prepare_output_parameter(send_buffer, buffer_max_len, data_len, &tcp_struct);
  if(ret)
    return ret;
  //����check_sum
  calc_send_buffer_checksum(send_buffer, data_len);
  return 0;
}

PROTOCOL_FUNC_RESP(blacklist_resp_handler)
{
  uint16 offset = 0;
  uint16 pack_size = 0;
  uint16 download_index = 0;
  sp_print_row(FLAG_CLEAR, 1, 0, "�������������");
  //��ǰ���
  sp_buffer_to_uint16(buffer + offset, &download_index);
  ctx->svrpara.download_index = download_index;
  offset += 2;
  //������־
  ctx->svrpara.end_flag = buffer[offset++];
  //�����ֽ���
  sp_buffer_to_uint16(buffer + offset, &pack_size);
  offset += 2;
  //���汾�εĺ�����λͼ
  memcpy(ctx->svrpara.tmp_blacklist_bitmap, buffer + offset, pack_size);
  offset += pack_size;
  ctx->svrpara.buffer_size += pack_size;
  return 0;
}

PROTOCOL_FUNC_PREPARE(do_prepare_heartbeat)
{
  sp_tcp_trans_unit tcp_struct;
  uint16 offset = 0;
  int ret = 0;
  memset(&tcp_struct, 0, sizeof(sp_tcp_trans_unit));
  //header
  prepare_header_by_cmd(SP_CMD_HEARTBEAT, &tcp_struct.header);
  //data
  //�豸ʱ��
  memcpy(tcp_struct.data, ctx->current_datetime + 1, 6);
  offset += 6;
  //�豸�����͵���ˮ��
  sp_uint32_to_buffer(ctx->syspara.confirmed_transno, tcp_struct.data + offset);
  offset += 4;
  //��ǰ��ˮ��
  sp_uint32_to_buffer(sp_get_transno(), tcp_struct.data + offset);
  offset += 4;
  tcp_struct.header.data_len = offset;
  //��������
  ret = prepare_output_parameter(send_buffer, buffer_max_len, data_len, &tcp_struct);
  if(ret)
    return ret;
  //����check_sum
  calc_send_buffer_checksum(send_buffer, data_len);
  return 0;
}

PROTOCOL_FUNC_RESP(heartbeat_resp_handler)
{
  uint16 offset = 0;
  uint32 syspara_verno, feerate_verno, timepara_verno, recv_trans_no, cur_trans_no;
  uint8 pos_time[6];
  static int8 beat_cnt = 1;
  if(beat_cnt > 0)
    SP_PRINT(0, 15, "H");
  else
    SP_PRINT(0, 15, "L");
  beat_cnt = -beat_cnt;
  //�������汾��
  offset = 0;
  memcpy(ctx->svrpara.blacklist_verno, buffer + offset, 6);
  offset += 6;
  //ϵͳ�汾��
  sp_buffer_to_uint32(buffer + offset, &syspara_verno);
  ctx->svrpara.syspara_verno = syspara_verno;
  offset += 4;
  //���ʰ汾��
  sp_buffer_to_uint32(buffer + offset, &feerate_verno);
  ctx->svrpara.feepara_verno = feerate_verno;
  offset += 4;
  //ʱ��β����汾��
  sp_buffer_to_uint32(buffer + offset, &timepara_verno);
  ctx->svrpara.timepara_verno = timepara_verno;
  offset += 4;
  //���յ�����ˮ��
  sp_buffer_to_uint32(buffer + offset, &recv_trans_no);
  offset += 4;
  //��ǰ��ˮ��
  sp_buffer_to_uint32(buffer + offset, &cur_trans_no);
  offset += 4;
  //pos��ʱ��
  memcpy(pos_time, buffer + offset, 6);
  offset += 6;
  return 0;
}

PROTOCOL_FUNC_PREPARE(do_prepare_test)
{
  sp_tcp_trans_unit tcp_struct;
  int ret = 0;
  memset(&tcp_struct, 0, sizeof(sp_tcp_trans_unit));
  //header
  prepare_header_by_cmd(SP_CMD_TEST, &tcp_struct.header);
  //data
  tcp_struct.header.data_len = 2;
  tcp_struct.data[0] = 0x06;
  tcp_struct.data[1] = 0x06;
  //��������
  ret = prepare_output_parameter(send_buffer, buffer_max_len, data_len, &tcp_struct);
  if(ret)
    return ret;
  //����check_sum
  calc_send_buffer_checksum(send_buffer, data_len);
  return 0;
}

PROTOCOL_FUNC_RESP(test_resp_handler)
{
  sp_print_row(FLAG_CLEAR, 1, 0, "������·���Է���");
  return 0;
}

PROTOCOL_FUNC_PREPARE(do_download_feerate)
{
  sp_tcp_trans_unit tcp_struct;
  int ret = 0;
  uint16 download_index = 0;
  memset(&tcp_struct, 0, sizeof(sp_tcp_trans_unit));
  //header
  prepare_header_by_cmd(SP_CMD_GET_FEERATE, &tcp_struct.header);
  //data
  tcp_struct.header.data_len = 2;
  sp_uint16_to_buffer(download_index, tcp_struct.data);
  ctx->svrpara.download_index = download_index;
  //��������
  ret = prepare_output_parameter(send_buffer, buffer_max_len, data_len, &tcp_struct);
  if(ret)
    return ret;
  //����check_sum
  calc_send_buffer_checksum(send_buffer, data_len);
  return 0;
}

PROTOCOL_FUNC_RESP(feerate_resp_handler)
{
  uint16 offset = 0;
  sp_print_row(FLAG_CLEAR, 1, 0, "���ط��ʱ���");
  //assert(data_len > 256 + 32)
  //�Ƿ���ҪУ��һ��crc
  memcpy(ctx->feerate_table, buffer, 256);
  offset += 256;
  //����ʹ�ÿ����λͼ
  memcpy(ctx->syspara.feetype_bitmap, buffer + offset, 32);
  offset += 32;
  return 0;
}

PROTOCOL_FUNC_PREPARE(do_download_syspara)
{
  sp_tcp_trans_unit tcp_struct;
  int ret = 0;
  memset(&tcp_struct, 0, sizeof(sp_tcp_trans_unit));
  //header
  prepare_header_by_cmd(SP_CMD_GET_SYSPARA, &tcp_struct.header);
  //data
  tcp_struct.header.data_len = 4;
  sp_uint32_to_buffer(ctx->syspara.syspara_verno, tcp_struct.data);
  //��������
  ret = prepare_output_parameter(send_buffer, buffer_max_len, data_len, &tcp_struct);
  if(ret)
    return ret;
  //����check_sum
  calc_send_buffer_checksum(send_buffer, data_len);
  return 0;
}

PROTOCOL_FUNC_RESP(syspara_resp_handler)
{
  uint16 offset = 0;
  sp_print_row(FLAG_CLEAR, 1, 0, "����ϵͳ��������");
  //updflag
  ctx->svrpara.upd_flag = buffer[offset++];
  if(ctx->svrpara.upd_flag)
  {
    //����ʱ����
    ctx->syspara.heartgap = buffer[offset++];
    //����ģʽ
    ctx->syspara.work_mode = buffer[offset++];
    //ϵͳʱ��
    memcpy(ctx->syspara.today_date, buffer + offset, 4);
    offset += 4;
    offset += 3;
    //����ʱ���1
    memcpy(ctx->syspara.truss_period1, buffer + offset, 2);
    offset += 2;
    //����ʱ���2
    memcpy(ctx->syspara.truss_period2, buffer + offset, 2);
    offset += 2;
    //����ʱ���3
    memcpy(ctx->syspara.truss_period3, buffer + offset, 2);
    offset += 2;
  }
  return 0;
}

PROTOCOL_FUNC_PREPARE(do_download_timepara)
{
  sp_tcp_trans_unit tcp_struct;
  int ret = 0;
  memset(&tcp_struct, 0, sizeof(sp_tcp_trans_unit));
  //header
  prepare_header_by_cmd(SP_CMD_GET_TIMEPARA, &tcp_struct.header);
  //data
  tcp_struct.header.data_len = 2;
  tcp_struct.data[0] = 0x01;
  tcp_struct.data[0] = 0x02;
  //��������
  ret = prepare_output_parameter(send_buffer, buffer_max_len, data_len, &tcp_struct);
  if(ret)
    return ret;
  //����check_sum
  calc_send_buffer_checksum(send_buffer, data_len);
  return 0;
}

PROTOCOL_FUNC_RESP(timepara_resp_handler)
{
  sp_print_row(FLAG_CLEAR, 1, 0, "ʱ��β�������");
  memcpy(ctx->timepara_table, buffer, sizeof(ctx->timepara_table));
  return 0;
}

PROTOCOL_FUNC_PREPARE(do_add_blacklist)
{
  sp_tcp_trans_unit tcp_struct;
  int ret = 0;
  memset(&tcp_struct, 0, sizeof(sp_tcp_trans_unit));
  //header
  prepare_header_by_cmd(SP_CMD_ADD_BLACKLIST, &tcp_struct.header);
  //data
  tcp_struct.header.data_len = 6;
  memcpy(tcp_struct.data, ctx->syspara.blacklist_verno, 6);
  //��������
  ret = prepare_output_parameter(send_buffer, buffer_max_len, data_len, &tcp_struct);
  if(ret)
    return ret;
  //����check_sum
  calc_send_buffer_checksum(send_buffer, data_len);
  return 0;
}

PROTOCOL_FUNC_RESP(add_blacklist_resp_handler)
{
  uint16 offset = 0;
  uint8 upd_flag = 0;
  uint16 blacklist_size = 0;
  sp_print_row(FLAG_CLEAR, 1, 0, "���º���������");
  //���±�־
  upd_flag = buffer[offset++];
  //��̨�������汾��
  memcpy(ctx->svrpara.blacklist_verno, buffer + offset, 6);
  offset += 6;
  if(upd_flag)
  {
    //��������
    sp_buffer_to_uint16(buffer + offset, &blacklist_size);
    offset += 2;
    if(blacklist_size > 0)
    {
      memcpy(ctx->svrpara.cardno_buffer, buffer + offset, blacklist_size);
      offset += blacklist_size;
    }
  }
  return 0;
}

PROTOCOL_FUNC_PREPARE(do_prepare_trussdtl)
{
  sp_tcp_trans_unit tcp_struct;
  int ret = 0;
  memset(&tcp_struct, 0, sizeof(sp_tcp_trans_unit));
  //header
  prepare_header_by_cmd(SP_CMD_TRUSSDTL, &tcp_struct.header);
  //data
  tcp_struct.header.data_len = sizeof(sp_transdtl);
  memcpy(tcp_struct.data, arg, tcp_struct.header.data_len);
  //��������
  ret = prepare_output_parameter(send_buffer, buffer_max_len, data_len, &tcp_struct);
  if(ret)
    return ret;
  //����check_sum
  calc_send_buffer_checksum(send_buffer, data_len);
  return 0;
}

PROTOCOL_FUNC_RESP(trussdtl_resp_handler)
{
  //������ˮָ��
  sp_recv_trussdtl* record = NULL;
  sp_trussdtl read_truss;
  int ret = 0;
  uint32 next_trussaddr = 0;
  //sp_print_row(FLAG_CLEAR, 0, 0, "rt,len=%d", data_len);
  record = (sp_recv_trussdtl*) buffer;
  //��ʾ��ˮ�Ѿ��ϴ��ɹ�
  if(record->retcode == 0)
  {
    if(memcmp(ctx->syspara.confirmed_trussno, record->trussno, 5) < 0)
    {
      if(ctx->syspara.confirmed_trussaddr < ctx->syspara.total_trussaddr)
      {
        next_trussaddr = get_next_trussdtl_addr(ctx->syspara.confirmed_trussaddr);
        ret = sp_read_trussdtl(&read_truss, next_trussaddr);
        if(ret)
          return ret;
        ctx->syspara.confirmed_trussaddr = next_trussaddr;
        memcpy(ctx->syspara.confirmed_trussno, read_truss.truss_no, 5);
        ret = sp_write_syspara(ctx);
        if(ret)
          return ret;
      }
    }
  }
  return 0;
}


//��Ÿ������ʹ���Ľӿ�
static protocol_process_handler global_process_handler[] =
{
  {SP_CMD_TEST, do_prepare_test, test_resp_handler},
  {SP_CMD_RT_TRANSDTL, do_prepare_transdtl, transdtl_resp_handler},
  {SP_CMD_AUTH, do_prepare_auth, auth_resp_handler},
  {SP_CMD_GET_BLACKLIST, do_prepare_blacklist, blacklist_resp_handler},
  {SP_CMD_HEARTBEAT, do_prepare_heartbeat, heartbeat_resp_handler},
  {SP_CMD_GET_FEERATE, do_download_feerate, feerate_resp_handler},
  {SP_CMD_GET_SYSPARA, do_download_syspara, syspara_resp_handler},
  {SP_CMD_GET_TIMEPARA, do_download_timepara, timepara_resp_handler},
  {SP_CMD_ADD_BLACKLIST, do_add_blacklist, add_blacklist_resp_handler},
  {SP_CMD_TRUSSDTL, do_prepare_trussdtl, trussdtl_resp_handler}
};



static pprotocol_process_handler do_find_tcp_handler(uint8 cmd)
{
  int  i = 0;
  int list_len = 0;
  list_len = sizeof(global_process_handler) / sizeof(protocol_process_handler);
  for(i = 0; i < list_len; i++)
  {
    if(global_process_handler[i].cmd == cmd)
    {
      return &global_process_handler[i];
    }
  }
  return NULL;
}




////////////////////////////�ⲿ����/////////////////////////////////////

/*
* ����: ��������
*/
int sp_send_request(sp_context* ctx, uint8 cmd, void* arg, uint16 time_out)
{
  uint8* send_buffer;
  uint16 send_len = 0;
  int ret = 0;
  uint32 now_tick = 0;
  pprotocol_process_handler handler = NULL;
  //�鿴��ǰʱ���Ƿ񳬹�ʧЧʱ��
  now_tick = GetTickCount();
  if(global_tcp_handler.expire_tick != 0)
  {
    if(now_tick > global_tcp_handler.expire_tick)
      return SP_E_EXCESSIVE_CALL;
  }
  //��¼ʧЧʱ��
  global_tcp_handler.expire_tick = now_tick + time_out;
  ++global_tcp_handler.current_tcp_seqno;
  handler = do_find_tcp_handler(cmd);
  send_buffer = (uint8*)malloc(MAX_SENDMSG_LEN);
  memset(send_buffer, 0, MAX_SENDMSG_LEN);
  ret = handler->prepare_req_data(ctx, send_buffer, MAX_SENDMSG_LEN, &send_len, arg);
  if(ret)
  {
    free(send_buffer);
    return ret;
  }
  ret = do_send_msg(send_buffer, send_len);
  if(ret)
  {
    free(send_buffer);
    return ret;
  }
  free(send_buffer);
  return 0;
}


/*
* ����:�ȴ����ݷ���
*/
int sp_prcocess_message(sp_context* ctx, uint8 wait_flag)
{
  uint8 recv_data[MAX_SENDMSG_LEN];
  uint16 data_len = 0;
  pprotocol_process_handler handler = NULL;
  sp_tcp_header* header = NULL;
  //���buffer��������Ƿ��Ѿ��㹻һ��������
  do_parse_resp(ctx, recv_data, &data_len);
  if(data_len <= 0)
  {
    return SP_E_BUFFER_NO_DATA;
  }
  header = (sp_tcp_header*)(recv_data);
  handler = do_find_tcp_handler(header->cmd_code);
  if(handler != NULL)
  {
    if(wait_flag == SP_SWITCH_ON)
    {
      if(header->seqno != global_tcp_handler.current_tcp_seqno)
      {
        sp_print_row(FLAG_CLEAR, 3, 0, "seqno[%d-%d,%d]", header->seqno, global_tcp_handler.current_tcp_seqno, global_tcp_handler.recv_len);
        return SP_E_SEQNO_NOT_MATCH;
      }
    }
    global_tcp_handler.expire_tick = 0;
    //ֻ��data���ִ���ȥ
    return handler->resp_process_handler(ctx, recv_data + sizeof(sp_tcp_header), header->data_len);
    //return handler->resp_process_handler(ctx, recv_data, data_len);
  }
  else
  {
    sp_print_row(FLAG_CLEAR, 3, 0, "δcmd_code=%02x", header->cmd_code);
    return SP_E_CMD_NOT_FOUND;
  }
  //sp_print_row(FLAG_DO_NOT_CLEAR, 3, 0, "δdata_len=%d", data_len);
}

void sp_init_com(void)
{
  //DCB sDCB;
  sDCB.baudrate   = 38400;
  sDCB.datab    = 8;
  sDCB.parity   = 0;
  sDCB.stopb    = 1;
  sDCB.limitlen = 1000;
  sDCB.timeout  = 100;//300
  do_init_tcp_handler();
  sDCB.pRecvfuc = do_receive_data;
  StartComm(COM1, &sDCB);
}
































#ifdef SP_DEBUG
///////////////////////////////////////////////////////////////////////
//
// ������       : SendAddrToCan
// ��������     : ����CAN�ĵ�ַ
// ����         : unsigned char cAddr �����õ�CAN��ַ
// ����ֵ       : void
//
///////////////////////////////////////////////////////////////////////
uint8 SetAddrForCan(unsigned char cAddr)
{
  uint8 cTemp[7];
  uint8 i;
  uint8 count;
  int cLength;

  //���
  cTemp[0] = 0x5a;                // ��־
  cTemp[1] = 0x01;                // ����
  cTemp[2] = cAddr;         // Ҫ���͵ĵ�ַ
  cTemp[3] = CANH;        // �ն���ϵͳ

  /*
      SF_Read(FPA_CFGS, FLA_CANbps, &cTemp[4],1); //��CANͨ������
      if(cTemp[4] == 0x02)    //20k
      cLength = 4;
    else
    {
        cTemp[1] = 0x02;             // ����
      cLength = 5;
    }
    */

  SF_Read(FPA_CFGS, FLA_CANbps, &cTemp[4], 1); //��CANͨ������0x00:5K,0x01:10K,0x02:20K,0x03:50K
  if(cTemp[4] > 0x07) // 0x04:100K,0x05:125K,0x06:250K,0x07:500K,0x08:1000K
    cTemp[4] = 0x02;
  cLength = 5;

#if(POS_COMM == COMM_INTERRUPT)
  memcpy(DataBuf, cTemp, cLength);
#endif
  // �ȴ���Ӧ
  count = 3;

  do
  {
    UART_SendBuf(COM1, cTemp, cLength);   // ����ַ��CAN
    for(i = 0x2f; i > 0 ; i--)
    {
      KillWatchDog();
      DelayNS(10);          //��Ҫ����ʱ,��Ȼ����ɹ�(ԭ�����)
      if(giRcvLen != 0)                    // ���ճ��Ȳ�Ϊ0, �յ�����
      {
        if(giRcvLen == 4)                // �������
        {
          if((rcv_buf[0] == 0x5a)  // �ж������Ƿ�Ϊ���͹�ȥ������
             && (rcv_buf[1] == 0x01)) //&&(rcv_buf[2] == cAddr)
            //&&(rcv_buf[3] == CANH)
          {
            if(rcv_buf[3] < 0x08)
            {
              rcv_buf[2] = 0;
              rcv_buf[3] = 0;
            }
            SF_Write(FPA_CFGS, FLA_CAN_VER, (uint8 *)&rcv_buf[2], 2, TRUE);
            gbResetCanAddr = FALSE;
            giRcvLen = 0;
            Beep(3);
            return TRUE;          // �ɹ����򷵻�
          }
          else
          {
            break;
          }
        }
      }
    }
    gnBaudrate = gnBaudrateBuf[3 - count];
    SF_Write(FPA_CFGS, FLA_COMM_BAUDRATE, (uint8*)&gnBaudrate, 2, TRUE);
    COMM1_Init();
    DelayNS(10);
    giRcvLen = 0;
  }
  while(--count > 0);

  // ��ʱ����յ������ݲ���ȷ����ʾ����Ա�����ϵ�ȡ��ַ
  gsPosCurrentState.sPosSystemState = State_Error;
#if(HD_POS == HD_SC)
  CLS();
  DelayNS(1);
  SP_PRINT(0, 0, "����ͨѶģ��ʧ��");
  SP_PRINT(0, 1, "��������--ȷ�ϼ�");
  SP_PRINT(0, 2, "��������ͨ�ŷ�ʽ");
  DelayNS(DISPDELAY);
#elif(HD_POS == HD_S2)
  ShowState(LED_ERROR, ERR_CommCAN);
  Beep(5) ;
#endif
  giRcvLen = 0;
  return FALSE;

}

#endif

