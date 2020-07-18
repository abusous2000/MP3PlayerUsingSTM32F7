/*
 * CS43L22.h
 *
 *  Created on: Jul 9, 2020
 *      Author: abusous2000
 */
#include "Strust4EmbeddedConf.h"
#if INCLUDE_CS43L22 != 0
#ifndef SOURCE_AUDIODRIVER_CS43L22_H_
#define SOURCE_AUDIODRIVER_CS43L22_H_
#include "ch.h"
#include "hal.h"
#include "audio.h"

#define OUTPUT_DEVICE_HEADPHONE       	((uint16_t)0x0002)
#define AUDIO_FREQUENCY_192K          	((uint32_t)192000)
#define CS43L22_ID            			0xE0
#define CS43L22_ID_MASK       			0xF8
#define CS43L22_CHIPID_ADDR    			0x01


#define AUDIO_RESET_LINE				PAL_LINE(GPIOD, 4U)//LINE_ARD_D15
#endif /* SOURCE_AUDIODRIVER_CS43L22_H_ */

#ifdef __cplusplus
 extern "C" {
#endif


#ifdef __cplusplus
 }
#endif
#endif
