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
#include <stdio.h>
#include <string.h>
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
#include "ut_string_class.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"

#include "xap_Dlg_Language.h"

#include "ap_Dialog_Tab.h"

AP_Dialog_Styles::AP_Dialog_Styles(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id)
{

	m_answer = a_OK;
	m_pParaPreview = NULL;
	m_pCharPreview = NULL;
	m_pAbiPreview = NULL;
	if(m_vecCharProps.getItemCount() > 0)
		m_vecCharProps.clear();
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
 * This method adds the key,value pair pszProp,pszVal to the Vector of 
 * all properties of the current style.
 * If the Property already exists it's value is replaced with pszVal
\param const XML_Char * pszProp the property name
\param const XML_Char * pszVal the value of this property.
*/
void AP_Dialog_Styles::addOrReplaceVecProp(const XML_Char * pszProp, 
												 const XML_Char * pszVal)
{
	UT_sint32 iCount = m_vecAllProps.getItemCount();
	const char * pszV = NULL;
	if(iCount <= 0)
	{
		m_vecAllProps.addItem((void *) pszProp);
		m_vecAllProps.addItem((void *) pszVal);
		return;
	}
	UT_sint32 i = 0;
	for(i=0; i < iCount ; i += 2)
	{
		pszV = (const XML_Char *) m_vecAllProps.getNthItem(i);
		if( (pszV != NULL) && (strcmp( pszV,pszProp) == 0))
			break;
	}
	if(i < iCount)
		m_vecAllProps.setNthItem(i+1, (void *) pszVal, NULL);
	else
	{
		m_vecAllProps.addItem((void *) pszProp);
		m_vecAllProps.addItem((void *) pszVal);
	}
	return;
}


/*!
 * This method removes the key,value pair  (pszProp,pszVal) given by pszProp 
 * from the Vector of all properties of the current style.
 * If the Property does not exists nothing happens
\param const XML_Char * pszProp the property name
*/
void AP_Dialog_Styles::removeVecProp(const XML_Char * pszProp)
{
	UT_sint32 iCount = m_vecAllProps.getItemCount();
	const char * pszV = NULL;
	if(iCount <= 0)
	{
		return;
	}
	UT_sint32 i = 0;
	for(i=0; i < iCount ; i += 2)
	{
		pszV = (const XML_Char *) m_vecAllProps.getNthItem(i);
		if( (pszV != NULL) && (strcmp( pszV,pszProp) == 0))
			break;
	}
	if(i < iCount)
	{
		m_vecAllProps.deleteNthItem(i+1);
		m_vecAllProps.deleteNthItem(i);
	}
	return;
}

/*!
 * This method adds the key,value pair pszProp,pszVal to the Vector of 
 * all attributes of the current style.
 * If the Property already exists it's value is replaced with pszVal
\param const XML_Char * pszProp the property name
\param const XML_Char * pszVal the value of this property.
*/
void AP_Dialog_Styles::addOrReplaceVecAttribs(const XML_Char * pszProp, 
												 const XML_Char * pszVal)
{
	UT_sint32 iCount = m_vecAllAttribs.getItemCount();
	const char * pszV = NULL;
	if(iCount <= 0)
	{
		m_vecAllAttribs.addItem((void *) pszProp);
		m_vecAllAttribs.addItem((void *) pszVal);
		return;
	}
	UT_sint32 i = 0;
	for(i=0; i < iCount ; i += 2)
	{
		pszV = (const XML_Char *) m_vecAllAttribs.getNthItem(i);
		if( (pszV != NULL) && (strcmp( pszV,pszProp) == 0))
			break;
	}
	if(i < iCount)
		m_vecAllAttribs.setNthItem(i+1, (void *) pszVal, NULL);
	else
	{
		m_vecAllAttribs.addItem((void *) pszProp);
		m_vecAllAttribs.addItem((void *) pszVal);
	}
	return;
}

void AP_Dialog_Styles::fillVecFromCurrentPoint(void)
{
	const XML_Char ** paraProps = NULL;
	getView()->getBlockFormat(&paraProps,false);
//
// This line expands all styles
//	getView()->getBlockFormat(&paraProps,true);
	const XML_Char ** charProps = NULL;
	getView()->getCharFormat(&charProps,false);
//
// This line expans all styles..
//
//	getView()->getCharFormat(&charProps,true);
	UT_sint32 i =0;
//
// Loop over all properties and add them to our vector
//
	m_vecAllProps.clear();
	const XML_Char * szName = NULL;
	const XML_Char * szValue = NULL;
	while(paraProps[i] != NULL)
	{
		szName = paraProps[i];
		szValue = paraProps[i+1];
		addOrReplaceVecProp(szName,szValue);
		i = i + 2;
	}
	i = 0;
	while(charProps[i] != NULL)
	{
		szName = charProps[i];
		szValue = charProps[i+1];
		addOrReplaceVecProp(szName,szValue);
		i = i + 2;
	}
}

void AP_Dialog_Styles::fillVecWithProps(const XML_Char * szStyle, bool bReplaceAttributes = true)
{
	PD_Style * pStyle = NULL;
	m_vecAllProps.clear();
	if( bReplaceAttributes)
		m_vecAllAttribs.clear();
	if(szStyle == NULL || ! getDoc()->getStyle(szStyle,&pStyle))
	{
		return;
	}

	const static XML_Char * paraFields[] = {"text-align", "text-indent", "margin-left", "margin-right", "margin-top", "margin-bottom", "line-height","tabstops","start-value","list-delim", "list-decimal","field-font","field-color"};

	const size_t nParaFlds = sizeof(paraFields)/sizeof(paraFields[0]);

	const static XML_Char * charFields[] = 
	{"bgcolor","color","font-family","font-size","font-stretch","font-style", 
	 "font-variant", "font-weight","text-decoration","lang"};

	const size_t nCharFlds = sizeof(charFields)/sizeof(charFields[0]);

	const static XML_Char * attribs[] = 
	{"followedby","basedon","listid","parentid","level","style","type"};

	const size_t nattribs = sizeof(attribs)/sizeof(attribs[0]);
	UT_uint32 i;
//
// Loop through all Paragraph properties and add those with non-null values
//
	for(i = 0; i < nParaFlds; i++)
	{
		const XML_Char * szName = paraFields[i];
		const XML_Char * szValue = NULL;		
		pStyle->getProperty(szName,szValue);
		if(szValue)
			addOrReplaceVecProp(szName, szValue);
	}
//
// Loop through all Character properties and add those with non-null values
//
	for(i = 0; i < nCharFlds; i++)
	{
		const XML_Char * szName = charFields[i];
		const XML_Char * szValue = NULL;		
		pStyle->getProperty(szName,szValue);
		if(szValue)
			addOrReplaceVecProp(szName, szValue);
	}
//
// Loop through all the attributes and add those with non-null values
//
	if(bReplaceAttributes)
	{
		for(i = 0; i < nattribs; i++)
		{
			const XML_Char * szName = attribs[i];
			const XML_Char * szValue = NULL;		
			pStyle->getAttribute(szName,szValue);
			if(szValue)
				addOrReplaceVecAttribs(szName, szValue);
		}
	}
}


/*!
 * This method returns a pointer to the const char * value associated with the
 * the property szProp. Stolen from ap_Dialog_Lists and ap_Dialog_Styles.
 * It assumes properties and values are stored the array like this:
 * vecProp(n)   :   vecProp(n+1)
 * "property"   :   "value"
 */
const XML_Char * AP_Dialog_Styles::getPropsVal(const XML_Char * szProp) const
{
	UT_sint32 i = m_vecAllProps.getItemCount();
	if(i <= 0) 
		return NULL;
	UT_sint32 j;
	const XML_Char * pszV = NULL;
	for(j= 0; j<i ;j=j+2)
	{
		pszV = (const XML_Char *) m_vecAllProps.getNthItem(j);
		if( (pszV != NULL) && (strcmp( pszV,szProp) == 0))
			break;
	}
	if( j < i )
		return  (const XML_Char *) m_vecAllProps.getNthItem(j+1);
	else
		return NULL;
}


/*!
 * This method returns a pointer to the const char * value associated with the
 * the attribute szProp. Stolen from ap_Dialog_Lists and ap_Dialog_Styles.
 * It assumes properties and values are stored the array like this:
 * vecProp(n)   :   vecProp(n+1)
 * "attribute"   :   "value"
 */
const XML_Char * AP_Dialog_Styles::getAttsVal(const XML_Char * szProp) const
{
	UT_sint32 i = m_vecAllAttribs.getItemCount();
	if(i <= 0) 
		return NULL;
	UT_sint32 j;
	const XML_Char * pszV = NULL;
	for(j= 0; j<i ;j=j+2)
	{
		pszV = (const XML_Char *) m_vecAllAttribs.getNthItem(j);
		if( (pszV != NULL) && (strcmp( pszV,szProp) == 0))
			break;
	}
	if( j < i )
		return  (const XML_Char *) m_vecAllAttribs.getNthItem(j+1);
	else
		return NULL;
}

void AP_Dialog_Styles::ModifyLang(void)
{
	UT_DEBUGMSG(("DOM: modify lang\n"));

	XAP_Dialog_Id id = XAP_DIALOG_ID_LANGUAGE;

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *) getFrame()->getDialogFactory();

	XAP_Dialog_Language * pDialog
		= (XAP_Dialog_Language *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);

	const XML_Char ** props_in = NULL;
	if (getView()->getCharFormat(&props_in))
	{
		pDialog->setLanguageProperty(UT_getAttribute("lang", props_in));
		FREEP(props_in);
	}

	pDialog->runModal(getFrame());

	bool bOK = (pDialog->getAnswer() == XAP_Dialog_Language::a_OK);

	if (bOK)
	{
		static XML_Char lang[50];
		const XML_Char * s;

		pDialog->getChangedLangProperty(&s);
		sprintf(lang,"%s",s);
		addOrReplaceVecProp("lang", lang);
	}

	pDialogFactory->releaseDialog(pDialog);
}

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
	UT_ASSERT(pDialog);

	// stuff the GR_Graphics into the dialog so that it
	// can query the system for font info relative to our
	// context.

	pDialog->setGraphicsContext(getView()->getLayout()->getGraphics());


	// stuff font properties into the dialog.
	// for a/p which are constant across the selection (always
	// present) we will set the field in the dialog.  for things
	// which change across the selection, we ask the dialog not
	// to set the field (by passing null).

	pDialog->setFontFamily(getPropsVal("font-family"));
	pDialog->setFontSize(getPropsVal("font-size"));
	pDialog->setFontWeight(getPropsVal("font-weight"));
	pDialog->setFontStyle(getPropsVal("font-style"));
	pDialog->setColor(getPropsVal("color"));
	pDialog->setBGColor(getPropsVal("bgcolor"));
