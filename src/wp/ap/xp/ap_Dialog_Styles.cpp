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

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "fl_SectionLayout.h"
#include "fp_Page.h"

#include "ut_misc.h"

#include "pd_Style.h"
#include "ap_Dialog_Styles.h"
#include "ut_string_class.h"

AP_Dialog_Styles::AP_Dialog_Styles(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id)
{

	m_answer = a_OK;
	m_pParaPreview = NULL;
	m_pCharPreview = NULL;
	if(m_vecCharProps.getItemCount() > 0)
		m_vecCharProps.clear();
}

AP_Dialog_Styles::~AP_Dialog_Styles(void)
{
	DELETEP(m_pParaPreview);
	DELETEP(m_pCharPreview);
}

AP_Dialog_Styles::tAnswer AP_Dialog_Styles::getAnswer(void) const
{
	return m_answer;
}

void AP_Dialog_Styles::_createParaPreviewFromGC(GR_Graphics * gc,
                                                UT_uint32 width,
						UT_uint32 height)
{
	UT_ASSERT(gc);
	// TODO: translate me

	UT_UCSChar * str;

	UT_UCS_cloneString_char (&str, "What Hath God Wrought");

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
	UT_UCS_strcpy_char( (UT_UCSChar *) szString, "What Hath God Wrought");
	m_pCharPreview->setDrawString((const UT_UCSChar *) szString);
//
// set our Vector of Character Properties into the preview class.
//
	m_pCharPreview->setVecProperties( &m_vecCharProps);
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

void AP_Dialog_Styles::_populatePreviews(void)
{
	PD_Style * pStyle = NULL;
	const char * szStyle = NULL;

	const static XML_Char * paraFields[] = {"text-align", "text-indent", "margin-left", "margin-right", 
											"margin-top", "margin-bottom", "line-height"};
	const size_t nParaFlds = sizeof(paraFields)/sizeof(paraFields[0]);
	const XML_Char * paraValues [nParaFlds];

//
// Note to Dom: Actually do this code for character previews
//
	const static XML_Char * charFields[] = 
	{"bgcolor","color","font-family","font-size","font-stretch","font-variant",
	"font-weight","text-decoration"};
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
		UT_String strDesc;

	    // first loop through and pass out each property:value combination for paragraphs
		for(i = 0; i < nParaFlds; i++)
		{
			const XML_Char * szName = paraFields[i];
			const XML_Char * szValue = NULL;		

			if (!pStyle->getProperty(szName, szValue))
				if (!pStyle->getAttribute(szName, szValue))
				{
					UT_DEBUGMSG(("DOM: could not obtain property/attribute %s (%d)\n",
								 szName, i));
					paraValues[i] = 0;
					continue;
				}

			UT_DEBUGMSG(("DOM: paragraph property is: (%s, %s)\n", szName, szValue));
				
			strDesc += (const char *)szName;
			strDesc += ":";
			strDesc += (const char *)szValue;
			strDesc += "; ";

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
					UT_DEBUGMSG(("DOM: could not obtain property/attribute %s (%d)\n",
								 szName, i));
					charValues[i] = 0;
					continue;
				}

			UT_DEBUGMSG(("DOM: char property is: (%s, %s)\n", szName, szValue));
				
			strDesc += (const char *)szName;
			strDesc += ":";
			strDesc += (const char *)szValue;

			if (i != nCharFlds)
				strDesc += "; ";

			charValues[i] = szValue;
//
// Put them in our property vector for the Character preview
//
			m_vecCharProps.addItem((void *) szName);
			m_vecCharProps.addItem((void *) szValue);
					
		}

		if (!strDesc.empty())
		{
			setDescription (strDesc.c_str());
		
			const XML_Char ** props_in = NULL;
			m_pView->getSectionFormat(&props_in);

			event_paraPreviewUpdated(UT_getAttribute("page-margin-left", props_in), UT_getAttribute("page-margin-right", props_in),
									 (const XML_Char *)paraValues[0], (const XML_Char *)paraValues[1],
									 (const XML_Char *)paraValues[2], (const XML_Char *)paraValues[3], 
									 (const XML_Char *)paraValues[4], (const XML_Char *)paraValues[5],
									 (const XML_Char *)paraValues[6]);
			UT_DEBUGMSG(("SEVIOR: Calling FontPreview Draw \n"));
			event_charPreviewUpdated();
		}
	}
}














