/*
 * audio.c
 *
 *  Created on: Aug 1, 2019
 *      Author: abusous2000
 */
#include "audio.h"
#include "ActionEvents.h"
#include "SDCard.h"
#include "hal_sai.h"
#include "CPal.h"
#include "MQTTClient.h"
#include "ccportab.h"

extern thread_t 					*libMadThd;
extern AudioPlayerDriverITF_Typedef	*pAudioPlayerDriverITF;
extern CPalTypedef 		  			*pGreenLedPAL;

static enum mad_flow onHeader(void *data);
static enum mad_flow onPostReadActionEvents(void *data);
static enum mad_flow onPreReadActionEvents(void *data);
static enum mad_flow onPlayerWrite(void *data);
static enum mad_flow processReadError(void *data,  struct mad_stream *stream);

static AudioPlayerDriverITF_Typedef  audioPlayerDriverImpl = {	.onHeaderCallback=onHeader,
																.onErrorCallback=processReadError,
																.onPreReadCallback=onPreReadActionEvents,
																.onPostReadCallback=onPostReadActionEvents,
																.onWriteCallback=onPlayerWrite,
                                                                .audioFileList = NULL};

static char const APStates[MAX_ACTION_EVENTS]        = { 'S', 'P', 'K', 'M', 'U'};
static char const *APStatesString[MAX_ACTION_EVENTS] = { "Stopped", "Playing", "Skipping","Mute","Pausing"};
uint8_t getAPStateShortName(uint8_t ndx){
  return APStates[ndx];
}
const char *getAPStateName(uint8_t ndx){
  return APStatesString[ndx];
}
//called by ISR in sai_lld_serve_tx_interrupt() @ hal_sai.c
//Its job is signle the mad thrd when DMA is either full or half...see output() @ mad.c for details
static void saicallback(SAIDriver *saip, size_t offset, size_t n) {
  (void)saip;
  (void)n;
  chSysLockFromISR();
  //Complete
  if ( offset != 0 )
	 chEvtSignalI(libMadThd, EVENT_MASK(COMPLETE_EVENT));
  else//half
     chEvtSignalI(libMadThd, EVENT_MASK(HALFWAY_EVENT));
  chSysUnlockFromISR();
}
//This variable filled by libmad decoder, I believe it contains the PCM values
extern uint16_t wave_buf[];
static SAIConfig_TypeDef saiConfig = { .end_cb=saicallback,
							    		.size=MP3_BUFF_SIZE,
										.tx_buffer=wave_buf,
										.CR1=SAI_CCR1,
										.CR2=0x1,
										.SR=0x0,
										.SLOTR=0x30100,
										.FRCR=0x50f1f,
										.GCR=0};

static SAIConfig_TypeDef saiConfig2 = { 	.end_cb=saicallback,
							    		.size=MP3_BUFF_SIZE,
										.tx_buffer=wave_buf,
										.CR1=SAI_CCR1,
										.CR2=0x1,
										.SR=0x8,
										.SLOTR=0xf0300,
										.FRCR=0x51f3f,
										.GCR=0};
static I2CConfig i2cfg = {
//	STM32_TIMINGR_PRESC(15U) |
//	STM32_TIMINGR_SCLDEL(4) | STM32_TIMINGR_SDADEL(2U) |
//	STM32_TIMINGR_SCLH(15U)  | STM32_TIMINGR_SCLL(21U),//0xf0420f15
	.timingr=0x40912732,
	.cr1=I2C_CR1_ANFOFF,// | I2C_CR1_NOSTRETCH,//// Disable NOSTRETCH & Analog filter
	.cr2=0//I2C_CR2_AUTOEND
};

AudioPlayerDriverITF_Typedef *getAudioPlayerDriver(void){
	audioPlayerDriverImpl.pSDCardDriverITF = getSDCardDriver();
	audioPlayerDriverImpl.pCodecDriverITF  = getCodecDriver();
	return &audioPlayerDriverImpl;
}
static void codecCtrlInterfaceInit(void){
    //TODO: Call codec to set freq. coming from header() @ mad.c
	i2cStart(&I2CD_CODEC, &i2cfg);
}
static uint8_t 		isAudioInit = 0;