//
// Set the background color for the preview
//
	static XML_Char  background[8];
	UT_RGBColor * bgCol = getView()->getCurrentPage()->getOwningSection()->getPaperColor();
	sprintf(background, "%02x%02x%02x",bgCol->m_red,
			bgCol->m_grn,bgCol->m_blu);
	pDialog->setBackGroundColor( (const XML_Char *) background);
		
	// these behave a little differently since they are
	// probably just check boxes and we don't have to
	// worry about initializing a combo box with a choice
	// (and because they are all stuck under one CSS attribute).

	bool bUnderline = false;
	bool bOverline = false;
	bool bStrikeOut = false;
	bool bTopline = false;
	bool bBottomline = false;
	const XML_Char * s = getPropsVal("text-decoration");
	if (s)
	{
		bUnderline = (strstr(s, "underline") != NULL);
		bOverline = (strstr(s, "overline") != NULL);
		bStrikeOut = (strstr(s, "line-through") != NULL);
		bTopline = (strstr(s, "topline") != NULL);
		bBottomline = (strstr(s, "bottomline") != NULL);
	}
	pDialog->setFontDecoration(bUnderline,bOverline,bStrikeOut,bTopline,bBottomline);
/*
#ifdef BIDI_ENABLED
    bool bDirection;
	s = UT_getAttribute("dir", props_in);
	if (s)
	{
	     bDirection = (strstr(s, "rtl") != NULL);
	}
	pDialog->setDirection(bDirection);
#endif
*/

	// run the dialog

	pDialog->runModal(getFrame());

	// extract what they did

	bool bOK = (pDialog->getAnswer() == XAP_Dialog_FontChooser::a_OK);

	if (bOK)
	{
		const XML_Char * s;

		if (pDialog->getChangedFontFamily(&s))
		{
			addOrReplaceVecProp("font-family", s);
		}

		if (pDialog->getChangedFontSize(&s))
		{
			addOrReplaceVecProp("font-size", s);
		}

		if (pDialog->getChangedFontWeight(&s))
		{
			addOrReplaceVecProp("font-weight", s);
		}

		if (pDialog->getChangedFontStyle(&s))
		{
			addOrReplaceVecProp("font-style", s);
		}

		if (pDialog->getChangedColor(&s))
		{
			addOrReplaceVecProp("color", s);
		}

		if (pDialog->getChangedBGColor(&s))
		{
			addOrReplaceVecProp("bgcolor", s);
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
#ifdef BIDI_ENABLED
		bool bDirection = false;
		bool bChangedDirection = pDialog->getChangedDirection(&bDirection);
#endif
*/

		if (bChangedUnderline || bChangedStrikeOut || bChangedOverline || bChangedTopline || bChangedBottomline)
		{
			UT_String decors;
			static XML_Char s[50];
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
			sprintf(s,"%s",decors.c_str());
			addOrReplaceVecProp("text-decoration", s);
		}
/*
#ifdef BIDI_ENABLED
		if(bChangedDirection)
		{
		    if (bDirection == 1)
		        s = "rtl";
		    else
		        s = "ltr";
		
			addOrReplaceVecProp("dir", s);
		}
#endif
*/
	}
	pDialogFactory->releaseDialog(pDialog);
}

