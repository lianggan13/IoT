#ifndef __SYN6288_H
#define __SYN6288_H

#include "sys.h"

void SYN_FrameInfo(u8 Music, u8 *HZdata);
void SYN_Send(u8 *HZdata);
void YS_SYN_Set(u8 *Info_data);

void TestSYN6288(void);

#endif