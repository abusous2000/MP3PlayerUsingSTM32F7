#include "Strust4EmbeddedConf.h"
#include "ch.h"
#include "hal.h"
#include "dbgtrace.h"
#include "Strust4Embedded.h"
#include "ssd1306.h"
#include "ButtonLEDs.h"
#include "PotReader.h"
#include "MQTTClient.h"
#include "audio.h"
#include "AudioPlayerThd.h"
#include "PPMFrameDecoder.h"
#include "EByteLora.h"
#include "gui.h"
#include "uGFXThread.h"
#include "IRReceiver.h"
#include "ccportab.h"

#ifdef USE_USBCFG
#include "usbcfg.h"
static void initUSBCFG(void);
#endif

AudioPlayerDriverITF_Typedef	*pAudioPlayerDriverITF;
thread_t 			  		    *mainThd;

static void initDrivers(void);
#if S4E_USE_WIFI_MODULE_THD != 0
/* WiFi Serial configuration. */
static const SerialConfig wifiSerialvfg = {
  WIFI_SERIALBAUD_RATE,
  0,
  USART_CR2_STOP1_BITS,
  0
};
#endif

#if HAL_USE_SERIAL != 0

/* VCP Serial configuration. */
static const SerialConfig myserialcfg = {
  115200,
  0,
  USART_CR2_STOP1_BITS,
  0
};
#endif

#if DEBUG_TRACE_PRINT != 0
BaseSequentialStream *GlobalDebugChannel = (BaseSequentialStream *)&PORTAB_SD;
#endif

#define LCD_COLOR_DARKYELLOW    ((uint32_t) 0xFF808000)
#define LCD_COLOR_RED           ((uint32_t) 0xFFFF0000)
#define LCD_COLOR_YELLOW        ((uint32_t) 0xFFFFFF00)
#define LCD_COLOR_WHITE         ((uint32_t) 0xFFFFFFFF)
void initMain(void);
#ifndef __cplusplus
void initMain(void){
 halInit();
 chSysInit();
}
#endif

int main(void) {
  initMain();
#if HAL_USE_SERIAL != 0
	#ifdef USE_USBCFG
	  initUSBCFG();
	#else
	  sdStart(&PORTAB_SD, &myserialcfg);
	#endif
#endif
#if S4E_USE_ETHERNET != 0
  initMQTTClient();
#endif
  initDrivers();

  initActonEventThd();
  initButtonsLEDs();
#if S4E_USE_WIFI_MODULE_THD != 0
  sdStart(&WIFI_SD, &wifiSerialvfg);
  initWifiCommunicationThd();
#endif
#if defined(LINE_LED_RED)
  initBlinkerThd(pRedLedPAL);
#else
  initBlinkerThd(pGreenLedPAL);
#endif
  initAudioPlayerThd();
#if USE_LCD_TFT
  inituGFXThd();
#endif
#if S4E_USE_EBYTE_LORA != 0
  initEByteLoraThread();
#endif

#if PPM_FRAME_DECODER != 0
 initPPMFrameDecoder();
#endif

#if S4E_USE_IR_RECEIVER != 0
  initIRReeceiver();
#endif

  while (true) {
	  chThdSleepMilliseconds(1500);
	  #if S4E_USE_MQTT != 0
	  if ( !isDefaultMQTTBrokerConnected() )
		  reconnectDefaultMQTTBroker();
	  #endif
  }
}
static NameValuePairStaticTypeDef readFilesFromFolder=  {.key=READ_FILES_FROM_FOLDER,	.value="/music"};
static void initDrivers(void){
  pAudioPlayerDriverITF     = getAudioPlayerDriver();
  mainThd					= chThdGetSelfX();

  initStruts4EmbeddedFramework();
  putSysProperty(&readFilesFromFolder);
#if S4E_USE_POT != 0
  initPotReader();
#endif
#if S4E_USE_SSD1306_LCD != 0
  ssd130InitAndConfig("MP3Player w/ STM32F7");
#endif

  return;
}
static bool pauseMsgSent = false;
void updateUI(AudioPlayerDriverITF_Typedef *pAudioPlayerDriver){
  int   forMinutes      = pAudioPlayerDriver->pAudioFileInfo->secondsRemaining / 60;
  int   remainingSec    = pAudioPlayerDriver->pAudioFileInfo->secondsRemaining % 60;
  char  payload[120]	= {0};

  if ( pAudioPlayerDriver->pAudioFileInfo != NULL &&
	  (!IS_PAUSING(pAudioPlayerDriver) || (IS_PAUSING(pAudioPlayerDriver) && !pauseMsgSent)) ){
	  updateLCD();
	  #if S4E_USE_WIFI_MODULE_THD != 0
	  chsnprintf(payload,sizeof(payload),"%s{\"track\":\"%s\",\"status\":\"%s\",\"vol\":%d,\"seconds_remaining\":%d,\"timeRemaining\":\"%d:%d\"}%s",
											  PAYLOAD_PREFIX,
											  pAudioPlayerDriver->pAudioFileInfo->filename,
											  getAPStateName(pAudioPlayerDriver->state),
											  getCurrentVolume(),
											  pAudioPlayerDriver->pAudioFileInfo->secondsRemaining,
											  forMinutes,
											  remainingSec,
											  PAYLOAD_SUFFIX);

	  sendWifiModuleMsg(payload);
	#endif
	#if S4E_USE_MQTT != 0
	  pauseMsgSent = IS_PAUSING(pAudioPlayerDriver)? true: false;
	  chsnprintf(payload,sizeof(payload),"{\"track\":\"%s\",\"status\":\"%s\",\"vol\":%d,\"seconds_remaining\":%d,\"timeRemaining\":\"%d:%d\"}",
	                                          pAudioPlayerDriver->pAudioFileInfo->filename,
	                                          getAPStateName(pAudioPlayerDriver->state),
	                                          getCurrentVolume(),
											  pAudioPlayerDriver->pAudioFileInfo->secondsRemaining,
	                                          forMinutes,
	                                          remainingSec);
	  sendToDefaultMQTTQueue(payload);
    #endif
  }

  return;
}

