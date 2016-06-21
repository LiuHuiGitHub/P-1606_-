#include "sys.h"
#include "app_brush.h"
#include "app_config.h"
#include "app_time.h"
#include "hwa_eeprom.h"
#include "hwa_mifare.h"
#include "sys_eeprom.h"
#include "sys_uart.h"
#include "sys_delay.h"
#include "drive_led.h"
#include "drive_key.h"
#include "mifare.h"
#include "drive_buzzer.h"
#include "string.h"
#include "app_test.h"

code UINT8 PWD_Card[] = { 0xAC, 0x1E, 0x57, 0xAF, 0x19, 0x4E };	//���뿨����
data UINT8 LastCardId[5] = {0x00,0x00,0x00,0x00,0x00};

void app_Show(void)
{
	UINT32 Money = s_Money.MoneySum / 100 % 1000000;
	if (b_FactorySystem == FALSE)
	{
		sys_delayms(1000);
		u8_ledDisBuff[0] = (UINT8)(Money / 100000);
		u8_ledDisBuff[1] = (UINT8)(Money / 10000 % 10);
		u8_ledDisBuff[2] = (UINT8)(Money / 1000 % 10);
		u8_ledDisBuff[3] = (UINT8)(Money / 100 % 10);
		u8_ledDisBuff[4] = (UINT8)(Money / 10 % 10);
		u8_ledDisBuff[5] = (UINT8)(Money % 10);
		u8_ledDot = 0;
		sys_delayms(1000);
		drv_ledRequestDisplayChannel0(s_System.Money, 1000, BIT0);
		drv_ledRequestDisplayChannel1(s_System.Time, 1000, 0);
	}
//	sys_taskDel(app_Show);
}

void app_brushInit(void)
{
	hwa_mifareInit();
}

