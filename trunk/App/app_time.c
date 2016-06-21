#include "app_time.h"
#include "app_config.h"
#include "drive_relay.h"
#include "drive_led.h"
#include "hwa_eeprom.h"
#include "app_test.h"
#include "string.h"

data UINT16 u16_DisplayTime[AD_CHANNEL_NUM];//ʣ��ʱ�䣬(��ʾʱ���10��)

void app_timeClear(UINT8 channel)
{
    UINT8 i;
    if(channel == AD_CHANNEL_NUM)
    {
        memset((UINT8*)u16_DisplayTime, 0x00, sizeof(u16_DisplayTime));
        for(i=0; i<AD_CHANNEL_NUM; i++)
        {
            memset(s_Money.Card_ID[i], 0x00, 5);
        }
    }
    else
    {
        u16_DisplayTime[channel] = 0;
        memset(s_Money.Card_ID[channel], 0x00, 5);
    }
    app_timeSaveTime();
    app_configWrite(MONEY_SECTOR);
}

void app_timeLoad(void)
{
    UINT8 i;
    if(hwa_eepromReadSector((UINT8*)u16_DisplayTime, TIME_SAVE_SECTOR) == FALSE)
    {
        app_timeClear(AD_CHANNEL_NUM);
    }
    else
    {
        for(i = 0; i < AD_CHANNEL_NUM; i++)
        {
            if(u16_DisplayTime[i] > 9999)
            {
                app_timeClear(AD_CHANNEL_NUM);
                break;
            }
        }
    }
}

void app_timeSaveTime(void)
{
    hwa_eepromWriteSector((UINT8*)u16_DisplayTime, TIME_SAVE_SECTOR);
}

extern BOOL b_TestMode;
void app_timeDisplay500ms(void)     //ʱ��Ϊ0ʱ��˸��ʾͨ����
{
	UINT8 channel;
	static BOOL b_lightFlashCount = FALSE;
    
    if (b_FactorySystem == FALSE)
	{
		b_lightFlashCount = !b_lightFlashCount;

		for (channel = 0; channel < AD_CHANNEL_NUM; channel++)
		{
            if(b_fuseState[channel] == FALSE)                   //����˿����
            {
                drv_ledDisplayChannel(channel, DISPALY_CH_ERROR);
                if(u16_DisplayTime[channel] != 0)               //���ʱ�䲻Ϊ0 ���ʱ��
                {
                    app_timeClear(channel);
                }
            }
            else if (u16_DisplayTime[channel] == 0)                       //ʱ��Ϊ0��˸��ʾͨ����
			{
				if (b_lightFlashCount)
				{
                    drv_ledDisplayChannel(channel, DISPLAY_NONE);
				}
				else
				{
                    drv_ledDisplayChannel(channel, DISPALY_CH_NULL);
				}
				drv_relayClose(channel);
			}
			else                                //ʱ�䲻Ϊ����ʾʱ��
			{
                drv_ledDisplayChannel(channel, u16_DisplayTime[channel]/10);
				drv_relayOpen(channel);
			}
		}
	}
}

void app_timeAddTime(UINT8 channel, UINT16 money)
{
	u16_DisplayTime[channel] += (UINT16)((UINT32)money*s_System.Time *10 / s_System.Money);    //TBD
    if(u16_DisplayTime[channel] > 9999)
    {
        u16_DisplayTime[channel] = 9999;
    }
    app_timeSaveTime();
}
		   
void app_timePower1min(void)
{
    UINT8 i;
    UINT8 tmp;
    static UINT8 u8_TimeCount10min = 0;
    for(i = 0; i < AD_CHANNEL_NUM; i++)
    {
        tmp = 10;
        if(u16_DisplayTime[i] > tmp)
        {
            u16_DisplayTime[i] -= tmp;
        }
        else if(u16_DisplayTime[i])
        {
            app_timeClear(i);
        }
    }
    if(++u8_TimeCount10min >= 10)
    {
        u8_TimeCount10min = 0;
        for(i = 0; i < AD_CHANNEL_NUM; i++)
        {
            if(u16_DisplayTime[i])
            {
                app_timeSaveTime();
                break;
            }
        }
    }
}
