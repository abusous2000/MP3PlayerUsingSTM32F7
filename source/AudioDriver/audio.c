/*
 * audio.c
 *
 *  Created on: Aug 1, 2019
 *      Author: abusous2000
 */
#include "Strust4EmbeddedConf.h"
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
                                                                .audioFileList = NULL,
                                                                .errorMsg = NULL};

static char const APStates[MAX_ACTION_EVENTS]        = { 'S', 'P', 'K', 'M', 'U'};
static char const *APStatesString[MAX_ACTION_EVENTS] = { "Stopped", "Playing", "Skipping","Mute","Pausing"};
uint8_t getAPStateShortName(uint8_t ndx){
  return APStates[ndx];
}
const char *getAPStateName(uint8_t ndx){
  return APStatesString[ndx];
}

//This variable filled by libmad decoder, I believe it contains the PCM values
extern uint16_t wave_buf[];

#ifdef STM32F407xx
//called by ISR in sai_lld_serve_tx_interrupt() @ hal_sai.c
//Its job is signle the mad thrd when DMA is either full or half...see output() @ mad.c for details
static void i2scallback(I2SDriver *i2sp){
  chSysLockFromISR();
  //Complete
  if ( i2sIsBufferComplete(i2sp)  )
	 chEvtSignalI(libMadThd, EVENT_MASK(COMPLETE_EVENT));
  else//half
     chEvtSignalI(libMadThd, EVENT_MASK(HALFWAY_EVENT));
  chSysUnlockFromISR();
}
static I2SConfig i2scfg=  {
			  .tx_buffer=wave_buf,
			  .rx_buffer=NULL,
			  .size=MP3_BUFF_SIZE,
			  .end_cb=i2scallback,
			  .i2scfgr=I2S_Mode_MasterTx | I2S_Standard_Phillips | I2S_DataFormat_16b | I2S_CPOL_Low |  SPI_I2SCFGR_I2SE,
			  .i2spr=0x206//I2S_MCLKOutput_Enable
			};

static I2CConfig i2cfg = {
    OPMODE_I2C,
    400000,
    FAST_DUTY_CYCLE_2
    //STD_DUTY_CYCLE
};
#else
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
static SAIConfig_TypeDef saiConfig = { .end_cb=saicallback,
							    		.size=MP3_BUFF_SIZE,
										.tx_buffer=wave_buf,
										.CR1=SAI_CCR1,
										.CR2=0x1,
										.SR=0x0,
										.SLOTR=0x30100,
										.FRCR=0x50f1f,
										.GCR=0};

static I2CConfig i2cfg = {
//	STM32_TIMINGR_PRESC(15U) |
//	STM32_TIMINGR_SCLDEL(4) | STM32_TIMINGR_SDADEL(2U) |
//	STM32_TIMINGR_SCLH(15U)  | STM32_TIMINGR_SCLL(21U),//0xf0420f15
	.timingr=0x40912732,
	.cr1=I2C_CR1_ANFOFF,// | I2C_CR1_NOSTRETCH,//// Disable NOSTRETCH & Analog filter
	.cr2=0//I2C_CR2_AUTOEND
};
#endif

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
	#ifdef STM32F407xx
	  /* Audio reset pin configuration */
	   palSetLineMode(AUDIO_RESET_LINE, AUDIO_RESET_MODE);
	   /* CODEC_I2C SCL and SDA pins configuration */
	   palSetLineMode(AUDIO_SCL_LINE, AUDIO_CODEC_I2C_MODE);
	   palSetLineMode(AUDIO_SDA_LINE, AUDIO_CODEC_I2C_MODE);
	   /* CODEC_I2S pins configuration: WS, SCK and SD pins */
	   palSetLineMode(AUDIO_I2S_SCK_LINE, AUDIO_I2S_PIN_MODE);
	   palSetLineMode(AUDIO_I2S_SD_LINE,  AUDIO_I2S_PIN_MODE);
	   palSetLineMode(AUDIO_I2S_WS_LINE,  AUDIO_I2S_PIN_MODE);
	   palSetLineMode(AUDIO_I2S_MCK_LINE, AUDIO_I2S_PIN_MODE);

	#else
//	  /* I2CD_CODEC-->CODEC SCL and SDA pins configuration -------------------------------------*/
	  palSetLineMode(AUDIO_I2C_SCL_LINE, PAL_MODE_ALTERNATE(SAI_I2C_SCL_AF) | AUDIO_I2C_LINE_MODE);
	  palSetLineMode(AUDIO_I2C_SDA_LINE, PAL_MODE_ALTERNATE(SAI_I2C_SDA_AF) | AUDIO_I2C_LINE_MODE);
	  /* CODE SAI pins configuration: WS, SCK and SD pins ... See Schematic Page 41 & 42 STM32F769i User Manual.-----------------------------*/
	  palSetLineMode(AUDIO_SAI_MCLKA_LINE, AUDIO_SAI_PIN_MODE);
	  palSetLineMode(AUDIO_SAI_FSA_LINE,   AUDIO_SAI_PIN_MODE);
	  palSetLineMode(AUDIO_SAI_SCKA_LINE,  AUDIO_SAI_PIN_MODE);
	  palSetLineMode(AUDIO_SAI_SDA_LINE,   AUDIO_SAI_PIN_MODE);

	#endif


