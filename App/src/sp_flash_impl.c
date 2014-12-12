#include "sp_flash_impl.h"
//////////////////////////˽�к���///////////////////////////////







////////////////////////////////�ⲿ����//////////////////////////////////////
void sp_get_trussno_unit(sp_trussno_unit* punit)
{
  sp_read_trussno_unit(SP_FLAG_SLAVE, punit);
}

uint8 sp_set_blacklist_by_cardno(sp_context* ctx)
{
  return 0;
}

uint32 sp_get_next_transdtl_addr(uint32 transdtl_addr)
{
  //����Ҫ�����һҳ�絽��һҳ�ˣ�Ҳ����һҳɾ��
  if(transdtl_addr + sizeof(sp_transdtl) > ADDR_TRANSDTL_END)
  {
    //�費��Ҫ����һҳɾ��
    //�Ƿ�Ҫ������Ҫ��������ҳ����ˮδ�ϴ�������??????
    return ADDR_TRANSDTL_START;
  }
  else
  {
    return transdtl_addr + sizeof(sp_transdtl);
  }
}

uint8 sp_update_transdtl_info(uint32 trans_addr, sp_transdtl* ptransdtl)
{
  uint8 ret = 0;
  uint32 page_no = 0;
  uint32 offset_addr = 0;
  sp_transdtl read_transdtl;
  //sp_disp_msg(30, "update left 9bytes ,page_addr=%08x", page_addr);
  sp_protocol_crc((uint8 *)(ptransdtl), sizeof(sp_transdtl) - 2, ptransdtl->crc);
  //
  page_no = trans_addr / FLASH_PAGE_SIZE;
  offset_addr = trans_addr - page_no * FLASH_PAGE_SIZE;
  ret = sp_flash_write(page_no, offset_addr, (uint8*)(ptransdtl), sizeof(sp_transdtl), 1);
  if(ret)
    return SP_E_FLASH_WRITE;
  offset_addr = trans_addr - page_no * FLASH_PAGE_SIZE;
  memset(&read_transdtl, 0, sizeof(read_transdtl));
  ret = sp_flash_read(page_no, offset_addr, (uint8*)&read_transdtl, sizeof(sp_transdtl));
  if(ret)
    return SP_E_FLASH_READ;
  //sp_disp_msg(100, "crc1=[%02x%02x],crc_read=[%02x%02x]",ptransdtl->crc[0], ptransdtl->crc[1],
  //read_transdtl.crc[0], read_transdtl.crc[1]);
  if(memcmp(read_transdtl.crc, ptransdtl->crc, 2) != 0)
    return SP_E_FLASH_CRC;
  return SP_SUCCESS;
}


/*******************************************************
*** ������:   sp_write_transno_unit
*** ��������: д��(��)��ˮ��
*** ����flag:    true----> ����ˮ��;       false-----> ����ˮ��
*** ����punit: ������Ľṹ���ָ��
*** ����ֵ:  0--�ɹ�  1--ʧ��
*** ����:   ����
*** ʱ��:   2014-07-03
*********************************************************/
uint8 sp_write_transno_unit(bool flag, sp_transno_unit* punit)
{
  uint8 ret = 0;
  uint16 pageno = 0;
  uint32 lastaddr = 0, startaddr = 0;
  sp_protocol_crc((uint8*)(punit), sizeof(sp_transno_unit) - 2, punit->crc);
  if(flag == SP_FLAG_MASTER)
  {
    startaddr = ADDR_TRANSNO_MASTER;
  }
  else
  {
    startaddr = ADDR_TRANSNO_SLAVE;
  }
  pageno = startaddr / FLASH_PAGE_SIZE; //�õ�ҳ��

  lastaddr = sp_get_transno_lastaddr(pageno);//��ȡ��������Ե�ַ
  //sp_disp_msg(999, "lastaddr=%d,2*size=%d", lastaddr, 2 * sizeof(sp_transno_unit));
  if(lastaddr + 2 * sizeof(sp_transno_unit) > FLASH_PAGE_SIZE) //�鿴�Ƿ�ᳬ������ַ
  {
    //sp_disp_msg(100, "��ˮ�Ŵ洢�����������²���");
    ret = sp_flashpage_erase(pageno);    //������ҳ
    if(ret)
    {
      sp_disp_error("������ˮ�Ŵ洢��ʧ��");
      return SP_E_FLASH_ERASE;
    }
    lastaddr = 0;   //��0��ʼд
  }
  else
  {
    lastaddr = lastaddr + sizeof(sp_transno_unit);
  }
  //��lastaddr��ʼ����д
  ret = sp_flash_write(pageno, lastaddr, (uint8*)punit, sizeof(sp_transno_unit), 0);//ֱ��д
  if(ret)
  {
    sp_disp_error("ֱ��дʧ�ܣ���������д");
    ret = sp_flash_write(pageno, lastaddr, (uint8*)punit, sizeof(sp_transno_unit), 1);//��������д
  }
  return ret;

}