void AP_Dialog_Styles::_tabCallback(const char *szTabStops,
									const char *szDflTabStop)
{
	// TODO: fix mem leaks
	if (szTabStops)
		addOrReplaceVecProp("tabstops", UT_strdup(szTabStops));
	if (szDflTabStop)
		addOrReplaceVecProp("default-tab-interval", UT_strdup(szDflTabStop));
}

static void
s_TabSaveCallBack (AP_Dialog_Tab * pDlg, FV_View * pView, 
				   const char * szTabStops, const char * szDflTabStop,
				   void * closure)
{
	UT_ASSERT(closure);

	AP_Dialog_Styles * pStyleDlg = static_cast<AP_Dialog_Styles *>(closure);

	pStyleDlg->_tabCallback(szTabStops, szDflTabStop);
}

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
	UT_ASSERT(pDialog);

	pDialog->setSaveCallback(s_TabSaveCallBack, (void *)this);

	pDialog->runModal(getFrame());

	pDialogFactory->releaseDialog(pDialog);
}

void AP_Dialog_Styles::ModifyLists(void)
{
	UT_DEBUGMSG(("DOM: Doing stuff in Modify Lists \n"));

//
// Fire up the Tab dialog
//

	XAP_Dialog_Id id = AP_DIALOG_ID_TAB;

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *) getFrame()->getDialogFactory();

	AP_Dialog_Tab * pDialog
		= (AP_Dialog_Tab *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);

