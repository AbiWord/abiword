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
#include "ut_string.h"
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "ap_Dialog_FormatFootnotes.h"
#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_UnixDialog_FormatTOC.h"


static void s_gchars_to_utf8str(const gchar * psz, UT_UTF8String & sStr)
{
    sStr = psz;
#if 0
    unsigned char sz[16];
	UT_uint32 i =0;
	guchar s =0;
	while(psz && *psz != 0)
	{
		s = static_cast<guchar>(*psz);
		i = s;
		g_unichar_to_utf8(i,sz);
		psz++;
		sStr += reinterpret_cast<const char *>(sz);
	}
#endif
}

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
	UT_String sNum =  UT_String_sprintf("%d",me->getDetailsLevel());
	sProp += sNum.c_str();

	me->setTOCProperty(sProp,sVal);
}


static void s_MainLevel_changed(GtkWidget * wid, AP_UnixDialog_FormatTOC * me )
{
	UT_UTF8String sLevel = static_cast<char *> (g_object_get_data(G_OBJECT(wid),"level"));
	UT_sint32 iLevel = atoi(sLevel.utf8_str());
	me->setMainLevel(iLevel);
}


static void s_DetailsLevel_changed(GtkWidget * wid, AP_UnixDialog_FormatTOC * me )
{
	UT_UTF8String sLevel = static_cast<char *> (g_object_get_data(G_OBJECT(wid),"level"));
	UT_sint32 iLevel = atoi(sLevel.utf8_str());
	me->setDetailsLevel(iLevel);
}

static void s_Indent_changed(GtkWidget * wid, AP_UnixDialog_FormatTOC * me )
{
	me->event_IndentChanged(wid);
}


static void s_StartAt_changed(GtkWidget * wid, AP_UnixDialog_FormatTOC * me )
{
	me->event_StartAtChanged(wid);
}

static void s_set_style(GtkWidget * wid, AP_UnixDialog_FormatTOC * me )
{
	me->setStyle(wid);
}

static void s_HasHeading_changed(GtkWidget * wid, AP_UnixDialog_FormatTOC * me)
{
	me->event_HasHeadingChanged(wid);
}

static void s_HasLabel_changed(GtkWidget * wid, AP_UnixDialog_FormatTOC * me)
{
	me->event_HasLabelChanged(wid);
}

static void s_check_changedDetails(GtkWidget * wid, AP_UnixDialog_FormatTOC * me)
{
	UT_UTF8String sProp = static_cast<char *> (g_object_get_data(G_OBJECT(wid),"toc-prop"));
	UT_UTF8String sVal = "1";
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wid)) == FALSE)
	{
		sVal = "0";
	}
	UT_String sNum =  UT_String_sprintf("%d",me->getDetailsLevel());
	sProp += sNum.c_str();
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
	  m_pXML(NULL),
	  m_iIndentValue(1),
	  m_iStartValue(1)
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
	_fillGUI();
}

void AP_UnixDialog_FormatTOC::setStyle(GtkWidget * wid)
{
	UT_UTF8String sVal;
	GtkWidget * pLabel = static_cast<GtkWidget *> (g_object_get_data(G_OBJECT(wid),"display-widget"));
	UT_UTF8String sProp = static_cast<char *> (g_object_get_data(G_OBJECT(pLabel),"toc-prop"));
	if(g_ascii_strcasecmp("toc-heading-style",sProp.utf8_str()) != 0)
	{
		UT_String sNum =  UT_String_sprintf("%d",getMainLevel());
		sProp += sNum.c_str();
	}
	sVal = getNewStyle(sProp);
	gtk_label_set_text(GTK_LABEL(pLabel),sVal.utf8_str());
	setTOCProperty(sProp,sVal);
	applyTOCPropsToDoc();
}

