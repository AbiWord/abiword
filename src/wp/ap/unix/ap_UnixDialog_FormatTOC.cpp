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


static void s_set_style(GtkWidget * wid, AP_UnixDialog_FormatTOC * me )
{
	me->setStyle(wid);
}

static void s_check_changed(GtkWidget * wid, AP_UnixDialog_FormatTOC * me)
{
	UT_UTF8String sProp = static_cast<char *> (g_object_get_data(G_OBJECT(wid),"toc-prop"));
	UT_UTF8String sVal = "1";
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wid)) == FALSE)
	{
		sVal = "0";
	}
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
	_fillGUI();
}

void AP_UnixDialog_FormatTOC::setStyle(GtkWidget * wid)
{
	UT_UTF8String sVal;
	GtkWidget * pEntry = static_cast<GtkWidget *> (g_object_get_data(G_OBJECT(wid),"entry-widget"));
	UT_UTF8String sProp = static_cast<char *> (g_object_get_data(G_OBJECT(pEntry),"toc-prop"));
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
	_createTABTypeItems();
	return m_windowMain;
}

void AP_UnixDialog_FormatTOC::_createLabelTypeItems(void)
{
	UT_sint32 i =1;
	UT_Vector * vecTypeList = AP_Dialog_FormatFootnotes::getFootnoteTypeLabelList();
	UT_Vector * vecPropList = getVecLabelPropValue();
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
			const gchar * szVal = static_cast<const gchar *>(vecPropList->getNthItem(j));
			g_object_set_data(G_OBJECT(pW),"toc-val",(gpointer)(szVal));

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
			const gchar * szVal = static_cast<const gchar *>(vecPropList->getNthItem(j));
			g_object_set_data(G_OBJECT(pW),"toc-val",(gpointer)(szVal));


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


void AP_UnixDialog_FormatTOC::_createTABTypeItems(void)
{
	UT_sint32 i =1;
	UT_Vector * vecLabels = getVecTABLeadersLabel();
	UT_Vector * vecProps = getVecTABLeadersProp();
	UT_sint32 nTypes = vecLabels->getItemCount();
	UT_String * sProp = NULL;
	UT_String * sVal = NULL;
	for(i=1;i<=4;i++)
	{
		UT_sint32 j = 0;
		sProp = new UT_String("toc-tab-leader");
		UT_String sTmp = UT_String_sprintf("%d",i);
		*sProp += sTmp;
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
		gtk_option_menu_set_menu(GTK_OPTION_MENU(_getWidget("wTabLeaderChoose",i)),wM);
	}
			
}

void  AP_UnixDialog_FormatTOC::event_Apply(void)
{
	UT_DEBUGMSG(("Doing apply \n"));

// Heading Text

	GtkWidget * pW = _getWidget("wHeadingText");
	UT_UTF8String sVal = gtk_entry_get_text(GTK_ENTRY(pW));
	setTOCProperty("toc-heading",sVal.utf8_str());


	UT_sint32 i =1;
	for(i=1; i< 4;i++)
	{
		pW = _getWidget("wTextAfter",i);
		sVal = gtk_entry_get_text(GTK_ENTRY(pW));
		UT_UTF8String sProp = static_cast<char *> (g_object_get_data(G_OBJECT(pW),"toc-prop"));
		setTOCProperty(sProp,sVal);

		pW = _getWidget("wTextBefore",i);
		sVal = gtk_entry_get_text(GTK_ENTRY(pW));
		sProp = static_cast<char *> (g_object_get_data(G_OBJECT(pW),"toc-prop"));
		setTOCProperty(sProp,sVal);
	}
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
					 G_CALLBACK(s_check_changed),
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

	UT_sint32 i =1;
	FV_View * pView = static_cast<FV_View *>(getActiveFrame()->getCurrentView());
	for(i=1; i<= 4;i++)
	{
		sVal = getTOCPropVal("toc-dest-style",i);
		pW= _getWidget("wDispStyle",i);
		gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());
		g_object_set_data(G_OBJECT(_getWidget("wChangeDisp",i)),"entry-widget",(gpointer)pW);
		g_object_set_data(G_OBJECT(pW),"toc-prop",_makeProp("toc-dest-style",i));


		sVal = getTOCPropVal("toc-has-label",i);
		pW = _getWidget("wHaveLabel",i);
		if(UT_stricmp(sVal.utf8_str(),"1") == 0)
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pW),TRUE);
		}
		else
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pW),FALSE);
		}
		g_object_set_data(G_OBJECT(pW),"toc-prop",_makeProp("toc-has-label",i));
		g_signal_connect(G_OBJECT(pW),
					 "toggled",
					 G_CALLBACK(s_check_changed),
					 (gpointer) this);

		sVal = getTOCPropVal("toc-label-after",i);
		pW = _getWidget("wTextAfter",i);
		gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());
		g_object_set_data(G_OBJECT(pW),"toc-prop",_makeProp("toc-label-after",i));

		sVal = getTOCPropVal("toc-label-before",i);
		pW = _getWidget("wTextBefore",i);
		gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());
		g_object_set_data(G_OBJECT(pW),"toc-prop",_makeProp("toc-label-before",i));

		if(i>1)
		{
			sVal = getTOCPropVal("toc-label-inherits",i);
			pW = _getWidget("wInherit",i);
			if(UT_stricmp(sVal.utf8_str(),"1") == 0)
			{
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pW),TRUE);
			}
			else
			{
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pW),FALSE);
			}
			g_object_set_data(G_OBJECT(pW),"toc-prop",_makeProp("toc-label-inherits",i));
			g_signal_connect(G_OBJECT(pW),
							 "toggled",
							 G_CALLBACK(s_check_changed),
							 (gpointer) this);
		}
/*
Have to deal with these later
	"toc-label-start1",
	"toc-label-start2",
	"toc-label-start3",
	"toc-label-start4",
*/
		sVal = getTOCPropVal("toc-label-type",i);
		pW = _getWidget("wLabelChoose",i); 
		UT_sint32 iHist = static_cast<UT_sint32>(pView->getLayout()->FootnoteTypeFromString(sVal.utf8_str()));
		gtk_option_menu_set_history(GTK_OPTION_MENU(pW),iHist);

		sVal = getTOCPropVal("toc-page-type",i);
		pW = _getWidget("wPageStyleChoose",i); 
		iHist = static_cast<UT_sint32>(pView->getLayout()->FootnoteTypeFromString(sVal.utf8_str()));
		gtk_option_menu_set_history(GTK_OPTION_MENU(pW),iHist);

		sVal = getTOCPropVal("toc-source-style",i);
		pW = _getWidget("wFillStyle",i);
		gtk_entry_set_text(GTK_ENTRY(pW),sVal.utf8_str());
		g_object_set_data(G_OBJECT(_getWidget("wChangeFill",i)),"entry-widget",(gpointer)pW);
		g_object_set_data(G_OBJECT(pW),"toc-prop",_makeProp("toc-source-style",i));

		sVal = getTOCPropVal("toc-tab-leader",i);
		pW = _getWidget("wTabLeaderChoose",i);
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
	UT_sint32 i =1;
	for(i=1 ;i<4; i++)
	{
		g_signal_connect(G_OBJECT(_getWidget("wChangeFill",i)),
					  "clicked",
					  G_CALLBACK(s_set_style),
					  (gpointer) this);
		g_signal_connect(G_OBJECT(_getWidget("wChangeDisp",i)),
					  "clicked",
					  G_CALLBACK(s_set_style),
					  (gpointer) this);
	}
}
