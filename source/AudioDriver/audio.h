
/**
  ******************************************************************************
  * @file    audio.h
  * @author  MCD Application Team
  * @brief   This header file contains the common defines and functions prototypes
  *          for the Audio driver.
   */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUDIO_H
#define __AUDIO_H
#include "ch.h"
#include "hal.h"
#include "Strust4Embedded.h"
#include <ff.h>
#include <string.h>
#include <stdint.h>
#include "PotReader.h"
#include "mad.h"
#include "SDCard.h"


#define DEFAULT_VOLUME          	  65
/* MUTE commands */
#define AUDIO_MUTE_ON                 1
#define AUDIO_MUTE_OFF                0

#define MP3_BUFF_SIZE                 1152*2*2

#ifdef STM32L476xx
#include "STM32L4_LCD.h"

#define AUDIO_I2C_ADDRESS             ((uint16_t) 0x4A)//0x94
#define SAI_CCR1                      0x002280
#define DISP_IGNORE_SECS              2

#elif defined(BOARD_ST_STM32F769I_DISCOVERY)
#include "ssd1306.h"
#define I2CD_CODEC                    	I2CD4
#define AUDIO_I2C_ADDRESS             	(0x34 >> 1)
#define SAI_CCR1                      	0x102280
#define DISP_IGNORE_SECS              	0
#define SAI_I2C_SCL_PORT   				GPIOD
#define SAI_I2C_SCL_PIN    				12U
#define SAI_I2C_SCL_AF     				4U
#define SAI_I2C_SDA_PORT   				GPIOB
#define SAI_I2C_SDA_PIN    				7U
#define SAI_I2C_SDA_AF     				11U

#define SAI_AF              			6U
#define SAI_MCLKA_PORT 					GPIOG
#define SAI_MCLKA_PIN  					7U
#define SAI_FSA_PORT   					GPIOE
#define SAI_FSA_PIN    					4U
#define SAI_SCKA_PORT  					GPIOE
#define SAI_SCKA_PIN   					5U
#define SAI_SDA_PORT   					GPIOE
#define SAI_SDA_PIN    					6U

#elif defined(BOARD_ST_STM32F746G_DISCOVERY)
#include "ssd1306.h"
#define I2CD_CODEC                    	I2CD3
#define AUDIO_I2C_ADDRESS             	(0x34 >> 1)
#define SAI_CCR1                      	0x102280
#define DISP_IGNORE_SECS              	0
#define SAI_I2C_SCL_PORT   				GPIOH
#define SAI_I2C_SCL_PIN    				7U
#define SAI_I2C_SCL_AF     				4U
#define SAI_I2C_SDA_PORT   				GPIOH
#define SAI_I2C_SDA_PIN    				8U
#define SAI_I2C_SDA_AF     				4U

#define SAI_AF              			10U
#define SAI_MCLKA_PORT 					GPIOI
#define SAI_MCLKA_PIN  					4U
#define SAI_FSA_PORT   					GPIOI
#define SAI_FSA_PIN    					7U
#define SAI_SCKA_PORT  					GPIOI
#define SAI_SCKA_PIN   					5U
#define SAI_SDA_PORT   					GPIOI
#define SAI_SDA_PIN    					6U
 
#else
#error("board not defined I2C for CODEC")
#endif
 enum AudioPlayerState {
   AP_STOPPED           = 0,
   AP_PLAYING           = 1,
   AP_SKIPPING          = 2,
   AP_PLAYING_AND_MUTE  = 3,
   AP_PAUSING           = 4
 };
 enum ActionEventEnum {
   PLAY_AUDIO        = 0,
   MUTE_PLAYER       = 1,
   VOL_UP            = 2,
   VOL_DOWN          = 3,
   NEXT_TRACK        = 4,
   PREV_TRACK        = 5,
   TOGGLE_PLAY_PAUSE = 6,
   PAUSE_PLAYER      = 7,
   IN_TRANSITION     = 8,
   VOL_SET           = 9,
   REPOSITION_ITERATOR = 10
 };

#define IS_PLAYING(x)    	(x != NULL && (x->state== AP_PLAYING || x->state == AP_PLAYING_AND_MUTE))
#define IS_PAUSING(x)    	(x != NULL && x->state== AP_PAUSING)
#define IS_PLAYING_AND_MUTE(x)    	(x != NULL && x->state== AP_PLAYING_AND_MUTE)
#define FILENAME_MAX_SIZE 	128
#define TRACK_KEY_MAX_SIZE 	12

