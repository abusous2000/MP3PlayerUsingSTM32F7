/******************************************************************************/
/* This file has been generated by the uGFX-Studio                            */
/*                                                                            */
/* http://ugfx.org                                                            */
/******************************************************************************/

#include "Strust4EmbeddedConf.h"
#if USE_LCD_TFT != 0
#include "Strust4Embedded.h"
#include "audio.h"
#include "colors.h"
#include "widgetstyles.h"
#include "gui.h"

static GHandle gHandles[MAX_GHANLDES]={0};
// Fonts
static font_t dejavu_sans_12;
static font_t dejavu_sans_16;
static font_t dejavu_sans_32_anti_aliased;
static int  yOffset = 5;
static uint16_t LCD_MAX_WIDTH_PLAYER=240;
const char *APP_NAME = "MP3Player w/ S4E & STM32F746g";

static const GWidgetStyle OrangeWidgetStyle = {
		Orange,							// window background
	HTML2COLOR(0x800000),			// focus

	// enabled color set
	{
		HTML2COLOR(0x0000FF),		// text
		HTML2COLOR(0x404040),		// edge
		HTML2COLOR(0xE0E0E0),		// fill
		HTML2COLOR(0x00E000)		// progress - active area
	},

	// disabled color set
	{
		HTML2COLOR(0xC0C0C0),		// text
		HTML2COLOR(0x808080),		// edge
		HTML2COLOR(0xE0E0E0),		// fill
		HTML2COLOR(0xC0E0C0)		// progress - active area
	},

	// pressed color set
	{
		HTML2COLOR(0xFF00FF),		// text
		HTML2COLOR(0x404040),		// edge
		HTML2COLOR(0x808080),		// fill
		HTML2COLOR(0x00E000),		// progress - active area
	}
};

