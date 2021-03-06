#include "Strust4EmbeddedConf.h"
#include "ch.h"
#include "hal.h"
#include "Strust4Embedded.h"
#include "ActionEvents.h"
#include "audio.h"
#include "gui.h"
#include "P9813_RGB_Driver.h"
#include "PPMFrameDecoder.h"
#include "EByteLora.h"


static int8_t 			  						mute 	   			= AUDIO_MUTE_OFF;
static int8_t                                   pause               = 0;
#if HAL_USE_RTC != 0
static int8_t 		  							volume  PLACE_IN_RAM_SECTION(CCM_RAM_SECTION)   			= DEFAULT_VOLUME;
#else
static int8_t 		  							volume     			= DEFAULT_VOLUME;
#endif
extern AudioPlayerDriverITF_Typedef				*pAudioPlayerDriverITF;

int8_t getCurrentVolume(void){
	return volume;
}
int8_t getCurrentMute(void){
	return mute;
}
#define PLAYER_CURRENTLY_MUTE  		(mute == 1)
#define PLAYER_CURRENTLY_NOT_MUTE   (mute == 0)
static void addAEConsoleText(ActionEvent_Typedef 	*pActionEvent, char *msg){
	char  payload[120]	= {0};

	if ( msg !=NULL )
		chsnprintf(payload,sizeof(payload),"%s--->AE:%s\tSource:%s\tData:%d\n",msg, pActionEvent->name,pActionEvent->eventSource,pActionEvent->u.data);
	else
		chsnprintf(payload,sizeof(payload),"AE:%s\tSource:%s\tData:%d\n",msg, pActionEvent->name,pActionEvent->eventSource,pActionEvent->u.data);

	guiAddTextToConsole(payload);
}
#if S4E_USE_EBYTE_LORA != 0
void eByteProcessReceivedMsg(EByteLoRaFrame_TypeDef	*pEByteLoRaFrame, MyMessage_TypeDef *pMyPayload){
  	dbgprintf("+++FrameID:%d\tHostID:%d\tAddH:%d\tAddL:%d\tChannel:%d\tMsgTypeId:%d\tVolume:%d\tButtons:%d\r\n",
  			  pEByteLoRaFrame->frameID,  	pEByteLoRaFrame->hostID,     pEByteLoRaFrame->fromAddressHigh, 	pEByteLoRaFrame->fromAddressLow,
			  pEByteLoRaFrame->fromChannel, pEByteLoRaFrame->msgTypeID,  pMyPayload->volume,          		pMyPayload->buttons);
  	if ( pMyPayload->volume )
			triggerActionEvent(SET_VOLUME_AE_NAME,NULL,pMyPayload->volume,SOURCE_EVENT_LORA);
  	if ( pMyPayload->buttons ){
  	 	if ( pMyPayload->buttons & 0b1)
  			triggerActionEvent(TOGGLE_MUTE_AE_NAME,NULL,pMyPayload->buttons,SOURCE_EVENT_LORA);
  	  	else
  		if ( pMyPayload->buttons & 0b10)
  			triggerActionEvent(TOGGLE_PAUSE_AE_NAME,NULL,pMyPayload->buttons,SOURCE_EVENT_LORA);
  	  	else
  		if ( pMyPayload->buttons & 0b100)
  			triggerActionEvent(NEXT_TRACK_AE_NAME,NULL,pMyPayload->buttons,SOURCE_EVENT_LORA);
  	  	else
  		if ( pMyPayload->buttons & 0b100)
  			triggerActionEvent(PREV_TRACK_AE_NAME,NULL,pMyPayload->buttons,SOURCE_EVENT_LORA);
  	}

  	eByteSendAckMsg(pEByteLoRaFrame);

	return;
}
#endif

