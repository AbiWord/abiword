/* AbiSource Application Framework
 * Copyright (C) 2001 AbiSource, Inc.
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

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_UnixGnomeDlg_ClipArt.h"
#include "ut_dialogHelper.h"
#include "xap_UnixFrame.h"
#include "xap_App.h"

// undefine this if you don't want the stock gnome icons in there too
#define USE_STOCK_ICONS_TOO 1

// TODO: make this use the GnomeIconList widget instead

XAP_Dialog * XAP_UnixGnomeDialog_ClipArt::static_constructor(XAP_DialogFactory * pFactory,
															 XAP_Dialog_Id id)
{
	XAP_UnixGnomeDialog_ClipArt * p = new XAP_UnixGnomeDialog_ClipArt(pFactory,id);
	return p;
}

XAP_UnixGnomeDialog_ClipArt::XAP_UnixGnomeDialog_ClipArt(XAP_DialogFactory * pDlgFactory,
														 XAP_Dialog_Id id)
	: XAP_Dialog_ClipArt (pDlgFactory,id), m_dialog(0), m_index (0)
{
}

XAP_UnixGnomeDialog_ClipArt::~XAP_UnixGnomeDialog_ClipArt(void)
{
}

#if 1

// simple fallback dummy impl.

GtkWidget * XAP_UnixGnomeDialog_ClipArt::_constructPreviewPane ()
{
	GtkWidget * clipArt = gnome_icon_selection_new ();

#ifdef USE_STOCK_ICONS_TOO	
	gnome_icon_selection_add_defaults (GNOME_ICON_SELECTION(clipArt));
#endif
	gnome_icon_selection_add_directory (GNOME_ICON_SELECTION(clipArt), 
										getInitialDir());

	return clipArt;
}

#else

// TODO: turn on this code someday

#define ICON_WIDTH 48

GtkWidget * XAP_UnixGnomeDialog_ClipArt::_constructPreviewPane ()
{
	GtkWidget * clipArt = gnome_icon_list_new_flags (ICON_WIDTH,
													 NULL,
													 0);

	
	gnome_icon_list_set_selection_mode (GNOME_ICON_LIST (clipArt), 
										GTK_SELECTION_SINGLE);

	
	gnome_icon_list_append (GNOME_ICON_LIST (clipArt),
							getInitialDir(), getInitialDir());

	return clipArt;
}
#endif

void XAP_UnixGnomeDialog_ClipArt::runModal(XAP_Frame * pFrame)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	GtkWidget * mainWindow = gnome_dialog_new (pSS->getValue (XAP_STRING_ID_DLG_CLIPART_Title),
											   GNOME_STOCK_BUTTON_OK, 
											   GNOME_STOCK_BUTTON_CANCEL, 
											   NULL);
	UT_ASSERT(mainWindow);
	
	m_dialog = mainWindow;

	GtkWidget * clipArt = _constructPreviewPane ();

	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG(mainWindow)->vbox), clipArt, 
						TRUE, FALSE, 0);
	
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

	// load after the show_all to give the impression that we're 
	// loading the icons
	gtk_widget_show_all (mainWindow);
	gnome_icon_selection_show_icons (GNOME_ICON_SELECTION(clipArt));
	
	gint val = gnome_dialog_run (GNOME_DIALOG(mainWindow));
	if (val == 0) /// ok btn
	{
		const gchar * graphic = gnome_icon_selection_get_icon(GNOME_ICON_SELECTION(clipArt), 
															  TRUE);
		if (graphic) {
			setGraphicName (graphic);
			setAnswer (XAP_Dialog_ClipArt::a_OK);
		}
		else {
			setAnswer (XAP_Dialog_ClipArt::a_CANCEL);
		}
	}
	else 
	{
		setAnswer (XAP_Dialog_ClipArt::a_CANCEL);
	}
	
	if (val != -1)
		gtk_object_destroy (GTK_OBJECT (mainWindow));
	
	return;
}

