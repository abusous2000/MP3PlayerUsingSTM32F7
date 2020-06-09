/******************************************************************************/
/* This file has been generated by the uGFX-Studio                            */
/*                                                                            */
/* http://ugfx.org                                                            */
/******************************************************************************/

#ifndef _S4E_GUI_H_
#define _S4E_GUI_H_
#if USE_LCD_TFT == 1
#include "gfx.h"

// Widget groups
#define GROUP_DEFAULT				0
#define MAX_GHANLDES 				40
#define CONTAINER1_GH_NDX 			0
#define VOLUME_GH_NDX 	  			1
#define TOOGLE_PAUSE_GH_NDX 	  	2
#define TOOGLE_MUTE_GH_NDX 	  		3
#define NEXT_TRACK_GH_NDX 	  		4
#define PREV_TRACK_GH_NDX 	  		5
#define TRACKS_LIST_GH_NDX 	  		6
#define STATUS_LABEL_GH_NDX 		7
#define STATUS_GH_NDX 	  			8
#define TRACK_LABEL_GH_NDX 			9
#define TRACK_GH_NDX 	  			10
#define APP_LABLE_GH_NDX 	  		11
#define CONSOLE_GH_NDX 	  			20
//
//// GListeners
//extern GListener glistener;
//
//// GHandles
//extern GHandle ghContainerPage0;
//extern GHandle ghConsole1;
//extern GHandle ghLabel1;
//extern GHandle ghLabelPM25;
//extern GHandle ghLabel3;
//extern GHandle ghLabelHCHO;
//extern GHandle ghLabel5;
//extern GHandle ghLabelTemp;
//extern GHandle ghLabel7;
//extern GHandle ghLabelHumid;
//extern GHandle ghLabel26;
//extern GHandle ghLabelPM1;
//extern GHandle ghLabel28;
//extern GHandle ghLabelPM10;
//extern GHandle ghVolume;
//extern GHandle ghTogglePause;
//extern GHandle ghToggleMute;
//extern GHandle ghNextTrack;
//extern GHandle ghPrevTrack;
//extern GHandle ghTracks;
//
//// Function Prototypes
//void guiCreate(void);
//void guiShowPage(unsigned pageIndex);
//void guiEventLoop(void);
#ifdef __cplusplus
 extern "C" {
#endif
void selectTrackInDropdownList(char *trackKey);
void checkboxMute(bool check);
void checkboxPause(bool check);
void slideVolumeSet(uint8_t vol);
void updateLCDTFTStatus(void);
#ifdef __cplusplus
}
#endif
#endif/*USE_LCD_TFT*/
#endif /* _GUI_H_ */