static const GWidgetStyle NavyWidgetStyle = {
		Navy,							// window background
	HTML2COLOR(0x800000),			// focus

	// enabled color set
	{
		HTML2COLOR(0x0000FF),		// text
		HTML2COLOR(0x404040),		// edge
		HTML2COLOR(0xE0E0E0),		// fill
		HTML2COLOR(0x00E000)		// progress - active area
	},

	// disabled color set
	{
		HTML2COLOR(0xC0C0C0),		// text
		HTML2COLOR(0x808080),		// edge
		HTML2COLOR(0xE0E0E0),		// fill
		HTML2COLOR(0xC0E0C0)		// progress - active area
	},

	// pressed color set
	{
		HTML2COLOR(0xFF00FF),		// text
		HTML2COLOR(0x404040),		// edge
		HTML2COLOR(0x808080),		// fill
		HTML2COLOR(0x00E000),		// progress - active area
	}
};
int16_t getLCDTFTHeight(void);
int16_t getLCDTFTWidth(void);
static void createPagePage0(void){

	GWidgetInit wi;
	gwinWidgetClearInit(&wi);

	LCD_MAX_WIDTH_PLAYER = getLCDTFTWidth();
	// create container widget: GHandles[CONTAINER1_GH_NDX]
	wi.g.show = FALSE;
	wi.g.x = 0;
	wi.g.y = 0;
//For some reason, uGFX only worked with portrait
#ifdef STM32F769xx
	wi.g.width  = getLCDTFTWidth();
	wi.g.height = getLCDTFTHeight() ;
#else
	wi.g.width  = getLCDTFTHeight();
	wi.g.height = getLCDTFTWidth() ;
#endif
	wi.g.parent = 0;
	wi.text = "MP3 Player";
	wi.customDraw = 0;
	wi.customParam = 0;
	wi.customStyle = 0;
	gHandles[CONTAINER1_GH_NDX] = gwinContainerCreate(0, &wi, 0);

	wi.g.show = TRUE;
	wi.g.x = 5;
	wi.g.y = yOffset; //10
	wi.g.width = LCD_MAX_WIDTH_PLAYER;
	wi.g.height = 55;
	wi.g.parent = gHandles[CONTAINER1_GH_NDX];
	wi.text = APP_NAME;
	wi.customDraw = gwinLabelDrawJustifiedCenter;
//	wi.customDraw = gwinLabelDrawJustifiedLeft;
	wi.customParam = 0;
	wi.customStyle = 0;
	gHandles[APP_LABLE_GH_NDX] = gwinLabelCreate(0, &wi);
	gwinSetFont(gHandles[APP_LABLE_GH_NDX], dejavu_sans_12);
	gwinSetStyle(gHandles[APP_LABLE_GH_NDX],&OrangeWidgetStyle);
	gwinLabelSetBorder(gHandles[APP_LABLE_GH_NDX], TRUE);

	yOffset +=wi.g.height + 5;
	// Create label widget: Label MP3 Player Status
	wi.g.show = TRUE;
	wi.g.x = 5;
	wi.g.y = yOffset; //10
	wi.g.width = 65;
	wi.g.height = 20;
	wi.g.parent = gHandles[CONTAINER1_GH_NDX];
	wi.text = "Status";
	wi.customDraw = gwinLabelDrawJustifiedLeft;
	wi.customParam = 0;
	wi.customStyle = 0;
	gHandles[STATUS_LABEL_GH_NDX] = gwinLabelCreate(0, &wi);
	gwinSetBgColor(gHandles[STATUS_LABEL_GH_NDX], red_studio);
//	gwinLabelSetBorder(gHandles[STATUS_LABEL_GH_NDX], TRUE);

	// Create label widget: Label for the track which is currently playing
	wi.g.show = TRUE;
	wi.g.x = 75;
	wi.g.y = yOffset;//10
	wi.g.width = 200;
	wi.g.height = 20;
	wi.g.parent = gHandles[CONTAINER1_GH_NDX];
	wi.text = "Track";
	wi.customDraw = gwinLabelDrawJustifiedLeft;
	wi.customParam = 0;
	wi.customStyle = 0;
	gHandles[TRACK_LABEL_GH_NDX] = gwinLabelCreate(0, &wi);
	gwinLabelSetBorder(gHandles[TRACK_LABEL_GH_NDX], FALSE);

	yOffset +=wi.g.height + 5;
	// Create label widget: Frame & Vol
	wi.g.show = TRUE;
	wi.g.x = 5;
	wi.g.y = yOffset; //35
	wi.g.width = 120;
	wi.g.height = 40;
	wi.g.parent = gHandles[CONTAINER1_GH_NDX];
	wi.text = "N/A";
	wi.customDraw = gwinLabelDrawJustifiedLeft;
	wi.customParam = 0;
	wi.customStyle = 0;
	gHandles[STATUS_GH_NDX] = gwinLabelCreate(0, &wi);
	gwinLabelSetBorder(gHandles[STATUS_GH_NDX], FALSE);
//	gwinSetFont(gHandles[STATUS_GH_NDX], dejavu_sans_32_anti_aliased);
//	gwinRedraw(gHandles[STATUS_GH_NDX]);

	// Create label widget: Track & time remaining
	wi.g.show = TRUE;
	wi.g.x = 130;
	wi.g.y = yOffset;
	wi.g.width = 140;
	wi.g.height = 40;
	wi.g.parent = gHandles[CONTAINER1_GH_NDX];
	wi.text = "0.0";
	wi.customDraw = gwinLabelDrawJustifiedLeft;
	wi.customParam = 0;
	wi.customStyle = 0;
	gHandles[TRACK_GH_NDX] = gwinLabelCreate(0, &wi);
	gwinLabelSetBorder(gHandles[TRACK_GH_NDX], FALSE);
//	gwinSetFont(gHandles[TRACK_GH_NDX], dejavu_sans_32_anti_aliased);
//	gwinRedraw(gHandles[TRACK_GH_NDX]);

    yOffset +=wi.g.height + 5;
    wi.g.x = 5;
    wi.g.y = yOffset;//75 or 80
    wi.g.width = LCD_MAX_WIDTH_PLAYER;
    wi.g.height = 35;
    wi.g.show = TRUE;
    wi.g.parent = gHandles[CONTAINER1_GH_NDX];
    wi.customDraw = 0;
    wi.customParam = 0;
    wi.customStyle = 0;
    wi.text = "Volume";
    gHandles[VOLUME_GH_NDX] = gwinSliderCreate(0, &wi);
//    gwinRedraw(gHandles[VOLUME_GH_NDX]);

    yOffset +=wi.g.height + 5;
	gwinWidgetClearInit(&wi);
    wi.g.x = 5;
    wi.g.y = yOffset;
	wi.g.width = 110;
	wi.g.height = 30;
	wi.text = " Pause";
	wi.g.show = TRUE;
	wi.g.parent = gHandles[CONTAINER1_GH_NDX];
	//gHandles[TOOGLE_PAUSE_GH_NDX] = gwinButtonCreate(0, &wi);
	gHandles[TOOGLE_PAUSE_GH_NDX] = gwinCheckboxCreate(0, &wi);

	gwinWidgetClearInit(&wi);
    wi.g.x = 135;
    wi.g.y = yOffset;
	wi.g.width = 110;
	wi.g.height = 30;
	wi.text = " Mute";
	wi.g.show = TRUE;
	wi.g.parent = gHandles[CONTAINER1_GH_NDX];
	gHandles[TOOGLE_MUTE_GH_NDX] = gwinCheckboxCreate(0, &wi);

	gwinWidgetClearInit(&wi);
    wi.g.x = 235;
    wi.g.y = yOffset;
	wi.g.width = 110;
	wi.g.height = 30;
	wi.text = " Sleep";
	wi.g.show = TRUE;
	wi.g.parent = gHandles[CONTAINER1_GH_NDX];
	gHandles[TOOGLE_SLEEP_GH_NDX] = gwinCheckboxCreate(0, &wi);

	yOffset +=wi.g.height + 5;
	gwinWidgetClearInit(&wi);
    wi.g.x = 5;
    wi.g.y = yOffset;//180
	wi.g.width = 110;
	wi.g.height = 30;
	wi.text = ">> Next";
	wi.g.show = TRUE;
	wi.g.parent = gHandles[CONTAINER1_GH_NDX];
	gHandles[NEXT_TRACK_GH_NDX] = gwinButtonCreate(0, &wi);
//	gwinSetStyle(gHandles[NEXT_TRACK_GH_NDX],&NavyWidgetStyle);

	gwinWidgetClearInit(&wi);
    wi.g.x = 135;
    wi.g.y = yOffset;
	wi.g.width = 110;
	wi.g.height = 30;
	wi.text = "<< Prev";
	wi.g.show = TRUE;
	wi.g.parent = gHandles[CONTAINER1_GH_NDX];
	gHandles[PREV_TRACK_GH_NDX] = gwinButtonCreate(0, &wi);
//	gwinSetStyle(gHandles[PREV_TRACK_GH_NDX],&NavyWidgetStyle);

	yOffset +=wi.g.height + 10;
	// The first list widget
	wi.g.width = LCD_MAX_WIDTH_PLAYER;
	wi.g.height = 120;
	wi.g.y = yOffset;
	wi.g.x = 5;
	wi.text = "Tracks";
	wi.g.parent = gHandles[CONTAINER1_GH_NDX];
	gHandles[TRACKS_LIST_GH_NDX] = gwinListCreate(0, &wi, FALSE);
	gwinListSetScroll(gHandles[TRACKS_LIST_GH_NDX], scrollSmooth);

}

