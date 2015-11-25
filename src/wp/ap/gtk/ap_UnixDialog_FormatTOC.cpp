/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
 * Copyright (C) 2004 Martin Sevior
 * Copyright (C) 2009 Hubert Figuiere
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
#include <gtk/gtk.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "pt_PieceTable.h"
#include "xap_UnixDialogHelper.h"
#include "xap_GtkComboBoxHelpers.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "ap_Dialog_FormatFootnotes.h"
#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_UnixDialog_FormatTOC.h"


static void s_delete_clicked(GtkWidget * wid, AP_UnixDialog_FormatTOC * /*me*/ )
{
    abiDestroyWidget( wid ) ;// will emit signals for us
}

static void s_destroy_clicked(GtkWidget * /*wid*/, AP_UnixDialog_FormatTOC * me )
{
   me->event_Close();
}

void AP_UnixDialog_FormatTOC::s_NumType_changed(GtkWidget * wid, 
												AP_UnixDialog_FormatTOC * me )
{

	GtkTreeIter iter;
	GtkComboBox * combo = GTK_COMBO_BOX(wid);
	gtk_combo_box_get_active_iter(combo, &iter);
	GtkTreeModel *store = gtk_combo_box_get_model(combo);
	UT_UTF8String sProp;
	if(wid == me->m_wLabelChoose) {
		sProp = "toc-label-type";
	}
	else if (wid == me->m_wPageNumberingChoose) {
		sProp = "toc-page-type";
	}
	else {
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}
	char * value2;
	gtk_tree_model_get(store, &iter, 2, &value2, -1);

	UT_UTF8String sVal = value2;
	UT_String sNum =  UT_String_sprintf("%d",me->getDetailsLevel());
	sProp += sNum.c_str();
	me->setTOCProperty(sProp,sVal);
	g_free(value2);
}


static void s_TabLeader_changed(GtkWidget * wid, AP_UnixDialog_FormatTOC * me )
{

	GtkTreeIter iter;
	GtkComboBox * combo = GTK_COMBO_BOX(wid);
	gtk_combo_box_get_active_iter(combo, &iter);
	GtkTreeModel *store = gtk_combo_box_get_model(combo);
	char * value1;
	char * value2;
	gtk_tree_model_get(store, &iter, 1, &value1, 2, &value2, -1);

	UT_UTF8String sProp = value1;
	UT_UTF8String sVal = value2;
	UT_String sNum =  UT_String_sprintf("%d",me->getDetailsLevel());
	sProp += sNum.c_str();
	me->setTOCProperty(sProp,sVal);
}


static void s_MainLevel_changed(GtkWidget * wid, AP_UnixDialog_FormatTOC * me )
{
	UT_sint32 iLevel = XAP_comboBoxGetActiveInt(GTK_COMBO_BOX(wid));
	me->setMainLevel(iLevel);
}


