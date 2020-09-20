#ifndef AudioPlayerThd_h
#define AudioPlayerThd_h


#ifndef AUDIO_THD_WA_STACK_SIZE
#define AUDIO_THD_WA_STACK_SIZE   		0x2000
#endif
#ifdef __cplusplus
 extern "C" {
#endif
void initAudioPlayerThd(void);
void audioPlayerPrepareGoingToSleep(void);
#ifdef __cplusplus
}
#endif

#endif
