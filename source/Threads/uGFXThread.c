/*
 * LCDThread.c
 *
 *  Created on: May 4, 2020
 *      Author: abusous2000
 */
#include "Strust4EmbeddedConf.h"
#if USE_LCD_TFT
#include "ch.h"
#include "hal.h"
#include "CommonHelper.h"
#include "gfx.h"
#include "ccportab.h"

CC_WEAK void inituGFXGUI(void *arg){(void)arg;
}
CC_WEAK void defaultuGFXHandler(GEvent 	*pe){(void)pe;
}
/*
 * If you are short on memory in RAM3 section, uncomment the below line
 */
PLACE_IN_RAM_SECTION(.ram0.wauGFXThd)
static THD_WORKING_AREA(wauGFXThd, LCD_THD_STACK_SIZE);
static THD_FUNCTION(uGFXThd, arg) {
  (void)arg;
    GListener glistener;
	geventListenerInit(&glistener);
	gwinAttachListener(&glistener);
	while (true) {
	    GEvent 						*pe = geventEventWait(&glistener, 0);
	    bool                        evantHandled = false;
	    switch (pe->type) {
		    case GEVENT_GWIN_LIST:{
					GEventGWinList  *pTrackListEvent=(GEventGWinList*)pe;
					if ( pTrackListEvent->gwin->onEventCB != NULL )
						evantHandled = pTrackListEvent->gwin->onEventCB(pe);
		        }
		    	break;
			case GEVENT_GWIN_SLIDER:{
					GEventGWinSlider  *pSliderEvent = ((GEventGWinSlider*)pe);
					if ( pSliderEvent->gwin->onEventCB != NULL )
						evantHandled = pSliderEvent->gwin->onEventCB(pe);
				}
				break;
			case GEVENT_GWIN_BUTTON:{
					GEventGWinButton  *be = (GEventGWinButton*)pe;
					if ( be->gwin->onEventCB != NULL )
						evantHandled = be->gwin->onEventCB(pe);
				}
				break;
			case GEVENT_GWIN_CHECKBOX:{
					GEventGWinCheckbox *cb = (GEventGWinCheckbox*)pe;
					if (cb->gwin->onEventCB != NULL)
						evantHandled = cb->gwin->onEventCB(pe);
			    }
				break;

			default:
			  break;
	    }
	    if ( !evantHandled)
	    	defaultuGFXHandler(pe);
	    chThdSleepMilliseconds(1);
  }
}

void inituGFXThd(void){
	inituGFXGUI(NULL);
    chThdCreateStatic(wauGFXThd,  sizeof(wauGFXThd),  NORMALPRIO+2,     uGFXThd, NULL);
}

#endif