static void s_DetailsLevel_changed(GtkWidget * wid, AP_UnixDialog_FormatTOC * me )
{
	UT_sint32 iLevel = XAP_comboBoxGetActiveInt(GTK_COMBO_BOX(wid));
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

static gboolean s_Text_changed (GtkWidget *widget, GdkEvent */*event*/, AP_UnixDialog_FormatTOC *me)
{
	UT_UTF8String sVal(gtk_entry_get_text(GTK_ENTRY(widget)));
	UT_UTF8String sProp;
	sProp = static_cast<const char *>(g_object_get_data(G_OBJECT(widget), "toc-prop"));
	UT_String sNum = UT_String_sprintf("%d", me->getDetailsLevel());
	sProp += sNum.c_str();
	me->setTOCProperty(sProp, sVal);
	return FALSE;
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
	  m_wLabelChoose(NULL),
	  m_wPageNumberingChoose(NULL),
	  m_pBuilder(NULL),
	  m_iIndentValue(1),
	  m_iStartValue(1)
{
}

AP_UnixDialog_FormatTOC::~AP_UnixDialog_FormatTOC(void)
{
  if (m_pBuilder)
    g_object_unref(G_OBJECT(m_pBuilder));
}

void AP_UnixDialog_FormatTOC::event_Close(void)
{
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
	setTOCProperty(sProp,sVal);
	applyTOCPropsToDoc();
}

void AP_UnixDialog_FormatTOC::setSensitivity(bool bSensitive)
{
	gtk_widget_set_sensitive (m_wApply, bSensitive ? TRUE : FALSE);
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
	gdk_window_raise (gtk_widget_get_window(m_windowMain));
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

void AP_UnixDialog_FormatTOC::notifyActiveFrame(XAP_Frame * /*pFrame*/)
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
	s_DetailsLevel_changed(_getWidget("wDetailsLevel"), this);
}

GtkWidget * AP_UnixDialog_FormatTOC::_getWidget(const char * szNameBase, UT_sint32 iLevel)
{
	UT_return_val_if_fail(m_pBuilder, NULL);

	UT_String sLocal = szNameBase;
	if(iLevel > 0)
	{
		UT_String sVal = UT_String_sprintf("%d",iLevel);
		sLocal += sVal;
	}
	return GTK_WIDGET(gtk_builder_get_object(m_pBuilder, sLocal.c_str()));
}

GtkWidget * AP_UnixDialog_FormatTOC::_constructWindow(void)
{
	m_pBuilder = newDialogBuilder("ap_UnixDialog_FormatTOC.ui");
	
	const XAP_StringSet * pSS = m_pApp->getStringSet ();

	m_windowMain   = _getWidget("ap_UnixDialog_FormatTOC");
	m_wApply = _getWidget("wApply");
	m_wClose = _getWidget("wClose");

	// set the dialog title
	std::string s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Title,s);
	abiDialogSetTitle(m_windowMain, "%s", s.c_str());

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
	std::string sLoc;
	sVal = getTOCPropVal("toc-dest-style",getMainLevel());
	GtkWidget * pW= _getWidget("wDispStyle");
	pt_PieceTable::s_getLocalisedStyleName(sVal.utf8_str(), sLoc);
	gtk_label_set_text(GTK_LABEL(pW), sLoc.c_str());


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
	pt_PieceTable::s_getLocalisedStyleName(sVal.utf8_str(), sLoc);
	gtk_label_set_text(GTK_LABEL(pW), sLoc.c_str());
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
	GtkComboBox *combo = GTK_COMBO_BOX(pW);
	UT_sint32 iHist = static_cast<UT_sint32>(pView->getLayout()->FootnoteTypeFromString(sVal.utf8_str()));
	gtk_combo_box_set_active(combo,iHist);

	sVal = getTOCPropVal("toc-page-type",getDetailsLevel());
	pW = _getWidget("wPageNumberingChoose"); 
	combo = GTK_COMBO_BOX(pW);
	iHist = static_cast<UT_sint32>(pView->getLayout()->FootnoteTypeFromString(sVal.utf8_str()));
	gtk_combo_box_set_active(combo,iHist);

	sVal = getTOCPropVal("toc-tab-leader",getDetailsLevel());
	pW = _getWidget("wTabLeaderChoose");
	combo = GTK_COMBO_BOX(pW);
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
	gtk_combo_box_set_active(combo,iHist);
}

void AP_UnixDialog_FormatTOC::_createLevelItems(void)
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet ();
	std::string s;

	GtkComboBox *combo;

	combo = GTK_COMBO_BOX(_getWidget("wLevelOption"));
	XAP_makeGtkComboBoxText(combo, G_TYPE_INT);
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level1,s);
	XAP_appendComboBoxTextAndInt(combo, s.c_str(), 1);
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level2,s);
	XAP_appendComboBoxTextAndInt(combo, s.c_str(), 2);
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level3,s);
	XAP_appendComboBoxTextAndInt(combo, s.c_str(), 3);
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level4,s);
	XAP_appendComboBoxTextAndInt(combo, s.c_str(), 4);
	gtk_combo_box_set_active(combo, 0);