/*******************************************************
*** ������:   sp_read_transno_unit
*** ��������: ��ȡ������ˮ��
*** ����flag:    true ����ˮ��;false ����ˮ��
*** ����punit:  ��С��Ԫ�Ľṹ��ָ��
*** ����ֵ:  0--�ɹ�  1--ʧ��
*** ����:   ����
*** ʱ��:   2014-07-03
********************************************************/
uint8 sp_read_transno_unit(bool flag, sp_transno_unit* ptrans_unit)
{
  uint16 pageno = 0;
  uint8 ret = 0;
  uint8 read_buf[FLASH_PAGE_SIZE] = {0};
  uint32 offset = 0;
  sp_transno_unit tmp_unt;
  uint8 tmp_crc[2] = {0};
  uint16 unit_len  = 0;
  bool is_exist_before = false;

  unit_len = sizeof(sp_transno_unit);
  if(flag == SP_FLAG_MASTER)
  {
    pageno = ADDR_TRANSNO_MASTER / FLASH_PAGE_SIZE;
  }
  else
  {
    pageno = ADDR_TRANSNO_SLAVE / FLASH_PAGE_SIZE;
  }
  ret = sp_flash_read(pageno, 0, read_buf, FLASH_PAGE_SIZE);
  if(ret)
    return SP_FAIL;
  for(offset = 0; offset < FLASH_PAGE_SIZE; offset += unit_len)
  {
    memcpy(&tmp_unt, read_buf + offset, unit_len);
    sp_protocol_crc((uint8*)&tmp_unt, unit_len - 2, tmp_crc);
    //sp_disp_msg(30, "transno=%d,crc1=%02x%02x,crc2=%02x%02x", tmp_unt.last_trans_no, tmp_crc[0], tmp_crc[1], tmp_unt.crc[0], tmp_unt.crc[1]);
    if(memcmp(tmp_crc, tmp_unt.crc, 2) == 0)
    {
      memcpy(ptrans_unit, &tmp_unt, unit_len);
      is_exist_before = true;
    }
    else
    {
      break;
    }
  }
  if(is_exist_before == true)
    return SP_SUCCESS;
  else
    return SP_E_FLASH_NOT_FOUNT;
}

uint32 sp_get_transno(void)
{
  uint8 ret = 0;
  sp_transno_unit transno_unit;
  ret = sp_read_transno_unit(SP_FLAG_MASTER, &transno_unit);
  if(ret)
    return 0;
  return transno_unit.last_trans_no;
}

int32 sp_get_last_transaddr(void)
{
  uint8 ret = 0;
  sp_transno_unit transno_unit;
  ret = sp_read_transno_unit(SP_FLAG_MASTER, &transno_unit);
  if(ret)
  {
    sp_disp_error("read transno error.,ret=%04x", ret);
    return 0;
  }
  return transno_unit.last_trans_addr;
}


