/* 
 QNX Photon Helper functions 
*/

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_misc.h"
#include "xav_View.h"
#include "xap_Frame.h"
#include "xap_App.h"
#include "xap_Args.h"
#include "ut_qnxHelper.h"
#include "Ap.h"


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
	PtExtentWidgetFamily(widget);
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
	unsigned short *index = NULL;
	int      ret;

	UT_ASSERT(widget);
	PtGetResource(widget, Pt_ARG_CBOX_SEL_ITEM, &index, 0);
	ret = (index) ? *index : 0;

	return ret;
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

int pretty_group(PtWidget_t *w, const char *title) {
	int n, width;
	PtArg_t args[10];

	n = 0;
	width = 0;

	if (title && *title) {
		const char *font = NULL;
		char *defaultfont = { "helv10" };

		PtGetResource(w, Pt_ARG_TITLE_FONT, &font, 0);
		if (!font) {
			font = defaultfont;	
		}

		PtSetArg(&args[n++], Pt_ARG_TITLE, title, 0);
		PtSetArg(&args[n++], Pt_ARG_CONTAINER_FLAGS, 
			Pt_SHOW_TITLE | Pt_ETCH_TITLE_AREA, 
			Pt_SHOW_TITLE | Pt_ETCH_TITLE_AREA);
	}
#define OUTLINE_GROUP (Pt_TOP_OUTLINE | Pt_TOP_BEVEL | \
					   Pt_BOTTOM_OUTLINE | Pt_BOTTOM_BEVEL | \
					   Pt_LEFT_OUTLINE | Pt_LEFT_BEVEL | \
					   Pt_RIGHT_OUTLINE | Pt_RIGHT_BEVEL)
	PtSetArg(&args[n++], Pt_ARG_BASIC_FLAGS, OUTLINE_GROUP, OUTLINE_GROUP);
	PtSetArg(&args[n++], Pt_ARG_BEVEL_WIDTH, 1, 0);
	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_HIGHLIGHTED, Pt_HIGHLIGHTED);
	PtSetArg(&args[n++], Pt_ARG_RESIZE_FLAGS, Pt_RESIZE_XY_AS_REQUIRED, 
											  Pt_RESIZE_XY_AS_REQUIRED | Pt_RESIZE_XY_ALWAYS);

	PtSetResources(w, n, args);

	return 0;
}

int OpenHelp(PtWidget_t *w,XAP_Dialog *dlg,PtCallbackInfo_t *cbinfo)
{
PhWindowEvent_t *we = (PhWindowEvent_t *)cbinfo->cbdata;
if(we->event_f == Ph_WM_HELP) {
	if(dlg->getHelpUrl().size() > 0)
	{
		char url[PATH_MAX] = "/usr/help/product/AbiWord/en-US/";
		sprintf(url,"%s%s.html",url,dlg->getHelpUrl().c_str());
		PtHelpUrl(url);
	}
}
return 0;
}

int SetupContextHelp(PtWidget_t *w,void *dlg)
{
	PtAddCallback(w,Pt_CB_WINDOW,OpenHelp,dlg);
	PtSetResource(w,Pt_ARG_WINDOW_RENDER_FLAGS,Pt_TRUE,Ph_WM_RENDER_HELP);
	PtSetResource(w,Pt_ARG_WINDOW_NOTIFY_FLAGS,Pt_TRUE,Ph_WM_HELP);
	return 0;
}

PtWidget_t *abiCreatePhabDialog(char *dialog,const XAP_StringSet *pSS, XAP_String_Id id)
{
ApDBase_t *db=NULL;
int n=0;
PtArg_t args[5];
char path[PATH_MAX];
PtWidget_t *window;
UT_UTF8String s;

sprintf(path,"%s/%s.wgtd",XAP_App::getApp()->getAbiSuiteAppDir(),dialog);

db = ApOpenDBaseFile(path);
if(!db) //Perfectly normal, use the dialog that's inside the binary
{
sprintf(path,"%s.wgtd",dialog);
db = ApOpenExecDBaseFile(XAP_App::getApp()->getArgs()->m_argv[0],path);
}
if(!db) return (PtWidget_t*)-1;

pSS->getValueUTF8(id,s);
PtSetArg(&args[n++],Pt_ARG_WINDOW_TITLE,s.utf8_str(),0);

window = ApCreateWidgetFamily(db,dialog,-1,-1,n,args);
ApCloseDBase(db);
return window;
}

PtWidget_t *abiPhabLocateWidget(PtWidget_t *parent,char *name)
{
PtWidget_t *cur;
cur =parent;
int d=0;
char *str;

while( (d = PtWidgetTree(parent,&cur,d) ) != Pt_TRAVERSE_DONE)
{
	PtGetResource(cur,Pt_ARG_USER_DATA,&str,0);
	if(str && strcmp(str,name)==0)
	{
		return cur;
	}
}

return (PtWidget_t *)-1;
}

void localizeLabel(PtWidget_t * widget, const XAP_StringSet * pSS, XAP_String_Id id)
{
	gchar * unixstr = NULL;	// used for conversions
	UT_UTF8String s;
	pSS->getValueUTF8(id,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	PtSetResource(widget,Pt_ARG_TEXT_STRING,unixstr,0);
	FREEP(unixstr);	
}


