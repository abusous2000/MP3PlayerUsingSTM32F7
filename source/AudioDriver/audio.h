
/**
  ******************************************************************************
  * @file    audio.h
  * @author  MCD Application Team
  * @brief   This header file contains the common defines and functions prototypes
  *          for the Audio driver.
   */

/* Define to prevent recursive inclusion -------------------------------------*/
#include "Strust4EmbeddedConf.h"
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

#ifdef STM32F407xx
#include "CS43L22.h"
#define I2S_DRIVER                      I2SD3
#define I2CD_CODEC                    	I2CD1
#define I2S_ENABLE_MASK                 0x0400
#define I2SCFGR_CLEAR_MASK              ((uint16_t)0xF040)
#define VOLUME_CONVERT(x)               ((x > 100)? 100:((uint8_t)((x * 255) / 100)))
/* Codec audio Standards */
#define CODEC_STANDARD                  0x04
#define I2S_STANDARD                    I2S_Standard_Phillips

/* The 7 bits Codec address (sent through I2C interface) */
#define AUDIO_I2C_ADDRESS                0x4A  /* b00100111 */
#define I2S_Mode_MasterTx                ((uint16_t)0x0200)
#define I2S_Standard_Phillips           ((uint16_t)0x0000)
#define I2S_DataFormat_16b              ((uint16_t)0x0000)
#define I2S_MCLKOutput_Enable           ((uint16_t)0x0200)
#define I2S_AudioFreq_Default           ((uint32_t)2)
#define I2S_CPOL_Low                    ((uint16_t)0x0000)
#define CODEC_DEVISE_ADDRESS            0xE0
//No need for pull-up resistors, it was there in the STM32F407 schematic
#define AUDIO_RESET_MODE            	PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_MID2
#define AUDIO_RESET_LINE 				PAL_LINE(GPIOD, 4U)//PD4
#define AUDIO_SCL_LINE 					PAL_LINE(GPIOB, 6U)//PB6
#define AUDIO_SDA_LINE 					PAL_LINE(GPIOB, 9U)//PB9

#define AUDIO_CODEC_I2C_AF          	4
#define AUDIO_CODEC_I2C_MODE        	PAL_MODE_ALTERNATE(AUDIO_CODEC_I2C_AF) | PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_OSPEED_MID2
#define AUDIO_I2S_AF                	6
#define AUDIO_I2S_PIN_MODE          	PAL_MODE_ALTERNATE(AUDIO_I2S_AF)       | PAL_STM32_OTYPE_PUSHPULL  | PAL_STM32_OSPEED_MID2
#define AUDIO_I2S_SCK_LINE 		    	PAL_LINE(GPIOC, 10U)//PC10
#define AUDIO_I2S_SD_LINE 		   		PAL_LINE(GPIOC, 12U)//PC12
#define AUDIO_I2S_WS_LINE 		    	PAL_LINE(GPIOA, 4U) //PA4
#define AUDIO_I2S_MCK_LINE 		    	PAL_LINE(GPIOC, 7U) //PC7

#elif defined(BOARD_ST_STM32F746G_DISCOVERY)
#include "wm8994.h"
#define I2CD_CODEC                    	I2CD3
#define CODEC_DEVISE_ADDRESS            WM8994_ID
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

#define AUDIO_I2C_SCL_LINE				PAL_LINE(SAI_I2C_SCL_PORT,SAI_I2C_SCL_PIN)//PH7
#define AUDIO_I2C_SDA_LINE 				PAL_LINE(SAI_I2C_SDA_PORT,SAI_I2C_SDA_PIN)//PH8
#define AUDIO_I2C_LINE_MODE  			PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_OPENDRAIN

#define AUDIO_SAI_MCLKA_LINE 			PAL_LINE(SAI_MCLKA_PORT,SAI_MCLKA_PIN)    //PI4
#define AUDIO_SAI_FSA_LINE 	    		PAL_LINE(SAI_FSA_PORT,SAI_FSA_PIN)        //PI7
#define AUDIO_SAI_SCKA_LINE 			PAL_LINE(SAI_SCKA_PORT,SAI_SCKA_PIN)      //PI5
#define AUDIO_SAI_SDA_LINE 	    		PAL_LINE(SAI_SDA_PORT,SAI_SDA_PIN)        //PI6
#define AUDIO_SAI_PIN_MODE 	    		PAL_MODE_ALTERNATE(SAI_AF) | PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_OSPEED_HIGHEST

#elif defined(BOARD_ST_STM32F769I_DISCOVERY)
#include "wm8994.h"
#define I2CD_CODEC                    	I2CD4
#define CODEC_DEVISE_ADDRESS            WM8994_ID
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
#define AUDIO_I2C_SCL_LINE				PAL_LINE(SAI_I2C_SCL_PORT, SAI_I2C_SCL_PIN)//PD12
#define AUDIO_I2C_SDA_LINE 				PAL_LINE(SAI_I2C_SDA_PORT, SAI_I2C_SDA_PIN)//PB7
#define AUDIO_I2C_LINE_MODE  			PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_OPENDRAIN

#define AUDIO_SAI_MCLKA_LINE 			PAL_LINE(SAI_MCLKA_PORT,   SAI_MCLKA_PIN)    //PG7
#define AUDIO_SAI_FSA_LINE 	    		PAL_LINE(SAI_FSA_PORT,     SAI_FSA_PIN)        //PE4
#define AUDIO_SAI_SCKA_LINE 			PAL_LINE(SAI_SCKA_PORT,    SAI_SCKA_PIN)      //PE5
#define AUDIO_SAI_SDA_LINE 	    		PAL_LINE(SAI_SDA_PORT,     SAI_SDA_PIN)        //PE6
#define AUDIO_SAI_PIN_MODE 	    		PAL_MODE_ALTERNATE(SAI_AF) | PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_OSPEED_HIGHEST
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

#define IS_PLAYING(x)    			(x != NULL && (x->state== AP_PLAYING || x->state == AP_PLAYING_AND_MUTE))
#define IS_PAUSING(x)    			(x != NULL && x->state== AP_PAUSING)
#define IS_PLAYING_AND_MUTE(x)    	(x != NULL && x->state== AP_PLAYING_AND_MUTE)
#define FILENAME_MAX_SIZE 	   		128
#define TRACK_KEY_MAX_SIZE 	  		12
#define TRACK_TITLE_MAX_SIZE  		50
#define TRACK_ARTIST_MAX_SIZE 		30

#if S4E_USE_POT != 0
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
  char 		  			title[TRACK_TITLE_MAX_SIZE];
  char 		  			artist[TRACK_ARTIST_MAX_SIZE];
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
  char                          *errorMsg;
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
