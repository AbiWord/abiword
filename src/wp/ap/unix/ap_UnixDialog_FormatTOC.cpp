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


static void s_set_style(GtkWidget * wid, AP_UnixDialog_FormatTOC * me )
{
	me->setStyle(wid);
}

static void s_check_changedMain(GtkWidget * wid, AP_UnixDialog_FormatTOC * me)
{
	UT_UTF8String sProp = static_cast<char *> (g_object_get_data(G_OBJECT(wid),"toc-prop"));
	UT_UTF8String sVal = "1";
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wid)) == FALSE)
	{
		sVal = "0";
	}
	if(UT_stricmp("toc-has-heading",sProp.utf8_str()) != 0)
	{
		UT_String sNum =  UT_String_sprintf("%d",me->getMainLevel());
		sProp += sNum.c_str();
	}
	me->setTOCProperty(sProp,sVal);
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
	  m_iMainLevel(1),
	  m_iDetailsLevel(1)
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
	GtkWidget * pEntry = static_cast<GtkWidget *> (g_object_get_data(G_OBJECT(wid),"entry-widget"));
	UT_UTF8String sProp = static_cast<char *> (g_object_get_data(G_OBJECT(pEntry),"toc-prop"));
	if(UT_stricmp("toc-heading-style",sProp.utf8_str()) != 0)
	{
		UT_String sNum =  UT_String_sprintf("%d",m_iMainLevel);
		sProp += sNum.c_str();
	}
	sVal = getNewStyle(sProp);
	gtk_entry_set_text(GTK_ENTRY(pEntry),sVal.utf8_str());
	setTOCProperty(sProp,sVal);
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
	localizeLabelMarkup(_getWidget( "lbDetails"), pSS, AP_STRING_ID_DLG_FormatTOC_DetailsTop);
	localizeLabelMarkup(_getWidget( "lbTabPage"), pSS, AP_STRING_ID_DLG_FormatTOC_DetailsTabPage);


	localizeButton(_getWidget( "wHaveHeading"), pSS, AP_STRING_ID_DLG_FormatTOC_HaveHeading);
	localizeButton(_getWidget( "wChangeHeadingstyle"), pSS, AP_STRING_ID_DLG_FormatTOC_ChangeStyle);


	localizeLabelMarkup(_getWidget( "lbMainLevelDefs"), pSS, AP_STRING_ID_DLG_FormatTOC_LevelDefs);
	localizeLabel(_getWidget( "lbFillStyle"), pSS, AP_STRING_ID_DLG_FormatTOC_FillStyle);
	localizeLabel(_getWidget( "lbDispStyle"), pSS, AP_STRING_ID_DLG_FormatTOC_DispStyle);
	localizeLabel(_getWidget( "lbTabLeader"), pSS, AP_STRING_ID_DLG_FormatTOC_TabLeader);
	localizeLabel(_getWidget( "lbIndent"), pSS, AP_STRING_ID_DLG_FormatTOC_Indent);
	localizeButton(_getWidget( "wHaveLabel"), pSS, AP_STRING_ID_DLG_FormatTOC_HaveLabel);	
	localizeButton(_getWidget( "wChangeFill"), pSS, AP_STRING_ID_DLG_FormatTOC_ChangeStyle);
	localizeButton(_getWidget( "wChangeDisp"), pSS, AP_STRING_ID_DLG_FormatTOC_ChangeStyle);

// Create the itemlists
	_createLabelTypeItems();
	_createTABTypeItems();
	_createLevelItems();
	return m_windowMain;
}

void AP_UnixDialog_FormatTOC::setMainLevel(UT_sint32 iLevel)
{
	m_iMainLevel = iLevel;
	UT_UTF8String sVal;
	sVal = getTOCPropVal("toc-dest-style",m_iMainLevel);
	GtkWidget * pW= _getWidget("wDispStyle");
	gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());


	sVal = getTOCPropVal("toc-has-label",m_iMainLevel);
	pW = _getWidget("wHaveLabel");
	if(UT_stricmp(sVal.utf8_str(),"1") == 0)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pW),TRUE);
	}
	else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pW),FALSE);
	}

	sVal = getTOCPropVal("toc-source-style",m_iMainLevel);
	pW = _getWidget("wFillStyle");
	gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());
}


