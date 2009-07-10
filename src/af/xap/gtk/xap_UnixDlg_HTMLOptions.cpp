/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 2002 AbiSource, Inc.
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

#include "xap_Frame.h"
#include "xap_UnixApp.h"
#include "xap_UnixDlg_HTMLOptions.h"

XAP_Dialog * XAP_UnixDialog_HTMLOptions::static_constructor (XAP_DialogFactory * pDF,
															 XAP_Dialog_Id id)
{
	return new XAP_UnixDialog_HTMLOptions(pDF,id);
}

XAP_UnixDialog_HTMLOptions::XAP_UnixDialog_HTMLOptions (XAP_DialogFactory * pDlgFactory,
														XAP_Dialog_Id id)
	: XAP_Dialog_HTMLOptions(pDlgFactory,id),
	  m_windowMain(NULL),
	  m_wIs4(NULL),
	  m_wAbiWebDoc(NULL),
	  m_wDeclareXML(NULL),
	  m_wAllowAWML(NULL),
	  m_wEmbedCSS(NULL),
	  m_wEmbedImages(NULL)
{
	// 
}

XAP_UnixDialog_HTMLOptions::~XAP_UnixDialog_HTMLOptions ()
{
	// 
}

typedef enum
{
	BUTTON_OK,
	BUTTON_SAVE_SETTINGS,
	BUTTON_RESTORE_SETTINGS,
	BUTTON_CANCEL
} ResponseId;

void XAP_UnixDialog_HTMLOptions::runModal (XAP_Frame * pFrame)
{
	if (pFrame == NULL) return;

	/* Build the window's widgets and arrange them
	 */
	GtkWidget * mainWindow = _constructWindow ();

	if (mainWindow == NULL) return;

	bool stop = false;
	while (!stop)
		switch (abiRunModalDialog (GTK_DIALOG (mainWindow), pFrame, this, BUTTON_OK, false))
			{
			case BUTTON_OK:
				event_OK ();
				stop = true;
				break;
			case BUTTON_SAVE_SETTINGS:
				event_SaveSettings ();
				break;
			case BUTTON_RESTORE_SETTINGS:
				event_RestoreSettings ();
				break;
			case BUTTON_CANCEL:
			default:
				event_Cancel ();
				stop = true;
				break;
			}
	abiDestroyWidget (mainWindow);
}

void XAP_UnixDialog_HTMLOptions::toggle_Is4 ()
{
	bool on = (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (m_wIs4)) == TRUE);
	set_HTML4 (on);
	refreshStates ();
}

void XAP_UnixDialog_HTMLOptions::toggle_AbiWebDoc ()
{
	bool on = (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (m_wAbiWebDoc)) == TRUE);
	set_PHTML (on);
	refreshStates ();
}

void XAP_UnixDialog_HTMLOptions::toggle_DeclareXML ()
{
	bool on = (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (m_wDeclareXML)) == TRUE);
	set_Declare_XML (on);
	refreshStates ();
}

void XAP_UnixDialog_HTMLOptions::toggle_AllowAWML ()
{
	bool on = (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (m_wAllowAWML)) == TRUE);
	set_Allow_AWML (on);
	refreshStates ();
}

void XAP_UnixDialog_HTMLOptions::toggle_EmbedCSS ()
{
	bool on = (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (m_wEmbedCSS)) == TRUE);
	set_Embed_CSS (on);
	refreshStates ();
}

void XAP_UnixDialog_HTMLOptions::toggle_EmbedImages ()
{
	bool on = (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (m_wEmbedImages)) == TRUE);
	set_Embed_Images (on);
	refreshStates ();
}

void XAP_UnixDialog_HTMLOptions::refreshStates ()
{
	gboolean on;

	on = get_HTML4 () ? TRUE : FALSE;
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_wIs4), on);

	on = get_PHTML () ? TRUE : FALSE;
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_wAbiWebDoc), on);

	on = get_Declare_XML () ? TRUE : FALSE;
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_wDeclareXML), on);

	on = can_set_Declare_XML () ? TRUE : FALSE;
	gtk_widget_set_sensitive (m_wDeclareXML, on);

	on = get_Allow_AWML () ? TRUE : FALSE;
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_wAllowAWML), on);

	on = can_set_Allow_AWML () ? TRUE : FALSE;
	gtk_widget_set_sensitive (m_wAllowAWML, on);

	on = get_Embed_CSS () ? TRUE : FALSE;
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_wEmbedCSS), on);

	on = can_set_Embed_CSS () ? TRUE : FALSE;
	gtk_widget_set_sensitive (m_wEmbedCSS, on);

	on = get_Embed_Images () ? TRUE : FALSE;
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_wEmbedImages), on);

	on = can_set_Embed_Images () ? TRUE : FALSE;
	gtk_widget_set_sensitive (m_wEmbedImages, on);
}

