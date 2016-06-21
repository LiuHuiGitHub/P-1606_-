#include "stc15f2k60s2.h"
#include "app_test.h"
#include "drive_relay.h"
#include "sys_delay.h"
#include "typedef.h"

UINT8 b_fuseState[2] = {0, 0};

sbit b_input_0 = P2^0;
sbit b_input_1 = P2^1;
BOOL b_lastInput_0 = FALSE;
BOOL b_lastInput_1 = FALSE;
UINT8 u8_timeCount_0 = 0;
UINT8 u8_timeCount_1 = 0;

UINT8 u8_testBuff_0 = 0x00;
UINT8 u8_testBuff_1 = 0x00;

void app_testInit(void)
{
    b_input_0 = 1;
    b_input_1 = 1;
}

void app_testHandler1ms(void)
{
    if(u8_timeCount_0 < 0xFF)
    {
        u8_timeCount_0++;
    }
    else
    {
        u8_testBuff_0 = 0x00;
    }
    if(b_input_0 != b_lastInput_0)
    {
        b_lastInput_0 = b_input_0;
        if(b_input_0)
        {
            u8_testBuff_0 >>= 1;
            if(u8_timeCount_0 > 5 && u8_timeCount_0 < 15)
            {
                u8_testBuff_0 |= 0x80;
            }
        }
        else
        {
            u8_timeCount_0 = 0;
        }
    }
    
    if(u8_timeCount_1 < 0xFF)
    {
        u8_timeCount_1++;
    }
    else
    {
        u8_testBuff_1 = 0x00;
    }
    if(b_input_1 != b_lastInput_1)
    {
        b_lastInput_1 = b_input_1;
        if(b_input_1)
        {
            u8_testBuff_1 >>= 1;
            if(u8_timeCount_1 > 5 && u8_timeCount_1 < 15)
            {
                u8_testBuff_1 |= 0x80;
            }
        }
        else
        {
            u8_timeCount_1 = 0;
        }
    }
}

BOOL app_testGetFuseState(UINT8 channel)
{
    UINT8 x, num = 0;
    drv_relayOpen(channel);
    sys_delayms(200);
    if(channel == 0)
    {
        x = u8_testBuff_0;
    }
    else
    {
        x = u8_testBuff_1;
    }
    while(x)        //计算有多少个1
    {
        x = x&(x-1);
        num++;
    }
    if(num >= 3)     
    {
        b_fuseState[channel] = TRUE;
        return TRUE;
    }
    return FALSE;
}
