#include "ch.h"
#include "hal.h"
#include <ff.h>
#include "mad.h"
#include "hal_sai.h"
#include <string.h>
#include "audio.h"

#define HAS_CB_METHOD(__method__) 			curentlyPlayingMadInfo !=NULL && curentlyPlayingMadInfo->__method__ != NULL


void *mp3_malloc(int s) {
	void *p= chHeapAlloc(NULL, s);
	if ( p == NULL)
		chSysHalt("no mem");
	return p;
}
void mp3_free(void *p) {
    dbgprintf("freeing mem\r\n");
	chHeapFree(p);
}

void *mp3_calloc(int num,int size)
{
 unsigned char *ptr;
 int i;
 dbgprintf("Calloc %d %d -> \r\n",num,size);
 ptr=mp3_malloc(size*num);
 if (!ptr) return NULL;
 for(i=0;i<num*size;i++) ptr[i]=0;
 return ptr;
}

struct MADBuffer {
  unsigned char *start;
  int eof;
  FIL *file;
  int samplerate;
};
//By default these buffers are placed in .ram3 (DTCM-RAM ) by default, you can check the .map file
//Therefore there is no need to invalidate the cache to get around cache coherence issue
//However, DTCM-RAM is small, and if you place the buffer in ram0, it will work but you will hear hissing sound on the DAC
//__attribute__((section(".ram0.wave_buf")))
adcsample_t wave_buf[CACHE_SIZE_ALIGN(adcsample_t,MP3_BUFF_SIZE)];
static AudioPlayerDriverITF_Typedef  *curentlyPlayingMadInfo;
/*
 * *******IMPORTANT*****
 * If the allocated memory is less than 0x4000= 4 * 4096, then madlib may give you
 * lots of decoding error like bad bit rate
 */
#define MP3BUF 				   4096 * 4
#define INTERRUPT_PLAYING(x)   (x == NEXT_TRACK || x == PREV_TRACK || x == REPOSITION_ITERATOR)
#define PAUSE_PLAYING(x)       (x == PAUSE_PLAYER)
static unsigned char *mp3buf = NULL;
//static unsigned char mp3buf[MP3BUF];
static FIL fIn;
//FIL fOut;
struct MADBuffer buffer;
struct mad_decoder decoder;

/*
 * This is the input callback. The purpose of this callback is to (re)fill
 * the stream buffer which is to be decoded. In this example, an entire file
 * has been mapped into memory, so we just call mad_stream_buffer() with the
 * address and length of the mapping. When this callback is called a second
 * time, we are finished decoding.
 */

static ActionEvent_Typedef blinkActionEvent = {.eventSource="libmad"};
enum mad_flow input(void *data, struct mad_stream *stream){
  struct MADBuffer *buffer 			= data;
  UINT 				unRead 			= 0, MaxRead;
  UINT 				RemainingBytes 	= stream->bufend - stream->next_frame;

  enum mad_flow  pauseOrSkip = 0;
  if ( HAS_CB_METHOD(onPreReadCallback) )
	  curentlyPlayingMadInfo->onPreReadCallback(buffer);
  //Skip
  if ((buffer->eof) || INTERRUPT_PLAYING(curentlyPlayingMadInfo->actionEventEnum) ){
	   curentlyPlayingMadInfo->state       = AP_SKIPPING;
      return MAD_FLOW_STOP;
  }
  else
  //Resume
  if ( PAUSE_PLAYING(curentlyPlayingMadInfo->actionEventEnum) ){
	  chSysLock();
	  curentlyPlayingMadInfo->state = AP_PAUSING;
	  chThdSuspendS((thread_reference_t *)&(curentlyPlayingMadInfo->playerThdRef));
	  chSysUnlock();
	  curentlyPlayingMadInfo->playerThdRef = NULL;
	  curentlyPlayingMadInfo->actionEventEnum = PLAY_AUDIO;
	  curentlyPlayingMadInfo->state = AP_PLAYING;
  }

  if ((RemainingBytes)&&(RemainingBytes<MP3BUF))
    memmove(buffer->start, stream->next_frame, RemainingBytes);

  MaxRead = (MP3BUF - RemainingBytes)&0xFE00;

  FRESULT fr = f_read(buffer->file, buffer->start + RemainingBytes, MaxRead, &unRead);
  if (unRead!=MaxRead) buffer->eof = 1;

//  dbgprintf("Read : %d (rem=%d - MaxRead=%d)\n\r",unRead,RemainingBytes,MaxRead);

  mad_stream_buffer(stream, buffer->start, RemainingBytes+unRead);

  if ( HAS_CB_METHOD(onPostReadCallback) )
 	 curentlyPlayingMadInfo->onPostReadCallback(&blinkActionEvent);

  return MAD_FLOW_CONTINUE;
}


enum mad_flow header(void *data, struct mad_header const *header)
{
  struct MADBuffer *buffer = data;

  if (buffer->samplerate!=(int)header->samplerate){
     //AUDIO_IO_ReInit(header->samplerate);
     buffer->samplerate=header->samplerate;
     if ( HAS_CB_METHOD(onHeaderCallback) )
    	 curentlyPlayingMadInfo->onHeaderCallback((void*)header);
  }

  return MAD_FLOW_CONTINUE;
}

/*
 * The following utility routine performs simple rounding, clipping, and
 * scaling of MAD's high-resolution samples down to 16 bits. It does not
 * perform any dithering or noise shaping, which would be recommended to
 * obtain any exceptional audio quality. It is therefore not recommended to
 * use this routine if high-quality output is desired.
 */

signed int scale(mad_fixed_t sample){
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));
#if 0
  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
