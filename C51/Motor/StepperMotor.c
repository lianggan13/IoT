/**************************************************************************************
*		              �������ʵ��												  *
ʵ���������س���󣬰��չ����ڲ�����Ƶ���ߣ����������ת
ע�������																				  
***************************************************************************************/

#include "reg52.h"			 //���ļ��ж����˵�Ƭ����һЩ���⹦�ܼĴ���
#include<intrins.h>		//��ΪҪ�õ������ƺ��������Լ������ͷ�ļ�

typedef unsigned int u16;	  //���������ͽ�����������
typedef unsigned char u8;

sbit MOTOA = P1^0;
sbit MOTOB = P1^1;
sbit MOTOC = P1^2;
sbit MOTOD = P1^3; 	

#define SPEED 200 

/*******************************************************************************
* �� �� ��         : delay
* ��������		   : ��ʱ������i=1ʱ����Լ��ʱ10us
*******************************************************************************/
void delay(u16 i)
{
	while(i--);	
}


/*******************************************************************************
* �� �� ��       : main
* ��������		 : ������
* ��    ��       : ��
* ��    ��    	 : ��
*******************************************************************************/
// A+ -> A- -> B+ -> B-
void main()
{	
	P1=0X00;
	while(1)
	{	
		MOTOA = 1;
		MOTOB = 0;
		MOTOC = 1;
		MOTOD = 1;
		delay(SPEED);

		MOTOA = 0;
		MOTOB = 1;
		MOTOC = 1; 
		MOTOD = 1;
		delay(SPEED);

		MOTOA = 1;
		MOTOB = 1;
		MOTOC = 0;
		MOTOD = 1;
		delay(SPEED);

		MOTOA = 1;
		MOTOB = 1;
		MOTOC = 1;
		MOTOD = 0;
		delay(SPEED);
							
	}
}