void XAP_UnixDialog_HTMLOptions::event_OK ()
{
	m_bShouldSave = true;
}

void XAP_UnixDialog_HTMLOptions::event_SaveSettings ()
{
	saveDefaults ();
	refreshStates ();
}

void XAP_UnixDialog_HTMLOptions::event_RestoreSettings ()
{
	restoreDefaults ();
	refreshStates ();
}

void XAP_UnixDialog_HTMLOptions::event_Cancel ()
{
	m_bShouldSave = false;
}

static void s_Is4 (GtkWidget * /* w */, XAP_UnixDialog_HTMLOptions * dlg)
{
	dlg->toggle_Is4 ();
}

static void s_AbiWebDoc (GtkWidget * /* w */, XAP_UnixDialog_HTMLOptions * dlg)
{
	dlg->toggle_AbiWebDoc ();
}

static void s_DeclareXML (GtkWidget * /* w */, XAP_UnixDialog_HTMLOptions * dlg)
{
	dlg->toggle_DeclareXML ();
}

static void s_AllowAWML (GtkWidget * /* w */, XAP_UnixDialog_HTMLOptions * dlg)
{
	dlg->toggle_AllowAWML ();
}

static void s_EmbedCSS (GtkWidget * /* w */, XAP_UnixDialog_HTMLOptions * dlg)
{
	dlg->toggle_EmbedCSS ();
}

static void s_EmbedImages (GtkWidget * /* w */, XAP_UnixDialog_HTMLOptions * dlg)
{
	dlg->toggle_EmbedImages ();
}

