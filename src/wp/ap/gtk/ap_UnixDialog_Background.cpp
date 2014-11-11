/* AbiWord
 * Copyright (C) 2000-2002 AbiSource, Inc.
 * Copyright (C) 2009, 2013 Hubert Figuiere
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <stdlib.h>
#include <stdio.h>

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"
#include "xap_Gtk2Compat.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_UnixDialog_Background.h"
#include "ut_debugmsg.h"

enum
{
	RED,
	GREEN,
	BLUE,
	OPACITY
};

static void s_color_cleared(GtkWidget * /*btn*/, AP_UnixDialog_Background * dlg)
{
	UT_return_if_fail(dlg);
	dlg->colorCleared();
}

static void s_color_changed(GtkWidget * csel,
#if GTK_CHECK_VERSION(3,4,0)
                GdkRGBA *,
#endif
			    AP_UnixDialog_Background * dlg)
{
	UT_ASSERT(csel && dlg);
  
	GtkColorChooser * w = GTK_COLOR_CHOOSER(csel);
	GdkRGBA cur;

	gtk_color_chooser_get_rgba (w, &cur);
	UT_RGBColor * rgbcolor = UT_UnixGdkColorToRGBColor(cur);
	UT_HashColor hash_color;
	dlg->setColor (hash_color.setColor(*rgbcolor) + 1); // return with # prefix
	delete rgbcolor;
}


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
	UT_return_if_fail(pFrame);

	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);
	m_dlg = mainWindow;

	switch ( abiRunModalDialog ( GTK_DIALOG(m_dlg), pFrame, this,
								 BUTTON_OK, true ) )
	{
		case BUTTON_OK:
			eventOk () ; break;
		default:
			eventCancel(); break ;
	}
}

GtkWidget * AP_UnixDialog_Background::_constructWindow (void)
{
	GtkWidget * dlg;

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	std::string s;
	
	if(isForeground())
	{
		pSS->getValueUTF8(AP_STRING_ID_DLG_Background_TitleFore,s);
		dlg = abiDialogNew ( "background dialog", TRUE, s.c_str()) ;
	}
	else if(isHighlight())
	{
		pSS->getValueUTF8(AP_STRING_ID_DLG_Background_TitleHighlight,s);
		dlg = abiDialogNew ( "background dialog", TRUE, s.c_str()) ;
	}
	else
	{
		pSS->getValueUTF8(AP_STRING_ID_DLG_Background_Title,s);
		dlg = abiDialogNew ( "background dialog", TRUE, s.c_str()) ;
	}

	gtk_window_set_resizable (GTK_WINDOW (dlg), false);

	pSS->getValueUTF8(XAP_STRING_ID_DLG_Cancel, s);
	abiAddButton (GTK_DIALOG(dlg), s, BUTTON_CANCEL ) ;
	pSS->getValueUTF8(XAP_STRING_ID_DLG_OK, s);
	abiAddButton (GTK_DIALOG(dlg), s, BUTTON_OK ) ;

	_constructWindowContents (gtk_dialog_get_content_area(GTK_DIALOG(dlg)));

	return dlg;
}

void AP_UnixDialog_Background::_constructWindowContents (GtkWidget * parent)
{
	GtkWidget *colorsel;

	GtkWidget * vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
	gtk_widget_show (vbox);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
	gtk_container_add (GTK_CONTAINER(parent), vbox);

	colorsel = gtk_color_chooser_widget_new();
#if !GTK_CHECK_VERSION(3,4,0)
	gtk_color_selection_set_has_palette (GTK_COLOR_SELECTION (colorsel), TRUE);
#endif
	gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(colorsel), false);
	gtk_widget_show (colorsel);
	gtk_container_add (GTK_CONTAINER(vbox), colorsel);

	const gchar *  pszC = getColor();
	UT_RGBColor c(255,255,255);
	if(strcmp(pszC,"transparent") != 0)
	{
		UT_parseColor(pszC,c);
	}
	GdkRGBA *gcolor = UT_UnixRGBColorToGdkRGBA(c);

	gtk_color_chooser_set_rgba( GTK_COLOR_CHOOSER ( colorsel ), gcolor);
	gdk_rgba_free(gcolor);

	m_wColorsel = colorsel;
//
// Button to clear background color
//
	GtkWidget * alignment = NULL;
	GtkWidget * clearColor = NULL;
	std::string s;
	
	if(!isForeground())
	{
		const XAP_StringSet * pSS = m_pApp->getStringSet();
		if(isHighlight())
		{
			pSS->getValueUTF8 (AP_STRING_ID_DLG_Background_ClearHighlight,s);
			clearColor = gtk_button_new_with_label (s.c_str());
		}
		else
		{
			pSS->getValueUTF8 (AP_STRING_ID_DLG_Background_ClearClr,s);
			clearColor = gtk_button_new_with_label (s.c_str());
		}
		gtk_widget_show(clearColor);

		alignment = gtk_alignment_new (1.0, 0.5, 0.0, 0.0);
		gtk_widget_show(alignment);
		gtk_container_add (GTK_CONTAINER (alignment), clearColor);
		gtk_box_pack_start (GTK_BOX (vbox), alignment, FALSE, FALSE, 0);

		g_signal_connect(G_OBJECT(clearColor), "clicked",
						G_CALLBACK(s_color_cleared),
						(gpointer) this);
	}
	g_signal_connect (G_OBJECT(colorsel),
#if GTK_CHECK_VERSION(3,4,0)
			  "color-activated",
#else
			  "color-changed",
#endif
			  G_CALLBACK(s_color_changed),
			  (gpointer) this);
}

void AP_UnixDialog_Background::eventOk (void)
{
	setAnswer (a_OK);
}

void AP_UnixDialog_Background::eventCancel (void)
{
	setAnswer(a_CANCEL);
}

void AP_UnixDialog_Background::colorCleared(void)
{
	setColor(NULL);
	GdkRGBA gcolor;
#if !GTK_CHECK_VERSION(3,0,0)
	gcolor.pixel = 0;
	gcolor.red = 0xffff;
	gcolor.blue = 0xffff;
	gcolor.green = 0xffff;
#else
	gcolor.red = 1.0;
	gcolor.blue = 1.0;
	gcolor.green = 1.0;
	gcolor.alpha = 1.0;
#endif
	gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER(m_wColorsel),
                                    &gcolor);
}