static int32_t toggleMute(ActionEvent_Typedef 	*pActionEvent){(void)pActionEvent;
	bool ignore = false;
	if ( pActionEvent!= NULL && pActionEvent->u.data > 0){
		  if ( PLAYER_CURRENTLY_MUTE ){
			  if (pActionEvent->u.data == MUTE_PLAYER_NOW)
				  ignore = true;
		  }
		  else{
			  if ( pActionEvent->u.data == UNMUTE_PLAYER_NOW)
				  ignore = true;
		  }
		dbgprintf("***Request to pause or not.. will be ignore. This is an invalid state..%d,%d\r\n",pause,pActionEvent->u.data);

		if ( ignore)
		  return MSG_OK;
	}


	pAudioPlayerDriverITF->mute = mute= !mute;
	pAudioPlayerDriverITF->pCodecDriverITF->SetMute(AUDIO_I2C_ADDRESS,mute);
	pAudioPlayerDriverITF->state =  mute?AP_PLAYING_AND_MUTE:AP_PLAYING;

	#if USE_LCD_TFT != 0
	addAEConsoleText(pActionEvent,"Toggling Mute");
	checkboxMute(mute);
	#endif

	return MSG_OK;
}
#define PLAYER_CURRENTLY_PAUSING  		(pause == 1)
#define PLAYER_CURRENTLY_PLAYING   		(pause == 0)
#define SLEEP_MS_AFTER_TOGGLING_PAUSE    2
static int32_t togglePausePlay(ActionEvent_Typedef 	*pActionEvent){(void)pActionEvent;
  getHeapUsageInfo();
  dbgprintf("Hard Rest(0)/Wakeup(2):%d\t%d\r\n", getRTCSystemWakeup(), (int32_t)(PWR->CSR1 & PWR_CSR1_SBF) );

  bool ignore = false;
  if ( pActionEvent != NULL && pActionEvent->u.data > 0){
	  if ( PLAYER_CURRENTLY_PAUSING ){
		  if (pActionEvent->u.data == PAUSE_PLAYER_NOW)
			  ignore = true;
	  }
	  else{
	      if ( pActionEvent->u.data == UNPAUSE_PLAYER_NOW)
	    	  ignore = true;
	  }
      dbgprintf("***Request to pause or not.. will be ignore. This is an invalid state..%d,%d\r\n",pause,pActionEvent->u.data);

      if ( ignore)
    	  return MSG_OK;
  }

  if ( PLAYER_CURRENTLY_PLAYING && IS_PLAYING(pAudioPlayerDriverITF) ){
     mute = AUDIO_MUTE_ON;
     pAudioPlayerDriverITF->pCodecDriverITF->SetMute(AUDIO_I2C_ADDRESS,mute);
	 pAudioPlayerDriverITF->actionEventEnum = PAUSE_PLAYER;
	 dbgprintf("***Pausing....\r\n");
	 pause = 1;
  }
  else
  if ( PLAYER_CURRENTLY_PAUSING && pAudioPlayerDriverITF->playerThdRef != NULL ){
     mute = AUDIO_MUTE_OFF;
     pAudioPlayerDriverITF->pCodecDriverITF->SetMute(AUDIO_I2C_ADDRESS,mute);
	 chThdResume((thread_reference_t*)&(pAudioPlayerDriverITF->playerThdRef),MSG_OK);
	 dbgprintf("***Resuming....\r\n");
	 pause = 0;
  }
  else
      dbgprintf("***AE is ignored since player isn't playing....%d,%d\r\n",pause,pAudioPlayerDriverITF->actionEventEnum);
  #if USE_LCD_TFT!= 0
  addAEConsoleText(pActionEvent,"Toggling Pause");
  if ( pActionEvent != NULL )
	  checkboxPause(pause);
  #endif
  return MSG_OK;
}

static int32_t setVolume(ActionEvent_Typedef   *pActionEvent){(void)pActionEvent;
   uint8_t newVolume = pActionEvent->dataType==CHAR_DTYPE? atoi(pActionEvent->u.pData): (int8_t)pActionEvent->u.data;

   if ( pAudioPlayerDriverITF->pCodecDriverITF->SetVolume(AUDIO_I2C_ADDRESS,newVolume) )
      dbgprintf("Vol Down I2C Failed\r\n");
   volume = newVolume;
   #if USE_LCD_TFT != 0
   addAEConsoleText(pActionEvent,"Set Volume");
   if ( strcmp(pActionEvent->eventSource,SOURCE_EVENT_LCD) != 0 )
		slideVolumeSet(volume);
   #endif
   return MSG_OK;
}

static int32_t volumeDown(ActionEvent_Typedef 	*pActionEvent){(void)pActionEvent;
   volume -= 5;
   volume = volume <= 5 ? DEFAULT_VOLUME : volume;
#if USE_LCD_TFT != 0
   addAEConsoleText(pActionEvent,"Volume Down");
if ( strcmp(pActionEvent->eventSource,SOURCE_EVENT_LCD) != 0 )
		slideVolumeSet(volume);
#endif
   if (pAudioPlayerDriverITF->pCodecDriverITF->SetVolume(AUDIO_I2C_ADDRESS,volume))
       dbgprintf("Vol Down I2C Failed\r\n");
   return MSG_OK;
}
static int32_t volumeUp(ActionEvent_Typedef 	*pActionEvent){(void)pActionEvent;
   volume += 5;
   volume = volume >100 ? 90 : volume;
   #if USE_LCD_TFT != 0
   addAEConsoleText(pActionEvent,"Volume Up");
   if ( strcmp(pActionEvent->eventSource,SOURCE_EVENT_LCD) != 0 )
		slideVolumeSet(volume);
   #endif
   if (pAudioPlayerDriverITF->pCodecDriverITF->SetVolume(AUDIO_I2C_ADDRESS,volume))
       dbgprintf("Vol Up I2C Failed\r\n");

   return MSG_OK;
}