void guiShowPage(unsigned pageIndex){
	// Hide all pages
	gwinHide(gHandles[CONTAINER1_GH_NDX]);

	// Show page selected page
	switch (pageIndex) {
	case 0:
		gwinShow(gHandles[CONTAINER1_GH_NDX]);
		break;

	default:
		break;
	}
}

void guiCreate(void){
	gwinSetDefaultFont(gdispOpenFont("DejaVuSans12"));
	dejavu_sans_12 = gdispOpenFont("DejaVuSansBold12");

	gwinSetDefaultStyle(&black, FALSE);

	createPagePage0();
	guiShowPage(0);
}
static ActionEvent_Typedef 		*pSetVolumeAE 			= NULL;
static ActionEvent_Typedef 		*pTogglePauseAE			= NULL;
static ActionEvent_Typedef 		*pToggleMuteAE			= NULL;
static ActionEvent_Typedef 		*pToggleSleepAE			= NULL;
static ActionEvent_Typedef 		*pNextTrackAE			= NULL;
static ActionEvent_Typedef 		*pPrevTrackAE			= NULL;
static ActionEvent_Typedef 		*pRepositionToTrackAE	= NULL;

extern AudioPlayerDriverITF_Typedef	*pAudioPlayerDriverITF;
void updateLCDTFTStatus(void){
	int   forMinutes      = pAudioPlayerDriverITF->pAudioFileInfo->secondsRemaining / 60;
	int   remainingSec    = pAudioPlayerDriverITF->pAudioFileInfo->secondsRemaining % 60;
	char  buffer[120]	= {0};

	gwinSetText(gHandles[TRACK_LABEL_GH_NDX], pAudioPlayerDriverITF->pAudioFileInfo->filename, TRUE);
	chsnprintf(buffer, sizeof(buffer),"F:%d V:%d",pAudioPlayerDriverITF->pAudioFileInfo->frame,getCurrentVolume());
	gwinSetText(gHandles[STATUS_LABEL_GH_NDX], getAPStateName(pAudioPlayerDriverITF->state), TRUE);
	gwinSetText(gHandles[STATUS_GH_NDX], buffer, TRUE);
	chsnprintf(buffer, sizeof(buffer),"Tk:%d T:%d %d:%d ",pAudioPlayerDriverITF->pAudioFileInfo->trackID, ++pAudioPlayerDriverITF->pAudioFileInfo->totalSecondsPlayed, forMinutes,remainingSec);
	char *status = pAudioPlayerDriverITF->errorMsg != NULL?pAudioPlayerDriverITF->errorMsg:buffer;
	gwinSetText(gHandles[TRACK_GH_NDX], buffer, TRUE);

	return;
}
uint8_t changeVolume(GEvent  *pe){
	GEventGWinSlider *pSliderEvent = ((GEventGWinSlider*)pe);
	if ( pSliderEvent->position != getCurrentVolume())
		triggerActionEvent(pSetVolumeAE->name, NULL, (uint8_t)pSliderEvent->position, SOURCE_EVENT_LCD);

    return true;
}