#if 0
	pDialog->setSaveCallback(s_TabSaveCallBack, (void *)this);

	pDialog->runModal(getFrame());
#else
	UT_ASSERT(UT_TODO);
#endif

	pDialogFactory->releaseDialog(pDialog);
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
	UT_ASSERT(pDialog);

	const static XML_Char * paraFields[] = {"text-align", "text-indent", "margin-left", "margin-right", "margin-top", "margin-bottom", "line-height","tabstops","start-value","list-delim", "list-decimal","field-font","field-color"};

//	const size_t num_paraFlds = sizeof(paraFields)/sizeof(paraFields[0]);
//
// Count the number paragraph properties
//
#define NUM_PARAPROPS  13

	static XML_Char paraVals[NUM_PARAPROPS][60];
	const XML_Char ** props = NULL;

	UT_uint32 i = 0;
	if(m_vecAllProps.getItemCount() <= 0)
		return;
	UT_uint32 countp = m_vecAllProps.getItemCount() + 1;
	props = (const XML_Char **) calloc(countp, sizeof(XML_Char *));
	countp--;
	for(i=0; i<countp; i++)
	{
		props[i] = (const XML_Char *) m_vecAllProps.getNthItem(i);
	}
	props[i] = NULL;

	if (!pDialog->setDialogData(props))
		return;

	FREEP(props);

	// let's steal the width from getTopRulerInfo.
	AP_TopRulerInfo info;
	getView()->getTopRulerInfo(&info);

	pDialog->setMaxWidth (info.u.c.m_xColumnWidth);

	// run the dialog
	pDialog->runModal(getFrame());

	// get the dialog answer
	AP_Dialog_Paragraph::tAnswer answer = pDialog->getAnswer();

	const XML_Char ** propitem = NULL;

	if(answer == AP_Dialog_Paragraph::a_OK)
	{
		// getDialogData() returns us XML_Char ** data we have to free
		pDialog->getDialogData(props);
		UT_ASSERT(props);

		// set properties into the vector. We have to save these as static char
        // strings so they persist past this method.

		if (props && props[0])
		{
			const XML_Char * szVals = NULL;
			for(i=0; i<NUM_PARAPROPS; i++)
			{
				szVals = UT_getAttribute(paraFields[i],props);
				if( szVals != NULL)
				{
					sprintf(paraVals[i],"%s",szVals);
					addOrReplaceVecProp((const XML_Char *) paraFields[i], (const XML_Char *) paraVals[i]);
				}
			}
		}
		// we have to loop through the props pairs, freeing each string
		// referenced, then freeing the pointers themselves
		if (props)
		{
			propitem = props;

			while (propitem[0] && propitem[1])
			{
				
				FREEP(propitem[0]);
				FREEP(propitem[1]);
				propitem += 2;
			}
		}

		// now free props
		FREEP(props);
	}
	pDialogFactory->releaseDialog(pDialog);
}

/*!
 * Extract all the props from the vector and apply them to the preview. We use
 * the style "tmp" to display the current style in the preview. 
 */
void AP_Dialog_Styles::updateCurrentStyle(void)
{
	if(m_pAbiPreview == NULL)
		return;
	const XML_Char ** props = NULL;
	UT_uint32 i = 0;
	if(m_vecAllProps.getItemCount() <= 0)
		return;
	UT_uint32 countp = m_vecAllProps.getItemCount() + 1;
	props = (const XML_Char **) calloc(countp, sizeof(XML_Char *));
	countp--;
	for(i=0; i<countp; i++)
	{
		props[i] = (const XML_Char *) m_vecAllProps.getNthItem(i);
	}
	props[i] = NULL;
	PD_Style * pStyle = NULL;
	getLDoc()->getStyle("tmp", &pStyle);
//
// clear out old description
//
	m_curStyleDesc.clear();
	for(i=0; i<countp; i+=2)
	{
		m_curStyleDesc += (const XML_Char *) m_vecAllProps.getNthItem(i);
		m_curStyleDesc += ":";
		m_curStyleDesc += (const XML_Char *) m_vecAllProps.getNthItem(i+1);
		if(i+2<countp)
			m_curStyleDesc += "; ";
	}
//
// Update the description in the Modify Dialog.
//
	setModifyDescription (m_curStyleDesc.c_str());

	if( pStyle == NULL)
	{
		const XML_Char * attrib[] = {PT_NAME_ATTRIBUTE_NAME,"tmp",PT_TYPE_ATTRIBUTE_NAME,"P","basedon","Normal","followedby","Normal","props",m_curStyleDesc.c_str(),NULL,NULL};
		getLDoc()->appendStyle(attrib);
	}
	else
	{
		getLDoc()->addStyleProperties("tmp",props);
		getLDoc()->updateDocForStyleChange("tmp",true);
	}
	getLView()->setPoint(m_posFocus+1);
	getLView()->setStyle("tmp");
	drawLocal();
	DELETEP(props);

}


