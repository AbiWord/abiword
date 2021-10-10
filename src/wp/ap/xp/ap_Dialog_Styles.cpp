/* -*- mode: C++; tab-width: 4; c-basic-offset: 4;  indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2021 Hubert Figuière
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
#include "ut_debugmsg.h"
#include "ut_misc.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "fl_SectionLayout.h"
#include "fp_Page.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "ut_misc.h"
#include "ap_TopRuler.h"
#include "pd_Style.h"
#include "ap_Dialog_Styles.h"
#include "ut_std_string.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"

#include "xap_Dlg_Language.h"
#include "ap_Dialog_Lists.h"
#include "ap_Dialog_Tab.h"

AP_Dialog_Styles::AP_Dialog_Styles(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialogstyles")
{

	m_answer = a_OK;
	m_pParaPreview = NULL;
	m_pCharPreview = NULL;
	m_pAbiPreview = NULL;
}

AP_Dialog_Styles::~AP_Dialog_Styles(void)
{
	DELETEP(m_pParaPreview);
	DELETEP(m_pCharPreview);
	DELETEP(m_pAbiPreview);
}

AP_Dialog_Styles::tAnswer AP_Dialog_Styles::getAnswer(void) const
{
	return m_answer;
}

/*!
 * This method extracts  the properties defined at the current point in the active
 * document and places them in a vectore where they can be easily displayed in
 * previews and the display area.
 */
void AP_Dialog_Styles::fillVecFromCurrentPoint(void)
{
	PP_PropertyVector paraProps;
//	getView()->getBlockFormat(paraProps,false);
//
// This line expands all styles
	getView()->getBlockFormat(paraProps,true);
	PP_PropertyVector charProps;
//	getView()->getCharFormat(charProps,false);
//
// This line expans all styles..
//
	getView()->getCharFormat(charProps,true);
//
// Loop over all properties and add them to our vector
//
	m_vecAllProps.clear();

	for (auto iter = paraProps.cbegin(); iter != paraProps.cend(); ++iter) {
		const std::string & szName = *iter;
		++iter;
		if (iter == paraProps.cend()) {
			break;
		}
		const std::string & szValue = *iter;
		if(szName.find("toc-") == std::string::npos) {
			m_vecAllProps.push_back(szName);
			m_vecAllProps.push_back(szValue);
		}
	}
	m_vecAllProps.insert(m_vecAllProps.end(), charProps.cbegin(), charProps.cend());
}

/*!
 * This method takes a style and extracts all the properties associated with it and
 * place them in a properties and attributes vector for easy modification by
 * the code.
 */
void AP_Dialog_Styles::fillVecWithProps(const gchar * szStyle, bool bReplaceAttributes = true)
{
	PD_Style * pStyle = NULL;
	m_vecAllProps.clear();
	if( bReplaceAttributes)
		m_vecAllAttribs.clear();
	if(szStyle == NULL || ! getDoc()->getStyle(szStyle,&pStyle))
	{
		return;
	}

	const static gchar * paraFields[] = {"text-align", "text-indent", "margin-left", "margin-right", "margin-top", "margin-bottom", "line-height","tabstops","start-value","list-delim", "list-style","list-decimal","field-font","field-color", "keep-together","keep-with-next","orphans","widows","dom-dir"};

	const size_t nParaFlds = sizeof(paraFields)/sizeof(paraFields[0]);

	const static gchar * charFields[] =
	{"bgcolor","color","font-family","font-size","font-stretch","font-style",
	 "font-variant", "font-weight","text-decoration","lang"};

	const size_t nCharFlds = sizeof(charFields)/sizeof(charFields[0]);

	const static gchar * attribs[] =
	{PT_FOLLOWEDBY_ATTRIBUTE_NAME,PT_BASEDON_ATTRIBUTE_NAME,PT_LISTID_ATTRIBUTE_NAME,PT_PARENTID_ATTRIBUTE_NAME,PT_LEVEL_ATTRIBUTE_NAME,PT_NAME_ATTRIBUTE_NAME,PT_STYLE_ATTRIBUTE_NAME,PT_TYPE_ATTRIBUTE_NAME};

	const size_t nattribs = sizeof(attribs)/sizeof(attribs[0]);
	UT_uint32 i;
	UT_DEBUGMSG(("Looking at Style %s \n",szStyle));
//
// Loop through all Paragraph properties and add those with non-null values
//
	for(i = 0; i < nParaFlds; i++)
	{
		const gchar * szName = paraFields[i];
		const gchar * szValue = NULL;
		pStyle->getProperty(szName,szValue);
		if(szValue) {
			PP_addOrSetAttribute(szName, szValue, m_vecAllProps);
		}
	}
//
// Loop through all Character properties and add those with non-null values
//
	for(i = 0; i < nCharFlds; i++)
	{
		const gchar * szName = charFields[i];
		const gchar * szValue = NULL;
		pStyle->getProperty(szName,szValue);
		if(szValue)
		{
			xxx_UT_DEBUGMSG(("Adding char prop %s value %s \n",szName,szValue));
			PP_addOrSetAttribute(szName, szValue, m_vecAllProps);
		}
	}
//
// Loop through all the attributes and add those with non-null values
//
	xxx_UT_DEBUGMSG(("Replace Attributes %d \n",bReplaceAttributes));
	if(bReplaceAttributes)
	{
		for(i = 0; i < nattribs; i++)
		{
			const gchar * szName = attribs[i];
			const gchar * szValue = NULL;
			pStyle->getAttributeExpand(szName,szValue);
			if(szValue) {
				PP_addOrSetAttribute(szName, szValue, m_vecAllAttribs);
			}
		}
	}
	else
	{
		UT_DEBUGMSG(("Attributes NOT updated \n"));
	}
}

