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
#include "xap_QNXFrame.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_About.h"
#include "xap_QNXDlg_About.h"

#include "gr_QNXGraphics.h"
#include "gr_QNXImage.h"
#include "ut_bytebuf.h"
#include "ut_png.h"

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

// These are all static callbacks, bound to GTK or GDK events.
#if 0
static void s_ok_clicked(GtkWidget * widget,
						 XAP_QNXDialog_About * dlg)
{
	UT_ASSERT(widget && dlg);

	dlg->event_OK();
}

static void s_url_clicked(GtkWidget * widget,
						  XAP_QNXDialog_About * dlg)
{
	UT_ASSERT(widget && dlg);

	dlg->event_URL();
}

static void s_delete_clicked(GtkWidget * /* widget */,
							 gpointer /* data */,
							 XAP_QNXDialog_About * dlg)
{
	UT_ASSERT(dlg);

	dlg->event_WindowDelete();
}

static int s_drawingarea_expose(GtkWidget * /* widget */,
								 GdkEventExpose * /* pExposeEvent */,
								 XAP_QNXDialog_About * dlg)
{
	UT_ASSERT(dlg);

	dlg->event_DrawingAreaExpose();

	return FALSE;
}
#endif

/*****************************************************************/

void XAP_QNXDialog_About::runModal(XAP_Frame * pFrame)
{
#if 0
	// stash away the frame
	m_pFrame = static_cast<XAP_QNXFrame *>(pFrame);

	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// assemble an image
	_preparePicture();
	
	// To center the dialog, we need the frame of its parent.
	XAP_QNXFrame * pQNXFrame = static_cast<XAP_QNXFrame *>(pFrame);
	UT_ASSERT(pQNXFrame);
	
	// Get the GtkWindow of the parent frame
	GtkWidget * parentWindow = pQNXFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	
	// Center our new dialog in its parent and make it a transient
	// so it won't get lost underneath
    centerDialog(parentWindow, mainWindow);
	gtk_window_set_transient_for(GTK_WINDOW(mainWindow), GTK_WINDOW(parentWindow));

	// Show the top level dialog,
	gtk_widget_show(mainWindow);

	// Make it modal, and stick it up top
	gtk_grab_add(mainWindow);

	// attach a new graphics context
	m_gc = new GR_QNXGraphics(m_drawingareaGraphic->window, NULL);
	
	// Run into the GTK event loop for this window.
	gtk_main();

	gtk_widget_destroy(mainWindow);
#endif
}

void XAP_QNXDialog_About::event_OK(void)
{
#if 0
	gtk_main_quit();
#endif
}

void XAP_QNXDialog_About::event_URL(void)
{
	m_pFrame->openURL("http://www.abisource.com/");
}

void XAP_QNXDialog_About::event_WindowDelete(void)
{
#if 0
	gtk_main_quit();
#endif
}

void XAP_QNXDialog_About::event_DrawingAreaExpose(void)
{
#if 0
	if (!m_gc)
		return;

	m_gc->drawImage(m_pGrImageSidebar, 0, 0);
#endif
}

