#include <STC15F2K60S2.H>
#include "drive_led.h"

sbit LED_CS0 = P1 ^ 6;
sbit LED_CS1 = P2 ^ 5;
sbit LED_CS2 = P1 ^ 5;
sbit LED_CS3 = P5 ^ 5;
sbit LED_CS4 = P5 ^ 4;
sbit LED_CS5 = P1 ^ 7;

#define CHANNEL_NUM			2

/*数码管管脚定义
P27~0
g c b f a e d h
*/

//与显示相关变量
#define SEG_PORT_A      BIT0
#define SEG_PORT_B      BIT7
#define SEG_PORT_C      BIT3
#define SEG_PORT_D      BIT6
#define SEG_PORT_E      BIT5
#define SEG_PORT_F      BIT1
#define SEG_PORT_G      BIT2
#define SEG_PORT_H      BIT4
/*Seg Coding*/
#define SEG_0           (SEG_PORT_A|SEG_PORT_B|SEG_PORT_C|SEG_PORT_D|SEG_PORT_E|SEG_PORT_F)
#define SEG_1           (SEG_PORT_B|SEG_PORT_C)
#define SEG_2           (SEG_PORT_A|SEG_PORT_B|SEG_PORT_D|SEG_PORT_E|SEG_PORT_G)
#define SEG_3           (SEG_PORT_A|SEG_PORT_B|SEG_PORT_C|SEG_PORT_D|SEG_PORT_G)
#define SEG_4           (SEG_PORT_B|SEG_PORT_C|SEG_PORT_F|SEG_PORT_G)
#define SEG_5           (SEG_PORT_A|SEG_PORT_C|SEG_PORT_D|SEG_PORT_F|SEG_PORT_G)
#define SEG_6           (SEG_PORT_A|SEG_PORT_C|SEG_PORT_D|SEG_PORT_E|SEG_PORT_F|SEG_PORT_G)
#define SEG_7           (SEG_PORT_A|SEG_PORT_B|SEG_PORT_C)
#define SEG_8           (SEG_PORT_A|SEG_PORT_B|SEG_PORT_C|SEG_PORT_D|SEG_PORT_E|SEG_PORT_F|SEG_PORT_G)
#define SEG_9           (SEG_PORT_A|SEG_PORT_B|SEG_PORT_C|SEG_PORT_D|SEG_PORT_F|SEG_PORT_G)
#define SEG_A           (SEG_PORT_A|SEG_PORT_B|SEG_PORT_C|SEG_PORT_E|SEG_PORT_F|SEG_PORT_G)
#define SEG_B           (SEG_PORT_C|SEG_PORT_D|SEG_PORT_E|SEG_PORT_F|SEG_PORT_G)
#define SEG_C           (SEG_PORT_A|SEG_PORT_D|SEG_PORT_E|SEG_PORT_F)
#define SEG_D           (SEG_PORT_B|SEG_PORT_C|SEG_PORT_D|SEG_PORT_E|SEG_PORT_G)
#define SEG_E           (SEG_PORT_A|SEG_PORT_D|SEG_PORT_E|SEG_PORT_F|SEG_PORT_G)
#define SEG_F           (SEG_PORT_A|SEG_PORT_E|SEG_PORT_F|SEG_PORT_G)
#define SEG_P           (SEG_PORT_A|SEG_PORT_B|SEG_PORT_E|SEG_PORT_F|SEG_PORT_G)
#define SEG_U           (SEG_PORT_B|SEG_PORT_C|SEG_PORT_D|SEG_PORT_E|SEG_PORT_F)
#define SEG_LINE        (SEG_PORT_G)
#define SEG_NULL        (~(SEG_8|SEG_PORT_H))
//	0,	1,	2,	3,	4,	5,	6,	7,	8,	9,	U,	-,	空,	8. ,A, P
code UINT8 ledCoding[] = { SEG_0,SEG_1,SEG_2,SEG_3,SEG_4,
                            SEG_5,SEG_6,SEG_7,SEG_8,SEG_9,
                            SEG_U,SEG_LINE,SEG_NULL,~SEG_NULL,SEG_A,SEG_P};
data UINT8 u8_ledDisBuff[CHANNEL_NUM * 3] = { 15, 11, 1, 6, 0, 6 }; // P-1606
data UINT8 u8_ledDot = 0;
static data UINT8 u8_ledIndex = 0;//从左到右为0,1,2
static data UINT8 u8_ledReqDisBuff[CHANNEL_NUM * 3];
static data UINT16 u16_ledReqDisTime[CHANNEL_NUM] = { 0, 0 };
static data UINT8 u8_ledReqDot = 0;