/*!
 * This method returns a pointer to the const char * value associated with the
 * the attribute szProp. This version will extract properties from any vector
 * with name:values set as:
 * It assumes properties and values are stored the array like this:
 * vecProp(n)   :   vecProp(n+1)
 * "attribute"   :   "value"
 */
const gchar * AP_Dialog_Styles::getVecVal(const UT_Vector *v, const gchar * szProp) const
{
	UT_sint32 i = v->getItemCount();
	if(i <= 0)
		return NULL;
	UT_sint32 j;
	const gchar * pszV = NULL;
	for(j= 0; j<i ;j=j+2)
	{
		pszV = (const gchar *) v->getNthItem(j);
		if( (pszV != NULL) && (strcmp( pszV,szProp) == 0))
			break;
	}
	if( j < i )
		return  (const gchar *) v->getNthItem(j+1);
	else
		return NULL;
}

/*!
 * This method runs the language dialog to allow the user to set the language
 * property of their style.
 */
void AP_Dialog_Styles::ModifyLang(void)
{
	UT_DEBUGMSG(("DOM: modify lang\n"));

	XAP_Dialog_Id id = XAP_DIALOG_ID_LANGUAGE;

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *) getFrame()->getDialogFactory();

	XAP_Dialog_Language * pDialog
		= (XAP_Dialog_Language *)(pDialogFactory->requestDialog(id));
	UT_return_if_fail (pDialog);

	PP_PropertyVector props_in;
	if (getView()->getCharFormat(props_in))
	{
		pDialog->setLanguageProperty(PP_getAttribute("lang", props_in).c_str());
	}

	pDialog->runModal(getFrame());

	bool bOK = (pDialog->getAnswer() == XAP_Dialog_Language::a_OK);

	if (bOK)
	{
		const gchar * s;

		pDialog->getChangedLangProperty(&s);
		PP_addOrSetAttribute("lang", s, m_vecAllProps);
	}

	pDialogFactory->releaseDialog(pDialog);
}

/*!
 * This method runs the tabs dialog to allow the user to edit the character
 * properties of their style.
 */
void AP_Dialog_Styles::ModifyFont(void)
{
//
// Fire up the Font Chooser dialog. Code stolen straight from ap_EditMethods.
//

	XAP_Dialog_Id id = XAP_DIALOG_ID_FONT;

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *) getFrame()->getDialogFactory();

	XAP_Dialog_FontChooser * pDialog
		= (XAP_Dialog_FontChooser *)(pDialogFactory->requestDialog(id));
	UT_return_if_fail (pDialog);

	// stuff the GR_Graphics into the dialog so that it
	// can query the system for font info relative to our
	// context.

	pDialog->setGraphicsContext(getView()->getLayout()->getGraphics());


	// stuff font properties into the dialog.
	// for a/p which are constant across the selection (always
	// present) we will set the field in the dialog.  for things
	// which change across the selection, we ask the dialog not
	// to set the field (by passing "").

	const std::string & sFontFamily = PP_getAttribute("font-family", m_vecAllProps);
	const std::string & sFontSize = PP_getAttribute("font-size", m_vecAllProps);
	const std::string & sFontWeight = PP_getAttribute("font-weight", m_vecAllProps);
	const std::string & sFontStyle = PP_getAttribute("font-style", m_vecAllProps);
	const std::string & sColor = PP_getAttribute("color", m_vecAllProps);
	const std::string & sBGColor = PP_getAttribute("bgcolor", m_vecAllProps);

	pDialog->setFontFamily(sFontFamily);
	pDialog->setFontSize(sFontSize);
	pDialog->setFontWeight(sFontWeight);
	pDialog->setFontStyle(sFontStyle);
	pDialog->setColor(sColor);
	pDialog->setBGColor(sBGColor);
//
// Set the background color for the preview
//
	static gchar  background[8];
	const UT_RGBColor * bgCol = getView()->getCurrentPage()->getFillType().getColor();
	sprintf(background, "%02x%02x%02x",bgCol->m_red,
			bgCol->m_grn,bgCol->m_blu);
	pDialog->setBackGroundColor( (const gchar *) background);

	// these behave a little differently since they are
	// probably just check boxes and we don't have to
	// worry about initializing a combo box with a choice
	// (and because they are all stuck under one CSS attribute).

	bool bUnderline2 = false;
	bool bOverline2 = false;
	bool bStrikeOut2 = false;
	bool bTopline2 = false;
	bool bBottomline2 = false;
	const std::string & sDecoration = PP_getAttribute("text-decoration", m_vecAllProps);
	if (!sDecoration.empty())
	{
		bUnderline2 = (strstr(sDecoration.c_str(), "underline") != NULL);
		bOverline2 = (strstr(sDecoration.c_str(), "overline") != NULL);
		bStrikeOut2 = (strstr(sDecoration.c_str(), "line-through") != NULL);
		bTopline2 = (strstr(sDecoration.c_str(), "topline") != NULL);
		bBottomline2 = (strstr(sDecoration.c_str(), "bottomline") != NULL);
	}
	pDialog->setFontDecoration(bUnderline2,bOverline2,bStrikeOut2,bTopline2,bBottomline2);