void checkboxPause(bool check){
	bool checkedAlready = gwinCheckboxIsChecked(gHandles[TOOGLE_PAUSE_GH_NDX]);
	if ( (check && !checkedAlready) || (!check && checkedAlready))
		gwinCheckboxCheck(gHandles[TOOGLE_PAUSE_GH_NDX],check);
}
void checkboxMute(bool check){
	bool checkedAlready = gwinCheckboxIsChecked(gHandles[TOOGLE_MUTE_GH_NDX]);
	if ( (check && !checkedAlready) || (!check && checkedAlready))
		gwinCheckboxCheck(gHandles[TOOGLE_MUTE_GH_NDX],check);
}
void slideVolumeSet(uint8_t vol){
	gwinSliderSetPosition(gHandles[VOLUME_GH_NDX],vol);
}
uint8_t handleCheckboxClicks(GEvent  *pe){
	GEventGWinCheckbox *cbe = (GEventGWinCheckbox*)pe;

	if ( cbe->gwin == gHandles[TOOGLE_PAUSE_GH_NDX]){
		bool pausingRequested = cbe->isChecked;
		bool playerCurrentlyPausing = !IS_PLAYING(pAudioPlayerDriverITF);
		bool playerCurrentlyPlaying = IS_PLAYING(pAudioPlayerDriverITF);
		if ( (playerCurrentlyPlaying && pausingRequested) ||
			 (playerCurrentlyPausing && !pausingRequested))
			triggerActionEvent(pTogglePauseAE->name, NULL, cbe->isChecked?PAUSE_PLAYER_NOW:UNPAUSE_PLAYER_NOW, SOURCE_EVENT_LCD);
	}
	else
	if ( cbe->gwin == gHandles[TOOGLE_MUTE_GH_NDX]){
		bool muteRequested   = cbe->isChecked;
		bool playerCurrentlyPlaying = !IS_PLAYING_AND_MUTE(pAudioPlayerDriverITF);
		bool playerCurrentlyMute    = IS_PLAYING_AND_MUTE(pAudioPlayerDriverITF);
		//trigger AE only if we are playing and mute is requested; or we are already on unmute and unmute is requested
		if ( (playerCurrentlyPlaying && muteRequested) ||
			 (playerCurrentlyMute && !muteRequested))
			triggerActionEvent(pToggleMuteAE->name, NULL, cbe->isChecked?MUTE_PLAYER_NOW:UNMUTE_PLAYER_NOW, SOURCE_EVENT_LCD);
	}
	else
	if ( cbe->gwin == gHandles[TOOGLE_SLEEP_GH_NDX]){
		triggerActionEvent(pToggleSleepAE->name, NULL, true, SOURCE_EVENT_LCD);
	}
	return true;
}