//This is a weak/virtual method that overrides the default in the BlinkerThd.c file, see for details
void periodicSysTrigger(uint32_t i){
#if S4E_USE_POT != 0
  checkOnPotVolumeChange();
#endif
  if ( i % 2 != 0 )
	  return;

  if ( i> 0 && i % 2 == 0 )
    updateUI(pAudioPlayerDriverITF);
  if (IS_PLAYING(pAudioPlayerDriverITF))
      --pAudioPlayerDriverITF->pAudioFileInfo->secondsRemaining;

  return;
}
#if HAL_USE_RTC != 0
void onRTCSleep(void){
	dbgprintf("Going sleep in 10ms...:goingToSleepCnt:%d\r\n",getGoingToSleepCnt());
	audioPlayerPrepareGoingToSleep();
	#if USE_LCD_TFT != 0
	turnOffLCD();
	#endif
    chThdSleepMilliseconds(10);
}
#endif
#ifdef USE_USBCFG
static void initUSBCFG(void){
	/* Configuring PG14 as AF8 assigning it to USART6_TX. */
//	palSetPadMode(GPIOG, 14, PAL_MODE_ALTERNATE(8) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_PUSHPULL);
//	/* Configuring PG9 as AF8 assigning it to USART6_RX. */
//	palSetPadMode(GPIOG, 9, PAL_MODE_ALTERNATE(8) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_PUSHPULL);
	/*
	* Initializes a serial-over-USB CDC driver.
	*/
	sduObjectInit(&PORTAB_SDU);
	sduStart(&PORTAB_SDU, &serusbcfg);
	chThdSleepMilliseconds(300);

	usbDisconnectBus(serusbcfg.usbp);
	chThdSleepMilliseconds(1500);
	usbStart(serusbcfg.usbp, &usbcfg);
	usbConnectBus(serusbcfg.usbp);

	sdStart(&PORTAB_SD_VCP, &myserialcfg);
}
#endif

//overriding the default handling of IR Events
#if S4E_USE_IR_RECEIVER != 0
static ActionEvent_Typedef 		*pSetVolumeUpAE			= NULL;
static ActionEvent_Typedef 		*pSetVolumeDownAE		= NULL;
static ActionEvent_Typedef 		*pTogglePauseAE			= NULL;
static ActionEvent_Typedef 		*pToggleMuteAE			= NULL;
static ActionEvent_Typedef 		*pToggleSleepAE			= NULL;
static ActionEvent_Typedef 		*pNextTrackAE			= NULL;
static ActionEvent_Typedef 		*pPrevTrackAE			= NULL;
static ActionEvent_Typedef 		*pRepositionToTrackAE	= NULL;