//////////////////////////////////////////////////////////////////////////////

	combo = GTK_COMBO_BOX(_getWidget("wDetailsLevel"));
	XAP_makeGtkComboBoxText(combo, G_TYPE_INT);
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level1,s);
	XAP_appendComboBoxTextAndInt(combo, s.c_str(), 1);
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level2,s);
	XAP_appendComboBoxTextAndInt(combo, s.c_str(), 2);
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level3,s);
	XAP_appendComboBoxTextAndInt(combo, s.c_str(), 3);
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Level4,s);
	XAP_appendComboBoxTextAndInt(combo, s.c_str(), 4);
	gtk_combo_box_set_active(combo, 0);
}

void AP_UnixDialog_FormatTOC::_createLabelTypeItems(void)
{
	const FootnoteTypeDesc* vecTypeList = AP_Dialog_FormatFootnotes::getFootnoteTypeLabelList();

//	sProp = new UT_UTF8String("toc-label-type");
	m_wLabelChoose = _getWidget("wLabelChoose");
	GtkComboBox * combo = GTK_COMBO_BOX(m_wLabelChoose);
	XAP_makeGtkComboBoxText2(combo, G_TYPE_INT, G_TYPE_STRING);
	const FootnoteTypeDesc* current = vecTypeList;
	for(; current->n != _FOOTNOTE_TYPE_INVALID; current++)
	{
		UT_DEBUGMSG(("Got label %s for prop %s \n",current->label,
					 current->prop));
		XAP_appendComboBoxTextAndIntString(combo, current->label, 
										   current->n, current->prop);
	}

// Now the Page Numbering style
//
//	sProp = new UT_UTF8String("toc-page-type");
	m_wPageNumberingChoose = _getWidget("wPageNumberingChoose");
	combo = GTK_COMBO_BOX(m_wPageNumberingChoose);
	XAP_makeGtkComboBoxText2(combo, G_TYPE_INT, G_TYPE_STRING);
	current = vecTypeList;
	for(; current->n != _FOOTNOTE_TYPE_INVALID; current++)
	{
		XAP_appendComboBoxTextAndIntString(combo, current->label, 
										   current->n, current->prop);

	}
}


void AP_UnixDialog_FormatTOC::_createTABTypeItems(void)
{
	const UT_GenericVector<const gchar*> * vecLabels = getVecTABLeadersLabel();
	const UT_GenericVector<const gchar*> * vecProps = getVecTABLeadersProp();
	UT_sint32 nTypes = vecLabels->getItemCount();
	UT_sint32 j = 0;
	const char *sProp = "toc-tab-leader";
	GtkComboBox * combo = GTK_COMBO_BOX(_getWidget("wTabLeaderChoose"));
	XAP_makeGtkComboBoxText2(combo, G_TYPE_STRING, G_TYPE_STRING);
	for(j=0; j< nTypes; j++)
	{
		const gchar *sVal = vecProps->getNthItem(j);
		const gchar * szLab = vecLabels->getNthItem(j);
		UT_DEBUGMSG(("Got label %s for item %d \n",szLab,j));
		XAP_appendComboBoxTextAndStringString(combo, szLab, sProp, sVal);
	}
}

void  AP_UnixDialog_FormatTOC::event_Apply(void)
{
	UT_DEBUGMSG(("Doing apply \n"));

// Heading Text

	GtkWidget * pW = _getWidget("edHeadingText");
	UT_UTF8String sVal;
	sVal = gtk_entry_get_text(GTK_ENTRY(pW));
	setTOCProperty("toc-heading",sVal.utf8_str());

// Text before and after

	pW = _getWidget("edTextAfter");
	sVal = gtk_entry_get_text(GTK_ENTRY(pW));
	UT_UTF8String sProp;
	sProp = static_cast<const char *> (g_object_get_data(G_OBJECT(pW),"toc-prop"));
	UT_String sNum =  UT_String_sprintf("%d",getDetailsLevel());
	sProp += sNum.c_str();
	setTOCProperty(sProp,sVal);

	pW = _getWidget("edTextBefore");
	sVal = gtk_entry_get_text(GTK_ENTRY(pW));
	sProp = static_cast<const char *> (g_object_get_data(G_OBJECT(pW),"toc-prop"));
	sProp += sNum.c_str();
	setTOCProperty(sProp,sVal);
	Apply();
}

