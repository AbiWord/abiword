/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
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
#include <stdio.h>
#include <string.h>

#include "ap_Features.h"

#include "ut_assert.h"
#include "ut_string.h"
#include "ut_std_string.h"
#include "ut_debugmsg.h"

#include "ap_Dialog_Id.h"

#include "ap_Dialog_FormatTOC.h"
#include "ap_Strings.h"

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "fv_View.h"
#include "pd_Document.h"
#include "pd_Style.h"
#include "pt_Types.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fp_ContainerObject.h"
#include "fp_TableContainer.h"
#include "fl_TableLayout.h"
#include "fl_BlockLayout.h"
#include "fl_DocLayout.h"
#include "ut_timer.h"
#include "pd_Document.h"
#include "ap_Dialog_Stylist.h"
#include "pp_Property.h"

AP_Dialog_FormatTOC::AP_Dialog_FormatTOC(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_Modeless(pDlgFactory,id),
	  m_pDoc(NULL),
	  m_pAutoUpdater(0),
	  m_iTick(0),
	  m_pAP(NULL),
	  m_bTOCFilled(false),
	  m_sTOCProps(""),
  	  m_iMainLevel(1),
	  m_iDetailsLevel(1)
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet ();
	static std::string s1, s2, s3, s4;

	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_None, s1);
	m_vecTABLeadersLabel.addItem(s1.c_str());
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Dot, s2);
	m_vecTABLeadersLabel.addItem(s2.c_str());
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Dash, s3);
	m_vecTABLeadersLabel.addItem(s3.c_str());
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Underline, s4);
	m_vecTABLeadersLabel.addItem(s4.c_str());
	m_vecTABLeadersProp.addItem("none");
	m_vecTABLeadersProp.addItem("dot");
	m_vecTABLeadersProp.addItem("hyphen");
	m_vecTABLeadersProp.addItem("underline");
}

AP_Dialog_FormatTOC::~AP_Dialog_FormatTOC(void)
{
	stopUpdater();
}

void AP_Dialog_FormatTOC::setActiveFrame(XAP_Frame * /*pFrame*/)
{
	updateDialog();
	notifyActiveFrame(getActiveFrame());
}

void AP_Dialog_FormatTOC::startUpdater(void)
{
	m_pAutoUpdater =  UT_Timer::static_constructor(autoUpdate,this);
	m_pAutoUpdater->set(500);
	m_pAutoUpdater->start();
}

/*!
 * Apply current style to the current selection in the current view
 */
void AP_Dialog_FormatTOC::Apply(void)
{
	FV_View * pView = static_cast<FV_View *>(getActiveFrame()->getCurrentView());
	if(pView->getPoint() == 0)
	{
		return;
	}
	if(!pView->isTOCSelected())
	{
		setSensitivity(false);
		return;
	}
	applyTOCPropsToDoc();
}

void AP_Dialog_FormatTOC::stopUpdater(void)
{
	if(m_pAutoUpdater == NULL)
	{
		return;
	}
	m_pAutoUpdater->stop();
	DELETEP(m_pAutoUpdater);
	m_pAutoUpdater = NULL;
}

/*!
 * Autoupdater of the dialog.
 */
void AP_Dialog_FormatTOC::autoUpdate(UT_Worker * pTimer)
{

	UT_return_if_fail (pTimer);
	
// this is a static callback method and does not have a 'this' pointer

	AP_Dialog_FormatTOC * pDialog = static_cast<AP_Dialog_FormatTOC *>(pTimer->getInstanceData());
	pDialog->updateDialog();
}

std::string AP_Dialog_FormatTOC::getNewStyle(const std::string & sProp) const
{
	// Handshaking code
	static std::string sNewStyle;
	FV_View * pView = static_cast<FV_View *>(getActiveFrame()->getCurrentView());
	if(pView->getPoint() == 0)
	{
		return sNewStyle;
	}
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	UT_return_val_if_fail (pFrame, sNewStyle);
	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	// use justMakeTheDialog instead of requestDialog to allow modeless and modal versions of
	// the stylist to exist

	AP_Dialog_Stylist * pDialog
		= static_cast<AP_Dialog_Stylist *>(pDialogFactory->justMakeTheDialog((AP_DIALOG_ID_STYLIST)));
	UT_return_val_if_fail (pDialog, sNewStyle);
	std::string sVal = getTOCPropVal(sProp);

	pDialog->setCurStyle(sVal);
	pDialog->runModal(pFrame);
	if(pDialog->isStyleValid())
	{
		sNewStyle = pDialog->getSelectedStyle();
	}
	pDialogFactory->releaseDialog(pDialog);
	return sNewStyle;
}

/*!
 * This method actually updates the dialog, in particular the style Tree and 
 * the current style.
 */