static int32_t nextTrack(ActionEvent_Typedef 	*pActionEvent){(void)pActionEvent;
   if ( IS_PAUSING(pAudioPlayerDriverITF) && pAudioPlayerDriverITF->playerThdRef != NULL ){
       togglePausePlay(pActionEvent);
       chThdSleepMilliseconds(SLEEP_MS_AFTER_TOGGLING_PAUSE);
   }
   if ( IS_PLAYING(pAudioPlayerDriverITF) ){
	   if ( IS_PLAYING_AND_MUTE(pAudioPlayerDriverITF) )
		   toggleMute(pActionEvent);
	   pAudioPlayerDriverITF->actionEventEnum = NEXT_TRACK;
	   dbgprintf("Next track\r\n");
   }
   else
	   dbgprintf("Ignoring Next track. Current State:%d\r\n",pAudioPlayerDriverITF->actionEventEnum);

   #if USE_LCD_TFT != 0
   addAEConsoleText(pActionEvent,"Next Track");
   #endif

   return MSG_OK;
}
static int32_t prevTrack(ActionEvent_Typedef 	*pActionEvent){(void)pActionEvent;
   if ( IS_PAUSING(pAudioPlayerDriverITF) && pAudioPlayerDriverITF->playerThdRef != NULL ){
       togglePausePlay(pActionEvent);
       chThdSleepMilliseconds(SLEEP_MS_AFTER_TOGGLING_PAUSE);
   }
   if ( IS_PLAYING(pAudioPlayerDriverITF) ){
	   if ( IS_PLAYING_AND_MUTE(pAudioPlayerDriverITF) )
		   toggleMute(pActionEvent);
	   pAudioPlayerDriverITF->actionEventEnum = PREV_TRACK;
	   dbgprintf("PREV track\r\n");
   }
   else
	   dbgprintf("Ignoring Next track. Current State:%d\r\n",pAudioPlayerDriverITF->actionEventEnum);
	#if USE_LCD_TFT != 0
	addAEConsoleText(pActionEvent,"Prev Track");
	#endif
   return MSG_OK;
}

int32_t repositionToTrack(ActionEvent_Typedef 	*pActionEvent){(void)pActionEvent;
   //if dirty, cleanse 1st
   if ( pActionEvent->u.pData != NULL && pActionEvent->u.pData[0] != 0){
	    if ( pActionEvent->u.pData[0] == '[' && pActionEvent->u.pData[1] == '"'){
	    	removeChar(pActionEvent->u.pData,'"' );
	    	removeChar(pActionEvent->u.pData,'[' );
	    	removeChar(pActionEvent->u.pData,']' );
	    }

	   AudioFileInfo_TypeDef   *pAudioFileInfo = (AudioFileInfo_TypeDef*)emlist_iterate_over(pAudioPlayerDriverITF->audioFileList,
			                                                                                 findAudioFileInfoByTrackKey,(void*)pActionEvent->u.pData);
	   if ( pAudioFileInfo != NULL){
		   if ( IS_PAUSING(pAudioPlayerDriverITF) && pAudioPlayerDriverITF->playerThdRef != NULL ){
		       togglePausePlay(pActionEvent);
		       chThdSleepMilliseconds(SLEEP_MS_AFTER_TOGGLING_PAUSE);
		   }
		   if ( IS_PLAYING(pAudioPlayerDriverITF) ){
			   pAudioPlayerDriverITF->actionEventEnum = REPOSITION_ITERATOR;
			   strlcpy(pAudioPlayerDriverITF->repositionToTrack,pActionEvent->u.pData,TRACK_KEY_MAX_SIZE);
			   dbgprintf("===>Trying to reposition to track with track ID:%s\r\n",pAudioPlayerDriverITF->repositionToTrack);
		   }
	   }
	   else
		   dbgprintf("===>Unable to fine track with track key:%s\r\n",(char*)pActionEvent->u.pData);
   }
   else{
	   dbgprintf("No track key was avail to reposition to\r\n");
   }

   return MSG_OK;
}