void AP_UnixDialog_FormatTOC::setSensitivity(bool bSensitive)
{
	gboolean gSensitive = TRUE;
	if(!bSensitive)
	{
		gSensitive = FALSE;
	}
	gtk_widget_set_sensitive (m_wApply,gSensitive);
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


void AP_UnixDialog_FormatTOC::event_StartAtChanged(GtkWidget * wSpin)
{
	UT_sint32 iNew = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(wSpin));
	bool bInc = true;
	if(iNew == m_iStartValue)
	{
		return;
	}
	UT_DEBUGMSG(("New StartAt value %d \n",iNew));
	if(iNew < m_iStartValue)
	{
		bInc = false;
	}
	m_iStartValue = iNew;
	incrementStartAt(getDetailsLevel(),bInc);
	UT_UTF8String sVal = getTOCPropVal("toc-label-start",getDetailsLevel());
	GtkWidget * pW = _getWidget("wStartEntry");
	gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());
}

void AP_UnixDialog_FormatTOC::event_HasHeadingChanged(GtkWidget * wid)
{
	UT_UTF8String sProp = static_cast<char *> (g_object_get_data(G_OBJECT(wid),"toc-prop"));
	UT_UTF8String sVal = "1";
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wid)) == FALSE)
	{
		sVal = "0";
		_setHasHeadingSensitivity(FALSE);
	}
	else
	{
		_setHasHeadingSensitivity(TRUE);
	}
	if(g_ascii_strcasecmp("toc-has-heading",sProp.utf8_str()) != 0)
	{
		UT_String sNum =  UT_String_sprintf("%d",getMainLevel());
		sProp += sNum.c_str();
	}
	setTOCProperty(sProp,sVal);
}

void AP_UnixDialog_FormatTOC::_setHasHeadingSensitivity(bool bSensitive)
{
	gtk_widget_set_sensitive(_getWidget("lbHeadingText"), bSensitive);
	gtk_widget_set_sensitive(_getWidget("edHeadingText"), bSensitive);
	gtk_widget_set_sensitive(_getWidget("lbHeadingStyle"), bSensitive);
	gtk_widget_set_sensitive(_getWidget("lbCurrentHeadingStyle"), bSensitive);
	gtk_widget_set_sensitive(_getWidget("lbChangeHeadingStyle"), bSensitive);		

}

void AP_UnixDialog_FormatTOC::event_HasLabelChanged(GtkWidget * wid)
{
	UT_UTF8String sProp = static_cast<char *> (g_object_get_data(G_OBJECT(wid),"toc-prop"));
	UT_String sNum =  UT_String_sprintf("%d",getMainLevel());
	sProp += sNum.c_str();
	UT_UTF8String sVal = "1";
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wid)) == FALSE)
	{
		sVal = "0";
	}
	setTOCProperty(sProp,sVal);
}

void AP_UnixDialog_FormatTOC::event_IndentChanged(GtkWidget * wSpin)
{
	UT_sint32 iNew = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(wSpin));
	bool bInc = true;
	UT_DEBUGMSG(("New Indent value %d \n",iNew));
	if(iNew == m_iIndentValue)
	{
		return;
	}
	if(iNew < m_iIndentValue)
	{
		bInc = false;
	}
	m_iIndentValue = iNew;
	incrementIndent(getDetailsLevel(),bInc);
	UT_UTF8String sVal = getTOCPropVal("toc-indent",getDetailsLevel());
	GtkWidget * pW = _getWidget("wIndentEntry");
	gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());
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
	UT_UTF8String s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Title,s);
	abiDialogSetTitle(m_windowMain, s.utf8_str());

// localize notebook tabs
	localizeLabel(_getWidget( "lbGeneral"), pSS, AP_STRING_ID_DLG_FormatTOC_General);
	localizeLabel(_getWidget( "lbLayoutDetails"), pSS, AP_STRING_ID_DLG_FormatTOC_LayoutDetails);

