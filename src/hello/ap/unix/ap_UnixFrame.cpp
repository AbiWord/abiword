/* AbiHello
 * Copyright (C) 1999 AbiSource, Inc.
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

#include <gtk/gtk.h>

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"

#include "ap_FrameData.h"
#include "xap_UnixFrame.h"

#include "ev_UnixToolbar.h"
#include "ap_Prefs_SchemeIds.h"
#include "xav_View.h"
#include "gr_UnixGraphics.h"
#include "ap_UnixFrame.h"
#include "xap_UnixApp.h"
#include "xap_UnixFontManager.h"
#include "ap_View.h"
#include "ap_UnixStatusBar.h"

#include "gr_UnixGraphics.h"

#define ENSUREP(p)      do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

AP_UnixFrame::AP_UnixFrame(XAP_UnixApp* app)
        : XAP_UnixFrame(app)
{
	m_pUnixStatusBar = NULL;
}

AP_UnixFrame::AP_UnixFrame(AP_UnixFrame* f)
	: XAP_UnixFrame(static_cast<XAP_UnixFrame *>(f))
{

}

AP_UnixFrame::~AP_UnixFrame(void)
{
	killFrameData();
}

bool AP_UnixFrame::initialize(void)
{
	if (!initFrameData())
		return false;

	if (!XAP_UnixFrame::initialize(AP_PREF_KEY_KeyBindings,AP_PREF_DEFAULT_KeyBindings,
								   AP_PREF_KEY_MenuLayout, AP_PREF_DEFAULT_MenuLayout,
								   AP_PREF_KEY_MenuLabelSet, AP_PREF_DEFAULT_MenuLabelSet,
								   AP_PREF_KEY_ToolbarLayouts, AP_PREF_DEFAULT_ToolbarLayouts,
								   AP_PREF_KEY_ToolbarLabelSet, AP_PREF_DEFAULT_ToolbarLabelSet))
	{
		return false;
	}

	_createTopLevelWindow();
	gtk_widget_show(m_wTopLevelWindow);

	loadDocument(NULL, 0);
	
	return true;
}

UT_Error AP_UnixFrame::loadDocument(const char * szFilename, int fileType)
{
	XAP_UnixFontManager * fontManager = ((XAP_UnixApp *) getApp())->getFontManager();

	GR_UnixGraphics* pG = new GR_UnixGraphics(m_dArea->window, fontManager, getApp());
	
	pG->setFont(pG->findFont("times", "normal", NULL, "bold", NULL, "72pt"));
				
	pG->setZoomPercentage(100);

	REPLACEP(((AP_FrameData*)m_pData)->m_pG, pG);

	AP_View* pView = new AP_View(m_app, pG, this);
	REPLACEP(m_pView, pView);

	if (m_pUnixStatusBar)
		m_pUnixStatusBar->setView(pView);
	
	return true;
}

bool AP_UnixFrame::initFrameData(void)
{
	UT_ASSERT(!m_pData);

	AP_FrameData* pData = new AP_FrameData();
	m_pData = (void*) pData;
	
	return (pData ? true : false);
}

void AP_UnixFrame::killFrameData(void)
{
	DELETEP(m_pData);
	m_pData = NULL;
}

XAP_Frame* AP_UnixFrame::cloneFrame(void)
{
	AP_UnixFrame* pClone = new AP_UnixFrame(this);
	ENSUREP(pClone);

	if (!pClone->initialize())
	{
		goto Cleanup;
	}

	pClone->show();

	return pClone;

 Cleanup:
	m_pUnixApp->forgetFrame(pClone);
	DELETEP(pClone);
	return NULL;
}

static gint expose(GtkWidget * w, GdkEventExpose* pExposeEvent);

void AP_UnixFrame::setXScrollRange(void)
{
}

void AP_UnixFrame::setYScrollRange(void)
{
}

void AP_UnixFrame::translateDocumentToScreen(UT_sint32&, UT_sint32&)
{
}

GtkWidget* AP_UnixFrame::_createDocumentWindow(void)
{
	GtkWidget* wSunkenBox = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(wSunkenBox), GTK_SHADOW_IN);
	
	m_dArea = gtk_drawing_area_new();

	gtk_object_set_user_data(GTK_OBJECT(m_dArea), this);
	gtk_signal_connect(GTK_OBJECT(m_dArea), "expose_event",
					   GTK_SIGNAL_FUNC(_fe::expose), this);

	gtk_widget_show(m_dArea);
	gtk_container_add(GTK_CONTAINER(wSunkenBox), m_dArea);

	return wSunkenBox;
}

void AP_UnixFrame::setStatusMessage(const char *szMsg)
{
	m_pUnixStatusBar->setStatusMessage(szMsg);
}

GtkWidget* AP_UnixFrame::_createStatusBarWindow(void)
{
	m_pUnixStatusBar = new AP_UnixStatusBar(this);
	UT_ASSERT(m_pUnixStatusBar);
	
	GtkWidget * w = m_pUnixStatusBar->createWidget();
	
	return w;
}
void AP_UnixFrame::_setWindowIcon(void)
{
}
void AP_UnixFrame::toggleRuler(bool bRulerOn)
{
}
