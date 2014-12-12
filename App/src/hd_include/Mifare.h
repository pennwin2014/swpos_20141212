#ifndef __MIFARE_H__		//�ڲ��ӿ�
#define __MIFARE_H__
	// �궨��
#define 	KEYA_MODE			0x10		//��keya��֤ģʽ
#define 	KEYB_MODE			0x20		//��keyb��֤ģʽ

#define 	INCREMENT			1		//Ǯ����ֵ
#define	DECREMENT			0		//Ǯ����ֵ

#define	REQUEST_STD 		0x26	
#define	REQUEST_ALL 		0x52

#define	BLOCK0				0x01
#define	BLOCK1				0x02
#define	BLOCK2				0x04
#define	BLOCK3				0x08

//******************* ����������붨��********************
#define	FM1715_OK				0			//��ȷ
#define	FM1715_NOTAGERR		1			//�޿�
#define	FM1715_CRCERR			2			//��ƬCRCУ�����
#define	FM1715_EMPTY			3			//��ֵ�������
#define	FM1715_AUTHERR		4			//��֤���ɹ�
#define	FM1715_PARITYERR		5			//��Ƭ��żУ�����
#define	FM1715_CODEERR		6			//ͨѶ����(BCCУ���)
#define	FM1715_SERNRERR		8			//��Ƭ���кŴ���(anti-collision ����)
#define	FM1715_SELECTERR		9			//��Ƭ���ݳ����ֽڴ���(SELECT����)
#define	FM1715_NOTAUTHERR	10			//��Ƭû��ͨ����֤
#define	FM1715_BITCOUNTERR	11			//�ӿ�Ƭ���յ���λ������
#define	FM1715_BYTECOUNTERR	12			//�ӿ�Ƭ���յ����ֽ������󣨽���������Ч��
#define	FM1715_RESTERR		13			//����restore��������
#define	FM1715_TRANSERR		14			//����transfer��������
#define	FM1715_WRITEERR		15			//����write��������
#define	FM1715_INCRERR		16			//����increment��������
#define	FM1715_DECRERR		17			//����decrement��������
#define	FM1715_READERR		18			//����read��������
#define 	FM1715_LOADKEYERR		19		//����LOADKEY��������
#define 	FM1715_FRAMINGERR		20      	//FM1715֡����
#define 	FM1715_REQERR			21      		//����req��������
#define 	FM1715_SELERR			22			//����sel��������
#define 	FM1715_ANTICOLLERR		23      	//����anticoll��������
#define	FM1715_INTIVALERR		24			//���ó�ʼ����������
#define 	FM1715_READVALERR		25		//���ø߼�����ֵ��������
#define	FM1715_DESELECTERR	26
#define	FM1715_CMD_ERR		42			//�������	
#define 	FM1715_READ_ERR		43			// ��������
#define 	FM1715_WRITE_ERR		44			// д������
#define   FM1715_PROTOCOL_ERR   45                //Э�����

//  ����
extern unsigned char	gcUID[5]    ;			// ���к�
extern unsigned long     glPrevIDBak;           	// ���к�

// ����
/*
***************************************************
����: ��ʼ��Mifare��ͷ������ز���
���: ��
����: ��
****************************************************
*/
void InitICReader(void);
// ����
unsigned char MIF_ReadBlock(
				 unsigned char *buff,
				 unsigned char Block_Adr);

unsigned char MIF_WriteBlock(
				 unsigned char *buff,
				 unsigned char Block_Adr);

unsigned char HL_Active(
				 unsigned char Block_Adr,
				 unsigned char hKeyunsigned,
				 unsigned char const *cKey);

//����Ǯ������
unsigned char MIF_Value(
				 unsigned char cmd,
				 unsigned char *buff,
				 unsigned char Block_Adr);