/*
    bool bDirection;
	s = UT_getAttribute("dir", props_in);
	if (s)
	{
	     bDirection = (strstr(s, "rtl") != NULL);
	}
	pDialog->setDirection(bDirection);
*/

	// run the dialog

	pDialog->runModal(getFrame());

	// extract what they did

	bool bOK = (pDialog->getAnswer() == XAP_Dialog_FontChooser::a_OK);

	if (bOK)
	{
		std::string s1;

		if (pDialog->getChangedFontFamily(s1))
		{
			PP_addOrSetAttribute("font-family", s1, m_vecAllProps);
		}

		if (pDialog->getChangedFontSize(s1))
		{
			PP_addOrSetAttribute("font-size", s1, m_vecAllProps);
		}

		if (pDialog->getChangedFontWeight(s1))
		{
			PP_addOrSetAttribute("font-weight", s1, m_vecAllProps);
		}

		if (pDialog->getChangedFontStyle(s1))
		{
			PP_addOrSetAttribute("font-style", s1, m_vecAllProps);
		}

		if (pDialog->getChangedColor(s1))
		{
			PP_addOrSetAttribute("color", s1, m_vecAllProps);
		}

		if (pDialog->getChangedBGColor(s1))
		{
			PP_addOrSetAttribute("bgcolor", s1, m_vecAllProps);
		}

		bool bUnderline = false;
		bool bChangedUnderline = pDialog->getChangedUnderline(&bUnderline);
		bool bOverline = false;
		bool bChangedOverline = pDialog->getChangedOverline(&bOverline);
		bool bStrikeOut = false;
		bool bChangedStrikeOut = pDialog->getChangedStrikeOut(&bStrikeOut);
		bool bTopline = false;
		bool bChangedTopline = pDialog->getChangedTopline(&bTopline);
		bool bBottomline = false;
		bool bChangedBottomline = pDialog->getChangedBottomline(&bBottomline);
/*
		bool bDirection = false;
		bool bChangedDirection = pDialog->getChangedDirection(&bDirection);
*/

		if (bChangedUnderline || bChangedStrikeOut || bChangedOverline || bChangedTopline || bChangedBottomline)
		{
			std::string decors;
			decors.clear();
			if(bUnderline)
				decors += "underline ";
			if(bStrikeOut)
				decors += "line-through ";
			if(bOverline)
				decors += "overline ";
			if(bTopline)
				decors += "topline ";
			if(bBottomline)
				decors += "bottomline ";
			if(!bUnderline && !bStrikeOut && !bOverline && !bTopline && !bBottomline)
				decors = "none";
			PP_addOrSetAttribute("text-decoration", decors, m_vecAllProps);
		}
/*
		if(bChangedDirection)
		{
		    if (bDirection == 1)
		        s = "rtl";
		    else
		        s = "ltr";

			PP_addOrSetAttribute("dir", s, m_vecAllProps);
		}
*/
	}
	pDialogFactory->releaseDialog(pDialog);
}

/*!
 * Used for the Tabs dialog to extract info from the tabs dialog.
 */
void AP_Dialog_Styles::_tabCallback(const char *szTabStops,
									const char *szDflTabStop)
{
	if (szTabStops)
		PP_addOrSetAttribute("tabstops", szTabStops, m_vecAllProps);
	if (szDflTabStop)
		PP_addOrSetAttribute("default-tab-interval", szDflTabStop, m_vecAllProps);
}

/*!
 * Used to extract data out of the Tabs dialog.
 */
static void
s_TabSaveCallBack (AP_Dialog_Tab * /*pDlg*/, FV_View * /*pView*/,
				   const char * szTabStops, const char * szDflTabStop,
				   void * closure)
{
	UT_return_if_fail (closure);

	AP_Dialog_Styles * pStyleDlg = static_cast<AP_Dialog_Styles *>(closure);

	pStyleDlg->_tabCallback(szTabStops, szDflTabStop);
}

/*!
 * This method fires up the Tabs dialog to allow the user to edit the properties
 * associated with Tabs for their style.
 */
void AP_Dialog_Styles::ModifyTabs(void)
{

//
// Fire up the Tab dialog
//
	XAP_Dialog_Id id = AP_DIALOG_ID_TAB;

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *) getFrame()->getDialogFactory();

	AP_Dialog_Tab * pDialog
		= (AP_Dialog_Tab *)(pDialogFactory->requestDialog(id));
	UT_return_if_fail (pDialog);

	pDialog->setSaveCallback(s_TabSaveCallBack, (void *)this);

	pDialog->runModal(getFrame());

	pDialogFactory->releaseDialog(pDialog);
}

/*!
 * This method runs the Lists dialog in amodal way so the user easily edit numbering
 * properties.
 */
