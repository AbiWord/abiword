/* AbiSource
 * 
 * Copyright (C) 2011 Volodymyr Rudyj <vladimir.rudoy@gmail.com>
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

#include "ap_UnixDialog_EpubExportOptions.h"

pt2Constructor ap_Dialog_EpubExportOptions_Constructor =
    AP_UnixDialog_EpubExportOptions::static_constructor;

XAP_Dialog * AP_UnixDialog_EpubExportOptions::static_constructor(
    XAP_DialogFactory* pDF, XAP_Dialog_Id id)
{
    return new AP_UnixDialog_EpubExportOptions(pDF,id);
}


AP_UnixDialog_EpubExportOptions::AP_UnixDialog_EpubExportOptions(
    XAP_DialogFactory* pDlgFactory, XAP_Dialog_Id id)
    : AP_Dialog_EpubExportOptions(pDlgFactory,id),
	  m_windowMain(NULL),
	  m_wEpub2(NULL),
	  m_wSplitDocument(NULL),
      m_wRenderMathMlToPng(NULL)
{
    
}


AP_UnixDialog_EpubExportOptions::~AP_UnixDialog_EpubExportOptions()
{
    
}

typedef enum
{
	BUTTON_OK,
	BUTTON_SAVE_SETTINGS,
	BUTTON_RESTORE_SETTINGS,
	BUTTON_CANCEL
} ResponseId;

void AP_UnixDialog_EpubExportOptions::runModal (XAP_Frame * pFrame)
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

void AP_UnixDialog_EpubExportOptions::toggle_Epub2()
{
	bool on = (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (m_wEpub2)) == TRUE);
	set_Epub2 (on);
	refreshStates ();
}

void AP_UnixDialog_EpubExportOptions::toggle_SplitDocument()
{
	bool on = (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (m_wSplitDocument)) == TRUE);
	set_SplitDocument (on);
	refreshStates ();
}

void AP_UnixDialog_EpubExportOptions::toggle_RenderMathMlToPng()
{
	bool on = (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (m_wRenderMathMlToPng)) == TRUE);
	set_RenderMathMlToPng (on);
	refreshStates ();
}

void AP_UnixDialog_EpubExportOptions::refreshStates()
{
	gboolean on;

	on = get_Epub2 () ? TRUE : FALSE;
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_wEpub2), on);

	on = can_set_Epub2 () ? TRUE : FALSE;
	gtk_widget_set_sensitive (m_wEpub2, on);

	on = get_SplitDocument () ? TRUE : FALSE;
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_wSplitDocument), on);

	on = can_set_SplitDocument () ? TRUE : FALSE;
	gtk_widget_set_sensitive (m_wSplitDocument, on);

	on = get_RenderMathMlToPng () ? TRUE : FALSE;
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_wRenderMathMlToPng), on);

	on = can_set_RenderMathMlToPng () ? TRUE : FALSE;
	gtk_widget_set_sensitive (m_wRenderMathMlToPng, on);

}

void AP_UnixDialog_EpubExportOptions::event_OK ()
{
	m_bShouldSave = true;
}

void AP_UnixDialog_EpubExportOptions::event_SaveSettings ()
{
	saveDefaults ();
	refreshStates ();
}

void AP_UnixDialog_EpubExportOptions::event_RestoreSettings ()
{
	restoreDefaults ();
	refreshStates ();
}

void AP_UnixDialog_EpubExportOptions::event_Cancel ()
{
	m_bShouldSave = false;
}

static void s_Epub2 (GtkWidget * /* w */, AP_UnixDialog_EpubExportOptions * dlg)
{
	dlg->toggle_Epub2();
}

static void s_SplitDocument(GtkWidget * /* w */, AP_UnixDialog_EpubExportOptions * dlg)
{
	dlg->toggle_SplitDocument();
}

static void s_RenderMathMlToPng (GtkWidget * /* w */, AP_UnixDialog_EpubExportOptions * dlg)
{
	dlg->toggle_RenderMathMlToPng ();
}