/*
*********************************************************************************************************
** �������� ��MIF_InitPurse
** �������� ����һ�����ʼ��ΪǮ����ʽ�����Ϊ0
** ��ڲ��� : cBlock : �������ݣ�
			: hKey д��������֤��ʽ��KEYA_MODE ��ʾKeya,KEYB_MODE��ʾKeyb
			: cKey �˿��Ӧ��������Կ
			: cTry  д��ʧ�ܺ��ظ�����
** ���ڲ��� ����
** ��    �� : 0 : ��ʾ���ɹ��� 1 : ��ʾ�ɹ�
*********************************************************************************************************
*/

unsigned char MIF_InitPurse(
						 unsigned char cBlock,
						 unsigned char hKey, 
						 unsigned char const *cKey, 
						 unsigned char cTry);
/*
*********************************************************************************************************
** �������� ��MIF_Purse
** �������� ����Ǯ�����м�/������
** ��ڲ��� ��cmd ������ INCREMENT ��ʾ��Ǯ����DECREMENT ��ʾ��Ǯ��
			: cBlock : �������ݣ�
			: hKey д��������֤��ʽ��KEYA_MODE ��ʾKeya,KEYB_MODE��ʾKeyb
			: cKey �˿��Ӧ��������Կ
			: pData д��Ŀ�����
			: cTry  д��ʧ�ܺ��ظ�����
** ���ڲ��� ����
** ��    �� : 0 : ��ʾ���ɹ��� 1 : ��ʾ�ɹ�
*********************************************************************************************************
*/

bool MIF_Purse(
						 unsigned char cmd,
						 unsigned char cBlock, 
						 unsigned char hKey, 
						 unsigned char const *cKey,
						 unsigned char *pData, 
						 unsigned char cTry);


/*
****************************************************
����: request  card
���: mode    ����ģʽ REQUEST_STD ��ʾ��׼Ѱ��  REQUEST_ALL��ʾѰ���п�
����: ����ֵ
*****************************************************
*/

unsigned char Request(unsigned char mode);
/*
***************************************************
����: halt  card ͣ��
���: ��
����: ����ֵ
****************************************************
*/

unsigned char MIF_Halt(void);

/*
*********************************************************************************************************
** �������� ��MIF_ReadCard
** �������� ����������
** ��ڲ��� ��cBlock : �������ݣ�
			: hKey ����������֤��ʽ��KEYA_MODE ��ʾKeya,KEYB_MODE��ʾKeyb
			: cKey �˿��Ӧ��������Կ
			: pData �����Ŀ�����
			: cTry  ����ʧ�ܺ��ظ�����
** ���ڲ��� ����
** ��    �� : 0 : ��ʾ���ɹ��� 1 : ��ʾ�ɹ�
*********************************************************************************************************
*/

bool MIF_ReadCard(
						 unsigned char cBlock, 
						 unsigned char hKey, 
						 unsigned char const *cKey,
						 unsigned char *pData, 
						 unsigned char cTry);
/*
*********************************************************************************************************
** �������� ��MIF_ReadCard2
** �������� ����ͬһ����ǰ��������
** ��ڲ��� ��cBlock : �������ݣ�
			: hKey ����������֤��ʽ��KEYA_MODE ��ʾKeya,KEYB_MODE��ʾKeyb
			: cKey �˿��Ӧ��������Կ
			: pData �����Ŀ�����
			: cTry  ����ʧ�ܺ��ظ�����
** ���ڲ��� ����
** ��    �� : 0 : ��ʾ���ɹ��� 1 : ��ʾ�ɹ�
*********************************************************************************************************
*/
bool MIF_ReadCard2(
						 unsigned char cBlock, 
						 unsigned char hKey,
						 unsigned char const *cKey, 
						 unsigned char *pData, 
						 unsigned char cTry);