void AP_Dialog_Styles::ModifyLists(void)
{
	UT_DEBUGMSG(("DOM: Doing stuff in Modify Lists \n"));
	UT_GenericVector<const gchar*> vp;

//
// Fire up a Modal version of Lists dialog
//

 	XAP_DialogFactory * pDialogFactory
  		= (XAP_DialogFactory *) getFrame()->getDialogFactory();

//
// Use this method so that we can have modeless Lists dialog and this
// modal version simultaneously
//
	AP_Dialog_Lists * pDialog
		= (AP_Dialog_Lists *)(pDialogFactory->justMakeTheDialog(AP_DIALOG_ID_LISTS));

	UT_return_if_fail (pDialog);

//
// Fill input list for Lists dialog
//
	const std::string & sListStyle = PP_getAttribute("list-style", m_vecAllProps);
	const std::string & sFieldFont = PP_getAttribute("field-font", m_vecAllProps);
	const std::string & sStartValue = PP_getAttribute("start-value", m_vecAllProps);
	const std::string & sListDelim = PP_getAttribute("list-delim", m_vecAllProps);
	const std::string & sMarginLeft = PP_getAttribute("margin-left", m_vecAllProps);
	const std::string & sListDecimal = PP_getAttribute("list-decimal", m_vecAllProps);
	const std::string & sTextIndent = PP_getAttribute("text-indent", m_vecAllProps);

	if(!sListStyle.empty())
	{
		vp.addItem("list-style");
		vp.addItem(sListStyle.c_str());
	}
	if(!sFieldFont.empty())
	{
		vp.addItem("field-font");
		vp.addItem(sFieldFont.c_str());
	}
	if(!sStartValue.empty())
	{
		vp.addItem("start-value");
		vp.addItem(sStartValue.c_str());
	}
	if(!sListDelim.empty())
	{
		vp.addItem("list-delim");
		vp.addItem(sListDelim.c_str());
	}
	if(!sMarginLeft.empty())
	{
		vp.addItem("margin-left");
		vp.addItem(sMarginLeft.c_str());
	}
	// TODO: Why is field-font here twice?
	if(!sFieldFont.empty())
	{
		vp.addItem("field-font");
		vp.addItem(sFieldFont.c_str());
	}

	if(!sListDecimal.empty())
	{
		vp.addItem("list-decimal");
		vp.addItem(sListDecimal.c_str());
	}
	if(!sTextIndent.empty())
	{
		vp.addItem("text-indent");
		vp.addItem(sTextIndent.c_str());
	}
	pDialog->fillDialogFromVector(&vp);
//
// Actually run the dialog
//
	pDialog->runModal(getFrame());

	if(pDialog->getAnswer() == AP_Dialog_Lists::a_OK)
	{
//
// Extract the properties
//
		UT_DEBUGMSG(("SEVIOR: Lists data should be changed. \n"));
		const UT_Vector * vo = pDialog->getOutProps();
		if(getVecVal(vo,"list-style"))
		{
			m_ListProps[0] = getVecVal(vo,"list-style");
			UT_DEBUGMSG(("SEVIOR: list-style %s \n",m_ListProps[0].c_str()));
			PP_addOrSetAttribute("list-style", m_ListProps[0], m_vecAllProps);
		}
		if(getVecVal(vo,"start-value"))
		{
			m_ListProps[1] = getVecVal(vo,"start-value");
			UT_DEBUGMSG(("SEVIOR: start-value %s \n",m_ListProps[1].c_str()));
			PP_addOrSetAttribute("start-value", m_ListProps[1], m_vecAllProps);
		}
		if(getVecVal(vo,"list-delim"))
		{
			m_ListProps[2] = getVecVal(vo,"list-delim");
			UT_DEBUGMSG(("SEVIOR: list-delim %s \n",m_ListProps[2].c_str()));
			PP_addOrSetAttribute("list-delim", m_ListProps[2], m_vecAllProps);
		}
		if(getVecVal(vo,"margin-left"))
		{
			m_ListProps[3] = getVecVal(vo,"margin-left");
			PP_addOrSetAttribute("margin-left", m_ListProps[3], m_vecAllProps);
		}
		if(getVecVal(vo,"field-font"))
		{
			m_ListProps[4] = getVecVal(vo,"field-font");
			PP_addOrSetAttribute("field-font", m_ListProps[4], m_vecAllProps);
		}
		if(getVecVal(vo,"list-decimal"))
		{
			m_ListProps[5] = getVecVal(vo,"list-decimal");
			PP_addOrSetAttribute("list-decimal", m_ListProps[5], m_vecAllProps);
		}
		if(getVecVal(vo,"text-indent"))
		{
			m_ListProps[6] = getVecVal(vo,"text-indent");
			PP_addOrSetAttribute("text-indent", m_ListProps[6], m_vecAllProps);
		}
		// TODO: Why is field-font here twice?
		if(getVecVal(vo,"field-font"))
		{
			m_ListProps[7] = getVecVal(vo,"field-font");
			PP_addOrSetAttribute("field-font", m_ListProps[7], m_vecAllProps);
		}
//
// Whew we're done!
//
	}
	delete pDialog;
}

/*!
 * Fire up the Paragraph dialog so we can modify the paragraph properties
 */
void AP_Dialog_Styles::ModifyParagraph(void)
{
	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(getFrame()->getDialogFactory());

	AP_Dialog_Paragraph * pDialog
		= (AP_Dialog_Paragraph *)(pDialogFactory->requestDialog(AP_DIALOG_ID_PARAGRAPH));
	UT_return_if_fail (pDialog);

	const static gchar * paraFields[] = {"text-align", "text-indent", "margin-left", "margin-right", "margin-top", "margin-bottom", "line-height","tabstops","start-value","list-delim", "list-decimal","list-style","field-font","field-color", "keep-together","keep-with-next","orphans","widows","dom-dir"};

    const size_t NUM_PARAPROPS = sizeof(paraFields)/sizeof(paraFields[0]);
//
// Count the number paragraph properties
//
//#define NUM_PARAPROPS  14

	if (m_vecAllProps.empty()) {
		return;
	}
	if (!pDialog->setDialogData(m_vecAllProps)) {
		return;
	}

	// let's steal the width from getTopRulerInfo.
	AP_TopRulerInfo info;
	getView()->getTopRulerInfo(&info);

	pDialog->setMaxWidth (info.u.c.m_xColumnWidth);

	// run the dialog
	pDialog->runModal(getFrame());

	// get the dialog answer
	AP_Dialog_Paragraph::tAnswer answer = pDialog->getAnswer();

	if(answer == AP_Dialog_Paragraph::a_OK)
	{
		PP_PropertyVector props;
		pDialog->getDialogData(props);

		// set properties into the vector. We have to save these as static char
        // strings so they persist past this method.
		// XXX fixme performance
		if (!props.empty())
		{
			for(UT_uint32 i = 0; i < NUM_PARAPROPS; i++)
			{
				const std::string & value = PP_getAttribute(paraFields[i], props);
				if(!value.empty())
				{
					PP_addOrSetAttribute(paraFields[i], value.c_str(), m_vecAllProps);
				}
			}
		}
	}
	pDialogFactory->releaseDialog(pDialog);
}

/*!
 * Extract all the props from the vector and apply them to the abi preview. We use
 * the style "tmp" to display the current style in the preview.
 */