// Heading settings

	localizeButtonMarkup(_getWidget( "cbHasHeading"), pSS, AP_STRING_ID_DLG_FormatTOC_HasHeading);
	localizeLabelUnderline(_getWidget( "lbHeadingText"), pSS, AP_STRING_ID_DLG_FormatTOC_HeadingText);
	localizeLabel(_getWidget( "lbHeadingStyle"), pSS, AP_STRING_ID_DLG_FormatTOC_HeadingStyle);

	localizeButton(_getWidget( "lbChangeHeadingStyle"), pSS, AP_STRING_ID_DLG_FormatTOC_ChangeStyle);

// Main level definitions
	localizeLabelMarkup(_getWidget( "lbMainLevelDefs"), pSS, AP_STRING_ID_DLG_FormatTOC_LevelDefs);
	localizeButtonUnderline(_getWidget( "wHasLabel"), pSS, AP_STRING_ID_DLG_FormatTOC_HasLabel);	
	localizeLabel(_getWidget( "lbFillStyle"), pSS, AP_STRING_ID_DLG_FormatTOC_FillStyle);
	localizeLabel(_getWidget( "lbDispStyle"), pSS, AP_STRING_ID_DLG_FormatTOC_DispStyle);
	localizeButton(_getWidget( "wChangeFill"), pSS, AP_STRING_ID_DLG_FormatTOC_ChangeStyle);
	localizeButton(_getWidget( "wChangeDisp"), pSS, AP_STRING_ID_DLG_FormatTOC_ChangeStyle);

// Details top
	localizeLabelMarkup(_getWidget( "lbDetails"), pSS, AP_STRING_ID_DLG_FormatTOC_DetailsTop);
	localizeLabelUnderline(_getWidget( "lbStartAt"), pSS, AP_STRING_ID_DLG_FormatTOC_StartAt);
	localizeLabelUnderline(_getWidget( "lbTextBefore"), pSS, AP_STRING_ID_DLG_FormatTOC_TextBefore);
	localizeLabelUnderline(_getWidget( "lbNumberingType"), pSS, AP_STRING_ID_DLG_FormatTOC_NumberingType);
	localizeLabelUnderline(_getWidget( "lbTextAfter"), pSS, AP_STRING_ID_DLG_FormatTOC_TextAfter);
	localizeButtonUnderline(_getWidget( "cbInherit"), pSS, AP_STRING_ID_DLG_FormatTOC_InheritLabel);

// Tabs and numbering
	localizeLabelMarkup(_getWidget( "lbTabPage"), pSS, AP_STRING_ID_DLG_FormatTOC_DetailsTabPage);
	localizeLabelUnderline(_getWidget( "lbTabLeader"), pSS, AP_STRING_ID_DLG_FormatTOC_TabLeader);
	localizeLabelUnderline(_getWidget( "lbPageNumbering"), pSS, AP_STRING_ID_DLG_FormatTOC_PageNumbering);
	localizeLabelUnderline(_getWidget( "lbIndent"), pSS, AP_STRING_ID_DLG_FormatTOC_Indent);

// Create the itemlists
	_createLabelTypeItems();
	_createTABTypeItems();
	_createLevelItems();
	return m_windowMain;
}

void AP_UnixDialog_FormatTOC::setMainLevel(UT_sint32 iLevel)
{
	AP_Dialog_FormatTOC::setMainLevel(iLevel);
	UT_UTF8String sVal;
	sVal = getTOCPropVal("toc-dest-style",getMainLevel());
	GtkWidget * pW= _getWidget("wDispStyle");
	gtk_label_set_text(GTK_LABEL(pW),sVal.utf8_str());


	sVal = getTOCPropVal("toc-has-label",getMainLevel());
	pW = _getWidget("wHasLabel");
	if(g_ascii_strcasecmp(sVal.utf8_str(),"1") == 0)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pW),TRUE);
	}
	else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pW),FALSE);
	}

	sVal = getTOCPropVal("toc-source-style",getMainLevel());
	pW = _getWidget("wFillStyle");
	gtk_label_set_text(GTK_LABEL(pW),sVal.utf8_str());
}