int32_t setRGBLED(ActionEvent_Typedef 	*pActionEvent){(void)pActionEvent;
#if S4E_USE_RGB != 0
   if ( pActionEvent->u.pData != NULL){
       char *strRed 	= strtok(pActionEvent->u.pData,"|");
       char *strGreen 	= strtok(NULL,"|");
       char *strBlue 	= strtok(NULL,"|");

	   uint8_t  red 	= strRed!=NULL?  atoi(strRed):  0;
	   uint8_t  green	= strGreen!=NULL?atoi(strGreen):0;
	   uint8_t  blue 	= strBlue!=NULL? atoi(strBlue): 0;
	   setColorRGB(0, red, green, blue);
   }
   else{
	   dbgprintf("No data to set RGB LED\r\n");
	   return MSG_RESET;
   }
#else
   dbgprintf("RGB LED compile option not enabled\r\n");
#endif
   return MSG_OK;
}
#if S4E_USE_PPM_FRAME_DECODER != 0
void onChannelPPMValueChange (uint8_t ch, uint8_t currentValue, uint8_t newValue){
	ButtonStats_Typedef buttonStatus;

	dbgprintf("OnChangeChannelValue: %d\t%d\t%d\r\n", ch, currentValue, newValue);
	switch(ch){
		case RC_CH3:{
			uint8_t vol =100 *(newValue-RC_MIN_VALUE) /(RC_MAX_VALUE - RC_MIN_VALUE);
			triggerActionEvent(SET_VOLUME_AE_NAME,NULL,vol,SOURCE_EVENT_RC);
		}
		break;
		case RC_SWA:
			buttonStatus = getRCButtonStatus(newValue);
			if ( buttonStatus != BUTTON_STATE_UNKNOWN)
				triggerActionEvent(TOGGLE_MUTE_AE_NAME,NULL,buttonStatus,SOURCE_EVENT_RC);
			break;
		case RC_SWB:
			buttonStatus = getRCButtonStatus(newValue);
			if ( buttonStatus != BUTTON_STATE_UNKNOWN)
				triggerActionEvent(TOGGLE_PAUSE_AE_NAME,NULL,buttonStatus,SOURCE_EVENT_RC);
			break;
	}

}
#endif

static int32_t loadSysProperties(ActionEvent_Typedef 	*pActionEvent){(void)pActionEvent;
#if S4E_USE_WIFI_MODULE_THD != 0
	sendGetPropertiesMsgToWifiModule(PROPERTIES_HOST,PROPERTIES_URL,PROPERTIES_FP);
#endif
   return MSG_OK;
}
static int32_t updateWifiHtml(ActionEvent_Typedef 	*pActionEvent){(void)pActionEvent;
#if S4E_USE_WIFI_MODULE_THD != 0
   chThdSleepMilliseconds(5000);
   initWifiModuleServer();
#endif
   return MSG_OK;
}