#else
  { signed int result;
    asm("ssat %0, %2, %1, asr %3" \
        : "=r" (result) \
        : "%r" (sample), \
          "M" (16), "M" (MAD_F_FRACBITS + 1 - 16) );
    return result; }
#endif
}

/*
 * This is the output callback function. It is called after each frame of
 * MPEG audio data has been completely decoded. The purpose of this callback
 * is to output (or play) the decoded PCM audio.
 */

enum mad_flow output(void *data,   struct mad_header const *header,  struct mad_pcm *pcm){(void)data;(void)header;
  unsigned int nchannels        = pcm->channels;
  unsigned int nsamples         = pcm->length;
  mad_fixed_t const *left_ch    = pcm->samples[0];
  mad_fixed_t const *right_ch   = pcm->samples[1];
  signed sample;
  int ndx = 0 ;

  ++curentlyPlayingMadInfo->pAudioFileInfo->frame;
  //dbgprintf("Batch:%d, nsamples=%d\r\n",++curentlyPlayingMadInfo->pAudioFileInfo->frame, nsamples );
  if ( IS_PLAYING(curentlyPlayingMadInfo) ){
	  //Block This thread until the DMA is half way (or completely) done transfering
	  //data to SAI/codec, plz see callback saicallback() method in audio.c for more details
	  curentlyPlayingMadInfo->waitAny = true;
      eventmask_t  em = chEvtWaitAny(EVENT_MASK(COMPLETE_EVENT) | EVENT_MASK(HALFWAY_EVENT));
      curentlyPlayingMadInfo->waitAny = false;
      //1st - Invalide the memory that we have used
//      cacheBufferInvalidate(&wave_buf[ndx], sizeof (wave_buf) / (2*sizeof (adcsample_t)));
      //2nd - Set ndx halfway when the half way callback notification comes from DMA's callback (see saicallback())
      ndx =  em & EVENT_MASK(HALFWAY_EVENT)? MP3_BUFF_SIZE/2: 0;
  }
  uint16_t *output=&wave_buf[ndx];
  while (nsamples--) {
    sample = scale(*left_ch++);
    *output++=sample;
    if (nchannels == 2) sample = scale(*right_ch++);
    *output++=sample;
  }
  //FRESULT fr = f_write(&fOut, write_buf, 4*pcm->length, &unWrite);
  if ( HAS_CB_METHOD(onWriteCallback) )
     curentlyPlayingMadInfo->onWriteCallback(pcm);
  //If this is the 1st frame, start DMA transfer
  if ( curentlyPlayingMadInfo->state == AP_STOPPED ){
     curentlyPlayingMadInfo->state = AP_PLAYING;
	 AUDIO_IO_Exchange();
   }

  return MAD_FLOW_CONTINUE;
}

/*
 * This is the error callback function. It is called whenever a decoding
 * error occurs. The error is indicated by stream->error; the list of
 * possible MAD_ERROR_* errors can be found in the mad.h (or stream.h)
 * header file.
 */

enum mad_flow error(void *data, struct mad_stream *stream, struct mad_frame *frame){(void)frame;
   struct MADBuffer *buffer = data;
   enum mad_flow rc = MAD_FLOW_CONTINUE;
   dbgprintf("Error Decoding 0x%04x (%s) at byte offset %u\r\n",  stream->error, mad_stream_errorstr(stream),  stream->this_frame - buffer->start);
   if ( HAS_CB_METHOD(onErrorCallback) )
  	   rc = curentlyPlayingMadInfo->onErrorCallback(data,stream);

  /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

  return rc;
}

/*
 * This is the function called by main() above to perform all the decoding.
 * It instantiates a decoder object and configures it with the input,
 * output, and error callback functions above. A single call to
 * mad_decoder_run() continues until a callback function returns
 * MAD_FLOW_STOP (to stop decoding) or MAD_FLOW_BREAK (to stop decoding and
 * signal an error).
 */


int decodeMP3File(AudioPlayerDriverITF_Typedef  *pmadInfo)
{
  int result,i;

  curentlyPlayingMadInfo = pmadInfo;

  dbgprintf("===>Mp3Decode: opening file \"%s\"\r\n", curentlyPlayingMadInfo->pAudioFileInfo->filename);
  FRESULT errFS = f_open(&fIn, curentlyPlayingMadInfo->pAudioFileInfo->filename, FA_READ);
  if(errFS != FR_OK){
	  dbgprintf("----Mp3Decode: Failed to open file \"%s\" for reading, err=%d\r\n", curentlyPlayingMadInfo->pAudioFileInfo->filename, errFS);
    return -1;
  }
  curentlyPlayingMadInfo->pAudioFileInfo->fileSize =f_size(&fIn);
  if ( mp3buf == NULL )
	  mp3buf = mp3_malloc(MP3BUF);
  buffer.start  = mp3buf;
  buffer.file   = &fIn;
  buffer.eof    = 0;
  buffer.samplerate = 0;
  curentlyPlayingMadInfo->state 						= AP_STOPPED;
  curentlyPlayingMadInfo->actionEventEnum				= PLAY_AUDIO;

  /* configure input, output, and error functions */
  mad_decoder_init(&decoder, &buffer,
		   input, header, 0 /* filter */, output,
		   error, 0 /* message */);

  /* start decoding */
  result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

  /* release the decoder */
  mad_decoder_finish(&decoder);
  errFS = f_close(&fIn);

  dbgprintf("===>Mp3Decode: CLOSING file \"%s\"\r\n", curentlyPlayingMadInfo->pAudioFileInfo->filename);
  dbgprintf("\r\n++++Decoding return %d++++\r\n",result);
  curentlyPlayingMadInfo = NULL;
  curentlyPlayingMadInfo->state = AP_STOPPED;
  curentlyPlayingMadInfo->actionEventEnum= PLAY_AUDIO;

  return result;
}