void AP_UnixDialog_FormatTOC::setDetailsLevel(UT_sint32 iLevel)
{
	AP_Dialog_FormatTOC::setDetailsLevel(iLevel);
	UT_UTF8String sVal;

	sVal = getTOCPropVal("toc-label-after",getDetailsLevel());
	GtkWidget * pW = _getWidget("edTextAfter");
	gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());

	sVal = getTOCPropVal("toc-label-before",getDetailsLevel());
	pW = _getWidget("edTextBefore");
	gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());

	sVal = getTOCPropVal("toc-label-start",getDetailsLevel());
	pW = _getWidget("wStartEntry");
	gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());


	sVal = getTOCPropVal("toc-indent",getDetailsLevel());
	pW = _getWidget("wIndentEntry");
	gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());
	

	sVal = getTOCPropVal("toc-label-inherits",getDetailsLevel());
	pW = _getWidget("cbInherit");
	if(g_ascii_strcasecmp(sVal.utf8_str(),"1") == 0)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pW),TRUE);
	}
	else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pW),FALSE);
	}


	FV_View * pView = static_cast<FV_View *>(getActiveFrame()->getCurrentView());
	sVal = getTOCPropVal("toc-label-type",getDetailsLevel());
	pW = _getWidget("wLabelChoose"); 
	UT_sint32 iHist = static_cast<UT_sint32>(pView->getLayout()->FootnoteTypeFromString(sVal.utf8_str()));
	gtk_option_menu_set_history(GTK_OPTION_MENU(pW),iHist);

	sVal = getTOCPropVal("toc-page-type",getDetailsLevel());
	pW = _getWidget("wPageNumberingChoose"); 
	iHist = static_cast<UT_sint32>(pView->getLayout()->FootnoteTypeFromString(sVal.utf8_str()));
	gtk_option_menu_set_history(GTK_OPTION_MENU(pW),iHist);

	sVal = getTOCPropVal("toc-tab-leader",getDetailsLevel());
	pW = _getWidget("wTabLeaderChoose");
	if(g_ascii_strcasecmp(sVal.utf8_str(),"none") == 0)
	{
		iHist = 0;
	}
	else if(g_ascii_strcasecmp(sVal.utf8_str(),"dot") == 0)
	{
		iHist = 1;
	}
	else if(g_ascii_strcasecmp(sVal.utf8_str(),"hyphen") == 0)
	{
		iHist = 2;
	}
	else if(g_ascii_strcasecmp(sVal.utf8_str(),"underline") == 0)
	{
		iHist = 3;
	}
	else
	{
		iHist = 1;
	}
	gtk_option_menu_set_history(GTK_OPTION_MENU(pW),iHist);

	
}

void AP_UnixDialog_FormatTOC::_createLevelItems(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet ();
	UT_UTF8String s;
	GtkWidget * wM = gtk_menu_new();

	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level1,s);
	GtkWidget * pW = gtk_menu_item_new_with_label(s.utf8_str());
	g_object_set_data(G_OBJECT(pW),"level",(gpointer)"1");
	g_signal_connect(G_OBJECT(pW),
					 "activate",
					 G_CALLBACK(s_MainLevel_changed),
					 (gpointer) this);
	gtk_menu_shell_append (GTK_MENU_SHELL (wM),pW);

	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level2,s);
	pW = gtk_menu_item_new_with_label(s.utf8_str());
	g_object_set_data(G_OBJECT(pW),"level",(gpointer)"2");
	g_signal_connect(G_OBJECT(pW),
					 "activate",
					 G_CALLBACK(s_MainLevel_changed),
					 (gpointer) this);
	gtk_menu_shell_append (GTK_MENU_SHELL (wM),pW);

	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level3,s);
	pW = gtk_menu_item_new_with_label(s.utf8_str());
	g_object_set_data(G_OBJECT(pW),"level",(gpointer)"3");
	g_signal_connect(G_OBJECT(pW),
					 "activate",
					 G_CALLBACK(s_MainLevel_changed),
					 (gpointer) this);
	gtk_menu_shell_append (GTK_MENU_SHELL (wM),pW);

	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level4,s);
	pW = gtk_menu_item_new_with_label(s.utf8_str());
	g_object_set_data(G_OBJECT(pW),"level",(gpointer)"4");
	g_signal_connect(G_OBJECT(pW),
					 "activate",
					 G_CALLBACK(s_MainLevel_changed),
					 (gpointer) this);
	gtk_menu_shell_append (GTK_MENU_SHELL (wM),pW);
	gtk_widget_show_all(wM);
	gtk_option_menu_set_menu(GTK_OPTION_MENU(_getWidget("wLevelOption")),wM);