static bool irInit = false;
void handlIREvent(IR_CMD_t command, bool repeatFlag){
	if ( !irInit){
	  pSetVolumeUpAE   	    = getActionEvent(VOLUME_UP_AE_NAME);
	  pSetVolumeDownAE   	= getActionEvent(VOLUME_DOWN_AE_NAME);
	  pTogglePauseAE 		= getActionEvent(TOGGLE_PAUSE_AE_NAME);
	  pToggleMuteAE  		= getActionEvent(TOGGLE_MUTE_AE_NAME);
	  pToggleSleepAE  	    = getActionEvent(GO_TO_SLEEP_AE_NAME);
	  pNextTrackAE   		= getActionEvent(NEXT_TRACK_AE_NAME);
	  pPrevTrackAE   		= getActionEvent(PREV_TRACK_AE_NAME);
	  irInit = true;
	}

	switch(command){
	    case BTN_POWER:
	       dbgprintf("PWR");
	       if(repeatFlag)
	         dbgprintf(" RPT");
	       else
	    	   triggerActionEvent(pToggleSleepAE->name, NULL, 0, SOURCE_EVENT_IR);
	       break;
	    case BTN_MODE:
	      dbgprintf("MODE");
	      if(repeatFlag)
	        dbgprintf(" RPT");
	      break;
	    case BTN_MUTE:
	      dbgprintf("MUTE");
	      if(repeatFlag)
	        dbgprintf(" RPT");
	      else
	    	triggerActionEvent(pToggleMuteAE->name, NULL, 0, SOURCE_EVENT_IR);
	      break;
	    case BTN_PREV:
	      dbgprintf("PREV");
	      if(repeatFlag)
	        dbgprintf(" RPT");
	      else
	    	 triggerActionEvent(pPrevTrackAE->name, NULL, 0, SOURCE_EVENT_IR);
	      break;
	    case BTN_NEXT:
	      dbgprintf("NEXT");
	      if(repeatFlag)
	        dbgprintf(" RPT");
	      else
	    	 triggerActionEvent(pNextTrackAE->name, NULL, 0, SOURCE_EVENT_IR);
	      break;
	    case BTN_PLAY_PAUSE:
	      dbgprintf("PLAY/PAUSE");
	      if(repeatFlag)
	        dbgprintf(" RPT");
	      else
			triggerActionEvent(pTogglePauseAE->name, NULL, 0, SOURCE_EVENT_IR);
	      break;
	    case BTN_VOL_DOWN:
	      dbgprintf("VOL-");
	      if(repeatFlag)
	        dbgprintf(" RPT");
	      else
	    	 triggerActionEvent(pSetVolumeDownAE->name, NULL, 0, SOURCE_EVENT_IR);
	      break;
	    case BTN_VOL_UP:
	      dbgprintf("VOL+");
	      if(repeatFlag)
	        dbgprintf(" RPT");
	      else
	    	 triggerActionEvent(pSetVolumeUpAE->name, NULL, 0, SOURCE_EVENT_IR);
	      break;
	    case BTN_EQ:
	      dbgprintf("EQ");
	      if(repeatFlag)
	        dbgprintf(" RPT");
	      break;
	    case BTN_0:
	      dbgprintf("0");
	      if(repeatFlag)
	        dbgprintf(" RPT");
	      break;
	    case BTN_LOOP:
	      dbgprintf("100+");
	      if(repeatFlag)
	        dbgprintf(" RPT");
	      break;
	    case BTN_USD:
	      dbgprintf("200+");
	      if(repeatFlag)
	        dbgprintf(" RPT");
	      break;
	    case BTN_1:
	      dbgprintf("1");
	      if(repeatFlag)
	        dbgprintf(" RPT");
	      break;
	    case BTN_2:
	      dbgprintf("2");
	      if(repeatFlag)
	        dbgprintf(" RPT");
	      break;
	    case BTN_3:
	      dbgprintf("3");
	      if(repeatFlag)
	        dbgprintf(" RPT");
	      break;
	    case BTN_4:
	      dbgprintf("4");
	      if(repeatFlag)
	        dbgprintf(" RPT");
	      break;
	    case BTN_5:
	      dbgprintf("5");
	      if(repeatFlag)
	        dbgprintf(" RPT");
	      break;
	    case BTN_6:
	      dbgprintf("6");
	      if(repeatFlag)
	        dbgprintf(" RPT");
	      break;
	    case BTN_7:
	      dbgprintf("7");
	      if(repeatFlag)
	        dbgprintf(" RPT");
	      break;
	    case BTN_8:
	      dbgprintf("8");
	      if(repeatFlag)
	        dbgprintf(" RPT");
	      break;
	    case BTN_9:
	      dbgprintf("9");
	      if(repeatFlag)
	        dbgprintf(" RPT");
	      break;
	    default:
	      dbgprintf("Unknown");
	      break;
	}
	dbgprintf("\n\r");
}
#endif