static int32_t newHTMLLoaded(ActionEvent_Typedef 	*pActionEvent){(void)pActionEvent;
    dbgprintf("newHTMLLoaded\r\n");
#if S4E_USE_RGB != 0
    setColorRGB(0, 0, 255, 0);
#endif
    //A good example by trigger another AE from inside another, but it will be executed asynchronously
    triggerActionEvent(SET_RGB_LED_AE_NAME,"0|0|0",0,NEW_HTML_LOADED_AE_NAME);

   return MSG_OK;
}
static int32_t performanceInfo(ActionEvent_Typedef 	*pActionEvent){(void)pActionEvent;
   getHeapUsageInfo();

   return MSG_OK;
}
static int32_t setUnixtime(ActionEvent_Typedef 	*pActionEvent){(void)pActionEvent;
	#if HAL_USE_RTC != 0
	if ( pActionEvent->u.pData != NULL){
		rtcSetTimeUnixSecFromString(pActionEvent->u.pData);
		dbgprintf("New RTC Date:%s", rtcGetTimeAsString());
	}
	#endif

   return MSG_OK;
}
void updateUI(AudioPlayerDriverITF_Typedef *pAudioPlayerDriver);
static int32_t goToSleep(ActionEvent_Typedef 	*pActionEvent){(void)pActionEvent;
	#if HAL_USE_RTC != 0
	pAudioPlayerDriverITF->state = AP_SLEEPING;
	updateUI(pAudioPlayerDriverITF);
	chThdSleepMilliseconds(10);

	rtcGoToSleep();
	#endif

   return MSG_OK;
}
static ActionEvent_Typedef actionEventToggleMute 	 	= {.name=TOGGLE_MUTE_AE_NAME,  			.eventSource="Center",      	.action=toggleMute, 	   .view=toggleMuteView,		.dataType = INT_DTYPE};
static ActionEvent_Typedef actionEventNextTrack  	 	= {.name=NEXT_TRACK_AE_NAME,			.eventSource="Up",          	.action=nextTrack,         .view=toggleMuteView,		.dataType = INT_DTYPE};
static ActionEvent_Typedef actionEventPrevTrack  	 	= {.name=PREV_TRACK_AE_NAME,			.eventSource=SOURCE_EVENT_MQTT,	.action=prevTrack,         .view=toggleMuteView,		.dataType = INT_DTYPE};
static ActionEvent_Typedef actionEventRepositionToTrack	= {.name=REPOSITION_TO_TRACK_AE_NAME,	.eventSource=SOURCE_EVENT_MQTT, .action=repositionToTrack, .view=NULL,					.dataType = CHAR_DTYPE};
static ActionEvent_Typedef actionEventTogglePausePlay	= {.name=TOGGLE_PAUSE_AE_NAME,			.eventSource="Down",        	.action=togglePausePlay,							    .dataType = INT_DTYPE};
static ActionEvent_Typedef actionEventVolumeDown   	 	= {.name=VOLUME_DOWN_AE_NAME,			.eventSource="Left",        	.action=volumeDown, 	   .view=volumeView,			.dataType = INT_DTYPE};
static ActionEvent_Typedef actionEventVolumeUp       	= {.name=VOLUME_UP_AE_NAME,				.eventSource="Right",       	.action=volumeUp, 		   .view=volumeView,			.dataType = INT_DTYPE};
static ActionEvent_Typedef actionEventSetVolume      	= {.name=SET_VOLUME_AE_NAME,			.eventSource="setVolume",   	.action=setVolume, 		   .view=volumeView,			.dataType = INT_DTYPE};
static ActionEvent_Typedef actionEventUpdateWifiHTML 	= {.name=UPDATE_WIFI_HTML_AE_NAME,		.eventSource="updateWifiHtml",  .action=updateWifiHtml,    .view=updateWifiHtmlView,	.dataType = INT_DTYPE};
static ActionEvent_Typedef actionEventLoadSysProperties = {.name=LOAD_SYS_PROPERTIES_AE_NAME,	.eventSource="WiFi",            .action=loadSysProperties, .view=loadSysPropertiesView,	.dataType = INT_DTYPE};
static ActionEvent_Typedef actionEventSetRGBLED         = {.name=SET_RGB_LED_AE_NAME,			.eventSource=SOURCE_EVENT_MQTT, .action=setRGBLED, 			.view=setRGBLEDView,		.dataType = INT_DTYPE};
static ActionEvent_Typedef actionEventNewHTMLLoaded	 	= {.name=NEW_HTML_LOADED_AE_NAME,  		.eventSource=SOURCE_EVENT_MQTT, .action=newHTMLLoaded, 	    .view=NULL,				    .dataType = INT_DTYPE};
static ActionEvent_Typedef actionEventPerformanceInfo 	= {.name=PERFORMANCE_INFO_AE_NAME,  	.eventSource=SOURCE_EVENT_MQTT, .action=performanceInfo, 	.view=NULL,				    .dataType = INT_DTYPE};
static ActionEvent_Typedef actionEventSetUnixtime      	= {.name=SET_UNIX_TIME_AE_NAME,			.eventSource="WiFi",   		    .action=setUnixtime, 		.view=NULL,			        .dataType = CHAR_DTYPE};
static ActionEvent_Typedef actionEventGoToSleep      	= {.name=GO_TO_SLEEP_AE_NAME,			.eventSource="WiFi",   		    .action=goToSleep, 		    .view=NULL,			        .dataType = INT_DTYPE};

ActionEvent_Typedef *gActionEvents[MAX_ACTION_EVENTS] = {&actionEventToggleMute,
		                                                 &actionEventNextTrack,
														 &actionEventPrevTrack,
														 &actionEventRepositionToTrack,
														 &actionEventTogglePausePlay,
														 &actionEventVolumeDown,
														 &actionEventVolumeUp,
                                                         &actionEventSetVolume,
                                                         &actionEventUpdateWifiHTML,
                                                         &actionEventLoadSysProperties,
														 &actionEventSetRGBLED,
														 &actionEventNewHTMLLoaded,
														 &actionEventPerformanceInfo,
														 &actionEventSetUnixtime,
														 &actionEventGoToSleep};
