/* AbiWord
 * Copyright (C) 2002 Gabriel Gerhardsson
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
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "xap_Dialog_Id.h"
#include "ap_UnixDialog_Download_File.h"
#include "ap_Strings.h"

#include "gr_UnixGraphics.h"
#include "gr_UnixImage.h"
#include "ut_bytebuf.h"
#include "ut_png.h"
#include "ut_worker.h"

XAP_Dialog * AP_UnixDialog_Download_File::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_UnixDialog_Download_File * p = new AP_UnixDialog_Download_File(pFactory,id);
	return (XAP_Dialog*)p;
}

AP_UnixDialog_Download_File::AP_UnixDialog_Download_File(XAP_DialogFactory * pDlgFactory,
											 XAP_Dialog_Id id)
	: AP_Dialog_Download_File(pDlgFactory,id)
{
	m_windowMain = NULL;

	m_gc = NULL;

	m_pG = NULL;
}

AP_UnixDialog_Download_File::~AP_UnixDialog_Download_File(void)
{
	DELETEP(m_gc);
	DELETEP(m_pG);
}

/*****************************************************************/

// These are all static callbacks, bound to GTK or GDK events.

static gint s_PBConfigure(GtkWidget* w, GdkEventConfigure *e)
{
	AP_UnixDialog_Download_File * pUnixDDF = (AP_UnixDialog_Download_File *)gtk_object_get_user_data(GTK_OBJECT(w));
	if (pUnixDDF)
		pUnixDDF->event_PBConfigure(e);
	return 1;
}
	
static gint s_PBExpose(GtkWidget * w, GdkEventExpose * /*pExposeEvent*/)
{
	AP_UnixDialog_Download_File * pUnixDDF = (AP_UnixDialog_Download_File *)gtk_object_get_user_data(GTK_OBJECT(w));
	if (pUnixDDF)
		pUnixDDF->event_PBExpose();
	return 0;
}



/*****************************************************************/


void AP_UnixDialog_Download_File::_runModal(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);	
	
	/*
	 * Init a graphical context for the progressbar
	 */
	XAP_UnixApp * app = static_cast<XAP_UnixApp *>(m_pFrame->getApp());
	XAP_UnixFontManager * fontManager = app->getFontManager();
	GR_UnixGraphics * pG = new GR_UnixGraphics(m_progressBar->window, fontManager, m_pFrame->getApp());
	m_pG = pG;
	UT_ASSERT(m_pG);

	GtkStyle * style = gtk_widget_get_style((static_cast<XAP_UnixFrame *> (m_pFrame))->getTopLevelWindow());
	UT_ASSERT(style);
	pG->init3dColors(style);

	GR_Font * pFont = m_pG->getGUIFont();
	UT_ASSERT(pFont);
	m_pG->setFont(pFont);

	switch ( abiRunModalDialog ( GTK_DIALOG(mainWindow),
								 pFrame, this, BUTTON_CANCEL, false ) )
	{
		default:
			event_Cancel () ; break ;
	}

	abiDestroyWidget ( mainWindow ) ;
}

void
AP_UnixDialog_Download_File::_abortDialog(void)
{
	event_WindowDelete();
}

void AP_UnixDialog_Download_File::event_WindowDelete(void)
{
	abiDestroyWidget ( m_windowMain ) ;
}

void AP_UnixDialog_Download_File::event_Cancel(void)
{
	_setUserAnswer(a_CANCEL);
}

void AP_UnixDialog_Download_File::event_PBConfigure(GdkEventConfigure *e)
{
	_setHeight((UT_uint32)e->height);

	UT_uint32 iWidth = (UT_uint32)e->width;
	if (iWidth != _getWidth())
		_setWidth(iWidth);
	
	/* Adjust the drawing rectangle from the (just changed) geometry */
	_reflowPBRect();
	_updateProgress(m_pFrame);
}

void AP_UnixDialog_Download_File::event_PBExpose(void)
{
	_updateProgress(m_pFrame);
}

/*****************************************************************/
GtkWidget * AP_UnixDialog_Download_File::_constructProgressBar(void)
{
	GtkWidget *pb;
	
	pb = createDrawingArea ();

	gtk_object_set_user_data(GTK_OBJECT(pb),this);
	gtk_widget_show(pb);
	/* Set minimum wanted width */
	gtk_widget_set_usize(pb, _getWidth(), s_iPBFixedHeight);

	return pb;
}

GtkWidget * AP_UnixDialog_Download_File::_constructWindow(void)
{
	GtkWidget *windowDL;
	GtkWidget *vboxMain;
	GtkWidget *label;
	GtkWidget *progressBar;

	// we use this for all sorts of strings that can't appear in the string sets
	char buf[4096];

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	windowDL = abiDialogNew ( true, getTitle());	
	g_object_set_data (G_OBJECT (windowDL), "windowDL", windowDL);
	//gtk_window_set_policy (GTK_WINDOW (windowDL), FALSE, FALSE, FALSE);
	
	vboxMain = GTK_DIALOG(windowDL)->vbox;
	
	sprintf(buf, pSS->getValue(AP_STRING_ID_DLG_DlFile_Status), getDescription(), getURL());
	label = gtk_label_new (buf);
	g_object_set_data (G_OBJECT (vboxMain), "label", label);
	gtk_misc_set_padding (GTK_MISC(label), 10, 10);
	gtk_box_pack_start (GTK_BOX(vboxMain), label, TRUE, TRUE, 0);
	gtk_widget_show (label);
	
	progressBar = _constructProgressBar();
	gtk_box_pack_start (GTK_BOX (vboxMain), progressBar, FALSE, TRUE, 0);
	
	abiAddStockButton ( GTK_DIALOG(windowDL), GTK_STOCK_CANCEL, BUTTON_CANCEL ) ;
		
	gtk_widget_set_events(GTK_WIDGET(progressBar), (GDK_EXPOSURE_MASK));
	g_signal_connect(G_OBJECT(progressBar), 
						"expose_event", 
						G_CALLBACK(s_PBExpose), 
						NULL);
	
	g_signal_connect(G_OBJECT(progressBar), 
						"configure_event", 
						G_CALLBACK(s_PBConfigure), 
						NULL);

	// Update member variables with the important widgets that
	// might need to be queried or altered later.
	m_windowMain = windowDL;
	m_progressBar = progressBar;

	return windowDL;
}