//////////////////////////////////////////////////////////////////////////////

	wM = gtk_menu_new();

	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level1,s);
	pW = gtk_menu_item_new_with_label(s.utf8_str());
	g_object_set_data(G_OBJECT(pW),"level",(gpointer)"1");
	g_signal_connect(G_OBJECT(pW),
					 "activate",
					 G_CALLBACK(s_DetailsLevel_changed),
					 (gpointer) this);
	gtk_menu_shell_append (GTK_MENU_SHELL (wM),pW);

	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level2,s);
	pW = gtk_menu_item_new_with_label(s.utf8_str());
	g_object_set_data(G_OBJECT(pW),"level",(gpointer)"2");
	g_signal_connect(G_OBJECT(pW),
					 "activate",
					 G_CALLBACK(s_DetailsLevel_changed),
					 (gpointer) this);
	gtk_menu_shell_append (GTK_MENU_SHELL (wM),pW);

	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level3,s);
    pW = gtk_menu_item_new_with_label(s.utf8_str());
	g_object_set_data(G_OBJECT(pW),"level",(gpointer)"3");
	g_signal_connect(G_OBJECT(pW),
					 "activate",
					 G_CALLBACK(s_DetailsLevel_changed),
					 (gpointer) this);
	gtk_menu_shell_append (GTK_MENU_SHELL (wM),pW);

	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level4,s);
	pW = gtk_menu_item_new_with_label(s.utf8_str());
	g_object_set_data(G_OBJECT(pW),"level",(gpointer)"4");
	g_signal_connect(G_OBJECT(pW),
					 "activate",
					 G_CALLBACK(s_DetailsLevel_changed),
					 (gpointer) this);
	gtk_menu_shell_append (GTK_MENU_SHELL (wM),pW);
	gtk_widget_show_all(wM);
	gtk_option_menu_set_menu(GTK_OPTION_MENU(_getWidget("wDetailsLevel")),wM);
	
}

void AP_UnixDialog_FormatTOC::_createLabelTypeItems(void)
{
	const UT_GenericVector<const gchar*> * vecTypeList = AP_Dialog_FormatFootnotes::getFootnoteTypeLabelList();
	const UT_GenericVector<const gchar*> * vecPropList = getVecLabelPropValue();
	UT_sint32 nTypes = vecTypeList->getItemCount();
	UT_UTF8String * sProp = NULL;
	UT_UTF8String * sVal = NULL;

	UT_sint32 j = 0;
	sProp = new UT_UTF8String("toc-label-type");
	GtkWidget * wM = gtk_menu_new();
	m_vecAllPropVals.addItem(sProp);
	for(j=0; j< nTypes; j++)
	{
		sVal = new UT_UTF8String(vecTypeList->getNthItem(j));
		m_vecAllPropVals.addItem(sVal);
		const gchar * szLab = static_cast<const gchar *>(vecTypeList->getNthItem(j));
		UT_DEBUGMSG(("Got label %s for item %d \n",szLab,j));
		GtkWidget * pW = gtk_menu_item_new_with_label(szLab);
		g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer)(sProp->utf8_str()));
		const gchar * szVal = static_cast<const gchar *>(vecPropList->getNthItem(j));
		g_object_set_data(G_OBJECT(pW),"toc-val",(gpointer)(szVal));
		g_signal_connect(G_OBJECT(pW),
			   "activate",
			   G_CALLBACK(s_NumType_changed),
			   (gpointer) this);
		gtk_menu_shell_append (GTK_MENU_SHELL (wM),pW);
	}
	gtk_widget_show_all(wM);
	gtk_option_menu_set_menu(GTK_OPTION_MENU(_getWidget("wLabelChoose")),wM);

