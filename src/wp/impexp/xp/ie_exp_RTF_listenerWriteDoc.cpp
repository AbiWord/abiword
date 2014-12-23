/* -*- c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

/******************************************************************
** This file is considered private to ie_exp_RTF.cpp
** This is a PL_Listener.  It's purpose is to actually write
** the contents of the document to the RTF file.
******************************************************************/

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "ut_rand.h"
#include "ut_locale.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_units.h"
#include "ut_png.h"
#include "ut_jpeg.h"
#include "ut_bytebuf.h"
#include "ut_math.h"
#include "ie_exp_RTF_listenerWriteDoc.h"
#include "ie_exp_RTF_AttrProp.h"
#include "pd_Document.h"
#include "pd_Style.h"
#include "pf_Frag_Strux.h"
#include "xap_App.h"
#include "pp_AttrProp.h"
#include "pp_Property.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "pt_Types.h"
#include "fl_AutoNum.h"
#include "fl_AutoLists.h"
#include "fl_BlockLayout.h"
#include "fp_Run.h"
#include "fl_Layout.h"
#include "fl_TableLayout.h"
#include "fl_FrameLayout.h"
#include "ut_rand.h"
#include "ut_svg.h"

#include "xap_EncodingManager.h"
#include "ut_string_class.h"

#include <pd_DocumentRDF.h>

static UT_sint32 convertInchToTwips(double inch)
{
	return static_cast<UT_sint32>(inch*1440.0 +0.5);
}


static UT_sint32 convertTwipsToEMU(UT_sint32 twip)
{
	return static_cast<UT_sint32>(914400.0*static_cast<double>(twip)/1440.0);
}

void s_RTF_ListenerWriteDoc::_closeSection(void)
{
	m_apiThisSection = 0;
	m_sdh = NULL;
	return;
}

void s_RTF_ListenerWriteDoc::_closeBlock(PT_AttrPropIndex  /*nextApi*/)
{
	if(!m_bInBlock)
		return;
	
	// first reset ie's char direciton info
	m_pie->setCharRTL(UT_BIDI_UNSET);
//
// Force the output of char properties for blank lines or list items.
//
	xxx_UT_DEBUGMSG(("SEVIOR: Close Block \n"));
	
	if(m_bInSpan)
	{
		_closeSpan();
	}

	if(m_sdh && m_pDocument->getStruxType(m_sdh) == PTX_Block)
  	{
//
// This is a blankline or list item
//
// output the character properties for this break.
//
		const PP_AttrProp * pSpanAP = NULL;
		m_pDocument->getSpanAttrProp(m_sdh,0,true,&pSpanAP);
		xxx_UT_DEBUGMSG(("SEVIOR: Close Block -open span \n"));
		_openSpan(m_apiThisBlock,pSpanAP);
	}

	m_bBlankLine = false;
	
	xxx_UT_DEBUGMSG(("Doing write par now \n"));
	m_pie->_rtf_keyword("par");
	_closeSpan();

	m_apiThisBlock = 0;
	m_sdh = NULL;
	return;
}

void s_RTF_ListenerWriteDoc::_closeSpan(void)
{
	if (!m_bInSpan)
		return;

	m_pie->_rtf_close_brace();
	m_bInSpan = false;
	return;
}

// Frame Background

static void s_background_properties (const gchar * pszBgStyle, const gchar * pszBgColor,
									 const gchar * pszBackgroundColor,
									 PP_PropertyMap::Background & background)
{
	if (pszBgStyle)
		{
			if (strcmp (pszBgStyle, "0") == 0)
				{
					background.m_t_background = PP_PropertyMap::background_none;
				}
			else if (strcmp (pszBgStyle, "1") == 0)
				{
					if (pszBgColor)
						{
							background.m_t_background = PP_PropertyMap::background_type (pszBgColor);
							if (background.m_t_background == PP_PropertyMap::background_solid)
								UT_parseColor (pszBgColor, background.m_color);
						}

				}
		}

	if (pszBackgroundColor)
		{
			background.m_t_background = PP_PropertyMap::background_type (pszBackgroundColor);
			if (background.m_t_background == PP_PropertyMap::background_solid)
				UT_parseColor (pszBackgroundColor, background.m_color);
		}
}

static void s_border_properties (const gchar * border_color, const gchar * border_style, const gchar * border_width,
								 const gchar * color, PP_PropertyMap::Line & line)
{
	/* frame-border properties:
	 * 
	 * (1) color      - defaults to value of "color" property
	 * (2) line-style - defaults to solid (in contrast to "none" in CSS)
	 * (3) thickness  - defaults to 1 layout unit (??, vs "medium" in CSS)
	 */
	line.reset ();

	PP_PropertyMap::TypeColor t_border_color = PP_PropertyMap::color_type (border_color);
	if (t_border_color)
		{
			line.m_t_color = t_border_color;
			if (t_border_color == PP_PropertyMap::color_color)
				UT_parseColor (border_color, line.m_color);
		}
	else if (color)
		{
			PP_PropertyMap::TypeColor t_color = PP_PropertyMap::color_type (color);

			line.m_t_color = t_color;
			if (t_color == PP_PropertyMap::color_color)
				UT_parseColor (color, line.m_color);
		}

	line.m_t_linestyle = PP_PropertyMap::linestyle_type (border_style);
	if (!line.m_t_linestyle)
		line.m_t_linestyle = PP_PropertyMap::linestyle_solid;

	line.m_t_thickness = PP_PropertyMap::thickness_type (border_width);
	if (line.m_t_thickness == PP_PropertyMap::thickness_length)
		{
			if (UT_determineDimension (border_width, (UT_Dimension)-1) == DIM_PX)
				{
					double thickness = UT_LAYOUT_RESOLUTION * UT_convertDimensionless (border_width);
					line.m_thickness = static_cast<UT_sint32>(thickness / UT_PAPER_UNITS_PER_INCH);
				}
			else
				line.m_thickness = convertInchToTwips(UT_convertToInches (border_width));

			if (!line.m_thickness)
				{
					double thickness = UT_LAYOUT_RESOLUTION;
					line.m_thickness = static_cast<UT_sint32>(thickness / UT_PAPER_UNITS_PER_INCH);
				}
		}
	else // ??
		{
			double thickness = UT_LAYOUT_RESOLUTION;
			line.m_thickness = static_cast<UT_sint32>(thickness / UT_PAPER_UNITS_PER_INCH);
		}
}

void s_RTF_ListenerWriteDoc::_writeSPNumProp(const char * prop, UT_sint32 val)
{
	m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword("sp");
	m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword("sn ");
	m_pie->write(prop);
	m_pie->_rtf_close_brace();
	m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword("sv ");
	UT_UTF8String sTmp = UT_UTF8String_sprintf("%d",val);
	m_pie->write(sTmp.utf8_str());
	m_pie->_rtf_close_brace();
	m_pie->_rtf_close_brace();
}



/*!
 * OK export all the TOC properties in RTF format. 
 */
void s_RTF_ListenerWriteDoc::_writeTOC(PT_AttrPropIndex apiTOC)
{
//
// OK get TOC properties
//
	const PP_AttrProp * pSectionAP = NULL;
	m_pDocument->getAttrProp(apiTOC,&pSectionAP);
	m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword("field");
	m_pie->_rtf_keyword("fldedit");
	m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword("*");
	m_pie->_rtf_keyword("fldinst ");
	m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword(" TOC ");
//
// For now just close it all up. Later we'll have to worry about exporting
// bookmarks and the text of each heading in the TOC
//
	m_pie->_rtf_close_brace();
	m_pie->_rtf_close_brace();
	m_pie->_rtf_close_brace();


	// I can't think of any properties we need for now.
	// If we need any later, we'll add them. -PL
	const gchar *pszTOCPID = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-id",pszTOCPID))
	{
	}
	else
	{
	}
#if 0
	m_sNumOff1 = "0.5in";
	m_sNumOff2 = "0.5in";
	m_sNumOff3 = "0.5in";
	m_sNumOff4 = "0.5in";
#endif


	const gchar *pszINDENT = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-indent1",pszINDENT))
	{
	}
	else
	{
	}
	pszINDENT = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-indent2",pszINDENT))
	{
	}
	else
	{
	}

	pszINDENT = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-indent3",pszINDENT))
	{
	}
	else
	{
	}

	pszINDENT = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-indent4",pszINDENT))
	{
	}
	else
	{
	}

	const gchar *pszTOCSRC = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-source-style1",pszTOCSRC))
	{
	}
	else
	{
	}
	pszTOCSRC = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-source-style2",pszTOCSRC))
	{
	}
	else
	{
	}
	pszTOCSRC = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-source-style3",pszTOCSRC))
	{
	}
	else
	{
	}
	pszTOCSRC = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-source-style4",pszTOCSRC))
	{
	}
	else
	{
	}
	const gchar * pszTOCDEST = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-dest-style1",pszTOCDEST))
	{
	}
	else
	{
	}
	pszTOCDEST = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-dest-style2",pszTOCDEST))
	{
	}
	else
	{
	}
	pszTOCDEST = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-dest-style3",pszTOCDEST))
	{
	}
	else
	{
	}
	pszTOCDEST = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-dest-style4",pszTOCDEST))
	{
	}
	else
	{
	}
	const gchar * pszTOCHEADING = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-heading",pszTOCHEADING))
	{
	}
	else
	{
	}

	const gchar * pszTOCHEADINGStyle = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-heading-style",pszTOCHEADINGStyle))
	{
	}
	else
	{
	}


	const gchar * pszTOCHASHEADING = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-has-heading",pszTOCHASHEADING))
	{
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCHASHEADING,"1") == 0)
		{
		}
		else
		{
		}
	}
//
// TOC Label
//
	const gchar * pszTOCLABEL = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-has-label1",pszTOCLABEL))
	{
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCLABEL,"1") == 0)
		{
		}
		else
		{
		}
	}
	pszTOCLABEL = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-has-label2",pszTOCLABEL))
	{
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCLABEL,"1") == 0)
		{
		}
		else
		{
		}
	}
	pszTOCLABEL = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-has-label3",pszTOCLABEL))
	{
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCLABEL,"1") == 0)
		{
		}
		else
		{
		}
	}
	pszTOCLABEL = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-has-label4",pszTOCLABEL))
	{
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCLABEL,"1") == 0)
		{
		}
		else
		{
		}
	}
//
// TOC Label Inherits
//
	const gchar * pszTOCLABELINHERITS = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-inherits1",pszTOCLABELINHERITS))
	{
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCLABELINHERITS,"1") == 0)
		{
		}
		else
		{
		}
	}
	pszTOCLABELINHERITS = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-inherits2",pszTOCLABELINHERITS))
	{
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCLABELINHERITS,"1") == 0)
		{
		}
		else
		{
		}
	}
	pszTOCLABELINHERITS = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-inherits3",pszTOCLABELINHERITS))
	{
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCLABELINHERITS,"1") == 0)
		{
		}
		else
		{
		}
	}
	pszTOCLABELINHERITS = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-inherits4",pszTOCLABELINHERITS))
	{
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCLABELINHERITS,"1") == 0)
		{
		}
		else
		{
		}
	}
//
// TOC Label Type
//
	const gchar * pszTOCLABELTYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-type1",pszTOCLABELTYPE))
	{
	}
	else
	{
	}
	pszTOCLABELTYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-type2",pszTOCLABELTYPE))
	{
	}
	else
	{
	}
	pszTOCLABELTYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-type3",pszTOCLABELTYPE))
	{
	}
	else
	{
	}
	pszTOCLABELTYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-type4",pszTOCLABELTYPE))
	{
	}
	else
	{
	}
//
// TOC Label Before Text
//
	const gchar * pszTOCSTRBEFORE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-before1",pszTOCSTRBEFORE))
	{
	}
	else
	{
	}
	pszTOCSTRBEFORE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-before2",pszTOCSTRBEFORE))
	{
	}
	else
	{
	}
	pszTOCSTRBEFORE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-before3",pszTOCSTRBEFORE))
	{
	}
	else
	{
	}
	pszTOCSTRBEFORE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-before4",pszTOCSTRBEFORE))
	{
	}
	else
	{
	}
//
// TOC Label After Text
//
	const gchar * pszTOCSTRAFTER = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-after1",pszTOCSTRAFTER))
	{
	}
	else
	{
	}
	pszTOCSTRAFTER = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-after2",pszTOCSTRAFTER))
	{
	}
	else
	{
	}
	pszTOCSTRAFTER = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-after2",pszTOCSTRAFTER))
	{
	}
	else
	{
	}
	pszTOCSTRAFTER = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-after4",pszTOCSTRAFTER))
	{
	}
	else
	{
	}
//
// TOC Label Initial Value
//
	const gchar * pszTOCLABELSTART = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-start1",pszTOCLABELSTART))
	{
	}
	else
	{
	}
	pszTOCLABELSTART = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-start2",pszTOCLABELSTART))
	{
	}
	else
	{
	}
	pszTOCLABELSTART = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-start3",pszTOCLABELSTART))
	{
	}
	else
	{
	}
	pszTOCLABELSTART = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-start4",pszTOCLABELSTART))
	{
	}
	else
	{
	}
//
// TOC Page Number Type
//
	const gchar * pszTOCPAGETYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-page-type1",pszTOCPAGETYPE))
	{
	}
	else
	{
	}
	pszTOCPAGETYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-page-type2",pszTOCPAGETYPE))
	{
	}
	else
	{
	}
	pszTOCPAGETYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-page-type3",pszTOCPAGETYPE))
	{
	}
	else
	{
	}
	pszTOCPAGETYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-page-type4",pszTOCPAGETYPE))
	{
	}
	else
	{
	}
//
// TOC TAB leader
//
	const gchar * pszTOCTABTYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-tab-leader1",pszTOCTABTYPE))
	{
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCTABTYPE,"none") == 0)
		{
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"dot") == 0)
		{
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"hyphen") == 0)
		{
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"underline") == 0)
		{
		}
		else
		{
		}
	}
	pszTOCTABTYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-tab-leader2",pszTOCTABTYPE))
	{
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCTABTYPE,"none") == 0)
		{
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"dot") == 0)
		{
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"hyphen") == 0)
		{
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"underline") == 0)
		{
		}
		else
		{
		}
	}
	pszTOCTABTYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-tab-leader3",pszTOCTABTYPE))
	{
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCTABTYPE,"none") == 0)
		{
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"dot") == 0)
		{
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"hyphen") == 0)
		{
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"underline") == 0)
		{
		}
		else
		{
		}
	}
	pszTOCTABTYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-tab-leader4",pszTOCTABTYPE))
	{
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCTABTYPE,"none") == 0)
		{
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"dot") == 0)
		{
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"hyphen") == 0)
		{
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"underline") == 0)
		{
		}
		else
		{
		}
	}

	pszTOCTABTYPE = NULL;
	if(pSectionAP && pSectionAP->getProperty("toc-range-bookmark",pszTOCTABTYPE))
	{
	}
	else
	{
	}

}

/*!
 * OK export all the frame properties in RTF format. Use the \shp definitions
 * for this.
 */
void s_RTF_ListenerWriteDoc::_openFrame(PT_AttrPropIndex apiFrame)
{
	if(m_bInFrame)
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}
//
// OK get frame properties
//
	const PP_AttrProp * pSectionAP = NULL;
	m_pDocument->getAttrProp(apiFrame,&pSectionAP);
	m_apiThisFrame = apiFrame;
	m_bInFrame = true;

	const gchar *pszFrameType = NULL;
	const gchar *pszWrapMode = NULL;
	const gchar *pszPositionTo = NULL;
	const gchar *pszXpos = NULL;
	const gchar *pszYpos = NULL;
	const gchar *pszColXpos = NULL;
	const gchar *pszColYpos = NULL;
	const gchar *pszPageXpos = NULL;
	const gchar *pszPageYpos = NULL;
	const gchar *pszWidth = NULL;
	const gchar *pszHeight = NULL;
	const gchar *pszXpad = NULL;
	const gchar *pszYpad = NULL;

	const gchar * pszColor = NULL;
	const gchar * pszBorderColor = NULL;
	const gchar * pszBorderStyle = NULL;
	const gchar * pszBorderWidth = NULL;
	
	FL_FrameType iFrameType = FL_FRAME_TEXTBOX_TYPE;
	FL_FrameFormatMode iFramePositionTo = FL_FRAME_POSITIONED_TO_BLOCK;
	FL_FrameWrapMode iFrameWrapMode = FL_FRAME_ABOVE_TEXT;
	UT_sint32 iXpos = convertInchToTwips(UT_convertToInches("0.0in"));
	UT_sint32 iYpos = convertInchToTwips(UT_convertToInches("0.0in"));
	UT_sint32 iXColumn = convertInchToTwips(UT_convertToInches("0.0in"));
	UT_sint32 iYColumn = convertInchToTwips(UT_convertToInches("0.0in"));
	UT_sint32 iXPage = convertInchToTwips(UT_convertToInches("0.0in"));
	UT_sint32 iYPage = convertInchToTwips(UT_convertToInches("0.0in"));
	UT_sint32 iWidth = convertInchToTwips(UT_convertToInches("1.0in"));
	UT_sint32 iHeight = convertInchToTwips(UT_convertToInches("1.0in"));
	UT_sint32 iXpad = convertInchToTwips(UT_convertToInches("0.03in"));
	UT_sint32 iYpad = convertInchToTwips(UT_convertToInches("0.03in"));

	PP_PropertyMap::Line leftLine;
	PP_PropertyMap::Line rightLine;
	PP_PropertyMap::Line topLine;
	PP_PropertyMap::Line botLine;
	PP_PropertyMap::Background  background;
				
// Frame Type

	if(!pSectionAP || !pSectionAP->getProperty("frame-type",pszFrameType))
	{
		iFrameType = FL_FRAME_TEXTBOX_TYPE;
	}
	else if(strcmp(pszFrameType,"textbox") == 0)
	{
		iFrameType = FL_FRAME_TEXTBOX_TYPE;
	}
	else if(strcmp(pszFrameType,"image") == 0)
	{
		iFrameType = FL_FRAME_WRAPPER_IMAGE;
	}
	else 
	{
		UT_DEBUGMSG(("Unknown Frame Type %s \n",pszFrameType));
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		iFrameType = FL_FRAME_TEXTBOX_TYPE;
	}

// Position-to

	if(!pSectionAP || !pSectionAP->getProperty("position-to",pszPositionTo))
	{
		iFramePositionTo = FL_FRAME_POSITIONED_TO_BLOCK;
	}
	else if(strcmp(pszPositionTo,"block-above-text") == 0)
	{
		iFramePositionTo = FL_FRAME_POSITIONED_TO_BLOCK;
	}
	else if(strcmp(pszPositionTo,"column-above-text") == 0)
	{
		iFramePositionTo = FL_FRAME_POSITIONED_TO_COLUMN;
	}
	else if(strcmp(pszPositionTo,"page-above-text") == 0)
	{
		iFramePositionTo = FL_FRAME_POSITIONED_TO_PAGE;
	}
	else 
	{
		UT_DEBUGMSG(("Unknown Position to %s \n",pszPositionTo));
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		iFramePositionTo =  FL_FRAME_POSITIONED_TO_BLOCK;
	}

// wrap-mode

	if(!pSectionAP || !pSectionAP->getProperty("wrap-mode",pszWrapMode))
	{
		iFrameWrapMode = FL_FRAME_ABOVE_TEXT;
	}
	else if(strcmp(pszWrapMode,"above-text") == 0)
	{
		iFrameWrapMode = FL_FRAME_ABOVE_TEXT;
	}
	else if(strcmp(pszWrapMode,"below-text") == 0)
	{
		iFrameWrapMode = FL_FRAME_BELOW_TEXT;
	}
	else if(strcmp(pszWrapMode,"wrapped-to-right") == 0)
	{
		iFrameWrapMode = FL_FRAME_WRAPPED_TO_RIGHT;
	}
	else if(strcmp(pszWrapMode,"wrapped-to-left") == 0)
	{
		iFrameWrapMode = FL_FRAME_WRAPPED_TO_LEFT;
	}
	else if(strcmp(pszWrapMode,"wrapped-both") == 0)
	{
		iFrameWrapMode = FL_FRAME_WRAPPED_BOTH_SIDES;
	}
	else 
	{
		UT_DEBUGMSG(("Unknown wrap-mode %s \n",pszWrapMode));
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		iFrameWrapMode = FL_FRAME_ABOVE_TEXT;
	}

// Xpos

	if(!pSectionAP || !pSectionAP->getProperty("xpos",pszXpos))
	{
		iXpos = convertInchToTwips(UT_convertToInches("0.0in"));
	}
	else
	{
		iXpos = convertInchToTwips(UT_convertToInches(pszXpos));
	}
	UT_DEBUGMSG(("xpos for frame is %s \n",pszXpos));