void drv_ledInit(void)
{
	P1M1 &= ~0xE0;      //推挽输出
	P1M0 |= 0xE0;
	P3M1 &= ~0xFF;      //推挽输出
	P3M0 |= 0xFF;
	P5M1 &= ~0x30;      //推挽输出
	P5M0 |= 0x30;
	P2M1 &= ~0x40;      //推挽输出
	P2M0 |= 0x40;
}

/* LED动态显示 */
void drv_ledHandler1ms(void)
{
	UINT8 buff, channel, place;
	LED_CS0 = 1;
	LED_CS1 = 1;
	LED_CS2 = 1;
	LED_CS3 = 1;
	LED_CS4 = 1;
	LED_CS5 = 1;
	u8_ledIndex++;
	u8_ledIndex %= 6;
	channel = u8_ledIndex / 3;
	if (u16_ledReqDisTime[channel] == 0)
	{
		buff = u8_ledDisBuff[u8_ledIndex];
		place = u8_ledDot;
	}
	else
	{
		u16_ledReqDisTime[channel]--;
		buff = u8_ledReqDisBuff[u8_ledIndex];
		place = u8_ledReqDot;
	}
	P3 = ledCoding[buff];
	if (place & (1 << u8_ledIndex))//show dot
	{
		P3 |= SEG_PORT_H;
	}
	if (u8_ledIndex == 0)
	{
		LED_CS0 = 0;
	}
	else if (u8_ledIndex == 1)
	{
		LED_CS1 = 0;
	}
	else if (u8_ledIndex == 2)
	{
		LED_CS2 = 0;
	}
	else if (u8_ledIndex == 3)
	{
		LED_CS3 = 0;
	}
	else if (u8_ledIndex == 4)
	{
		LED_CS4 = 0;
	}
	else
	{
		LED_CS5 = 0;
	}
}
//
//void drv_ledShowNumber(UINT16 L_num, UINT16 R_num, UINT8 DotPlace)
//{
//	if (L_num > 999)
//	{
//		L_num = 999;
//	}
//	if (R_num > 999)
//	{
//		R_num = 999;
//	}
//	u8_ledDisBuff[0] = (UINT8)(L_num / 100);
//	u8_ledDisBuff[1] = (UINT8)(L_num / 10 % 10);
//	u8_ledDisBuff[2] = (UINT8)(L_num % 10);
//	u8_ledDisBuff[3] = (UINT8)(R_num / 100);
//	u8_ledDisBuff[4] = (UINT8)(R_num / 10 % 10);
//	u8_ledDisBuff[5] = (UINT8)(R_num % 10);
//	u8_ledDot = DotPlace;
//}

void drv_ledDisplayChannel(UINT8 channel, UINT16 value)
{
    UINT8 place;
	if (channel >= 2)
	{
		return;
	}
    place = channel*3;
	if (value == DISPLAY_NONE)
	{
		u8_ledDisBuff[place] = 12;
		u8_ledDisBuff[place + 1] = 12;
		u8_ledDisBuff[place + 2] = 12;
	}
    else if(value == DISPALY_CH_NULL)
	{
		u8_ledDisBuff[place] = 12;
		u8_ledDisBuff[place + 1] = channel+1;
		u8_ledDisBuff[place + 2] = 12;
	}
    else
    {
        if (value > 999)
        {
            value = 999;
        }
        u8_ledDisBuff[place] = (UINT8)(value / 100);
        u8_ledDisBuff[place + 1] = (UINT8)(value / 10 % 10);
        u8_ledDisBuff[place + 2] = (UINT8)(value % 10);
    }
	u8_ledDot = 0;
}

void drv_ledRequestDisplayChannel0(UINT16 value, UINT16 time, UINT8 dot)
{
	u8_ledReqDisBuff[0] = (UINT8)(value / 100);
	u8_ledReqDisBuff[1] = (UINT8)(value / 10 % 10);
	u8_ledReqDisBuff[2] = (UINT8)(value % 10);
	u16_ledReqDisTime[0] = time;
	u8_ledReqDot &= 0xF8;
	u8_ledReqDot |= dot;
}
void drv_ledRequestDisplayChannel1(UINT16 value, UINT16 time, UINT8 dot)
{
	u8_ledReqDisBuff[3] = (UINT8)(value / 100);
	u8_ledReqDisBuff[4] = (UINT8)(value / 10 % 10);
	u8_ledReqDisBuff[5] = (UINT8)(value % 10);
	u16_ledReqDisTime[1] = time;
	u8_ledReqDot &= 0x07;
	u8_ledReqDot |= dot;
}