#ifdef STM32F407xx
//	  i2sStart(&I2S_DRIVER, &i2scfg);
	  codecCtrlInterfaceInit();
#else
	  codecCtrlInterfaceInit();
	  sai_lld_init();
	  saiStart(&SAID,&saiConfig);
#endif
	  isAudioInit = 1;
  }
  return;
}

void AUDIO_IO_DeInit(void){
	isAudioInit = 0;
	i2cStop(&I2CD_CODEC);
#ifdef STM32F407xx
	i2sStop(&I2S_DRIVER);
#else
	saiStop(&SAID);
#endif


}
void AUDIO_IO_ReInit(uint32_t AudioFreq){(void)AudioFreq;
#ifdef STM32F407xx
	  i2sStart(&I2S_DRIVER, &i2scfg);
#else
	  saiStart(&SAID, &saiConfig);
#endif
}
void AUDIO_IO_Exchange(void){
#ifdef STM32F407xx
	i2sStartExchange(&I2S_DRIVER);
#else
	saiStartExchange(&SAID);
#endif
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
#ifdef STM32F407xx
  i2sStart(&I2S_DRIVER, &i2scfg);
#else
  saiStart(&SAID, &saiConfig);
#endif
  if ( decodeMP3File(pAudioPlayerDriverITF) !=0 ){
      dbgprintf("failed in libmad; unknown error\r\n");
      return FR_DISK_ERR;//TODO: a hack, plz clean up
  }
  /*
   *
   * *******IMPORTANT*****
   * If saiStop() not called here and called from somewhere else, the thread will hang
   */
#ifdef STM32F407xx
  i2sStopExchange(&I2S_DRIVER);
#else
  saiStopExchange(&SAID);
#endif

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
static uint32_t Mp3ReadId3V2Text(FIL* pInFile, uint32_t unDataLen, char* pszBuffer, uint32_t unBufferSize)
{
  UINT unRead = 0;
  BYTE byEncoding = 0;
  if((f_read(pInFile, &byEncoding, 1, &unRead) == FR_OK) && (unRead == 1))
  {
    unDataLen--;
    if(unDataLen <= (unBufferSize - 1))
    {
      if((f_read(pInFile, pszBuffer, unDataLen, &unRead) == FR_OK) ||
          (unRead == unDataLen))
      {
        if(byEncoding == 0)
        {
          // ISO-8859-1 multibyte
          // just add a terminating zero
          pszBuffer[unDataLen] = 0;
        }
        else if(byEncoding == 1)
        {
          // UTF16LE unicode
          uint32_t r = 0;
          uint32_t w = 0;
          if((unDataLen > 2) && (pszBuffer[0] == 0xFF) && (pszBuffer[1] == 0xFE))
          {
            // ignore BOM, assume LE
            r = 2;
          }
          for(; r < unDataLen; r += 2, w += 1)
          {
            // should be acceptable for 7 bit ascii
            pszBuffer[w] = pszBuffer[r];
          }
          pszBuffer[w] = 0;
        }
      }
      else
      {
        return 1;
      }
    }
    else
    {
      // we won't read a partial text
      if(f_lseek(pInFile, f_tell(pInFile) + unDataLen) != FR_OK)
      {
        return 1;
      }
    }
  }
  else
  {
    return 1;
  }
  return 0;
}
static uint32_t Mp3ReadId3V2Tag(FIL* pInFile, AudioFileInfo_TypeDef   *pAudioFileInfo){
  pAudioFileInfo->artist[0] = 0;
  pAudioFileInfo->title[0] = 0;

  BYTE id3hd[10];
  UINT unRead = 0;
  if((f_read(pInFile, id3hd, 10, &unRead) != FR_OK) || (unRead != 10))
  {
    return 1;
  }
  else
  {
    uint32_t unSkip = 0;
    if((unRead == 10) &&
        (id3hd[0] == 'I') &&
        (id3hd[1] == 'D') &&
        (id3hd[2] == '3'))
    {
      unSkip += 10;
      unSkip = ((id3hd[6] & 0x7f) << 21) | ((id3hd[7] & 0x7f) << 14) | ((id3hd[8] & 0x7f) << 7) | (id3hd[9] & 0x7f);

      // try to get some information from the tag
      // skip the extended header, if present
      uint8_t unVersion = id3hd[3];
      if(id3hd[5] & 0x40)
      {
        BYTE exhd[4];
        f_read(pInFile, exhd, 4, &unRead);
        size_t unExHdrSkip = ((exhd[0] & 0x7f) << 21) | ((exhd[1] & 0x7f) << 14) | ((exhd[2] & 0x7f) << 7) | (exhd[3] & 0x7f);
        unExHdrSkip -= 4;
        if(f_lseek(pInFile, f_tell(pInFile) + unExHdrSkip) != FR_OK)
        {
          return 1;
        }
      }
      uint32_t nFramesToRead = 2;
      while(nFramesToRead > 0)
      {
        char frhd[10];
        if((f_read(pInFile, frhd, 10, &unRead) != FR_OK) || (unRead != 10))
        {
          return 1;
        }
        if((frhd[0] == 0) || (strncmp(frhd, "3DI", 3) == 0))
        {
          break;
        }
        char szFrameId[5] = {0, 0, 0, 0, 0};
        memcpy(szFrameId, frhd, 4);
        uint32_t unFrameSize = 0;
        uint32_t i = 0;
        for(; i < 4; i++){
          if(unVersion == 3){
            // ID3v2.3
            unFrameSize <<= 8;
            unFrameSize += frhd[i + 4];
          }
          if(unVersion == 4){
            // ID3v2.4
            unFrameSize <<= 7;
            unFrameSize += frhd[i + 4] & 0x7F;
          }
        }

        if(strcmp(szFrameId, "TPE1") == 0){
          // artist
          if(Mp3ReadId3V2Text(pInFile, unFrameSize, pAudioFileInfo->artist, sizeof(pAudioFileInfo->artist)) != 0)
            break;
          nFramesToRead--;
        }
        else if(strcmp(szFrameId, "TIT2") == 0){
          // title
          if(Mp3ReadId3V2Text(pInFile, unFrameSize, pAudioFileInfo->title, sizeof(pAudioFileInfo->title)) != 0)
            break;
          nFramesToRead--;
        }
        else{
          if(f_lseek(pInFile, f_tell(pInFile) + unFrameSize) != FR_OK)
          {
            return 1;
          }
        }
      }
      dbgprintf("Mp3Decode: Skipping %u bytes of ID3v2 tag\r\n", unSkip + 1);
    }
    if(f_lseek(pInFile, unSkip) != FR_OK)
    {
      return 1;
    }
  }

  return 0;
}

#define METADATA_RECORD_MAX_RECORD_LEN 		FILENAME_MAX_SIZE+TRACK_KEY_MAX_SIZE+10
#define PADS 								5
#define CHARS_TO_SHOW 						20

static uint32_t trackID = 0;
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
    		  memset(pAudioFileInfo,0,sizeof(AudioFileInfo_TypeDef));
    		  pAudioFileInfo->trackID = ++trackID;
    		  pAudioFileInfo->audioFileType = AUDIO_FILE_TYPE_MP3;
    		  strlcpy(pAudioFileInfo->filename,fpath,FILENAME_MAX_SIZE);
    		  chsnprintf(pAudioFileInfo->trackKey,sizeof(pAudioFileInfo->trackKey),"TRACK_%d",pAudioFileInfo->trackID);
    		  FIL 				   fIn;
    		  FRESULT                errFS            = FR_OK;
    		  errFS = f_open(&fIn, pAudioFileInfo->filename, FA_READ);
    		  if(errFS == FR_OK){
    		      Mp3ReadId3V2Tag(&fIn,pAudioFileInfo);
    			  f_close(&fIn);
    		  }
    		  else
    			   dbgprintf("----Mp3Decode: Failed to open file \"%s\" for reading, err=%d\r\n", pAudioFileInfo->filename, errFS);
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

#if S4E_USE_MQTT
//returns none null if found, else return null to continue looking for a match
LinkedListElement* audioFileInfoToJson(LinkedListElement* pItem, void *arg){
	char                    *_payload 	    = (char*)arg;
   	char 					metadataRecord[METADATA_RECORD_MAX_RECORD_LEN]={0};
	AudioFileInfo_TypeDef   *pAudioFileInfo = (AudioFileInfo_TypeDef*)pItem->value;
	uint16_t                len 			= strlen(_payload);
	bool 					spaceAvail 		= len <  (MAX_MQTT_PAYLOAD - METADATA_RECORD_MAX_RECORD_LEN -PADS);

	char *titleToUse = pAudioFileInfo->title[0] == 0?pAudioFileInfo->filename:pAudioFileInfo->title;
	int8_t  ndx = strlen(titleToUse) - CHARS_TO_SHOW;
	ndx = ndx >= 0 ? ndx: 0;
   	chsnprintf(metadataRecord,METADATA_RECORD_MAX_RECORD_LEN,"\"%s\":\"%s\"",titleToUse+ndx,pAudioFileInfo->trackKey);
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
#endif
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