void AUDIO_IO_Init(void){
  if ( isAudioInit == 0 ){
#ifdef STM32L476xx
		//  /* Audio reset pin configuration -------------------------------------------------*/
	  palSetLineMode(LINE_AUDIO_RST, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_MEDIUM);
      /* CODEC SCL and SDA pins configuration -------------------------------------*/
      palSetPadMode(GPIOB, 6,   PAL_MODE_ALTERNATE(4) | PAL_STM32_OSPEED_FAST | PAL_STM32_OTYPE_OPENDRAIN);
      palSetPadMode(GPIOB, 7,   PAL_MODE_ALTERNATE(4) | PAL_STM32_OSPEED_FAST | PAL_STM32_OTYPE_OPENDRAIN);
      /* CODE SAI pins configuration: WS, SCK and SD pins -----------------------------*/
      RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN;
      palSetPadMode(GPIOE, 6, PAL_STM32_OTYPE_PUSHPULL | PAL_MODE_ALTERNATE(13) | PAL_STM32_OSPEED_HIGH);
      palSetPadMode(GPIOE, 5, PAL_STM32_OTYPE_PUSHPULL | PAL_MODE_ALTERNATE(13) | PAL_STM32_OSPEED_HIGH);
      palSetPadMode(GPIOE, 4, PAL_STM32_OTYPE_PUSHPULL | PAL_MODE_ALTERNATE(13) | PAL_STM32_OSPEED_HIGH);
      palSetPadMode(GPIOE, 2, PAL_STM32_OTYPE_PUSHPULL | PAL_MODE_ALTERNATE(13) | PAL_STM32_OSPEED_HIGH);
#else
	  /* I2CD_CODEC-->CODEC SCL and SDA pins configuration -------------------------------------*/
      palSetPadMode(SAI_I2C_SCL_PORT, SAI_I2C_SCL_PIN,    PAL_MODE_ALTERNATE(SAI_I2C_SCL_AF) | PAL_STM32_OSPEED_HIGHEST|  PAL_STM32_OTYPE_OPENDRAIN );
      palSetPadMode(SAI_I2C_SDA_PORT, SAI_I2C_SDA_PIN,    PAL_MODE_ALTERNATE(SAI_I2C_SDA_AF) | PAL_STM32_OSPEED_HIGHEST|  PAL_STM32_OTYPE_OPENDRAIN );
	  /* CODE SAI pins configuration: WS, SCK and SD pins ... See Schematic Page 41 & 42 STM32F759i User Manual.-----------------------------*/
      palSetPadMode(SAI_MCLKA_PORT, SAI_MCLKA_PIN, PAL_STM32_OTYPE_PUSHPULL | PAL_MODE_ALTERNATE(SAI_AF) | PAL_STM32_OSPEED_HIGHEST);
	  palSetPadMode(SAI_FSA_PORT,   SAI_FSA_PIN,   PAL_STM32_OTYPE_PUSHPULL | PAL_MODE_ALTERNATE(SAI_AF) | PAL_STM32_OSPEED_HIGHEST);
	  palSetPadMode(SAI_SCKA_PORT,  SAI_SCKA_PIN,  PAL_STM32_OTYPE_PUSHPULL | PAL_MODE_ALTERNATE(SAI_AF) | PAL_STM32_OSPEED_HIGHEST);
	  palSetPadMode(SAI_SDA_PORT,   SAI_SDA_PIN,   PAL_STM32_OTYPE_PUSHPULL | PAL_MODE_ALTERNATE(SAI_AF) | PAL_STM32_OSPEED_HIGHEST);
#endif
	  codecCtrlInterfaceInit();
	  sai_lld_init();
	  saiStart(&SAID,&saiConfig);
	  isAudioInit = 1;
  }
  return;
}

void AUDIO_IO_DeInit(void){
	isAudioInit = 0;
	i2cStop(&I2CD_CODEC);
	saiStop(&SAID);
}
void AUDIO_IO_ReInit(uint32_t AudioFreq){(void)AudioFreq;
 saiStart(&SAID, &saiConfig);
}
void AUDIO_IO_Exchange(void){
  saiStartExchange(&SAID);
}

//returns none null if found, else return null to continue looking for a match
LinkedListElement* findAudioFileInfo(LinkedListElement* pItem, void *arg){
	char                    *filenameToMatchwith = (char*)arg;
	AudioFileInfo_TypeDef   *pAudioFileInfo = (AudioFileInfo_TypeDef*)pItem->value;
	bool                    match           = strcmp(pAudioFileInfo->filename,filenameToMatchwith)==0;
	if ( match )
		dbgprintf("Matching for: \"%s\"--->%d\r\n", filenameToMatchwith,match);
	return match?pItem:NULL;
}