void AP_Dialog_Styles::updateCurrentStyle(void)
{
	if(m_pAbiPreview == NULL)
		return;
	if(m_vecAllProps.empty()) {
		return;
	}
	PD_Style * pStyle = NULL;
	getLDoc()->getStyle("tmp", &pStyle);
//
// clear out old description
//
	m_curStyleDesc = PP_makePropString(m_vecAllProps);
	xxx_UT_DEBUGMSG(("New props of style %s \n",m_curStyleDesc.c_str()));
//
// Update the description in the Modify Dialog.
//
	setModifyDescription (m_curStyleDesc.c_str());
	const std::string & szBasedon = PP_getAttribute("basedon", m_vecAllAttribs);
	std::string fullProps;
	PD_Style * pBasedon = NULL;
	if (!szBasedon.empty() && m_pDoc->getStyle(szBasedon.c_str(), &pBasedon))
	{
		PP_PropertyVector vecProps;
		pBasedon->getAllProperties(vecProps, 0);
		for (PP_PropertyVector::size_type i = 0; i < vecProps.size(); i += 2)	{
			UT_std_string_setProperty(fullProps, vecProps[i], vecProps[i + 1]);
		}
	}
//
// Overwrite any basedon props with the current setting of this style
//
	UT_std_string_addPropertyString(fullProps, m_curStyleDesc);

	if( pStyle == NULL)
	{
		const PP_PropertyVector attrib = {
			PT_NAME_ATTRIBUTE_NAME, "tmp",
			PT_TYPE_ATTRIBUTE_NAME, "P",
			"basedon", PP_getAttribute("basedon", m_vecAllAttribs),
			"followedby", PP_getAttribute("followedby", m_vecAllAttribs),
			"props",fullProps
		};
		getLDoc()->appendStyle(attrib);
	}
	else
	{
		const PP_PropertyVector atts = {
			"props", fullProps.c_str()
		};
		getLDoc()->addStyleAttributes("tmp", atts);
		getLDoc()->updateDocForStyleChange("tmp",true);
	}
	getLView()->setPoint(m_posFocus+1);
	getLView()->setStyle("tmp");
	invalidatePreview();
}


/*!
 * Take the current style description and use it to define a new style
 * in the main document.
 */
bool AP_Dialog_Styles::createNewStyle(const gchar * szName)
{
	UT_DEBUGMSG(("DOM: new style %s\n", szName));

	if(m_vecAllProps.empty()) {
		return false;
	}

	m_curStyleDesc = PP_makePropString(m_vecAllProps);
//
// Update the description in the Main Dialog.
//
	setDescription (m_curStyleDesc.c_str());
//
// Append the new style to the main document
//
	PD_Style * pStyle = NULL;

	UT_return_val_if_fail (szName, false);

	getDoc()->getStyle("szName", &pStyle);
	if(pStyle != NULL)
		return false;
//
// Assemble the attributes we need for this new style
//
	const PP_PropertyVector attrib = {
		PT_NAME_ATTRIBUTE_NAME, szName,
		PT_TYPE_ATTRIBUTE_NAME, PP_getAttribute(PT_TYPE_ATTRIBUTE_NAME, m_vecAllAttribs),
		"basedon", PP_getAttribute("basedon", m_vecAllAttribs),
		"followedby", PP_getAttribute("followedby", m_vecAllAttribs),
		"props", m_curStyleDesc
	};
	return getDoc()->appendStyle(attrib);
}


/*!
 * Take the current style description and modify the description of the style
 * in the main document.
 */
bool AP_Dialog_Styles::applyModifiedStyleToDoc(void)
{
	if(m_vecAllProps.empty()) {
		return false;
	}

	PP_PropertyVector attribs = m_vecAllAttribs;
	m_curStyleDesc = PP_makePropString(m_vecAllProps);
	attribs.push_back("props");
	attribs.push_back(m_curStyleDesc);
//
// Update the description in the Main Dialog.
//
	setDescription (m_curStyleDesc.c_str());
//
// Set the style in the main document
//
	const gchar * szStyle = getCurrentStyle();
	if(szStyle == NULL)
		return false;
//
// This creates a new indexAP from the attributes/properties here.
// This allows properties to be removed from a pre-existing style
//
	return getDoc()->setAllStyleAttributes(szStyle, attribs);
}

/*!
 * Pointer to the current FV_View of the document we're working with.
 */
void AP_Dialog_Styles::setView( FV_View * pView)
{
	m_pView = pView;
}

/*!
 * Pointer to the frame of the real document we're working with.
 */
void AP_Dialog_Styles::setFrame( XAP_Frame * pFrame)
{
	m_pFrame = pFrame;
}

/*!
 * Pointer to current the PD_Document for real document we're working with.
 */
void AP_Dialog_Styles::setDoc( PD_Document * pDoc)
{
	m_pDoc = pDoc;
}


/*!
 * Pointer to the current FV_View of the document we're working with.
 */
FV_View * AP_Dialog_Styles::getView(void) const
{
	return m_pView;
}
/*!
 * Pointer to the frame of the real document we're working with.
 */
XAP_Frame * AP_Dialog_Styles::getFrame(void) const
{
	return m_pFrame;
}

/*!
 * Pointer to current the PD_Document for real document we're working with.
 */
PD_Document * AP_Dialog_Styles::getDoc(void) const
{
	return m_pDoc;
}

/*!
 * Create the preview for the Paragraph preview for styles.
 */