// Now the Page Numbering style
//
	sProp = new UT_UTF8String("toc-page-type");
	wM = gtk_menu_new();
	m_vecAllPropVals.addItem(sProp);
	for(j=0; j< nTypes; j++)
	{
		sVal = new UT_UTF8String(vecTypeList->getNthItem(j));
		m_vecAllPropVals.addItem(sVal);
		GtkWidget * pW = gtk_menu_item_new_with_label(vecTypeList->getNthItem(j));
		g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer)sProp->utf8_str());
		const gchar * szVal = static_cast<const gchar *>(vecPropList->getNthItem(j));
		g_object_set_data(G_OBJECT(pW),"toc-val",(gpointer)(szVal));
		g_signal_connect(G_OBJECT(pW),
						 "activate",
						 G_CALLBACK(s_NumType_changed),
						 (gpointer) this);
		gtk_menu_shell_append (GTK_MENU_SHELL (wM),pW);
	}
	gtk_widget_show_all(wM);
	gtk_option_menu_set_menu(GTK_OPTION_MENU(_getWidget("wPageNumberingChoose")),wM);
}


void AP_UnixDialog_FormatTOC::_createTABTypeItems(void)
{
	const UT_GenericVector<const gchar*> * vecLabels = getVecTABLeadersLabel();
	const UT_GenericVector<const gchar*> * vecProps = getVecTABLeadersProp();
	UT_sint32 nTypes = vecLabels->getItemCount();
	UT_UTF8String * sProp = NULL;
	UT_UTF8String * sVal = NULL;
	UT_sint32 j = 0;
	sProp = new UT_UTF8String("toc-tab-leader");
	GtkWidget * wM = gtk_menu_new();
	for(j=0; j< nTypes; j++)
	{
		m_vecAllPropVals.addItem(sProp);
		sVal = new UT_UTF8String(vecProps->getNthItem(j));
		m_vecAllPropVals.addItem(sVal);
		const gchar * szLab = static_cast<const gchar *>(vecLabels->getNthItem(j));
		UT_DEBUGMSG(("Got label %s for item %d \n",szLab,j));
		GtkWidget * pW = gtk_menu_item_new_with_label(szLab);
		g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer)(sProp->utf8_str()));
		g_object_set_data(G_OBJECT(pW),"toc-val",(gpointer)(sVal->utf8_str()));

		g_signal_connect(G_OBJECT(pW),
						 "activate",
						 G_CALLBACK(s_NumType_changed),
						 (gpointer) this);
		gtk_menu_shell_append (GTK_MENU_SHELL (wM),pW);
	}
	gtk_widget_show_all(wM);
	gtk_option_menu_set_menu(GTK_OPTION_MENU(_getWidget("wTabLeaderChoose")),wM);
}