void AP_UnixDialog_FormatTOC::setDetailsLevel(UT_sint32 iLevel)
{
	m_iDetailsLevel = iLevel;
	UT_UTF8String sVal;

	sVal = getTOCPropVal("toc-label-after",m_iDetailsLevel);
	GtkWidget * pW = _getWidget("wTextAfter");
	gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());

	sVal = getTOCPropVal("toc-label-before",m_iDetailsLevel);
	pW = _getWidget("wTextBefore");
	gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());

	sVal = getTOCPropVal("toc-label-inherits",m_iDetailsLevel);
	pW = _getWidget("wInherit");
	if(UT_stricmp(sVal.utf8_str(),"1") == 0)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pW),TRUE);
	}
	else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pW),FALSE);
	}


	FV_View * pView = static_cast<FV_View *>(getActiveFrame()->getCurrentView());
	sVal = getTOCPropVal("toc-label-type",m_iDetailsLevel);
	pW = _getWidget("wLabelChoose"); 
	UT_sint32 iHist = static_cast<UT_sint32>(pView->getLayout()->FootnoteTypeFromString(sVal.utf8_str()));
	gtk_option_menu_set_history(GTK_OPTION_MENU(pW),iHist);

	sVal = getTOCPropVal("toc-page-type",m_iDetailsLevel);
	pW = _getWidget("wPageStyleChoose"); 
	iHist = static_cast<UT_sint32>(pView->getLayout()->FootnoteTypeFromString(sVal.utf8_str()));
	gtk_option_menu_set_history(GTK_OPTION_MENU(pW),iHist);

	sVal = getTOCPropVal("toc-tab-leader",m_iDetailsLevel);
	pW = _getWidget("wTabLeaderChoose");
	if(UT_stricmp(sVal.utf8_str(),"none") == 0)
	{
		iHist = 0;
	}
	else if(UT_stricmp(sVal.utf8_str(),"dot") == 0)
	{
		iHist = 1;
	}
	else if(UT_stricmp(sVal.utf8_str(),"hyphen") == 0)
	{
		iHist = 2;
	}
	else if(UT_stricmp(sVal.utf8_str(),"underline") == 0)
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
	GtkWidget * wM = gtk_menu_new();

	GtkWidget * pW = gtk_menu_item_new_with_label(pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level1).utf8_str());
	g_object_set_data(G_OBJECT(pW),"level",(gpointer)"1");
	g_signal_connect(G_OBJECT(pW),
					 "activate",
					 G_CALLBACK(s_MainLevel_changed),
					 (gpointer) this);
	gtk_menu_shell_append (GTK_MENU_SHELL (wM),pW);

	pW = gtk_menu_item_new_with_label(pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level2).utf8_str());
	g_object_set_data(G_OBJECT(pW),"level",(gpointer)"2");
	g_signal_connect(G_OBJECT(pW),
					 "activate",
					 G_CALLBACK(s_MainLevel_changed),
					 (gpointer) this);
	gtk_menu_shell_append (GTK_MENU_SHELL (wM),pW);

	pW = gtk_menu_item_new_with_label(pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level3).utf8_str());
	g_object_set_data(G_OBJECT(pW),"level",(gpointer)"3");
	g_signal_connect(G_OBJECT(pW),
					 "activate",
					 G_CALLBACK(s_MainLevel_changed),
					 (gpointer) this);
	gtk_menu_shell_append (GTK_MENU_SHELL (wM),pW);

	pW = gtk_menu_item_new_with_label(pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level4).utf8_str());
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

	pW = gtk_menu_item_new_with_label(pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level1).utf8_str());
	g_object_set_data(G_OBJECT(pW),"level",(gpointer)"1");
	g_signal_connect(G_OBJECT(pW),
					 "activate",
					 G_CALLBACK(s_DetailsLevel_changed),
					 (gpointer) this);
	gtk_menu_shell_append (GTK_MENU_SHELL (wM),pW);

	pW = gtk_menu_item_new_with_label(pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level2).utf8_str());
	g_object_set_data(G_OBJECT(pW),"level",(gpointer)"2");
	g_signal_connect(G_OBJECT(pW),
					 "activate",
					 G_CALLBACK(s_DetailsLevel_changed),
					 (gpointer) this);
	gtk_menu_shell_append (GTK_MENU_SHELL (wM),pW);

    pW = gtk_menu_item_new_with_label(pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level3).utf8_str());
	g_object_set_data(G_OBJECT(pW),"level",(gpointer)"3");
	g_signal_connect(G_OBJECT(pW),
					 "activate",
					 G_CALLBACK(s_DetailsLevel_changed),
					 (gpointer) this);
	gtk_menu_shell_append (GTK_MENU_SHELL (wM),pW);

	pW = gtk_menu_item_new_with_label(pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level4).utf8_str());
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
	UT_Vector * vecTypeList = AP_Dialog_FormatFootnotes::getFootnoteTypeLabelList();
	UT_Vector * vecPropList = getVecLabelPropValue();
	UT_sint32 nTypes = vecTypeList->getItemCount();
	UT_String * sProp = NULL;
	UT_String * sVal = NULL;

	UT_sint32 j = 0;
	sProp = new UT_String("toc-label-type");
	GtkWidget * wM = gtk_menu_new();
	m_vecAllPropVals.addItem(static_cast<void *>(sProp));
	for(j=0; j< nTypes; j++)
	{
		sVal = new UT_String(static_cast<char *>(vecTypeList->getNthItem(j)));
		m_vecAllPropVals.addItem(static_cast<void *>(sVal));
		const gchar * szLab = static_cast<const gchar *>(vecTypeList->getNthItem(j));
		UT_DEBUGMSG(("Got label %s for item %d \n",szLab,j));
		GtkWidget * pW = gtk_menu_item_new_with_label(szLab);
		g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer)(sProp->c_str()));
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
	sProp = new UT_String("toc-page-type");
	wM = gtk_menu_new();
	m_vecAllPropVals.addItem(static_cast<void *>(sProp));
	for(j=0; j< nTypes; j++)
	{
		sVal = new UT_String(static_cast<char *>(vecTypeList->getNthItem(j)));
		m_vecAllPropVals.addItem(static_cast<void *>(sVal));
		GtkWidget * pW = gtk_menu_item_new_with_label(static_cast<char *>(vecTypeList->getNthItem(j)));
		g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer)sProp->c_str());
		const gchar * szVal = static_cast<const gchar *>(vecPropList->getNthItem(j));
		g_object_set_data(G_OBJECT(pW),"toc-val",(gpointer)(szVal));
		g_signal_connect(G_OBJECT(pW),
						 "activate",
						 G_CALLBACK(s_NumType_changed),
						 (gpointer) this);
		gtk_menu_shell_append (GTK_MENU_SHELL (wM),pW);
	}
	gtk_widget_show_all(wM);
	gtk_option_menu_set_menu(GTK_OPTION_MENU(_getWidget("wPageStyleChoose")),wM);
}