/*****************************************************************/
#if 0
GtkWidget * XAP_QNXDialog_About::_constructWindow(void)
{
	GtkWidget *windowAbout;
	GtkWidget *hboxAbout;
	GtkWidget *drawingareaGraphic;
	GtkWidget *vboxInfo;
	GtkWidget *labelTitle;
	GtkWidget *labelVersion;
	GtkWidget *textCopyright;
	GtkWidget *hbox2;
	GtkWidget *buttonURL;
	GtkWidget *buttonOK;

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// we use this for all sorts of strings that can't appear in the string sets
	char buf[4096];

	g_snprintf(buf, 4096, XAP_ABOUT_TITLE, m_pApp->getApplicationName());

	windowAbout = gtk_window_new (GTK_WINDOW_DIALOG);
	gtk_object_set_data (GTK_OBJECT (windowAbout), "windowAbout", windowAbout);
	gtk_widget_set_usize (windowAbout, 0, 350);
	gtk_window_set_title (GTK_WINDOW (windowAbout), buf);
	gtk_window_set_policy (GTK_WINDOW (windowAbout), FALSE, FALSE, FALSE);

	hboxAbout = gtk_hbox_new (FALSE, 0);
	gtk_object_set_data (GTK_OBJECT (windowAbout), "hboxAbout", hboxAbout);
	gtk_widget_show (hboxAbout);
	gtk_container_add (GTK_CONTAINER (windowAbout), hboxAbout);

	drawingareaGraphic = gtk_drawing_area_new ();
	gtk_object_set_data (GTK_OBJECT (windowAbout), "drawingareaGraphic", drawingareaGraphic);
	gtk_widget_set_events(drawingareaGraphic, GDK_EXPOSURE_MASK);
	gtk_signal_connect (GTK_OBJECT(drawingareaGraphic), "expose_event",
						GTK_SIGNAL_FUNC(s_drawingarea_expose), (gpointer) this);
	gtk_widget_show (drawingareaGraphic);
	gtk_box_pack_start (GTK_BOX (hboxAbout), drawingareaGraphic, TRUE, TRUE, 0);
	// This size is kinda arbitrary, and will need to be adjusted as the graphics change
	gtk_widget_set_usize (drawingareaGraphic, 200, 350);

	vboxInfo = gtk_vbox_new (FALSE, 0);
	gtk_object_set_data (GTK_OBJECT (windowAbout), "vboxInfo", vboxInfo);
	gtk_widget_show (vboxInfo);
	gtk_box_pack_start (GTK_BOX (hboxAbout), vboxInfo, TRUE, TRUE, 8);

	labelTitle = gtk_label_new (m_pApp->getApplicationName());
	gtk_object_set_data (GTK_OBJECT (windowAbout), "labelTitle", labelTitle);
	gtk_widget_show (labelTitle);
	gtk_box_pack_start (GTK_BOX (vboxInfo), labelTitle, FALSE, TRUE, 18);

	// make the font really big
	GtkStyle * bigstyle = gtk_style_copy(gtk_widget_get_style(labelTitle));
	UT_ASSERT(bigstyle);
	gdk_font_unref(bigstyle->font);
	bigstyle->font = gdk_font_load("-*-helvetica-bold-r-*-*-*-240-*-*-*-*-*-*");
	gtk_widget_set_style(labelTitle, bigstyle);
	
	g_snprintf(buf, 4096, XAP_ABOUT_VERSION, XAP_App::s_szBuild_Version);
	
	labelVersion = gtk_label_new (buf);
	gtk_object_set_data (GTK_OBJECT (windowAbout), "labelVersion", labelVersion);
	gtk_widget_show (labelVersion);
	gtk_box_pack_start (GTK_BOX (vboxInfo), labelVersion, FALSE, FALSE, 0);

	char buf2[4096];
	g_snprintf(buf2, 4096, XAP_ABOUT_GPL_LONG_LINE_BROKEN, m_pApp->getApplicationName());
	
	g_snprintf(buf, 4096, "%s\n\n%s", XAP_ABOUT_COPYRIGHT, buf2);
	
	textCopyright = gtk_text_new (NULL, NULL);
	gtk_object_set_data (GTK_OBJECT (windowAbout), "textCopyright", textCopyright);
	gtk_widget_show (textCopyright);
	gtk_box_pack_start (GTK_BOX (vboxInfo), textCopyright, TRUE, FALSE, 10);
	gtk_widget_set_usize (textCopyright, 290, 200);
	gtk_widget_realize (textCopyright);
	gtk_text_insert (GTK_TEXT (textCopyright), NULL, NULL, NULL, buf, strlen(buf));

	// make the font slightly smaller
	GtkStyle * smallstyle = gtk_style_copy(gtk_widget_get_style(textCopyright));
	UT_ASSERT(smallstyle);
	gdk_font_unref(smallstyle->font);
	smallstyle->font = gdk_font_load("-*-helvetica-medium-r-*-*-*-100-*-*-*-*-*-*");
	gtk_widget_set_style(textCopyright, smallstyle);
	
	hbox2 = gtk_hbox_new (FALSE, 10);
	gtk_object_set_data (GTK_OBJECT (windowAbout), "hbox2", hbox2);
	gtk_widget_show (hbox2);
	gtk_box_pack_start (GTK_BOX (vboxInfo), hbox2, FALSE, TRUE, 10);

	buttonURL = gtk_button_new_with_label ("www.abisource.com");
	gtk_object_set_data (GTK_OBJECT (windowAbout), "buttonURL", buttonURL);
	gtk_widget_show (buttonURL);
	gtk_box_pack_start (GTK_BOX (hbox2), buttonURL, FALSE, TRUE, 0);
	gtk_widget_set_usize (buttonURL, 140, 24);

	buttonOK = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_OK));
	gtk_object_set_data (GTK_OBJECT (windowAbout), "buttonOK", buttonOK);
	gtk_widget_show (buttonOK);
	gtk_box_pack_end (GTK_BOX (hbox2), buttonOK, FALSE, TRUE, 0);
	gtk_widget_set_usize (buttonOK, 85, 24);

	// Since we do drawing, we need a graphics context which can
	// understand PNG data.
	
	
	gtk_signal_connect(GTK_OBJECT(buttonOK),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_ok_clicked),
					   (gpointer) this);
	
	gtk_signal_connect(GTK_OBJECT(buttonURL),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_url_clicked),
					   (gpointer) this);

	gtk_signal_connect_after(GTK_OBJECT(windowAbout),
							 "delete_event",
							 GTK_SIGNAL_FUNC(s_delete_clicked),
							 (gpointer) this);

	gtk_signal_connect_after(GTK_OBJECT(windowAbout),
							 "destroy",
							 NULL,
							 NULL);

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
	
	m_pGrImageSidebar = new GR_QNXImage(NULL,NULL);
	m_pGrImageSidebar->convertFromPNG(pBB, iImageWidth, iImageHeight);

	DELETEP(pBB);
}
#endif