#if S4E_USE_POT == 1
#define DEFAULT_VOLUME_TO_USE POT_VALUE_BTWN_0_TO_100
#else
#define DEFAULT_VOLUME_TO_USE DEFAULT_VOLUME
#endif

enum AudioFileType {
  AUDIO_FILE_TYPE_MP3  = 0,
  AUDIO_FILE_TYPE_WAV  = 1
};

typedef struct{
  char 		  			filename[FILENAME_MAX_SIZE];
  char 		  			trackKey[TRACK_KEY_MAX_SIZE];
  size_t        		fileSize;
  enum AudioFileType	audioFileType;
  uint16_t         		frame;
  uint16_t				trackID;
  uint16_t				durationSec;
  uint16_t				secondsRemaining;
  uint32_t 				totalSecondsPlayed;
}AudioFileInfo_TypeDef;

typedef struct
{
  uint32_t  (*Init)(uint16_t, uint16_t, uint8_t, uint32_t);
  void      (*DeInit)(void);
  uint32_t  (*ReadID)(uint16_t);
  uint32_t  (*Play)(uint16_t, uint16_t*, uint16_t);
  uint32_t  (*Pause)(uint16_t);
  uint32_t  (*Resume)(uint16_t);
  uint32_t  (*Stop)(uint16_t, uint32_t);
  uint32_t  (*SetFrequency)(uint16_t, uint32_t);
  uint32_t  (*SetVolume)(uint16_t, uint8_t);
  uint32_t  (*SetMute)(uint16_t, uint32_t);
  uint32_t  (*SetOutputMode)(uint16_t, uint8_t);
  uint32_t  (*Reset)(uint16_t);
  uint32_t  address;
}CodecDriverITF_Typedef;


typedef enum ActionEventEnum ActionEventEnum_Typedef;
typedef enum AudioPlayerState AudioPlayerState_Typedef;
typedef enum mad_flow (*onEventCallback_t)(void *data);
typedef enum mad_flow (*onErrorCallback_t)(void *data,  struct mad_stream *stream);
typedef struct{
  AudioFileInfo_TypeDef         *pAudioFileInfo;
  LinkedList 					*audioFileList;
  onErrorCallback_t	    	    onErrorCallback;
  onEventCallback_t	    	    onPreReadCallback;
  onEventCallback_t	    	    onPostReadCallback;
  onEventCallback_t	    	    onWriteCallback;
  onEventCallback_t	    	    onHeaderCallback;
  onEventCallback_t	    	    onEndCallback;
  char 		  					repositionToTrack[TRACK_KEY_MAX_SIZE];
  __IO thread_reference_t 	    playerThdRef;
  __IO ActionEventEnum_Typedef  actionEventEnum;
  int8_t 			  			mute;
  __IO AudioPlayerState_Typedef state;
  CodecDriverITF_Typedef        *pCodecDriverITF;
  bool                          waitAny;
  SDCardDriverITF_Typedef     	*pSDCardDriverITF;
}AudioPlayerDriverITF_Typedef;




#ifdef __cplusplus
 extern "C" {
#endif

void AUDIO_IO_Init(void);
void AUDIO_IO_DeInit(void);
void AUDIO_IO_Exchange(void);
void AUDIO_IO_ReInit2(void);
void AUDIO_IO_ReInit(uint32_t AudioFreq);
int decodeMP3File(AudioPlayerDriverITF_Typedef  *pmadInfo);
FRESULT processFileFromSDCard(char *fpath);
AudioPlayerDriverITF_Typedef *getAudioPlayerDriver(void);
CodecDriverITF_Typedef *getCodecDriver(void);
const char *getAPStateName(uint8_t ndx);
uint8_t getAPStateShortName(uint8_t ndx);
FRESULT playAudioFile(AudioFileInfo_TypeDef* pAudioFileInfo);
LinkedListElement* findAudioFileInfo(LinkedListElement* pItem, void *arg);
void sendJSONTracksDropdown(void);
//returns none null if found, else return null to continue looking for a match
LinkedListElement* findAudioFileInfoByTrackKey(LinkedListElement* pItem, void *arg);
void onTrackStartedPlaying(void);
#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_H */