// Ypos

	if(!pSectionAP || !pSectionAP->getProperty("ypos",pszYpos))
	{
		iYpos = convertInchToTwips(UT_convertToInches("0.0in"));
	}
	else
	{
		iYpos = convertInchToTwips(UT_convertToInches(pszYpos));
	}
	UT_DEBUGMSG(("ypos for frame is %s \n",pszYpos));

// ColXpos

	if(!pSectionAP || !pSectionAP->getProperty("frame-col-xpos",pszColXpos))
	{
		iXColumn = convertInchToTwips(UT_convertToInches("0.0in"));
	}
	else
	{
		iXColumn = convertInchToTwips(UT_convertToInches(pszColXpos));
	}
	UT_DEBUGMSG(("ColXpos for frame is %s \n",pszColXpos));

// colYpos

	if(!pSectionAP || !pSectionAP->getProperty("frame-col-ypos",pszColYpos))
	{
		iYColumn = convertInchToTwips(UT_convertToInches("0.0in"));
	}
	else
	{
		iYColumn = convertInchToTwips(UT_convertToInches(pszColYpos));
	}
	UT_DEBUGMSG(("ColYpos for frame is %s units %d \n",pszColYpos,iYColumn));


// PageXpos

	if(!pSectionAP || !pSectionAP->getProperty("frame-page-xpos",pszPageXpos))
	{
		iXPage = convertInchToTwips(UT_convertToInches("0.0in"));
	}
	else
	{
		iXPage = convertInchToTwips(UT_convertToInches(pszPageXpos));
	}
	UT_DEBUGMSG(("PageXpos for frame is %s \n",pszPageXpos));
// PageYpos

	if(!pSectionAP || !pSectionAP->getProperty("frame-page-ypos",pszPageYpos))
	{
		iYPage = UT_convertToLogicalUnits("0.0in");
	}
	else
	{
		iYPage = UT_convertToLogicalUnits(pszPageYpos);
	}
	UT_DEBUGMSG(("PageYpos for frame is %s units %d \n",pszColYpos,iYPage));

// Width

	if(!pSectionAP || !pSectionAP->getProperty("frame-width",pszWidth))
	{
		iWidth = convertInchToTwips(UT_convertToInches("1.0in"));
	}
	else
	{
		iWidth = convertInchToTwips(UT_convertToInches(pszWidth));
	}

// Height

	if(!pSectionAP || !pSectionAP->getProperty("frame-height",pszHeight))
	{
		iHeight = convertInchToTwips(UT_convertToInches("1.0in"));
	}
	else
	{
		iHeight = convertInchToTwips(UT_convertToInches(pszHeight));
	}

// Xpadding


	if(!pSectionAP || !pSectionAP->getProperty("xpad",pszXpad))
	{
		iXpad = convertInchToTwips(UT_convertToInches("0.03in"));
	}
	else
	{
		iXpad = convertInchToTwips(UT_convertToInches(pszXpad));
	}


// Ypadding


	if(!pSectionAP || !pSectionAP->getProperty("ypad",pszYpad))
	{
		iYpad = convertInchToTwips(UT_convertToInches("0.03in"));
	}
	else
	{
		iYpad = convertInchToTwips(UT_convertToInches(pszYpad));
	}


	/* Frame-border properties:
	 */
	if(pSectionAP)
	{
		pSectionAP->getProperty ("color", pszColor);

		pSectionAP->getProperty ("bot-color",pszBorderColor);
		pSectionAP->getProperty ("bot-style",pszBorderStyle);
		pSectionAP->getProperty ("bot-thickness",pszBorderWidth);
	}

	s_border_properties (pszBorderColor, pszBorderStyle, pszBorderWidth, pszColor, botLine);

	pszBorderColor = NULL;
	pszBorderStyle = NULL;
	pszBorderWidth = NULL;

	if(pSectionAP)
	{
		pSectionAP->getProperty ("left-color", pszBorderColor);
		pSectionAP->getProperty ("left-style", pszBorderStyle);
		pSectionAP->getProperty ("left-thickness", pszBorderWidth);
	}

	s_border_properties (pszBorderColor, pszBorderStyle, pszBorderWidth, pszColor, leftLine);

	pszBorderColor = NULL;
	pszBorderStyle = NULL;
	pszBorderWidth = NULL;

	if(pSectionAP)
	{
		pSectionAP->getProperty ("right-color",pszBorderColor);
		pSectionAP->getProperty ("right-style",pszBorderStyle);
		pSectionAP->getProperty ("right-thickness", pszBorderWidth);
	}

	s_border_properties (pszBorderColor, pszBorderStyle, pszBorderWidth, pszColor, rightLine);

	pszBorderColor = NULL;
	pszBorderStyle = NULL;
	pszBorderWidth = NULL;

	if(pSectionAP)
	{
		pSectionAP->getProperty ("top-color",  pszBorderColor);
		pSectionAP->getProperty ("top-style",  pszBorderStyle);
		pSectionAP->getProperty ("top-thickness",pszBorderWidth);
	}

	s_border_properties (pszBorderColor, pszBorderStyle, pszBorderWidth, pszColor, topLine);

	/* Frame fill
	 */
	background.reset ();

	const gchar * pszBgStyle = NULL;
	const gchar * pszBgColor = NULL;
	const gchar * pszBackgroundColor = NULL;

	if(pSectionAP)
	{
		pSectionAP->getProperty ("bg-style",    pszBgStyle);
		pSectionAP->getProperty ("bgcolor",     pszBgColor);
		pSectionAP->getProperty ("background-color", pszBackgroundColor);
	}

	s_background_properties (pszBgStyle, pszBgColor, pszBackgroundColor, background);

	UT_uint32 kk = 0;
	std::string sFrameProps, esc;
	const gchar * szName = NULL;
	const gchar * szValue = NULL;
	while (pSectionAP->getNthProperty (kk++, szName, szValue))
	{
		if (kk != 1)
		{
			sFrameProps += "; ";
		}
		sFrameProps += szName; 
		sFrameProps += ":";
		sFrameProps += szValue;
	}
	
//
// OK got all the props of the frame.
//
	m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword("shp");
	m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword("*");
	m_pie->_rtf_keyword("shpinst");
	if( iFramePositionTo == FL_FRAME_POSITIONED_TO_BLOCK)
	{
		m_pie->_rtf_keyword("shpz",0); // All at z= 0;
		m_pie->_rtf_keyword("shpbxmargin");
		m_pie->_rtf_keyword("shpbypara"); // position relative to next paragraph
		if(iFrameWrapMode <= FL_FRAME_BELOW_TEXT)
		{
			m_pie->_rtf_keyword("shpwr",3); // no text wrapping
			m_pie->_rtf_keyword("shpfblwtxt",0); // text below frame
		}
		else if(iFrameWrapMode > FL_FRAME_BELOW_TEXT)
		{
			m_pie->_rtf_keyword("shpwr",2); // text wrapping
			m_pie->_rtf_keyword("shpwrk",0); // text wrap both sides
			m_pie->_rtf_keyword("shpfblwtxt",0); // text below frame
		}
	}
	else if( iFramePositionTo == FL_FRAME_POSITIONED_TO_COLUMN)
	{
		iXpos = iXColumn;
		iYpos = iYColumn;
		m_pie->_rtf_keyword("shpz",0); // All at z= 0;
		m_pie->_rtf_keyword("shpbxmargin");
		m_pie->_rtf_keyword("shpbymargin"); // position relative to margin
		if(iFrameWrapMode <= FL_FRAME_BELOW_TEXT)
		{
			m_pie->_rtf_keyword("shpwr",3); // no text wrapping
			m_pie->_rtf_keyword("shpfblwtxt",0); // text below frame
		}
		else if(iFrameWrapMode > FL_FRAME_BELOW_TEXT)
		{
			m_pie->_rtf_keyword("shpwr",2); // text wrapping
			m_pie->_rtf_keyword("shpwrk",0); // text wrap both sides
			m_pie->_rtf_keyword("shpfblwtxt",0); // text below frame
		}
	}
	else if( iFramePositionTo == FL_FRAME_POSITIONED_TO_PAGE)
	{
		iXpos = iXPage;
		iYpos = iYPage;
		m_pie->_rtf_keyword("shpz",0); // All at z= 0;
		m_pie->_rtf_keyword("shpbxmargin");
		m_pie->_rtf_keyword("shpbypage"); // position relative to page

		if(iFrameWrapMode <= FL_FRAME_BELOW_TEXT)
		{
			m_pie->_rtf_keyword("shpwr",3); // no text wrapping
			m_pie->_rtf_keyword("shpfblwtxt",0); // text below frame
		}
		else if(iFrameWrapMode > FL_FRAME_BELOW_TEXT)
		{
			m_pie->_rtf_keyword("shpwr",2); // text wrapping
			m_pie->_rtf_keyword("shpwrk",0); // text wrap both sides
			m_pie->_rtf_keyword("shpfblwtxt",0); // text below frame
		}
	}
	else
	{
		UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
		m_pie->_rtf_keyword("shpz",0); // All at z= 0;
		m_pie->_rtf_keyword("shpbypara");
		m_pie->_rtf_keyword("shpwr",3);
		m_pie->_rtf_keyword("shpbxmargin");
        m_pie->_rtf_keyword("shpfblwtxt",0);
	}
	m_pie->_rtf_keyword("shpleft",iXpos);
	m_pie->_rtf_keyword("shptop",iYpos);
	UT_sint32 iRight = iXpos + iWidth;
	UT_sint32 iBot = iYpos + iHeight;
	m_pie->_rtf_keyword("shpbottom",iBot);
	m_pie->_rtf_keyword("shpright",iRight);
	UT_uint32 lid = UT_rand();
	m_pie->_rtf_keyword("shplid",lid);


// OK Shape properties now

	if(iFrameType == FL_FRAME_TEXTBOX_TYPE)
	{
		_writeSPNumProp("shapeType",202); // Textbox

		if(background.m_t_background != PP_PropertyMap::background_none)
		{
			UT_RGBColor color = background.m_color;
			UT_sint32 iCol = color.m_red+color.m_grn*0x100+color.m_blu*0x10000;
			if(iCol != 0)
			{
				_writeSPNumProp("fillColor",iCol); // Background color
				_writeSPNumProp("fillType",0); // solid color
			}
		}
	}
	else
	{
//
// Image name
//
		const gchar * pszDataID = NULL;
		if(pSectionAP)
			pSectionAP->getAttribute(PT_STRUX_IMAGE_DATAID, pszDataID);
		if(pszDataID != NULL)
		{
			std::string mimetype;
			const UT_ByteBuf * pbb = NULL;
			bool bFoundDataItem = m_pDocument->getDataItemDataByName(pszDataID, 
                                                                     &pbb,
                                                                     &mimetype, 
                                                                     NULL);
			if (!bFoundDataItem)
			{
				UT_DEBUGMSG(("RTF_Export: cannot get dataitem for image\n"));
				return;
			}

			_writeSPNumProp("shapeType",75);  // Image

// OK the sp stuff for the image inside the "pib" tag.
//
			xxx_UT_DEBUGMSG(("export frame image braceLevel %d \n",m_pie->m_braceLevel));
			m_pie->_rtf_open_brace();
			m_pie->_rtf_keyword("sp");
			m_pie->_rtf_open_brace();
			m_pie->_rtf_keyword("sn ");
			m_pie->write("pib");
			m_pie->_rtf_close_brace();
			m_pie->_rtf_open_brace();
			m_pie->_rtf_keyword("sv ");


			m_pie->_rtf_open_brace();
			{
				m_pie->_rtf_keyword("pict");
				
				// get the width/height of the image from the image itself.

				UT_sint32 iImageWidth, iImageHeight;

				if(mimetype == "image/png") 
				{
					m_pie->_rtf_keyword("pngblip");
					UT_PNG_getDimensions(pbb,iImageWidth,iImageHeight);
				}
				else if(mimetype == "image/jpeg") 
				{
					m_pie->_rtf_keyword("jpegblip");
					UT_JPEG_getDimensions(pbb,iImageWidth,iImageHeight);
				}
				else if (mimetype == "image/svg+xml")
				{
					m_pie->_rtf_keyword("svgblip");
					UT_sint32 layoutwidth,layoutheight;
					UT_SVG_getDimensions(pbb,NULL,iImageWidth,iImageHeight,layoutwidth,layoutheight);
				}
				// compute scale factors...

				double dImageWidth = static_cast<double>(iImageWidth);
				double dImageHeight = static_cast<double>(iImageHeight);
				dImageWidth = UT_convertDimToInches(dImageWidth,DIM_PT);
				dImageHeight = UT_convertDimToInches(dImageHeight,DIM_PT);

				
				// <pictsize>

				m_pie->_rtf_keyword("picw",iImageWidth);
				m_pie->_rtf_keyword("pich",iImageHeight);

				m_pie->_rtf_keyword("picwgoal",iWidth);
				double dWidth = static_cast<double>(iWidth)/1440.;
				double scalex = 100.0*dWidth/dImageWidth;
				UT_uint32 iscalex = static_cast<UT_uint32>(scalex);
				m_pie->_rtf_keyword("picscalex",iscalex);
					
				m_pie->_rtf_keyword("pichgoal",iHeight);
				double dHeight = static_cast<double>(iHeight)/1440.;
				double scaley = 100.0*dHeight/dImageHeight;
				UT_uint32 iscaley = static_cast<UT_uint32>(scaley);
				m_pie->_rtf_keyword("picscaley",iscaley);

				// TODO deal with <metafileinfo>
				
				// <data>

				// TODO create meaningful values for bliptag and bliduid...
				// we emit "\bliptag<N>{\*\blipuid <N16>}"
				// where <N> is an integer.
				// where <N16> is a 16-byte integer in hex.

				m_pie->_rtf_nl();
				UT_uint32 tag = UT_newNumber ();
				m_pie->_rtf_keyword("bliptag",tag);
				m_pie->_rtf_open_brace();
				{
					m_pie->_rtf_keyword("*");
					m_pie->_rtf_keyword("blipuid");
					UT_String buf;
					UT_String_sprintf(buf,"%032x",tag);
					m_pie->_rtf_chardata(buf.c_str(),buf.size());
				}
				m_pie->_rtf_close_brace();
			}

			UT_uint32 lenData = pbb->getLength();
			const UT_Byte * pData = pbb->getPointer(0);
			UT_uint32 k;

			for (k=0; k<lenData; k++)
			{
				if (k%32==0)
					m_pie->_rtf_nl();
				UT_String buf;
				UT_String_sprintf(buf,"%02x",pData[k]);
				m_pie->_rtf_chardata(buf.c_str(),2);
			}
			m_pie->_rtf_close_brace();  // close pict
			m_pie->_rtf_close_brace(); // close sv
			m_pie->_rtf_close_brace(); // close sp
			xxx_UT_DEBUGMSG(("finish export frame image braceLevel %d \n",m_pie->m_braceLevel));
		}

	}

	_writeSPNumProp("dxTextLeft",convertTwipsToEMU(iXpad));
	_writeSPNumProp("dxTextRight",convertTwipsToEMU(iXpad));
	_writeSPNumProp("dxTextTop",convertTwipsToEMU(iYpad));
	_writeSPNumProp("dxTextBottom",convertTwipsToEMU(iYpad));

	// Print the abiword props string; this is for the internal copy/paste
	m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword("abiframeprops ",sFrameProps.c_str());
	m_pie->_rtf_close_brace();

	m_bTextBox = false;

	if(iFrameType == FL_FRAME_TEXTBOX_TYPE)
	{
		m_pie->_rtf_open_brace();
        m_pie->_rtf_keyword("shptxt"); // Is a text box
		m_bTextBox = true;
	}
	m_bInSpan = false;
	m_bJustOpennedFrame = true;
}



void s_RTF_ListenerWriteDoc::_closeFrame(void)
{
	if(!m_bInFrame) // Can happen dragging frames around
	{
		return;
	}
	m_pie->_rtf_close_brace();
	m_pie->_rtf_close_brace();
	if(m_bTextBox)
	{
		m_pie->_rtf_close_brace();
	}
	m_bInFrame = false;
	m_bJustOpennedFrame = false;
}

void s_RTF_ListenerWriteDoc::_openSpan(PT_AttrPropIndex apiSpan,  const PP_AttrProp * pInSpanAP)
{
	if (m_bInSpan)
	{
		if (m_apiLastSpan == apiSpan)
			return;
		_closeSpan();
	}

	m_pie->_rtf_open_brace();

	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;
    
    const PP_AttrProp * pDeepestAP = NULL; // the one that is the most local
    
    bool bHaveSpanProps = 0;
	bool bHaveSectionProps = m_pDocument->getAttrProp(m_apiThisSection,&pSectionAP);
    bool bHaveBlockProps = m_pDocument->getAttrProp(m_apiThisBlock,&pBlockAP);
	if(pInSpanAP == NULL)
	{
        bHaveSpanProps = m_pDocument->getAttrProp(apiSpan,&pSpanAP);
	}
	else
	{
		pSpanAP = pInSpanAP;
	}

    if (bHaveSpanProps && (0 != pSpanAP)) 
	{
        pDeepestAP = pSpanAP;
    }
    else if (bHaveBlockProps  && (0 != pBlockAP)) 
	{
        pDeepestAP = pBlockAP;
    }
    else if (bHaveSectionProps && (0 != pSectionAP)) 
	{
        pDeepestAP = pSectionAP;
    }
    
    if (NULL != pDeepestAP) 
	{
		const gchar * styleSzValue = 0;
		bool have_style  = pDeepestAP->getAttribute (PT_STYLE_ATTRIBUTE_NAME, 
													 styleSzValue);
        if (!have_style) 
		{
            if (bHaveBlockProps && (0 != pBlockAP)) 
			{
                have_style = pBlockAP->getAttribute (PT_STYLE_ATTRIBUTE_NAME, 
													 styleSzValue);
            }
        }
        if (!have_style) 
		{
            if (bHaveSectionProps && (0 != pSectionAP)) 
			{
                have_style = pSectionAP->getAttribute (PT_STYLE_ATTRIBUTE_NAME, 													   styleSzValue);
            }
        }
        
        if (have_style) 
		{
            int styleID = m_pie->_getStyleNumber(styleSzValue);
            const char* styleType = "s";
            
            //get the style from the styleName
            PD_Style* pStyle = NULL;
			m_pDocument->getStyle(styleSzValue,&pStyle);
            UT_ASSERT_HARMLESS(pStyle);
            if (pStyle && pStyle->isCharStyle()) 
			{
                styleType = "cs";
            }
            m_pie->_rtf_keyword(styleType, styleID);
        }
    }
   
	m_pie->_write_charfmt(s_RTF_AttrPropAdapter_AP(pSpanAP, pBlockAP, pSectionAP, m_pDocument));
	m_bBlankLine = false;
	m_bInSpan = true;
	m_apiLastSpan = apiSpan;


	// export the generated ODT+CT move identifier
    if ( pBlockAP )
	{
		const gchar * szDeltaMoveID = 0;
		bool haveAttr = pBlockAP->getAttribute("delta:move-id", szDeltaMoveID );
		if ( haveAttr )
		{
			UT_DEBUGMSG(("DOM: szDeltaMoveID = %s\n", szDeltaMoveID ));
			m_pie->_rtf_open_brace();
			m_pie->_rtf_keyword("*");
			m_pie->_rtf_keyword("deltamoveid" );
			m_pie->_rtf_chardata( szDeltaMoveID, strlen(szDeltaMoveID) );
			m_pie->_rtf_close_brace();
		}
		else
		{
			UT_DEBUGMSG(("DOM: szDeltaMoveID = %s\n", "NOT FOUND..." ));
		}
	}
#ifdef DEBUG
	{
		const gchar * t = 0;
		if( pBlockAP )
		{
			if( pBlockAP->getAttribute("baz", t ) )
			{
				UT_DEBUGMSG(("DOM: b have baz value:%s\n", t ));
			}
		}
		if(pBlockAP)
			UT_DEBUGMSG(("DOM: b have baz:%d\n", pBlockAP->getAttribute("baz", t )));
		if(pSpanAP)
			UT_DEBUGMSG(("DOM: s have baz:%d\n", pSpanAP->getAttribute("baz", t )));
		if(pBlockAP)
			UT_DEBUGMSG(("DOM: b have baz2:%d\n", pBlockAP->getAttribute("baz2", t )));
		if(pSpanAP)
			UT_DEBUGMSG(("DOM: s have baz2:%d\n", pSpanAP->getAttribute("baz2", t )));
		
	}
#endif
	
}

