/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
 * Copyright (C) 2004 Martin Sevior
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
#include <gtk/gtk.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "ap_Dialog_FormatFootnotes.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_UnixDialog_FormatTOC.h"


static void s_delete_clicked(GtkWidget * wid, AP_UnixDialog_FormatTOC * me )
{
    abiDestroyWidget( wid ) ;// will emit signals for us
}

static void s_destroy_clicked(GtkWidget * wid, AP_UnixDialog_FormatTOC * me )
{
   me->event_Close();
}

static void s_NumType_changed(GtkWidget * wid, AP_UnixDialog_FormatTOC * me )
{
	UT_UTF8String sProp = static_cast<char *> (g_object_get_data(G_OBJECT(wid),"toc-prop"));
	UT_UTF8String sVal = static_cast<char *> (g_object_get_data(G_OBJECT(wid),"toc-val"));

	me->setTOCProperty(sProp,sVal);
}


static void s_response_triggered(GtkWidget * widget, gint resp, AP_UnixDialog_FormatTOC * dlg)
{
	UT_return_if_fail(widget && dlg);
	
	if ( resp == GTK_RESPONSE_APPLY )
	  dlg->event_Apply();
	else if ( resp == GTK_RESPONSE_CLOSE )
	  abiDestroyWidget(widget);
}

XAP_Dialog * AP_UnixDialog_FormatTOC::static_constructor(XAP_DialogFactory * pFactory,
														  XAP_Dialog_Id id)
{
	return new AP_UnixDialog_FormatTOC(pFactory,id);
}

AP_UnixDialog_FormatTOC::AP_UnixDialog_FormatTOC(XAP_DialogFactory * pDlgFactory,
												   XAP_Dialog_Id id)
	: AP_Dialog_FormatTOC(pDlgFactory,id), 
	  m_windowMain(NULL),
	  m_wApply(NULL),
	  m_wClose(NULL),
	  m_pXML(NULL)
{
}

AP_UnixDialog_FormatTOC::~AP_UnixDialog_FormatTOC(void)
{
}

void AP_UnixDialog_FormatTOC::event_Close(void)
{
//	UT_VECTOR_PURGEALL(UT_String *,m_vecAllPropVals);
	destroy();
}

void AP_UnixDialog_FormatTOC::setTOCPropsInGUI(void)
{
}

void AP_UnixDialog_FormatTOC::destroy(void)
{
	finalize();
	gtk_widget_destroy(m_windowMain);
	m_windowMain = NULL;
}

void AP_UnixDialog_FormatTOC::activate(void)
{
	UT_ASSERT (m_windowMain);
	gdk_window_raise (m_windowMain->window);
}

void AP_UnixDialog_FormatTOC::notifyActiveFrame(XAP_Frame *pFrame)
{
    UT_ASSERT(m_windowMain);
}

void AP_UnixDialog_FormatTOC::runModeless(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);

	// Populate the window's data items
	_populateWindowData();
	_connectSignals();
	abiSetupModelessDialog(GTK_DIALOG(mainWindow),pFrame,this,GTK_RESPONSE_CLOSE);
	startUpdater();
}

GtkWidget * AP_UnixDialog_FormatTOC::_getWidget(const char * szNameBase, UT_sint32 iLevel)
{
	if(m_pXML == NULL)
	{
		return NULL;
	}
	UT_String sLocal = szNameBase;
	if(iLevel > 0)
	{
		UT_String sVal = UT_String_sprintf("%d",iLevel);
		sLocal += sVal;
	}
	return glade_xml_get_widget(m_pXML, sLocal.c_str());
}

GtkWidget * AP_UnixDialog_FormatTOC::_constructWindow(void)
{
	// get the path where our glade file is located
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(m_pApp);
	UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
	glade_path += "/ap_UnixDialog_FormatTOC.glade";

	// load the dialog from the glade file
	m_pXML = abiDialogNewFromXML( glade_path.c_str() );
	if (!m_pXML)
		return NULL;
	
	const XAP_StringSet * pSS = m_pApp->getStringSet ();

	m_windowMain   = _getWidget("ap_UnixDialog_FormatTOC");
	m_wApply = _getWidget("wApply");
	m_wClose = _getWidget("wClose");

	// set the dialog title
	abiDialogSetTitle(m_windowMain, pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Title).utf8_str());

// Heading settings

	localizeLabelMarkup(_getWidget( "lbHeading"), pSS, AP_STRING_ID_DLG_FormatTOC_Heading);
	localizeLabel(_getWidget( "lbHeadingText"), pSS, AP_STRING_ID_DLG_FormatTOC_HeadingText);
	localizeLabel(_getWidget( "lbHeadingStyle"), pSS, AP_STRING_ID_DLG_FormatTOC_HeadingStyle);
	localizeButton(_getWidget( "wHaveHeading"), pSS, AP_STRING_ID_DLG_FormatTOC_HaveHeading);
	localizeButton(_getWidget( "wChangeHeadingstyle"), pSS, AP_STRING_ID_DLG_FormatTOC_ChangeStyle);

