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
	PhPoint_t pos;

	UT_ASSERT(widget);

	//Force a re-calculation of size
	PtExtentWidgetFamily(widget);	
	PtCalcAbsPosition(parent, NULL, PtWidgetDim(widget, NULL), &pos);

	PtSetResource(widget, Pt_ARG_POS, &pos, 0);

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