void AP_Dialog_Styles::_createParaPreviewFromGC(GR_Graphics * gc,
                                                UT_uint32 width,
						UT_uint32 height)
{
	UT_return_if_fail (gc);

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	UT_return_if_fail (pSS);

	UT_UCS4String str(pSS->getValue(AP_STRING_ID_DLG_Styles_LBL_TxtMsg));

	m_pParaPreview = new AP_Preview_Paragraph(gc, str.ucs4_str(), static_cast<XAP_Dialog*>(this));
	UT_return_if_fail (m_pParaPreview);

	m_pParaPreview->setWindowSize(width, height);
}


void AP_Dialog_Styles::_createCharPreviewFromGC(GR_Graphics * gc,
                                                UT_uint32 width,
						UT_uint32 height)
{
	UT_return_if_fail (gc);

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	UT_return_if_fail (pSS);

//
// Set the Background color for the preview.
//
	static gchar  background[8];
	const UT_RGBColor * bgCol = getView()->getCurrentPage()->getFillType().getColor();
	sprintf(background, "%02x%02x%02x",bgCol->m_red,bgCol->m_grn,bgCol->m_blu);

	m_pCharPreview = new XAP_Preview_FontPreview(gc,background);
	UT_return_if_fail (m_pCharPreview);

	m_pCharPreview->setWindowSize(width, height);
//
// Text for the Preview
//
	static UT_UCSChar szString[60];
	UT_UCS4_strcpy_utf8_char( (UT_UCSChar *) szString, pSS->getValue(AP_STRING_ID_DLG_Styles_LBL_TxtMsg));
	m_pCharPreview->setDrawString((const UT_UCSChar *) szString);
//
// set our Vector of Character Properties into the preview class.
//
	m_pCharPreview->setVecProperties( &m_mapCharProps);
}

/*!
 * This is the preview for the second dialog pane. It puts a complete mini-abiword
 * in a graphics context so all the results of any style change is immediately
 * obvious.
 */
void AP_Dialog_Styles::_createAbiPreviewFromGC(GR_Graphics * gc,
                                                UT_uint32 width,
											   UT_uint32 height )
{
	UT_return_if_fail (gc);
	if(m_pAbiPreview)
		DELETEP(m_pAbiPreview);
	m_pAbiPreview = new AP_Preview_Abi(gc,width,height,getFrame(),PREVIEW_ZOOMED);
	UT_return_if_fail (m_pAbiPreview);
}

/*!
 * This is the FV_View pointer for our mini-abi in the second pane preview.
 */
FV_View * AP_Dialog_Styles::getLView(void) const
{
	return m_pAbiPreview->getView();
}

/*!
 * This is the pd_Document pointer for our mini-Abi in the second pane preview.
 */
PD_Document * AP_Dialog_Styles::getLDoc(void) const
{
	return m_pAbiPreview->getDoc();
}

/*!
 * This updates the mini-Abi in the second pane preview.
 */
void  AP_Dialog_Styles::invalidatePreview(void)
{
    if (m_pAbiPreview) {
        m_pAbiPreview->queueDraw();
    }
}

/*!
 * This puts some tet in the second pane preview so we can see the effect of the
 * Styles.
 */