BOOL app_BrushGetChannelState(UINT8 channel)					//��ȡͨ���Ƿ�ռ��
{
	UINT8 i;
    if(b_fuseState[channel] == FALSE)							//����˿������Ϊ�Ѿ�ռ��
    {
        return TRUE;
    }
	for (i = 0; i < 5; i++)
	{
		if (s_Money.Card_ID[channel][i] != 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}
UINT8 app_BrushGetNoUseChannel(void)							//��ȡͨ����ǰδʹ�õ�һ��ͨ��
{
	UINT8 channel;
	for (channel = 0; channel < CHANNEL_NUMBER; channel++)
	{
		if (app_BrushGetChannelState(channel) == FALSE)
		{
			return channel;
		}
	}
	return CHANNEL_NUMBER;
}

UINT8 app_BrushGetSurplusChannelNum(void)						//��ȡʣ��ͨ������
{
	UINT8 SurplusChannelNum = CHANNEL_NUMBER;
	UINT8 channel;
	for (channel = 0; channel < CHANNEL_NUMBER; channel++)
	{
		if (app_BrushGetChannelState(channel))
		{
			SurplusChannelNum--;
		}
	}
	return SurplusChannelNum;
}

void app_brushBrush(UINT8 channel, UINT16 money)				//��channel��Ǯ
{
	if (channel < CHANNEL_NUMBER)
	{
		memcpy(s_Money.Card_ID[channel], LastCardId, 5);
		app_configWrite(MONEY_SECTOR);							//�ۼ�Ӫҵ����濨��
		app_timeAddTime(channel, money);
		drv_buzzerNumber(1);
	}
	else
	{
		drv_buzzerNumber(2);
	}
	drv_ledRequestDisplayChannel0(0, 0, 0);
	drv_ledRequestDisplayChannel1(0, 0, 0);
}

BOOL app_brushNotifyBrush(UINT16 money)							//��ˢ����ѡ��ͨ��
{
	UINT8 channel;
	channel = drive_keyGetKey();
	if (channel < CHANNEL_NUMBER)
	{
        if(app_testGetFuseState(channel) == FALSE)
        {
            drv_buzzerNumber(2);
            return FALSE;
        }
		if (app_BrushGetChannelState(channel) == FALSE)
		{
			app_brushBrush(channel, money);
			return TRUE;
		}
		drv_buzzerNumber(2);
	}
	return FALSE;
}

BOOL app_brushGetChannelIdState(UINT8 channel)					//��ȡ��ͨ���Ƿ��Ѵ�������ˢ�Ŀ�ռ��
{
	if (memcmp(s_Money.Card_ID[channel], gCard_UID, 5) == 0
        && b_fuseState[channel] == TRUE)
	{
		return TRUE;
	}
	return FALSE;
}

#define NO_CHANNEL       CHANNEL_NUMBER
UINT8 ReturnCardId(void)
{
    if(app_brushGetChannelIdState(0))
    {
        return 0;
    }
    if(app_brushGetChannelIdState(1))
    {
        return 1;
    }
	return NO_CHANNEL;
}

/*�����������ؿ�Ƭ���ͣ�
	0->�޿�
	1->����
	2->���뿨
	3->�û���
	4->����
	*/
#define NONE_CARD       0
#define MEM_CARD        1
#define USER_CARD       2
#define PWD_CARD        3

UINT8 app_brushCard(void)
{
	UINT8 Sector;
	UINT8 CardIndex;
    UINT8 i;
	for (CardIndex = MEM_CARD; CardIndex <= PWD_CARD; CardIndex++)
	{
		if (b_FactorySystem)
		{
			CardIndex = PWD_CARD;
		}
		if (CardIndex == MEM_CARD)
		{
			Load_Key(&s_System.MGM_Card);
		}
		else if (CardIndex == USER_CARD)
		{
			Load_Key(&s_System.USER_Card);
		}
		else if (CardIndex == PWD_CARD)
		{
			Load_Key(PWD_Card);
		}
		MIF_Halt();
		if (Request(RF_CMD_REQUEST_STD) != FM1702_OK)
		{
			continue;
		}
        for(i=0; i<2; i++)
        {
            if (AntiColl() == FM1702_OK && SelectCard() == FM1702_OK)
            {
                if (CardIndex == USER_CARD)     //�û�����֤Ǯ��������
                {
                    Sector = s_System.Sector;
                }
                else                            //��������뿨��֤1����
                {
                    Sector = 1;
                }
                if (Authentication(gCard_UID, Sector, 0x60) == FM1702_OK)
                {
                    return CardIndex;
                }
            }
        }
	}
	return NONE_CARD;
}

void app_brushMemSetting(void)
{
	BOOL flag = FALSE;
#define U8_FIRST_BRUSH_CARD_DLY     5
	UINT8 u8_FirstBrushCardDly = 0;			//��һ��ˢ������ʾ��Ϣ���ٴ�ˢ�����
    
    app_timeClear(AD_CHANNEL_NUM);          //���ʱ��

	do
	{
		if (app_brushCard() == MEM_CARD && hwa_mifareReadBlock(gBuff, 4))
		{
			if (gBuff[0] == 0x01 && gBuff[1] == 0x0A)			//������
			{
				if (u8_FirstBrushCardDly)
				{
					s_System.Money += 10;
					if (s_System.Money > 200)
					{
						s_System.Money = 10;
					}
					flag = TRUE;
				}
				drv_ledDisplayChannel(0, s_System.Money);
				drv_ledDisplayChannel(1, 0);
				u8_ledDot = 1 << 0;
			}
			else if (gBuff[0] == 0xFA && gBuff[1] == 0x01)		//ʱ�����
			{
				if (u8_FirstBrushCardDly)
				{
					s_System.Time += 30;
					if (s_System.Time > 600)
					{
						s_System.Time = 30;
					}
					flag = TRUE;
				}
				drv_ledDisplayChannel(0, 0);
				drv_ledDisplayChannel(1, s_System.Time);
			}
            else
            {
                return;
            }
			drv_buzzerNumber(1);
			u8_FirstBrushCardDly = 1;
		}
		sys_delayms(1000);
		u8_FirstBrushCardDly++;
	} while (u8_FirstBrushCardDly < U8_FIRST_BRUSH_CARD_DLY);

	if (flag)
	{
		app_configWrite(SYSTEM_SETTING_SECTOR);
	}
}

void app_brushCycle500ms(void)
{
	UINT8 channel;
	static UINT16 u16_BrushMoney = 0;                //��ǰˢ�����
#define BRUSH_SEL_CHANNEL_TIME             120
	static UINT8 u8_BrushSelChannelTime = BRUSH_SEL_CHANNEL_TIME;

	if (u16_BrushMoney)                               //��ˢ�����ж��Ƿ�ѡ��ͨ��
	{
		if (app_brushNotifyBrush(u16_BrushMoney) || --u8_BrushSelChannelTime == 0)
		{
			if (u8_BrushSelChannelTime == 0)
			{
				app_brushBrush(app_BrushGetNoUseChannel(), u16_BrushMoney);
			}
			u16_BrushMoney = 0;
		}
	}

    switch (app_brushCard())
    {
    case MEM_CARD:
        app_brushMemSetting();
        break;

    case PWD_CARD:										//�ӳ�ʼ���ж�ȡ�������룬��������E2
        if (hwa_mifareReadBlock(gBuff, 4))			//��ȡ�������û��������Լ�����
        {
            memcpy(&s_System, gBuff, 16);
            s_System.Refund = 0;
//            s_System.Refund++;
//            s_System.Refund &= 0x01;
            if (hwa_mifareReadBlock(gBuff, 5))			//��ȡ�������û��������Լ�����
            {
                if(gBuff[0] == 0x01)
                {
                    s_System.RecoveryOldCard = 1;
                }
                else
                {
                    s_System.RecoveryOldCard = 0;
                }
            }
            else
            {
                break;
            }
            app_configWrite(SYSTEM_SETTING_SECTOR);
            app_timeClear(AD_CHANNEL_NUM);              //���ʱ��
            drv_ledRequestDisplayChannel0(s_System.Sector, 2000, 0);
            drv_ledRequestDisplayChannel1(s_System.Refund, 2000, 0);
            drv_buzzerNumber(1);
            b_FactorySystem = FALSE;
        }
        break;

    case USER_CARD:
//                memset(gBuff, 0x00, sizeof(gBuff));
//                pMoney->money = 20000;										//��Ǯ
//                if (hwa_mifareWriteSector(gBuff, s_System.Sector))
//                {
//                    drv_buzzerNumber(1);
//                }
//                break;

        if (hwa_mifareReadSector(gBuff, s_System.Sector))
        {
            channel = ReturnCardId();
            
            if (u16_BrushMoney
                && memcmp(LastCardId, gCard_UID, 5))		//���ϴοۿ�Ų�һ�£�������ˢ��
            {
                break;
            }
            if (channel == NO_CHANNEL && app_BrushGetSurplusChannelNum() == 0)     //ͨ��ȫ��ռ�ã�������ˢ���ۿ�
            {
                drv_buzzerNumber(2);
                break;
            }
            if (pMoney->money >= s_System.Money)						//ȷ��������
            {
                pMoney->money -= s_System.Money;
                if (hwa_mifareWriteSector(gBuff, s_System.Sector))
                {
                    memcpy(LastCardId, gCard_UID, 5);
                    s_Money.MoneySum += s_System.Money;					//�ۼ�Ӫҵ��

                    if (channel == NO_CHANNEL)						    //�¿����Զ�ѡ��δʹ�õ�ͨ��
                    {
                        u16_BrushMoney += s_System.Money;              //�ۼ�ˢ�����
                        drv_buzzerNumber(1);
                        drv_ledRequestDisplayChannel0(pMoney->money / 100, 0xFFFF, BIT2);		//��ʾ���
                        drv_ledRequestDisplayChannel1(pMoney->money % 100 * 10, 0xFFFF, 0);
                        sys_delayms(2000);

                        drv_ledRequestDisplayChannel0(u16_BrushMoney / 100, 0xFFFF, BIT2);//��ʾˢ�����
                        drv_ledRequestDisplayChannel1(u16_BrushMoney % 100 * 10, 0xFFFF, 0);
                    }
                    else												//ֱ�Ӽ�Ǯ
                    {
                        app_timeAddTime(channel, s_System.Money);
						app_configWrite(MONEY_SECTOR);				//����Ӫҵ��
                        u16_BrushMoney = 0;
                        drv_buzzerNumber(1);
                        sys_delayms(2000);
                    }
                    u8_BrushSelChannelTime = BRUSH_SEL_CHANNEL_TIME;
                    break;
                }
                break;
            }
            else
            {
                drv_buzzerNumber(3);
                break;
            }
        }
        break;

    default:
        break;
    }
}

