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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_WordCount.h"
#include "ap_UnixDialog_WordCount.h"

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

static UT_sint32 i_WordCountunix_first_time = 0;

XAP_Dialog * AP_UnixDialog_WordCount::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	AP_UnixDialog_WordCount * p = new AP_UnixDialog_WordCount(pFactory,id);
	return p;
}

AP_UnixDialog_WordCount::AP_UnixDialog_WordCount(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: AP_Dialog_WordCount(pDlgFactory,id)
{
	m_windowMain = NULL;
	m_buttonClose = NULL;
	m_wContent = NULL;
	m_pTableframe = NULL;
	m_buttonUpdate = NULL;
}

AP_UnixDialog_WordCount::~AP_UnixDialog_WordCount(void)
{
}

/*****************************************************************/

static void s_autocheck_clicked(GtkWidget * widget, AP_UnixDialog_WordCount * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Checkbox();
}

static void s_updateRate_changed(GtkWidget * widget, 
								 AP_UnixDialog_WordCount * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Spin();
}

/*****************************************************************/

void  AP_UnixDialog_WordCount::activate(void)
{
	UT_ASSERT (m_windowMain);
        
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_windowMain), m_WindowName);
	setCountFromActiveFrame ();
	_updateWindowData ();
	gdk_window_raise (m_windowMain->window);
}

void AP_UnixDialog_WordCount::s_response(GtkWidget * wid, gint id, AP_UnixDialog_WordCount * me )
{
  if ( id == BUTTON_CLOSE )
    abiDestroyWidget( wid ) ;// will emit signals for us
  else if ( id == BUTTON_UPDATE )
    me->event_Update () ;
}

void AP_UnixDialog_WordCount::runModeless(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();

	UT_ASSERT(mainWindow);

	abiSetupModelessDialog(GTK_DIALOG(mainWindow), pFrame, this, BUTTON_CLOSE);

	// Show the top level dialog,
	gtk_widget_show(mainWindow);

	// Now construct the timer for auto-updating
	GR_Graphics * pG = NULL;
	m_pAutoUpdateWC = UT_Timer::static_constructor(autoupdateWC,this,pG);

	if(i_WordCountunix_first_time == 0)
	{	
		//  Set it update evey second to start with
		m_Update_rate = 1000;
		m_bAutoWC = true;
		i_WordCountunix_first_time = 1;
	}

	setUpdateCounter();
}

void    AP_UnixDialog_WordCount::setUpdateCounter( void )
{
	m_bDestroy_says_stopupdating = false;
	m_bAutoUpdate_happening_now = false;

	gfloat f_Update_rate = ((gfloat) m_Update_rate)/ 1000.0;
	gtk_adjustment_set_value( m_Spinrange, f_Update_rate );
	gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON( m_pAutospin), m_Spinrange);
	if(m_bAutoWC == true)
	{
		m_pAutoUpdateWC->stop();
		m_pAutoUpdateWC->set(m_Update_rate);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_pAutocheck), TRUE);
	}
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_pAutocheck), FALSE);
	}
	set_sensitivity();
}
         
void    AP_UnixDialog_WordCount::autoupdateWC(UT_Worker * pTimer)
{
	UT_ASSERT(pTimer);

	// this is a static callback method and does not have a 'this' pointer.

	AP_UnixDialog_WordCount * pDialog =  (AP_UnixDialog_WordCount *) pTimer->getInstanceData();

	// Handshaking code

	if( pDialog->m_bDestroy_says_stopupdating != true)
	{
		pDialog->m_bAutoUpdate_happening_now = true;
		pDialog->event_Update();
		pDialog->m_bAutoUpdate_happening_now = false;
	}
}        

void AP_UnixDialog_WordCount::event_OK(void)
{
	m_answer = AP_Dialog_WordCount::a_OK;
	destroy();
}

void AP_UnixDialog_WordCount::event_Update(void)
{
	setCountFromActiveFrame();
	_updateWindowData();
}

void  AP_UnixDialog_WordCount::set_sensitivity(void)
{
	if(m_bAutoWC == true)
	{
		gtk_widget_set_sensitive(m_buttonUpdate,FALSE);
		gtk_widget_set_sensitive(m_pAutospin,TRUE);
		gtk_widget_set_sensitive(m_pAutospinlabel,TRUE);
	}
	else
	{
		gtk_widget_set_sensitive(m_buttonUpdate,TRUE);
		gtk_widget_set_sensitive(m_pAutospin,FALSE);
		gtk_widget_set_sensitive(m_pAutospinlabel,FALSE);
	}
}