void  AP_UnixDialog_FormatTOC::event_Apply(void)
{
	UT_DEBUGMSG(("Doing apply \n"));

// Heading Text

	GtkWidget * pW = _getWidget("edHeadingText");
	UT_UTF8String sVal;
	s_gchars_to_utf8str(gtk_entry_get_text(GTK_ENTRY(pW)),sVal);
	setTOCProperty("toc-heading",sVal.utf8_str());

// Text before and after

	pW = _getWidget("edTextAfter");
	s_gchars_to_utf8str(gtk_entry_get_text(GTK_ENTRY(pW)),sVal);
	UT_UTF8String sProp;
	s_gchars_to_utf8str(static_cast<char *> (g_object_get_data(G_OBJECT(pW),"toc-prop")),sProp);
	UT_String sNum =  UT_String_sprintf("%d",getDetailsLevel());
	sProp += sNum.c_str();
	setTOCProperty(sProp,sVal);

	pW = _getWidget("edTextBefore");
	s_gchars_to_utf8str(gtk_entry_get_text(GTK_ENTRY(pW)),sVal);
	s_gchars_to_utf8str(static_cast<char *> (g_object_get_data(G_OBJECT(pW),"toc-prop")),sProp);
	sProp += sNum.c_str();
	setTOCProperty(sProp,sVal);
	Apply();
}

gpointer AP_UnixDialog_FormatTOC::_makeProp(const char * szProp, UT_sint32 i)
{
	UT_UTF8String sLocal = szProp;
	UT_UTF8String sVal = UT_UTF8String_sprintf("%d",i);
	sLocal += sVal;
	UT_UTF8String * pS = new UT_UTF8String(sLocal);
	m_vecAllPropVals.addItem(pS);
	return (gpointer) pS->utf8_str();
}

/*!
 * Fill the GUI tree with the styles as defined in the XP tree.
 */
