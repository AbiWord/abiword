/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
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
#include <string.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_QNXApp.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_About.h"
#include "xap_QNXDlg_About.h"
#include "xap_Frame.h"
#include "xap_QNXFrameImpl.h"

#include "gr_QNXGraphics.h"
#include "gr_QNXImage.h"
#include "ut_bytebuf.h"
#include "ut_png.h"
#include "ut_qnxHelper.h"

#include <stdio.h>

/*****************************************************************/

extern unsigned char g_pngSidebar[];		// see ap_wp_sidebar.cpp
extern unsigned long g_pngSidebar_sizeof;	// see ap_wp_sidebar.cpp

/*****************************************************************/

XAP_Dialog * XAP_QNXDialog_About::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_QNXDialog_About * p = new XAP_QNXDialog_About(pFactory,id);
	return p;
}

XAP_QNXDialog_About::XAP_QNXDialog_About(XAP_DialogFactory * pDlgFactory,
											 XAP_Dialog_Id id)
	: XAP_Dialog_About(pDlgFactory,id)
{
	m_windowMain = NULL;
	m_buttonOK = NULL;
	m_buttonURL = NULL;
	m_drawingareaGraphic = NULL;
	m_gc = NULL;
	m_pGrImageSidebar = NULL;
}

XAP_QNXDialog_About::~XAP_QNXDialog_About(void)
{
	DELETEP(m_gc);
	DELETEP(m_pGrImageSidebar);
}

/*****************************************************************/
static int s_ok_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	UT_ASSERT(widget && data);
	XAP_QNXDialog_About * dlg = (XAP_QNXDialog_About *)data;

	dlg->event_OK();
	return Pt_CONTINUE;
}

static int s_url_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	UT_ASSERT(widget && data);
	XAP_QNXDialog_About * dlg = (XAP_QNXDialog_About *)data;

	dlg->event_URL();
	return Pt_CONTINUE;
}

static int s_delete_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	UT_ASSERT(data);
	XAP_QNXDialog_About * dlg = (XAP_QNXDialog_About *)data;

	dlg->event_WindowDelete();
	return Pt_CONTINUE;
}

static int s_drawingarea_expose(PtWidget_t * w, PhTile_t * damage)
{
	PtArg_t args[1];
	PhRect_t raw_canvas;

	PtSuperClassDraw(PtBasic, w, damage);
	PtCalcCanvas(w, &raw_canvas);
	PtClipAdd(w, &raw_canvas);

	XAP_QNXDialog_About *pQNXAbout;
    PtGetResource(w, Pt_ARG_POINTER, &pQNXAbout,0);

    UT_ASSERT(pQNXAbout);
	pQNXAbout->event_DrawingAreaExpose();

	PtClipRemove();
	return Pt_CONTINUE;
}
/*
static int s_drawingarea_expose(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	UT_ASSERT(data);
	XAP_QNXDialog_About * dlg = (XAP_QNXDialog_About *)data;

	dlg->event_DrawingAreaExpose();
	return Pt_CONTINUE;
}
*/

/*****************************************************************/

void XAP_QNXDialog_About::runModal(XAP_Frame * pFrame)
{

	XAP_QNXFrameImpl * pQNXFrameImpl = (XAP_QNXFrameImpl*)pFrame->getFrameImpl();
	PtWidget_t *parentWindow =	pQNXFrameImpl->getTopLevelWindow();	
	UT_ASSERT(parentWindow);

	PtSetParentWidget(parentWindow);
	// Build the window's widgets and arrange them
	PtWidget_t * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// assemble an image
	_preparePicture();
	

	// attach a new graphics context
	GR_QNXAllocInfo ai(mainWindow,m_drawingareaGraphic,pFrame->getApp());
	m_gc = (GR_QNXGraphics *)XAP_App::getApp()->newGraphics(ai);


	UT_QNXCenterWindow(/*parentWindow | */ NULL, mainWindow);
	UT_QNXBlockWidget(parentWindow, 1);
	PtRealizeWidget(mainWindow);

	int count;
	count = PtModalStart();
	done = 0;
	while(!done) {
		PtProcessEvent();
	}
	PtModalEnd(MODAL_END_ARG(count));

	UT_QNXBlockWidget(parentWindow, 0);
	PtDestroyWidget(mainWindow);
}

