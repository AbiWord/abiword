/* 
 QNX Photon Helper functions 
*/

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_misc.h"
#include"xav_View.h"
#include"xap_Frame.h"
#include"xap_App.h"
#include "ut_qnxHelper.h"


#include <stdio.h>


int  UT_QNXCenterWindow(PtWidget_t *parent, PtWidget_t *widget) {
	PtArg_t  args[2];
	PhRect_t rect;
	PhPoint_t pos;

	UT_ASSERT(widget);

/* TODO: Change the code so centering on a parent works */
	if (1 || !parent) {		//Center on the screen
#if 0
		PgHWCaps_t 			caps;
		PgVideoModeInfo_t 	mode;

		if (PgGetGraphicsHWCaps(&caps) == -1) {
			return -1;
		}

		if (PgGetVideoModeInfo(caps.current_video_mode, &mode) == -1) {
			return -2;
		}

		rect.ul.x =
		rect.ul.y = 0;

		rect.lr.x = mode.width;
		rect.lr.y = mode.height;
#else
		PhWindowQueryVisible(Ph_QUERY_GRAPHICS, 0, 1, &rect);
#endif
	}
	else {		// Center on the widget
		PhArea_t *parea;

		//Assume that the parent is visible already.
		//PtExtentWidget(widget);		//Force a re-calculation of size
		PtSetArg(&args[0], Pt_ARG_AREA, &parea, 0);
		PtGetResources(parent, 1, args);

		rect.ul.x = 
		rect.lr.x = parea->pos.x;
		rect.ul.y = 
		rect.lr.y = parea->pos.y;

		rect.lr.x += parea->size.w;
		rect.lr.y += parea->size.h;
	}

	// Now position the widget ...
	PtExtentWidget(widget);		//Force a re-calculation of size
	PtSetArg(&args[0], Pt_ARG_WIDTH, 0, 0);
	PtSetArg(&args[1], Pt_ARG_HEIGHT, 0, 0);
	PtGetResources(widget, 2, args);

/*
	printf("Centering on window %d,%d - %d,%d Widget %d/%d \n",
		rect.ul.x, rect.ul.y, rect.lr.x, rect.lr.y, args[0].value, args[1].value);
*/
	pos.x = rect.ul.x;
	pos.y = rect.ul.y;

	pos.x += (rect.lr.x - rect.ul.x - args[0].value) / 2;
	pos.y += (rect.lr.y - rect.ul.y - args[1].value) / 2;
//	printf("Final position %d,%d \n", pos.x, pos.y); 
	PtSetArg(&args[0], Pt_ARG_POS, &pos, 0);
	PtSetResources(widget, 1, args);

	return 0;
}

int UT_QNXGetWidgetArea(PtWidget_t *widget, short *x, short *y, unsigned short *w, unsigned short *h) {
	PhArea_t *parea;
	PtArg_t  arg;

	UT_ASSERT(widget);
	PtExtentWidget(widget);
	PtSetArg(&arg, Pt_ARG_AREA, &parea, 0);
	PtGetResources(widget, 1, &arg);

/*
	printf("Got area %d,%d %d/%d \n",
		parea->pos.x, parea->pos.y, parea->size.w, parea->size.h);
*/

	if (x) *x = parea->pos.x;
	if (y) *y = parea->pos.y;
	if (w) *w = parea->size.w;
	if (h) *h = parea->size.h;

	return 0;
}

int  UT_QNXBlockWidget(PtWidget_t *widget, int block) {
	PtArg_t  arg;

	UT_ASSERT(widget);

	PtSetArg(&arg, Pt_ARG_FLAGS, (block) ? Pt_BLOCKED : 0, Pt_BLOCKED);
	PtSetResources(widget, 1, &arg);

	return 0;
}

int  UT_QNXComboSetPos(PtWidget_t *widget, int index) {
	PtArg_t  arg;

	UT_ASSERT(widget);

	PtSetArg(&arg, Pt_ARG_CBOX_SEL_ITEM, index, 0);
	PtSetResources(widget, 1, &arg);

	return 0;
}

int  UT_QNXComboGetPos(PtWidget_t *widget) {
	PtArg_t  arg;
	int      *index = NULL;

	UT_ASSERT(widget);

	PtSetArg(&arg, Pt_ARG_CBOX_SEL_ITEM, &index, 0);
	PtGetResources(widget, 1, &arg);

	return (index) ? *index : 0;
}

/**** All for the modal/modeless dialogs (from unix) ****/

static int focus_in_event(PtWidget_t *w, void *data, PtCallbackInfo_t *info) {
      XAP_Frame *pFrame=(XAP_Frame *)data;
      UT_ASSERT(pFrame);
      pFrame->getCurrentView()->focusChange(AV_FOCUS_NEARBY);

      return Pt_CONTINUE;
}

static int focus_out_event(PtWidget_t *w, void *data, PtCallbackInfo_t *info) {
      XAP_Frame *pFrame=(XAP_Frame *)data;
      if(pFrame == NULL) return Pt_CONTINUE;
      AV_View * pView = pFrame->getCurrentView();
      if(pView != NULL) {
	     pView->focusChange(AV_FOCUS_NONE);
      }

      return Pt_CONTINUE;
}

static int focus_out_event_Modeless(PtWidget_t *w, void *data, PtCallbackInfo_t *info) {
	XAP_App *pApp = (XAP_App *)data;
	XAP_Frame *pFrame = pApp->getLastFocussedFrame();
	if(pFrame == NULL) {
		UT_uint32 nframes =  pApp->getFrameCount();
		if(nframes > 0 && nframes < 10) {     
			pFrame = pApp->getFrame(0);
		}
       	else {
			return Pt_CONTINUE;
		}
	}
	if(pFrame == NULL) return Pt_CONTINUE;
	AV_View * pView = pFrame->getCurrentView();

	UT_ASSERT(pView);
	if(pView!= NULL) {
		pView->focusChange(AV_FOCUS_NONE);
	}
	return Pt_CONTINUE;
}


static int focus_in_event_Modeless(PtWidget_t *w, void *data, PtCallbackInfo_t *info) {
	XAP_App *pApp=(XAP_App *)data;
	XAP_Frame *pFrame= pApp->getLastFocussedFrame();

	if(pFrame == NULL) {
		UT_uint32 nframes =  pApp->getFrameCount();
		if(nframes > 0 && nframes < 10) {     
			pFrame = pApp->getFrame(0);
		}
		else {
			return Pt_CONTINUE;
		}
	}
	if(pFrame == NULL) return Pt_CONTINUE;
	AV_View * pView = pFrame->getCurrentView();
	if(pView!= NULL) {
		pView->focusChange(AV_FOCUS_MODELESS);
	}
	return Pt_CONTINUE;
}

void connectFocus(PtWidget_t *widget, XAP_Frame *frame)
{
	PtAddCallback(widget, Pt_CB_GOT_FOCUS, focus_in_event, frame);
	PtAddCallback(widget, Pt_CB_LOST_FOCUS, focus_out_event, frame);
	//???
	PtAddCallback(widget, Pt_CB_DESTROYED, focus_out_event, frame);
}

void connectFocusModeless(PtWidget_t *widget, XAP_App * pApp)
{
	PtAddCallback(widget, Pt_CB_GOT_FOCUS, focus_in_event_Modeless, pApp);
	PtAddCallback(widget, Pt_CB_LOST_FOCUS, focus_out_event_Modeless, pApp);
	//???
	PtAddCallback(widget, Pt_CB_DESTROYED, focus_out_event_Modeless, pApp);
}