/*******************************************************
*** ������:   sp_write_transdtl
*** ��������:   д������ˮ��flash
*** ����:   ������ˮ�ṹ��ָ��
*** ����ֵ:  0--�ɹ�  1--ʧ��
*** ����:   ����
*** ʱ��:   2014-07-03
*********************************************************/
uint8 sp_write_transdtl(sp_context* ctx, sp_transdtl* ptransdtl)
{
  //1����ȡ������ˮ��
  uint8 ret = 0;
  uint16 pageno, write_cnt = 0, offset_addr = 0;
  uint32 next_addr = 0;
  sp_transno_unit master_unit, slave_unit;
  sp_transdtl read_transdtl;//tmp_trandtl
  uint8 read_buf[FLASH_PAGE_SIZE];
  ret = sp_read_transno_unit(SP_FLAG_MASTER, &master_unit);
  if(ret)
    return ret;
  ret = sp_read_transno_unit(SP_FLAG_SLAVE, &slave_unit);
  if(ret)
    return ret;
  //2���Ա�������ˮ���Ƿ�һ��
  if(master_unit.last_trans_no != slave_unit.last_trans_no)
  {
    sp_disp_error("����ˮ�źʹ���ˮ�Ų�һ��");
    //������ˮ�Ŵ洢�������ݸ��ǵ�����ˮ����
    pageno = ADDR_TRANSNO_SLAVE / FLASH_PAGE_SIZE;
    sp_flash_read(pageno, 0, read_buf, FLASH_PAGE_SIZE);
    pageno = ADDR_TRANSNO_MASTER / FLASH_PAGE_SIZE;
    ret = sp_flash_write(pageno, 0, read_buf, FLASH_PAGE_SIZE, 1);//������д
    //���ɹ�����ѭ��д
    write_cnt = MAX_FLASH_WRITE_CNT;
    while(ret != SP_SUCCESS)
    {
      ret = sp_flash_write(pageno, 0, read_buf, FLASH_PAGE_SIZE, 1);//������д
      if(write_cnt-- < 1)
      {
        return SP_FAIL;//���ش���
      }
    }
  }
  //3�����ݴ���ˮ����ĵ�ַ��ȡ��ˮ����
  pageno = slave_unit.last_trans_addr / FLASH_PAGE_SIZE;
  offset_addr = slave_unit.last_trans_addr - pageno * FLASH_PAGE_SIZE;
  memset(&read_transdtl, 0, sizeof(read_transdtl));
  sp_flash_read(pageno, offset_addr, (uint8*)(&read_transdtl), sizeof(read_transdtl));
  //4���Ա���ˮ�����ˮ�źʹ���ˮ�Ŵ洢���е���ˮ���Ƿ�һ��
  if(read_transdtl.termseqno != slave_unit.last_trans_no)
  {
    sp_disp_error("��ˮ�Ų����,termseqno=%d,last_trans_no=%d", read_transdtl.termseqno, slave_unit.last_trans_no);
    return SP_FAIL;
  }
  //5��д�µ�����ˮ��
  //memcpy(&tmp_trandtl, ptransdtl, sizeof(sp_transdtl));
  ////5.1����ȡ��һ����ˮ��ַ
  next_addr = sp_get_next_transdtl_addr(slave_unit.last_trans_addr);

  ////5.2�����õ�ַ�Լ���ˮ�е���ˮ��д������ˮ�Ŵ洢��
  memset((&master_unit), 0, sizeof(master_unit));
  master_unit.last_trans_no = ptransdtl->termseqno;
  master_unit.last_trans_addr = next_addr;
  memcpy(master_unit.date, ptransdtl->transdatetime, 3);
  master_unit.sum_amt += ptransdtl->amount;
  //6��д�µ�����ˮ��
  ret = sp_write_transno_unit(SP_FLAG_MASTER, &master_unit);
  write_cnt = MAX_FLASH_WRITE_CNT;
  while(ret != SP_SUCCESS)
  {
    ret = sp_write_transno_unit(true, &master_unit);
    if(write_cnt-- < 1)
    {
      return SP_FAIL;
    }
  }
  //7��д�µĴ���ˮ��
  ret = sp_write_transno_unit(SP_FLAG_SLAVE, &master_unit);
  write_cnt = MAX_FLASH_WRITE_CNT;
  while(ret != SP_SUCCESS)
  {
    ret = sp_write_transno_unit(SP_FLAG_SLAVE, &master_unit);
    if(write_cnt-- < 1)
    {
      return SP_FAIL;
    }
  }
  write_cnt = MAX_FLASH_WRITE_CNT;
  while(1)
  {
    //8�� д������ˮ���Ǻڿ�������ˮ��Ҫ�ں���������º���9�ֽ�
    // sp_disp_msg(100, "write transdtl in addr=%08x", master_unit.last_trans_addr);
    pageno = master_unit.last_trans_addr / FLASH_PAGE_SIZE;
    offset_addr = master_unit.last_trans_addr - FLASH_PAGE_SIZE * pageno;
    sp_flash_write(pageno, offset_addr, (uint8*)ptransdtl, sizeof(sp_transdtl), 1);//ʹ�ò�����д�ķ�ʽ
    //9����ȡһ����ˮ�����Ƿ���ڴ��е�һ��
    sp_flash_read(pageno, offset_addr, (uint8*)(&read_transdtl), sizeof(sp_transdtl));
    if(read_transdtl.termseqno == ptransdtl->termseqno)
    {
      break;
    }
    if(write_cnt-- < 1)
    {
      return SP_FAIL;
    }
  }
  //����һ����ˮͳ����Ϣ
  ctx->syspara.total_transaddr = master_unit.last_trans_addr;
  ctx->syspara.total_transno = ctx->record.termseqno;
  ret = sp_write_syspara(ctx);
  if(ret)
  {
    sp_disp_error("����ϵͳ��Ϣʧ��, ret=%04x", ret);
    return ret;
  }

  return SP_SUCCESS;
}



/*******************************************************
*** ������:   sp_read_transdtl
*** ��������: ��ȡ��ˮ����
*** ����ptransdtl:  ��ˮ�Ľṹ��ָ��
*** ����ֵ:  0--�ɹ�  1--ʧ��
*** ����:   ����
*** ʱ��:   2014-07-03
*********************************************************/
uint8 sp_read_transdtl(sp_transdtl* ptransdtl, uint32 trans_addr)
{
  uint8 ret = 0;
  uint16 pageno = 0, offset_addr = 0;
  uint8 tmp_crc[2];
  //1��������ˮ����ĵ�ַ��ȡ��ˮ
  pageno = trans_addr / FLASH_PAGE_SIZE;
  offset_addr = trans_addr - pageno * FLASH_PAGE_SIZE;
  ret =  sp_flash_read(pageno, offset_addr, (uint8*)ptransdtl, sizeof(sp_transdtl));
  if(ret)
    return ret;
  //2�����crc�Ƿ���ȷ
  sp_protocol_crc((uint8*)ptransdtl, sizeof(sp_transdtl) - 2, tmp_crc);
  if(memcmp(tmp_crc, ptransdtl->crc, 2) != 0)
  {
    return SP_E_FLASH_CRC;
  }
  return SP_SUCCESS;
}