/*!
 * Take the current style description and use it to define a new style
 * in the main document.
 */
bool AP_Dialog_Styles::createNewStyle(const XML_Char * szName)
{
	const XML_Char ** props = NULL;
	UT_uint32 i = 0;
	if(m_vecAllProps.getItemCount() <= 0)
		return false;
	UT_uint32 countp = m_vecAllProps.getItemCount() + 1;
	props = (const XML_Char **) calloc(countp, sizeof(XML_Char *));
	countp--;
	for(i=0; i<countp; i++)
	{
		props[i] = (const XML_Char *) m_vecAllProps.getNthItem(i);
	}
	props[i] = NULL;
//
// clear out old description
//
	m_curStyleDesc.clear();
	for(i=0; i<countp; i+=2)
	{
		m_curStyleDesc += (const XML_Char *) m_vecAllProps.getNthItem(i);
		m_curStyleDesc += ":";
		m_curStyleDesc += (const XML_Char *) m_vecAllProps.getNthItem(i+1);
		if(i+2<countp)
			m_curStyleDesc += "; ";
	}
//
// Update the description in the Main Dialog.
//
	setDescription (m_curStyleDesc.c_str());
//
// Append the new style to the main document
//
	PD_Style * pStyle = NULL;
	
	UT_ASSERT(szName);
	if(szName == NULL)
		return false;
	getDoc()->getStyle("szName", &pStyle);
	if(pStyle != NULL)
		return false;
//
// Assemble the attributes we need for this new style
//
	const XML_Char * attrib[] = {PT_NAME_ATTRIBUTE_NAME,szName,PT_TYPE_ATTRIBUTE_NAME,getAttsVal(PT_TYPE_ATTRIBUTE_NAME),"basedon",getAttsVal("basedon"),"followedby",getAttsVal("followedby"),"props",m_curStyleDesc.c_str(),NULL,NULL};
	bool bres = getDoc()->appendStyle(attrib);
	DELETEP(props);
	return bres;
}


/*!
 * Take the current style description and modify the description of the style
 * in the main document.
 */
bool AP_Dialog_Styles::applyModifiedStyleToDoc(void)
{
	const XML_Char ** props = NULL;
	UT_uint32 i = 0;
	if(m_vecAllProps.getItemCount() <= 0)
		return false;
	UT_uint32 countp = m_vecAllProps.getItemCount() + 1;
	props = (const XML_Char **) calloc(countp, sizeof(XML_Char *));
	countp--;
	for(i=0; i<countp; i++)
	{
		props[i] = (const XML_Char *) m_vecAllProps.getNthItem(i);
	}
	props[i] = NULL;
	UT_uint32 counta = m_vecAllAttribs.getItemCount() + 3;
	const XML_Char ** attribs = NULL;
	attribs = (const XML_Char **) calloc(counta, sizeof(XML_Char *));
	counta = counta -3;
	UT_sint32 iatt;
	for(iatt=0; iatt<counta; iatt++)
	{
		attribs[iatt] = (const XML_Char *) m_vecAllAttribs.getNthItem(iatt);
	}
	attribs[iatt++] = "props";
//
// clear out old description
//
	m_curStyleDesc.clear();
	UT_sint32 j;
	for(j=0; j<countp; j+=2)
	{
		m_curStyleDesc += (const XML_Char *) m_vecAllProps.getNthItem(j);
		m_curStyleDesc += ":";
		m_curStyleDesc += (const XML_Char *) m_vecAllProps.getNthItem(j+1);
		if(j+2<countp)
			m_curStyleDesc += "; ";
	}
	attribs[iatt++] = m_curStyleDesc.c_str();
	attribs[iatt] = NULL;
//
// Update the description in the Main Dialog.
//
	setDescription (m_curStyleDesc.c_str());
//
// Set the style in the main document
//
	const XML_Char * szStyle = getCurrentStyle();
	if(szStyle == NULL)
		return false;
//
// This creates a new indexAP from the attributes/properties here.
// This allows properties to be removed from a pre-existing style
//
	for(i=0; attribs[i] != NULL; i = i + 2)
	{
		UT_DEBUGMSG(("SEVIOR: name %s , value %s \n",attribs[i],attribs[i+1]));
	}
	bool bres = getDoc()->setAllStyleAttributes(szStyle,attribs);
	DELETEP(props);
	DELETEP(attribs);
	return bres;
}

void AP_Dialog_Styles::setView( FV_View * pView)
{
	m_pView = pView;
}

void AP_Dialog_Styles::setFrame( XAP_Frame * pFrame)
{
	m_pFrame = pFrame;
}

void AP_Dialog_Styles::setDoc( PD_Document * pDoc)
{
	m_pDoc = pDoc;
}


