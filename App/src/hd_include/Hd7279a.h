#ifndef __HD7279_H__	//�ڲ��ӿ� 
#define  __HD7279_H__
/************************************************************
			7279 ��ֵӳ��
*************************************************************/
//  (���ּ�0-9�ֱ�ΪASCII���'0'-'9',
//  +/A,-B,*/C, . �ֱ�ӳ��Ϊ'+','-','*','.'
//	�������ܼ���������
#define 	KB_MODE		0xb3 	//ģʽ��
#define 	KB_MENU		0xb4	//���ܼ�
#define   KB_ENTER		0xb5	//ȷ�ϼ�
#define 	KB_CANCEL		0xb6	//�����
#define	KB_NONE         	0x00

#define 		NOFLASH 	0xff
#define     	STANDMODE   	(uint8)0         //��׼����ģʽָʾ��
#define     	FIXMODE      		(uint8)1         //��ֵ����ģʽָʾ��
#define     	NUMMODE      	(uint8)2         //�������ģʽָʾ��	
#define     	S2KEYInit      SCKEYInit	
uint8       GetS2KEY(void);     			//S2ȡ��ֵ����
uint8       GetSCKEY(void);     			//SCȡ��ֵ����
void        SCKEYInit(bool bIsBeep);    	//7279��ʼ��   ��0��������
void        LedPrint(uint8 cRow, uint8 cArray, char *String, uint8 cFlashNum);//S2�����ʾ����	            
void 	LedCLS(uint8 cRow);         	//S2����
void        LedInit(void);        			//S2��ʾ��ʼ��
void        SelectModeLed(uint8 mode);	//ѡ������ģʽָʾ��
#endif
