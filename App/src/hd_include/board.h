#ifndef __BOARD_H__
#define __BOARD_H__

// ������
#define    	BEEP         			1 << 21             // BEEP		(P1.21)
// TCP/IPģ���������ſ���
// �����͵������
#define     LOCK1       			1 << 11	         	// (P0.11)   
#define     LOCK2       			1 << 23            	// (P1.23)   

#define	 	TCPIP_EXIT()			IO1SET   	= LOCK2
#define 	TCPIP_ENTER()			IO1CLR		= LOCK2

#define LCDLightON()		IO1SET = LCD_BAK // ��LCD����
#define LCDLightOFF()		IO1CLR = LCD_BAK // ��LCD����
#define LEDON()			    IO0CLR = LED     // LED ��
#define LEDOFF()			IO0SET = LED     // LED ��

// ������ʾ
#define 	BP_SUPER_SHORT    		1           		// ���̴ٷ����� 30ms
#define 	BP_SHORT				3		        	// �̴ٷ����� 100ms
#define 	BP_MIDDLE				20		        	// �з����� 200ms
#define 	BP_LONG					100		        	// �������� 1000ms
#define 	BP_LONG_LONG			255

//Ӳ��ѡ��
//KeyType
#define  KMKEY  	KMKEY                    //���ڻ�
#define  S2KEY  	S2KEY                    //��ʽ�շѻ�
#define  SCKEY  	SCKEY                    //̨ʽ�շѻ�
//DispType
#define  Led        Led                  //LED��ʾ
#define  Lcd        Lcd                  //LCD��ʾ
// ��ͷ���͡�
#define  IC			IC
#define  ID			ID
#define  SelectKeyAndDisp(KeyType,DispType,READER) {\
		GetKey=Get##KeyType;\
		KeyInit=KeyType##Init;\
		Print=DispType##Print;\
		DispInit=DispType##Init;\
		CLS=DispType##CLS;\
		Init_Reader=Init##READER##Reader;\
		ReadCardUID=Read##READER##CardUID;\
		}
extern uint8 (*GetKey)(void);//ȡ��ֵ
extern void  (*KeyInit)(bool bIsBeep);//������ʼ����
extern void  (*Print)(uint8 cRow,uint8 cArray, char *String , uint8 cFlashnum);//�����ʾ����
extern void  (*DispInit)(void);//LCD���������ʾ��ʼ��
extern void  (*CLS)(uint8 cRow);//����
extern void  (*Init_Reader)(void);// ��ʼ����ͷ
extern uint8  (*ReadCardUID)(uint8 *cUID);// �������к�
/****************************************************
����: ���Ʒ��������д���
���: cCount    ���д���
����: ��
*****************************************************/
void Beep(unsigned char cCount);
void InitBoard(void);
void BeeperCtr(void);                            //���п���
void DelayNS(int dly); 
extern uint8 BattCheck(uint8 IsCheckBatter);    // ��ؼ��
extern unsigned char	cBeep ;					// ����������ʱ��

#endif