/*
*********************************************************************************************************
** �������� ��MIF_ReadCard3
** �������� ����ͬһ������������
** ��ڲ��� ��cBlock : �������ݣ�
			: hKey ����������֤��ʽ��KEYA_MODE ��ʾKeya,KEYB_MODE��ʾKeyb
			: cKey �˿��Ӧ��������Կ
			: pData �����Ŀ�����
			: cTry  ����ʧ�ܺ��ظ�����
** ���ڲ��� ����
** ��    �� : 0 : ��ʾ���ɹ��� 1 : ��ʾ�ɹ�
*********************************************************************************************************
*/
	
bool MIF_ReadCard3(
						 unsigned char cBlock, 
						 unsigned char hKey,
						 unsigned char const *cKey, 
						 unsigned char *pData, 
						 unsigned char cTry);	
/*
*********************************************************************************************************
** �������� ��MIF_WriteCard
** �������� ��д������
** ��ڲ��� ��cBlock : �������ݣ�
			: hKey д��������֤��ʽ��KEYA_MODE ��ʾKeya,KEYB_MODE��ʾKeyb
			: cKey �˿��Ӧ��������Կ
			: pData д��Ŀ�����
			: cTry  д��ʧ�ܺ��ظ�����
** ���ڲ��� ����
** ��    �� : 0 : ��ʾ���ɹ��� 1 : ��ʾ�ɹ�
*********************************************************************************************************
*/
bool MIF_WriteCard(
						 unsigned char cBlock, 
						 unsigned char hKey, 
						 unsigned char const *cKey,
						 unsigned char *pData, 
						 unsigned char cTry);
/*
*********************************************************************************************************
** �������� ��ReadICCardUID
** �������� ���������к�4���ֽ�
** ��ڲ��� ��cUID �ɹ����ʾ��Ƭ���к�
** ���ڲ��� ����
** ��    �� : 0 : ��ʾ�ɹ��� ��0 : ��ʾ���ɹ�
*********************************************************************************************************
*/

uint8 ReadICCardUID(unsigned char *cUID);	




/*
*********************************************************************************************************

***********************************************CPU����������***************************************

*********************************************************************************************************
*/

/*
*********************************************************************************************************
** �������� ��ResetCard_A
** �������� ����λ�ǽ�CPU��
** ��ڲ��� ����
** ���ڲ��� ����
** ��    �� : TRUE:�ɹ�False:ʧ��
*********************************************************************************************************
*/
uint8 ResetCard_A(void);            


/*
*****************************************************************************
Name: 		CpuCard_Apdu	
Function:	Ӧ��Э�����ݵ�Ԫ��Ϣ��������
InPara:     	cCID:�̶�Ϊ0x0F����ʹ��CID
			cSendLen:�������ݳ���
            		cSendBuf:�����������ݡ�
            
OutPara:    	cRcvLen: �������ݳ���
            		cRcvBuf: ������������
return:     ����ִ�гɹ� 0x9000
			��������ʧ�� 0x0001
			���ճ��ȴ��� 0x0002
			Э�����     0x0003
			CID�������ֵ0x0004
		    ������ֵΪ�������
*****************************************************************************
*/
uint16 CpuCard_Apdu(uint8 cCID, uint8 cSendLen, uint8 *pSendBuf, uint8 *pRcvLen, uint8 *pRcvBuf);


/*
*********************************************************************************************************
** �������� ��Deselect_A
** �������� ��ȡ����ѡ��Ŀ�Ƭ
** ��ڲ��� ����
** ���ڲ��� ����
** ��    �� : FM1715_OK:�ɹ�FM1715_NOTAGERR:ʧ��
*********************************************************************************************************
*/
uint8 Deselect_A(void) ;



/*
*********************************************************************************************************
** �������� ��Reset_Reader_TimeOut
** �������� �������ͷָ��ִ�г�ʱʱ��
** ��ڲ��� ����
** ���ڲ��� ����
** ��    �� : ��
*********************************************************************************************************
*/
void Reset_Reader_TimeOut(void);
#endif
