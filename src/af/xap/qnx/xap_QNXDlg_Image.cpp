/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <stdlib.h>
#include <stdio.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_QNXDlg_Image.h"
#include "ut_qnxHelper.h"


/*****************************************************************/

XAP_Dialog * XAP_QNXDialog_Image::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_QNXDialog_Image * p = new XAP_QNXDialog_Image(pFactory,id);
	return p;
}

XAP_QNXDialog_Image::XAP_QNXDialog_Image(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: XAP_Dialog_Image(pDlgFactory,id)
{
}

XAP_QNXDialog_Image::~XAP_QNXDialog_Image(void)
{
}

void XAP_QNXDialog_Image::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	PtWidget_t *mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);
	connectFocus(mainWindow,pFrame);
	
   // To center the dialog, we need the frame of its parent.
    XAP_QNXFrame * pQNXFrame = static_cast<XAP_QNXFrame *>(pFrame);
    UT_ASSERT(pQNXFrame);
    
    // Get the Window of the parent frame
    PtWidget_t * parentWindow = pQNXFrame->getTopLevelWindow();
    UT_ASSERT(parentWindow);
    
    // Center our new dialog in its parent and make it a transient
    // so it won't get lost underneath
    // Make it modal, and stick it up top
	UT_QNXCenterWindow(parentWindow, mainWindow);
	UT_QNXBlockWidget(parentWindow, 1);

    // Show the top level dialog,
	PtRealizeWidget(mainWindow);

    // Run the event loop for this window.
	int count;
	count = PtModalStart();
	done=0;
	do {
    		PtProcessEvent();
	} while (!done);

	PtModalEnd(MODAL_END_ARG(count));
	UT_QNXBlockWidget(parentWindow,0);
	PtDestroyWidget(mainWindow);

}

int ph_event_toggle(PtWidget_t *w,void *data,PtCallbackInfo_t *cb)
{
XAP_QNXDialog_Image *dlg=(XAP_QNXDialog_Image*)data;
dlg->event_AspectCheckbox();
return Pt_CONTINUE;
}

void XAP_QNXDialog_Image::event_AspectCheckbox()
{

if(PtWidgetFlags(m_aspectToggle) & Pt_SET)
{
setPreserveAspect(true);
}else
{
setPreserveAspect(false);
}
}


int ph_event_ok(PtWidget_t *w,void *data,PtCallbackInfo_t *cb)
{
XAP_QNXDialog_Image *dlg=(XAP_QNXDialog_Image*)data;
dlg->event_OK();
return Pt_CONTINUE;
}

void XAP_QNXDialog_Image::event_OK()
{
event_AspectCheckbox();
//SetWidth();
//SetHeight();
setAnswer(XAP_Dialog_Image::a_OK);
done=1;
}

int ph_event_cancel(PtWidget_t *w,void *data,PtCallbackInfo_t *cb)
{
XAP_QNXDialog_Image *dlg=(XAP_QNXDialog_Image*)data;

dlg->event_Cancel();
return Pt_CONTINUE;
}

void XAP_QNXDialog_Image::event_Cancel()
{
setAnswer(XAP_Dialog_Image::a_Cancel);
done=1;
}

int ph_event_numeric_change(PtWidget_t *w,void *data,PtCallbackInfo_t *cb)
{
XAP_QNXDialog_Image *dlg= (XAP_QNXDialog_Image*)data;
dlg->event_Numeric(w);
return Pt_CONTINUE;
}

void XAP_QNXDialog_Image::event_Numeric(PtWidget_t *w)
{
float *value;
float val=0;

PtGetResource(w,Pt_ARG_NUMERIC_VALUE,&value,0);

if(w == m_width)
{
setWidth(*value);
if(PtWidgetFlags(m_aspectToggle) & Pt_SET)
{
	val=UT_convertToInches(getHeightString());
	PtSetResource(m_height,Pt_ARG_NUMERIC_VALUE,&val,0);

}
}
else if( w == m_height)
{
setHeight(*value);
if(PtWidgetFlags(m_aspectToggle) & Pt_SET)
{	
	val=UT_convertToInches(getWidthString());
	PtSetResource(m_width,Pt_ARG_NUMERIC_VALUE,&val,0);
}
}

}


