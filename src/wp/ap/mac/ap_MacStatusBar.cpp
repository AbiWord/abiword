/* AbiWord
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

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_Frame.h"
#include "xap_MacFrame.h"
#include "gr_MacGraphics.h"
#include "ap_MacStatusBar.h"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_MacStatusBar::AP_MacStatusBar(XAP_Frame * pFrame)
	: AP_StatusBar(pFrame)
{
	m_wStatusBar = NULL;
}

AP_MacStatusBar::~AP_MacStatusBar(void)
{
    if (m_wStatusBar) {
        DisposeControl (m_wStatusBar);
    }
}

void AP_MacStatusBar::setView(AV_View * pView)
{
/*
	// We really should allocate m_pG in createWidget(), but
	// unfortunately, the actual window (m_wStatusBar->window)
	// is not created until the frame's top-level window is
	// shown.

	DELETEP(m_pG);	
	XAP_MacApp * app = static_cast<XAP_MacApp *>(m_pFrame->getApp());
	XAP_MacFontManager * fontManager = app->getFontManager();
	GR_MacGraphics * pG = new GR_MacGraphics(m_wStatusBar->window, fontManager, m_pFrame->getApp());
	m_pG = pG;
	UT_ASSERT(m_pG);

	GtkStyle * style = gtk_widget_get_style((static_cast<XAP_MacFrame *> (m_pFrame))->getTopLevelWindow());
	UT_ASSERT(style);
	pG->init3dColors(style);

	GR_Font * pFont = m_pG->getGUIFont();
	m_pG->setFont(pFont);

	// Now that we've initialized the graphics context and
	// installed the GUI font, let the base class do it's
	// think and layout the fields.
	*/
	AP_StatusBar::setView(pView);
}

ControlHandle AP_MacStatusBar::createWidget(void)
{
#if 0
	UT_ASSERT(!m_pG && !m_wStatusBar);
	
	m_wStatusBar = gtk_drawing_area_new();

	gtk_object_set_user_data(GTK_OBJECT(m_wStatusBar),this);
	gtk_widget_show(m_wStatusBar);
	gtk_widget_set_usize(m_wStatusBar, -1, s_iFixedHeight);

	gtk_widget_set_events(GTK_WIDGET(m_wStatusBar), (GDK_EXPOSURE_MASK));

	gtk_signal_connect(GTK_OBJECT(m_wStatusBar), "expose_event",
					   GTK_SIGNAL_FUNC(_fe::expose), NULL);
  
	gtk_signal_connect(GTK_OBJECT(m_wStatusBar), "configure_event",
					   GTK_SIGNAL_FUNC(_fe::configure_event), NULL);

#endif
	return m_wStatusBar;
}


void AP_MacStatusBar::show(void)
{
    ShowControl (m_wStatusBar);
}


void AP_MacStatusBar::hide(void)
{
    HideControl (m_wStatusBar);
}
