/**************************************************************************************
*		              DS1302ʱ��ʵ��												  *
ע�����	ע�⽫ʱ��ģ���JP1302�̽�Ƭ�̽ӣ�
			74HC595ģ��Ķ̽�ƬJP595�ε���
			NE555ģ��Ķ̽�ƬJ11�ε���
***************************************************************************************/

#include <reg52.h>
#include <intrins.h>
#include "ds1302.h"
#include "../Utility/CommData.h"
#include "../LED/DynamicLed.h"

//---����ds1302ʹ�õ�IO��---//
sbit DSIO = P3 ^ 4;
sbit RST = P3 ^ 5;
sbit SCLK = P3 ^ 6;

// ��ַ�Ĵ�����ʽ:
// D7 1
// D6 RAM/_CK =1Ƭ��RAM��=0������ʱ�ӼĴ���ѡ��λ
// D5 ~ D1 ��ַλ
// D0 RD/_W ��дѡ��=1����=0д

//---DS1302д��Ͷ�ȡʱ����ĵ�ַ����---//
//---�� �� ʱ(24ʱ����) �� �� �� �� (���λ��дλ);-------//
unsigned char code READ_RTC_ADDR[7] = {0x81, 0x83, 0x85, 0x87, 0x89, 0x8b, 0x8d};  // ����ַ
unsigned char code WRITE_RTC_ADDR[7] = {0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c}; // д��ַ

//---DS1302ʱ�ӳ�ʼ��2016��5��7��������12��00��00�롣---//
//---�洢˳���� �� ʱ �� �� �� �� ���洢��ʽ BCD��)---//
unsigned char TIME[7] = {0, 0, 0x12, 0x07, 0x05, 0x06, 0x16};

/*******************************************************************************
 * �� �� ��         : Ds1302Write
 * ��������		   : ��DS1302�����ַ+���ݣ�
 * ��    ��         : addr,dat
 * ��    ��         : ��
 *******************************************************************************/

void Ds1302Write(unsigned char addr, unsigned char dat)
{
	unsigned char n;
	RST = 0;
	_nop_();

	SCLK = 0; // �Ƚ�SCLK�õ͵�ƽ��
	_nop_();
	RST = 1; // Ȼ��RST(CE)�øߵ�ƽ��
	_nop_();

	for (n = 0; n < 8; n++) // ��ʼ���Ͱ�λ��ַ����
	{
		DSIO = addr & 0x01; // ���ݴӵ�λ��ʼ����
		addr >>= 1;
		SCLK = 1; // ������������ʱ��DS1302��ȡ����
		_nop_();
		SCLK = 0;
		_nop_();
	}
	for (n = 0; n < 8; n++) // д��8λ����
	{
		DSIO = dat & 0x01;
		dat >>= 1;
		SCLK = 1; // ������������ʱ��DS1302��ȡ����
		_nop_();
		SCLK = 0;
		_nop_();
	}

	RST = 0; // �������ݽ���
	_nop_();
}

/*******************************************************************************
 * �� �� ��         : Ds1302Read
 * ��������		   : ��ȡһ����ַ������
 * ��    ��         : addr
 * ��    ��         : dat
 *******************************************************************************/

unsigned char Ds1302Read(unsigned char addr)
{
	unsigned char n, dat, dat1;
	RST = 0;
	_nop_();

	SCLK = 0; // �Ƚ�SCLK�õ͵�ƽ��
	_nop_();
	RST = 1; // Ȼ��RST(CE)�øߵ�ƽ��
	_nop_();

	for (n = 0; n < 8; n++) // ��ʼ���Ͱ�λ��ַ����
	{
		DSIO = addr & 0x01; // ���ݴӵ�λ��ʼ����
		addr >>= 1;
		SCLK = 1; // ������������ʱ��DS1302��ȡ����
		_nop_();
		SCLK = 0; // DS1302�½���ʱ����������
		_nop_();
	}
	_nop_();
	for (n = 0; n < 8; n++) // ��ȡ8λ����
	{
		dat1 = DSIO; // �����λ��ʼ����
		dat = (dat >> 1) | (dat1 << 7);
		SCLK = 1;
		_nop_();
		SCLK = 0; // DS1302�½���ʱ����������
		_nop_();
	}

	RST = 0;
	_nop_(); // ����ΪDS1302��λ���ȶ�ʱ��,����ġ�
	SCLK = 1;
	_nop_();
	DSIO = 0;
	_nop_();
	DSIO = 1;
	_nop_();
	return dat;
}

/*******************************************************************************
 * �� �� ��         : Ds1302Init
 * ��������		   : ��ʼ��DS1302.
 * ��    ��         : ��
 * ��    ��         : ��
 *******************************************************************************/

void Ds1302Init()
{
	unsigned char n;
	//
	Ds1302Write(0x8E, 0X00); // ��ֹд���������ǹر�д�������� д��ַ:0x8E ֵ: 0x00
	for (n = 0; n < 7; n++)	 // д��7���ֽڵ�ʱ���źţ�����ʱ��������
	{
		Ds1302Write(WRITE_RTC_ADDR[n], TIME[n]);
	}
	Ds1302Write(0x8E, 0x80); // ��д��������
}

/*******************************************************************************
 * �� �� ��         : Ds1302ReadTime
 * ��������		   : ��ȡʱ����Ϣ
 * ��    ��         : ��
 * ��    ��         : ��
 *******************************************************************************/

void Ds1302ReadTime()
{
	unsigned char n;
	for (n = 0; n < 7; n++) // ��ȡ7���ֽڵ�ʱ���źţ�����ʱ��������
	{
		TIME[n] = Ds1302Read(READ_RTC_ADDR[n]);
	}
}

void SetDateTime()
{
	Ds1302ReadTime();
	// DisplayData[0] = smgduan[TIME[2]/16];  //ʱ
	DisplayData[0] = smgduan[TIME[2] & 0xf0]; // ʱ
	DisplayData[1] = smgduan[TIME[2] & 0x0f];
	DisplayData[2] = 0x40;
	DisplayData[3] = smgduan[TIME[1] / 16]; // ��
	DisplayData[4] = smgduan[TIME[1] & 0x0f];
	DisplayData[5] = 0x40;
	DisplayData[6] = smgduan[TIME[0] / 16]; // ��
	DisplayData[7] = smgduan[TIME[0] & 0x0f];
}

void TestDS1302()
{
	Ds1302Init();
	while (1)
	{
		SetDateTime(); // ���ݴ�������
		DigDisplay();  // �������ʾ����
	}
}