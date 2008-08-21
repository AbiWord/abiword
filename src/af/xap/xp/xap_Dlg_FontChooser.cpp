/* AbiSource Application Framework
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
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "xap_Dlg_FontChooser.h"
#include "ut_string_class.h"
#include "gr_Graphics.h"
#include "gr_Painter.h"

/*****************************************************************/

// your typographer's standard nonsense latin font phrase
#define PREVIEW_ENTRY_DEFAULT_STRING	"Lorem ipsum dolor sit amet, consectetaur adipisicing..."

XAP_Dialog_FontChooser::XAP_Dialog_FontChooser(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialogfont")
{
	m_answer				= a_CANCEL;
	m_pGraphics				= NULL;
	m_pColorBackground      = NULL;
	m_pFontPreview          = NULL;
	m_pFontFamily			= NULL;
	m_pFontSize				= NULL;
	m_pFontWeight			= NULL;
	m_pFontStyle			= NULL;
	m_pColor				= NULL;
	m_pBGColor				= NULL;
	m_bUnderline			= false;
	m_bOverline				= false;
	m_bStrikeout			= false;
	m_bTopline		    	= false;
	m_bBottomline			= false;
	m_bHidden               = false;
	m_bSuperScript			= false;
	m_bSubScript			= false;
	m_bChangedFontFamily	= false;
	m_bChangedFontSize		= false;
	m_bChangedFontWeight	= false;
	m_bChangedFontStyle		= false;
	m_bChangedColor			= false;
	m_bChangedBGColor       = false;
	m_bChangedUnderline		= false;
	m_bChangedOverline		= false;
	m_bChangedStrikeOut		= false;
	m_bChangedTopline		= false;
	m_bChangedBottomline   	= false;
	m_bChangedHidden        = false;
	m_bChangedSuperScript	= false;
	m_bChangedSubScript		= false;

	if(m_vecProps.getItemCount() > 0)
		m_vecProps.clear();

	UT_UCS4_cloneString_char (&m_drawString, PREVIEW_ENTRY_DEFAULT_STRING);
}

XAP_Dialog_FontChooser::~XAP_Dialog_FontChooser(void)
{
	FREEP(m_pFontFamily);
	FREEP(m_pFontSize);
	FREEP(m_pFontWeight);
	FREEP(m_pFontStyle);
	FREEP(m_pColor);
	FREEP(m_pBGColor);
	FREEP(m_drawString);
	DELETEP(m_pFontPreview);
}

void XAP_Dialog_FontChooser::setDrawString(const UT_UCSChar * str)
{
	FREEP(m_drawString);
	UT_sint32 len = UT_UCS4_strlen(str);
	if(len <= 0)
	{
		UT_UCS4_cloneString_char (&m_drawString, PREVIEW_ENTRY_DEFAULT_STRING);
	}
	else
	{
	    UT_UCS4_cloneString(&m_drawString, str);
	}
}

void XAP_Dialog_FontChooser::setGraphicsContext(GR_Graphics * pGraphics)
{
	m_pGraphics = pGraphics;
}

void XAP_Dialog_FontChooser::_createFontPreviewFromGC(GR_Graphics * gc,
													  UT_uint32 width,
													  UT_uint32 height)
{
	UT_ASSERT(gc);

	m_pFontPreview = new XAP_Preview_FontPreview(gc,m_pColorBackground);
	UT_return_if_fail(m_pFontPreview);

	m_pFontPreview->setWindowSize(width, height);
	m_pFontPreview->setVecProperties( & m_vecProps);
}

void XAP_Dialog_FontChooser::addOrReplaceVecProp(const gchar * pszProp,
												 const gchar * pszVal)
{
	UT_sint32 iCount = m_vecProps.getItemCount();
	const char * pszV = NULL;
	if(iCount <= 0)
	{
		m_vecProps.addItem((void *) pszProp);
		m_vecProps.addItem((void *) pszVal);
		return;
	}
	UT_sint32 i = 0;
	for(i=0; i < iCount ; i += 2)
	{
		pszV = (const gchar *) m_vecProps.getNthItem(i);
		if( (pszV != NULL) && (strcmp( pszV,pszProp) == 0))
			break;
	}
	if(i < iCount)
		m_vecProps.setNthItem(i+1, (void *) pszVal, NULL);
	else
	{
		m_vecProps.addItem((void *) pszProp);
		m_vecProps.addItem((void *) pszVal);
	}
	return;
}

/*!
 * This method updates the drawing area in the dialog.
 */
void XAP_Dialog_FontChooser::event_previewExposed(const UT_UCSChar * pszChars)
{
	UT_UCSChar * pszNew = NULL;
	if(!pszChars || UT_UCS4_strlen(pszChars) <= 0)
	{
		//FREEP(pszChars); // we should not g_free it here
		UT_UCS4_cloneString_char (&pszNew, PREVIEW_ENTRY_DEFAULT_STRING);
		if (!pszNew)
			return;

		m_pFontPreview->setDrawString(pszNew);
	}
	else
	{
		m_pFontPreview->setDrawString(pszChars);
	}
	m_pFontPreview->draw();
	
	FREEP(pszNew);
}


/*!
 * This method clears the drawing area in the dialog.
 */
void XAP_Dialog_FontChooser::event_previewClear(void)
{
	m_pFontPreview->clearScreen();
}

/*!
 * This method returns a pointer to the const char * value associated with the
 * the property szProp. Stolen from ap_Dialog_Lists.
 * It assumes properties and values are stored the array like this:
 * vecProp(n)   :   vecProp(n+1)
 * "property"   :   "value"
 */
const gchar * XAP_Dialog_FontChooser::getVal(const gchar * szProp) const
{
	UT_sint32 i = m_vecProps.getItemCount();
	if(i <= 0)
		return NULL;
	UT_sint32 j;
	const gchar * pszV = NULL;
	for(j= 0; j<i ;j=j+2)
	{
		pszV = (const gchar *) m_vecProps.getNthItem(j);
		if( (pszV != NULL) && (strcmp( pszV,szProp) == 0))
			break;
	}
	if( j < i )
		return  (const gchar *) m_vecProps.getNthItem(j+1);
	else
		return NULL;
}

/*!
 * This method sets all the local properties from a vector of pointers
 * to const gchar * strings of Property - Value pairs.
 * This method wipes out all the old values and clears all the bools
 * assciated with them.
 */
void XAP_Dialog_FontChooser::setAllPropsFromVec(UT_Vector * vProps)
{
	UT_sint32 remCount = vProps->getItemCount();
	if(remCount <= 0)
		return;
	UT_sint32 locCount = m_vecProps.getItemCount();
	if(locCount>=0)
		m_vecProps.clear();
	UT_sint32 i = 0;
	for(i=0; i< remCount; i++)
	{
		m_vecProps.addItem(vProps->getNthItem(i));
	}
//
// Do the Text decorations
//
	const gchar * s = NULL;
	s = getVal("text-decoration");
	m_bUnderline = (NULL != strstr(s,"underline"));
	m_bOverline = (NULL != strstr(s,"overline"));
	m_bStrikeout = (NULL != strstr(s,"line-through"));
	m_bTopline = (NULL != strstr(s,"topline"));
	m_bBottomline = (NULL != strstr(s,"bottomline"));

	s = getVal("display");
	m_bHidden = !strcmp(s,"none");
	
	s = getVal("text-position");
	m_bSuperScript = strcmp(s,"superscript")==0;
	m_bSubScript = strcmp(s,"subscript")==0;
}

void XAP_Dialog_FontChooser::setFontFamily(const gchar * pFontFamily)
{
	CLONEP((char *&) m_pFontFamily, pFontFamily);
	addOrReplaceVecProp("font-family",pFontFamily);
}

void XAP_Dialog_FontChooser::setFontSize(const gchar * pFontSize)
{
	CLONEP((char *&) m_pFontSize, pFontSize);
	addOrReplaceVecProp("font-size",pFontSize);
}

void XAP_Dialog_FontChooser::setFontWeight(const gchar * pFontWeight)
{
	CLONEP((char *&) m_pFontWeight, pFontWeight);
	addOrReplaceVecProp("font-weight",pFontWeight);
}

void XAP_Dialog_FontChooser::setFontStyle(const gchar * pFontStyle)
{
	CLONEP((char *&)m_pFontStyle, pFontStyle);
	addOrReplaceVecProp("font-style",pFontStyle);
}

void XAP_Dialog_FontChooser::setColor(const gchar * pColor)
{
	CLONEP((char *&)m_pColor, pColor);
	addOrReplaceVecProp("color",pColor);
}

void XAP_Dialog_FontChooser::setBGColor(const gchar * pBGColor)
{
	CLONEP((char *&)m_pBGColor, pBGColor);
	addOrReplaceVecProp("bgcolor",pBGColor);
}

void XAP_Dialog_FontChooser::setSuperScript(bool bSuperScript)
{
	static char none[] = "superscript";
	static char empty[]  = "";

	if(bSuperScript)
	{
		addOrReplaceVecProp("text-position",none);
	}
	else
	{
		addOrReplaceVecProp("text-position",empty);
	}
	m_bSuperScript = bSuperScript;
	
}

void XAP_Dialog_FontChooser::setSubScript(bool bSubScript)
{
	static char none[] = "subscript";
	static char empty[]  = "";

	if(bSubScript)
	{
		addOrReplaceVecProp("text-position",none);
	}
	else
	{
		addOrReplaceVecProp("text-position",empty);
	}
	m_bSubScript = bSubScript;
	
}

void XAP_Dialog_FontChooser::setHidden(bool bHidden)
{
	static char none[] = "none";
	static char empty[]  = "";

	if(bHidden)
	{
		addOrReplaceVecProp("display",none);
	}
	else
	{
		addOrReplaceVecProp("display",empty);
	}
	m_bHidden = bHidden;
}

void XAP_Dialog_FontChooser::setBackGroundColor(const gchar * pBackground)
{
	m_pColorBackground = pBackground;
}

void XAP_Dialog_FontChooser::setFontDecoration(bool bUnderline, bool bOverline, bool bStrikeOut, bool bTopline, bool bBottomline)
{
	m_bUnderline = bUnderline;
	m_bOverline = bOverline;
	m_bStrikeout = bStrikeOut;
	m_bTopline = bTopline;
	m_bBottomline = bBottomline;

	static gchar s[50];
	UT_String decors;
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
	addOrReplaceVecProp("text-decoration",(const gchar *) s);
}

XAP_Dialog_FontChooser::tAnswer XAP_Dialog_FontChooser::getAnswer(void) const
{
	return m_answer;
}

bool XAP_Dialog_FontChooser::getChangedFontFamily(const gchar ** pszFontFamily) const
{
	bool bchanged = didPropChange(m_pFontFamily,getVal("font-family"));
	bool useVal = (bchanged && !m_bChangedFontFamily);
	if (pszFontFamily && useVal)
		*pszFontFamily = getVal("font-family");
	else if(pszFontFamily)
		*pszFontFamily = m_pFontFamily ;
	return bchanged;
}

bool XAP_Dialog_FontChooser::getChangedFontSize(const gchar ** pszFontSize) const
{
	bool bchanged = didPropChange(m_pFontSize,getVal("font-size"));
	bool useVal = (bchanged && !m_bChangedFontSize);
	if (pszFontSize && useVal)
		*pszFontSize = getVal("font-size");
	else if(pszFontSize)
		*pszFontSize = m_pFontSize ;
	return bchanged;
}

bool XAP_Dialog_FontChooser::getChangedFontWeight(const gchar ** pszFontWeight) const
{
	bool bchanged = didPropChange(m_pFontWeight,getVal("font-weight"));
	bool useVal = (bchanged && !m_bChangedFontWeight);
	if (pszFontWeight && useVal)
		*pszFontWeight = getVal("font-weight");
	else if(pszFontWeight)
		*pszFontWeight = m_pFontWeight ;
	return bchanged;
}

bool XAP_Dialog_FontChooser::getChangedFontStyle(const gchar ** pszFontStyle) const
{
	bool bchanged = didPropChange(m_pFontStyle,getVal("font-style"));
	bool useVal = (bchanged && !m_bChangedFontStyle);
	if (pszFontStyle && useVal)
		*pszFontStyle = getVal("font-style");
	else if(pszFontStyle)
		*pszFontStyle = m_pFontStyle ;
	return bchanged;
}

bool XAP_Dialog_FontChooser::getChangedBGColor(const gchar ** pszBGColor) const
{
	bool bchanged = didPropChange(m_pBGColor,getVal("bgcolor"));
	bool useVal = (bchanged && !m_bChangedBGColor);
	if (pszBGColor && useVal)
		*pszBGColor = getVal("bgcolor");
	else if(pszBGColor)
		*pszBGColor = m_pBGColor ;
	return bchanged;
}


bool XAP_Dialog_FontChooser::getChangedColor(const gchar ** pszColor) const
{
	bool bchanged = didPropChange(m_pColor,getVal("color"));
	bool useVal = (bchanged && !m_bChangedColor);
	if (pszColor && useVal)
		*pszColor = getVal("color");
	else if(pszColor)
		*pszColor = m_pColor ;
	return bchanged;
}

/*!
 * Compare two prop values and gracefully handle the cases of NULL pointers
 */
bool XAP_Dialog_FontChooser::didPropChange(const gchar * v1, const gchar * v2) const
{
	if(v1 == NULL && v2 == NULL)
		return false;
	if(v1 == NULL || v2 == NULL)
		return true;
	return (strcmp(v1,v2) != 0);
}

bool XAP_Dialog_FontChooser::getChangedHidden(bool * pbHidden) const
{
	if (pbHidden)
		*pbHidden = m_bHidden;
	return m_bChangedHidden;
}

bool XAP_Dialog_FontChooser::getChangedSuperScript(bool * pbSuperScript) const
{
	if (pbSuperScript)
		*pbSuperScript = m_bSuperScript;
	return m_bChangedSuperScript;
}

bool XAP_Dialog_FontChooser::getChangedSubScript(bool * pbSubScript) const
{
	if (pbSubScript)
		*pbSubScript = m_bSubScript;
	return m_bChangedSubScript;
}

bool XAP_Dialog_FontChooser::getChangedUnderline(bool * pbUnderline) const
{
	if (pbUnderline)
		*pbUnderline = m_bUnderline;
	return m_bChangedUnderline;
}

bool XAP_Dialog_FontChooser::getChangedOverline(bool * pbOverline) const
{
	if (pbOverline)
		*pbOverline = m_bOverline;
	return m_bChangedOverline;
}

bool XAP_Dialog_FontChooser::getChangedStrikeOut(bool * pbStrikeOut) const
{
	if (pbStrikeOut)
		*pbStrikeOut = m_bStrikeout;
	return m_bChangedStrikeOut;
}

bool XAP_Dialog_FontChooser::getChangedTopline(bool * pbTopline) const
{
	if (pbTopline)
		*pbTopline = m_bTopline;
	return m_bChangedTopline;
}

bool XAP_Dialog_FontChooser::getChangedBottomline(bool * pbBottomline) const
{
	if (pbBottomline)
		*pbBottomline = m_bBottomline;
	return m_bChangedBottomline;
}

/////////////////////////////////////////////////////////////////////////

XAP_Preview_FontPreview::XAP_Preview_FontPreview(GR_Graphics * gc, const gchar * pszClrBackground)
	: XAP_Preview(gc),
		m_pFont(NULL),
		m_iAscent(0),
		m_iDescent(0),
		m_iHeight(0)
{
	if(pszClrBackground != NULL && strcmp(pszClrBackground,"transparent")!=0)
		UT_parseColor(pszClrBackground,m_clrBackground);
	else
		UT_setColor(m_clrBackground,255,255,255);

}

XAP_Preview_FontPreview::~XAP_Preview_FontPreview()
{
}

/*!
 * This method assigns a pointer to a
 *  vector with Char * strings of span-level properties
 * The vector has const gchar * string in the order
 * (n) Property (n+1) Value
 *
 * This code stolen from ap_Dialog_Lists.cpp
 */
void XAP_Preview_FontPreview::setVecProperties( const UT_Vector * vFontProps)
{
	m_vecProps = const_cast<UT_Vector *>(vFontProps);
}

/*!
 * This method returns a pointer to the const char * value associated with the
 * the property szProp. Stolen from ap_Dialog_Lists.
 */
const gchar * XAP_Preview_FontPreview::getVal(const gchar * szProp)
{
	UT_sint32 i = m_vecProps->getItemCount();
	if(i <= 0)
		return NULL;
	UT_sint32 j;
	const gchar * pszV = NULL;
	for(j= 0; j<i ;j=j+2)
	{
		pszV = (const gchar *) m_vecProps->getNthItem(j);
		if( (pszV != NULL) && (strcmp( pszV,szProp) == 0))
			break;
	}
	if( j < i )
		return  (const gchar *) m_vecProps->getNthItem(j+1);
	else
		return NULL;
}

/*
 *
 * Finally draw the characters in the preview.
 *
 */
void XAP_Preview_FontPreview::draw(void)
{
//
// Get text decorations.
//
	bool isUnder,isOver,isStrike;

	const gchar * pDecor = getVal("text-decoration");
	if(pDecor)
	{
		isUnder = (NULL != strstr(pDecor,"underline"));
		isOver = (NULL != strstr(pDecor,"overline"));
		isStrike = (NULL != strstr(pDecor,"line-through"));
	}
	else
	{
		isUnder = false;
		isOver = false;
		isStrike = false;
	}

//
// Do foreground and background colors.
//
	UT_RGBColor FGcolor(0,0,0);
	const char * pszFGColor = getVal("color");
	if(pszFGColor)
		UT_parseColor(getVal("color"),FGcolor);
	UT_RGBColor BGcolor(m_clrBackground);
	const char * pszBGColor = getVal("bgcolor");
	if(pszBGColor && strcmp(pszBGColor,"transparent") != 0)
		UT_parseColor(getVal("bgcolor"),BGcolor);
//
// Get the font and bold/italic- ness
//
	//GR_Font * pFont;
	const char* pszFamily	= getVal("font-family");
	const char* pszStyle	= getVal("font-style");
	const char* pszVariant	= getVal("font-variant");
	const char* pszWeight	= getVal("font-weight");
	const char* pszStretch	= getVal("font-stretch");
	const char* pszSize		= getVal("font-size");
	if(!pszFamily)
		pszFamily = "Times New Roman";

	if(!pszStyle)
		pszStyle = "normal";

	if(!pszVariant)
		pszVariant = "normal";

	if(!pszStretch)
		pszStretch = "normal";

	if(!pszSize)
		pszSize="12pt";

	if(!pszWeight)
		pszWeight = "normal";

	m_pFont = m_gc->findFont(pszFamily, pszStyle,
							 pszVariant, pszWeight,
							 pszStretch, pszSize,
							 NULL);

	UT_ASSERT_HARMLESS(m_pFont);
	if(!m_pFont)
	{
		clearScreen();
		return;
	}

	m_gc->setFont(m_pFont);		

	m_iAscent = m_gc->getFontAscent(m_pFont);
	m_iDescent = m_gc->getFontDescent(m_pFont);
	m_iHeight = m_gc->getFontHeight(m_pFont);
	
//
// Clear the screen!
//
	clearScreen();
//
// Calculate the draw coordinates position
//
	UT_sint32 iWinWidth = m_gc->tlu(getWindowWidth());
	UT_sint32 iWinHeight = m_gc->tlu(getWindowHeight());
	UT_sint32 iTop = (iWinHeight - m_iHeight)/2;
	UT_sint32 len = UT_UCS4_strlen(m_pszChars);
	UT_sint32 twidth = m_gc->measureString(m_pszChars,0,len,NULL);
	UT_sint32 iLeft = (iWinWidth - twidth)/2;
//
// Fill the background color
//
	GR_Painter painter(m_gc);

	if(pszBGColor)
		painter.fillRect(BGcolor,iLeft,iTop,twidth,m_iHeight);
//
// Do the draw chars at last!
//
	m_gc->setColor(FGcolor);
	painter.drawChars(m_pszChars, 0, len, iLeft, iTop);

//
// Do the decorations
//
	if(isUnder)
	{
		UT_sint32 iDrop = iTop + m_iAscent + m_iDescent/3;
		painter.drawLine(iLeft,iDrop,iLeft+twidth,iDrop);
	}
	if(isOver)
	{
		UT_sint32 iDrop = iTop + m_gc->tlu(1) + (UT_MAX(m_gc->tlu(10),m_iAscent) - m_gc->tlu(10))/8;
		painter.drawLine(iLeft,iDrop,iLeft+twidth,iDrop);
	}
	if(isStrike)
	{
		UT_sint32 iDrop = iTop + m_iAscent * 2 /3;
		painter.drawLine(iLeft,iDrop,iLeft+twidth,iDrop);
	}

	// bad hardcoded color, but this will probably [ <-this assumption is the bad thing :) ] never be different anyway
	m_gc->setColor(UT_RGBColor(0,0,0));
	painter.drawLine(0, 0, m_gc->tlu(getWindowWidth()), 0);
	painter.drawLine(m_gc->tlu(getWindowWidth()) - m_gc->tlu(1), 0, m_gc->tlu(getWindowWidth()) - m_gc->tlu(1),
		       m_gc->tlu(getWindowHeight()));
	painter.drawLine(m_gc->tlu(getWindowWidth()) - m_gc->tlu(1), m_gc->tlu(getWindowHeight()) - m_gc->tlu(1), 0,
		       m_gc->tlu(getWindowHeight()) - m_gc->tlu(1));
	painter.drawLine(0, m_gc->tlu(getWindowHeight()) - m_gc->tlu(1), 0, 0);
}

void XAP_Preview_FontPreview::clearScreen(void)
{
	UT_sint32 iWidth = m_gc->tlu(getWindowWidth());
	UT_sint32 iHeight = m_gc->tlu(getWindowHeight());

	GR_Painter painter(m_gc);

	// clear the whole drawing area, except for the border
	painter.fillRect(m_clrBackground, 0 + m_gc->tlu(1), 0 + m_gc->tlu(1), iWidth - m_gc->tlu(2), iHeight - m_gc->tlu(2));
}