/*!
 * Fill the GUI tree with the styles as defined in the XP tree.
 */
void  AP_UnixDialog_FormatTOC::_fillGUI(void)
{
	UT_UTF8String sVal;
	std::string sLoc;
	sVal = getTOCPropVal("toc-has-heading");

	GtkWidget * pW;
	GtkComboBox * combo = GTK_COMBO_BOX(_getWidget("wLevelOption"));
	gtk_combo_box_set_active(combo, getMainLevel()-1);

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
	pt_PieceTable::s_getLocalisedStyleName(sVal.utf8_str(), sLoc);
	gtk_label_set_text(GTK_LABEL(pW), sLoc.c_str());
	g_object_set_data(G_OBJECT(_getWidget("lbChangeHeadingStyle")),"display-widget",(gpointer)pW);
	g_object_set_data(G_OBJECT(pW),"toc-prop",(gpointer) "toc-heading-style");


	FV_View * pView = static_cast<FV_View *>(getActiveFrame()->getCurrentView());

	sVal = getTOCPropVal("toc-dest-style",getMainLevel());
	pW= _getWidget("wDispStyle");
	pt_PieceTable::s_getLocalisedStyleName(sVal.utf8_str(), sLoc);
	gtk_label_set_text(GTK_LABEL(pW), sLoc.c_str());
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
	XAP_comboBoxSetActiveFromIntCol(GTK_COMBO_BOX(pW),1,iHist);

	sVal = getTOCPropVal("toc-page-type",getDetailsLevel());
	pW = _getWidget("wPageNumberingChoose"); 
	iHist = static_cast<UT_sint32>(pView->getLayout()->FootnoteTypeFromString(sVal.utf8_str()));
	XAP_comboBoxSetActiveFromIntCol(GTK_COMBO_BOX(pW),1,iHist);

	sVal = getTOCPropVal("toc-source-style",getMainLevel());
	pW = _getWidget("wFillStyle");
	pt_PieceTable::s_getLocalisedStyleName(sVal.utf8_str(), sLoc);
	gtk_label_set_text(GTK_LABEL(pW), sLoc.c_str());
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
	gtk_combo_box_set_active(GTK_COMBO_BOX(pW),iHist);
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

	g_signal_connect(G_OBJECT(_getWidget("wLevelOption")),
					 "changed",
					 G_CALLBACK(s_MainLevel_changed),
					 (gpointer) this);
	g_signal_connect(G_OBJECT(_getWidget("wDetailsLevel")),
					 "changed",
					 G_CALLBACK(s_DetailsLevel_changed),
					 (gpointer) this);
	g_signal_connect(G_OBJECT(_getWidget("wLabelChoose")),
					 "changed",
					 G_CALLBACK(s_NumType_changed),
					 (gpointer) this);
	g_signal_connect(G_OBJECT(_getWidget("wPageNumberingChoose")),
					 "changed",
					 G_CALLBACK(s_NumType_changed),
					 (gpointer) this);
	g_signal_connect(G_OBJECT(_getWidget("wTabLeaderChoose")),
					 "changed",
					 G_CALLBACK(s_TabLeader_changed),
					 (gpointer) this);
	g_signal_connect(G_OBJECT(_getWidget("edTextBefore")),
					 "focus-out-event",
					 G_CALLBACK(s_Text_changed),
					 (gpointer) this);
	g_signal_connect(G_OBJECT(_getWidget("edTextAfter")),
					 "focus-out-event",
					 G_CALLBACK(s_Text_changed),
					 (gpointer) this);
}
