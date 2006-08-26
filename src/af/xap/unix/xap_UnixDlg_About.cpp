/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
#include <stdlib.h>
#include <string.h>
#include <glade/glade.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_About.h"
#include "xap_UnixDlg_About.h"

#include "gr_UnixGraphics.h"
#include "gr_UnixImage.h"
#include "ut_bytebuf.h"
#include "ut_png.h"
#include "gr_Painter.h"

/*****************************************************************/

extern unsigned char g_pngSidebar[];            // see ap_wp_sidebar.cpp
extern unsigned long g_pngSidebar_sizeof;       // see ap_wp_sidebar.cpp

/*****************************************************************/

XAP_Dialog * XAP_UnixDialog_About::static_constructor(XAP_DialogFactory * pFactory,
						      XAP_Dialog_Id id)
{
  XAP_UnixDialog_About * p = new XAP_UnixDialog_About(pFactory,id);
  return p;
}

XAP_UnixDialog_About::XAP_UnixDialog_About(XAP_DialogFactory * pDlgFactory,
					   XAP_Dialog_Id id)
  : XAP_Dialog_About(pDlgFactory,id)
{
  m_windowMain = NULL;
  m_drawingareaGraphic = NULL;
  m_gc = NULL;
  m_pGrImageSidebar = NULL;
}

XAP_UnixDialog_About::~XAP_UnixDialog_About(void)
{
  DELETEP(m_gc);
  DELETEP(m_pGrImageSidebar);
}

/*****************************************************************/

gint XAP_UnixDialog_About::s_drawingarea_expose(GtkWidget * /* widget */,
						GdkEventExpose * /* pExposeEvent */,
						XAP_UnixDialog_About * dlg)
{
  UT_return_val_if_fail(dlg, FALSE);	
  dlg->event_DrawingAreaExpose();	
  return FALSE;
}

/*****************************************************************/

void XAP_UnixDialog_About::runModal(XAP_Frame * pFrame)
{
	// stash away the frame
	m_pFrame = pFrame;
  
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);
	
	// assemble an image
	_preparePicture();
  
	// attach a new graphics context
	XAP_App *pApp = XAP_App::getApp();
  
	//m_gc = new GR_UnixGraphics(m_drawingareaGraphic->window, NULL, pApp);
	GR_UnixAllocInfo ai(m_drawingareaGraphic->window, NULL);
	m_gc = (GR_UnixGraphics*) XAP_App::getApp()->newGraphics(ai);
	
	switch ( abiRunModalDialog ( GTK_DIALOG(mainWindow), pFrame, this, BUTTON_CLOSE, true ) )
	{
		case BUTTON_URL:
			event_URL();
			break;
    	default:
      		break;
    }
}

void XAP_UnixDialog_About::event_URL(void)
{
	XAP_App::getApp()->openURL("http://www.abisource.com/");
}

void XAP_UnixDialog_About::event_DrawingAreaExpose(void)
{
	UT_return_if_fail(m_gc);

	GR_Painter painter (m_gc);
	painter.drawImage(m_pGrImageSidebar, 0, 0);
}

/*****************************************************************/

GtkWidget * XAP_UnixDialog_About::_constructWindow(void)
{
	GtkWidget *window;
	
	// get the path where our glade file is located
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(XAP_App::getApp());	
	UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
	glade_path += "/xap_UnixDlg_About.glade";
	
	// load the dialog from the glade file
	GladeXML *xml = abiDialogNewFromXML( glade_path.c_str() );
	if (!xml)
		return NULL;
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	window = glade_xml_get_widget(xml, "ap_UnixDlg_About");
	m_drawingareaGraphic = glade_xml_get_widget(xml, "daLogo");
	
	// set the dialog title and some non-localizable strings
	
	abiDialogSetTitle(window, XAP_ABOUT_TITLE, pApp->getApplicationName());
	
	UT_String title = UT_String_sprintf("%s %s", pApp->getApplicationName(), XAP_App::s_szBuild_Version);
	setLabelMarkup(glade_xml_get_widget(xml, "lbTitle"), title.c_str());
	
	UT_String license = UT_String_sprintf(XAP_ABOUT_GPL_LONG_LINE_BROKEN, pApp->getApplicationName());
	setLabelMarkup(glade_xml_get_widget(xml, "lbLicense"), license.c_str());
	
	setLabelMarkup(glade_xml_get_widget(xml, "lbCopyright"), XAP_ABOUT_COPYRIGHT);
	
	// connect some signals
	// FIXME: fix the drawing of a nice logo
	/*g_signal_connect (G_OBJECT(m_drawingareaGraphic), "expose_event",
		    G_CALLBACK(s_drawingarea_expose), static_cast<gpointer>(this));*/
	
	return window;
 }

void XAP_UnixDialog_About::_preparePicture(void)
{
	UT_ByteBuf * pBB = new UT_ByteBuf(g_pngSidebar_sizeof);
	pBB->ins(0,g_pngSidebar,g_pngSidebar_sizeof);
	
	UT_sint32 iImageWidth;
	UT_sint32 iImageHeight;
	
	UT_PNG_getDimensions(pBB, iImageWidth, iImageHeight);
	
	// create a pixmap from our included data
	m_pGrImageSidebar = new GR_UnixImage(NULL);
	m_pGrImageSidebar->convertFromBuffer(pBB, iImageWidth, iImageHeight);
	
	DELETEP(pBB);
}