/*
*  ˵��:��ҳ��������ֽڴ��crc
*/
uint8 sp_write_timepara_table(sp_context* ctx)
{
  uint16 pageno = 0;
  int32 ret = 0;
  uint8 timepara_table[FLASH_PAGE_SIZE];
  uint16 table_size = 0;
  uint8 crc[2];
  //����crc��crc����crc��λ��
  table_size = 4 * sizeof(sp_timepara);
  memset(timepara_table, 0, FLASH_PAGE_SIZE);
  memcpy(timepara_table, ctx->timepara_table, table_size);
  sp_protocol_crc(timepara_table, table_size, timepara_table + FLASH_PAGE_SIZE - 2);
  //����ҳ��
  pageno = ADDR_TIMEPARA / FLASH_PAGE_SIZE;
  //�Ȳ���ԭ����һ��ҳ
  ret = sp_flashpage_erase(pageno);
  if(ret)
  {
    sp_disp_error("sp_flashpage_erase fail,ret=%d", ret);
    return SP_E_FLASH_ERASE;
  }
  ret = sp_flash_write(pageno, 0, timepara_table, FLASH_PAGE_SIZE, 1);
  if(ret)
  {
    sp_disp_error("sp_flash_write fail,ret=%d", ret);
    return SP_E_FLASH_WRITE;
  }
  //���¶�ȡ�����ö�ȡ������У��crc
  memset(timepara_table, 0, FLASH_PAGE_SIZE);
  ret = sp_flash_read(pageno, 0, timepara_table, FLASH_PAGE_SIZE);
  if(ret)
  {
    sp_disp_error("sp_flash_read fail,ret=%d", ret);
    return SP_E_FLASH_READ;
  }
  sp_protocol_crc(timepara_table, table_size, crc);
  if(memcmp(timepara_table + FLASH_PAGE_SIZE - 2, crc, 2) != 0)
  {
    sp_disp_error("calc crc fail");
    return SP_E_FLASH_CRC;
  }
  return SP_SUCCESS;
}

uint8 sp_read_timepara_table(sp_context* ctx)
{
  uint16 pageno = 0;
  uint8 read_buffer[FLASH_PAGE_SIZE];
  int32 ret = 0;
  uint8 crc[2];
  uint16 read_size = 0;
  pageno = ADDR_TIMEPARA / FLASH_PAGE_SIZE;
  ret = sp_flash_read(pageno, 0, read_buffer, FLASH_PAGE_SIZE);
  if(ret)
    return SP_E_FLASH_READ;
  read_size = sizeof(ctx->timepara_table);
  //����һ��crc
  sp_protocol_crc(read_buffer, read_size, crc);
  if(memcmp(read_buffer + FLASH_PAGE_SIZE - 2, crc, 2) != 0)
  {
    return SP_E_FLASH_CRC;
  }
  memcpy(ctx->timepara_table, read_buffer, read_size);
  return SP_SUCCESS;
}

uint8 sp_write_feerate_table(sp_context* ctx)
{
  uint16 pageno = 0;
  int32 ret = 0;
  uint8 read_feerate_table[256];
  memset(read_feerate_table, 0, 256);
  //����crc��crc����crc��λ��
  ctx->feerate_table[255] = sp_calc_crc8(ctx->feerate_table, 255);
  //����ҳ��
  pageno = ADDR_FEERATE / FLASH_PAGE_SIZE;
  //�Ȳ���ԭ����һ��ҳ
  ret = sp_flashpage_erase(pageno);
  if(ret)
  {
    sp_disp_error("sp_flashpage_erase fail,ret=%d", ret);
    return SP_E_FLASH_ERASE;
  }
  ret = sp_flash_write(pageno, 0, ctx->feerate_table, 256, 1);
  if(ret)
  {
    sp_disp_error("sp_flash_write fail,ret=%d", ret);
    return SP_E_FLASH_WRITE;
  }
  //���¶�ȡ�����ö�ȡ�����ݼ���crc
  ret = sp_flash_read(pageno, 0, read_feerate_table, 256);
  if(ret)
  {
    sp_disp_error("sp_flash_read fail,ret=%d", ret);
    return SP_E_FLASH_READ;
  }
  read_feerate_table[255] = sp_calc_crc8(read_feerate_table, 255);
  if(ctx->feerate_table[255] != read_feerate_table[255])
  {
    sp_disp_error("calc crc fail");
    return SP_E_FLASH_CRC;
  }
  return SP_SUCCESS;
}

uint8 sp_read_feerate_table(sp_context* ctx)
{
  uint16 pageno = 0;
  uint8 crc = 0;
  int32 ret = 0;
  pageno = ADDR_FEERATE / FLASH_PAGE_SIZE;
  ret = sp_flash_read(pageno, 0, ctx->feerate_table, 256);
  if(ret)
    return SP_E_FLASH_READ;
  //����һ��crc���������������1
  crc = sp_calc_crc8(ctx->feerate_table, 255);
  if(ctx->feerate_table[255] != crc)
  {
    return SP_E_FLASH_CRC;
  }
  else
  {
    return SP_SUCCESS;
  }

}