void AP_Dialog_FormatTOC::updateDialog(void)
{
	XAP_Frame * pFrame = getActiveFrame();
	if (pFrame == 0)
	{
		setSensitivity(false);
		return;
	}
	// Handshaking code
	FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
	if(pView->getPoint() == 0)
	{
		return;
	}
	if(!pView->isTOCSelected())
	{
		setSensitivity(false);
		return;
	}
	setSensitivity(true);
	PD_Document * pDoc = pView->getDocument();
	if((m_iTick != pView->getTick()) || (m_pDoc != pDoc) || !m_bTOCFilled)
	{
		m_iTick = pView->getTick();
		if(pDoc != m_pDoc)
		{
			m_pDoc = pDoc;
		}
		fillTOCPropsFromDoc();
		setTOCPropsInGUI();
		return;
	}
}			

/*!
 * Finalize the dialog.
 */
void  AP_Dialog_FormatTOC::finalize(void)
{
	stopUpdater();
	modeless_cleanup();
}

std::string AP_Dialog_FormatTOC::getTOCPropVal(const std::string & sProp) const
{
	return UT_std_string_getPropVal(m_sTOCProps,sProp);
}


std::string AP_Dialog_FormatTOC::getTOCPropVal(const char * szProp) const
{
	return UT_std_string_getPropVal(m_sTOCProps,szProp);
}


std::string AP_Dialog_FormatTOC::getTOCPropVal(const char * szProp, UT_sint32 i) const
{
	std::string sProp = szProp;
	std::string sVal = UT_std_string_sprintf("%d",i);
	sProp += sVal;
	return UT_std_string_getPropVal(m_sTOCProps,sProp);
}

void AP_Dialog_FormatTOC::setTOCProperty(const char * szProp, const char * szVal)
{
	std::string sProp = szProp;
	std::string sVal = szVal;
/*	don't return on empty prop strings - see Bug 9141
	if(sVal.size() == 0)
	{
		return;
	}
*/
	UT_DEBUGMSG((" Prop: %s Val: %s \n",sProp.c_str(),sVal.c_str()));
	UT_std_string_setProperty(m_sTOCProps,sProp,sVal);
//	m_sTOCProps.dump();
}

void AP_Dialog_FormatTOC::setTOCProperty(const std::string & sProp,
					 const std::string & sVal)
{
/*	don't return on empty prop strings - see Bug 9141
	if(sVal.size() == 0)
	{
		return;
	}
*/
	UT_DEBUGMSG((" Prop: %s Val: %s \n",sProp.c_str(),sVal.c_str()));
	UT_std_string_setProperty(m_sTOCProps,sProp,sVal);
//	m_sTOCProps.dump();
}

/*!
Retrieves a property value from the document, and stores it in the dialog property list.
@param szProp The property name to retrieve
@return true if the property value was retrieved from the document, false if 
        the property value was retrieved from default property values list or could
		not be retrieved at all.
*/
bool AP_Dialog_FormatTOC::setPropFromDoc(const char * szProp)
{
	UT_return_val_if_fail (m_pAP, false);
	bool bRes = true;
	const char * szVal = NULL;
	m_pAP->getProperty(szProp,szVal);
	if(szVal == NULL)
	{
		bRes = false;
		const PP_Property * pProp = PP_lookupProperty(szProp);
		if(pProp == NULL)
		{
			UT_ASSERT_HARMLESS(0);
			return bRes;
		}
		szVal = pProp->m_pszInitial;
	}
	setTOCProperty(szProp,szVal);
	return bRes;
}

/*!
 * Increment the "start at" property
 */
void AP_Dialog_FormatTOC::incrementStartAt(UT_sint32 iLevel, bool bInc)
{
	std::string sProp("toc-label-start");
	sProp += UT_std_string_sprintf("%d",iLevel);
	std::string sStartVal = getTOCPropVal(sProp);
	UT_sint32 iVal = atoi(sStartVal.c_str());
	if(bInc)
	{
		iVal++;
	}
	else
	{
		iVal--;
	}
	sStartVal = UT_std_string_sprintf("%d",iVal);
	setTOCProperty(sProp,sStartVal);
}


/*!
 * Increment the "indent" property
 */
void AP_Dialog_FormatTOC::incrementIndent(UT_sint32 iLevel, bool bInc)
{
	std::string sProp = "toc-indent";
	sProp += UT_std_string_sprintf("%d",iLevel);
	std::string sVal = getTOCPropVal(sProp);
	double inc = getIncrement(sVal.c_str());
	if(!bInc)
	{
		inc = -inc;
	}
	sVal = UT_incrementDimString(sVal.c_str(),inc);
	setTOCProperty(sProp,sVal);
}