GtkWidget * AP_UnixDialog_EpubExportOptions::_constructWindow ()
{
	const XAP_StringSet * pSS = m_pApp->getStringSet ();

	const char * title   = "EPUB Export Options";
	const char * label   = "Select EPUB export options:";
	const char * save    = static_cast<const char *>(pSS->getValue (XAP_STRING_ID_DLG_HTMLOPT_ExpSave));
	const char * restore = static_cast<const char *>(pSS->getValue (XAP_STRING_ID_DLG_HTMLOPT_ExpRestore));

	const char * Epub2              = "EPUB 2.0.1";
	const char * SplitDocument      = "Split document";
	const char * RenderMathMlToPng  = "Use PNG instead of MathML";


	/* This is the top level GTK widget, the window.
	 * It's created with a "dialog" style.
	 */
	m_windowMain = abiDialogNew ("EPUB export options dialog", true, title);

	if (m_windowMain == NULL) return NULL;

	/* This is the top level organization widget, which packs things vertically
	 */
	GtkWidget * vboxMain = gtk_dialog_get_content_area(GTK_DIALOG(m_windowMain));

	/* The top item in the vbox is a simple label
	 */
	GtkWidget * labelActivate = gtk_widget_new (GTK_TYPE_LABEL,
						    "label", label,
						    "justify", GTK_JUSTIFY_LEFT,
						    "xalign", 0.0, "yalign", 0.0,
						    "xpad", 10, "ypad", 5,
						    NULL);
	if (labelActivate)
	{
		gtk_widget_show (labelActivate);
		gtk_box_pack_start (GTK_BOX (vboxMain), labelActivate, FALSE, TRUE, 0);
	}

	m_wEpub2 = gtk_check_button_new_with_label (Epub2);
	if (m_wEpub2)
	{
		gtk_container_set_border_width(GTK_CONTAINER(m_wEpub2), 5);
		gtk_widget_show(m_wEpub2);
		gtk_box_pack_start(GTK_BOX(vboxMain), m_wEpub2, TRUE, TRUE, 0);
	        g_signal_connect(G_OBJECT(m_wEpub2), "toggled",
                         G_CALLBACK(s_Epub2), static_cast<gpointer> (this));
	}

	m_wSplitDocument = gtk_check_button_new_with_label (SplitDocument);
	if (m_wSplitDocument)
    {
        gtk_container_set_border_width(GTK_CONTAINER(m_wSplitDocument), 5);
        gtk_widget_show(m_wSplitDocument);
        gtk_box_pack_start(GTK_BOX(vboxMain), m_wSplitDocument, TRUE, TRUE, 0);
        g_signal_connect(G_OBJECT(m_wSplitDocument), "toggled",
                         G_CALLBACK(s_SplitDocument), static_cast<gpointer> (this));
    }
    
	m_wRenderMathMlToPng = gtk_check_button_new_with_label (RenderMathMlToPng);
	if (m_wRenderMathMlToPng) 
    {
        gtk_container_set_border_width(GTK_CONTAINER(m_wRenderMathMlToPng), 5);
        gtk_widget_show(m_wRenderMathMlToPng);
        gtk_box_pack_start(GTK_BOX(vboxMain), m_wRenderMathMlToPng, TRUE, TRUE, 0);
        g_signal_connect(G_OBJECT(m_wRenderMathMlToPng), "toggled",
                         G_CALLBACK(s_RenderMathMlToPng), static_cast<gpointer> (this));
    }
    
	refreshStates ();

	abiAddStockButton (GTK_DIALOG(m_windowMain), save,    BUTTON_SAVE_SETTINGS);
	abiAddStockButton (GTK_DIALOG(m_windowMain), restore, BUTTON_RESTORE_SETTINGS);

	abiAddStockButton (GTK_DIALOG(m_windowMain), GTK_STOCK_CANCEL, BUTTON_CANCEL);
	abiAddStockButton (GTK_DIALOG(m_windowMain), GTK_STOCK_OK,     BUTTON_OK);
  
	return m_windowMain;
}
