#include <STC15F2K60S2.H>
#include "drive_key.h"
#include "sys_delay.h"


UINT8 drive_keyGetKey(void)
{
	UINT8 channel;
	if (P23 == 0)
	{
        sys_delayms(10);
        if(P23 == 0)
        {
            channel = 0;
        }
	}
	else if (P22 == 0)
	{
        sys_delayms(10);
        if(P22 == 0)
        {
            channel = 1;
        }
	}
	else
	{
		channel = KEY_NUMBER;
	}
	return channel;
}