/*******************************************************
*** ������:   sp_write_sysinfo
*** ��������:   дϵͳ������flash
*** ����:   ȫ�ֽṹ��ָ��
*** ����ֵ:  0--�ɹ�  1--ʧ��
*** ����:   ����
*** ʱ��:   2014-07-03
*********************************************************/
uint8 sp_write_syspara(sp_context* ctx)
{
  uint16 pageno = 0;
  int32 ret = 0;
  sp_syspara read_sysinfo;
  memset(&read_sysinfo, 0, sizeof(sp_syspara));
  //����crc��crc����crc��λ��
  sp_protocol_crc((uint8*)(&ctx->syspara), sizeof(sp_syspara) - 2, ctx->syspara.crc);
  //����ҳ��
  pageno = ADDR_SYSINFO / FLASH_PAGE_SIZE;
  //�Ȳ���ԭ����һ��ҳ
  ret = sp_flashpage_erase(pageno);
  if(ret)
  {
    sp_disp_error("sp_flashpage_erase fail,ret=%d", ret);
    return SP_E_FLASH_ERASE;
  }
  ret = sp_flash_write(pageno, 0, (uint8*)(&ctx->syspara), sizeof(sp_syspara), 1);
  if(ret)
  {
    sp_disp_error("sp_flash_write fail,ret=%d", ret);
    return SP_E_FLASH_WRITE;
  }
  //���¶�ȡ�����ö�ȡ�����ݼ���crc
  ret = sp_flash_read(pageno, 0, (uint8*)(&read_sysinfo), sizeof(sp_syspara));
  if(ret)
  {
    sp_disp_error("sp_flash_read fail,ret=%d", ret);
    return SP_E_FLASH_READ;
  }
  sp_protocol_crc((uint8*)(&read_sysinfo), sizeof(sp_syspara) - 2, read_sysinfo.crc);
  if(memcmp(ctx->syspara.crc, read_sysinfo.crc, 2) != 0)
  {
    sp_disp_error("calc crc fail,[%02x%02x]==[%02x%02x]", ctx->syspara.crc[0], ctx->syspara.crc[1]
                  , read_sysinfo.crc[0], read_sysinfo.crc[1]);
    return SP_E_FLASH_CRC;
  }
  return SP_SUCCESS;
}


/*******************************************************
*** ������:   sp_read_syspara
*** ��������: ��ȡϵͳ��Ϣ
*** ����:  ȫ�ֽṹ��ָ��
*** ����ֵ:  0--�ɹ�  1--ʧ��
*** ����:   ����
*** ʱ��:   2014-07-03
*********************************************************/
uint8 sp_read_syspara(sp_context* ctx)
{
  uint16 pageno = 0;
  uint8 crc[2] = {0};
  int32 ret = 0;
  pageno = ADDR_SYSINFO / FLASH_PAGE_SIZE;
  ret = sp_flash_read(pageno, 0, (uint8*)(&ctx->syspara), sizeof(ctx->syspara));
  if(ret)
    return SP_E_FLASH_READ;
  //����һ��crc���������������1
  sp_protocol_crc((uint8*)(&ctx->syspara), sizeof(sp_syspara) - 2, crc);
  if(memcmp(ctx->syspara.crc, crc, 2) != 0)
  {
    return SP_E_FLASH_CRC;
  }
  else
  {
    return SP_SUCCESS;
  }
}

uint8 sp_read_blacklist(sp_context* ctx)
{
  return 0;
}

uint8 sp_write_blacklist(sp_context* ctx, uint16 offset_addr)
{
  int pageno = 0;
  int ret = 0;
  pageno = (ADDR_BLACKLIST + offset_addr) / FLASH_PAGE_SIZE;
  //�Ȳ���ԭ����һ��ҳ
  ret = sp_flashpage_erase(pageno);
  if(ret)
  {
    sp_disp_error("sp_flashpage_erase fail,ret=%d", ret);
    return SP_E_FLASH_ERASE;
  }
  ret = sp_flash_write(pageno, 0, ctx->svrpara.tmp_blacklist_bitmap, FLASH_PAGE_SIZE, 1);
  if(ret)
  {
    sp_disp_error("sp_flash_write fail,ret=%d", ret);
    return SP_E_FLASH_WRITE;
  }

  return 0;
}

//��ȡ��ˮ�Ŵ洢�������һ����Ч��ַ
uint16 sp_get_transno_lastaddr(uint16 pageno)
{
  int16 offset = 0;
  uint8 ret = 0;
  int16 unit_len = 0;
  sp_transno_unit tmp_unt;
  uint8 crc[2] = {0};
  uint8 page_buffer[FLASH_PAGE_SIZE];
  bool is_exist_before = false;
  uint16 result_offset = FLASH_PAGE_SIZE;
  unit_len = sizeof(sp_transno_unit);
  //һ���԰�һҳȫ������
  memset(page_buffer, 0, FLASH_PAGE_SIZE);
  ret = sp_flash_read(pageno, 0, page_buffer, FLASH_PAGE_SIZE);
  if(ret)
  {
    sp_disp_error("��ȡ����, ret=%04x", ret);
    return FLASH_PAGE_SIZE;
  }
  for(offset = 0; offset < FLASH_PAGE_SIZE; offset += unit_len)
  {
    memcpy(&tmp_unt, page_buffer + offset, unit_len);
    sp_protocol_crc((uint8*)(&tmp_unt), unit_len - 2, crc);
    //sp_disp_error("trano=%d,c1=%02x%02x,c2=%02x%02x", tmp_unt.last_trans_no, crc[0], crc[1], tmp_unt.crc[0], tmp_unt.crc[1]);
    if(memcmp(tmp_unt.crc, crc, 2) == 0)
    {
      is_exist_before = true;
      result_offset = offset;
    }
    else
    {
      break;
    }
  }
  if(is_exist_before == true)
    return result_offset;
  else
    return FLASH_PAGE_SIZE;
}

