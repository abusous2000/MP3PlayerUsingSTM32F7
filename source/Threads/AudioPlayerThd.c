#include "Strust4EmbeddedConf.h"
#include "ch.h"
#include "hal.h"
#include "Strust4Embedded.h"
#include "PotReader.h"
#include "audio.h"
#include "AudioPlayerThd.h"
#define ERR_MSG_UNABLE_TO_INIT_CODEC    "****ERROR: Unable to Init Codec\r\n"
#define ERR_MSG_INAVLID_DEVICE_ID       "****ERROR: Invalid Codec Device ID\r\n"

#if HAL_USE_RTC != 0
static AudioFileInfo_TypeDef savedInBackupAudioFileInfo PLACE_IN_RAM_SECTION(CCM_RAM_SECTION)  = {0};
#endif
extern AudioPlayerDriverITF_Typedef	*pAudioPlayerDriverITF;
thread_t                        	*libMadThd;
static thread_reference_t   playerThdRef = NULL;

static void sendErrorAndSuspendThd(char *errorMsg){
	  pAudioPlayerDriverITF->errorMsg = errorMsg;
	  dbgprintf(ERR_MSG_UNABLE_TO_INIT_CODEC);
	  chSysLock();
	  chThdSuspendS(&playerThdRef);
	  chSysUnlock();

}
bool containsChar(char *str, char c, int max){
	for(int i = 0; i < max; ++i){
		if ( *str == c )
			return true;
		else
			++str;
	}

	return false;
}
static THD_WORKING_AREA(waAudioPlayerThd, AUDIO_THD_WA_STACK_SIZE);// Min. WA is 0x2000 for libmad to work, else it will crash
static THD_FUNCTION(AudioPlayerThd, arg) {(void)arg;
  bool 			fromSleepMode = false;
  chRegSetThreadName("AudioPlayerThd");
  CodecDriverITF_Typedef        *pCodecDriverITF = pAudioPlayerDriverITF->pCodecDriverITF;
  SDCardDriverITF_Typedef  	    *pSDCardDriverITF= pAudioPlayerDriverITF->pSDCardDriverITF;

  libMadThd=chThdGetSelfX();
  pSDCardDriverITF->init(pSDCardDriverITF);
  if(0 != pCodecDriverITF->Init(AUDIO_I2C_ADDRESS, OUTPUT_DEVICE_HEADPHONE, getCurrentVolume(), AUDIO_FREQUENCY_192K))
	  sendErrorAndSuspendThd(ERR_MSG_UNABLE_TO_INIT_CODEC);
  uint32_t deviceID = pCodecDriverITF->ReadID(AUDIO_I2C_ADDRESS);
  if ( CODEC_DEVISE_ADDRESS != deviceID )
	  sendErrorAndSuspendThd(ERR_MSG_INAVLID_DEVICE_ID);

  pCodecDriverITF->Play(AUDIO_I2C_ADDRESS,NULL,0);
#if HAL_USE_RTC != 0
  AudioFileInfo_TypeDef *pSavedInBackupAudioFileInfo = &savedInBackupAudioFileInfo;
  if ( //getRTCSystemWakeup() &&
	   containsChar(savedInBackupAudioFileInfo.trackKey,0,TRACK_KEY_MAX_SIZE) ){
	  fromSleepMode = true;
	  pAudioPlayerDriverITF->actionEventEnum = REPOSITION_ITERATOR;
	  strlcpy(pAudioPlayerDriverITF->repositionToTrack,savedInBackupAudioFileInfo.trackKey,TRACK_KEY_MAX_SIZE);
	  dbgprintf("++++On restart/waking up...Skipping to track:%s++++\r\n", pAudioPlayerDriverITF->repositionToTrack);

  }
#endif
  while(true){
      FRESULT err = pSDCardDriverITF->mount(pSDCardDriverITF);
      while(err == FR_OK){
    	  //Populate list of files only once unless it was dismounted or if it is the 1st time
    	  if ( pAudioPlayerDriverITF->audioFileList == NULL){
    		  pAudioPlayerDriverITF->audioFileList = emlist_create();
    		  err = pSDCardDriverITF->processFiles(pSDCardDriverITF);
    	  }
          if ( err == FR_OK){
        	    LinkedListIterator iterator = emlist_iterator(pAudioPlayerDriverITF->audioFileList);
        	    LinkedListElement* candidate = NULL;
				#if S4E_USE_MQTT != 0
        	    sendJSONTracksDropdown();
				#endif
    	    	while(err == FR_OK && (candidate = emlist_iterator_next(&iterator)) != NULL) {
					#if HAL_USE_RTC != 0
    	    		savedInBackupAudioFileInfo = *(AudioFileInfo_TypeDef*)candidate->value;
					#endif
    	    		if ( !fromSleepMode)
    	    			err=playAudioFile((AudioFileInfo_TypeDef*)candidate->value);
    	            fromSleepMode = false;
        	    	if ( err != FR_OK)
        	    		break;
        	    	//If prev. track was asked for; just reposition the iterator
        	    	if ( pAudioPlayerDriverITF->actionEventEnum == PREV_TRACK ){
        	    		iterator.next = candidate->prev != NULL ? candidate->prev : pAudioPlayerDriverITF->audioFileList->last;
        	    	}
        	    	else
        	    	//reposition the iterator if needed
        	    	if ( pAudioPlayerDriverITF->actionEventEnum == REPOSITION_ITERATOR && pAudioPlayerDriverITF->repositionToTrack[0] != 0){
        	    		LinkedListElement   *pItem = emlist_iterate_over2(pAudioPlayerDriverITF->audioFileList,findAudioFileInfoByTrackKey,(void*)pAudioPlayerDriverITF->repositionToTrack);

        	    		if ( pItem != NULL)
        	    			iterator.next = pItem;
        	    		else
        	    			dbgprintf("No matching track to play found for track key:: %s\r\n", pAudioPlayerDriverITF->repositionToTrack);
        	    		pAudioPlayerDriverITF->repositionToTrack[0] = 0;
        	    	}
         	    }
          }

          chThdSleepMilliseconds(1550);
      }
      pSDCardDriverITF->disconnect(pSDCardDriverITF);
      emlist_destroy(pAudioPlayerDriverITF->audioFileList,true);
      pAudioPlayerDriverITF->audioFileList = NULL;
      if ( err != FR_OK){
    	  chThdSleepMilliseconds(1550);
    	  pSDCardDriverITF->init(pSDCardDriverITF);
      }
   }
}
void audioPlayerPrepareGoingToSleep(void){
#if HAL_USE_RTC != 0
	if (  pAudioPlayerDriverITF->pAudioFileInfo != NULL )
		savedInBackupAudioFileInfo.totalSecondsPlayed = pAudioPlayerDriverITF->pAudioFileInfo->totalSecondsPlayed;
#endif
}
void initAudioPlayerThd(void){
//   chThdCreateFromHeap(NULL,THD_WORKING_AREA_SIZE(0x2000),"AudioPlayerThd",  NORMALPRIO+10,    AudioPlayerThd, NULL);
//   void *wsp = chHeapAllocAligned(NULL, THD_WORKING_AREA_SIZE(0x3000), PORT_WORKING_AREA_ALIGN);
//   chThdCreateStatic(wsp,  THD_WORKING_AREA_SIZE(0x3000),  NORMALPRIO+10,     AudioPlayerThd, NULL);
      chThdCreateStatic(waAudioPlayerThd,  sizeof(waAudioPlayerThd),  NORMALPRIO+10,     AudioPlayerThd, NULL);
}