void AP_UnixDialog_WordCount::event_Checkbox(void)
{
	if(GTK_TOGGLE_BUTTON( m_pAutocheck)->active)
	{
		m_pAutoUpdateWC->stop();
		// This actually does gtk_timer_add...
		m_pAutoUpdateWC->set(m_Update_rate);
		m_bAutoWC = true;
	}
	else
	{
		m_pAutoUpdateWC->stop();
		m_bAutoWC = false;
	}
	set_sensitivity();
}

void AP_UnixDialog_WordCount::event_Spin(void)
{
	gfloat update_rate =  1000.0*gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(m_pAutospin));
	m_Update_rate = (guint) update_rate;

	// We need this because calling adds a new timer to the gtk list!
	// So we have to stop the timer to remove it from the gtk list before
	// changing the speed of the timer.

	m_pAutoUpdateWC->stop();

	// This actually does gtk_timer_add...

	m_pAutoUpdateWC->set(m_Update_rate);
}

void AP_UnixDialog_WordCount::event_WindowDelete(void)
{
	m_answer = AP_Dialog_WordCount::a_CANCEL;	
	destroy();
}

void AP_UnixDialog_WordCount::notifyActiveFrame(XAP_Frame *pFrame)
{
        UT_ASSERT(m_windowMain);
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_windowMain), m_WindowName);
	event_Update();
}

void AP_UnixDialog_WordCount::destroy(void)
{
	m_bDestroy_says_stopupdating = true;
	while (m_bAutoUpdate_happening_now == true) 
	  ;
	m_pAutoUpdateWC->stop();
	m_answer = AP_Dialog_WordCount::a_CANCEL;	
	modeless_cleanup();
	gtk_widget_destroy(m_windowMain);
	m_windowMain = NULL;
	DELETEP(m_pAutoUpdateWC);
}

/*****************************************************************/

GtkWidget * AP_UnixDialog_WordCount::_constructWindow(void)
{
	GtkWidget * vbox;

        ConstructWindowName();
	m_windowMain = abiDialogNew("word count dialog", TRUE, m_WindowName);

	vbox = GTK_DIALOG(m_windowMain)->vbox;
	gtk_widget_show (vbox);

	_constructWindowContents ();
	gtk_box_pack_start (GTK_BOX (vbox), m_wContent, FALSE, FALSE, 0);

	m_buttonUpdate = abiAddStockButton(GTK_DIALOG(m_windowMain), GTK_STOCK_REFRESH, BUTTON_UPDATE);
	m_buttonClose = abiAddStockButton(GTK_DIALOG(m_windowMain), GTK_STOCK_CLOSE, BUTTON_CLOSE);

	gtk_widget_grab_focus (m_buttonUpdate);
	_connectSignals ();

	gtk_widget_show_all (m_windowMain);
	
	return m_windowMain;
}

void AP_UnixDialog_WordCount::_updateWindowData(void)
{
	//gtk_widget_destroy(m_wContent);
	//GtkWidget * table = _constructWindowContents();

	// Update the data in the word count
	char *tmp;
	char *rtmp;
	tmp = g_strdup_printf ("%d", m_count.word);
	gtk_label_get(GTK_LABEL(m_labelWCount),&rtmp);
	if(UT_stricmp( tmp, rtmp) != 0 )
	{
		gtk_label_set_text(GTK_LABEL(m_labelWCount),tmp);
	}
	g_free (tmp);

	tmp = g_strdup_printf ("%d", m_count.para);
	gtk_label_get(GTK_LABEL(m_labelPCount),&rtmp);
	if(UT_stricmp( tmp, rtmp) != 0 )
	{
		gtk_label_set_text(GTK_LABEL(m_labelPCount),tmp);
	}
	g_free (tmp);

	tmp = g_strdup_printf ("%d", m_count.ch_sp);
	gtk_label_get(GTK_LABEL(m_labelCCount),&rtmp);
	if(UT_stricmp( tmp, rtmp) != 0 )
	{
		gtk_label_set_text(GTK_LABEL(m_labelCCount),tmp);
	}
	g_free (tmp);

	tmp = g_strdup_printf ("%d", m_count.ch_no);
	gtk_label_get(GTK_LABEL(m_labelCNCount),&rtmp);
	if(UT_stricmp( tmp, rtmp) != 0 )
	{
		gtk_label_set_text(GTK_LABEL(m_labelCNCount),tmp);
	}
	g_free (tmp);

	tmp = g_strdup_printf ("%d", m_count.line);
	gtk_label_get(GTK_LABEL(m_labelLCount),&rtmp);
	if(UT_stricmp( tmp, rtmp) != 0 )
	{
		gtk_label_set_text(GTK_LABEL(m_labelLCount),tmp);
	}
	g_free (tmp);

	tmp = g_strdup_printf ("%d", m_count.page);
	gtk_label_get(GTK_LABEL(m_labelPgCount),&rtmp);
	if(UT_stricmp( tmp, rtmp) != 0 )
	{
		gtk_label_set_text(GTK_LABEL(m_labelPgCount),tmp);
	}
	g_free (tmp);

	gtk_frame_set_label (GTK_FRAME(m_pTableframe), getActiveFrame ()->getTitle (60));
	//	gtk_widget_show (m_pTableframe);
}

