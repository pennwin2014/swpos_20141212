#ifndef __CONFIG_H 
#define __CONFIG_H

//�������Ͷ���(��һ������Ķ�)
typedef unsigned char  	uint8;                  /* defined for unsigned 8-bits integer variable 	       �޷���8λ���ͱ���  */
typedef signed   char  	int8;                    /* defined for signed 8-bits integer variable		�з���8λ���ͱ���  */
typedef unsigned short  uint16;               /* defined for unsigned 16-bits integer variable 	       �޷���16λ���ͱ��� */
typedef signed   short  int16;                  /* defined for signed 16-bits integer variable 		�з���16λ���ͱ��� */
typedef unsigned int    uint32;               /* defined for unsigned 32-bits integer variable 	�޷���32λ���ͱ��� */
typedef signed   int    int32;                  /* defined for signed 32-bits integer variable 		�з���32λ���ͱ��� */
typedef float          	fp32;                   /* single precision floating point variable (32bits)     �����ȸ�������32λ���ȣ� */
typedef double          fp64;                   /* double precision floating point variable (64bits)   ˫���ȸ�������64λ���ȣ� */

typedef unsigned char 	bool;

#ifndef TRUE
#define TRUE  (uint8)1
#endif

#ifndef true
#define true  (uint8)1
#endif

#ifndef FALSE
#define FALSE (uint8)0
#endif
#ifndef false
#define false (uint8)0
#endif

#ifndef NULL
 #define NULL ((void *)0)
#endif
#ifndef null
#define null ((void *)0)
#endif

#ifndef VALID_DATA
#define VALID_DATA (uint8)2
#endif

#ifndef INVALID_DATA
#define INVALID_DATA (uint8)-1
#endif

/********************************/
/*      ARM specital code      			*/
/*      ARM���������            	*/
/********************************/

//ϵͳͷ�ļ�(��һ������Ķ�)
#include    "LPC2294.h"
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    "stdarg.h"


/********************************/
/*Application Program Configurations*/
/*     Ӧ�ó�������             */
/********************************/
//This segment could be modified as needed.
//���¸�����Ҫ�Ķ�

/********************************/
/*Configuration of the example */
/*     �����ӵ�����             */
/********************************/
/* System configuration .Fosc��Fcclk��Fcco��Fpclk must be defined */
/* ϵͳ����, Fosc��Fcclk��Fcco��Fpclk���붨��*/
#define Fosc            11059200                    //Crystal frequence,10MHz~25MHz��should be the same as actual status. 
						    //Ӧ����ʵ��һ������Ƶ��,10MHz~25MHz��Ӧ����ʵ��һ��
#define Fcclk           (Fosc * 4)                  //System frequence,should be (1~32)multiples of Fosc,and should be equal or less  than 60MHz. 
						    //ϵͳƵ�ʣ�����ΪFosc��������(1~32)����<=60MHZ
#define Fcco            (Fcclk * 4)                 //CCO frequence,should be 2��4��8��16 multiples of Fcclk, ranged from 156MHz to 320MHz. 
						    //CCOƵ�ʣ�����ΪFcclk��2��4��8��16������ΧΪ156MHz~320MHz
#define Fpclk           (Fcclk / 4) * 4             //VPB clock frequence , must be 1��2��4 multiples of (Fcclk / 4).
						    //VPBʱ��Ƶ�ʣ�ֻ��Ϊ(Fcclk / 4)��1��2��4��

#define KillWatchDog()   {IRQDisable();WDFEED =  0XAA;WDFEED =  0X55; IRQEnable();}


// �û�ͷ�ļ�
#include    "target.h"              //This line may not be deleted ��һ�䲻��ɾ��
#include    "comm.h"
#include    "Timer.h"
#include    "Mifare.h"
#include    "Lcd.h"
#include    "rtc.h"
#include    "M25PE16.h"
#include    "board.h"
#include 	"Hd7279a.h"
#include 	"psam.h"


#endif
/*********************************************************************************************************
**                            End Of File
********************************************************************************************************/
