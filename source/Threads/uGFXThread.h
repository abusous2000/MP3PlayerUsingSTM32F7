/*
 * LCDThread.h
 *
 *  Created on: May 4, 2020
 *      Author: abusous2000
 */

#ifndef SOURCE_THREADS_UGFXTHREAD_H_
#define SOURCE_THREADS_UGFXTHREAD_H_
#if USE_LCD_TFT
#include "gfx.h"
#ifdef __cplusplus
 extern "C" {
#endif
void inituGFXGUI(void *arg);
void inituGFXThd(void);
void defaultuGFXHandler(GEvent 	*pe);
#ifdef __cplusplus
}
#endif
#endif
#endif /* SOURCE_THREADS_UGFXTHREAD_H_ */