// Level 1 Settings

	localizeLabelMarkup(_getWidget( "lbLevel1Defs"), pSS, AP_STRING_ID_DLG_FormatTOC_Level1Defs);
	UT_sint32 i = 1;
	for(i=1; i<=4; i++)
	{
		localizeLabel(_getWidget( "lbHaveLabel1",i), pSS, AP_STRING_ID_DLG_FormatTOC_HaveLabel);
		localizeLabel(_getWidget( "lbFillStyle1",i), pSS, AP_STRING_ID_DLG_FormatTOC_FillStyle);
		localizeLabel(_getWidget( "lbDispStyle1",i), pSS, AP_STRING_ID_DLG_FormatTOC_DispStyle);
		localizeLabel(_getWidget( "lbTabLeader1",i), pSS, AP_STRING_ID_DLG_FormatTOC_TabLeader);
		localizeLabel(_getWidget( "lbIndent1",i), pSS, AP_STRING_ID_DLG_FormatTOC_Indent);
		localizeButton(_getWidget( "wHaveLabel1",i), pSS, AP_STRING_ID_DLG_FormatTOC_HaveLabel);	
		localizeButton(_getWidget( "wChangeFill1",i), pSS, AP_STRING_ID_DLG_FormatTOC_ChangeStyle);
		localizeButton(_getWidget( "wChangeDisp1",i), pSS, AP_STRING_ID_DLG_FormatTOC_ChangeStyle);
	}

// Level 2 Settings

	localizeLabelMarkup(_getWidget( "lbLevel2Defs"), pSS, AP_STRING_ID_DLG_FormatTOC_Level2Defs);


// Level 3 Settings

	localizeLabelMarkup(_getWidget( "lbLevel3Defs"), pSS, AP_STRING_ID_DLG_FormatTOC_Level3Defs);


// Level 4 Settings

	localizeLabelMarkup(_getWidget( "lbLevel4Defs"), pSS, AP_STRING_ID_DLG_FormatTOC_Level4Defs);

// Create the itemlists
	_createLabelTypeItems();
	return m_windowMain;
}

void AP_UnixDialog_FormatTOC::_createLabelTypeItems(void)
{
	UT_sint32 i =1;
	UT_Vector * vecTypeList = AP_Dialog_FormatFootnotes::getFootnoteTypeLabelList();
	UT_sint32 nTypes = vecTypeList->getItemCount();
	UT_String * sProp = NULL;
	UT_String * sVal = NULL;
	for(i=1;i<=4;i++)
	{
		UT_sint32 j = 0;
		sProp = new UT_String("toc-label-type");
		UT_String sTmp = UT_String_sprintf("%d",i);
		*sProp += sTmp;
		GtkWidget * wM = gtk_menu_new();
		for(j=0; j< nTypes; j++)
		{
			m_vecAllPropVals.addItem(static_cast<void *>(sProp));
			sVal = new UT_String(static_cast<char *>(vecTypeList->getNthItem(j)));
			m_vecAllPropVals.addItem(static_cast<void *>(sVal));
			const gchar * szLab = static_cast<const gchar *>(vecTypeList->getNthItem(j));
			UT_DEBUGMSG(("Got label %s for item %d \n",szLab,j));
			GtkWidget * pW = gtk_menu_item_new_with_label(szLab);
			g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer)(sProp->c_str()));
			g_object_set_data(G_OBJECT(pW),"toc-val",(gpointer)(sVal->c_str()));

			g_signal_connect(G_OBJECT(pW),
			   "activate",
			   G_CALLBACK(s_NumType_changed),
			   (gpointer) this);
			gtk_menu_shell_append (GTK_MENU_SHELL (wM),pW);
		}
		gtk_widget_show_all(wM);
		gtk_option_menu_set_menu(GTK_OPTION_MENU(_getWidget("wLabelChoose",i)),wM);

// Now the Page Numbering style
//
		sProp = new UT_String("toc-page-type");
		sTmp = UT_String_sprintf("%d",i);
		*sProp += sTmp;
		wM = gtk_menu_new();
		for(j=0; j< nTypes; j++)
		{
			m_vecAllPropVals.addItem(static_cast<void *>(sProp));
			sVal = new UT_String(static_cast<char *>(vecTypeList->getNthItem(j)));
			m_vecAllPropVals.addItem(static_cast<void *>(sVal));
			GtkWidget * pW = gtk_menu_item_new_with_label(static_cast<char *>(vecTypeList->getNthItem(j)));
			g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer)sProp->c_str());
			g_object_set_data(G_OBJECT(pW),"toc-val",(gpointer)sVal->c_str());

			g_signal_connect(G_OBJECT(pW),
			   "activate",
			   G_CALLBACK(s_NumType_changed),
			   (gpointer) this);
			gtk_menu_shell_append (GTK_MENU_SHELL (wM),pW);
		}
		gtk_widget_show_all(wM);
		gtk_option_menu_set_menu(GTK_OPTION_MENU(_getWidget("wPageStyleChoose",i)),wM);
	}
			
}

void  AP_UnixDialog_FormatTOC::event_Apply(void)
{
	Apply();
}

/*!
 * Fill the GUI tree with the styles as defined in the XP tree.
 */
void  AP_UnixDialog_FormatTOC::_fillGUI(void)
{
}

void  AP_UnixDialog_FormatTOC::_populateWindowData(void)
{
	_fillGUI();
	setTOCPropsInGUI();
}

void  AP_UnixDialog_FormatTOC::_connectSignals(void)
{
	g_signal_connect(G_OBJECT(m_windowMain), "response", 
					 G_CALLBACK(s_response_triggered), this);
	// the catch-alls
	// Dont use gtk_signal_connect_after for modeless dialogs
	g_signal_connect(G_OBJECT(m_windowMain),
			   "destroy",
			   G_CALLBACK(s_destroy_clicked),
			   (gpointer) this);
	g_signal_connect(G_OBJECT(m_windowMain),
			   "delete_event",
			   G_CALLBACK(s_delete_clicked),
			   (gpointer) this);
}