uint8_t handleButtonClicks(GEvent  *pe){
	GEventGWinButton *be = (GEventGWinButton*)pe;

	if ( be->gwin == gHandles[NEXT_TRACK_GH_NDX])
		triggerActionEvent(pNextTrackAE->name, NULL, 0, SOURCE_EVENT_LCD);
	else
	if ( be->gwin == gHandles[PREV_TRACK_GH_NDX])
		triggerActionEvent(pPrevTrackAE->name, NULL, 0, SOURCE_EVENT_LCD);
	return true;
}
//returns none null if found, else return null to continue looking for a match
static LinkedListElement* populateTracksList(LinkedListElement* pItem, void *arg){(void)arg;
	AudioFileInfo_TypeDef *pAudioFileInfo = (AudioFileInfo_TypeDef*)pItem->value;

	char *titleToUse = (pAudioFileInfo->title[0] == 0)?pAudioFileInfo->filename:pAudioFileInfo->title;
	gwinListAddItemWithParam(gHandles[TRACKS_LIST_GH_NDX], titleToUse, TRUE,(uint32_t)pAudioFileInfo->trackKey);
	return NULL;
}
void onTrackStartedPlaying(void){
	//If the tracks dropdown list is empty, most likely it wasn't loaded properly at at startup
	if ( gwinListItemCount(gHandles[TRACKS_LIST_GH_NDX]) == 0 ){
		emlist_iterate_over(pAudioPlayerDriverITF->audioFileList,populateTracksList,NULL);
	}

	for(int item=0; item < gwinListItemCount(gHandles[TRACKS_LIST_GH_NDX]); ++item){
		char *trackKey   = (char*)gwinListItemGetParam(gHandles[TRACKS_LIST_GH_NDX],item);
		if ( trackKey == pAudioPlayerDriverITF->pAudioFileInfo->trackKey){
			gwinListSetSelected(gHandles[TRACKS_LIST_GH_NDX],item,true);
			gwinListViewItem(gHandles[TRACKS_LIST_GH_NDX],item);
			return;
		}
	}
	return;
}

uint8_t trackSelected(GEvent  *pe){
	GEventGWinList  *pTrackListEvent=(GEventGWinList*)pe;

	char *trackKey   = (char*)gwinListItemGetParam(gHandles[TRACKS_LIST_GH_NDX],pTrackListEvent->item);
	if ( trackKey != NULL )
		triggerActionEvent(pRepositionToTrackAE->name, trackKey, 0, SOURCE_EVENT_LCD);

    return true;
}
void turnOffLCD(void){
	#ifdef BOARD_ST_STM32F769I_DISCOVERY
	BSP_LCD_Reset();
	#endif
	#ifdef BOARD_ST_STM32F746G_DISCOVERY
	//It seems that R85 has to be removed to turn off screen
	//https://ask.embedded-wizard.de/3857/white-screen-flashing-at-the-switch-on
	//As is we get a white screen for some reason
	gdispClear(Black);
	palClearPad(GPIOK, GPIOK_LCD_BL_CTRL);
    palClearPad(GPIOI, GPIOI_LCD_DISP);
    #endif

}
void inituGFXGUI(void *arg){(void)arg;
	gfxInit();

	gdispClear(Black);
	guiCreate();


	pSetVolumeAE   		= getActionEvent(SET_VOLUME_AE_NAME);
	pTogglePauseAE 		= getActionEvent(TOGGLE_PAUSE_AE_NAME);
	pToggleMuteAE  		= getActionEvent(TOGGLE_MUTE_AE_NAME);
	pToggleSleepAE  	= getActionEvent(GO_TO_SLEEP_AE_NAME);
	pNextTrackAE   		= getActionEvent(NEXT_TRACK_AE_NAME);
	pPrevTrackAE   		= getActionEvent(PREV_TRACK_AE_NAME);
	pRepositionToTrackAE= getActionEvent(REPOSITION_TO_TRACK_AE_NAME);

	emlist_iterate_over(pAudioPlayerDriverITF->audioFileList,populateTracksList,NULL);
	gHandles[VOLUME_GH_NDX]->onEventCB 			= changeVolume;
	gHandles[TOOGLE_PAUSE_GH_NDX]->onEventCB 	= handleCheckboxClicks;
	gHandles[TOOGLE_MUTE_GH_NDX]->onEventCB 	= handleCheckboxClicks;
	gHandles[TOOGLE_SLEEP_GH_NDX]->onEventCB 	= handleCheckboxClicks;
	gHandles[NEXT_TRACK_GH_NDX]->onEventCB 		= handleButtonClicks;
	gHandles[PREV_TRACK_GH_NDX]->onEventCB 		= handleButtonClicks;
	gHandles[TRACKS_LIST_GH_NDX]->onEventCB 	= trackSelected;

	slideVolumeSet(getCurrentVolume());
}
#endif