void AP_UnixDialog_WordCount::_populateWindowData(void)
{

}

GtkWidget * AP_UnixDialog_WordCount::_constructWindowContents(void)
{
	GtkWidget * tophbox;
	GtkWidget * labelWords;
	GtkWidget * labelPara;
	GtkWidget * labelCharNo;	
	GtkWidget * labelChar;
	GtkWidget * labelLine;
	GtkWidget * labelPage;
	GtkWidget * dataTable;
	XML_Char * unixstr = NULL;	// used for conversions
	char *tmp;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	m_wContent = gtk_vbox_new (FALSE, 4);

	// do the checkbox and the spin button
	tophbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(m_wContent), tophbox,TRUE,TRUE,0);

	m_pAutocheck = gtk_check_button_new_with_label(pSS->getValueUTF8(AP_STRING_ID_DLG_WordCount_Auto_Update).c_str());
	gtk_box_pack_start(GTK_BOX(tophbox),m_pAutocheck,TRUE,TRUE,0);
        
	m_Spinrange = (GtkAdjustment *) gtk_adjustment_new(1.0, 0.1, 10.0, 0.1, 1.0, 0.0);
	m_pAutospin = gtk_spin_button_new (m_Spinrange, 1.0, 1);
	gtk_box_pack_start (GTK_BOX (tophbox), m_pAutospin, TRUE, TRUE, 0);
	m_pAutospinlabel = gtk_label_new (pSS->getValueUTF8 (AP_STRING_ID_DLG_WordCount_Update_Rate).c_str());
	gtk_label_set_justify (GTK_LABEL (m_pAutospinlabel), GTK_JUSTIFY_LEFT );
	gtk_box_pack_start (GTK_BOX (tophbox), m_pAutospinlabel, FALSE, TRUE, 0);

	m_pTableframe = gtk_frame_new (NULL);
	gtk_box_pack_start (GTK_BOX (m_wContent), m_pTableframe, TRUE, TRUE, 0);
	//        gtk_widget_set_usize(m_pTableframe,-2,120);
	gtk_frame_set_label (GTK_FRAME (m_pTableframe), getActiveFrame ()->getTitle (60));

	// Constrcut the table full of Word Count information
	// and put it in our frame

	dataTable = gtk_table_new (7, 2, TRUE);	
	gtk_container_set_border_width (GTK_CONTAINER (dataTable), 8);
	gtk_table_set_col_spacings (GTK_TABLE (dataTable), 12);

	// The figures
	tmp = g_strdup_printf ("%d", m_count.word);
	m_labelWCount = gtk_label_new (tmp);
	g_free (tmp);
	gtk_table_attach (GTK_TABLE (dataTable), m_labelWCount, 1, 2, 0, 1,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (m_labelWCount), 1.0, 0.5);
	tmp = g_strdup_printf ("%d", m_count.para);
	m_labelPCount = gtk_label_new (tmp);
	g_free (tmp);
	gtk_table_attach (GTK_TABLE (dataTable), m_labelPCount, 1, 2, 1, 2,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (m_labelPCount), 1.0, 0.5);
	tmp = g_strdup_printf ("%d", m_count.ch_sp);
	m_labelCCount = gtk_label_new (tmp);
	g_free (tmp);
	gtk_table_attach (GTK_TABLE (dataTable), m_labelCCount, 1, 2, 2, 3,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (m_labelCCount), 1.0, 0.5);
	tmp = g_strdup_printf ("%d", m_count.ch_no);
	m_labelCNCount = gtk_label_new (tmp);
	g_free (tmp);
	gtk_table_attach (GTK_TABLE (dataTable), m_labelCNCount, 1, 2, 3, 4,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (m_labelCNCount), 1.0, 0.5);
	tmp = g_strdup_printf ("%d", m_count.line);
	m_labelLCount = gtk_label_new (tmp);
	g_free (tmp);
	gtk_table_attach (GTK_TABLE (dataTable), m_labelLCount, 1, 2, 4, 5,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (m_labelLCount), 1.0, 0.5);
	tmp = g_strdup_printf ("%d", m_count.page);	
	m_labelPgCount = gtk_label_new (tmp);
	g_free (tmp);
	gtk_table_attach (GTK_TABLE (dataTable), m_labelPgCount, 1, 2, 5, 6,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (m_labelPgCount), 1.0, 0.5);
	
	// The labels
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValueUTF8(AP_STRING_ID_DLG_WordCount_Words).c_str());	
	labelWords = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_label_set_justify (GTK_LABEL (labelWords), GTK_JUSTIFY_LEFT);
	gtk_label_set_justify (GTK_LABEL (labelWords), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (dataTable), labelWords, 0, 1, 0, 1,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelWords), 0, 0.5);
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValueUTF8(AP_STRING_ID_DLG_WordCount_Paragraphs).c_str());	
	labelPara = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_table_attach (GTK_TABLE (dataTable), labelPara, 0, 1, 1, 2,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelPara), 0, 0.5);
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValueUTF8(AP_STRING_ID_DLG_WordCount_Characters_Sp).c_str());	
	labelChar = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_table_attach (GTK_TABLE (dataTable), labelChar, 0, 1, 2, 3,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelChar), 0, 0.5);
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValueUTF8(AP_STRING_ID_DLG_WordCount_Characters_No).c_str());	
	labelCharNo = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_table_attach (GTK_TABLE (dataTable), labelCharNo, 0, 1, 3, 4,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelCharNo), 0, 0.5);
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValueUTF8(AP_STRING_ID_DLG_WordCount_Lines).c_str());	
	labelLine = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_table_attach (GTK_TABLE (dataTable), labelLine, 0, 1, 4, 5,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelLine), 0, 0.5);
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValueUTF8(AP_STRING_ID_DLG_WordCount_Pages).c_str());	
	labelPage = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_table_attach (GTK_TABLE (dataTable), labelPage, 0, 1, 5, 6,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelPage), 0, 0.5);

	gtk_container_add (GTK_CONTAINER(m_pTableframe), dataTable);
	gtk_widget_show_all (m_wContent);

	return (m_wContent);
}