void s_RTF_ListenerWriteDoc::_outputData(const UT_UCSChar * data, UT_uint32 length, PT_DocPosition pos, bool bIgnorePosition)
{
	UT_String sBuf;
	const UT_UCSChar * pData;
	char mbbuf[30];
	int mblen;

	#define FlushBuffer() do {m_pie->_rtf_chardata(sBuf.c_str(), sBuf.size()); sBuf.clear();} while (0)

	UT_return_if_fail(sizeof(UT_Byte) == sizeof(char));

	for (pData=data; (pData<data+length); /**/)
	{
		// first handle direciton issues
		UT_BidiCharType type = UT_BIDI_LTR;

		if(  !bIgnorePosition
		   && m_pDocument->exportGetVisDirectionAtPos(pos + (pData - data),type)
		  )
		{
			if(m_pie->isCharRTL() != UT_BIDI_LTR && !FRIBIDI_IS_RTL(type))
			{
				// changing from rtl to ltr
				FlushBuffer();

// when reading this rtf back into AW, we do not want
// the ltrch converted into an override -- issue
// custom abinodiroverride keyword

				m_pie->_rtf_keyword("abinodiroverride"); 
				m_pie->_rtf_keyword("ltrch");
				m_pie->setCharRTL(UT_BIDI_LTR);
			}
			else if(m_pie->isCharRTL() != UT_BIDI_RTL && FRIBIDI_IS_RTL(type))
			{
				// changing from ltr to rtl
				FlushBuffer();

// when reading this rtf back into AW, we do not want
// the ltrch converted into an override -- issue
// custom abinodiroverride keyword

				m_pie->_rtf_keyword("abinodiroverride"); 
				m_pie->_rtf_keyword("rtlch");
				m_pie->setCharRTL(UT_BIDI_RTL);
			}
	   }
			
		switch (*pData)
		{
		case '\\':
		case '{':
		case '}':
			sBuf += '\\';
			sBuf += (char)*pData++;
			break;

		case UCS_LF:					// LF -- representing a Forced-Line-Break
			FlushBuffer();
			m_pie->_rtf_keyword("line");
			pData++;
			break;

		case UCS_VTAB:					// VTAB -- representing a Forced-Column-Break
			FlushBuffer();
			m_pie->_rtf_keyword("column");
			pData++;
			break;

		case UCS_FF:					// FF -- representing a Forced-Page-Break
			FlushBuffer();
			m_pie->_rtf_keyword("page");
			pData++;
			break;

		case UCS_NBSP:					// NBSP -- non breaking space
			FlushBuffer();
			m_pie->_rtf_keyword("~");
			m_pie->m_bLastWasKeyword = false;       // no space needed afterward
			
			pData++;
			break;

		case UCS_TAB:					// TAB -- a tab
			FlushBuffer();
			m_pie->_rtf_keyword("tab");
			pData++;
			break;

		default:
			// remove supperfluous direction markers ...
			if(*pData == UCS_LRM && m_pie->isCharRTL() == UT_BIDI_LTR)
			{
				pData++;
				continue;
			}
			else if(*pData == UCS_RLM && m_pie->isCharRTL() == UT_BIDI_RTL)
			{
				pData++;
				continue;
			}
		
			
			if (XAP_EncodingManager::get_instance()->cjk_locale())
			{
				/*FIXME: can it happen that wctomb will fail under CJK locales? */
				m_wctomb.wctomb_or_fallback(mbbuf,mblen,*pData++);
				if (mbbuf[0] & 0x80)
				{
					FlushBuffer();
					for(int i=0;i<mblen;++i) {
						unsigned char c = mbbuf[i];
						m_pie->_rtf_nonascii_hex2(c);
					}
				}
				else
				{
					for(int i=0;i<mblen;++i) {
						switch (mbbuf[i])
						{
							case '\\':
							case '{':
							case '}':
								sBuf += '\\';
						}
						sBuf += mbbuf[i];
					}
				}
			} else if (!m_pie->m_atticFormat)
			{
				if (*pData > 0xffff) {
					m_pie->_rtf_keyword("uc", 1);
					UT_UCS4Char ch = *pData - 0x10000;
					short si;
					si = (ch >> 10 & 0x3ff) + 0xD800;
					m_pie->_rtf_keyword("u",si);
					m_pie->_rtf_nonascii_hex2('?');
					si =  (ch & 0x3ff) + 0xDC00;
					m_pie->_rtf_keyword("u",si);
					m_pie->_rtf_nonascii_hex2('?');
					pData++;
				}
				else if (*pData > 0x00ff)		// emit unicode character
				{
					FlushBuffer();

					// RTF spec says that we should emit an ASCII-equivalent
					// character for each unicode character, so that dumb/older
					// readers don't lose a char.  i don't have a good algorithm
					// for deciding how to do this, so i'm not going to put out
					// any chars.  so i'm setting \uc0 before emitting \u<u>.
					// TODO decide if we should be smarter here and do a \uc1\u<u><A> ??
					// TODO if so, we may need to begin a sub-brace level to avoid
					// TODO polluting the global context w/r/t \uc.

					UT_UCSChar lc = XAP_EncodingManager::get_instance()->try_UToWindows(*pData);
					m_pie->_rtf_keyword("uc",lc && lc<256 ? 1 : 0);
					unsigned short ui = ((unsigned short)(*pData));	// RTF is limited to +/-32K ints
					signed short si = *((signed short *)(&ui));		// so we need to write negative
					m_pie->_rtf_keyword("u",si);					// numbers for large unicode values.
					if (lc && lc <256)
						m_pie->_rtf_nonascii_hex2(lc);
					pData++;
				}
				else if (*pData > 0x007f)
				{
					FlushBuffer();

					// for chars between 7f and ff, we could just send them
					// out as is, or we could send them out in hex or as a
					// unicode sequence.  when i originally did this, i chose
					// hex, so i'm not going to change it now.

					m_pie->_rtf_nonascii_hex2(*pData);
					pData++;
				}
				else
				{
					sBuf += (char)*pData++;
				}
			} else {
				/*
				    wordpad (and probably word6/7) don't understand
				    \uc0\u<UUUU> format at all.
				*/
				UT_UCSChar c = *pData++;
				UT_UCSChar lc = XAP_EncodingManager::get_instance()->try_UToWindows(c);
				if (lc==0 || lc >255)
				{
					/*
					    can't be represented in windows encoding.
					    So emit unicode (though attic apps won't understand it.
					    This branch is shamelessly copied from
					    branch if (*pData > 0x00ff) above.
					*/
					FlushBuffer();

					// RTF spec says that we should emit an ASCII-equivalent
					// character for each unicode character, so that dumb/older
					// readers don't lose a char.  i don't have a good algorithm
					// for deciding how to do this, so i'm not going to put out
					// any chars.  so i'm setting \uc0 before emitting \u<u>.
					// TODO decide if we should be smarter here and do a \uc1\u<u><A> ??
					// TODO if so, we may need to begin a sub-brace level to avoid
					// TODO polluting the global context w/r/t \uc.

					m_pie->_rtf_keyword("uc",0);
					unsigned short ui = ((unsigned short)(*pData));	// RTF is limited to +/-32K ints
					signed short si = *((signed short *)(&ui));		// so we need to write negative
					m_pie->_rtf_keyword("u",si);					// numbers for large unicode values.
				}
				else
				{
					if (lc > 0x007f)
					{
						FlushBuffer();

						// for chars between 7f and ff, we could just send them
						// out as is, or we could send them out in hex or as a
						// unicode sequence.  when i originally did this, i chose
						// hex, so i'm not going to change it now.

						m_pie->_rtf_nonascii_hex2(lc);
					}
					else
					{
						sBuf += static_cast<char>(lc);
					}
				}
			};
			break;
		}
	}

	FlushBuffer();
}

s_RTF_ListenerWriteDoc::s_RTF_ListenerWriteDoc(PD_Document * pDocument,
											   IE_Exp_RTF * pie,
											   bool bToClipboard, 
											   bool bHasMultiBlock)
  : m_wctomb(XAP_EncodingManager::get_instance()->getNative8BitEncodingName())
{
	// The overall syntax for an RTF file is:
	//
	// <file> := '{' <header> <document> '}'
	//
	// We are responsible for <document>
	//
	// <document> := <info>? <docfmt>* <section>+

	m_pDocument = pDocument;
	m_pie = pie;
	m_bInSpan = false;
	m_bInBlock = false;
	m_apiLastSpan = 0;
	m_apiThisSection = 0;
	m_apiThisBlock = 0;
	m_apiThisFrame = 0;
	m_bInFrame = false;
	m_bJustOpennedFrame = false;
	m_sdh = NULL;
	m_bToClipboard = bToClipboard;
	m_bStartedList = false;
	m_bBlankLine = true;
	m_Table.setDoc(m_pDocument);
	m_iCurRow = -1;
	m_bNewTable = false;
	m_iLeft = -1;
	m_iRight = -1;
	m_iTop = -1;
	m_iBot = -1;
	m_LastLinestyle = PP_PropertyMap::linestyle_solid;
	m_sLastColor = "000000";
	_setTabEaten(false);
	_setListBlock(false);

	// when we are going to the clipboard, we should implicitly
	// assume that we are starting in the middle of a section
	// and block.  when going to a file we should not.
	m_bJustStartingDoc = !m_bToClipboard;
	m_bJustStartingSection = !m_bToClipboard;

	m_wctomb.setOutCharset(XAP_EncodingManager::get_instance()->WindowsCharsetName());
	// TODO emit <info> if desired
	m_currID = 0;
	_rtf_info ();
	_rtf_docfmt();						// deal with <docfmt>
	m_apiSavedBlock = 0;
	m_sdhSavedBlock = NULL;
	m_bOpennedFootnote = false;
	m_iFirstTop = 0;
	m_bHyperLinkOpen = false;
	m_bRDFAnchorOpen = false;
	m_bAnnotationOpen = false;
	m_iAnnotationNumber = 0;
	m_pAnnContent = NULL;
	m_pSavedBuf = NULL;
	m_bOpenBlockForSpan = bHasMultiBlock;
	m_bTextBox = false;
	// <section>+ will be handled by the populate code.
}

s_RTF_ListenerWriteDoc::~s_RTF_ListenerWriteDoc()
{
	UT_DEBUGMSG(("~s_RTF_ListenerWriteDoc() rdfstack.sz:%lu\n" , (long unsigned)m_rdfAnchorStack.size() ));

	_closeSpan();

	// for( std::list< std::string >::iterator iter = m_rdfAnchorStack.begin();
	// 	 iter != m_rdfAnchorStack.end(); ++iter )
	// {
	// 	std::string xmlid = *iter;

	// 	m_pie->_rtf_open_brace();
	// 	m_pie->_rtf_keyword("*");
	// 	UT_DEBUGMSG(("_writeRDFAnchor(dtor) end... id:%s\n", xmlid.c_str() ));
	// 	m_pie->_rtf_keyword("rdfanchorend");
	// 	m_pie->_rtf_chardata( xmlid.c_str(), xmlid.length());
	// 	m_pie->_rtf_close_brace();
	// }
	
	_closeBlock();
	_closeSection();
}

bool s_RTF_ListenerWriteDoc::populate(fl_ContainerLayout* /*sfh*/,
									  const PX_ChangeRecord * pcr)
{
	m_posDoc = pcr->getPosition();
	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

			PT_AttrPropIndex api = pcr->getIndexAP();

			PT_BufIndex bi = pcrs->getBufIndex();
			const UT_UCSChar * pData = m_pDocument->getPointer(bi);
//
// Code to deal with the tab following a list label. Eat it!!
//
			UT_uint32 length = pcrs->getLength();
			if(_isListBlock() && !_isTabEaten())
			{
				if(*pData == UCS_TAB)
				{
					_setTabEaten(true);
					pData++;
					length--;
					if(length == 0)
						return true;
				}
			}

			if (m_bOpenBlockForSpan)
			{
				m_bOpenBlockForSpan = false;

				pf_Frag * pf1 = m_pDocument->getFragFromPosition(pcr->getPosition());
				if (pf1 != NULL)
				{
					// scan backwards for the block props of this span.
					while (pf1 != NULL)
					{
						if (pf1->getType() == pf_Frag::PFT_Strux)
						{
							m_apiThisBlock = ((pf_Frag_Strux*)pf1)->getIndexAP();
							break;
						}
						pf1 = pf1->getPrev();
					}
				}
			}

			_openSpan(api);
			_outputData(pData,length,pcr->getPosition(),false);

			return true;
		}

	case PX_ChangeRecord::PXT_InsertObject:
		{
			const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);
			PT_AttrPropIndex api = pcr->getIndexAP();
			switch (pcro->getObjectType())
			{
			case PTO_Image:
				_closeSpan();
				_writeImageInRTF(pcro);
				return true;

				//#if 0
			// TODO deal with these other inline objects....

			case PTO_Field:
				_closeSpan();
				_openTag("field","/",false,api);
				return true;

			case PTO_Math:
				_closeSpan();
				_openTag("math","/",false,api);
				return true;

			case PTO_Embed:
				_closeSpan();
				_openTag("embed","/",false,api);
				return true;

				//#endif

			case PTO_Bookmark:
				_closeSpan ();
				_writeBookmark(pcro);
				return true;
			case PTO_Hyperlink:
			{
				UT_DEBUGMSG(("PTO_Hyperlink pcro:%p\n", pcro ));
				_closeSpan ();
				const PP_AttrProp * pAP = NULL;
				m_pDocument->getAttrProp(api,&pAP);
				const gchar * pName;
				const gchar * pValue;
				bool bFound = false;
				UT_uint32 k = 0;
				while(pAP->getNthAttribute(k++, pName, pValue))
				{
					bFound = (0 == g_ascii_strncasecmp(pName,"xlink:href",10));
					if(bFound)
						break;
				}
				if(bFound)
				{
					//this is the start of the hyperlink
					_writeHyperlink(pcro);
				}
				else
				{
//
// This is the end of hyperlink marker, signified by no xlink::href tag
//
					m_bHyperLinkOpen = false;
					m_pie->_rtf_close_brace();
					m_pie->_rtf_close_brace();
				}
			    return true;
			}
			case PTO_Annotation:
			{
				_closeSpan ();
				const PP_AttrProp * pAP = NULL;
				m_pDocument->getAttrProp(api,&pAP);
				const gchar * pName;
				const gchar * pValue;
				bool bFound = false;
				UT_uint32 k = 0;
				while(pAP && pAP->getNthAttribute(k++, pName, pValue))
				{
					bFound = (0 == g_ascii_strncasecmp(pName,"annotation",10));
					if(bFound)
						break;
				}
				if(bFound)
				{
					//this is the start of the Annotation
					_writeAnnotation(pcro);
				}
				else
				{
					UT_return_val_if_fail(m_pAnnContent, true);
//
// This is the end of the Annotation marker, signified by no Annotation tag
//
					m_bAnnotationOpen = false;
					m_pie->_rtf_open_brace();
					m_pie->_rtf_keyword("*");
					m_pie->_rtf_keyword_space("atrfend",m_iAnnotationNumber);
					m_pie->_rtf_close_brace();
//
// Now assemble all the stuff for the annotation content and write it to the
// stream
//
					m_pie->_rtf_open_brace();
					m_pie->_rtf_keyword("*");
					m_pie->_rtf_keyword("atnauthor",m_sAnnAuthor.utf8_str());
					m_pie->_rtf_close_brace();
					m_pie->_rtf_keyword("chatn");

					m_pie->_rtf_open_brace();
					m_pie->_rtf_keyword("*");
					m_pie->_rtf_keyword("annotation");

					m_pie->_rtf_open_brace();
					m_pie->_rtf_keyword("*");
					m_pie->_rtf_keyword_space("atnref",m_iAnnotationNumber);
					m_pie->_rtf_close_brace();

					m_pie->_rtf_open_brace();
					m_pie->_rtf_keyword("*");
					m_pie->_rtf_keyword("atndate",m_sAnnDate.utf8_str());
					m_pie->_rtf_close_brace();
					m_pie->write(reinterpret_cast<const char *>(m_pAnnContent->getPointer(0)),m_pAnnContent->getLength());
					DELETEP(m_pAnnContent);
					m_pie->_rtf_close_brace();
					m_pie->_rtf_close_brace();

			}
			    return true;
			}
			case PTO_RDFAnchor:
			{
				_closeSpan ();
				_writeRDFAnchor(pcro);
				return true;
			}	
			default:
				return false;
			}
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return true;

	default:
		UT_ASSERT_NOT_REACHED();
		return false;
	}
}

/*!
 * This method writes out the all the boiler plate needed before every field
 * definition.
 */
void s_RTF_ListenerWriteDoc::_writeFieldPreamble(const PP_AttrProp * pSpanAP)
{
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;

	m_pDocument->getAttrProp(m_apiThisSection,&pSectionAP);
	m_pDocument->getAttrProp(m_apiThisBlock,&pBlockAP);
	m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword("field");
	m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword("*");
	m_pie->_rtf_keyword("fldinst");
	m_pie->write(" ");
	m_pie->_rtf_open_brace();
	m_pie->_write_charfmt(s_RTF_AttrPropAdapter_AP(pSpanAP, pBlockAP, pSectionAP, m_pDocument));
	m_pie->write(" ");
}


/*!
 * This method writes out the current field value and closes all the braces.
 */
void s_RTF_ListenerWriteDoc::_writeFieldTrailer(void)
{
	const UT_UCSChar * szFieldValue = _getFieldValue();
	if(szFieldValue == NULL)
	{
		m_pie->_rtf_close_brace();
		return;
	}
	m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword("fldrslt");
	m_pie->write(" ");
	m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword("noproof");
	m_pie->write(" ");
	UT_uint32 len = UT_UCS4_strlen(szFieldValue);
	_outputData(szFieldValue,len,0,true);
	m_pie->_rtf_close_brace();
	m_pie->_rtf_close_brace();
	m_pie->_rtf_close_brace();
}

/*!
 * This method returns the field value at the current document location.
 * If there is not a field at the current document location return NULL.
 */
const UT_UCSChar * s_RTF_ListenerWriteDoc::_getFieldValue(void)
{
//
// Grab the first format handle in the PieceTable and turn it into a layout class.
// Check that is it a block.
//
	if(m_sdh == NULL)
	{
		m_pDocument->getStruxOfTypeFromPosition(m_posDoc,PTX_Block,&m_sdh);
	}
	fl_ContainerLayout* sfh = m_pDocument->getNthFmtHandle(m_sdh,0);
	const fl_Layout * pL = reinterpret_cast<const fl_Layout *>(sfh);
	UT_return_val_if_fail(pL,NULL);
	if(pL && pL->getType() != PTX_Block)
	{
	  UT_return_val_if_fail(0, NULL);
	}
	const fl_BlockLayout* pBL = static_cast<const fl_BlockLayout *>(pL);
	bool bDirection;
	UT_sint32 x, y, x2, y2, height;
	fp_Run * pRun = pBL->findPointCoords(m_posDoc,false,x,y,x2,y2,height,bDirection);
//
// Check the run to make sure it is a field.
//
	while(pRun && pRun->getType() == FPRUN_FMTMARK)
	{
		pRun = pRun->getNextRun();
	}
	if((pRun== NULL) || pRun->getType() != FPRUN_FIELD )
	{
	  UT_return_val_if_fail(0, NULL);
	}
//
// Now get the value of this field
//
	return static_cast<fp_FieldRun *>(pRun)->getValue();
}