uint32 get_next_trussdtl_addr(uint32 trussdtl_addr)
{
  //����Ҫ�����һҳ�絽��һҳ�ˣ�Ҳ����һҳɾ��
  if(trussdtl_addr + sizeof(sp_trussdtl) > ADDR_TRUSSDTL_END)
  {
    //�費��Ҫ����һҳɾ��
    //�Ƿ�Ҫ������Ҫ��������ҳ����ˮδ�ϴ�������??????
    return ADDR_TRUSSDTL_START;
  }
  else
  {
    return trussdtl_addr + sizeof(sp_trussdtl);
  }
}
static uint32 get_trussno_lastaddr(uint16 pageno)
{
  int16 offset = 0;
  uint8 ret = 0;
  int16 unit_len = 0;
  sp_trussno_unit trussno_unit;
  uint8 crc[2] = {0};
  uint8 page_buffer[FLASH_PAGE_SIZE];
  uint8 correct_unit_cnt = 0;
  uint16 result_offset = FLASH_PAGE_SIZE;
  unit_len = sizeof(sp_trussno_unit);
  //һ���԰�һҳȫ������
  memset(page_buffer, 0, FLASH_PAGE_SIZE);
  ret = sp_flash_read(pageno, 0, page_buffer, FLASH_PAGE_SIZE);
  if(ret)
  {
    sp_disp_error("��ȡ����, ret=%04x", ret);
    return FLASH_PAGE_SIZE;
  }
  for(offset = 0; offset < FLASH_PAGE_SIZE; offset += unit_len)
  {
    memcpy(&trussno_unit, page_buffer + offset, unit_len);
    sp_protocol_crc((uint8*)(&trussno_unit), unit_len - 2, crc);
    //sp_disp_error("trano=%d,c1=%02x%02x,c2=%02x%02x", tmp_unt.last_trans_no, crc[0], crc[1], tmp_unt.crc[0], tmp_unt.crc[1]);
    if(memcmp(trussno_unit.crc, crc, 2) == 0)
    {
      correct_unit_cnt++;
      result_offset = offset;
    }
    else
    {
      break;
    }
  }
  if(correct_unit_cnt > 0)
    return result_offset;
  else
    return FLASH_PAGE_SIZE;
}

uint8 sp_write_trussno_unit(bool flag, sp_trussno_unit* trussno_unit)
{
  uint8 ret = 0;
  uint16 pageno = 0;
  uint32 lastaddr = 0, startaddr = 0;
  sp_protocol_crc((uint8*)(trussno_unit), sizeof(sp_trussno_unit) - 2, trussno_unit->crc);
  if(flag == SP_FLAG_MASTER)
  {
    startaddr = ADDR_TRUSSNO_MASTER;
  }
  else
  {
    startaddr = ADDR_TRUSSNO_SLAVE;
  }
  pageno = startaddr / FLASH_PAGE_SIZE; //�õ�ҳ��
  lastaddr = get_trussno_lastaddr(pageno);//��ȡ��������Ե�ַ
  //��Ϊ����ʼ��ַ�����Լӵ���2*sizeof
  if(lastaddr + 2 * sizeof(sp_trussno_unit) > FLASH_PAGE_SIZE) //�鿴�Ƿ�ᳬ������ַ
  {
    //sp_disp_msg(100, "��ˮ�Ŵ洢�����������²���");
    ret = sp_flashpage_erase(pageno);    //������ҳ
    if(ret)
    {
      sp_disp_error("������ˮ�Ŵ洢��ʧ��");
      return SP_E_FLASH_ERASE;
    }
    lastaddr = 0;   //��0��ʼд
  }
  else
  {
    lastaddr = lastaddr + sizeof(sp_trussno_unit);
  }
  //��lastaddr��ʼ����д
  ret = sp_flash_write(pageno, lastaddr, (uint8*)trussno_unit, sizeof(sp_trussno_unit), 0);//ֱ��д
  if(ret)
  {
    sp_disp_error("ֱ��дʧ�ܣ���������д");
    ret = sp_flash_write(pageno, lastaddr, (uint8*)trussno_unit, sizeof(sp_trussno_unit), 1);//��������д
  }
  return ret;
}