void AP_Dialog_Styles::_populateAbiPreview(bool isNew)
{
//
// Text for the Preview
//

	static UT_UCSChar szString[60];
	static UT_UCSChar sz1[4];
	static UT_UCSChar sz2[4];
	static UT_UCSChar sz3[4];
	static UT_UCSChar szSpace[4];
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	UT_UCS4_strcpy_utf8_char( (UT_UCSChar *) szString, pSS->getValue(AP_STRING_ID_DLG_Styles_LBL_TxtMsg));
	UT_UCS4_strcpy_char( (UT_UCSChar *) sz1, " 1");
	UT_UCS4_strcpy_char( (UT_UCSChar *) sz2, " 2");
	UT_UCS4_strcpy_char( (UT_UCSChar *) sz3, " 3");
	UT_UCS4_strcpy_char( (UT_UCSChar *) szSpace, "  ");
	UT_uint32 len =UT_UCS4_strlen(szString);
	UT_uint32 len1 =UT_UCS4_strlen(sz1);
	UT_uint32 lenSpace =UT_UCS4_strlen(szSpace);
//
// Set all the margins to 0
//
	const PP_PropertyVector props =  {
		"page-margin-left","0.0in",
		"page-margin-right","0.0in",
		"page-margin-top","0.0in",
		"page-margin-bottom","0.0in",
		"page-margin-footer","0.0in",
		"page-margin-header","0.0in"
	};
	getLView()->setSectionFormat(props);
//
// First Paragraph
//
	m_posBefore = getLView()->getPoint();
	UT_uint32 i=0;
	for(i=0;i<15;i++)
	{
		getLView()->cmdCharInsert((UT_UCSChar *) szString,len);
		getLView()->cmdCharInsert((UT_UCSChar *) szSpace,lenSpace);
	}
	getLView()->cmdCharInsert((UT_UCSChar *) sz1,len1);

	static std::string Grey;
	static std::string szFGColor;
	UT_RGBColor FGColor(0,0,0);
	UT_RGBColor BGColor(255,255,255);
	const UT_RGBColor * pageCol = NULL;
	getLView()->setStyle("Normal");

	PP_PropertyVector props_in;
	getLView()->getCharFormat(props_in);
	const std::string & pszFGColor = PP_getAttribute("color", props_in);
	const std::string & pszBGColor = PP_getAttribute("bgcolor", props_in);
	if(!pszFGColor.empty()) {
		UT_parseColor(pszFGColor.c_str(),FGColor);
	}
//
// Save the Foreground color for later
//
	szFGColor = UT_std_string_sprintf("%02x%02x%02x",FGColor.m_red,
									  FGColor.m_grn, FGColor.m_blu);
	if(pszBGColor.empty())
	{
		pageCol = getLView()->getCurrentPage()->getFillType().getColor();
		Grey = UT_std_string_sprintf("%02x%02x%02x",(pageCol->m_red+FGColor.m_red)/2,
				(pageCol->m_grn+FGColor.m_grn)/2, (pageCol->m_blu+FGColor.m_blu)/2);
	}
	else if(pszBGColor == "transparent")
	{
		pageCol = getLView()->getCurrentPage()->getFillType().getColor();
		Grey = UT_std_string_sprintf("%02x%02x%02x",(pageCol->m_red+FGColor.m_red)/2,
				(pageCol->m_grn+FGColor.m_grn)/2, (pageCol->m_blu+FGColor.m_blu)/2);
	}
	else
	{
		UT_parseColor(pszBGColor.c_str(), BGColor);
		Grey = UT_std_string_sprintf("%02x%02x%02x",(BGColor.m_red+FGColor.m_red)/2,
				(BGColor.m_grn + FGColor.m_grn)/2, (BGColor.m_blu+FGColor.m_blu)/2);
	}
//
// Set the "Greyed" color for text in previous block
//
	const PP_PropertyVector GreyCol = {
		"color", Grey
	};
	getLDoc()->changeSpanFmt(PTC_AddFmt, m_posBefore,getLView()->getPoint(), PP_NOPROPS, GreyCol);

	getLView()->insertParagraphBreak();
//
// Second Paragraph in focus. Our Vectors containing the current settings have
// been filled from calls in the platform layer.
//
	PD_Style * pStyle = NULL;
	getLDoc()->getStyle("tmp", &pStyle);

	m_curStyleDesc = PP_makePropString(m_vecAllProps);
//
// Update the description in the Modify Dialog.
//
	setModifyDescription (m_curStyleDesc.c_str());
	xxx_UT_DEBUGMSG(("Style desc is %s \n",m_curStyleDesc.c_str()));
	if( pStyle == NULL)
	{
		if(m_curStyleDesc.empty()) {
			m_curStyleDesc = "font-style:normal";
		}
		PP_PropertyVector attrib = {
			PT_NAME_ATTRIBUTE_NAME, "tmp",
			PT_TYPE_ATTRIBUTE_NAME, "P",
			"basedon", "None",
			"followedby", "Current Settings",
			"props", m_curStyleDesc,
		};
		if(!isNew)
		{
			attrib[3] = PP_getAttribute(PT_TYPE_ATTRIBUTE_NAME, m_vecAllAttribs);
			attrib[5] = PP_getAttribute("basedon", m_vecAllAttribs);
			attrib[7] = PP_getAttribute("followedby", m_vecAllAttribs);
		}
		getLDoc()->appendStyle(attrib);
	}
	else
	{
		getLDoc()->addStyleProperties("tmp", m_vecAllProps);
		getLDoc()->addStyleAttributes("tmp", m_vecAllAttribs);
	}

	getLView()->setStyle("tmp");
	m_posFocus = getLView()->getPoint();
//
// Set Color Back
//
	const std::string & pszFGColorL = PP_getAttribute("color", m_vecAllAttribs);
	if(pszFGColorL.empty())
	{
		const PP_PropertyVector FGCol = {
			"color", szFGColor
		};
		getLView()->setCharFormat(FGCol);
	}

	for(i=0; i<8; i++)
	{
		getLView()->cmdCharInsert((UT_UCSChar *) szString,len);
		getLView()->cmdCharInsert((UT_UCSChar *) szSpace,lenSpace);
	}
	getLView()->cmdCharInsert((UT_UCSChar *) sz2,len1);
//
// Third Paragraph
//
	getLView()->insertParagraphBreak();
	m_posAfter = getLView()->getPoint();
	getLView()->setCharFormat(GreyCol);
	for(i=0; i<15; i++)
	{
		getLView()->cmdCharInsert((UT_UCSChar *) szString,len);
		getLView()->cmdCharInsert((UT_UCSChar *) szSpace,lenSpace);
	}
	getLView()->cmdCharInsert((UT_UCSChar *) sz3,len1);
}

/*!
 * remove the mini-Abi preview in the second pane.
 */
