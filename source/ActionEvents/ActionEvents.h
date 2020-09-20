#ifndef ACTIONEVENTS_H_
#define ACTIONEVENTS_H_
#include "ActionEventsThd.h"


#define TOGGLE_MUTE_AE_NAME                 "toggleMute"
#define NEXT_TRACK_AE_NAME                  "nextTrack"
#define PREV_TRACK_AE_NAME                  "prevTrack"
#define REPOSITION_TO_TRACK_AE_NAME			"repositionToTrack"
#define TOGGLE_PAUSE_AE_NAME                "togglePause"
#define VOLUME_UP_AE_NAME                   "volumeUp"
#define VOLUME_DOWN_AE_NAME                 "volumeDown"
#define SET_VOLUME_AE_NAME                  "setVolume"
#define UPDATE_WIFI_HTML_AE_NAME            "updateWiFi"
#define LOAD_SYS_PROPERTIES_AE_NAME         "loadSysProperties"
#define SET_RGB_LED_AE_NAME                 "setRGBLED"
#define NEW_HTML_LOADED_AE_NAME				"newHTMLLoaded"
#define NEW_HTML_LOADED_AE_NAME				"newHTMLLoaded"
#define PERFORMANCE_INFO_AE_NAME			"performanceInfo"
#define SET_UNIX_TIME_AE_NAME				"setUnixtime"
#define GO_TO_SLEEP_AE_NAME                 "goToSleep"

#define PAUSE_PLAYER_NOW    				2
#define UNPAUSE_PLAYER_NOW    				3
#define MUTE_PLAYER_NOW    					2
#define UNMUTE_PLAYER_NOW    				3
#ifdef __cplusplus
 extern "C" {
#endif
int8_t getCurrentMute(void);
int8_t getCurrentVolume(void);
int32_t toggleMuteView(ActionEvent_Typedef 	*pActionEvent);
int32_t volumeView(ActionEvent_Typedef 	*pActionEvent);
int32_t updateWifiHtmlView(ActionEvent_Typedef 	*pActionEvent);
int32_t loadSysPropertiesView(ActionEvent_Typedef 	*pActionEvent);
int32_t setRGBLEDView(ActionEvent_Typedef 	*pActionEvent);
void updateLCD(void);
#ifdef __cplusplus
}
#endif
#endif /* ACTIONEVENTS_H_ */
