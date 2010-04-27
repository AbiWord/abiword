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

#include <stdlib.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_MergeCells.h"
#include "ap_UnixDialog_MergeCells.h"
#include "ap_UnixDialog_Columns.h"

static void s_merge_left(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_MergeCells * dlg = static_cast<AP_UnixDialog_MergeCells *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->setMergeType(AP_Dialog_MergeCells::radio_left);
	dlg->onMerge();
}

static void s_merge_right(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_MergeCells * dlg = static_cast<AP_UnixDialog_MergeCells *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->setMergeType(AP_Dialog_MergeCells::radio_right);
	dlg->onMerge();
}

static void s_response(GtkWidget * wid, gint /*id*/, AP_UnixDialog_MergeCells * /*me*/ )
{
    abiDestroyWidget( wid ) ;// will emit signals for us
}

static void s_merge_above(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_MergeCells * dlg = static_cast<AP_UnixDialog_MergeCells *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->setMergeType(AP_Dialog_MergeCells::radio_above);
	dlg->onMerge();
}

static void s_merge_below(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_MergeCells * dlg = static_cast<AP_UnixDialog_MergeCells *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->setMergeType(AP_Dialog_MergeCells::radio_below);
	dlg->onMerge();
}

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_MergeCells::static_constructor(XAP_DialogFactory * pFactory,
													       XAP_Dialog_Id id)
{
	return new AP_UnixDialog_MergeCells(pFactory,id);
}

AP_UnixDialog_MergeCells::AP_UnixDialog_MergeCells(XAP_DialogFactory * pDlgFactory,
										             XAP_Dialog_Id id)
	: AP_Dialog_MergeCells(pDlgFactory,id)
{
	m_windowMain = NULL;
}

AP_UnixDialog_MergeCells::~AP_UnixDialog_MergeCells(void)
{
}

void AP_UnixDialog_MergeCells::runModeless(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);

	// Populate the window's data items
	_populateWindowData();
	_connectSignals();
	abiSetupModelessDialog(GTK_DIALOG(mainWindow),pFrame,this,BUTTON_CLOSE);
	startUpdater();
}

void AP_UnixDialog_MergeCells::setSensitivity(AP_Dialog_MergeCells::mergeWithCell mergeThis, bool bSens)
{
	switch(mergeThis)
	{
	case AP_Dialog_MergeCells::radio_left:
		gtk_widget_set_sensitive( m_wMergeLeft, bSens);
		gtk_widget_set_sensitive( m_lwMergeLeft, bSens);
		break;
	case AP_Dialog_MergeCells::radio_right:
		gtk_widget_set_sensitive( m_wMergeRight, bSens);
		gtk_widget_set_sensitive( m_lwMergeRight, bSens);
		break;
	case AP_Dialog_MergeCells::radio_above:
		gtk_widget_set_sensitive( m_wMergeAbove, bSens);
		gtk_widget_set_sensitive( m_lwMergeAbove, bSens);
		break;
	case AP_Dialog_MergeCells::radio_below:
		gtk_widget_set_sensitive( m_wMergeBelow, bSens);
		gtk_widget_set_sensitive( m_lwMergeBelow, bSens);
		break;
	default:
		break;
	}
}

void AP_UnixDialog_MergeCells::event_Close(void)
{
	m_answer = AP_Dialog_MergeCells::a_CANCEL;
	destroy();
}

void AP_UnixDialog_MergeCells::destroy(void)
{
	finalize();
	gtk_widget_destroy(m_windowMain);
	m_windowMain = NULL;
}
void AP_UnixDialog_MergeCells::activate(void)
{
	UT_ASSERT (m_windowMain);
        
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_windowMain), m_WindowName);
	setAllSensitivities();
	gdk_window_raise (m_windowMain->window);
}

void AP_UnixDialog_MergeCells::notifyActiveFrame(XAP_Frame * /*pFrame*/)
{
    UT_ASSERT(m_windowMain);
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_windowMain), m_WindowName);
	setAllSensitivities();
}

/*****************************************************************/

GtkWidget * AP_UnixDialog_MergeCells::_constructWindow(void)
{
	GtkWidget * vboxMain;
	GtkWidget * windowMergeCells;
	ConstructWindowName();
	windowMergeCells = abiDialogNew ( "merge cell dialog", TRUE, static_cast<char *>(m_WindowName));
	
	vboxMain = GTK_DIALOG(windowMergeCells)->vbox ;
	gtk_container_set_border_width (GTK_CONTAINER (vboxMain), 10);	
	_constructWindowContents();
	gtk_box_pack_start (GTK_BOX (vboxMain), m_wContents, FALSE, FALSE, 0);
	abiAddStockButton ( GTK_DIALOG(windowMergeCells), GTK_STOCK_CLOSE, BUTTON_CLOSE ) ;

	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_windowMain = windowMergeCells;

	return windowMergeCells;
}