FV_View * AP_Dialog_Styles::getView(void) const
{
	return m_pView;
}

XAP_Frame * AP_Dialog_Styles::getFrame(void) const
{
	return m_pFrame;
}

PD_Document * AP_Dialog_Styles::getDoc(void) const
{
	return m_pDoc;
}

void AP_Dialog_Styles::_createParaPreviewFromGC(GR_Graphics * gc,
                                                UT_uint32 width,
						UT_uint32 height)
{
	UT_ASSERT(gc);

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	UT_UCSChar * str;

	UT_UCS_cloneString_char (&str, pSS->getValue(AP_STRING_ID_DLG_Styles_LBL_TxtMsg));

	m_pParaPreview = new AP_Preview_Paragraph(gc, str, static_cast<XAP_Dialog*>(this));
	UT_ASSERT(m_pParaPreview);

	FREEP(str);
	
	m_pParaPreview->setWindowSize(width, height);
}


void AP_Dialog_Styles::_createCharPreviewFromGC(GR_Graphics * gc,
                                                UT_uint32 width,
						UT_uint32 height)
{
	UT_ASSERT(gc);

	const XAP_StringSet * pSS = m_pApp->getStringSet();

//
// Set the Background color for the preview.
//
	static XML_Char  background[8];
	UT_RGBColor * bgCol = getView()->getCurrentPage()->getOwningSection()->getPaperColor();
	sprintf(background, "%02x%02x%02x",bgCol->m_red,bgCol->m_grn,bgCol->m_blu);

	m_pCharPreview = new XAP_Preview_FontPreview(gc,background);
	UT_ASSERT(m_pCharPreview);
	
	m_pCharPreview->setWindowSize(width, height);
//
// Text for the Preview
//
	static UT_UCSChar szString[60];
	UT_UCS_strcpy_char( (UT_UCSChar *) szString, pSS->getValue(AP_STRING_ID_DLG_Styles_LBL_TxtMsg));
	m_pCharPreview->setDrawString((const UT_UCSChar *) szString);
//
// set our Vector of Character Properties into the preview class.
//
	m_pCharPreview->setVecProperties( &m_vecCharProps);
}


void AP_Dialog_Styles::_createAbiPreviewFromGC(GR_Graphics * gc,
                                                UT_uint32 width,
											   UT_uint32 height )
{
	UT_ASSERT(gc);
	if(m_pAbiPreview)
		DELETEP(m_pAbiPreview);
	m_pAbiPreview = new AP_Preview_Abi(gc,width,height,getFrame(),PREVIEW_ZOOMED);
	UT_ASSERT(m_pAbiPreview);
}

FV_View * AP_Dialog_Styles::getLView(void) const
{
	return m_pAbiPreview->getView();
}

PD_Document * AP_Dialog_Styles::getLDoc(void) const
{
	return m_pAbiPreview->getDoc();
}

void  AP_Dialog_Styles::drawLocal(void)
{
	m_pAbiPreview->draw();
}


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
	UT_UCS_strcpy_char( (UT_UCSChar *) szString, pSS->getValue(AP_STRING_ID_DLG_Styles_LBL_TxtMsg));
	UT_UCS_strcpy_char( (UT_UCSChar *) sz1, " 1");
	UT_UCS_strcpy_char( (UT_UCSChar *) sz2, " 2");
	UT_UCS_strcpy_char( (UT_UCSChar *) sz3, " 3");
	UT_UCS_strcpy_char( (UT_UCSChar *) szSpace, "  ");
	UT_uint32 len =UT_UCS_strlen(szString);
	UT_uint32 len1 =UT_UCS_strlen(sz1);
	UT_uint32 lenSpace =UT_UCS_strlen(szSpace);
	const char * szStyle = NULL;
	if(!isNew)
	{
		szStyle = getCurrentStyle();
	}
//
// Set all the margins to 0
//
	const XML_Char * props[] = {"page-margin-left","0.0in",
							   "page-margin-right","0.0in",
							   "page-margin-top","0.0in",
							   "page-margin-bottom","0.0in",
							   "page-margin-footer","0.0in",
							   "page-margin-header","0.0in",NULL};
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

	const XML_Char * pszFGColor = NULL;
    const XML_Char * pszBGColor = NULL;
	static XML_Char Grey[8];
	static XML_Char szFGColor[8];
	UT_RGBColor FGColor(0,0,0);
	UT_RGBColor BGColor(255,255,255);
	UT_RGBColor * pageCol = NULL;
	getLView()->setStyle("Normal");

	const XML_Char ** props_in = NULL;
	getLView()->getCharFormat(&props_in);
	pszFGColor = UT_getAttribute("color", props_in);
	pszBGColor = UT_getAttribute("bgcolor",props_in);
	if(pszFGColor != NULL)
		UT_parseColor(pszFGColor,FGColor);
