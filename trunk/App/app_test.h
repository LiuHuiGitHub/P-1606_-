#ifndef __APP_TEST_H__
#define __APP_TEST_H__

#include "typedef.h"

extern UINT8 b_fuseState[2];

void app_testInit(void);
void app_testHandler1ms(void);
BOOL app_testGetFuseState(UINT8 channel);

#endif