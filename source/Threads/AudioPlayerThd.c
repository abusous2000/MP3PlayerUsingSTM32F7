#include "Strust4EmbeddedConf.h"
#include "ch.h"
#include "hal.h"
#include "Strust4Embedded.h"
#include "PotReader.h"
#include "audio.h"
#include "AudioPlayerThd.h"
#include "wm8994.h"

extern AudioPlayerDriverITF_Typedef	*pAudioPlayerDriverITF;
thread_t                        	*libMadThd;

static THD_WORKING_AREA(waAudioPlayerThd, AUDIO_THD_WA_STACK_SIZE);// Min. WA is 0x2000 for libmad to work, else it will crash
static THD_FUNCTION(AudioPlayerThd, arg) {
  (void)arg;
  chRegSetThreadName("AudioPlayerThd");
  CodecDriverITF_Typedef        *pCodecDriverITF = pAudioPlayerDriverITF->pCodecDriverITF;
  SDCardDriverITF_Typedef  	    *pSDCardDriverITF= pAudioPlayerDriverITF->pSDCardDriverITF;

  libMadThd=chThdGetSelfX();
  if ( pSDCardDriverITF->init == NULL )
	  pSDCardDriverITF = getSDCardDriver();
  pSDCardDriverITF->init(pSDCardDriverITF);
  uint32_t deviceID = pCodecDriverITF->ReadID(AUDIO_I2C_ADDRESS);
  deviceID = pCodecDriverITF->ReadID(AUDIO_I2C_ADDRESS);
  pCodecDriverITF->Reset(AUDIO_I2C_ADDRESS);
  if ( WM8994_ID != deviceID ){
	  chSysHalt("Invalid device ID");
  }
  pCodecDriverITF->Reset(AUDIO_I2C_ADDRESS);
  if(0 != pCodecDriverITF->Init(AUDIO_I2C_ADDRESS, OUTPUT_DEVICE_HEADPHONE, DEFAULT_VOLUME, AUDIO_FREQUENCY_192K)){
	  chSysHalt("Unable to Init Codec");
  }
  if(0 != pCodecDriverITF->Play(AUDIO_I2C_ADDRESS,NULL,0) ){
      chSysHalt("Unable to Play Codec");
  }
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

        	    sendJSONTracksDropdown();
    	    	while(err == FR_OK && (candidate = emlist_iterator_next(&iterator)) != NULL) {

        	    	err=playAudioFile((AudioFileInfo_TypeDef*)candidate->value);
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
      if ( err != FR_OK)
    	  chThdSleepMilliseconds(1550);
   }
}

void initAudioPlayerThd(void){
//   chThdCreateFromHeap(NULL,THD_WORKING_AREA_SIZE(0x2000),"AudioPlayerThd",  NORMALPRIO+10,    AudioPlayerThd, NULL);
//   void *wsp = chHeapAllocAligned(NULL, THD_WORKING_AREA_SIZE(0x3000), PORT_WORKING_AREA_ALIGN);
//   chThdCreateStatic(wsp,  THD_WORKING_AREA_SIZE(0x3000),  NORMALPRIO+10,     AudioPlayerThd, NULL);
      chThdCreateStatic(waAudioPlayerThd,  sizeof(waAudioPlayerThd),  NORMALPRIO+10,     AudioPlayerThd, NULL);
}
