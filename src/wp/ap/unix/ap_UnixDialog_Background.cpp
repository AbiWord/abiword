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

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "ut_dialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_UnixDialog_Background.h"

enum
{
	RED,
	GREEN,
	BLUE,
	OPACITY
};

static void s_color_cleared(GtkWidget * btn, AP_UnixDialog_Background * dlg)
{
	UT_ASSERT(dlg);
	dlg->colorCleared();
}

static void s_ok_clicked (GtkWidget * btn, AP_UnixDialog_Background * dlg)
{
	UT_ASSERT(dlg);
	dlg->eventOk();
}

static void s_cancel_clicked (GtkWidget * btn, AP_UnixDialog_Background * dlg)
{
	UT_ASSERT(dlg);
	dlg->eventCancel();
}

static void s_delete_clicked(GtkWidget * /* widget */,
							 gpointer /* data */,
							 AP_UnixDialog_Background * dlg)
{
	UT_ASSERT(dlg);
	dlg->eventCancel();
}

#define CTI(c, v) (int)(c[v] * 255.0)

static void s_color_changed(GtkWidget * csel,
			    AP_UnixDialog_Background * dlg)
{
	UT_ASSERT(csel && dlg);
  
	GtkColorSelection * w = GTK_COLOR_SELECTION(csel);
	gdouble cur [4];

	gtk_color_selection_get_color (w, cur);

	static char buf_color[12];
	sprintf(buf_color, "%02x%02x%02x",CTI(cur, RED),CTI(cur, GREEN),CTI(cur, BLUE));
	dlg->setColor ((const XML_Char *) buf_color);
}

#undef CTI

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_Background::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_UnixDialog_Background * p = new AP_UnixDialog_Background(pFactory,id);
	return p;
}

AP_UnixDialog_Background::AP_UnixDialog_Background(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_Background(pDlgFactory,id)
{
}

AP_UnixDialog_Background::~AP_UnixDialog_Background(void)
{
}

void AP_UnixDialog_Background::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	m_dlg = mainWindow;

	connectFocus(GTK_WIDGET(mainWindow), pFrame);
	
	// To center the dialog, we need the frame of its parent.
	XAP_UnixFrame * pUnixFrame = static_cast<XAP_UnixFrame *>(pFrame);
	UT_ASSERT(pUnixFrame);
	
	// Get the GtkWindow of the parent frame
	GtkWidget * parentWindow = pUnixFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	
	// Center our new dialog in its parent and make it a transient
	// so it won't get lost underneath
	centerDialog(parentWindow, mainWindow);

	// Show the top level dialog,
	gtk_widget_show(mainWindow);

	// Make it modal, and stick it up top
	gtk_grab_add(mainWindow);

	// run into the gtk main loop for this window
	gtk_main();

	if(mainWindow && GTK_IS_WIDGET(mainWindow))
		gtk_widget_destroy(mainWindow);
}

GtkWidget * AP_UnixDialog_Background::_constructWindow (void)
{
	GtkWidget * dlg;
	GtkWidget * k;
	GtkWidget * cancel;
	GtkWidget * actionarea;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	dlg = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW(dlg), 
						  pSS->getValue(AP_STRING_ID_DLG_Background_Title));

	actionarea = GTK_DIALOG (dlg)->action_area;

	k = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_OK));
	gtk_widget_show(k);
	gtk_container_add (GTK_CONTAINER(actionarea), k);
	gtk_signal_connect (GTK_OBJECT(k), "clicked", 
						GTK_SIGNAL_FUNC(s_ok_clicked), (gpointer)this);

	cancel = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
	gtk_widget_show(cancel);
	gtk_container_add (GTK_CONTAINER(actionarea), cancel);
	gtk_signal_connect (GTK_OBJECT(cancel), "clicked", 
						GTK_SIGNAL_FUNC(s_cancel_clicked), (gpointer)this);

	gtk_signal_connect_after(GTK_OBJECT(dlg),
							 "destroy",
							 NULL,
							 NULL);

	gtk_signal_connect(GTK_OBJECT(dlg),
					   "delete_event",
					   GTK_SIGNAL_FUNC(s_delete_clicked),
					   (gpointer) this);
  
	_constructWindowContents (GTK_DIALOG(dlg)->vbox);
	
	return dlg;
}

void AP_UnixDialog_Background::_constructWindowContents (GtkWidget * parent)
{
	GtkWidget *colorsel;

	GtkWidget * vbox = gtk_vbox_new(false,0);
	gtk_widget_show(vbox);
	gtk_container_add (GTK_CONTAINER(parent), vbox);

	colorsel = gtk_color_selection_new();
	gtk_widget_show (colorsel);
	gtk_container_add (GTK_CONTAINER(vbox), colorsel);

	const XML_Char *  pszC = getColor();
	UT_RGBColor c(255,255,255);
	if(strcmp(pszC,"transparent") != 0)
	{
		UT_parseColor(pszC,c);
	}
	gdouble currentColor[4] = { 0, 0, 0, 0 };
	currentColor[RED] = ((gdouble) c.m_red / (gdouble) 255.0);
	currentColor[GREEN] = ((gdouble) c.m_grn / (gdouble) 255.0);
	currentColor[BLUE] = ((gdouble) c.m_blu / (gdouble) 255.0);

	gtk_color_selection_set_color (GTK_COLOR_SELECTION(colorsel),
								   currentColor);
	m_wColorsel = colorsel;
//
// Button to clear background color
//
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	GtkWidget * clearColor = gtk_button_new_with_label (pSS->getValue (AP_STRING_ID_DLG_Background_ClearClr));
	gtk_widget_show(clearColor);
	
	gtk_container_add(GTK_CONTAINER(vbox),clearColor);

	gtk_signal_connect (GTK_OBJECT(colorsel), "color-changed",
						GTK_SIGNAL_FUNC(s_color_changed),
						(gpointer) this);
	gtk_signal_connect(GTK_OBJECT(clearColor), "clicked",
						GTK_SIGNAL_FUNC(s_color_cleared),
						(gpointer) this);
}

void AP_UnixDialog_Background::eventOk (void)
{
	setAnswer (a_OK);
	gtk_main_quit();
}

void AP_UnixDialog_Background::eventCancel (void)
{
	setAnswer(a_CANCEL);
	gtk_main_quit ();
}

void AP_UnixDialog_Background::colorCleared(void)
{
	setColor(NULL);
	gdouble currentColor[4] = { 1., 1., 1., 0 };
	gtk_color_selection_set_color (GTK_COLOR_SELECTION(m_wColorsel),
								   currentColor);
}	
	