uint8 sp_read_trussno_unit(bool flag, sp_trussno_unit* punit)
{
  uint16 pageno = 0;
  uint8 ret = 0;
  uint8 read_buf[FLASH_PAGE_SIZE] = {0};
  uint32 offset = 0;
  sp_trussno_unit trussno_unt;
  uint8 tmp_crc[2] = {0};
  uint16 unit_len  = 0;
  uint8 correct_unit_cnt = 0;

  unit_len = sizeof(sp_trussno_unit);
  if(flag == SP_FLAG_MASTER)
  {
    pageno = ADDR_TRUSSNO_MASTER / FLASH_PAGE_SIZE;
  }
  else
  {
    pageno = ADDR_TRUSSNO_SLAVE / FLASH_PAGE_SIZE;
  }
  ret = sp_flash_read(pageno, 0, read_buf, FLASH_PAGE_SIZE);
  if(ret)
    return SP_FAIL;
  for(offset = 0; offset < FLASH_PAGE_SIZE; offset += unit_len)
  {
    memcpy(&trussno_unt, read_buf + offset, unit_len);
    sp_protocol_crc((uint8*)&trussno_unt, unit_len - 2, tmp_crc);
    //sp_disp_msg(30, "transno=%d,crc1=%02x%02x,crc2=%02x%02x", tmp_unt.last_trans_no, tmp_crc[0], tmp_crc[1], tmp_unt.crc[0], tmp_unt.crc[1]);
    if(memcmp(tmp_crc, trussno_unt.crc, 2) == 0)
    {
      memcpy(punit, &trussno_unt, unit_len);
      correct_unit_cnt++;
    }
    else
    {
      break;
    }
  }
  if(correct_unit_cnt > 0)
    return SP_SUCCESS;
  else
    return SP_E_FLASH_NOT_FOUNT;
}

//hhmm
uint8 sp_get_truss_index_by_transtime(sp_context* ctx, const uint8 hm[2])
{
  //��Ҫ��24Сʱ�Ĵ���С�ڣ������Ǿ�ȷ���ֵ�ʱ��
  if(sp_is_in_period(SP_TRUSS_PERIOD0, ctx->syspara.truss_period1, hm))
    return 1;
  else if(sp_is_in_period(ctx->syspara.truss_period1, ctx->syspara.truss_period2, hm))
    return 2;
  else if(sp_is_in_period(ctx->syspara.truss_period2, ctx->syspara.truss_period3, hm))
    return 3;
  else if(sp_is_in_period(ctx->syspara.truss_period3, SP_TRUSS_PERIOD0, hm))
    return 4;
  else//�����ܳ���
    return 0;
}


//transtime ----yymmddhhmmss
void sp_generate_trussno_by_transdatetime(sp_context* ctx, const uint8 transtime[6], uint8 trussno[5])
{
  trussno[0] = 0x20;
  memcpy(trussno + 1, transtime, 3);
  trussno[4] = sp_get_truss_index_by_transtime(ctx, transtime + 3);
}


uint8 sp_read_trussdtl(sp_trussdtl* trussdtl, uint32 truss_addr)
{
  uint8 ret = 0;
  uint16 pageno = 0, offset_addr = 0;
  uint16 len = 0;
  uint8 crc[2];
  //1��������ˮ����ĵ�ַ��ȡ��ˮ
  pageno = truss_addr / FLASH_PAGE_SIZE;
  offset_addr = truss_addr - pageno * FLASH_PAGE_SIZE;
  len = sizeof(sp_trussdtl);
  ret =  sp_flash_read(pageno, offset_addr, (uint8*)trussdtl, len);
  if(ret)
    return ret;
  //2�����crc�Ƿ���ȷ
  sp_protocol_crc((uint8*)trussdtl, len - 2, crc);
  if(memcmp(crc, trussdtl->crc, 2) != 0)
  {
    return SP_E_FLASH_CRC;
  }
  return SP_SUCCESS;
}

