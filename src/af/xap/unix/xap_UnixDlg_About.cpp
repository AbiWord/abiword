/* AbiSource Application Framework
 * Copyright (C) 1998-2002 AbiSource, Inc.
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
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_About.h"
#include "xap_UnixDlg_About.h"

#include "gr_UnixGraphics.h"
#include "gr_UnixImage.h"
#include "ut_bytebuf.h"
#include "ut_png.h"

/*****************************************************************/

extern unsigned char g_pngSidebar[];		// see ap_wp_sidebar.cpp
extern unsigned long g_pngSidebar_sizeof;	// see ap_wp_sidebar.cpp

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
  m_pFrame = static_cast<XAP_UnixFrame *>(pFrame);
  
  // Build the window's widgets and arrange them
  GtkDialog * mainWindow = GTK_DIALOG ( _constructWindow() );
  gtk_widget_show (GTK_WIDGET(mainWindow)); // need to realize the window right away, so we have a graphic's context for the drawing area's window

  // assemble an image
  _preparePicture();
  
  // attach a new graphics context
  XAP_App *pApp = m_pFrame->getApp();
  
#ifndef WITH_PANGO	
  m_gc = new GR_UnixGraphics(m_drawingareaGraphic->window, NULL, pApp);
#else
  m_gc = new GR_UnixGraphics(m_drawingareaGraphic->window, pApp);
#endif
  
  switch ( abiRunModalDialog ( mainWindow, pFrame, this, BUTTON_CLOSE, true ) )
    {
    case BUTTON_URL:
      event_URL () ; break;
    default:
      break ;
    }
}

void XAP_UnixDialog_About::event_URL(void)
{
  m_pFrame->openURL("http://www.abisource.com/");
}

void XAP_UnixDialog_About::event_DrawingAreaExpose(void)
{
  UT_return_if_fail(m_gc);
  m_gc->drawImage(m_pGrImageSidebar, 0, 0);
}

/*****************************************************************/

GtkWidget * XAP_UnixDialog_About::_constructWindow(void)
{
  GtkWidget *windowAbout;
  GtkWidget *hboxAbout;
  GtkWidget *drawingareaGraphic;
  GtkWidget *vboxInfo;
  GtkWidget *labelTitle;
  GtkWidget *labelVersion;
  GtkWidget *textCopyright;
  
  // we use this for all sorts of strings that can't appear in the string sets
  char buf[4096];

  windowAbout = abiDialogNew ("about dialog", TRUE, XAP_ABOUT_TITLE, m_pApp->getApplicationName());
  gtk_widget_set_usize (windowAbout, 0, 350);
  
  hboxAbout = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hboxAbout);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(windowAbout)->vbox), hboxAbout);
  
  drawingareaGraphic = createDrawingArea ();
  gtk_widget_set_events(drawingareaGraphic, GDK_EXPOSURE_MASK);
  g_signal_connect (G_OBJECT(drawingareaGraphic), "expose_event",
		    G_CALLBACK(s_drawingarea_expose), (gpointer) this);
  gtk_widget_set_usize (drawingareaGraphic, 200, 350);
  gtk_widget_show (drawingareaGraphic);
  gtk_box_pack_start (GTK_BOX (hboxAbout), drawingareaGraphic, 
		      TRUE, TRUE, 0);
   
  vboxInfo = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vboxInfo);
  gtk_box_pack_start (GTK_BOX (hboxAbout), vboxInfo, TRUE, TRUE, 8);

  labelTitle = gtk_label_new (m_pApp->getApplicationName());
  gtk_widget_show (labelTitle);
  gtk_box_pack_start (GTK_BOX (vboxInfo), labelTitle, FALSE, TRUE, 18);
  
  // make the font really big
  GtkStyle * bigstyle = gtk_style_copy(gtk_widget_get_style(labelTitle));
  UT_ASSERT(bigstyle);
  gdk_font_unref(bigstyle->private_font);
  bigstyle->private_font = gdk_font_load("-*-helvetica-bold-r-*-*-*-240-*-*-*-*-*-*");
  gtk_widget_set_style(labelTitle, bigstyle);
  
  g_snprintf(buf, 4096, XAP_ABOUT_VERSION, XAP_App::s_szBuild_Version);
  
  labelVersion = gtk_label_new (buf);
  gtk_widget_show (labelVersion);
  gtk_box_pack_start (GTK_BOX (vboxInfo), labelVersion, FALSE, FALSE, 0);
  
  char buf2[4096];
  g_snprintf(buf2, 4096, XAP_ABOUT_GPL_LONG_LINE_BROKEN, m_pApp->getApplicationName());
	
  g_snprintf(buf, 4096, "%s\n\n%s", XAP_ABOUT_COPYRIGHT, buf2);
  
  textCopyright = gtk_text_view_new ();
  gtk_widget_show (textCopyright);
  gtk_box_pack_start (GTK_BOX (vboxInfo), textCopyright, TRUE, FALSE, 10);
  gtk_widget_set_usize (textCopyright, 290, 200);
  gtk_widget_realize (textCopyright);
  gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(textCopyright)), buf, -1);
  
  // make the font slightly smaller
  GtkStyle * smallstyle = gtk_style_copy(gtk_widget_get_style(textCopyright));
  UT_ASSERT(smallstyle);
  gdk_font_unref(smallstyle->private_font);
  smallstyle->private_font = gdk_font_load("-*-helvetica-medium-r-*-*-*-100-*-*-*-*-*-*");
  gtk_widget_set_style(textCopyright, smallstyle);

  // add the buttons
  abiAddButton (GTK_DIALOG(windowAbout), 
				"http://www.abisource.com", BUTTON_URL);
  abiAddStockButton (GTK_DIALOG(windowAbout), 
					 GTK_STOCK_CLOSE, BUTTON_CLOSE);
  
  // Since we do drawing, we need a graphics context which can
  // understand PNG data.
  
  // Update member variables with the important widgets that
  // might need to be queried or altered later.
  
  m_windowMain = windowAbout;
  m_drawingareaGraphic = drawingareaGraphic;
  
  return windowAbout;
}

void XAP_UnixDialog_About::_preparePicture(void)
{
  UT_ByteBuf * pBB = new UT_ByteBuf(g_pngSidebar_sizeof);
  pBB->ins(0,g_pngSidebar,g_pngSidebar_sizeof);

  UT_sint32 iImageWidth;
  UT_sint32 iImageHeight;
  
  UT_PNG_getDimensions(pBB, iImageWidth, iImageHeight);
  
  m_pGrImageSidebar = new GR_UnixImage(NULL);
  m_pGrImageSidebar->convertFromBuffer(pBB, iImageWidth, iImageHeight);
  
  DELETEP(pBB);
}
