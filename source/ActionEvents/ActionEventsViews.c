/*
 * ActionEventsViews.c
 *
 *  Created on: Dec 27, 2019
 *      Author: abusous2000
 */
#include "Strust4EmbeddedConf.h"
#include "ch.h"
#include "hal.h"
#include "Strust4Embedded.h"
#include "ActionEvents.h"
#include "audio.h"
#include "gui.h"

extern AudioPlayerDriverITF_Typedef	*pAudioPlayerDriverITF;
void updateLCD(void){
#if S4E_USE_SSD1306_LCD == 1
   int   forMinutes      = pAudioPlayerDriverITF->pAudioFileInfo->secondsRemaining / 60;
   int   remainingSec    = pAudioPlayerDriverITF->pAudioFileInfo->secondsRemaining % 60;

   char  buffer1[20]={0};
   char  minBuffer[10];
   char  secBuffer[10];
   char  buffer2[20]={0};

   fillScreenRow(SSD1306_COLOR_BLACK,3);
   fillScreenRow(SSD1306_COLOR_BLACK,4);
   chsnprintf(buffer1, sizeof(buffer1),"F:%d V:%d S:%c",pAudioPlayerDriverITF->pAudioFileInfo->frame,getCurrentVolume(),getAPStateShortName(pAudioPlayerDriverITF->state));
   chsnprintf(buffer2, sizeof(buffer2),"Trk:%d T:%d %d:%d ",pAudioPlayerDriverITF->pAudioFileInfo->trackID, ++pAudioPlayerDriverITF->pAudioFileInfo->totalSecondsPlayed, forMinutes,remainingSec);
   LCD_Display_String(pAudioPlayerDriverITF->pAudioFileInfo->filename,16, false);
   LCD_Display_String(buffer2,30, false);
   LCD_Display_String(buffer1,44, false);
   LCD_Display_Update();
#endif
#if USE_LCD_TFT == 1
   updateLCDTFTStatus();
#endif

   return;
}
int32_t loadSysPropertiesView(ActionEvent_Typedef 	*pActionEvent){(void)pActionEvent;
   dbgprintf("***sent loadSysPropertiesView\r\n");

   return MSG_OK;
}
int32_t updateWifiHtmlView(ActionEvent_Typedef 	*pActionEvent){(void)pActionEvent;
   dbgprintf("***sent WifiModule msg to refresh its config properties\r\n");
   updateLCD();

#if S4E_USE_SSD1306_LCD == 1
   fillScreenRow(SSD1306_COLOR_BLACK,5);
   LCD_Display_String("Updated Wifi",60, false);
#endif

   return MSG_OK;
}
int32_t toggleMuteView(ActionEvent_Typedef 	*pActionEvent){(void)pActionEvent;
	dbgprintf("***Setting Mute: %d\r\n",getCurrentMute());
	updateLCD();

	return MSG_OK;
}

int32_t volumeView(ActionEvent_Typedef 	*pActionEvent){(void)pActionEvent;
   dbgprintf("Vol.: %d\r\n", getCurrentVolume());
   updateLCD();

   return MSG_OK;
}
int32_t setRGBLEDView(ActionEvent_Typedef 	*pActionEvent){(void)pActionEvent;
   dbgprintf("Set RGB.: %s\r\n", pActionEvent->u.pData);

   return MSG_OK;
}