void AP_Dialog_Styles::destroyAbiPreview(void)
{
	DELETEP(m_pAbiPreview);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void AP_Dialog_Styles::event_paraPreviewUpdated (const gchar * pageLeftMargin,
						 const gchar * pageRightMargin,
						 const gchar * align,
						 const gchar * firstLineIndent,
						 const gchar * leftIndent,
						 const gchar * rightIndent,
						 const gchar * beforeSpacing,
						 const gchar * afterSpacing,
						 const gchar * lineSpacing) const
{
	AP_Dialog_Paragraph::tAlignState tAlign = AP_Dialog_Paragraph::align_LEFT;
	AP_Dialog_Paragraph::tIndentState tIndent = AP_Dialog_Paragraph::indent_NONE;
	AP_Dialog_Paragraph::tSpacingState tSpacing = AP_Dialog_Paragraph::spacing_MULTIPLE;

	const char * sz = NULL;
	const char * pPlusFound = NULL;

	UT_return_if_fail (m_pParaPreview);

	if (align) {
		if (!strcmp(align, "right"))
			tAlign = AP_Dialog_Paragraph::align_RIGHT;
		else if (!strcmp(align, "center"))
			tAlign = AP_Dialog_Paragraph::align_CENTERED;
		else if (!strcmp(align, "justify"))
			tAlign = AP_Dialog_Paragraph::align_JUSTIFIED;

	}

	if (firstLineIndent) {

		sz = (const char *)firstLineIndent;

		if (UT_convertDimensionless(sz) > (double) 0)
		{
			tIndent = AP_Dialog_Paragraph::indent_FIRSTLINE;
		}
		else if (UT_convertDimensionless(sz) < (double) 0)
		{
			tIndent = AP_Dialog_Paragraph::indent_HANGING;
		}

	}
	std::string sLineSpacing;
	if (lineSpacing) {

		sLineSpacing = lineSpacing;
		sz = (const char *)lineSpacing;

		pPlusFound = strrchr(sz, '+');
		if (pPlusFound && *(pPlusFound + 1) == 0) {
			tSpacing = AP_Dialog_Paragraph::spacing_ATLEAST;
			// remove the '+'
			sLineSpacing.resize(sLineSpacing.size() - 1);
		} else if(UT_hasDimensionComponent(sz)) {
			tSpacing = AP_Dialog_Paragraph::spacing_EXACTLY;
		} else if(!strcmp("1.0", sz)) {
			tSpacing = AP_Dialog_Paragraph::spacing_SINGLE;
		} else if(!strcmp("1.5", sz)) {
			tSpacing = AP_Dialog_Paragraph::spacing_ONEANDHALF;
		} else if(!strcmp("2.0", sz)) {
			tSpacing = AP_Dialog_Paragraph::spacing_DOUBLE;
		}
	}

	m_pParaPreview->setFormat (pageLeftMargin,
							   pageRightMargin,
							   tAlign,
							   firstLineIndent,
							   tIndent,
							   leftIndent,
							   rightIndent,
							   beforeSpacing,
							   afterSpacing,
							   sLineSpacing.c_str(),
							   tSpacing);

	// force a redraw
	m_pParaPreview->queueDraw();
}

/*!
 * Update the character preview in the front pane.
 */
void AP_Dialog_Styles::event_charPreviewUpdated (void) const
{
	UT_return_if_fail (m_pCharPreview); // add this when we make a char preview

	// force a redraw
	m_pCharPreview->setVecProperties( &m_mapCharProps);
	m_pCharPreview->queueDraw();
}

/*!
 * Fill both the paragraph and character previews in the front pane with some
 * Text so we can see the text of the chosen style.
 */
void AP_Dialog_Styles::_populatePreviews(bool isModify)
{
	PD_Style * pStyle = NULL;
	const char * szStyle = NULL;
	const static gchar * paraFields[] = {"text-align", "text-indent", "margin-left", "margin-right", "margin-top", "margin-bottom", "line-height","tabstops","start-value","list-delim", "list-style","field-font","list-decimal","field-color", "keep-together","keep-with-next","orphans","widows","dom-dir"};
	const size_t nParaFlds = sizeof(paraFields)/sizeof(paraFields[0]);
	const gchar * paraValues [nParaFlds];

	const static gchar * charFields[] =
	{"bgcolor","color","font-family","font-size","font-stretch","font-style",
	 "font-variant", "font-weight","text-decoration","lang"};

	const size_t nCharFlds = sizeof(charFields)/sizeof(charFields[0]);

	szStyle = getCurrentStyle();

	if (!szStyle) // having nothing displayed is totally valid
	{
		return;
	}
//
// Load up our properties vector
//
	fillVecWithProps(szStyle,true);

	// update the previews and the description label
	if (getDoc()->getStyle (szStyle, &pStyle))
	{
		UT_uint32 i;
//
// Clear any previous stuff from our style description. This description must
// persist beyond this method because we want to modify it.
//
		m_curStyleDesc.clear();

	    // first loop through and pass out each property:value combination for paragraphs
		for(i = 0; i < nParaFlds; i++)
		{
			const gchar * szName = paraFields[i];
			const gchar * szValue = NULL;
			pStyle->getProperty(szName,szValue);
			if (szValue == NULL)
			{
				pStyle->getPropertyExpand(szName,szValue);
				if (szValue == NULL)
				{
					paraValues[i] = nullptr;
					continue;
				}
				else
				{
					paraValues[i] = szValue;
				}
			}
			else
			{
				paraValues[i] = szValue;
				m_curStyleDesc += (const char *)szName;
				m_curStyleDesc += ":";

				if (szValue && *szValue)
				    m_curStyleDesc += (const char *)szValue;
				m_curStyleDesc += "; ";
			}
		}

// Clear out old contents of the char vector if they exist
		m_mapCharProps.clear();

	    // now loop through and pass out each property:value combination for characters

		for(i = 0; i < nCharFlds; i++)
		{
			const gchar * szName = charFields[i];
			const gchar * szValue = NULL;
			pStyle->getProperty(szName,szValue);
			if (szValue == NULL)
			{
				pStyle->getPropertyExpand(szName,szValue);
				if (szValue == NULL)
				{
					continue;
				}
			}
			else
			{
				m_curStyleDesc += (const char *)szName;
				m_curStyleDesc += ":";
				if(szValue && *szValue)
				    m_curStyleDesc += (const char *)szValue;
				m_curStyleDesc += "; ";
			}
//
// Put them in our property vector for the Character preview
//
			m_mapCharProps[szName] = szValue;
		}

		if (!m_curStyleDesc.empty())
		{
			if(!isModify)
			{
				setDescription (m_curStyleDesc.c_str());
			}
			else
			{
				setModifyDescription (m_curStyleDesc.c_str());
			}
			// these aren't set at a style level, but we need to put them in there anyway
			PP_PropertyVector props_in;
			getView()->getSectionFormat(props_in);

			if(!isModify)
				event_paraPreviewUpdated(
					PP_getAttribute("page-margin-left", props_in).c_str(),
					PP_getAttribute("page-margin-right", props_in).c_str(),
					(const gchar *)paraValues[0], (const gchar *)paraValues[1],
					(const gchar *)paraValues[2], (const gchar *)paraValues[3],
					(const gchar *)paraValues[4], (const gchar *)paraValues[5],
					(const gchar *)paraValues[6]);
			if(!isModify)
				event_charPreviewUpdated();
		}
	}
}
