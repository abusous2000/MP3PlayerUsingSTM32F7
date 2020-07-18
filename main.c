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
#include "uGFXThread.h"
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

  while (true) {
	    chThdSleepMilliseconds(2500);
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
