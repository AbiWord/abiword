/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2009-2016 Hubert Figuiere
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
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "xap_Dlg_FontChooser.h"
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
	m_pFontPreview          = NULL;
	m_bUnderline			= false;
	m_bOverline				= false;
	m_bStrikeout			= false;
	m_bTopline		    	= false;
	m_bBottomline			= false;
	m_bHidden               = false;
	m_bSuperScript			= false;
	m_bSubScript			= false;
	m_bChangedFontFamily	= false;
	m_bChangedTextTransform	= false;
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

	UT_UCS4_cloneString_char (&m_drawString, PREVIEW_ENTRY_DEFAULT_STRING);
}

XAP_Dialog_FontChooser::~XAP_Dialog_FontChooser(void)
{
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

	m_pFontPreview = new XAP_Preview_FontPreview(gc,m_sColorBackground.c_str());
	UT_return_if_fail(m_pFontPreview);

	m_pFontPreview->setWindowSize(width, height);
	m_pFontPreview->setVecProperties( & m_mapProps);
}

void XAP_Dialog_FontChooser::addOrReplaceVecProp(const std::string & sProp, const std::string & sVal)
{
	m_mapProps[sProp] = sVal;
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
 * This method returns a std::string of the value associated with the
 * the property sProp.
 */
std::string XAP_Dialog_FontChooser::getVal(const std::string & sProp) const
{
	PropMap::const_iterator iter = m_mapProps.find(sProp);
	if(iter == m_mapProps.end()) {
		return "";
	}
	return iter->second;
}

/*!
 * This method sets all the local properties from a vector of string
 * Property - Value pairs.
 * This method wipes out all the old values and clears all the bools
 * assciated with them.
 */
void XAP_Dialog_FontChooser::setAllPropsFromVec(const std::vector<std::string> & vProps)
{
	UT_sint32 remCount = vProps.size();
	if(remCount <= 0)
		return;
	// BAD BAD, we have wrong count
	UT_ASSERT_HARMLESS(remCount % 2 == 0);
	if(remCount % 2) {
		remCount--;
	}
	m_mapProps.clear();
	UT_sint32 i = 0;
	for(i=0; i< remCount; i+=2)
	{
		m_mapProps.insert(std::make_pair(vProps[i],
                                                 vProps[i+1]));
	}
//
// Do the Text decorations
//
	const std::string sDecor = getVal("text-decoration");
	m_bUnderline = (NULL != strstr(sDecor.c_str(),"underline"));
	m_bOverline = (NULL != strstr(sDecor.c_str(),"overline"));
	m_bStrikeout = (NULL != strstr(sDecor.c_str(),"line-through"));
	m_bTopline = (NULL != strstr(sDecor.c_str(),"topline"));
	m_bBottomline = (NULL != strstr(sDecor.c_str(),"bottomline"));

	const std::string sDisplay = getVal("display");
	m_bHidden = sDisplay != "none";

	const std::string sPosition = getVal("text-position");
	m_bSuperScript = sPosition == "superscript";
	m_bSubScript = sPosition == "subscript";
}

void XAP_Dialog_FontChooser::setFontFamily(const std::string& sFontFamily)
{
	m_sFontFamily = sFontFamily;
	addOrReplaceVecProp("font-family",sFontFamily);
}

void XAP_Dialog_FontChooser::setTextTransform(const std::string& sTextTransform)
{
	m_sTextTransform = sTextTransform;
	addOrReplaceVecProp("text-transform",sTextTransform);
}

void XAP_Dialog_FontChooser::setFontSize(const std::string& sFontSize)
{
	m_sFontSize = sFontSize;
	addOrReplaceVecProp("font-size",sFontSize);
}

void XAP_Dialog_FontChooser::setFontWeight(const std::string& sFontWeight)
{
	m_sFontWeight = sFontWeight;
	addOrReplaceVecProp("font-weight",sFontWeight);
}

void XAP_Dialog_FontChooser::setFontStyle(const std::string& sFontStyle)
{
	m_sFontStyle = sFontStyle;
	addOrReplaceVecProp("font-style",sFontStyle);
}

void XAP_Dialog_FontChooser::setColor(const std::string& sColor)
{
	m_sColor = sColor;
	addOrReplaceVecProp("color",sColor);
}

void XAP_Dialog_FontChooser::setBGColor(const std::string& sBGColor)
{
	m_sBGColor = sBGColor;
	addOrReplaceVecProp("bgcolor",sBGColor);
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
	m_sColorBackground = pBackground;
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

bool XAP_Dialog_FontChooser::getChangedFontFamily(std::string& szFontFamily) const
{
	std::string fontFamily = getVal("font-family");
	bool bchanged = didPropChange(m_sFontFamily, fontFamily);
	bool useVal = (bchanged && !m_bChangedFontFamily);
	if (useVal)
		szFontFamily = fontFamily;
	else
		szFontFamily = m_sFontFamily;
	return bchanged;
}

bool XAP_Dialog_FontChooser::getChangedTextTransform(std::string& szTextTransform) const
{
	std::string textTransform = getVal("text-transform");
	bool bchanged = didPropChange(m_sTextTransform, textTransform);
	bool useVal = (bchanged && !m_bChangedTextTransform);
	if (useVal)
		szTextTransform = textTransform;
	else
		szTextTransform = m_sTextTransform;
	return bchanged;
}

bool XAP_Dialog_FontChooser::getChangedFontSize(std::string& szFontSize) const
{
	std::string fontSize = getVal("font-size");
	bool bchanged = didPropChange(m_sFontSize, fontSize);
	bool useVal = (bchanged && !m_bChangedFontSize);
	if (useVal)
		szFontSize = fontSize;
	else
		szFontSize = m_sFontSize;
	return bchanged;
}

bool XAP_Dialog_FontChooser::getChangedFontWeight(std::string& szFontWeight) const
{
	std::string fontWeight = getVal("font-weight");
	bool bchanged = didPropChange(m_sFontWeight, fontWeight);
	bool useVal = (bchanged && !m_bChangedFontWeight);
	if (useVal)
		szFontWeight = fontWeight;
	else
		szFontWeight = m_sFontWeight;
	return bchanged;
}

bool XAP_Dialog_FontChooser::getChangedFontStyle(std::string& szFontStyle) const
{
	std::string fontStyle = getVal("font-style");
	bool bchanged = didPropChange(m_sFontStyle, fontStyle);
	bool useVal = (bchanged && !m_bChangedFontStyle);
	if (useVal)
		szFontStyle = fontStyle;
	else
		szFontStyle = m_sFontStyle;
	return bchanged;
}

bool XAP_Dialog_FontChooser::getChangedBGColor(std::string& szBGColor) const
{
	std::string bgColor = getVal("bgcolor");
	bool bchanged = didPropChange(m_sBGColor, bgColor);
	bool useVal = (bchanged && !m_bChangedBGColor);
	if (useVal)
		szBGColor = bgColor;
	else
		szBGColor = m_sBGColor;
	return bchanged;
}


bool XAP_Dialog_FontChooser::getChangedColor(std::string& szColor) const
{
	std::string color = getVal("color");
	bool bchanged = didPropChange(m_sColor, color);
	bool useVal = (bchanged && !m_bChangedColor);
	if (useVal)
		szColor = color;
	else
		szColor = m_sColor;
	return bchanged;
}

/*!
 * Compare two prop values and gracefully handle the cases of NULL pointers
 */
bool XAP_Dialog_FontChooser::didPropChange(const std::string & v1, const std::string & v2) const
{
	if(v1.empty() && v2.empty())
		return false;
	else if(v1.empty() || v2.empty())
		return true;
	return v1 != v2;
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

void XAP_Preview_FontPreview::setVecProperties( const XAP_Dialog_FontChooser::PropMap * vFontProps)
{
	m_mapProps = vFontProps;
}

/*!
 * This method returns a pointer to the const char * value associated with the
 * the property szProp. Stolen from ap_Dialog_Lists.
 */
std::string XAP_Preview_FontPreview::getVal(const std::string & sProp) const
{
	XAP_Dialog_FontChooser::PropMap::const_iterator iter = m_mapProps->find(sProp);
	if(iter == m_mapProps->end()) {
		return "";
	}
	return iter->second;
}

/*
 *
 * Finally draw the characters in the preview.
 *
 */
void XAP_Preview_FontPreview::draw(const UT_Rect *clip)
{
	UT_UNUSED(clip);
//
// Get text decorations.
//
	bool isUnder,isOver,isStrike;

	const std::string sDecor = getVal("text-decoration");
	if(!sDecor.empty())
	{
		isUnder = (NULL != strstr(sDecor.c_str(),"underline"));
		isOver = (NULL != strstr(sDecor.c_str(),"overline"));
		isStrike = (NULL != strstr(sDecor.c_str(),"line-through"));
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
	const std::string sFGColor = getVal("color");
	if(!sFGColor.empty())
		UT_parseColor(sFGColor.c_str(),FGcolor);
	UT_RGBColor BGcolor(m_clrBackground);
	const std::string sBGColor = getVal("bgcolor");
	if(!sBGColor.empty() && sBGColor != "transparent")
		UT_parseColor(sBGColor.c_str(),BGcolor);
//
// Get the font and bold/italic- ness
//
	//GR_Font * pFont;
	std::string sFamily = getVal("font-family");
	std::string sStyle = getVal("font-style");
	std::string sVariant = getVal("font-variant");
	std::string sStretch = getVal("font-stretch");
	std::string sSize = getVal("font-size");
	std::string sWeight = getVal("font-weight");

	if(sFamily.empty())
		sFamily = "Times New Roman";

	if(sStyle.empty())
		sStyle = "normal";

	if(sVariant.empty())
		sVariant = "normal";

	if(sStretch.empty())
		sStretch = "normal";

	if(sSize.empty())
		sSize="12pt";

	if(sWeight.empty())
		sWeight = "normal";

	m_pFont = m_gc->findFont(sFamily.c_str(), sStyle.c_str(),
							 sVariant.c_str(), sWeight.c_str(),
							 sStretch.c_str(), sSize.c_str(),
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

	if(!sBGColor.empty())
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
