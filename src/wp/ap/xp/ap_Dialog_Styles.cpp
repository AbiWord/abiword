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

#include "ut_misc.h"

#include "pd_Style.h"
#include "ap_Dialog_Styles.h"
#include "ut_string_class.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"

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

void AP_Dialog_Styles::fillVecWithProps(const XML_Char * szStyle)
{
	PD_Style * pStyle = NULL;
	m_vecAllProps.clear();
	m_vecAllAttribs.clear();
	if(szStyle == NULL || ! m_pDoc->getStyle(szStyle,&pStyle))
	{
		return;
	}

	const static XML_Char * paraFields[] = {"text-align", "text-indent", "margin-left", "margin-right", "margin-top", "margin-bottom", "line-height","tabstops","start-value","list-delim", "list-decimal","field-font","field-color"};

	const size_t nParaFlds = sizeof(paraFields)/sizeof(paraFields[0]);

	const static XML_Char * charFields[] = 
	{"bgcolor","color","font-family","font-size","font-stretch","font-style", 
	 "font-variant", "font-weight","text-decoration"};

	const size_t nCharFlds = sizeof(charFields)/sizeof(charFields[0]);

	const static XML_Char * attribs[] = 
	{"followedby","basedon","listid","parentid","level","style"};

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
		if(!szValue)
			pStyle->getAttribute(szName,szValue);
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
		if(!szValue)
			pStyle->getAttribute(szName,szValue);
		if(szValue)
			addOrReplaceVecProp(szName, szValue);
	}
//
// Loop through all the attributes and add those with non-null values
//
	for(i = 0; i < nattribs; i++)
	{
		const XML_Char * szName = attribs[i];
		const XML_Char * szValue = NULL;		
		pStyle->getProperty(szName,szValue);
		if(!szValue)
			pStyle->getAttribute(szName,szValue);
		if(szValue)
			addOrReplaceVecAttribs(szName, szValue);
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

void AP_Dialog_Styles::ModifyFont(void)
{
	UT_DEBUGMSG(("SEVIOR: Doing stuff in Modify Fonts\n"));
//
// Fire up the Font Chooser dialog. Code stolen straight from ap_EditMethods.
//

	XAP_Dialog_Id id = XAP_DIALOG_ID_FONT;

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(m_pFrame->getDialogFactory());

	XAP_Dialog_FontChooser * pDialog
		= (XAP_Dialog_FontChooser *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);

	// stuff the GR_Graphics into the dialog so that it
	// can query the system for font info relative to our
	// context.

	pDialog->setGraphicsContext(m_pView->getLayout()->getGraphics());


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
	UT_RGBColor * bgCol = m_pView->getCurrentPage()->getOwningSection()->getPaperColor();
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
	const XML_Char * s = getPropsVal("text-decoration");
	if (s)
	{
		bUnderline = (strstr(s, "underline") != NULL);
		bOverline = (strstr(s, "overline") != NULL);
		bStrikeOut = (strstr(s, "line-through") != NULL);
	}
	pDialog->setFontDecoration(bUnderline,bOverline,bStrikeOut);
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

	pDialog->runModal(m_pFrame);

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
/*
#ifdef BIDI_ENABLED
		bool bDirection = false;
		bool bChangedDirection = pDialog->getChangedDirection(&bDirection);
#endif
*/

		if (bChangedUnderline || bChangedStrikeOut || bChangedOverline)
		{
			if (bUnderline && bStrikeOut && bOverline)
				s = "underline line-through overline";
			else if (bUnderline && bOverline)
				s = "underline overline";
			else if (bStrikeOut && bOverline)
				s = "line-through overline";
			else if (bStrikeOut && bUnderline)
				s = "line-through underline";
			else if (bStrikeOut)
				s = "line-through";
			else if (bUnderline)
				s = "underline";
			else if (bOverline)
				s = "overline";
			else
				s = "none";

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
	updateCurrentStyle();
}

void AP_Dialog_Styles::ModifyTabs(void)
{
	UT_DEBUGMSG(("SEVIOR: Doing stuff in Modify Tabs \n"));
//
// Fire up the tabs dialog. TODO. Should move the PieceTable manipulations into
// ap_EditMethods so we can intercept them.
//

}

void AP_Dialog_Styles::ModifyLists(void)
{
	UT_DEBUGMSG(("SEVIOR: Doing stuff in Modify Lists \n"));
}
void AP_Dialog_Styles::ModifyParagraph(void)
{
	UT_DEBUGMSG(("SEVIOR: Doing stuff in Modify Lists \n"));
}

/*!
 * Extract all the props from the vector and apply them to the preview. We use
 * the style "tmp" to display the current style in the preview. 
 */
void AP_Dialog_Styles::updateCurrentStyle()
{

	const XML_Char ** props = NULL;
	UT_uint32 i = 0;
	if(m_vecAllProps.getItemCount() <= 0)
		return;
	UT_uint32 countp = m_vecAllProps.getItemCount() + 1;
	props = (const XML_Char **) calloc(countp, sizeof(XML_Char *));

	for(i=0; i<countp; i++)
	{
		props[i] = (const XML_Char *) m_vecAllProps.getNthItem(i);
	}
	props[i] = NULL;
	PD_Style * pStyle = NULL;
	getLDoc()->getStyle("tmp", &pStyle); 
	if( pStyle == NULL)
	{
		const XML_Char * attrib[] = {PT_NAME_ATTRIBUTE_NAME,"tmp",PT_TYPE_ATTRIBUTE_NAME,"P","basedon","Normal","followedby","Normal","props","",NULL};
		getLDoc()->appendStyle(attrib);
	}
	getLDoc()->setStyleProperties("tmp",props);
	getLView()->setPoint(m_posFocus);
	getLView()->setStyle("tmp");
	drawLocal();
	DELETEP(props);

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
	UT_RGBColor * bgCol = m_pView->getCurrentPage()->getOwningSection()->getPaperColor();
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
	m_pAbiPreview = new AP_Preview_Abi(gc,width,height,m_pFrame,PREVIEW_ZOOMED);
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
		szStyle = getCurrentStyle();
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
	if(pszBGColor == NULL && strcmp(pszBGColor,"transparent")==0)
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
// Second Paragraph in focus
//
	if(!isNew)
		getLView()->setStyle(szStyle);
	m_posFocus = getLView()->getPoint();
//
// Set Color Back
//
	const XML_Char * FGCol[3] = {"color",(const XML_Char *) szFGColor,NULL};
	getLView()->setCharFormat(FGCol);
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

	// update the previews and the description label
	if (m_pDoc->getStyle (szStyle, &pStyle))
	{
		UT_uint32 i;
//
// Clear any previous stuff from our style description. This description must
// persist beyond this method because we want to modify it.
//
		m_curStyleDesc = "";

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
			m_pView->getSectionFormat(&props_in);
			
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