FRESULT playAudioFile(AudioFileInfo_TypeDef* pAudioFileInfo){
  CodecDriverITF_Typedef        *pCodecDriverITF = pAudioPlayerDriverITF->pCodecDriverITF;
  pAudioPlayerDriverITF->pAudioFileInfo = pAudioFileInfo;
  saiStart(&SAID, &saiConfig);
  if ( !pAudioPlayerDriverITF->mute )
      pCodecDriverITF->SetMute(AUDIO_I2C_ADDRESS,AUDIO_MUTE_OFF);
  //lcdInitAndConfig();
  if ( decodeMP3File(pAudioPlayerDriverITF) !=0 ){
      dbgprintf("failed in libmad; unknown error\r\n");
      return FR_DISK_ERR;//TODO: a hack, plz clean up
  }
  /*
   *
   * *******IMPORTANT*****
   * If saiStop() not called here and called from somewhere else, the thread will hang
   */
  saiStop(&SAID);
  if ( !pAudioPlayerDriverITF->mute )
      pCodecDriverITF->SetMute(AUDIO_I2C_ADDRESS,AUDIO_MUTE_ON);

  return FR_OK;
}
uint32_t trackID = 0;
//This method override the weak version in SDCard.c
FRESULT processFile(char *fpath){
  size_t	   unLen = strlen(fpath);

  chDbgCheck(audioPlayerDriverImpl.audioFileList!=NULL);
  dbgprintf("#########processFile: FILE \"%s\"\r\n", fpath);
  if(unLen > 3){
      if(strcasecmp(fpath + unLen - 4, ".mp3") == 0){
    	  AudioFileInfo_TypeDef   *pAudioFileInfo = (AudioFileInfo_TypeDef*)emlist_iterate_over(audioPlayerDriverImpl.audioFileList,findAudioFileInfo,(void*)fpath);

    	  //make sure it is not in the list prior to adding it
    	  if ( pAudioFileInfo == NULL){
    		  pAudioFileInfo = chHeapAlloc(NULL, sizeof(AudioFileInfo_TypeDef) );
    		  chDbgCheck(pAudioFileInfo!=NULL);
    		  pAudioFileInfo->trackID = ++trackID;
    		  pAudioFileInfo->audioFileType = AUDIO_FILE_TYPE_MP3;
    		  strlcpy(pAudioFileInfo->filename,fpath,FILENAME_MAX_SIZE);
    		  chsnprintf(pAudioFileInfo->trackKey,sizeof(pAudioFileInfo->trackKey),"TRACK_%d",pAudioFileInfo->trackID);
    		  emlist_insert(audioPlayerDriverImpl.audioFileList,pAudioFileInfo);
    		  dbgprintf("=====>Add file to Audio List: \"%s\"\r\n", fpath);
    	  }
      }
      else
      if(strcasecmp(fpath + unLen - 4, ".wav") == 0){
    	  //TODO:Not yet implemented: WAV Driver implementation
      }
  }
  return FR_OK;
}
//returns none null if found, else return null to continue looking for a match
LinkedListElement* findAudioFileInfoByTrackKey(LinkedListElement* pItem, void *arg){
	char                    *trackKeyToMatchwith = (char*)arg;
	AudioFileInfo_TypeDef   *pAudioFileInfo = (AudioFileInfo_TypeDef*)pItem->value;
	bool                    match           = strcmp(pAudioFileInfo->trackKey,trackKeyToMatchwith)==0;
	if ( match )
		dbgprintf("Reposition to Track: %s--->%s\r\n", trackKeyToMatchwith, pAudioFileInfo->filename);
	return match?pItem:NULL;
}

