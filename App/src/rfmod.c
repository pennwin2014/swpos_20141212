

#include "rfmod.h"


typedef uint16(* adpu_device_func)(adpu_cmd_t* cmd);

/////////////////////////////////////////////////////////////////////////////////////////
static uint16 sam_adpu_proxy(adpu_cmd_t* cmd)
{
  /*
    *******************************************************************************
    **�������ƣ�CosCommand
    **�������ܣ�PSAM �� Apdu ָ���������
    **��ڲ�����
    **pSData :��������ָ�룬cSNum���������ݳ���
    **���ڲ���:pRData :��������ָ�룬cRNum���������ݳ���ָ��
    **���أ�COS ָ�����
    *******************************************************************************
    */
  uint16 ret = 0;//, i = 0;
  // char show_msg[1000];
  ret = CosCommand(cmd->send_buf, cmd->send_len, cmd->recv_buf, &cmd->recv_len);
  /*
    memset(show_msg, 0 , 1000);
    for(i = 0; i < cmd->send_len; i++)
    {
      sprintf(show_msg + 2 * i, "%02x", cmd->send_buf[i]);
    }
    sprintf(show_msg + 2 * i, "  [sendlen=%d]", cmd->send_len);
    sp_wait_for_retkey(SP_KEY_CONFIRM, __FUNCTION__, __LINE__, show_msg);

    memset(show_msg, 0 , 1000);
    for(i = 0; i < cmd->recv_len; i++)
    {
      sprintf(show_msg + 2 * i, "%02x", cmd->recv_buf[i]);
    }
    sprintf(show_msg + 2 * i, "  [ret=%04x]{recvlen=%d}", ret, cmd->recv_len);
    sp_wait_for_retkey(SP_KEY_CONFIRM,  __FUNCTION__, __LINE__, show_msg);

  */
  cmd->sw = ret;
  return ret;

}

static uint16 cpu_adpu_proxy(adpu_cmd_t* cmd)
{
  /*
  Name: CpuCard_Apdu
  Function:Ӧ��Э�����ݵ�Ԫ��Ϣ��������
  InPara: cCID:�̶�Ϊ 0x0F ��ʹ�� CID
  cSendLen:�������ݳ���
  cSendBuf:������������
  OutPara: cRcvLen: �������ݳ���
  cRcvBuf: ������������
  return: ����ִ�гɹ� 0x9000
  ��������ʧ�� 0x0001
  ���ճ��ȴ��� 0x0002
  Э����� 0x0003
  CID �������ֵ 0x0004
  ������ֵΪ�������

  */
  uint8 sw1, sw2;
  uint16 ret = 0;
  ret = CpuCard_Apdu(0x0F, cmd->send_len, cmd->send_buf, &cmd->recv_len, cmd->recv_buf);
  sw1 = cmd->recv_buf[cmd->recv_len - 2];
  sw2 = cmd->recv_buf[cmd->recv_len - 1];
  cmd->sw = (sw1 << 8) | sw2;
  return ret;
}

static int do_adpu_command(adpu_cmd_t* cmd, adpu_device_func func)
{
  func(cmd);
  if(cmd->sw != COS_R_SUCCESS)
    return cmd->sw;
  return 0;
}

int sp_cpu_adpu_command(adpu_cmd_t* cmd)
{
  return do_adpu_command(cmd, cpu_adpu_proxy);
}
int sp_sam_adpu_command(adpu_cmd_t* cmd)
{
  return do_adpu_command(cmd, sam_adpu_proxy);
}

int sp_cpucard_poweron(void)
{
  int ret = 0;
  sp_cpucard_deselect();//�����ķ���ֵ
  ret = sp_cpu_reset_card();
  if(ret)
  {
    //sp_disp_error("CPU ResetCard_A fail");
    return 1;
  }

  return 0;
}
int sp_cpucard_deselect(void)
{
  int ret = 0;
  ret = Deselect_A();
  if(FM1715_NOTAGERR == ret)
  {
    //sp_disp_error("CPU Deselect_A fail");
  }
  else if(FM1715_OK == ret)
  {
    //sp_disp_error("CPU Deselect_A ok");
  }
  else
  {
    //sp_disp_error("CPU Deselect_A else, ret = %d", ret);
  }
  return ret;
}

int sp_cpu_reset_card()
{
  int ret = 0;
  ret = (ResetCard_A() == TRUE) ? 0 : 1;
  if(ret)
    return ret;
  return 0;
}

int sp_cpu_deselect_card(void)
{
  return Deselect_A() == 0 ? 1 : 0;
}

int sp_cpu_select_df03(void)
{
  uint8 cSendBuf[7] = {0x00, 0xA4, 0x00, 0x00, 0x02, 0xDF, 0x03};
  uint8 cRcvBuf[64], cRcvLen;
  return CpuCard_Apdu(0x0f, 7, cSendBuf, &cRcvLen, cRcvBuf);
}

int sp_read_uid(uint8* uid)
{
  return ReadICCardUID(uid);
}

uint8 sp_hl_active(uint8 Block_Adr, uint8 hKey, uint8 const *cKey)
{
  return HL_Active(Block_Adr, hKey, cKey) == 0 ? 1 : 0;
}

uint8 sp_m1_readblock(uint8 *buff, uint8 Block_Adr)
{
  return MIF_ReadBlock(buff, Block_Adr) == 0 ? 1 : 0;
}

bool sp_m1_readcard(uint8 cBlock, uint8 hKey, uint8 const *cKey, uint8 *pData, uint8 cTry)
{
  return MIF_ReadCard(cBlock, hKey, cKey, pData, cTry);
}

uint8 sp_request(uint8 mode)
{
  return Request(mode);// == 0 ? 1 : 0;
}