/*!
 * Returns the increment associated with the dimension defined in the string.
\param const char * sz the dimensioned string.
\returns double -  the increment associated with the dimension in sz
*/
double AP_Dialog_FormatTOC::getIncrement(const char * sz)
{
	double inc = 0.1;
	UT_Dimension dim =  UT_determineDimension(sz);
	if(dim == DIM_IN)
	{
		inc = 0.02;
	}
	else if(dim == DIM_CM)
	{
		inc = 0.1;
	}
	else if(dim == DIM_MM)
	{
		inc = 1.0;
	}
	else if(dim == DIM_PI)
	{
		inc = 1.0;
	}
	else if(dim == DIM_PT)
	{
		inc = 1.0;
	}
	else if(dim == DIM_PX)
	{
		inc = 1.0;
	}
	else
	{
		inc = 0.02;
	}
	return inc;
}

void AP_Dialog_FormatTOC::fillTOCPropsFromDoc(void)
{
	FV_View * pView = static_cast<FV_View *>(getActiveFrame()->getCurrentView());
	PD_Document * pDoc = pView->getDocument();
	if(pDoc != m_pDoc)
	{
		m_pDoc = pDoc;
	}
	if(!pView->isTOCSelected())
	{
		fl_BlockLayout * pBL = pView->getCurrentBlock();
		pBL->getAP(m_pAP);
	}
	else
	{
		PT_DocPosition pos = pView->getSelectionAnchor()+1;
		pf_Frag_Strux* sdhTOC = NULL;
		m_pDoc->getStruxOfTypeFromPosition(pos,PTX_SectionTOC, &sdhTOC);
		UT_return_if_fail (sdhTOC);
//
// OK Now lets gets all props from here and place them in our local cache
//
		PT_AttrPropIndex iAPI = m_pDoc->getAPIFromSDH(sdhTOC);
		m_pDoc->getAttrProp(iAPI,&m_pAP);
	}
	m_bTOCFilled = true;
	setPropFromDoc("toc-dest-style1");
	setPropFromDoc("toc-dest-style2");
	setPropFromDoc("toc-dest-style3");
	setPropFromDoc("toc-dest-style4");

	setPropFromDoc("toc-has-heading");

	setPropFromDoc("toc-has-label1");
	setPropFromDoc("toc-has-label2");
	setPropFromDoc("toc-has-label3");
	setPropFromDoc("toc-has-label4");

	bool bRes = setPropFromDoc("toc-heading");
	if (!bRes)
	{
		std::string pszTOCHeading;
		const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet ();
		pSS->getValueUTF8(AP_STRING_ID_TOC_TocHeading, pszTOCHeading);
		setTOCProperty("toc-heading", pszTOCHeading.c_str());
	}
	setPropFromDoc("toc-heading-style");
	setPropFromDoc("toc-id");

	setPropFromDoc("toc-indent1");
	setPropFromDoc("toc-indent2");
	setPropFromDoc("toc-indent3");
	setPropFromDoc("toc-indent4");

	setPropFromDoc("toc-label-after1");
	setPropFromDoc("toc-label-after2");
	setPropFromDoc("toc-label-after3");
	setPropFromDoc("toc-label-after4");

	setPropFromDoc("toc-label-before1");
	setPropFromDoc("toc-label-before2");
	setPropFromDoc("toc-label-before3");
	setPropFromDoc("toc-label-before4");

	setPropFromDoc("toc-label-inherits1");
	setPropFromDoc("toc-label-inherits2");
	setPropFromDoc("toc-label-inherits3");
	setPropFromDoc("toc-label-inherits4");

	setPropFromDoc("toc-label-start1");
	setPropFromDoc("toc-label-start2");
	setPropFromDoc("toc-label-start3");
	setPropFromDoc("toc-label-start4");

	setPropFromDoc("toc-label-type1");
	setPropFromDoc("toc-label-type2");
	setPropFromDoc("toc-label-type3");
	setPropFromDoc("toc-label-type4");

	setPropFromDoc("toc-page-type1");
	setPropFromDoc("toc-page-type2");
	setPropFromDoc("toc-page-type3");
	setPropFromDoc("toc-page-type4");

	setPropFromDoc("toc-source-style1");
	setPropFromDoc("toc-source-style2");
	setPropFromDoc("toc-source-style3");
	setPropFromDoc("toc-source-style4");

	setPropFromDoc("toc-tab-leader1");
	setPropFromDoc("toc-tab-leader2");
	setPropFromDoc("toc-tab-leader3");
	setPropFromDoc("toc-tab-leader4");

	setPropFromDoc("toc-label-start1");
	setPropFromDoc("toc-label-start2");
	setPropFromDoc("toc-label-start3");
	setPropFromDoc("toc-label-start4");
}

void AP_Dialog_FormatTOC::applyTOCPropsToDoc(void)
{
	FV_View * pView = static_cast<FV_View *>(getActiveFrame()->getCurrentView());
	PT_DocPosition pos = pView->getSelectionAnchor()+1;
	pView->setTOCProps(pos,m_sTOCProps.c_str());
//	m_sTOCProps.dump();
}