//
// Save the Foreground color for later
//
	sprintf(szFGColor, "%02x%02x%02x",FGColor.m_red,
				FGColor.m_grn, FGColor.m_blu);
	if(pszBGColor == NULL)
	{
		pageCol = getLView()->getCurrentPage()->getOwningSection()->getPaperColor();
		sprintf(Grey, "%02x%02x%02x",(pageCol->m_red+FGColor.m_red)/2,
				(pageCol->m_grn+FGColor.m_grn)/2, (pageCol->m_blu+FGColor.m_blu)/2);
	}
	else if(strcmp(pszBGColor,"transparent")==0)
	{
		pageCol = getLView()->getCurrentPage()->getOwningSection()->getPaperColor();
		sprintf(Grey, "%02x%02x%02x",(pageCol->m_red+FGColor.m_red)/2,
				(pageCol->m_grn+FGColor.m_grn)/2, (pageCol->m_blu+FGColor.m_blu)/2);
	}
	else
	{
		UT_parseColor(pszBGColor,BGColor);
		sprintf(Grey, "%02x%02x%02x",(BGColor.m_red+FGColor.m_red)/2,
				(BGColor.m_grn + FGColor.m_grn)/2, (BGColor.m_blu+FGColor.m_blu)/2);
	}
//
// Set the "Greyed" color for text in previous block
//
	const XML_Char * GreyCol[3] = {"color",(const XML_Char *) Grey,NULL};
	getLDoc()->changeSpanFmt(PTC_AddFmt,m_posBefore,getLView()->getPoint(),NULL,GreyCol);
	
	getLView()->insertParagraphBreak();
//
// Second Paragraph in focus. Our Vectors containing the current settings have
// been filled from calls in the platform layer.
//
	UT_uint32 countp = m_vecAllProps.getItemCount()+1;
	const XML_Char ** lprop = NULL;
	lprop = (const XML_Char **) calloc(countp, sizeof(XML_Char *));
	countp--;
	for(i=0; i<countp; i++)
	{
		lprop[i] = (const XML_Char *) m_vecAllProps.getNthItem(i);
	}
	lprop[i] = NULL;

	PD_Style * pStyle = NULL;
	getLDoc()->getStyle("tmp", &pStyle);
//
// clear out old description
//
	m_curStyleDesc.clear();
	for(i=0; i<countp; i+=2)
	{
		m_curStyleDesc += (const XML_Char *) m_vecAllProps.getNthItem(i);
		m_curStyleDesc += ":";
		m_curStyleDesc += (const XML_Char *) m_vecAllProps.getNthItem(i+1);
		if(i+2<countp)
			m_curStyleDesc += "; ";
	}
//
// Update the description in the Modify Dialog.
//
	setModifyDescription (m_curStyleDesc.c_str());

	if( pStyle == NULL)
	{
		if(strlen(m_curStyleDesc.c_str()) == 0)
			m_curStyleDesc += "font-style:normal";
		const XML_Char * attrib[] = {PT_NAME_ATTRIBUTE_NAME,"tmp",PT_TYPE_ATTRIBUTE_NAME,"P","basedon","None","followedby","Current Settings","props",m_curStyleDesc.c_str(),NULL,NULL};
		getLDoc()->appendStyle(attrib);
	}
	else
	{
		getLDoc()->addStyleProperties("tmp",lprop);
	}

	getLView()->setStyle("tmp");
	m_posFocus = getLView()->getPoint();
//
// Set Color Back
//
	pszFGColor = UT_getAttribute("color", lprop);
	if(pszFGColor == NULL)
	{
		const XML_Char * FGCol[3] = {"color",(const XML_Char *) szFGColor,NULL};
		getLView()->setCharFormat(FGCol);
	}
	DELETEP(lprop);
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