uint8 sp_write_trussdtl(sp_context* ctx, sp_trussdtl* trussdtl)
{
  //1����ȡ������ˮ��
  uint8 ret = 0;
  uint16 pageno, write_cnt = 0, offset_addr = 0;
  uint32 next_addr = 0;
  sp_trussno_unit master_unit, slave_unit;
  sp_trussdtl read_trussdtl;
  uint8 read_buf[FLASH_PAGE_SIZE];
  ret = sp_read_trussno_unit(SP_FLAG_MASTER, &master_unit);
  if(ret)
    return ret;
  ret = sp_read_trussno_unit(SP_FLAG_SLAVE, &slave_unit);
  if(ret)
    return ret;
  //2���Ա�������ˮ���Ƿ�һ��
  if(master_unit.last_truss_addr != slave_unit.last_truss_addr)
  {
    sp_disp_error("�����������δ洢��������ˮ��ַ��һ��");
    //������ˮ�Ŵ洢�������ݸ��ǵ�����ˮ����
    pageno = ADDR_TRUSSNO_SLAVE / FLASH_PAGE_SIZE;
    sp_flash_read(pageno, 0, read_buf, FLASH_PAGE_SIZE);
    pageno = ADDR_TRUSSNO_MASTER / FLASH_PAGE_SIZE;
    ret = sp_flash_write(pageno, 0, read_buf, FLASH_PAGE_SIZE, 1);//������д
    //���ɹ�����ѭ��д
    write_cnt = MAX_FLASH_WRITE_CNT;
    while(ret != SP_SUCCESS)
    {
      ret = sp_flash_write(pageno, 0, read_buf, FLASH_PAGE_SIZE, 1);//������д
      if(write_cnt-- < 1)
      {
        return SP_FAIL;//���ش���
      }
    }
  }
  //3�����ݴӵ�Ԫ��ĵ�ַ��ȡ��ˮ����
  pageno = slave_unit.last_truss_addr / FLASH_PAGE_SIZE;
  offset_addr = slave_unit.last_truss_addr - pageno * FLASH_PAGE_SIZE;
  memset(&read_trussdtl, 0, sizeof(read_trussdtl));
  sp_flash_read(pageno, offset_addr, (uint8*)(&read_trussdtl), sizeof(read_trussdtl));
  //4���Ա���ˮ������κźʹӴ洢���е����һ�����κ��Ƿ�һ��
  if(memcmp(read_trussdtl.truss_no, slave_unit.last_truss_no, 5) != 0)
  {
    sp_disp_error("���κŲ����,r[%02x],s[%02x]", read_trussdtl.truss_no[0], slave_unit.last_truss_no[0]);
    return SP_FAIL;
  }
  //5��д�µ������������κ�
  ////5.1����ȡ��һ��������ˮ��ַ
  next_addr = get_next_trussdtl_addr(slave_unit.last_truss_addr);
  ////5.2�����õ�ַ�Լ���ˮ�е���ˮ��д������ˮ�Ŵ洢��
  memset((&master_unit), 0, sizeof(master_unit));
  master_unit.last_truss_addr = next_addr;
  memcpy(master_unit.last_truss_no, trussdtl->truss_no, 5);
  sp_protocol_crc((uint8*)(&master_unit), sizeof(master_unit), master_unit.crc);
  //6��д�µ�����ˮ��
  ret = sp_write_trussno_unit(SP_FLAG_MASTER, &master_unit);
  write_cnt = MAX_FLASH_WRITE_CNT;
  while(ret != SP_SUCCESS)
  {
    ret = sp_write_trussno_unit(SP_FLAG_MASTER, &master_unit);
    if(write_cnt-- < 1)
    {
      return SP_FAIL;
    }
  }
  //7��д�µĴ���ˮ��
  ret = sp_write_trussno_unit(SP_FLAG_SLAVE, &master_unit);
  write_cnt = MAX_FLASH_WRITE_CNT;
  while(ret != SP_SUCCESS)
  {
    ret = sp_write_trussno_unit(SP_FLAG_SLAVE, &master_unit);
    if(write_cnt-- < 1)
    {
      return SP_FAIL;
    }
  }
  write_cnt = MAX_FLASH_WRITE_CNT;
  while(1)
  {
    //8�� д������ˮ���Ǻڿ�������ˮ��Ҫ�ں���������º���9�ֽ�
    // sp_disp_msg(100, "write transdtl in addr=%08x", master_unit.last_trans_addr);
    pageno = master_unit.last_truss_addr / FLASH_PAGE_SIZE;
    offset_addr = master_unit.last_truss_addr - FLASH_PAGE_SIZE * pageno;
    ret = sp_flash_write(pageno, offset_addr, (uint8*)trussdtl, sizeof(sp_trussdtl), 1);//ʹ�ò�����д�ķ�ʽ
    if(ret)
      return SP_E_FLASH_WRITE;
    sp_sleep(10);
    //9����ȡһ����ˮ�����Ƿ���ڴ��е�һ��
    ret = sp_flash_read(pageno, offset_addr, (uint8*)(&read_trussdtl), sizeof(sp_trussdtl));
    if(ret)
      return SP_E_FLASH_READ;
    if(memcmp(read_trussdtl.truss_no, trussdtl->truss_no, 5) == 0)
    {
      break;
    }
    if(write_cnt-- < 1)
    {
      return SP_FAIL;
    }
  }
  //����һ����ˮͳ����Ϣ
  ctx->syspara.total_trussaddr = master_unit.last_truss_addr;
  memcpy(ctx->syspara.total_trussno, trussdtl->truss_no, 5);
  ret = sp_write_syspara(ctx);
  if(ret)
  {
    sp_disp_error("����ϵͳ��Ϣʧ��, ret=%04x", ret);
    return ret;
  }
  return SP_SUCCESS;
}



void sp_disp_flash(int32 addr, int32 len)
{
  char disp_msg[1024];
  uint8 array[512];
  uint8 ret = 0;
  int page_no = 0, i = 0;
  uint32 counter = 0;
  uint32 offset_addr = 0;
  memset(disp_msg, 0, 1024);
  memset(array, 0, 512);
  page_no = addr / FLASH_PAGE_SIZE;
  offset_addr = addr - page_no * FLASH_PAGE_SIZE;
  counter = sp_get_min(len, 512);
  ret = sp_flash_read(page_no, offset_addr, array, counter);
  if(ret)
  {
    sp_disp_error("sp_flash_read error");
    return;
  }
  for(i = 0; i < counter; i++)
  {
    sprintf(disp_msg + 2 * i, "%02x", array[i]);
  }
  sp_wait_for_retkey(SP_KEY_MUL, __FUNCTION__, __LINE__, disp_msg);
}