void  AP_UnixDialog_FormatTOC::_fillGUI(void)
{
	UT_UTF8String sVal;
	sVal = getTOCPropVal("toc-has-heading");

	GtkWidget * pW = _getWidget("wLevelOption");
	gtk_option_menu_set_history(GTK_OPTION_MENU(pW), getMainLevel()-1);

	pW = _getWidget("cbHasHeading");
	if(g_ascii_strcasecmp(sVal.utf8_str(),"1") == 0)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pW),TRUE);
		_setHasHeadingSensitivity(TRUE);
	}
	else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pW),FALSE);
		_setHasHeadingSensitivity(FALSE);
	}
	g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer) "toc-has-heading");
	g_signal_connect(G_OBJECT(pW),
					 "toggled",
					 G_CALLBACK(s_HasHeading_changed),
					 (gpointer) this);
	
	sVal = getTOCPropVal("toc-heading");
	pW = _getWidget("edHeadingText");
	gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());
	g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer) "toc-heading");


	sVal = getTOCPropVal("toc-heading-style");
	pW = _getWidget("lbCurrentHeadingStyle");
	gtk_label_set_text(GTK_LABEL(pW),sVal.utf8_str());
	g_object_set_data(G_OBJECT(_getWidget("lbChangeHeadingStyle")),"display-widget",(gpointer)pW);
	g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer) "toc-heading-style");


	FV_View * pView = static_cast<FV_View *>(getActiveFrame()->getCurrentView());

	sVal = getTOCPropVal("toc-dest-style",getMainLevel());
	pW= _getWidget("wDispStyle");
	gtk_label_set_text(GTK_LABEL(pW),sVal.utf8_str());
	g_object_set_data(G_OBJECT(_getWidget("wChangeDisp")),"display-widget",(gpointer)pW);
	g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer) "toc-dest-style");


	sVal = getTOCPropVal("toc-has-label",getMainLevel());
	pW = _getWidget("wHasLabel");
	if(g_ascii_strcasecmp(sVal.utf8_str(),"1") == 0)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pW),TRUE);
	}
	else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pW),FALSE);
	}
	g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer) "toc-has-label");
	g_signal_connect(G_OBJECT(pW),
					 "toggled",
					 G_CALLBACK(s_HasLabel_changed),
					 (gpointer) this);

	sVal = getTOCPropVal("toc-label-after",getDetailsLevel());
	pW = _getWidget("edTextAfter");
	gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());
	g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer) "toc-label-after");

	sVal = getTOCPropVal("toc-label-before",getDetailsLevel());
	pW = _getWidget("edTextBefore");
	gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());
	g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer) "toc-label-before");

	sVal = getTOCPropVal("toc-label-inherits",getDetailsLevel());
	pW = _getWidget("cbInherit");
	if(g_ascii_strcasecmp(sVal.utf8_str(),"1") == 0)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pW),TRUE);
	}
	else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pW),FALSE);
	}
	g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer) "toc-label-inherits");
	g_signal_connect(G_OBJECT(pW),
					 "toggled",
					 G_CALLBACK(s_check_changedDetails),
					 (gpointer) this);


	sVal = getTOCPropVal("toc-label-start",getDetailsLevel());
	pW = _getWidget("wStartEntry");
	gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON (_getWidget("wStartSpin")),
                                             (gdouble) m_iStartValue );
	g_signal_connect(G_OBJECT(_getWidget("wStartSpin")),
							  "value-changed",
							  G_CALLBACK(s_StartAt_changed),
							  reinterpret_cast<gpointer>(this));

	sVal = getTOCPropVal("toc-indent",getDetailsLevel());
	pW = _getWidget("wIndentEntry");
	gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON (_getWidget("wIndentSpin")),
                                             (gdouble) m_iIndentValue );
	g_signal_connect(G_OBJECT(_getWidget("wIndentSpin")),
							  "value-changed",
							  G_CALLBACK(s_Indent_changed),
							  reinterpret_cast<gpointer>(this));
	

	sVal = getTOCPropVal("toc-label-type",getDetailsLevel());
	pW = _getWidget("wLabelChoose"); 
	UT_sint32 iHist = static_cast<UT_sint32>(pView->getLayout()->FootnoteTypeFromString(sVal.utf8_str()));
	gtk_option_menu_set_history(GTK_OPTION_MENU(pW),iHist);

	sVal = getTOCPropVal("toc-page-type",getDetailsLevel());
	pW = _getWidget("wPageNumberingChoose"); 
	iHist = static_cast<UT_sint32>(pView->getLayout()->FootnoteTypeFromString(sVal.utf8_str()));
	gtk_option_menu_set_history(GTK_OPTION_MENU(pW),iHist);

	sVal = getTOCPropVal("toc-source-style",getMainLevel());
	pW = _getWidget("wFillStyle");
	gtk_label_set_text(GTK_LABEL(pW),sVal.utf8_str());
	g_object_set_data(G_OBJECT(_getWidget("wChangeFill")),"display-widget",(gpointer)pW);
	g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer) "toc-source-style");

	sVal = getTOCPropVal("toc-tab-leader",getDetailsLevel());
	pW = _getWidget("wTabLeaderChoose");
	if(g_ascii_strcasecmp(sVal.utf8_str(),"none") == 0)
	{
		iHist = 0;
	}
	else if(g_ascii_strcasecmp(sVal.utf8_str(),"dot") == 0)
	{
		iHist = 1;
	}
	else if(g_ascii_strcasecmp(sVal.utf8_str(),"hyphen") == 0)
	{
		iHist = 2;
	}
	else if(g_ascii_strcasecmp(sVal.utf8_str(),"underline") == 0)
	{
		iHist = 3;
	}
	else
	{
		iHist = 1;
	}
	gtk_option_menu_set_history(GTK_OPTION_MENU(pW),iHist);
}

void  AP_UnixDialog_FormatTOC::_populateWindowData(void)
{
	fillTOCPropsFromDoc();
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
	g_signal_connect(G_OBJECT(_getWidget("lbChangeHeadingStyle")),
					  "clicked",
					  G_CALLBACK(s_set_style),
					  (gpointer) this);

	g_signal_connect(G_OBJECT(_getWidget("wChangeFill")),
					 "clicked",
					 G_CALLBACK(s_set_style),
					 (gpointer) this);
	g_signal_connect(G_OBJECT(_getWidget("wChangeDisp")),
					 "clicked",
					 G_CALLBACK(s_set_style),
					 (gpointer) this);
}