void AP_Dialog_Styles::destroyAbiPreview(void)
{
	DELETEP(m_pAbiPreview);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void AP_Dialog_Styles::event_paraPreviewUpdated (const XML_Char * pageLeftMargin,
						 const XML_Char * pageRightMargin,
						 const XML_Char * align,
						 const XML_Char * firstLineIndent,						 
						 const XML_Char * leftIndent,
						 const XML_Char * rightIndent,
						 const XML_Char * beforeSpacing,
						 const XML_Char * afterSpacing,
						 const XML_Char * lineSpacing) const
{
  // Whomever designed this preview and the Paragraph dialog should be shot

	AP_Dialog_Paragraph::tAlignState tAlign = AP_Dialog_Paragraph::align_LEFT;
	AP_Dialog_Paragraph::tIndentState tIndent = AP_Dialog_Paragraph::indent_NONE;
	AP_Dialog_Paragraph::tSpacingState tSpacing = AP_Dialog_Paragraph::spacing_MULTIPLE;

	const char * sz = NULL;
	char * pPlusFound = NULL;

	UT_ASSERT(m_pParaPreview);

	if (!align)
		goto LblIndent; // skip to the next label if nothing's set here

	if (!UT_strcmp(align, "right"))
		tAlign = AP_Dialog_Paragraph::align_RIGHT;
	else if (!UT_strcmp(align, "center"))
		tAlign = AP_Dialog_Paragraph::align_CENTERED;
	else if (!UT_strcmp(align, "justify"))
		tAlign = AP_Dialog_Paragraph::align_JUSTIFIED;

 LblIndent:
	if (!firstLineIndent)
		goto LblSpacing;

	sz = (const char *)firstLineIndent;

	if (UT_convertDimensionless(sz) > (double) 0)
    {
		tIndent = AP_Dialog_Paragraph::indent_FIRSTLINE;
    }
	else if (UT_convertDimensionless(sz) < (double) 0)
    {
		tIndent = AP_Dialog_Paragraph::indent_HANGING;
    }

 LblSpacing:
	if (!lineSpacing)
		goto LblSet;

	sz = (const char *)lineSpacing;

	pPlusFound = strrchr(sz, '+');
	if (pPlusFound && *(pPlusFound + 1) == 0)
		tSpacing = AP_Dialog_Paragraph::spacing_ATLEAST;

	{
		if(UT_hasDimensionComponent(sz))
			tSpacing = AP_Dialog_Paragraph::spacing_EXACTLY;
		else if(!UT_strcmp("1.0", sz))
			tSpacing = AP_Dialog_Paragraph::spacing_SINGLE;
		else if(!UT_strcmp("1.5", sz))
			tSpacing = AP_Dialog_Paragraph::spacing_ONEANDHALF;
		else if(!UT_strcmp("2.0", sz))
			tSpacing = AP_Dialog_Paragraph::spacing_DOUBLE;
	}

 LblSet:
	m_pParaPreview->setFormat (pageLeftMargin,
							   pageRightMargin,
							   tAlign,
							   firstLineIndent,
							   tIndent,
							   leftIndent,
							   rightIndent,
							   beforeSpacing,
							   afterSpacing,
							   lineSpacing,
							   tSpacing);
	
	// force a redraw
	m_pParaPreview->draw();
}

void AP_Dialog_Styles::event_charPreviewUpdated (void) const
{
	UT_ASSERT (m_pCharPreview); // add this when we make a char preview

	// force a redraw
	if(m_pCharPreview) 
	{
		m_pCharPreview->setVecProperties( &m_vecCharProps);
		m_pCharPreview->draw();
	}
}

void AP_Dialog_Styles::_populatePreviews(bool isModify)
{
	PD_Style * pStyle = NULL;
	const char * szStyle = NULL;

	const static XML_Char * paraFields[] = {"text-align", "text-indent", "margin-left", "margin-right", 
											"margin-top", "margin-bottom", "line-height"};
	const size_t nParaFlds = sizeof(paraFields)/sizeof(paraFields[0]);
	const XML_Char * paraValues [nParaFlds];

	const static XML_Char * charFields[] = 
	{"bgcolor","color","font-family","font-size","font-stretch","font-style", 
	 "font-variant", "font-weight","text-decoration"};
	const size_t nCharFlds = sizeof(charFields)/sizeof(charFields[0]);
	const XML_Char * charValues [nCharFlds];

	szStyle = getCurrentStyle();

	if (!szStyle) // having nothing displayed is totally valid
	{
		return;
	}
//
// Load up our properties vector
//
	fillVecWithProps(szStyle);

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
			const XML_Char * szName = paraFields[i];
			const XML_Char * szValue = NULL;		

			if (!pStyle->getProperty(szName, szValue))
				if (!pStyle->getAttribute(szName, szValue))
				{
					paraValues[i] = 0;
					continue;
				}
				
			m_curStyleDesc += (const char *)szName;
			m_curStyleDesc += ":";
			m_curStyleDesc += (const char *)szValue;
			m_curStyleDesc += "; ";

			paraValues[i] = szValue;
		}

// Clear out old contents of the char vector if they exist
		if(m_vecCharProps.getItemCount() > 0)
			m_vecCharProps.clear();

	    // now loop through and pass out each property:value combination for characters

		for(i = 0; i < nCharFlds; i++)
		{
			const XML_Char * szName = charFields[i];
			const XML_Char * szValue = NULL;		

			if (!pStyle->getProperty(szName, szValue))
				if (!pStyle->getAttribute(szName, szValue))
				{
					charValues[i] = 0;
					continue;
				}

			m_curStyleDesc += (const char *)szName;
			m_curStyleDesc += ":";
			m_curStyleDesc += (const char *)szValue;
			m_curStyleDesc += "; ";

			charValues[i] = szValue;
//
// Put them in our property vector for the Character preview
//
			m_vecCharProps.addItem((void *) szName);
			m_vecCharProps.addItem((void *) szValue);
					
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
			const XML_Char ** props_in = NULL;
			getView()->getSectionFormat(&props_in);
			
			if(!isModify)
				event_paraPreviewUpdated(UT_getAttribute("page-margin-left", props_in), 
									 UT_getAttribute("page-margin-right", props_in),
									 (const XML_Char *)paraValues[0], (const XML_Char *)paraValues[1],
									 (const XML_Char *)paraValues[2], (const XML_Char *)paraValues[3], 
									 (const XML_Char *)paraValues[4], (const XML_Char *)paraValues[5],
									 (const XML_Char *)paraValues[6]);
			if(!isModify)
				event_charPreviewUpdated();
		}
	}
}