void XAP_QNXDialog_About::event_OK(void)
{
	done = 1;
}

void XAP_QNXDialog_About::event_URL(void)
{

}

void XAP_QNXDialog_About::event_WindowDelete(void)
{
	done = 1;
}

void XAP_QNXDialog_About::event_DrawingAreaExpose(void) {
	if (!m_gc)
		return;

	m_gc->drawImage(m_pGrImageSidebar, 0, 0);
}

/*****************************************************************/
PtWidget_t * XAP_QNXDialog_About::_constructWindow(void)
{
	PtWidget_t *windowAbout;
	PtWidget_t *drawingareaGraphic;
	PtWidget_t *buttonURL;
	PtWidget_t *buttonOK;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	
	// we use this for all sorts of strings that can't appear in the string sets
	char buf[4096];
	snprintf(buf, 4096, XAP_ABOUT_TITLE, m_pApp->getApplicationName());

	windowAbout = abiCreatePhabDialog("xap_QNXDlg_About",pSS,XAP_STRING_ID_DLG_OK); 
	PtSetResource(windowAbout,Pt_ARG_WINDOW_TITLE,buf,0);
	SetupContextHelp(windowAbout,this);
	PtAddCallback(windowAbout, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);
	PtAddHotkeyHandler(windowAbout,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);

	drawingareaGraphic = abiPhabLocateWidget(windowAbout,"rawGraphic"); 
	PtSetResource(drawingareaGraphic, Pt_ARG_POINTER, this,0);
	PtSetResource(drawingareaGraphic, Pt_ARG_RAW_DRAW_F,  &s_drawingarea_expose, 1);

	PtSetResource(abiPhabLocateWidget(windowAbout,"lblAppName"), Pt_ARG_TEXT_STRING, m_pApp->getApplicationName(), 0);

	snprintf(buf, 4096, XAP_ABOUT_VERSION, XAP_App::s_szBuild_Version);
	PtSetResource(abiPhabLocateWidget(windowAbout,"lblVersion"), Pt_ARG_TEXT_STRING, buf, 0);
	
	char buf2[4096];
	snprintf(buf2, 4096, XAP_ABOUT_GPL_LONG_LINE_BROKEN, m_pApp->getApplicationName());
	snprintf(buf, 4096, "%s\n\n%s", XAP_ABOUT_COPYRIGHT, buf2);
	
	PtSetResource(abiPhabLocateWidget(windowAbout,"multiLicense"), Pt_ARG_TEXT_STRING, buf, 0);

	buttonURL = abiPhabLocateWidget(windowAbout,"btnUrl"); 
	PtSetResource(buttonURL, Pt_ARG_TEXT_STRING, "www.abisource.com", 0);
	PtAddCallback(buttonURL, Pt_CB_ACTIVATE, s_url_clicked, this);

	buttonOK = abiPhabLocateWidget(windowAbout,"btnOK"); 
	localizeLabel(buttonOK, pSS, XAP_STRING_ID_DLG_OK);
	PtAddCallback(buttonOK, Pt_CB_ACTIVATE, s_ok_clicked, this);

	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_windowMain = windowAbout;
	m_buttonOK = buttonOK;
	m_buttonURL = buttonURL;
	m_drawingareaGraphic = drawingareaGraphic;

	return windowAbout;
}

void XAP_QNXDialog_About::_preparePicture(void)
{
	UT_ByteBuf * pBB = new UT_ByteBuf(g_pngSidebar_sizeof);
	pBB->ins(0,g_pngSidebar,g_pngSidebar_sizeof);

	UT_sint32 iImageWidth;
	UT_sint32 iImageHeight;
		
	UT_PNG_getDimensions(pBB, iImageWidth, iImageHeight);
	
	m_pGrImageSidebar = new GR_QNXImage(NULL);
	m_pGrImageSidebar->convertFromBuffer(pBB, iImageWidth, iImageHeight);

	DELETEP(pBB);
}
