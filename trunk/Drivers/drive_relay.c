#include "STC15F2K60S2.h"
#include "drive_relay.h"
#include "sys_delay.h"

sbit relay0 = P1 ^ 3;
sbit relay1 = P1 ^ 4;

void drv_relayInit(void)
{
    relay0 = 0;
    relay1 = 0;
    P1M1 &= ~0x18;      //�������
    P1M0 |= 0x18;
}

void drv_relayOpen(UINT8 ch)
{
    if(ch == 0)
    {
        relay0 = 1;
    }
    else if(ch == 1)
    {
        relay1 = 1;
    }
    else if(ch == ALL_RELAY)
    {
        relay0 = 1;
        relay1 = 1;
    }
}

void drv_relayClose(UINT8 ch)
{
    if(ch == 0)
    {
        relay0 = 0;
    }
    else if(ch == 1)
    {
        relay1 = 0;
    }
    else if(ch == ALL_RELAY)
    {
        relay0 = 0;
        relay1 = 0;
    }
}