PtWidget_t *XAP_QNXDialog_Image::_constructWindow()
{
  const XAP_StringSet * pSS = m_pApp->getStringSet();
	PtWidget_t *mainwindow;
	PtWidget_t *btnOK;
	PtWidget_t *btnCancel;
	PtWidget_t *width;
	PtWidget_t *height;
	PtWidget_t *aspectToggle;	
	char buff[30];
		const PtArg_t args[] = {
		Pt_ARG(Pt_ARG_WINDOW_TITLE,pSS->getValue(XAP_STRING_ID_DLG_Image_Title),0),
		Pt_ARG(Pt_ARG_WINDOW_RENDER_FLAGS,0,ABI_MODAL_WINDOW_RENDER_FLAGS),
		Pt_ARG(Pt_ARG_WINDOW_MANAGED_FLAGS,0,ABI_MODAL_WINDOW_MANAGE_FLAGS)
  	};

	 const PhArea_t area1 = { { 10, 11 }, { 80, 21 } };
	 const PtArg_t args1[] = {
		Pt_ARG( Pt_ARG_AREA, &area1, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING,pSS->getValue(XAP_STRING_ID_DLG_Image_Height), 0 ),
		};

	 const PhArea_t area2 = { { 10, 44 }, { 95, 21 } };
	 const PtArg_t args2[] = {
		Pt_ARG( Pt_ARG_AREA, &area2, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING,pSS->getValue(XAP_STRING_ID_DLG_Image_Width), 0 ),
		};

	 const PhArea_t area3 = { { 10, 77 }, { 205, 24 } };
	 const PtArg_t args3[] = {
		Pt_ARG( Pt_ARG_AREA, &area3, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING,pSS->getValue(XAP_STRING_ID_DLG_Image_Aspect), 0 ),
		};

	 const PhArea_t area4 = { { 102, 5 }, { 113, 27 } };
	 const PtArg_t args4[] = {
		Pt_ARG( Pt_ARG_AREA, &area4, 0 ),
		};

	 const PhArea_t area5 = { { 102, 38 }, { 113, 27 } };
	 const PtArg_t args5[] = {
		Pt_ARG( Pt_ARG_AREA, &area5, 0 ),
		};

	 const PhArea_t area6 = { { 168, 108 }, { 50, 27 } };
	 const PtArg_t args6[] = {
		Pt_ARG( Pt_ARG_AREA, &area6, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_OK), 0 ),
		};

	 const PhArea_t area7 = { { 111, 108 }, { 52, 27 } };
	 const PtArg_t args7[] = {
		Pt_ARG( Pt_ARG_AREA, &area7, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_Cancel), 0 ),
		};

	mainwindow=PtCreateWidget(PtWindow,NULL,sizeof(args) / sizeof(PtArg_t),args);
		PtAddCallback(mainwindow,Pt_CB_WINDOW_CLOSING,ph_event_cancel,this);

	PtCreateWidget( PtLabel, NULL, sizeof(args1) / sizeof(PtArg_t), args1 );

	PtCreateWidget( PtLabel, NULL, sizeof(args2) / sizeof(PtArg_t), args2 );

	aspectToggle=PtCreateWidget( PtToggleButton, NULL, sizeof(args3) / sizeof(PtArg_t), args3 );
		PtAddCallback(aspectToggle,Pt_CB_ACTIVATE,ph_event_toggle,this);

	width=PtCreateWidget( PtNumericFloat, NULL, sizeof(args4) / sizeof(PtArg_t), args4 );
		PtAddCallback(width,Pt_CB_NUMERIC_CHANGED,ph_event_numeric_change,this);

	height=PtCreateWidget( PtNumericFloat, NULL, sizeof(args5) / sizeof(PtArg_t), args5 );
		PtAddCallback(height,Pt_CB_NUMERIC_CHANGED,ph_event_numeric_change,this);

	btnOK=PtCreateWidget( PtButton, NULL, sizeof(args6) / sizeof(PtArg_t), args6 );
		PtAddCallback(btnOK,Pt_CB_ACTIVATE,ph_event_ok,this);

	btnCancel=PtCreateWidget( PtButton, NULL, sizeof(args7) / sizeof(PtArg_t), args7 );
		PtAddCallback(btnCancel,Pt_CB_ACTIVATE,ph_event_cancel,this);

if(getPreserveAspect()) {
PtSetResource(aspectToggle,Pt_ARG_FLAGS,Pt_TRUE,Pt_SET);
} else 
	PtSetResource(aspectToggle,Pt_ARG_FLAGS,Pt_FALSE,Pt_SET);
double val=UT_convertToInches(getHeightString());
PtSetResource(height,Pt_ARG_NUMERIC_VALUE,&val,0);

val=UT_convertToInches(getWidthString());
PtSetResource(width,Pt_ARG_NUMERIC_VALUE,&val,0);

m_width=width;
m_height=height;
m_aspectToggle=aspectToggle;

return mainwindow;
}