GtkWidget * XAP_UnixDialog_HTMLOptions::_constructWindow ()
{
	const XAP_StringSet * pSS = m_pApp->getStringSet ();

	const char * title   = static_cast<const char *>(pSS->getValue (XAP_STRING_ID_DLG_HTMLOPT_ExpTitle));
	const char * label   = static_cast<const char *>(pSS->getValue (XAP_STRING_ID_DLG_HTMLOPT_ExpLabel));
	const char * save    = static_cast<const char *>(pSS->getValue (XAP_STRING_ID_DLG_HTMLOPT_ExpSave));
	const char * restore = static_cast<const char *>(pSS->getValue (XAP_STRING_ID_DLG_HTMLOPT_ExpRestore));

	const char * Is4         = static_cast<const char *>(pSS->getValue (XAP_STRING_ID_DLG_HTMLOPT_ExpIs4));
	const char * AbiWebDoc   = static_cast<const char *>(pSS->getValue (XAP_STRING_ID_DLG_HTMLOPT_ExpAbiWebDoc));
	const char * DeclareXML  = static_cast<const char *>(pSS->getValue (XAP_STRING_ID_DLG_HTMLOPT_ExpDeclareXML));
	const char * AllowAWML   = static_cast<const char *>(pSS->getValue (XAP_STRING_ID_DLG_HTMLOPT_ExpAllowAWML));
	const char * EmbedCSS    = static_cast<const char *>(pSS->getValue (XAP_STRING_ID_DLG_HTMLOPT_ExpEmbedCSS));
	const char * EmbedImages = static_cast<const char *>(pSS->getValue (XAP_STRING_ID_DLG_HTMLOPT_ExpEmbedImages));

	/* This is the top level GTK widget, the window.
	 * It's created with a "dialog" style.
	 */
	m_windowMain = abiDialogNew ("HTML export options dialog", true, title);

	if (m_windowMain == NULL) return NULL;

	/* This is the top level organization widget, which packs things vertically
	 */
	GtkWidget * vboxMain = GTK_DIALOG(m_windowMain)->vbox;

	/* The top item in the vbox is a simple label
	 */
	GtkWidget * labelActivate = gtk_label_new (label);
	if (labelActivate)
		{
			gtk_widget_show (labelActivate);
			gtk_box_pack_start (GTK_BOX (vboxMain), labelActivate, FALSE, TRUE, 0);
			gtk_label_set_justify (GTK_LABEL (labelActivate), GTK_JUSTIFY_LEFT);
			gtk_misc_set_alignment (GTK_MISC (labelActivate), 0, 0);
			gtk_misc_set_padding (GTK_MISC (labelActivate), 10, 5);
		}

	m_wIs4 = gtk_check_button_new_with_label (Is4);
	if (m_wIs4)
		{
			gtk_container_set_border_width (GTK_CONTAINER (m_wIs4), 5);
			gtk_widget_show (m_wIs4);
			gtk_box_pack_start (GTK_BOX (vboxMain), m_wIs4, TRUE, TRUE, 0);
			g_signal_connect (G_OBJECT (m_wIs4), "toggled",
							  G_CALLBACK (s_Is4), static_cast<gpointer>(this));
		}
	m_wAbiWebDoc = gtk_check_button_new_with_label (AbiWebDoc);
	if (m_wAbiWebDoc)
		{
			gtk_container_set_border_width (GTK_CONTAINER (m_wAbiWebDoc), 5);
			gtk_widget_show (m_wAbiWebDoc);
			gtk_box_pack_start (GTK_BOX (vboxMain), m_wAbiWebDoc, TRUE, TRUE, 0);
			g_signal_connect (G_OBJECT (m_wAbiWebDoc), "toggled",
							  G_CALLBACK (s_AbiWebDoc), static_cast<gpointer>(this));
		}
	m_wDeclareXML = gtk_check_button_new_with_label (DeclareXML);
	if (m_wDeclareXML)
		{
			gtk_container_set_border_width (GTK_CONTAINER (m_wDeclareXML), 5);
			gtk_widget_show (m_wDeclareXML);
			gtk_box_pack_start (GTK_BOX (vboxMain), m_wDeclareXML, TRUE, TRUE, 0);
			g_signal_connect (G_OBJECT (m_wDeclareXML), "toggled",
							  G_CALLBACK (s_DeclareXML), static_cast<gpointer>(this));
		}
	m_wAllowAWML = gtk_check_button_new_with_label (AllowAWML);
	if (m_wAllowAWML)
		{
			gtk_container_set_border_width (GTK_CONTAINER (m_wAllowAWML), 5);
			gtk_widget_show (m_wAllowAWML);
			gtk_box_pack_start (GTK_BOX (vboxMain), m_wAllowAWML, TRUE, TRUE, 0);
			g_signal_connect (G_OBJECT (m_wAllowAWML), "toggled",
							  G_CALLBACK (s_AllowAWML), static_cast<gpointer>(this));
		}
	m_wEmbedCSS = gtk_check_button_new_with_label (EmbedCSS);
	if (m_wEmbedCSS)
		{
			gtk_container_set_border_width (GTK_CONTAINER (m_wEmbedCSS), 5);
			gtk_widget_show (m_wEmbedCSS);
			gtk_box_pack_start (GTK_BOX (vboxMain), m_wEmbedCSS, TRUE, TRUE, 0);
			g_signal_connect (G_OBJECT (m_wEmbedCSS), "toggled",
							  G_CALLBACK (s_EmbedCSS), static_cast<gpointer>(this));
		}
	m_wEmbedImages = gtk_check_button_new_with_label (EmbedImages);
	if (m_wEmbedImages)
		{
			gtk_container_set_border_width (GTK_CONTAINER (m_wEmbedImages), 5);
			gtk_widget_show (m_wEmbedImages);
			gtk_box_pack_start (GTK_BOX (vboxMain), m_wEmbedImages, TRUE, TRUE, 0);
			g_signal_connect (G_OBJECT (m_wEmbedImages), "toggled",
							  G_CALLBACK (s_EmbedImages), static_cast<gpointer>(this));
		}

	refreshStates ();

	abiAddStockButton (GTK_DIALOG(m_windowMain), save,    BUTTON_SAVE_SETTINGS);
	abiAddStockButton (GTK_DIALOG(m_windowMain), restore, BUTTON_RESTORE_SETTINGS);

	abiAddStockButton (GTK_DIALOG(m_windowMain), GTK_STOCK_CANCEL, BUTTON_CANCEL);
	abiAddStockButton (GTK_DIALOG(m_windowMain), GTK_STOCK_OK,     BUTTON_OK);
  
	return m_windowMain;
}