GtkWidget * AP_UnixDialog_MergeCells::_constructWindowContents(void)
{
	GtkWidget *wContents;

	wContents = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (wContents);
	GtkWidget *frame1;
	GtkWidget *table1;
	GtkWidget *wlMergeLeft;
	GtkWidget *wlMergeRight;
	GtkWidget *wlMergeAbove;
	GtkWidget *wlMergeBelow;
	GtkWidget *wMergeLeft;
	GtkWidget *wMergeRight;
	GtkWidget *wMergeAbove;
	GtkWidget *wMergeBelow;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	UT_UTF8String s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_MergeCells_Frame,s);
	frame1 = gtk_frame_new (s.utf8_str());
	gtk_widget_show (frame1);
	gtk_container_add (GTK_CONTAINER (wContents), frame1);
	gtk_container_set_border_width (GTK_CONTAINER (frame1), 3);
	gtk_frame_set_shadow_type(GTK_FRAME(frame1), GTK_SHADOW_NONE);

	table1 = gtk_table_new (4, 2, FALSE);
	gtk_widget_show (table1);
	gtk_container_add (GTK_CONTAINER (frame1), table1);
	gtk_table_set_col_spacings (GTK_TABLE (table1), 2);

	pSS->getValueUTF8(AP_STRING_ID_DLG_MergeCells_Left,s);
	wlMergeLeft = gtk_label_new (s.utf8_str());
	gtk_widget_show (wlMergeLeft);
	gtk_table_attach (GTK_TABLE (table1), wlMergeLeft, 0, 1, 0, 1,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (wlMergeLeft), 0, 0.5);
	pSS->getValueUTF8(AP_STRING_ID_DLG_MergeCells_Right,s);
	wlMergeRight = gtk_label_new (s.utf8_str());
	gtk_widget_show (wlMergeRight);
	gtk_table_attach (GTK_TABLE (table1), wlMergeRight, 0, 1, 1, 2,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (wlMergeRight), 0, 0.5);

	pSS->getValueUTF8(AP_STRING_ID_DLG_MergeCells_Above,s);
	wlMergeAbove = gtk_label_new (s.utf8_str());
	gtk_widget_show (wlMergeAbove);
	gtk_table_attach (GTK_TABLE (table1), wlMergeAbove, 0, 1, 2, 3,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (wlMergeAbove), 0, 0.5);

	pSS->getValueUTF8(AP_STRING_ID_DLG_MergeCells_Below,s);
	wlMergeBelow = gtk_label_new (s.utf8_str());
	gtk_widget_show (wlMergeBelow);
	gtk_table_attach (GTK_TABLE (table1), wlMergeBelow, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (wlMergeBelow), 0, 0.5);

	wMergeLeft = gtk_button_new();
	gtk_widget_show (wMergeLeft);
	label_button_with_abi_pixmap(wMergeLeft, "tb_MergeLeft_xpm");

	gtk_table_attach (GTK_TABLE (table1), wMergeLeft, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 3, 0);


	wMergeRight = gtk_button_new();
	gtk_widget_show (wMergeRight);
	label_button_with_abi_pixmap(wMergeRight, "tb_MergeRight_xpm");
	gtk_table_attach (GTK_TABLE (table1), wMergeRight, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 3, 0);

	wMergeAbove = gtk_button_new();
	gtk_widget_show (wMergeAbove);
	label_button_with_abi_pixmap(wMergeAbove, "tb_MergeAbove_xpm");

	gtk_table_attach (GTK_TABLE (table1), wMergeAbove, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 3, 0);


	wMergeBelow = gtk_button_new();
	gtk_widget_show (wMergeBelow);
	label_button_with_abi_pixmap(wMergeBelow, "tb_MergeBelow_xpm");

	gtk_table_attach (GTK_TABLE (table1), wMergeBelow, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 3, 0);

	m_wMergeLeft = wMergeLeft;
	m_wMergeRight = wMergeRight;
	m_wMergeAbove = wMergeAbove;
	m_wMergeBelow = wMergeBelow;
	m_lwMergeLeft = wlMergeLeft;
	m_lwMergeRight = wlMergeRight;
	m_lwMergeAbove = wlMergeAbove;
	m_lwMergeBelow = wlMergeBelow;

	m_wContents = wContents;
	
	return m_wContents;
}

static void s_destroy_clicked(GtkWidget * /* widget */,
			      AP_UnixDialog_MergeCells * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_Close();
}


static void s_delete_clicked(GtkWidget * widget,
			     gpointer,
			     gpointer * /*dlg*/)
{
	abiDestroyWidget(widget);
}

void AP_UnixDialog_MergeCells::_connectSignals(void)
{
  g_signal_connect(G_OBJECT(m_windowMain), "response", 
		   G_CALLBACK(s_response), this);

	// the catch-alls
	// Dont use gtk_signal_connect_after for modeless dialogs
	g_signal_connect(G_OBJECT(m_windowMain),
			   "destroy",
			   G_CALLBACK(s_destroy_clicked),
			   static_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(m_windowMain),
			   "delete_event",
			   G_CALLBACK(s_delete_clicked),
			   static_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_wMergeLeft),
						   "clicked",
						   G_CALLBACK(s_merge_left),
						   static_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(m_wMergeRight),
						   "clicked",
						   G_CALLBACK(s_merge_right),
						   static_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(m_wMergeAbove),
						   "clicked",
						   G_CALLBACK(s_merge_above),
						   static_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(m_wMergeBelow),
						   "clicked",
						   G_CALLBACK(s_merge_below),
						   static_cast<gpointer>(this));

}

void AP_UnixDialog_MergeCells::_populateWindowData(void)
{
   setAllSensitivities();
}

void AP_UnixDialog_MergeCells::_storeWindowData(void)
{
}