static void s_destroy_clicked(GtkWidget * /* widget */,
			      AP_UnixDialog_WordCount * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_OK();
}

static void s_delete_clicked(GtkWidget * widget,
			     gpointer,
			     gpointer * dlg)
{
	abiDestroyWidget(widget);
}

void AP_UnixDialog_WordCount::_connectSignals(void)
{
  g_signal_connect(G_OBJECT(m_windowMain), "response", 
		   G_CALLBACK(s_response), this);

	// the catch-alls
	// Dont use gtk_signal_connect_after for modeless dialogs
	gtk_signal_connect(GTK_OBJECT(m_windowMain),
			   "destroy",
			   GTK_SIGNAL_FUNC(s_destroy_clicked),
			   (gpointer) this);
	gtk_signal_connect(GTK_OBJECT(m_windowMain),
			   "delete_event",
			   GTK_SIGNAL_FUNC(s_delete_clicked),
			   (gpointer) this);

	// the control buttons
	g_signal_connect (G_OBJECT (m_Spinrange), "value_changed",
						G_CALLBACK (s_updateRate_changed),
						(gpointer) this);

	g_signal_connect(G_OBJECT(m_pAutocheck),
					   "clicked",
					   G_CALLBACK(s_autocheck_clicked),
					   (gpointer) this);
}