#define METADATA_RECORD_MAX_RECORD_LEN 		FILENAME_MAX_SIZE+TRACK_KEY_MAX_SIZE+10
#define PADS 								5
#define CHARS_TO_SHOW 						13
//returns none null if found, else return null to continue looking for a match
LinkedListElement* audioFileInfoToJson(LinkedListElement* pItem, void *arg){
	char                    *_payload 	    = (char*)arg;
   	char 					metadataRecord[METADATA_RECORD_MAX_RECORD_LEN]={0};
	AudioFileInfo_TypeDef   *pAudioFileInfo = (AudioFileInfo_TypeDef*)pItem->value;
	uint16_t                len 			= strlen(_payload);
	bool 					spaceAvail 		= len <  (MAX_MQTT_PAYLOAD - METADATA_RECORD_MAX_RECORD_LEN -PADS);

	if ( !spaceAvail ){
		dbgprintf("MQTT Message larger than max, see audioFileInfoToJson for details\n\r");
		return pItem;
	}
	int8_t  ndx = strlen(pAudioFileInfo->filename) - CHARS_TO_SHOW;
	ndx = ndx >= 0 ? ndx: 0;
   	chsnprintf(metadataRecord,METADATA_RECORD_MAX_RECORD_LEN,"\"%s\":\"%s\"",pAudioFileInfo->filename+ndx,pAudioFileInfo->trackKey);
   	spaceAvail = (len+strlen(metadataRecord)) <  (MAX_MQTT_PAYLOAD - METADATA_RECORD_MAX_RECORD_LEN-PADS);
   	if ( pItem->next != NULL && spaceAvail)
   		strcat(metadataRecord,",");
   	strcat(_payload,metadataRecord);

	return spaceAvail?NULL:pItem;
}
static MqttPublishInfo_Typedef populateDDTopicInfo      = {.topic=MQTT_PUBLISH_POPULATE_DD_TOPIC, .qos = 0, .retain = 0};
void sendJSONTracksDropdown(void){
    if ( isDefaultMQTTBrokerConnected() ){
    	char *_payload =chHeapAlloc(NULL,MAX_MQTT_PAYLOAD);
        trackID = 0;
        strcpy(_payload,"{");
    	emlist_iterate_over(audioPlayerDriverImpl.audioFileList,audioFileInfoToJson,(void*)_payload);
        strcat(_payload,"}");
        sendToTopicMQTTQueue(&populateDDTopicInfo,_payload);
        dbgprintf("----->Sending populate drop down message:%s\n\r",_payload);
        chHeapFree(_payload);
    }

    return;
}

void onDefaultMQTTBrokerConnect(MqttConnection_Typedef *pMqttConnection){(void)pMqttConnection;
	sendJSONTracksDropdown();
}
CC_WEAK void onTrackStartedPlaying(void){

}
static enum mad_flow onHeader(void *data){
  struct mad_header const *header = (struct mad_header *)data;

  //TODO:This is a rough estimate of the duration; therefore this is not exact
  uint16_t duration = (pAudioPlayerDriverITF->pAudioFileInfo->fileSize * 8) / header->bitrate;
  if ( duration > 1000)
	  duration++;
  pAudioPlayerDriverITF->pAudioFileInfo->secondsRemaining 	= duration;
  pAudioPlayerDriverITF->pAudioFileInfo->durationSec        = duration;
  pAudioPlayerDriverITF->pAudioFileInfo->totalSecondsPlayed = 0;
  pAudioPlayerDriverITF->pAudioFileInfo->frame 				= 0;
  dbgprintf("MP3 Header Info: %s\n\r", pAudioPlayerDriverITF->pAudioFileInfo->filename);
  dbgprintf("-->-->Bitrate    : %d\n\r",header->bitrate);
  dbgprintf("-->-->Samplerate : %d\n\r",header->samplerate);
  dbgprintf("-->-->Duration   : %d\n\r",pAudioPlayerDriverITF->pAudioFileInfo->durationSec);
  onTrackStartedPlaying();
  return MAD_FLOW_CONTINUE ;
}

static enum mad_flow onPostReadActionEvents(void *data){(void)data;
   return MAD_FLOW_CONTINUE;
}
static enum mad_flow onPreReadActionEvents(void *data){(void)data;
   pGreenLedPAL->toggle(pGreenLedPAL);
   return MAD_FLOW_CONTINUE;
}
static enum mad_flow onPlayerWrite(void *data){(void)data;
   pGreenLedPAL->toggle(pGreenLedPAL);
   return MAD_FLOW_CONTINUE;
}
static enum mad_flow processReadError(void *data,  struct mad_stream *stream){(void)data;(void)stream;
//    return stream->error == 0x0103 || stream->error == 0x0102|| stream->error == 0x0104? MAD_FLOW_STOP: MAD_FLOW_IGNORE;
    return MAD_FLOW_IGNORE;
}