void AP_UnixDialog_FormatTOC::_createTABTypeItems(void)
{
	UT_Vector * vecLabels = getVecTABLeadersLabel();
	UT_Vector * vecProps = getVecTABLeadersProp();
	UT_sint32 nTypes = vecLabels->getItemCount();
	UT_String * sProp = NULL;
	UT_String * sVal = NULL;
	UT_sint32 j = 0;
	sProp = new UT_String("toc-tab-leader");
	GtkWidget * wM = gtk_menu_new();
	for(j=0; j< nTypes; j++)
	{
		m_vecAllPropVals.addItem(static_cast<void *>(sProp));
		sVal = new UT_String(static_cast<char *>(vecProps->getNthItem(j)));
		m_vecAllPropVals.addItem(static_cast<void *>(sVal));
		const gchar * szLab = static_cast<const gchar *>(vecLabels->getNthItem(j));
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
	gtk_option_menu_set_menu(GTK_OPTION_MENU(_getWidget("wTabLeaderChoose")),wM);
}

void  AP_UnixDialog_FormatTOC::event_Apply(void)
{
	UT_DEBUGMSG(("Doing apply \n"));

// Heading Text

	GtkWidget * pW = _getWidget("wHeadingText");
	UT_UTF8String sVal = gtk_entry_get_text(GTK_ENTRY(pW));
	setTOCProperty("toc-heading",sVal.utf8_str());

// Text before and after

	pW = _getWidget("wTextAfter");
	sVal = gtk_entry_get_text(GTK_ENTRY(pW));
	UT_UTF8String sProp = static_cast<char *> (g_object_get_data(G_OBJECT(pW),"toc-prop"));
	UT_String sNum =  UT_String_sprintf("%d",m_iDetailsLevel);
	sProp += sNum.c_str();
	setTOCProperty(sProp,sVal);

	pW = _getWidget("wTextBefore");
	sVal = gtk_entry_get_text(GTK_ENTRY(pW));
	sProp = static_cast<char *> (g_object_get_data(G_OBJECT(pW),"toc-prop"));
	sProp += sNum.c_str();
	setTOCProperty(sProp,sVal);
	Apply();
}

gpointer AP_UnixDialog_FormatTOC::_makeProp(const char * szProp, UT_sint32 i)
{
	UT_String sLocal = szProp;
	UT_String sVal = UT_String_sprintf("%d",i);
	sLocal += sVal;
	UT_String * pS = new UT_String(sLocal);
	m_vecAllPropVals.addItem(static_cast<void *>(pS));
	return (gpointer) pS->c_str();
}

/*!
 * Fill the GUI tree with the styles as defined in the XP tree.
 */
void  AP_UnixDialog_FormatTOC::_fillGUI(void)
{
	UT_UTF8String sVal;
	sVal = getTOCPropVal("toc-has-heading");
	GtkWidget * pW = _getWidget("wHaveHeading");
	if(UT_stricmp(sVal.utf8_str(),"1") == 0)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pW),TRUE);
	}
	else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pW),FALSE);
	}
	g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer) "toc-has-heading");
	g_signal_connect(G_OBJECT(pW),
					 "toggled",
					 G_CALLBACK(s_check_changedMain),
					 (gpointer) this);

	sVal = getTOCPropVal("toc-heading");
	pW = _getWidget("wHeadingText");
	gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());
	g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer) "toc-heading");


	sVal = getTOCPropVal("toc-heading-style");
	pW = _getWidget("wHeadingStyle");
	gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());
	g_object_set_data(G_OBJECT(_getWidget("wChangeHeadingStyle")),"entry-widget",(gpointer)pW);
	g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer) "toc-heading-style");


	FV_View * pView = static_cast<FV_View *>(getActiveFrame()->getCurrentView());

	sVal = getTOCPropVal("toc-dest-style",m_iMainLevel);
	pW= _getWidget("wDispStyle");
	gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());
	g_object_set_data(G_OBJECT(_getWidget("wChangeDisp")),"entry-widget",(gpointer)pW);
	g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer) "toc-dest-style");


	sVal = getTOCPropVal("toc-has-label",m_iMainLevel);
	pW = _getWidget("wHaveLabel");
	if(UT_stricmp(sVal.utf8_str(),"1") == 0)
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
					 G_CALLBACK(s_check_changedMain),
					 (gpointer) this);

	sVal = getTOCPropVal("toc-label-after",m_iDetailsLevel);
	pW = _getWidget("wTextAfter");
	gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());
	g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer) "toc-label-after");

	sVal = getTOCPropVal("toc-label-before",m_iDetailsLevel);
	pW = _getWidget("wTextBefore");
	gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());
	g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer) "toc-label-before");

	sVal = getTOCPropVal("toc-label-inherits",m_iDetailsLevel);
	pW = _getWidget("wInherit");
	if(UT_stricmp(sVal.utf8_str(),"1") == 0)
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
/*
Have to deal with these later
	"toc-label-start1",
	"toc-label-start2",
	"toc-label-start3",
	"toc-label-start4",
*/
	sVal = getTOCPropVal("toc-label-type",m_iDetailsLevel);
	pW = _getWidget("wLabelChoose"); 
	UT_sint32 iHist = static_cast<UT_sint32>(pView->getLayout()->FootnoteTypeFromString(sVal.utf8_str()));
	gtk_option_menu_set_history(GTK_OPTION_MENU(pW),iHist);

	sVal = getTOCPropVal("toc-page-type",m_iDetailsLevel);
	pW = _getWidget("wPageStyleChoose"); 
	iHist = static_cast<UT_sint32>(pView->getLayout()->FootnoteTypeFromString(sVal.utf8_str()));
	gtk_option_menu_set_history(GTK_OPTION_MENU(pW),iHist);

	sVal = getTOCPropVal("toc-source-style",m_iMainLevel);
	pW = _getWidget("wFillStyle");
	gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());
	g_object_set_data(G_OBJECT(_getWidget("wChangeFill")),"entry-widget",(gpointer)pW);
	g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer) "toc-source-style");

	sVal = getTOCPropVal("toc-tab-leader",m_iDetailsLevel);
	pW = _getWidget("wTabLeaderChoose");
	if(UT_stricmp(sVal.utf8_str(),"none") == 0)
	{
		iHist = 0;
	}
	else if(UT_stricmp(sVal.utf8_str(),"dot") == 0)
	{
		iHist = 1;
	}
	else if(UT_stricmp(sVal.utf8_str(),"hyphen") == 0)
	{
		iHist = 2;
	}
	else if(UT_stricmp(sVal.utf8_str(),"underline") == 0)
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
	g_signal_connect(G_OBJECT(_getWidget("wChangeHeadingStyle")),
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