void	 s_RTF_ListenerWriteDoc::_openTag(const char * szPrefix, const char * szSuffix,
										  bool /*bNewLineAfter*/, PT_AttrPropIndex api)
{
	UT_UNUSED(szSuffix);
	 xxx_UT_DEBUGMSG(("TODO: Write code to go in here. In _openTag, szPrefix = %s  szSuffix = %s api = %x \n",szPrefix,szSuffix,api));
	 if(strcmp(szPrefix,"field") == 0)
	 {
		 const PP_AttrProp * pSpanAP = NULL;
		 const gchar * pszType = NULL;
		 m_pDocument->getAttrProp(api, &pSpanAP);
		 pSpanAP->getAttribute("type", pszType);
		 if(pszType == NULL)
		 {
			 return;
		 }
		 if(strcmp(pszType,"list_label") == 0)
		 {
			 return;
		 }

		 if(strcmp(pszType,"footnote_ref") == 0)
		 {
			 _openSpan(api,pSpanAP);
			 m_pie->_rtf_keyword("chftn");
			 return;
		 }
		 else if(strcmp(pszType,"footnote_anchor") == 0)
		 {
			 _openSpan(api,pSpanAP);
			 m_pie->_rtf_keyword("chftn");
			 return;
		 }
		 else if(strcmp(pszType,"endnote_ref") == 0)
		 {
			 _openSpan(api,pSpanAP);
			 m_pie->_rtf_keyword("chftn");
			 return;
		 }
		 else if(strcmp(pszType,"endnote_anchor") == 0)
		 {
			 _openSpan(api,pSpanAP);
			 m_pie->_rtf_keyword("chftn");
			 return;
		 }
		 else if(strcmp(pszType,"page_number") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("PAGE ");
			 m_pie->_rtf_close_brace();
			 m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(strcmp(pszType,"time") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("TIME ");
			 m_pie->_rtf_close_brace();
			 m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(strcmp(pszType,"page_ref") == 0)
		 {
                         m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDpageDref"); // abiword extension for now.
             m_pie->_rtf_close_brace();
			 return;
		 }
		 else if(strcmp(pszType,"page_count") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("NUMPAGES ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 return;
		 }
		 else if(strcmp(pszType,"date") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("TIME  \\");
			 m_pie->_rtf_keyword("@");
			 m_pie->write(" \"dddd, MMMM dd, yyyy\" ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(strcmp(pszType,"date_mmddyy") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("DATE ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(strcmp(pszType,"date_ddmmyy") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("TIME \\");
			 m_pie->_rtf_keyword("@");
			 m_pie->write(" ""m/d/yy"" ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(strcmp(pszType,"date_mdy") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("TIME  \\");
			 m_pie->_rtf_keyword("@");
			 m_pie->write(" \"MMMM d, yyyy\" ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(strcmp(pszType,"date_mthdy") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("TIME  \\");
			 m_pie->_rtf_keyword("@");
			 m_pie->write(" \"MMM d, yy\" ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(strcmp(pszType,"date_dfl") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("SAVEDATE  ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(strcmp(pszType,"date_ntdfl") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("TIME  \\");
			 m_pie->_rtf_keyword("@");
			 m_pie->write(" \"MM-d-yy\" ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(strcmp(pszType,"date_wkday") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("TIME  \\");
			 m_pie->_rtf_keyword("@");
			 m_pie->write(" \"dddd\" ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(strcmp(pszType,"date_doy") == 0)
		 {
             m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDdateDdoy"); // abiword extension for now.
             m_pie->_rtf_close_brace();
			 return;
		 }
		 else if(strcmp(pszType,"time_miltime") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("TIME  \\");
			 m_pie->_rtf_keyword("@");
			 m_pie->write(" \"HH:mm:ss\" ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(strcmp(pszType,"time_ampm") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("TIME  \\");
			 m_pie->_rtf_keyword("@");
			 m_pie->write(" \"h:mm:ss am/pm\" ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(strcmp(pszType,"time_zone") == 0)
		 {
             m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDtimeDzone"); // abiword extension for now.
             m_pie->_rtf_close_brace();
			 return;
		 }
		 else if(strcmp(pszType,"time_epoch") == 0)
		 {
             m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDtimeDepoch"); // abiword extension for now.
             m_pie->_rtf_close_brace();
			 return;
		 }
		 else if(strcmp(pszType,"word_count") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("NUMWORDS ");
			 m_pie->_rtf_close_brace();
			 m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(strcmp(pszType,"char_count") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
  			 m_pie->write("NUMCHARS  ");
			 m_pie->_rtf_close_brace();
			 m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(strcmp(pszType,"line_count") == 0)
		 {
		         m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDlineDcount"); // abiword extension for now.
             m_pie->_rtf_close_brace();
			 return;
		 }
		 else if(strcmp(pszType,"para_count") == 0)
		 {
                         m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDparaDcount"); // abiword extension for now.
             m_pie->_rtf_close_brace();
			 UT_DEBUGMSG(("SEVIOR: paragraph count field here \n"));
			 return;
		 }
		 else if(strcmp(pszType,"nbsp_count") == 0)
		 {
             m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDnbspDcount"); // abiword extension for now.
             m_pie->_rtf_close_brace();
			 return;
		 }
		 else if(strcmp(pszType,"file_name") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("FILENAME ");
			 m_pie->_rtf_close_brace();
			 m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(strcmp(pszType,"app_ver") == 0)
		 {
                         m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDappDver"); // abiword extension for now.
			 m_pie->_rtf_close_brace();
			 return;
		 }
		 else if(strcmp(pszType,"app_id") == 0)
		 {
		         m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDappDid"); // abiword extension for now.
                         m_pie->_rtf_close_brace();
			 UT_DEBUGMSG(("SEVIOR: Application ID field here \n"));
			 return;
		 }
		 else if(strcmp(pszType,"app_options") == 0)
		 {
                         m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDappDoptions"); // abiword extension for now.
			 m_pie->_rtf_close_brace();
			 return;
		 }
		 else if(strcmp(pszType,"app_target") == 0)
		 {
             m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDappDtarget"); // abiword extension for now.
             m_pie->_rtf_close_brace();
			 return;
		 }
		 else if(strcmp(pszType,"app_compiledate") == 0)
		 {
		         m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDappDcompiledate"); // abiword extension for now.
                         m_pie->_rtf_close_brace();
			 return;
		 }
		 else if(strcmp(pszType,"app_compiletime") == 0)
		 {
		         m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDappDcompiletime"); // abiword extension for now.
			 m_pie->_rtf_close_brace();
			 return;
		 }
		 else if(strcmp(pszType,"meta_creator") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
  			 m_pie->write("AUTHOR ");
			 m_pie->_rtf_close_brace();
			 m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(strcmp(pszType,"meta_date") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
  			 m_pie->write("CREATEDATE ");
			 m_pie->_rtf_close_brace();
			 m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(strcmp(pszType,"meta_description") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
  			 m_pie->write("COMMENTS ");
			 m_pie->_rtf_close_brace();
			 m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(strcmp(pszType,"meta_keywords") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
  			 m_pie->write("KEYWORDS ");
			 m_pie->_rtf_close_brace();
			 m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(strcmp(pszType,"meta_title") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
  			 m_pie->write("TITLE ");
			 m_pie->_rtf_close_brace();
			 m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else
		 {
			 UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			 return;
		 }
	 }
	 else if(strcmp(szPrefix,"math") == 0)
	 {
		 const PP_AttrProp * pSpanAP = NULL;
		 const gchar * pszDataId = NULL;
		 const gchar * pszLatexId = NULL;
		 m_pDocument->getAttrProp(api, &pSpanAP);
		 pSpanAP->getAttribute("dataid", pszDataId);
		 pSpanAP->getAttribute("latexid", pszLatexId);
		 UT_UTF8String sProps;
		 if(pszDataId == NULL)
		 {
			 return;
		 }
		 //
		 // Export the MathML associated with this
		 //
		 const UT_ByteBuf * pbb = NULL;
		 bool bFoundDataItem = false;
		 UT_uint32 lenData = 0;
		 const UT_Byte * pData = NULL;
		 UT_uint32 k = 0;
		 UT_String buf;
		 if(pszDataId)
		 {
			bFoundDataItem = m_pDocument->getDataItemDataByName(static_cast<const char*>(pszDataId),
                                                                &pbb,
                                                                NULL,
                                                                NULL);
			if (!bFoundDataItem)
			{

				UT_DEBUGMSG(("RTF_Export: cannot get dataitem for math\n"));
				return;
			}
		    m_pie->_rtf_open_brace();
			m_pie->_rtf_keyword("*");
			m_pie->_rtf_keyword("abimathmldata ");
			buf = pszDataId;
			buf += " ";
			m_pie->_rtf_chardata(buf.c_str(),buf.size());

			lenData = pbb->getLength();
			pData = pbb->getPointer(0);
			for (k=0; k<lenData; k++)
			{
				if (k%32==0)
					m_pie->_rtf_nl();
				UT_String_sprintf(buf,"%02x",pData[k]);
				m_pie->_rtf_chardata(buf.c_str(),2);
			}
			m_pie->_rtf_close_brace();

		 }
		 //
		 // Now export the Latex associated with this
		 //
		 if(pszLatexId)
		 {
             bFoundDataItem = m_pDocument->getDataItemDataByName(static_cast<const char*>(pszLatexId),
                                                                 &pbb, NULL, 
                                                                 NULL);
			if (!bFoundDataItem)
			{

				UT_DEBUGMSG(("RTF_Export: cannot get dataitem for latex\n"));
				return;
			}
		        m_pie->_rtf_open_brace();
			m_pie->_rtf_keyword("*");
			m_pie->_rtf_keyword("abilatexdata ");
			buf = pszLatexId;
			buf += " ";
			m_pie->_rtf_chardata(buf.c_str(),buf.size());

			lenData = pbb->getLength();
			pData = pbb->getPointer(0);
			for (k=0; k<lenData; k++)
			{
				if (k%32==0)
					m_pie->_rtf_nl();
				UT_String_sprintf(buf,"%02x",pData[k]);
				m_pie->_rtf_chardata(buf.c_str(),2);
			}
			m_pie->_rtf_close_brace();
		 }
		 //
		 // Now export the snapshot associated with this
		 //
		 std::string sMime, sSnapshot = std::string("snapshot-svg-") + pszDataId;
         bFoundDataItem = m_pDocument->getDataItemDataByName(sSnapshot.c_str(),
                                                             &pbb, &sMime, NULL);
		 if (!bFoundDataItem)
		 {
			 sSnapshot = std::string("snapshot-png-") + pszDataId;
    		 bFoundDataItem = m_pDocument->getDataItemDataByName(sSnapshot.c_str(),
	          	                                                 &pbb, &sMime, NULL);
		 }
		 if (bFoundDataItem)
			_writeEmbedData (sSnapshot, pbb, sMime);
		 //
		 // Now export math codes into the RTF stream
		 //
		 m_pie->_rtf_open_brace();
		 m_pie->_rtf_keyword("*");
		 m_pie->_rtf_keyword("abimathml ");
		 UT_UTF8String sAllProps;
		 UT_UTF8String sPropName;
		 UT_UTF8String sPropVal;
		 UT_sint32 i = 0;
		 const gchar * szProp = NULL;
		 const gchar * szVal = NULL;
		 for(i = 0; i < 50; i++)
		 {
		       szProp = NULL;
		       szVal = NULL;
		       pSpanAP->getNthProperty(i,szProp,szVal);
		       if((szProp != NULL) && (szVal != NULL))
		       { 
			   sPropName = szProp;
			   sPropVal = szVal;
			   UT_UTF8String_setProperty(sAllProps,sPropName,sPropVal);
		       }
		       else
		       {
		           break;
		       }
		 }
		 sPropName = "dataid";
		 sPropVal =pszDataId;
		 UT_UTF8String_setProperty(sAllProps,sPropName,sPropVal);
		 if(pszLatexId)
		 {
		       sPropName = "latexid";
		       sPropVal =pszLatexId;
		       UT_UTF8String_setProperty(sAllProps,sPropName,sPropVal);
		 }
		 m_pie->write(sAllProps.utf8_str());
		 m_pie->_rtf_close_brace();
	 }
	 else if(strcmp(szPrefix,"embed") == 0)
	 {
		 const PP_AttrProp * pSpanAP = NULL;
		 const gchar * pszDataId = NULL, * pszOrigDataId = NULL;
		 m_pDocument->getAttrProp(api, &pSpanAP);
		 pSpanAP->getAttribute("dataid", pszDataId);
		 UT_UTF8String sProps;
		 if(pszDataId == NULL)
		 {
			 return;
		 }
		 //
		 // Export the data associated with this
		 //
		 const UT_ByteBuf * pbb = NULL;
		 bool bFoundDataItem = false;
		 UT_String buf;
		 UT_UTF8String sUID;
		 std::string mime_type;
		bFoundDataItem = m_pDocument->getDataItemDataByName(static_cast<const char*>(pszDataId),
                                                            &pbb,
                                                            &mime_type,
                                                            NULL);
		if (!bFoundDataItem)
		{
			UT_DEBUGMSG(("RTF_Export: cannot get dataitem for embedded object\n"));
			return;
		}
		// we need unique IDs, old objects were created with simple IDs
		pszOrigDataId = pszDataId;
		if (strncmp(pszDataId, "obj-", 4))
		{
			UT_UTF8String s;
			UT_UUID *uuid = m_pDocument->getNewUUID();
			UT_return_if_fail(uuid != NULL);
			sUID = "obj-";
			uuid->toString(s);
			sUID += s;
			pszDataId = static_cast <const char *>(sUID.utf8_str());
		}
		_writeEmbedData (pszDataId, pbb, mime_type);

		 //
		 // Now export the snapshot associated with this
		 //
		 std::string sMime, sSnapshot = std::string("snapshot-svg-") + pszOrigDataId;
         bFoundDataItem = m_pDocument->getDataItemDataByName(sSnapshot.c_str(),
                                                             &pbb, &sMime, NULL);
		 if (!bFoundDataItem)
		 {
			 sSnapshot = std::string("snapshot-png-") + pszOrigDataId;
    		 bFoundDataItem = m_pDocument->getDataItemDataByName(sSnapshot.c_str(),
	          	                                                 &pbb, &sMime, NULL);
			 sSnapshot = std::string("snapshot-png-") + pszDataId;
		 }
		 else
			 sSnapshot = std::string("snapshot-svg-") + pszDataId;
		 if (bFoundDataItem)
		 {
			_writeEmbedData (sSnapshot, pbb, sMime);
		 }
		 m_pie->_rtf_open_brace();
		 m_pie->_rtf_keyword("*");
		 m_pie->_rtf_keyword("abiembed ");
		 UT_UTF8String sAllProps;
		 UT_UTF8String sPropName;
		 UT_UTF8String sPropVal;
		 UT_sint32 i = 0;
		 const gchar * szProp = NULL;
		 const gchar * szVal = NULL;
		 for(i = 0; i < 50; i++)
		 {
		   szProp = NULL;
		   szVal = NULL;
		   pSpanAP->getNthProperty(i,szProp,szVal);
		   if((szProp != NULL) && (szVal != NULL))
		   { 
		     sPropName = szProp;
		     sPropVal = szVal;
		     UT_UTF8String_setProperty(sAllProps,sPropName,sPropVal);
		   }
		   else
		   {
		     break;
		   }
		 }
		 sPropName = "dataid";
		 sPropVal =pszDataId;
		 UT_UTF8String_setProperty(sAllProps,sPropName,sPropVal);
		 m_pie->write(sAllProps.utf8_str());
		 m_pie->_rtf_close_brace();
	 }

}

void s_RTF_ListenerWriteDoc::_writeEmbedData (const std::string & Name, const UT_ByteBuf * pbb, const std::string & mime_type)
{
    m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword("*");
	m_pie->_rtf_keyword("abiembeddata ");
	UT_String buf = Name;
	buf += " mime-type:";
	buf += mime_type;
	buf += " ";
	m_pie->_rtf_chardata(buf.c_str(),buf.size());

	UT_uint32 k, lenData = pbb->getLength();
	const UT_Byte * pData = pbb->getPointer(0);
	for (k=0; k<lenData; k++)
	{
		if (k%32==0)
			m_pie->_rtf_nl();
		UT_String_sprintf(buf,"%02x",pData[k]);
		m_pie->_rtf_chardata(buf.c_str(),2);
	}
	m_pie->_rtf_close_brace();
}

/*!
 * This exports all the properties in a cell strux by extending rtf with
 * a \*\abicellproperties keyword.
 * Code mostly copied for fl_TableLayout::_lookupProperties. This code should
 * should be updated when new properties are defined.
 */
void s_RTF_ListenerWriteDoc::_export_AbiWord_Cell_props(PT_AttrPropIndex api, bool bFill)
{
//
// Export abiword table properties as an extension
// Use these for cutting and pasting within abiword.
//
	UT_String sCellProps;
	sCellProps.clear();
	_fillCellProps(api, sCellProps);
	UT_String sTopAttach = "top-attach";
	UT_String sTop = UT_String_getPropVal(sCellProps,sTopAttach);
	UT_String sBotAttach = "bot-attach";
	UT_String sBot = UT_String_getPropVal(sCellProps,sBotAttach);
	if(bFill)
	{
		UT_String sLeftAttach = "left-attach";
		m_iFirstTop = atoi(sTop.c_str());
		UT_String sLeft = UT_String_getPropVal(sCellProps,sLeftAttach);
		UT_sint32 iFirstLeft = atoi(sLeft.c_str());
//
// Export cells to the left of the current cell.
//
		UT_sint32 i = 0;
		UT_String sRightAttach = "right-attach";
		UT_String sTempProps;
		UT_String sTTop = "0";
		UT_String sTBot = "1";
		for(i=0;i< iFirstLeft;i++)
		{
			sTempProps.clear();
			UT_String_setProperty(sTempProps,sLeftAttach,
								  UT_String_sprintf("%d",i));
			UT_String_setProperty(sTempProps,sRightAttach,
								  UT_String_sprintf("%d",i+1));
			UT_String_setProperty(sTempProps,sTopAttach,sTTop);
			UT_String_setProperty(sTempProps,sBotAttach,sTBot);
//
// Export an open and close cell extension. The Abi importer will place 
// a paragraph in for us
//
			m_pie->_rtf_open_brace();
			m_pie->_rtf_keyword("*");
			m_pie->_rtf_keyword("abicellprops ",sTempProps.c_str());
			m_pie->_rtf_close_brace();
			m_pie->_rtf_open_brace();
			m_pie->_rtf_keyword("*");
			m_pie->_rtf_keyword("abiendcell");
			m_pie->_rtf_close_brace();
		}
	}
//
// Adjust the top and bottom attaches for the offset within the table if the
// select starts before the start of the table
//
	if(m_iFirstTop > 0)
	{
		UT_sint32 iTop = atoi(sTop.c_str()) - m_iFirstTop;
		sTop = UT_String_sprintf("%d",iTop);
		UT_String_setProperty(sCellProps,sTopAttach,sTop);
		UT_sint32 iBot = atoi(sBot.c_str()) - m_iFirstTop;
		sBot = UT_String_sprintf("%d",iBot);
		UT_String_setProperty(sCellProps,sBotAttach,sBot);
	}
	xxx_UT_DEBUGMSG(("Cell props are %s \n",sCellProps.c_str()));
	m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword("*");
	m_pie->_rtf_keyword("abicellprops ",sCellProps.c_str());
	m_pie->_rtf_close_brace();
}

/*!
 * Convience function to lookup a property via const char * string.
 * If the property is not present sVal is returned with zero size.
 */
void s_RTF_ListenerWriteDoc::_getPropString(const UT_String sPropString, const char * szProp, UT_String & sVal)
{
	sVal.clear();
	const UT_String sProp(szProp);
	sVal = UT_String_getPropVal(sPropString,sProp);
}

/*!
 * Export all the properties of this cell to the RTF stream.
 * api is the Attribute Property Index.
 * sTableProps is the UT_String containing all the Table Properties defined
 * for the table. 
 */
void s_RTF_ListenerWriteDoc::_exportCellProps(PT_AttrPropIndex  api, UT_String & sTableProps)
{
	UT_String sCellProps;
	UT_String sWork;
	UT_sint32 iThick =1;
	UT_sint32 iColor =0;
	bool bDrawBorder = true;
	_fillCellProps(api,sCellProps);

//
// Alignements of cells
//
	m_pie->_rtf_keyword("clvertalt"); // only top alignment for now.
//
// Other aligments are:
// \clvertalc Vertical center alignment.
// \clvertalb Virtical Bottom alignemnt.
//
// Text flow:
//
	m_pie->_rtf_keyword("cltxlrtb"); // Only left to right, top to bottom now
//
// Others:
// \cltxtbrl Text in a cell flows right to left and top to bottom.
// \cltxbtlr Text in a cell flows left to right and bottom to top.
// \cltxlrtbv Text in a cell flows left to right and top to bottom, vertical.
// \cltxtbrlv Text in a cell flows top to bottom and right to left, vertical.

//
// Top Border definitions
//
	_getPropString(sCellProps,"top-style",sWork);
	bDrawBorder = true;
	if(sWork.size() == 0)
	{
	        _getPropString(sTableProps,"top-style",sWork) ;
	}
	if(sWork.size()>0)
	{
		PP_PropertyMap::TypeLineStyle linestyle = PP_PropertyMap::linestyle_type(sWork.c_str());
		if(linestyle == PP_PropertyMap::linestyle_inherit)
		{
			linestyle = m_LastLinestyle;
		}
		else if (linestyle == PP_PropertyMap::linestyle_none)
		{
			bDrawBorder = false;
		}
		m_LastLinestyle = linestyle;
		if(bDrawBorder)
		{
			m_pie->_rtf_keyword("clbrdrt"); // cell top border
			if(linestyle == PP_PropertyMap::linestyle_solid)
			{
				m_pie->_rtf_keyword("brdrs"); // plain border
			}
			else if( linestyle == PP_PropertyMap::linestyle_dotted)
			{
				m_pie->_rtf_keyword("brdrdot"); // plain border
			}
			else if( linestyle == PP_PropertyMap::linestyle_dashed)
			{
				m_pie->_rtf_keyword("brdrdash"); // plain border
			}
		}
		else
		{
			m_pie->_rtf_keyword("clbrdrt"); // cell top border
			m_pie->_rtf_keyword("brdrnone"); // no border
		}

	}
	else
	{
		m_pie->_rtf_keyword("clbrdrt"); // cell top border
		m_pie->_rtf_keyword("brdrs"); // plain border
	}
	if(bDrawBorder)
	{
		_getPropString(sCellProps,"top-thickness",sWork);
		if(sWork.size()>0)
		{
			m_pie->_rtf_keyword_ifnotdefault_twips("brdrw",sWork.c_str(),-1);
		}
		else
		{
			m_pie->_rtf_keyword("brdrw",10*iThick); //border thickness
		}
		_getPropString(sCellProps,"top-color",sWork);
		if(sWork.size()>0)
		{
			bool bWriteColor = true;
			if (strcmp (sWork.c_str(), "inherit") == 0)
			{
				iColor =  m_pie->_findOrAddColor(m_sLastColor.c_str());
			}
			else if(strcmp (sWork.c_str(), "transparent") == 0)
			{
				bWriteColor = false;
				iColor = m_pie->_findOrAddColor(sWork.c_str());
			}
			else
			{
				iColor =  m_pie->_findOrAddColor(sWork.c_str());
			}
			m_sLastColor = sWork;
			if(bWriteColor)
			{
				m_pie->_rtf_keyword("brdrcf",iColor);
			}
		}
	}
	m_pie->write(" ");											

//
//write out the background colour of the cell
//
	_getPropString(sCellProps,"background-color",sWork);
	if(sWork.size()>0)
	{
		bool bWriteColor = true;
		if (strcmp (sWork.c_str(), "inherit") == 0)
		{
			iColor =  m_pie->_findOrAddColor(m_sLastColor.c_str());
		}
		else if(strcmp (sWork.c_str(), "transparent") == 0)
		{
			bWriteColor = false;
			iColor = m_pie->_findOrAddColor(sWork.c_str());
		}
		else
		{
			iColor =  m_pie->_findOrAddColor(sWork.c_str());
		}
		m_sLastColor = sWork;

		if(bWriteColor)
		{
			m_pie->_rtf_keyword("clcbpat",iColor); // cell background color
		}
	}
//
// Background style
//
	_getPropString(sCellProps,"bg-style",sWork);
	if(sWork.size() > 0)
	{
//
// We haven't implemented this yet
//
// Allowed patterns are:
// \clbghoriz,\clbgvert,\clbgfdiag,clbgbdiag,\clbgcross,\clbgdcross,
// \clbgdkhor,\clbgdkvert,\clbgdkfdiag,clbgdkbdiag,\clbgdkcross,\clbgdkdcross
//
// Pattern Line color is
//
// \clcfpatN
	}

//
// Left Border Definitions
//
	_getPropString(sCellProps,"left-style",sWork);
	bDrawBorder = true;
	if(sWork.size() == 0)
	{
	        _getPropString(sTableProps,"left-style",sWork); 
	}
	if(sWork.size()>0)
	{
		PP_PropertyMap::TypeLineStyle linestyle = PP_PropertyMap::linestyle_type(sWork.c_str());
		if(linestyle == PP_PropertyMap::linestyle_inherit)
		{
			linestyle = m_LastLinestyle;
		}
		else if (linestyle == PP_PropertyMap::linestyle_none)
		{
			bDrawBorder = false;
		}
		m_LastLinestyle = linestyle;
		if(bDrawBorder)
		{
			m_pie->_rtf_keyword("clbrdrl"); // cell left border
			if(linestyle == PP_PropertyMap::linestyle_solid)
			{
				m_pie->_rtf_keyword("brdrs"); // plain border
			}
			else if( linestyle == PP_PropertyMap::linestyle_dotted)
			{
				m_pie->_rtf_keyword("brdrdot"); // plain border
			}
			else if( linestyle == PP_PropertyMap::linestyle_dashed)
			{
				m_pie->_rtf_keyword("brdrdash"); // plain border
			}
		}
		else
		{
			m_pie->_rtf_keyword("clbrdrl"); // cell left border
			m_pie->_rtf_keyword("brdrnone"); // no border
		}
	}
	else
	{
		m_pie->_rtf_keyword("clbrdrl"); // cell left border
		m_pie->_rtf_keyword("brdrs"); // plain border
	}
	if(bDrawBorder)
	{
		_getPropString(sCellProps,"left-thickness",sWork);
		if(sWork.size()>0)
		{
			m_pie->_rtf_keyword_ifnotdefault_twips("brdrw",sWork.c_str(),-1);
		}
		else
		{
			m_pie->_rtf_keyword("brdrw",10*iThick); //border thickness
		}
		_getPropString(sCellProps,"left-color",sWork);
		if(sWork.size()>0)
		{
			bool bWriteColor = true;
			if (strcmp (sWork.c_str(), "inherit") == 0)
			{
				iColor =  m_pie->_findOrAddColor(m_sLastColor.c_str());
			}
			else if(strcmp (sWork.c_str(), "transparent") == 0)
			{
				bWriteColor = false;
				iColor = m_pie->_findOrAddColor(sWork.c_str());
			}
			else
			{
				iColor =  m_pie->_findOrAddColor(sWork.c_str());
			}
			m_sLastColor = sWork;
			if(bWriteColor)
			{
				m_pie->_rtf_keyword("brdrcf",iColor);
			}
		}
	}
	m_pie->write(" ");											
//
// Bottom Border Definitions
//
	_getPropString(sCellProps,"bot-style",sWork);
	bDrawBorder = true;
	if(sWork.size() == 0)
	{
	        _getPropString(sTableProps,"bot-style",sWork); 
	}
	if(sWork.size()>0)
	{
		PP_PropertyMap::TypeLineStyle linestyle = PP_PropertyMap::linestyle_type(sWork.c_str());
		if(linestyle == PP_PropertyMap::linestyle_inherit)
		{
			linestyle = m_LastLinestyle;
		}
		else if (linestyle == PP_PropertyMap::linestyle_none)
		{
			bDrawBorder = false;
		}
		m_LastLinestyle = linestyle;
		if(bDrawBorder)
		{
			m_pie->_rtf_keyword("clbrdrb"); // cell bottom border
			if(linestyle == PP_PropertyMap::linestyle_solid)
			{
				m_pie->_rtf_keyword("brdrs"); // plain border
			}
			else if( linestyle == PP_PropertyMap::linestyle_dotted)
			{
				m_pie->_rtf_keyword("brdrdot"); // plain border
			}
			else if( linestyle == PP_PropertyMap::linestyle_dashed)
			{
				m_pie->_rtf_keyword("brdrdash"); // plain border
			}
		}
		else
		{
			m_pie->_rtf_keyword("clbrdrb"); // cell bottom border
			m_pie->_rtf_keyword("brdrnone"); // no border
		}

	}
	else
	{
		m_pie->_rtf_keyword("clbrdrb"); // cell bottom border
		m_pie->_rtf_keyword("brdrs"); // plain border
	}
	if(bDrawBorder)
	{
		_getPropString(sCellProps,"bot-thickness",sWork);
		if(sWork.size()>0)
		{
			m_pie->_rtf_keyword_ifnotdefault_twips("brdrw",sWork.c_str(),-1);
		}
		else
		{
			m_pie->_rtf_keyword("brdrw",10*iThick); //border thickness
		}
		_getPropString(sCellProps,"bot-color",sWork);
		if(sWork.size()>0)
		{
			bool bWriteColor = true;
			if (strcmp (sWork.c_str(), "inherit") == 0)
			{
				iColor =  m_pie->_findOrAddColor(m_sLastColor.c_str());
			}
			else if(strcmp (sWork.c_str(), "transparent") == 0)
			{
				bWriteColor = false;
				iColor = m_pie->_findOrAddColor(sWork.c_str());
			}
			else
			{
				iColor =  m_pie->_findOrAddColor(sWork.c_str());
			}
			m_sLastColor = sWork;
			if(bWriteColor)
			{
				m_pie->_rtf_keyword("brdrcf",iColor);
			}
		}
	}
	m_pie->write(" ");											
//
// Right Border Definitions
//
	_getPropString(sCellProps,"right-style",sWork);
	bDrawBorder = true;
	if(sWork.size() == 0)
	{
	        _getPropString(sTableProps,"right-style",sWork); 
	}
	if(sWork.size()>0)
	{
		PP_PropertyMap::TypeLineStyle linestyle = PP_PropertyMap::linestyle_type(sWork.c_str());
		if(linestyle == PP_PropertyMap::linestyle_inherit)
		{
			linestyle = m_LastLinestyle;
		}
		else if (linestyle == PP_PropertyMap::linestyle_none)
		{
			bDrawBorder = false;
		}
		m_LastLinestyle = linestyle;
		if(bDrawBorder)
		{
			m_pie->_rtf_keyword("clbrdrr"); // cell right border
			if(linestyle == PP_PropertyMap::linestyle_solid)
			{
				m_pie->_rtf_keyword("brdrs"); // plain border
			}
			else if( linestyle == PP_PropertyMap::linestyle_dotted)
			{
				m_pie->_rtf_keyword("brdrdot"); // plain border
			}
			else if( linestyle == PP_PropertyMap::linestyle_dashed)
			{
				m_pie->_rtf_keyword("brdrdash"); // plain border
			}
		}
		else
		{
			m_pie->_rtf_keyword("clbrdrr"); // cell right border
			m_pie->_rtf_keyword("brdrnone"); // no border
		}

	}
	else
	{
		m_pie->_rtf_keyword("clbrdrr"); // cell right border
		m_pie->_rtf_keyword("brdrs"); // plain border
	}
	if(bDrawBorder)
	{
		_getPropString(sCellProps,"right-thickness",sWork);
		if(sWork.size()>0)
		{
			m_pie->_rtf_keyword_ifnotdefault_twips("brdrw",sWork.c_str(),-1);
		}
		else
		{
			m_pie->_rtf_keyword("brdrw",10*iThick); //border thickness
		}
		_getPropString(sCellProps,"right-color",sWork);
		if(sWork.size()>0)
		{
			bool bWriteColor = true;
			if (strcmp (sWork.c_str(), "inherit") == 0)
			{
				iColor =  m_pie->_findOrAddColor(m_sLastColor.c_str());
			}
			else if(strcmp (sWork.c_str(), "transparent") == 0)
			{
				bWriteColor = false;
				iColor = m_pie->_findOrAddColor(sWork.c_str());
			}
			else
			{
				iColor =  m_pie->_findOrAddColor(sWork.c_str());
			}
			m_sLastColor = sWork;
			if(bWriteColor)
			{
				m_pie->_rtf_keyword("brdrcf",iColor);
			}
		}
	}
	m_pie->write(" ");											
}

/*!
 * Fill the supplied UT_String with all the properties defined for a cell
 */
void s_RTF_ListenerWriteDoc::_fillCellProps(PT_AttrPropIndex api, UT_String & sCellProps)
{
	const PP_AttrProp* pSectionAP = NULL;
	m_pDocument->getAttrProp(api, &pSectionAP);
	const gchar* pszHomogeneous = NULL;
	pSectionAP->getProperty("homogeneous", pszHomogeneous);
	UT_String sPropVal;
	UT_String sProp;
	const gchar* pszLeftOffset = NULL;
	const gchar* pszTopOffset = NULL;
	const gchar* pszRightOffset = NULL;
	const gchar* pszBottomOffset = NULL;
	pSectionAP->getProperty("cell-margin-left", pszLeftOffset);
	pSectionAP->getProperty("cell-margin-top", pszTopOffset);
	pSectionAP->getProperty("cell-margin-right", pszRightOffset);
	pSectionAP->getProperty("cell-margin-bottom", pszBottomOffset);

	if(pszLeftOffset && pszLeftOffset[0])
	{
		sProp = "cell-margin-left";
		sPropVal= pszLeftOffset;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}

	if(pszTopOffset && pszTopOffset[0])
	{
		sProp = "cell-margin-top";
		sPropVal= pszTopOffset;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}

	if(pszRightOffset && pszRightOffset[0])
	{
		sProp = "cell-margin-right";
		sPropVal= pszRightOffset;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}

	if(pszBottomOffset && pszBottomOffset[0])
	{
		sProp = "cell-margin-bottom";
		sPropVal= pszBottomOffset;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}
	const gchar* pszLeftAttach = NULL;
	const gchar* pszRightAttach = NULL;
	const gchar* pszTopAttach = NULL;
	const gchar* pszBottomAttach = NULL;
	pSectionAP->getProperty("left-attach", pszLeftAttach);
	pSectionAP->getProperty("right-attach", pszRightAttach);
	pSectionAP->getProperty("top-attach", pszTopAttach);
	pSectionAP->getProperty("bot-attach", pszBottomAttach);
	xxx_UT_DEBUGMSG(("CellLayout _lookupProps top %s bot %s left %s right %s \n",pszTopAttach,pszBottomAttach,pszLeftAttach,pszRightAttach)); 
	if(pszLeftAttach && pszLeftAttach[0])
	{
		sProp = "left-attach";
		sPropVal= pszLeftAttach;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}
	if(pszRightAttach && pszRightAttach[0])
	{
		sProp = "right-attach";
		sPropVal= pszRightAttach;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}
	if(pszTopAttach && pszTopAttach[0])
	{
		sProp = "top-attach";
		sPropVal= pszTopAttach;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}
	if(pszBottomAttach && pszBottomAttach[0])
	{
		sProp = "bot-attach";
		sPropVal= pszBottomAttach;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}

	/* cell-border properties:
	 */
	const gchar * pszColor = NULL;
	pSectionAP->getProperty ("color", pszColor);
	if (pszColor)
	{
		sProp = "color";
		sPropVal= pszColor;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}
	const gchar * pszBorderColor = NULL;
	const gchar * pszBorderStyle = NULL;
	const gchar * pszBorderWidth = NULL;

	pSectionAP->getProperty ("bot-color",       pszBorderColor);
	if (pszBorderColor && *pszBorderColor)
	{
		sProp = "bot-color";
		sPropVal= pszBorderColor;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}
	pSectionAP->getProperty ("bot-style",       pszBorderStyle);
	if (pszBorderStyle && *pszBorderStyle)
	{
		sProp = "bot-style";
		sPropVal= pszBorderStyle;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}

	pSectionAP->getProperty ("bot-thickness",   pszBorderWidth);
	if (pszBorderWidth && *pszBorderWidth)
	{
		sProp = "bot-thickness";
		sPropVal= pszBorderWidth;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}
	pszBorderColor = NULL;
	pszBorderStyle = NULL;
	pszBorderWidth = NULL;

	pSectionAP->getProperty ("left-color",      pszBorderColor);
	pSectionAP->getProperty ("left-style",      pszBorderStyle);
	pSectionAP->getProperty ("left-thickness",  pszBorderWidth);

	if (pszBorderColor && *pszBorderColor)
	{
		sProp = "left-color";
		sPropVal= pszBorderColor;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}
	if (pszBorderStyle && *pszBorderStyle)
	{
		sProp = "left-style";
		sPropVal= pszBorderStyle;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}
	if (pszBorderWidth && *pszBorderWidth)
	{
		sProp = "left-thickness";
		sPropVal= pszBorderWidth;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}

	pszBorderColor = NULL;
	pszBorderStyle = NULL;
	pszBorderWidth = NULL;

	pSectionAP->getProperty ("right-color",     pszBorderColor);
	pSectionAP->getProperty ("right-style",     pszBorderStyle);
	pSectionAP->getProperty ("right-thickness", pszBorderWidth);

	if (pszBorderColor && *pszBorderColor)
	{
		sProp = "right-color";
		sPropVal= pszBorderColor;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}
	if (pszBorderStyle && *pszBorderStyle)
	{
		sProp = "right-style";
		sPropVal= pszBorderStyle;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}
	if (pszBorderWidth && *pszBorderWidth)
	{
		sProp = "right-thickness";
		sPropVal= pszBorderWidth;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}

	pszBorderColor = NULL;
	pszBorderStyle = NULL;
	pszBorderWidth = NULL;

	pSectionAP->getProperty ("top-color",       pszBorderColor);
	pSectionAP->getProperty ("top-style",       pszBorderStyle);
	pSectionAP->getProperty ("top-thickness",   pszBorderWidth);
	if (pszBorderColor && *pszBorderColor)
	{
		sProp = "top-color";
		sPropVal= pszBorderColor;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}
	if (pszBorderStyle && *pszBorderStyle)
	{
		sProp = "top-style";
		sPropVal= pszBorderStyle;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}
	if (pszBorderWidth && *pszBorderWidth)
	{
		sProp = "top-thickness";
		sPropVal= pszBorderWidth;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}

	/* cell fill
	 */
	const gchar * pszBgStyle = NULL;
	const gchar * pszBgColor = NULL;
	const gchar * pszBackgroundColor = NULL;

	pSectionAP->getProperty ("bg-style",         pszBgStyle);
	if (pszBgStyle && *pszBgStyle)
	{
		sProp = "bg-style";
		sPropVal= pszBgStyle;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}
	pSectionAP->getProperty ("bgcolor",          pszBgColor);
	if (pszBgColor && *pszBgColor)
	{
		sProp = "bgcolor";
		sPropVal= pszBgColor;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}
	pSectionAP->getProperty ("background-color", pszBackgroundColor);
	if (pszBackgroundColor && *pszBackgroundColor)
	{
		sProp = "background-color";
		sPropVal= pszBackgroundColor;
		UT_String_setProperty(sCellProps,sProp,sPropVal);
	}
}


void s_RTF_ListenerWriteDoc::_open_cell(PT_AttrPropIndex api)
{
//
// If we copy text to the clipboard we may not cover a open table strux.
// Put this in to prevent crashes.
//
	if(m_Table.getNestDepth() < 1)
	{
		_open_table(api,true);
		_export_AbiWord_Cell_props(api,true);
	}
	else
	{
		_export_AbiWord_Cell_props(api,false);
	}

	//UT_sint32 iOldRow = m_iTop;
	UT_sint32 iOldRight = m_iRight;
	xxx_UT_DEBUGMSG(("Setting cell API 1 NOW!!!!!!!!!!!!!!!!! %d \n",api));
	PT_AttrPropIndex prevAPI = api;
	m_Table.OpenCell(api);
	bool bNewRow = false;
	xxx_UT_DEBUGMSG(("iOldRow %d newTop %d \n",iOldRow,m_Table.getTop()));
	if(	(m_Table.getLeft() < iOldRight) || m_bNewTable)
	{
		xxx_UT_DEBUGMSG(("NEW ROW DETECTED !!!!!!!!!!!!!!!!!\n"));
		if(m_bNewTable)
		{
			m_pie->_rtf_open_brace();
			if(m_Table.getNestDepth() > 1)
			{
				m_pie->_rtf_close_brace();
			}
			else
			{
				_newRow();
			}
		}
		else
		{
			bNewRow = true;
//
// Now we have to output cell markers for all the vertically merged cells
// on the right edge of previous row
//
			UT_sint32 count = m_Table.getPrevNumRightMostVMerged();
			UT_sint32 i =0;
			for(i=0; i<count; i++)
			{

				if(m_Table.getNestDepth() < 2)
				{
					UT_sint32 iRight = getRightOfCell(m_Table.getCurRow() -1,m_iRight + i);
					if(iRight == (m_iRight + i +1))
					{
						m_pie->_rtf_keyword("cell");
					}
				}
				else
				{
					UT_sint32 iRight = getRightOfCell(m_Table.getCurRow() -1,m_iRight + i);
					if(iRight == (m_iRight + i +1))
					{
						m_pie->_rtf_keyword("nestcell");
					}
				}
			}
			if(m_Table.getNestDepth() < 2)
			{
				m_pie->_rtf_keyword("row");
				_newRow();
			}
			else
			{
				m_Table.OpenCell(prevAPI);
				_newRow();
				m_Table.OpenCell(api);
				m_pie->_rtf_keyword("nestrow");
			}
			if(m_Table.getNestDepth() > 1)
			{
//
// This closes off the \nesttableprops
				m_pie->_rtf_close_brace();
			}
		}
	}
//
// reset api. It may have been screwed in _newRow
//
	xxx_UT_DEBUGMSG(("Setting cell API 1 NOW!!!!!!!!!!!!!!!!! %d \n",api));
	m_Table.OpenCell(api);
	if(bNewRow)
	{
//
// Output cell markers for all vertically merged cells at the start of the row
//
//
// fix me have to handle horizontally and vertically merged cells at the
// left of a table.

		UT_sint32 i =0;
		if(m_Table.getNestDepth() < 2)
		{
			for(i = 0; i < m_Table.getLeft(); i++)
			{
				m_pie->_rtf_keyword("cell");
			}
		}
		else
		{
			for(i = 0; i < m_Table.getLeft(); i++)
			{
				xxx_UT_DEBUGMSG(("Writing nestcell in wrong spot 1 \n"));
				m_pie->_rtf_keyword("nestcell");
			}
		}
	}
//
// Now output vertically merged cell markers between the last right position and this cell's left.
//
	else
	{
		if(!m_bNewTable)
		{
			UT_sint32 i =0;
			if(m_Table.getNestDepth() < 2)
			{
				for(i = m_iRight; i < m_Table.getLeft(); i++)
//
// We don't output these cell's if they're horiztonally merged too.
//
				{
					UT_sint32 iRight = getRightOfCell(m_Table.getCurRow(),i);
					if(iRight == (i +1))
					{
						m_pie->_rtf_keyword("cell");
					}
				}
			}
			else
			{
				for(i = m_iRight; i < m_Table.getLeft(); i++)
				{
					xxx_UT_DEBUGMSG(("Writing nestcell in wrong spot 2 \n"));
					UT_sint32 iRight = getRightOfCell(m_Table.getCurRow(),i);
					if(iRight == (i +1))
					{
						m_pie->_rtf_keyword("nestcell");
					}
				}
			}
		}
	}
	m_bNewTable = false;
	m_iLeft = m_Table.getLeft();
	m_iRight = m_Table.getRight();
	m_iTop = m_Table.getTop();
	m_iBot = m_Table.getBot();
}

/*!
 * This returns the right-attach of the cell to the right of the cell at (row,
 * col)
 */
UT_sint32  s_RTF_ListenerWriteDoc::getRightOfCell(UT_sint32 row,UT_sint32 col)
{
	pf_Frag_Strux* sdhCell = m_pDocument->getCellSDHFromRowCol(m_Table.getTableSDH(),true,PD_MAX_REVISION,row,col);
	if(sdhCell == NULL)
	{
		return -1;
	}
	const char * szRight;
	m_pDocument->getPropertyFromSDH(sdhCell,true,PD_MAX_REVISION,"right-attach",&szRight);
	UT_sint32 iRight = atoi(szRight);
	return iRight;
}
	
void s_RTF_ListenerWriteDoc::_newRow(void)
{
	UT_sint32 i;
	m_Table.incCurRow();
	m_pie->_rtf_nl();
	if(m_Table.getNestDepth() > 1)
	{
		m_pie->_rtf_keyword("itap",m_Table.getNestDepth());
		m_pie->_rtf_open_brace();
		m_pie->_rtf_keyword("*");
		m_pie->_rtf_keyword("nesttableprops");
	} 
	m_pie->_rtf_keyword("trowd");
	m_pie->write(" ");
	m_pie->_rtf_keyword("itap",m_Table.getNestDepth());

//
// Set spacing between cells
//
	const char * szColSpace = m_Table.getTableProp("table-col-spacing");
	if(szColSpace && *szColSpace)
	{
		double dspace = UT_convertToInches(szColSpace) * 360.0;
		UT_sint32 iSpace =0;
		iSpace = static_cast<UT_sint32>(dspace);
		m_pie->_rtf_keyword("trgaph",iSpace);
	}
	else
	{
		m_pie->_rtf_keyword("trgaph",36);
		szColSpace = "0.05in";
	}
	double dColSpace = UT_convertToInches(szColSpace);
//
// Hardwire left-justification (for now)
//
	m_pie->_rtf_keyword("trql");
//
// Height of row. Hardwired to zero (take maximum cell height for row) for now.
//
	m_pie->_rtf_keyword("trrh",0);
//
// Lookup column positions.
//
	const char * szColumnProps = NULL;
	const char * szColumnLeftPos = NULL;
	szColumnProps = m_Table.getTableProp("table-column-props");
	szColumnLeftPos = m_Table.getTableProp("table-column-leftpos");
	double cellLeftPos = 0;
	if(szColumnLeftPos && *szColumnLeftPos)
	{
		cellLeftPos = UT_convertToInches(szColumnLeftPos);
	}
	UT_sint32 iLeftTwips = 0;
	iLeftTwips =  (UT_sint32) (cellLeftPos*1440.0);
	m_pie->_rtf_keyword("trleft",iLeftTwips);
	UT_GenericVector<fl_ColProps *> vecColProps;
	vecColProps.clear();
	if(szColumnProps && *szColumnProps)
	{
		UT_String sProps = szColumnProps;
		UT_sint32 sizes = sProps.size();
		i =0 ;
		UT_sint32 j =0;
		while(i < sizes)
		{
			for (j=i; (j<sizes) && (sProps[j] != '/') ; j++) {}
			if((j+1)>i && sProps[j] == '/')
			{
				UT_String sSub = sProps.substr(i,(j-i));
				double colWidth = UT_convertToInches(sSub.c_str())* 10000.0;
				i = j + 1;
				fl_ColProps * pColP = new fl_ColProps;
				pColP->m_iColWidth = static_cast<UT_sint32>(colWidth);
				vecColProps.addItem(pColP);
			}
		}
	}
	else
	{
//
// Autofit (or not) the row. Look up col widths
//
		m_pie->_rtf_keyword("trautofit",1);
	}
//
// Handle table line types.
//
	const char * szLineThick = m_Table.getTableProp("table-line-thickness");
	UT_sint32 iThick = -1;
	if(szLineThick && *szLineThick)
	{
		iThick = atoi(szLineThick);
		if(iThick > 0)
		{
			_outputTableBorders(iThick);
		}
	}
	else
	{
		_outputTableBorders(1);
	}
//
// OK now output all the cell properties, including merged cell controls.
//
	UT_sint32 row = m_Table.getCurRow();
	UT_sint32 col = m_Table.getLeft();
	double cellpos = cellLeftPos + dColSpace*0.5;
	double colwidth = 0.0;
	double dcells = static_cast<double>(m_Table.getNumCols());
#if 0
//
// fixme. Write this function to determine the width of a nested cell
//
	if(m_Table.getNestDepth() < 2)
	{
		colwidth = (_getColumnWidthInches() - dColSpace*0.5)/dcells;
	}
	else
	{
		colwidth = m_Table.findThisColWidth();
	}
#endif
	colwidth = (_getColumnWidthInches() - dColSpace*0.5)/dcells;

	UT_sint32 iNext = 1;
	UT_String sTableProps;
	PT_AttrPropIndex tableAPI = m_Table.getTableAPI();
	_fillTableProps(tableAPI,sTableProps);
	
	for(i=0; i < m_Table.getNumCols(); i = iNext)
	{
		m_Table.setCellRowCol(row,i);
		xxx_UT_DEBUGMSG(("SEVIOR: set to row %d i %d left %d right %d \n",row,i,m_Table.getLeft(),m_Table.getRight()));
		if(m_Table.getRight() <= i)
		{
			pf_Frag_Strux* cellSDH = m_pDocument->getCellSDHFromRowCol(m_Table.getTableSDH(),true,PD_MAX_REVISION,
																		  row,i);
			UT_ASSERT_HARMLESS(cellSDH);
			if(cellSDH)
			{
				m_pDocument->miniDump(cellSDH,8);
			}
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			iNext = i+1;
		}
		else
		{
			iNext = m_Table.getRight();
		}
//
// Export all the properties of the cell
//
		PT_AttrPropIndex cellAPI = m_Table.getCellAPI();
		_exportCellProps(cellAPI,sTableProps);
//
// Look if we have a vertically merged cell at this (row,i)
//
		if(m_Table.getTop() < row)
		{
			m_pie->_rtf_keyword("clvmrg");
		}
//
// Look to see if this is the first cell of a set of vertically merged cells
//		
		if((m_Table.getBot() > row +1) && (m_Table.getTop() == row))
		{
			m_pie->_rtf_keyword("clvmgf");
		}
#if 0
//
// Look to see if we have a horizontally merged cell.
//
		if(m_bNewTable && (m_Table.getLeft() < i))
		{
			m_pie->_rtf_keyword("clmrg");
		}
//
// Look to see if this is the first of a group of horizonatally merged cells.
//
		if(m_bNewTable && (m_Table.getRight() > i +1))
		{
			m_pie->_rtf_keyword("clmrgf");
		}
#endif
//
// output cellx for each cell
//
		double thisX = 0.0;
		UT_sint32 j =0;
		if(vecColProps.getItemCount() > 0)
		{
			for(j= 0; (j< m_Table.getRight()) && (j < vecColProps.getItemCount()); j++)
			{
				fl_ColProps * pColP = vecColProps.getNthItem(j);
				double bigWidth = static_cast<double>(pColP->m_iColWidth);
				thisX += bigWidth/10000.0;
			}
		}
		else
		{
			for(j= 0; j< m_Table.getRight(); j++)
			{
				thisX += colwidth;
			}
		}
		thisX += cellpos;
		UT_sint32 iCellTwips = 0;
		iCellTwips = (UT_sint32) (thisX*1440.0);
		m_pie->_rtf_keyword("cellx",iCellTwips);
	}
	if(vecColProps.getItemCount() > 0)
	{
		UT_VECTOR_PURGEALL(fl_ColProps *,vecColProps);
	}
	m_Table.setCellRowCol(row,col);
}

void s_RTF_ListenerWriteDoc::_outputTableBorders(UT_sint32 iThick)
{
	m_pie->_rtf_keyword("trbrdrt"); // top border
	m_pie->_rtf_keyword("brdrs"); // plain border
	m_pie->_rtf_keyword("brdrw",10*iThick); //border thickness
	m_pie->write(" ");											
	m_pie->_rtf_keyword("trbrdrl"); // left border
	m_pie->_rtf_keyword("brdrs");
	m_pie->_rtf_keyword("brdrw",10*iThick); // border thickness
	m_pie->write(" ");											
	m_pie->_rtf_keyword("trbrdrb"); // bottom border
	m_pie->_rtf_keyword("brdrs");
	m_pie->_rtf_keyword("brdrw",10*iThick); // border thickness
	m_pie->write(" ");											
	m_pie->_rtf_keyword("trbrdrr"); // right border
	m_pie->_rtf_keyword("brdrs");
	m_pie->_rtf_keyword("brdrw",10*iThick); // border thickness
	m_pie->write(" ");											
}

void s_RTF_ListenerWriteDoc::_outputCellBorders(UT_sint32 iThick)
{
	m_pie->_rtf_keyword("clbrdrt"); // cell top border
	m_pie->_rtf_keyword("brdrs"); // plain border
	m_pie->_rtf_keyword("brdrw",10*iThick); //border thickness
	m_pie->write(" ");											
	m_pie->_rtf_keyword("clbrdrl"); // cell left border
	m_pie->_rtf_keyword("brdrs");
	m_pie->_rtf_keyword("brdrw",10*iThick); // border thickness
	m_pie->write(" ");											
	m_pie->_rtf_keyword("clbrdrb"); // cell bottom border
	m_pie->_rtf_keyword("brdrs");
	m_pie->_rtf_keyword("brdrw",10*iThick); // border thickness
	m_pie->write(" ");											
	m_pie->_rtf_keyword("clbrdrr"); // cell right border
	m_pie->_rtf_keyword("brdrs");
	m_pie->_rtf_keyword("brdrw",10*iThick); // border thickness
	m_pie->write(" ");											
}

double s_RTF_ListenerWriteDoc::_getColumnWidthInches(void)
{
	double pageWidth = m_pDocument->m_docPageSize.Width(DIM_IN);

	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;
	m_pDocument->getAttrProp(m_apiThisSection,&pSectionAP);
	const gchar * szColumns = PP_evalProperty("columns",
												 pSpanAP,pBlockAP,pSectionAP,
												 m_pDocument,true);
	const gchar * szColumnGap = PP_evalProperty("column-gap",
												   pSpanAP,pBlockAP,pSectionAP,
												   m_pDocument,true);
	const gchar * szMarginLeft = PP_evalProperty("page-margin-left",
													pSpanAP,pBlockAP,pSectionAP,
													m_pDocument,true);
	const gchar * szMarginRight = PP_evalProperty("page-margin-right",
													 pSpanAP,pBlockAP,pSectionAP,
													 m_pDocument,true);
	UT_sint32 iNumCols = 1;
	if(szColumns && *szColumns)
	{
		iNumCols = atoi(szColumns);
	}
	double dNumCols = static_cast<double>(iNumCols);
	double lMarg = UT_convertToInches(szMarginLeft);
	double rMarg = UT_convertToInches(szMarginRight);
	double dGap = UT_convertToInches(szColumnGap);
	double colWidth = pageWidth - lMarg - rMarg - dGap*(dNumCols - 1.0);
	colWidth = colWidth/dNumCols;
	return colWidth;
}

/*!
 * This exports all the properties in atable strux by extend rtf with
 * a \*\abitableproperties keyword.
 * Code mostly copied for fl_TableLayout::_lookupProperties. This code should
 * should be updated when new properties are defined.
 */
void s_RTF_ListenerWriteDoc::_export_AbiWord_Table_props(PT_AttrPropIndex api)
{
//
// Export abiword table properties as an extension
// Use these for cutting and pasting within abiword.
//
	m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword("*");
	UT_String sTableProps;
	sTableProps.clear();
	_fillTableProps(api,sTableProps);
	xxx_UT_DEBUGMSG(("Table props are %s \n",sTableProps.c_str()));
	m_pie->_rtf_keyword("abitableprops ",sTableProps.c_str());
	m_pie->_rtf_close_brace();
}

/*!
 * This method fills the suppiled string with all the properties of the 
 * table given by api
 */
void s_RTF_ListenerWriteDoc::_fillTableProps(PT_AttrPropIndex api, UT_String & sTableProps)
{
	const PP_AttrProp* pSectionAP = NULL;
	m_pDocument->getAttrProp(api, &pSectionAP);
	const gchar* pszHomogeneous = NULL;
	pSectionAP->getProperty("homogeneous", pszHomogeneous);
	UT_String sPropVal;
	UT_String sProp;
	if (pszHomogeneous && pszHomogeneous[0])
	{
		if(atoi(pszHomogeneous) == 1)
		{
			sProp = "homogeneous";
			sPropVal= "1";
			UT_String_setProperty(sTableProps,sProp,sPropVal);
		}
	}
	const gchar* pszLeftOffset = NULL;
	const gchar* pszTopOffset = NULL;
	const gchar* pszRightOffset = NULL;
	const gchar* pszBottomOffset = NULL;
	pSectionAP->getProperty("table-margin-left", pszLeftOffset);
	pSectionAP->getProperty("table-margin-top", pszTopOffset);
	pSectionAP->getProperty("table-margin-right", pszRightOffset);
	pSectionAP->getProperty("table-margin-bottom", pszBottomOffset);

	if(pszLeftOffset && pszLeftOffset[0])
	{
		sProp = "table-margin-left";
		sPropVal= pszLeftOffset;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}

	if(pszTopOffset && pszTopOffset[0])
	{
		sProp = "table-margin-top";
		sPropVal= pszTopOffset;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}

	if(pszRightOffset && pszRightOffset[0])
	{
		sProp = "table-margin-right";
		sPropVal= pszRightOffset;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}

	if(pszBottomOffset && pszBottomOffset[0])
	{
		sProp = "table-margin-bottom";
		sPropVal= pszBottomOffset;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}

	const gchar * pszLineThick = NULL;
	pSectionAP->getProperty("table-line-thickness", pszLineThick);
	if(pszLineThick && *pszLineThick)
	{
		sProp = "table-line-thickness";
		sPropVal= pszLineThick;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}

	const gchar * pszTableColSpacing = NULL;
	const gchar * pszTableRowSpacing = NULL;
	pSectionAP->getProperty("table-col-spacing", pszTableColSpacing);
	pSectionAP->getProperty("table-row-spacing", pszTableRowSpacing);
	if(pszTableColSpacing && *pszTableColSpacing)
	{
		sProp = "table-col-spacing";
		sPropVal= pszTableColSpacing;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}
	if(pszTableRowSpacing && *pszTableRowSpacing)
	{
		sProp = "table-row-spacing";
		sPropVal= pszTableRowSpacing;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}
	const gchar * pszLeftColPos = NULL;
	const gchar * pszColumnProps = NULL;
	pSectionAP->getProperty("table-column-leftpos", pszLeftColPos);
	pSectionAP->getProperty("table-column-props", pszColumnProps);
	if(pszLeftColPos && *pszLeftColPos)
	{
		sProp = "table-column-leftpos";
		sPropVal= pszLeftColPos;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}
	if(pszColumnProps && *pszColumnProps)
	{
		sProp = "table-column-props";
		sPropVal= pszColumnProps;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}
//
// global row height type
//
	const gchar * pszRowHeightType = NULL;
	const gchar * pszRowHeight = NULL;
	pSectionAP->getProperty("table-row-height-type", pszRowHeightType);
	if(pszRowHeightType && *pszRowHeightType)
	{
		sProp = "table-row-height-type";
		sPropVal= pszRowHeightType;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}
	pSectionAP->getProperty("table-row-height", pszRowHeight);
	if(pszRowHeight && *pszRowHeight)
	{
		sProp = "table-row-height";
		sPropVal= pszRowHeight;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}
//
// Positioned row controls
//
	const gchar * pszRowHeights = NULL;
	pSectionAP->getProperty("table-row-heights", pszRowHeights);
	if(pszRowHeights && *pszRowHeights)
	{
		sProp = "table-row-heights";
		sPropVal= pszRowHeights;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}

	/* table-border properties:
	 */
	const gchar * pszColor = NULL;
	pSectionAP->getProperty ("color", pszColor);
	if (pszColor)
	{
		sProp = "color";
		sPropVal= pszColor;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}
	const gchar * pszBorderColor = NULL;
	const gchar * pszBorderStyle = NULL;
	const gchar * pszBorderWidth = NULL;

	pSectionAP->getProperty ("bot-color",       pszBorderColor);
	if (pszBorderColor && *pszBorderColor)
	{
		sProp = "bot-color";
		sPropVal= pszBorderColor;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}
	pSectionAP->getProperty ("bot-style",       pszBorderStyle);
	if (pszBorderStyle && *pszBorderStyle)
	{
		sProp = "bot-style";
		sPropVal= pszBorderStyle;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}

	pSectionAP->getProperty ("bot-thickness",   pszBorderWidth);
	if (pszBorderWidth && *pszBorderWidth)
	{
		sProp = "bot-thickness";
		sPropVal= pszBorderStyle;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}
	pszBorderColor = NULL;
	pszBorderStyle = NULL;
	pszBorderWidth = NULL;

	pSectionAP->getProperty ("left-color",      pszBorderColor);
	pSectionAP->getProperty ("left-style",      pszBorderStyle);
	pSectionAP->getProperty ("left-thickness",  pszBorderWidth);

	if (pszBorderColor && *pszBorderColor)
	{
		sProp = "left-color";
		sPropVal= pszBorderColor;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}
	if (pszBorderStyle && *pszBorderStyle)
	{
		sProp = "left-style";
		sPropVal= pszBorderStyle;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}
	if (pszBorderWidth && *pszBorderWidth)
	{
		sProp = "left-thickness";
		sPropVal= pszBorderStyle;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}

	pszBorderColor = NULL;
	pszBorderStyle = NULL;
	pszBorderWidth = NULL;

	pSectionAP->getProperty ("right-color",     pszBorderColor);
	pSectionAP->getProperty ("right-style",     pszBorderStyle);
	pSectionAP->getProperty ("right-thickness", pszBorderWidth);

	if (pszBorderColor && *pszBorderColor)
	{
		sProp = "right-color";
		sPropVal= pszBorderColor;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}
	if (pszBorderStyle && *pszBorderStyle)
	{
		sProp = "right-style";
		sPropVal= pszBorderStyle;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}
	if (pszBorderWidth && *pszBorderWidth)
	{
		sProp = "right-thickness";
		sPropVal= pszBorderStyle;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}

	pszBorderColor = NULL;
	pszBorderStyle = NULL;
	pszBorderWidth = NULL;

	pSectionAP->getProperty ("top-color",       pszBorderColor);
	pSectionAP->getProperty ("top-style",       pszBorderStyle);
	pSectionAP->getProperty ("top-thickness",   pszBorderWidth);
	if (pszBorderColor && *pszBorderColor)
	{
		sProp = "top-color";
		sPropVal= pszBorderColor;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}
	if (pszBorderStyle && *pszBorderStyle)
	{
		sProp = "top-style";
		sPropVal= pszBorderStyle;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}
	if (pszBorderWidth && *pszBorderWidth)
	{
		sProp = "top-thickness";
		sPropVal= pszBorderStyle;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}

	/* table fill
	 */
	const gchar * pszBgStyle = NULL;
	const gchar * pszBgColor = NULL;
	const gchar * pszBackgroundColor = NULL;

	pSectionAP->getProperty ("bg-style",         pszBgStyle);
	if (pszBgStyle && *pszBgStyle)
	{
		sProp = "bg-style";
		sPropVal= pszBgStyle;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}
	pSectionAP->getProperty ("bgcolor",          pszBgColor);
	if (pszBgColor && *pszBgColor)
	{
		sProp = "bgcolor";
		sPropVal= pszBgColor;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}
	pSectionAP->getProperty ("background-color", pszBackgroundColor);
	if (pszBackgroundColor && *pszBackgroundColor)
	{
		sProp = "background-color";
		sPropVal= pszBackgroundColor;
		UT_String_setProperty(sTableProps,sProp,sPropVal);
	}
	sProp = "table-sdh";
	UT_String_sprintf(sPropVal,"%p",m_Table.getTableSDH());
	UT_String_setProperty(sTableProps,sProp,sPropVal);
	if(sTableProps.size() == 0)
	{
		sTableProps += " ";
	}
}

void s_RTF_ListenerWriteDoc::_open_table(PT_AttrPropIndex api,bool bIsCell)
{
	pf_Frag_Strux* sdhTable = NULL;
	if(bIsCell)
	{
		PT_DocPosition posCell = m_pDocument->getStruxPosition(m_sdh);
		bool b = m_pDocument->getStruxOfTypeFromPosition(posCell,PTX_SectionTable,&sdhTable);
		UT_return_if_fail(b);
		api = m_pDocument->getAPIFromSDH(sdhTable);
		m_Table.OpenTable(sdhTable,api);
	}
	else
	{
		m_Table.OpenTable(m_sdh,api);
	}
	m_bNewTable = true;
	m_iLeft = -1;
	m_iRight = -1;
	m_iTop = -1;
	m_iBot = -1;
	m_iFirstTop = 0;
//
// Export the AbiWord table Properties as RTF extension
//
	_export_AbiWord_Table_props(api);
#if 1 //#TF
	m_pie->_rtf_keyword("par");
#endif
	
	if(m_Table.getNestDepth() > 1)
	{
		m_pie->_rtf_open_brace();
	}
}

void s_RTF_ListenerWriteDoc::_close_cell(void)
{
	if(m_Table.getNestDepth() < 1)
	{
		return;
	}
	if(m_Table.getNestDepth() < 2)
	{
		m_pie->_rtf_keyword("cell");
	}
	else
	{
		m_pie->_rtf_keyword("nestcell");
	}
	m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword("*");
	m_pie->_rtf_keyword("abiendcell");
	m_pie->_rtf_close_brace();

	m_Table.CloseCell();
}

void s_RTF_ListenerWriteDoc::_close_table(void)
{
//
// First output the cells we need to cover any vertically merged cells
//
	UT_sint32 count = m_Table.getNumCols() - m_Table.getRight();
	UT_sint32 i = 0;
	for(i=0; i< count; i++)
	{
		m_pie->_rtf_keyword("cell");
	}
//
// Close off the last row
//
	if(m_Table.getNestDepth() < 2)
	{
		m_pie->_rtf_keyword("row");
	}
	else
	{				
		_newRow();
		m_pie->_rtf_keyword("nestrow");
	}
	m_pie->_rtf_close_brace();
	if(m_Table.getNestDepth() > 1)
	{
		m_pie->_rtf_close_brace();
	}
	m_Table.CloseTable();
	if(m_Table.getNestDepth() < 1)
	{
		m_iCurRow = -1;
		m_iLeft = -1;
		m_iRight = -1;
		m_iTop = -1;
		m_iBot = -1;
	}
	else
	{
		m_iCurRow = m_Table.getTop();
		m_iLeft = m_Table.getLeft();
		m_iRight = m_Table.getRight();
		m_iTop = m_Table.getTop();
		m_iBot = m_Table.getBot();
	}
	m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword("*");
	m_pie->_rtf_keyword("abiendtable");
	m_pie->_rtf_close_brace();
}


bool s_RTF_ListenerWriteDoc::populateStrux(pf_Frag_Strux* sdh,
										   const PX_ChangeRecord * pcr,
										   fl_ContainerLayout* * psfh)
{
	UT_return_val_if_fail(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux, false);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
	*psfh = 0;							// we don't need it.

	m_posDoc = pcrx->getPosition();
	switch (pcrx->getStruxType())
	{
	case PTX_Section:
		{

			// begin a new section.  in RTF this is expressed as
			//
			// <section> := <secfmt>* <hdrftr>? <para>+ (\sect <section>)?
			//
			// here we deal with everything except for the <para>+
//
// OK first we have so see if there is a header/footer associated with this section
//
			PT_AttrPropIndex indexAP = pcr->getIndexAP();
			const PP_AttrProp* pAP = NULL;
			m_pDocument->getAttrProp(indexAP, &pAP);
			const gchar* pszHeaderID = NULL;
			bool bHeader = false;
			const gchar* pszFooterID = NULL;
			bool bFooter = false;
			const gchar* pszHeaderEvenID = NULL;
			bool bHeaderEven = false;
			const gchar* pszFooterEvenID = NULL;
			bool bFooterEven = false;
			const gchar* pszHeaderFirstID = NULL;
			bool bHeaderFirst = false;
			const gchar* pszFooterFirstID = NULL;
			bool bFooterFirst = false;
#if 0
			const gchar* pszHeaderLastID = NULL;
			const gchar* pszFooterLastID = NULL;
#endif
			
// header,headerl (odd) ,headerr(even) ,headerf(first) ,footer,footerl,footerr,footerf

			pAP->getAttribute("header", pszHeaderID);
			if(pszHeaderID != NULL)
			{
				bHeader = true;
			}
			pAP->getAttribute("footer", pszFooterID);
			if(pszFooterID != NULL)
			{
				bFooter = true;
			}
			pAP->getAttribute("header-even", pszHeaderEvenID);
			if(pszHeaderEvenID != NULL)
			{
				bHeaderEven = true;
			}

			pAP->getAttribute("footer-even", pszFooterEvenID);
			if(pszFooterEvenID != NULL)
			{
				bFooterEven = true;
			}
			pAP->getAttribute("header-first", pszHeaderFirstID);
			if(pszHeaderFirstID != NULL)
			{
				bHeaderFirst = true;
			}

			pAP->getAttribute("footer-first", pszFooterFirstID);
			if(pszFooterFirstID != NULL)
			{
				bFooterFirst = true;
			}
#if 0
			pAP->getAttribute("header-last", pszHeaderLastID);
			if(pszHeaderLastID != NULL)
			{
				bHeaderLast = true;
			}

			pAP->getAttribute("footer-last", pszFooterLastID);
			if(pszFooterLastID != NULL)
			{
				bFooterLast = true;
			}
#endif
			if(bHeader && !bHeaderEven)
			{
			        m_bInBlock = false;
				m_pie->exportHdrFtr("header",pszHeaderID,"header");
			}
			else if(bHeader)
			{
				m_bInBlock = false;
				m_pie->exportHdrFtr("header",pszHeaderID,"headerl");
			}
			if(bHeaderEven)
			{
				m_bInBlock = false;
				m_pie->exportHdrFtr("header-even",pszHeaderEvenID,"headerr");
			}
			if(bHeaderFirst)
			{
				m_bInBlock = false;
				m_pie->exportHdrFtr("header-first",pszHeaderFirstID,"headerf");
			}
			if(bFooter && !bFooterEven)
			{
			        m_bInBlock = false;
				m_pie->exportHdrFtr("footer",pszFooterID,"footer");
			}
			else if(bFooter)
			{
				m_bInBlock = false;
				m_pie->exportHdrFtr("footer",pszFooterID,"footerl");
			}
			if(bFooterEven)
			{
				m_bInBlock = false;
				m_pie->exportHdrFtr("footer-even",pszFooterEvenID,"footerr");
			}
			if(bFooterFirst)
			{
				m_bInBlock = false;
				m_pie->exportHdrFtr("footer-first",pszFooterFirstID,"footerf");
			}
			_closeSpan();
#if 0 // #TF
			_closeBlock();
#endif
			_closeSection();
			_setTabEaten(false);

			m_sdh = sdh;
			_rtf_open_section(pcr->getIndexAP());
			m_bInBlock = false;
			m_bBlankLine = true;
			return true;
		}

	case PTX_SectionHdrFtr:
		{
			_closeSpan();
#if 0 //#TF
			_closeBlock();
#endif
			_closeSection();
			_setTabEaten(false);
			return false;
		}
	case PTX_SectionFootnote:
	    {
			_closeSpan();
			m_bOpennedFootnote = true;

			// we set m_bInBlock to false to prevent issue of \par keyword; the block
			// which gets inserted into the footnote resets this into the normal state, so
			// that when we exit the footnote section, we will be again in block and the
			// block that contains the footnote will get closed as normal
			m_bInBlock = false;
			m_apiSavedBlock = m_apiThisBlock;
			m_sdhSavedBlock = m_sdh;
			_setTabEaten(false);
			m_sdh = sdh;
			m_pie->_rtf_open_brace();
			m_pie->_rtf_keyword("footnote");
			xxx_UT_DEBUGMSG(("_rtf_listenerWriteDoc: Openned Footnote \n"));
			return true;
		}
	case PTX_EndFootnote:
	    {
			_closeSpan();
			_setTabEaten(false);
			m_sdh = m_sdhSavedBlock;
			m_apiThisBlock = m_apiSavedBlock;
			m_pie->_rtf_close_brace();
			xxx_UT_DEBUGMSG(("_rtf_listenerWriteDoc: Closed Footnote \n"));
			return true;
		}
	case PTX_SectionAnnotation:
	    {
			_closeSpan();
			_setTabEaten(false);
			m_bOpennedFootnote = true;

			// we set m_bInBlock to false to prevent issue of \par keyword; the block
			// which gets inserted into the footnote resets this into the normal state, so
			// that when we exit the footnote section, we will be again in block and the
			// block that contains the footnote will get closed as normal
			m_bInBlock = false;
			m_apiSavedBlock = m_apiThisBlock;
			m_sdhSavedBlock = m_sdh;
			m_sdh = sdh;
			const PP_AttrProp * pAnnotationAP = NULL;
			m_pDocument->getAttrProp(pcr->getIndexAP(),&pAnnotationAP);
			const char* pszAuthor;
			const char* pszTitle;
			const char *pszDate;
			if(!pAnnotationAP || !pAnnotationAP->getProperty("annotation-author", (const char *&)pszAuthor))
			{
			    pszAuthor = "n/a";
			}
			if(*pszAuthor == 0)
			{
			    pszAuthor = "n/a";
			}
			m_sAnnAuthor = pszAuthor;
			if(!pAnnotationAP || !pAnnotationAP->getProperty("annotation-title", (const char *&)pszTitle))
			{
			    pszTitle = "n/a";
			}
			if(*pszTitle == 0)
			{
			    pszTitle = "n/a";
			}
			m_sAnnTitle = pszTitle;
			if(!pAnnotationAP || !pAnnotationAP->getProperty("annotation-date", (const char *&)pszDate))
			{
			    pszDate = "n/a";
			}
			if(*pszDate == 0)
			{
			    pszDate = "n/a";
			}
			m_sAnnDate = pszDate;
			//
			// Swap out current buffer for a annotation buffer
			// we'll paste the annotation content back in later
			//
			m_pSavedBuf = m_pie->getByteBuf();
			m_pAnnContent = new UT_ByteBuf(0);
			m_pie->setByteBuf(m_pAnnContent);
			return true;
		}
	case PTX_EndAnnotation:
	    {
	                m_pie->setByteBuf(m_pSavedBuf);
			_closeSpan();
			_setTabEaten(false);
			m_sdh = m_sdhSavedBlock;
			m_apiThisBlock = m_apiSavedBlock;
			return true;
		}

	case PTX_SectionFrame:
	    {
			_closeSpan();
			// see comments under case PTX_SectionFootnote:
			m_bInBlock = false;
			_setTabEaten(false);
			m_sdh = NULL;
			_openFrame(pcr->getIndexAP());
			UT_DEBUGMSG(("_rtf_listenerWriteDoc: openned Frame \n"));
			return true;
		}
	case PTX_EndFrame:
	    {

			_closeSpan();
			_setTabEaten(false);
			m_sdh = sdh;
			_closeFrame();
			return true;
		}
	case PTX_SectionTOC:
	    {
			_closeSpan();

			// see comments under case PTX_SectionFootnote:
			m_bInBlock = false;
			_setTabEaten(pcr->getIndexAP() != 0);
			m_sdh = sdh;
			UT_DEBUGMSG(("_rtf_listenerWriteDoc: Found TOC \n"));
			_writeTOC(pcr->getIndexAP());
			return true;
		}
	case PTX_EndTOC:
	    {

			_closeSpan();
			_setTabEaten(false);
			m_sdh = NULL;
			return true;
		}
	case PTX_SectionEndnote:
	    {
			_closeSpan();
			m_bOpennedFootnote = true;

			// see comments under case PTX_SectionFootnote:
			m_bInBlock = false;
			m_apiSavedBlock = m_apiThisBlock;
			m_sdhSavedBlock = m_sdh;
			_setTabEaten(false);
			m_sdh = sdh;
			m_pie->_rtf_open_brace();
			m_pie->_rtf_keyword("footnote");
			m_pie->_rtf_keyword("ftnalt");
			xxx_UT_DEBUGMSG(("_rtf_listenerWriteDoc: Openned Endnote \n"));
			return true;
		}
	case PTX_EndEndnote:
	    {
			_closeSpan();

			_setTabEaten(false);
			m_sdh = m_sdhSavedBlock;
			m_apiThisBlock = m_apiSavedBlock;
			m_pie->_rtf_close_brace();
			xxx_UT_DEBUGMSG(("_rtf_listenerWriteDoc: Closed Endnote \n"));
			return true;
		}
	case PTX_SectionTable:
	    {
			_closeSpan();
			_setTabEaten(false);
			m_sdh = sdh;
			_open_table(pcr->getIndexAP());
			xxx_UT_DEBUGMSG(("_rtf_listenerWriteDoc: openned table \n"));
			return true;
		}
	case PTX_SectionCell:
	    {
			_closeSpan();
			// in rtf cell is a block, while in AW cell contains a block
			// in order to avoid a superfluous paragraph marker we will pretend that we
			// are not in a block
			// see comments under case PTX_SectionFootnote:
			m_bInBlock = false;
			_setTabEaten(false);
			m_sdh = sdh;
			m_bBlankLine = true; // Need this as well!
			_open_cell(pcr->getIndexAP());
			return true;
		}
	case PTX_EndTable:
	    {
			_closeSpan();
			m_bInBlock = false;
			_setTabEaten(false);
			m_sdh = sdh;
			_close_table();
			return true;
		}
	case PTX_EndCell:
	    {

			_closeSpan();
			m_bInBlock = false;
			_setTabEaten(false);
			m_sdh = sdh;
			_close_cell();
			return true;
		}
	case PTX_Block:
		{
			xxx_UT_DEBUGMSG(("_rtf_listenerWriteDoc: Populate block \n"));
			_closeSpan();
			if(!m_bBlankLine && !m_bOpennedFootnote)
			{
				m_bInBlock = true;
			}
			_closeBlock(pcr->getIndexAP());
			_setListBlock(false);
			_setTabEaten(false);
			m_sdh = sdh;
			_rtf_open_block(pcr->getIndexAP());
			m_bBlankLine = true;	
			m_bInBlock = true;
		return true;
		}

	default:
		UT_ASSERT_NOT_REACHED();
		return false;
	}
}

bool s_RTF_ListenerWriteDoc::change(fl_ContainerLayout* /*sfh*/,
									const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT_NOT_REACHED();	// this function is not used.
	return false;
}

bool s_RTF_ListenerWriteDoc::insertStrux(fl_ContainerLayout* /*sfh*/,
										  const PX_ChangeRecord * /*pcr*/,
										  pf_Frag_Strux* /*sdh*/,
										  PL_ListenerId /* lid */,
										  void (* /*pfnBindHandles*/)(pf_Frag_Strux* /* sdhNew */,
																	  PL_ListenerId /* lid */,
																	  fl_ContainerLayout* /* sfhNew */))
{
	UT_ASSERT_NOT_REACHED();	// this function is not used.
	return false;
}

bool s_RTF_ListenerWriteDoc::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT_NOT_REACHED();	// this function is not used.
	return false;
}

//////////////////////////////////////////////////////////////////

/*
  {info

  {\title     #PCDATA}
  {\author    #PCDATA}
  {\manager   #PCDATA}
  {\company   #PCDATA}
  {\category  #PCDATA}
  {\keywords  #PCDATA}
  {\comment   #PCDATA}
  {\doccomm   #PCDATA}

  TODO:
  \userprops
    \propname
    \proptype
    \staticval

  }
 */
void s_RTF_ListenerWriteDoc::_rtf_info(void)
{
	// Define the number of alternative chars for unicode escapes.
	const UT_uint32 iNumAltChars = 1; 
	// The keys for the information that we'll put in the info block.
	const char * keys[] = {PD_META_KEY_TITLE,      
	                       PD_META_KEY_CREATOR,
	                       PD_META_KEY_CONTRIBUTOR, 
	                       PD_META_KEY_PUBLISHER,
	                       PD_META_KEY_SUBJECT,    
	                       PD_META_KEY_KEYWORDS,
	                       PD_META_KEY_DESCRIPTION,
	                       PD_META_KEY_TYPE,
	                       NULL};
	const char * rtfkeys[] = {"title", "author", "manager", "company", "subject", "keywords", 
	                         "doccomm", "category", NULL};

	if (!m_pie->isCopying ()) {
		std::string propVal ;
		
		m_pie->_rtf_open_brace () ;
		m_pie->_rtf_keyword("info");
		m_pie->_rtf_keyword("uc", iNumAltChars);

		for (UT_uint32 i=0; keys[i]; i++)
		{
			if (m_pDocument->getMetaDataProp (keys[i], propVal) && propVal.size())
			{
				m_pie->_rtf_open_brace () ; 
				m_pie->_rtf_keyword(rtfkeys[i]);
				m_pie->_rtf_pcdata(propVal, iNumAltChars); 
				m_pie->_rtf_close_brace();
			}
		}
		m_pie->_rtf_close_brace();
	}
}

void s_RTF_ListenerWriteDoc::_rtf_docfmt(void)
{
	// emit everything necessary for <docfmt>* portion of the document

	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;

	// <docfmt>

	const gchar * szDefaultTabs = PP_evalProperty("default-tab-interval",
													 pSpanAP,pBlockAP,pSectionAP,
													 m_pDocument,true);
	m_pie->_rtf_keyword_ifnotdefault_twips("deftab",static_cast<const char*>(szDefaultTabs),1440);

	// <docfmt> -- document views and zoom level

	m_pie->_rtf_keyword("viewkind",1);	/* PageLayout */

	// TODO <docfmt> -- footnotes and endnotes

	// <docfmt> -- page information

	UT_String szPaperWidth;
	UT_String szPaperHeight;

	bool landscape = !m_pDocument->m_docPageSize.isPortrait();

	{
		UT_LocaleTransactor t(LC_NUMERIC, "C");
		
		double width = m_pDocument->m_docPageSize.Width(DIM_IN);
		double height = m_pDocument->m_docPageSize.Height(DIM_IN);
			
		UT_String_sprintf(szPaperWidth, "%fin", width);
		UT_String_sprintf(szPaperHeight, "%fin", height);		
	}

	m_pie->_rtf_keyword_ifnotdefault_twips("paperw",szPaperWidth.c_str(),0);
	m_pie->_rtf_keyword_ifnotdefault_twips("paperh",szPaperHeight.c_str(),0);

	const gchar * szLeftMargin = PP_evalProperty("page-margin-left",
													 pSpanAP,pBlockAP,pSectionAP,
													 m_pDocument,true);
	m_pie->_rtf_keyword_ifnotdefault_twips("margl",static_cast<const char*>(szLeftMargin),1800);
	const gchar * szRightMargin = PP_evalProperty("page-margin-right",
													 pSpanAP,pBlockAP,pSectionAP,
													 m_pDocument,true);
	m_pie->_rtf_keyword_ifnotdefault_twips("margr",static_cast<const char*>(szRightMargin),1800);
	const gchar * szTopMargin = PP_evalProperty("page-margin-top",
													 pSpanAP,pBlockAP,pSectionAP,
													 m_pDocument,true);
	m_pie->_rtf_keyword_ifnotdefault_twips("margt",static_cast<const char*>(szTopMargin),1440);
	const gchar * szBottomMargin = PP_evalProperty("page-margin-bottom",
													 pSpanAP,pBlockAP,pSectionAP,
													 m_pDocument,true);
	m_pie->_rtf_keyword_ifnotdefault_twips("margb",static_cast<const char*>(szBottomMargin),1440);

	if (landscape)
		m_pie->_rtf_keyword("landscape");
	m_pie->_rtf_keyword("widowctrl");	// enable widow and orphan control

	// TODO <docfmt> -- linked styles
	// TODO <docfmt> -- compatibility options
	// TODO <docfmt> -- forms
	// TODO <docfmt> -- revision marks
	// TODO <docfmt> -- comments (annotations)
	// TODO <docfmt> -- bidirectional controls
	// TODO <docfmt> -- page borders
}

//////////////////////////////////////////////////////////////////

void s_RTF_ListenerWriteDoc::_rtf_open_section(PT_AttrPropIndex api)
{
	m_apiThisSection = api;

	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;

	m_pDocument->getAttrProp(m_apiThisSection,&pSectionAP);

	const gchar * szColumns = PP_evalProperty("columns",
												 pSpanAP,pBlockAP,pSectionAP,
												 m_pDocument,true);
	const gchar * szColumnGap = PP_evalProperty("column-gap",
												   pSpanAP,pBlockAP,pSectionAP,
												   m_pDocument,true);

	const gchar * szColumnLine = PP_evalProperty("column-line",
													pSpanAP,pBlockAP,pSectionAP,
													m_pDocument,true);


	const gchar * szMarginLeft = PP_evalProperty("page-margin-left",
												 pSpanAP,pBlockAP,pSectionAP,
												 m_pDocument,true);

	const gchar * szMarginTop = PP_evalProperty("page-margin-top",
												 pSpanAP,pBlockAP,pSectionAP,
												 m_pDocument,true);

	const gchar * szMarginRight = PP_evalProperty("page-margin-right",
												 pSpanAP,pBlockAP,pSectionAP,
												 m_pDocument,true);

	const gchar * szMarginBottom = PP_evalProperty("page-margin-bottom",
												 pSpanAP,pBlockAP,pSectionAP,
												 m_pDocument,true);

	const gchar * szHeaderY = PP_evalProperty("page-margin-header",
												 pSpanAP,pBlockAP,pSectionAP,
												 m_pDocument,true);

	const gchar * szFooterY = PP_evalProperty("page-margin-footer",
												 pSpanAP,pBlockAP,pSectionAP,
												 m_pDocument,true);

// 	const gchar * szSpaceAfter = PP_evalProperty("section-space-after",
// 												 pSpanAP,pBlockAP,pSectionAP,
// 												 m_pDocument,true);

	const gchar * szRestartNumbering = PP_evalProperty("section-restart",
												 pSpanAP,pBlockAP,pSectionAP,
												 m_pDocument,true);

	const gchar * szRestartAt = PP_evalProperty("section-restart-value",
												 pSpanAP,pBlockAP,pSectionAP,
												 m_pDocument,true);
	const gchar * szHeaderExists = NULL;
	pSectionAP->getAttribute("header", szHeaderExists);
	const gchar * szFooterExists = NULL;
	pSectionAP->getAttribute("footer", szFooterExists);
	const gchar * szDomDir = PP_evalProperty("dom-dir",
												 pSpanAP,pBlockAP,pSectionAP,
												 m_pDocument,true);

	bool bSectRTL = strcmp (szDomDir,"rtl") == 0;

	bool bColLine = false;
	if (szColumnLine && !strcmp (szColumnLine, "on"))
		bColLine = true;

	// TODO add other properties here

	m_pie->_rtf_nl();
	_closeSpan();                   // In case it's open.
	if(m_bStartedList)
	{
		m_pie->_rtf_close_brace();
		m_bStartedList = false;
	}
	if (m_bJustStartingDoc)			// 'sect' is a delimiter, rather than a plain start
	{

		m_bJustStartingDoc = false;
	}
	else
		m_pie->_rtf_keyword("sect");							// begin a new section
	m_bJustStartingSection = true;

	m_pie->_rtf_keyword("sectd");								// restore all defaults for this section
	m_pie->_rtf_keyword("sbknone");								// no page break implied
	m_pie->_rtf_keyword_ifnotdefault("cols",static_cast<const char*>(szColumns),1);
	m_pie->_rtf_keyword_ifnotdefault_twips("colsx",static_cast<const char*>(szColumnGap),720);

	{
		UT_LocaleTransactor t(LC_NUMERIC, "C");
		
		if (bColLine)
		{
			m_pie->_rtf_keyword ("linebetcol");
		}
		if(szHeaderY)
		{
			double hMarg = UT_convertToInches(szHeaderY);
			UT_String sHeaderY;
			
			UT_String_sprintf(sHeaderY,"%fin",hMarg);
			m_pie->_rtf_keyword_ifnotdefault_twips("headery", static_cast<const char*>(sHeaderY.c_str()), 720);			
		}
		if(szFooterY)
		{
			double hMarg = UT_convertToInches(szFooterY);
			UT_String sFooterY;
			
			UT_String_sprintf(sFooterY,"%fin",hMarg);
			m_pie->_rtf_keyword_ifnotdefault_twips("footery", static_cast<const char*>(sFooterY.c_str()), 720);			
		}
		if(szMarginTop)
		{
			double tMarg = UT_convertToInches(szMarginTop);
			UT_String sRtfTop;
			
			
			UT_String_sprintf(sRtfTop,"%fin",tMarg);
			m_pie->_rtf_keyword_ifnotdefault_twips("margtsxn", static_cast<const char*>(sRtfTop.c_str()), 1440);
		}

		if(szMarginBottom)
		{
			double bMarg = UT_convertToInches(szMarginBottom);
			UT_String sRtfBot;
			UT_String_sprintf(sRtfBot,"%fin",bMarg);
			m_pie->_rtf_keyword_ifnotdefault_twips("margbsxn", static_cast<const char*>(sRtfBot.c_str()), 1440);
		}
	}

	if(szMarginLeft)
	{
		m_pie->_rtf_keyword_ifnotdefault_twips("marglsxn", static_cast<const char*>(szMarginLeft), 1440);
	}
	if(szMarginRight)
	{
		m_pie->_rtf_keyword_ifnotdefault_twips("margrsxn", static_cast<const char*>(szMarginRight), 1440);
	}

	if(szRestartNumbering && strcmp(szRestartNumbering,"1") == 0)
	{
		m_pie->_rtf_keyword("pgnrestart");
		if(szRestartAt)
		{
			UT_sint32 num = atoi(szRestartAt);
			m_pie->_rtf_keyword("pgnx",num);
		}
	}
	else
	{
		m_pie->_rtf_keyword("pgncont");
	}

	if (bSectRTL)
		m_pie->_rtf_keyword("rtlsect");
	else
		m_pie->_rtf_keyword("ltrsect");
}

//////////////////////////////////////////////////////////////////

void s_RTF_ListenerWriteDoc::_rtf_open_block(PT_AttrPropIndex api)
{
	m_apiThisBlock = api;

	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;

	m_pDocument->getAttrProp(m_apiThisSection,&pSectionAP);
	m_pDocument->getAttrProp(m_apiThisBlock,&pBlockAP);

	m_pie->_rtf_nl();

	// 'par' is a delimiter, rather than a plain start.
	// NO! par is \r and closes a paragraph, not opens it
	// also, block-level character properties are applied to \par so it really matters
	// that gets associated with the correct paragraph
	
	if(m_bStartedList && !m_bInFrame && !m_bOpennedFootnote )
	{
		m_pie->_rtf_close_brace();
	}
	m_bStartedList = false;

	//
	// If span was openned in a previous closeBlock because of a blank line
	// close it now.
	//
	_closeSpan();
	
	m_pie->_write_parafmt(pSpanAP, pBlockAP, pSectionAP, m_bStartedList, m_sdh, m_currID, m_bIsListBlock,
						  m_Table.getNestDepth());

	m_bJustStartingSection = false;
	m_bOpennedFootnote = false;
	m_bJustOpennedFrame = false;
	if(m_Table.getNestDepth() > 0 && m_Table.isCellJustOpenned())
	{
		m_Table.setCellJustOpenned(false);
	}
	
	
	m_pie->_output_revision(s_RTF_AttrPropAdapter_AP(pSpanAP, pBlockAP, pSectionAP, m_pDocument),true,m_sdh,
							m_Table.getNestDepth(), m_bStartedList, m_bIsListBlock, m_currID);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
void s_RTF_ListenerWriteDoc::_writeBookmark(const PX_ChangeRecord_Object * pcro)
{
	PT_AttrPropIndex api = pcro->getIndexAP();
	const PP_AttrProp * pBookmarkAP = NULL;
	m_pDocument->getAttrProp(api,&pBookmarkAP);

	const gchar * szType = NULL;
	bool bFound = pBookmarkAP->getAttribute("type", szType);
	if (!bFound) {
		UT_DEBUGMSG (("RTF_Export: cannot get type for bookmark\n"));
		return;
	}
	const gchar * szName = NULL;
	bFound = pBookmarkAP->getAttribute("name", szName);
	if (!bFound) {
		UT_DEBUGMSG (("RTF_Export: cannot get name for bookmark\n"));
		return;
	}
	m_pie->_rtf_open_brace();
	{
		m_pie->_rtf_keyword("*");
		if (strcmp (szType, "start") == 0) {
			m_pie->_rtf_keyword("bkmkstart");
		}
		else if (strcmp (szType, "end") == 0) {
			m_pie->_rtf_keyword("bkmkend");
		}
		m_pie->_rtf_chardata(szName, strlen(szName));
		m_pie->_rtf_close_brace();
	}
}


void s_RTF_ListenerWriteDoc::_writeRDFAnchor(const PX_ChangeRecord_Object * pcro)
{
	UT_DEBUGMSG(("_writeRDFAnchor() pcro:%p\n", pcro ));
	
	PT_AttrPropIndex api = pcro->getIndexAP();
	const PP_AttrProp * pAP = NULL;
	m_pDocument->getAttrProp(api,&pAP);
	RDFAnchor a(pAP);
	
	m_pie->_rtf_open_brace();
	{
		m_pie->_rtf_keyword("*");
		std::string xmlid = a.getID();
		if( a.isEnd() )
		{
			UT_DEBUGMSG(("_writeRDFAnchor() end... id:%s\n", xmlid.c_str() ));
			m_bRDFAnchorOpen = false;
			m_pie->_rtf_keyword("rdfanchorend");

			//
			// Allow outer xmlid tag to close before inner one.
			//
			std::list< std::string >::iterator iter = find( m_rdfAnchorStack.begin(),
															m_rdfAnchorStack.end(),
															xmlid );
			if( iter == m_rdfAnchorStack.end() )
			{
				UT_DEBUGMSG(("_writeRDFAnchor() XXX closing an RDF Anchor which is not open!... id:%s\n",
							 xmlid.c_str() ));
			}
			else
			{
				m_rdfAnchorStack.erase( iter );
			}
//			m_rdfAnchorStack.pop_back();
		}
		else
		{
			UT_DEBUGMSG(("_writeRDFAnchor() start... id:%s\n", xmlid.c_str() ));
			m_bRDFAnchorOpen = true;
			m_pie->_rtf_keyword("rdfanchorstart");
			m_rdfAnchorStack.push_back( xmlid );
		}
		m_pie->_rtf_chardata( xmlid.c_str(), xmlid.length());
	}
	m_pie->_rtf_close_brace();
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
void s_RTF_ListenerWriteDoc::_writeHyperlink(const PX_ChangeRecord_Object * pcro)
{
	PT_AttrPropIndex api = pcro->getIndexAP();
	const PP_AttrProp * pHyperlinkAP = NULL;
	m_pDocument->getAttrProp(api,&pHyperlinkAP);

	const gchar * szHyper = NULL;
	bool bFound = pHyperlinkAP->getAttribute("xlink:href", szHyper);
	if (!bFound)
	{
		UT_DEBUGMSG (("RTF_Export: cannot get address for hyperlink\n"));
		return;
	}
	_writeFieldPreamble(pHyperlinkAP);
	m_pie->write("HYPERLINK ");
	m_pie->write("\"");
	m_pie->write(szHyper);
	m_pie->write("\"");
	m_bHyperLinkOpen = true;
	m_pie->_rtf_close_brace();
	m_pie->_rtf_close_brace();
	m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword("*");
	m_pie->_rtf_keyword("fldrslt");
}



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
void s_RTF_ListenerWriteDoc::_writeAnnotation(const PX_ChangeRecord_Object * pcro)
{
	PT_AttrPropIndex api = pcro->getIndexAP();
	const PP_AttrProp * pAnnotationAP = NULL;
	m_pDocument->getAttrProp(api,&pAnnotationAP);
	UT_return_if_fail(pAnnotationAP);

	const gchar * szAnn = NULL;
	bool bFound = pAnnotationAP->getAttribute("annotation", szAnn);
	if (!bFound)
	{
		UT_DEBUGMSG (("RTF_Export: cannot get address for Annotation\n"));
		return;
	}
	m_iAnnotationNumber = UT_newNumber();
	m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword("*");
	m_pie->_rtf_keyword_space("atrfstart",m_iAnnotationNumber);
	m_bAnnotationOpen = true;
}


void s_RTF_ListenerWriteDoc::_writeImageInRTF(const PX_ChangeRecord_Object * pcro)
{
	PT_AttrPropIndex api = pcro->getIndexAP();
	const PP_AttrProp * pImageAP = NULL;
	m_pDocument->getAttrProp(api,&pImageAP);

	// fetch the "name" of the image and use it to fetch the actual image data.

	const gchar * szDataID = NULL;
	bool bFoundDataID = pImageAP->getAttribute("dataid",szDataID);
	if (!bFoundDataID)
	{
		UT_DEBUGMSG(("RTF_Export: cannot get dataid for image\n"));
		return;
	}
	const UT_ByteBuf * pbb = NULL;
	std::string mimetype;
	bool bFoundDataItem = m_pDocument->getDataItemDataByName(szDataID,
                                                             &pbb, &mimetype, 
															 NULL);
	if (!bFoundDataItem)
	{
		UT_DEBUGMSG(("RTF_Export: cannot get dataitem for image\n"));
		return;
	}

	// see if the image has a width/height attribute that should
	// override the actual pixel size of the image.

	const gchar * szWidthProp = NULL;
	const gchar * szHeightProp = NULL;
	const gchar * szCroplProp = NULL;
	const gchar * szCroprProp = NULL;
	const gchar * szCroptProp = NULL;
	const gchar * szCropbProp = NULL;
	bool bFoundWidthProperty = pImageAP->getProperty("width",szWidthProp);
	bool bFoundHeightProperty = pImageAP->getProperty("height",szHeightProp);
	bool bFoundCropl = pImageAP->getProperty ("cropl",szCroplProp);
	bool bFoundCropr = pImageAP->getProperty ("cropr",szCroprProp);
	bool bFoundCropt = pImageAP->getProperty ("cropt",szCroptProp);
	bool bFoundCropb = pImageAP->getProperty ("cropb",szCropbProp);


	// if everything is ok, we need to dump the image data (in hex)
	// to the RTF stream with some screwy keywords...
	//
	// we need to emit:     {\*\shppict{\pict <stuff>}}
	// we do not deal with: {\*\nonshppict...}
	//
	// <stuff> ::= <brdr>? <shading>? <pictype> <pictsize> <metafileinfo>? <data>

	m_pie->_rtf_open_brace();
	{
		m_pie->_rtf_keyword("*");
		m_pie->_rtf_keyword("shppict");
		m_pie->_rtf_open_brace();
		{
			m_pie->_rtf_keyword("pict");
			// TODO deal with <brdr>
			// TODO deal with <shading>

			// <pictype> -- we store everything internall as PNG, so that's all
			//              we output here.  TODO consider listing multiple formats
			//              here -- word97 seems to, but this really bloats the file.

			// get the width/height of the image from the image itself.

			UT_sint32 iImageWidth, iImageHeight;
			iImageWidth = iImageHeight = 0;

			if(mimetype == "image/png") {
				m_pie->_rtf_keyword("pngblip");
				UT_PNG_getDimensions(pbb,iImageWidth,iImageHeight);
			}
			else if(mimetype == "image/jpeg") 
			{
				m_pie->_rtf_keyword("jpegblip");
				UT_JPEG_getDimensions(pbb,iImageWidth,iImageHeight);
			}
			else if (mimetype == "image/svg+xml")
			{
				m_pie->_rtf_keyword("svgblip");
				UT_sint32 layoutwidth,layoutheight;
				UT_SVG_getDimensions(pbb,NULL,iImageWidth,iImageHeight,layoutwidth,layoutheight);
			}

			// compute scale factors...

			double dImageWidth = static_cast<double>(iImageWidth);
			double dImageHeight = static_cast<double>(iImageHeight);
			dImageWidth = UT_convertDimToInches(dImageWidth,DIM_PT);
			dImageHeight = UT_convertDimToInches(dImageHeight,DIM_PT);

			// <pictsize>

			m_pie->_rtf_keyword("picw",iImageWidth);
			m_pie->_rtf_keyword("pich",iImageHeight);
			if (bFoundWidthProperty)
			{
			        double dWidth = UT_convertToInches(szWidthProp);   // Our "goal" width is _before_ scaling
				double scalex = dWidth/dImageWidth;                // How intuitive!
			        const gchar * szWidthGoal = UT_convertInchesToDimensionString(DIM_IN, dImageWidth,".4");
				m_pie->_rtf_keyword_ifnotdefault_twips("picwgoal",static_cast<const char*>(szWidthGoal),0);
				UT_uint32 iscalex = static_cast<UT_uint32>(100.0*scalex);
				m_pie->_rtf_keyword("picscalex",iscalex);

			}
			if (bFoundHeightProperty)
			{
			        double dHeight = UT_convertToInches(szHeightProp);
				double scaley = dHeight/dImageHeight;
			        const gchar * szHeightGoal = UT_convertInchesToDimensionString(DIM_IN, dImageHeight,".4");
				m_pie->_rtf_keyword_ifnotdefault_twips("pichgoal",static_cast<const char*>(szHeightGoal),0);
				UT_uint32 iscaley = static_cast<UT_uint32>(100.0*scaley);
				m_pie->_rtf_keyword("picscaley",iscaley);
			}
			if (bFoundCropl)
			{
				m_pie->_rtf_keyword_ifnotdefault_twips("piccropl",static_cast<const char*>(szCroplProp),0);
			}
			if (bFoundCropr)
			{
				m_pie->_rtf_keyword_ifnotdefault_twips("piccropr",static_cast<const char*>(szCroprProp),0);
			}
			if (bFoundCropt)
			{
				m_pie->_rtf_keyword_ifnotdefault_twips("piccropt",static_cast<const char*>(szCroptProp),0);
			}
			if (bFoundCropb)
			{
				m_pie->_rtf_keyword_ifnotdefault_twips("piccropb",static_cast<const char*>(szCropbProp),0);
			}


			// TODO deal with <metafileinfo>

			// <data>

			// TODO create meaningful values for bliptag and bliduid...
			// we emit "\bliptag<N>{\*\blipuid <N16>}"
			// where <N> is an integer.
			// where <N16> is a 16-byte integer in hex.

			m_pie->_rtf_nl();
			UT_uint32 tag = UT_newNumber ();
			m_pie->_rtf_keyword("bliptag",tag);
			m_pie->_rtf_open_brace();
			{
				m_pie->_rtf_keyword("*");
				m_pie->_rtf_keyword("blipuid");
				UT_String buf;
				UT_String_sprintf(buf,"%032x",tag);
				m_pie->_rtf_chardata(buf.c_str(),buf.size());
			}
			m_pie->_rtf_close_brace();

			UT_uint32 lenData = pbb->getLength();
			const UT_Byte * pData = pbb->getPointer(0);
			UT_uint32 k;

			for (k=0; k<lenData; k++)
			{
				if (k%32==0)
					m_pie->_rtf_nl();
				UT_String buf;
				UT_String_sprintf(buf,"%02x",pData[k]);
				m_pie->_rtf_chardata(buf.c_str(),2);
			}
		}
		m_pie->_rtf_close_brace();
	}
	m_pie->_rtf_close_brace();
}
