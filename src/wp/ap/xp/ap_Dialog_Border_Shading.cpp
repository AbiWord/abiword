/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Marc Maurer
 * Copyright (c) 2009 Hubert Figuiere
 * Copyright (c) 2010 Maleesh Prasan
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

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "fv_View.h"
#include "pd_Document.h"
#include "pt_Types.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fp_ContainerObject.h"
#include "fp_TableContainer.h"
#include "fl_TableLayout.h"
#include "fl_BlockLayout.h"
#include "fl_DocLayout.h"
#include "ut_timer.h"
#include "fg_GraphicRaster.h"
#include "fg_GraphicVector.h"
#include "ap_Dialog_Border_Shading.h"
#include "ut_png.h"
#include "gr_Painter.h"
#include "ut_units.h"
#include "ap_Strings.h"
#include "pp_PropertyMap.h"

AP_Dialog_Border_Shading::AP_Dialog_Border_Shading(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_Modeless(pDlgFactory,id, "interface/dialogbordershading"),
	  m_answer(a_OK),
	  m_pBorderShadingPreview(NULL),
	  m_borderColor(0,0,0),
	  m_lineStyle(LS_NORMAL),
	  m_bgFillStyle(NULL),
	  m_bSettingsChanged(false),
	  m_iOldPos(0),
	  m_pAutoUpdaterMC(NULL),
	  m_bDestroy_says_stopupdating(false),
	  m_bAutoUpdate_happening_now(false)
// 	  m_sImagePath(""),
// 	  m_iGraphicType(0),
// 	  m_pImage(NULL),
// 	  m_pGraphic(NULL)
{
	const char * sBordersThickness[BORDER_SHADING_NUMTHICKNESS] ={"0.25pt","0.5pt",
													   "0.75pt","1.0pt",
													   "1.5pt","2.25pt","3pt",
													   "4.5pt","6.0pt"};

	const char * sShadingOffset[BORDER_SHADING_NUMOFFSETS] ={"0.25pt","0.5pt",
														"0.75pt","1.0pt",
														"1.5pt","2.25pt","3pt",
														"4.5pt","6.0pt"};

	UT_sint32 i = 0;
	for(i=0; i< BORDER_SHADING_NUMTHICKNESS ;i++)
	{
		m_dThickness[i] = UT_convertToInches(sBordersThickness[i]);
	}

	UT_sint32 j = 0;
	for(j=0; j< BORDER_SHADING_NUMOFFSETS ;j++)
	{
		m_dShadingOffset[j] = UT_convertToInches(sShadingOffset[j]);
	}

	if(m_vecProps.getItemCount() > 0)
		m_vecProps.clear();
/*
	if(m_vecPropsAdjRight.getItemCount() > 0)
		m_vecPropsAdjRight.clear();
	  
	if(m_vecPropsAdjBottom.getItemCount() > 0)
		m_vecPropsAdjBottom.clear();
*/
	guint border_style_id = (guint)PP_PropertyMap::linestyle_none - 1;
	m_sDefaultStyle = UT_String_sprintf("%d", border_style_id);
}

AP_Dialog_Border_Shading::~AP_Dialog_Border_Shading(void)
{
	stopUpdater();
	DELETEP(m_pBorderShadingPreview);
// 	DELETEP(m_pGraphic);
// 	DELETEP(m_pImage);
}

AP_Dialog_Border_Shading::tAnswer AP_Dialog_Border_Shading::getAnswer(void) const
{
	return m_answer;
}

void AP_Dialog_Border_Shading::setActiveFrame(XAP_Frame * /*pFrame*/)
{
	notifyActiveFrame(getActiveFrame());
}

void AP_Dialog_Border_Shading::ConstructWindowName(void)
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	gchar * tmp = NULL;
	UT_XML_cloneNoAmpersands(tmp, pSS->getValue(AP_STRING_ID_DLG_BorderShading_Title));
	BuildWindowName(static_cast<char *>(m_WindowName),static_cast<char*>(tmp),sizeof(m_WindowName));
	FREEP(tmp);
}

void AP_Dialog_Border_Shading::startUpdater(void)
{
	m_bDestroy_says_stopupdating = false;
	m_bAutoUpdate_happening_now = false;
	m_pAutoUpdaterMC =  UT_Timer::static_constructor(autoUpdateMC,this);
	m_pAutoUpdaterMC->set(100); // use a fast time, so the dialogs behavior looks "snappy"
	m_pAutoUpdaterMC->start();
}

void AP_Dialog_Border_Shading::stopUpdater(void)
{
	if(m_pAutoUpdaterMC == NULL)
	{
		return;
	}
	m_bDestroy_says_stopupdating = true;
	m_pAutoUpdaterMC->stop();
	DELETEP(m_pAutoUpdaterMC);
	m_pAutoUpdaterMC = NULL;
}
/*!
 Auto-updater of the dialog.
 */
void AP_Dialog_Border_Shading::autoUpdateMC(UT_Worker * pTimer)
{
	UT_return_if_fail (pTimer);
	
	// get the dialog pointer from the timer instance data
	AP_Dialog_Border_Shading * pDialog = static_cast<AP_Dialog_Border_Shading *>(pTimer->getInstanceData());

	// Handshaking code
	if( pDialog->m_bDestroy_says_stopupdating != true)
	{
		pDialog->m_bAutoUpdate_happening_now = true;
		pDialog->setAllSensitivities();
		pDialog->setCurBlockProps();
		pDialog->m_bAutoUpdate_happening_now = false;
	}
}        

/*! 
 Sets the sensitivity of the radio buttons to top/bottom/left/right line buttons
 Call this right after constructing the widget and before dropping into the main loop.
 */
void AP_Dialog_Border_Shading::setAllSensitivities(void)
{
	setSensitivity(true);
}

void AP_Dialog_Border_Shading::setCurBlockProps(void)
{
	XAP_Frame *frame = XAP_App::getApp()->getLastFocussedFrame();
	if (frame) {
		FV_View * pView = static_cast<FV_View *>(frame->getCurrentView());

		if (m_bSettingsChanged || m_iOldPos == pView->getPoint())
			return;	
		
		m_iOldPos = pView->getPoint();

		/*
		 * update the border colors
		 */
		
		fl_BlockLayout* current_block = pView->getCurrentBlock();

		const char* style_left	= current_block->getProperty("left-style");
		const char* style_right	= current_block->getProperty("right-style");
		const char* style_top	= current_block->getProperty("top-style");
		const char* style_bot	= current_block->getProperty("bot-style");


		// 8/8/2010 Maleesh - Update the border styles.
		UT_UTF8String active_style	= m_sDefaultStyle.c_str();
		UT_UTF8String default_style = m_sDefaultStyle.c_str();

		if (style_left)
		{
			m_vecProps.addOrReplaceProp("left-style", style_left);
			if (active_style == default_style)
				active_style = style_left;
		}
		else
			m_vecProps.removeProp("left-style");

		if (style_right)
		{
			m_vecProps.addOrReplaceProp("right-style", style_right);
			if (active_style == default_style)
				active_style = style_right;
		}
		else
			m_vecProps.removeProp("right-style");

		if (style_top)
		{
			m_vecProps.addOrReplaceProp("top-style", style_top);
			if (active_style == default_style)
				active_style = style_top;
		}
		else
			m_vecProps.removeProp("top-style");

		if (style_bot)
		{
			m_vecProps.addOrReplaceProp("bot-style", style_bot);
			if (active_style == default_style)
				active_style = style_bot;	
		}
		else
			m_vecProps.removeProp("bot-style");

		setBorderStyleInGUI(active_style);

		const char* color_left	= current_block->getProperty("left-color");
		const char* thickness_left	= current_block->getProperty("left-thickness");

		if (color_left)
		{
			m_vecProps.addOrReplaceProp("left-color", color_left);
			m_vecProps.addOrReplaceProp("right-color", color_left);
			m_vecProps.addOrReplaceProp("top-color", color_left);
			m_vecProps.addOrReplaceProp("bot-color", color_left);

			UT_RGBColor clr;
			clr.setColor(color_left);
			setBorderColorInGUI(clr);
		}
		else
		{
			m_vecProps.removeProp("left-color");
			m_vecProps.removeProp("right-color");
			m_vecProps.removeProp("top-color");
			m_vecProps.removeProp("bot-color");
		}

		if (thickness_left)
		{
			m_vecProps.addOrReplaceProp("left-thickness", thickness_left);
			m_vecProps.addOrReplaceProp("right-thickness", thickness_left);
			m_vecProps.addOrReplaceProp("top-thickness", thickness_left);
			m_vecProps.addOrReplaceProp("bot-thickness", thickness_left);

			UT_UTF8String thickness_utf8 = thickness_left;
			setBorderThicknessInGUI(thickness_utf8);
		}
		else
		{
			m_vecProps.removeProp("left-thickness");
			m_vecProps.removeProp("right-thickness");
			m_vecProps.removeProp("top-thickness");
			m_vecProps.removeProp("bot-thickness");
		}

		// 8/8/2010 Maleesh - Update shading.

		const char* shading_pattern	= current_block->getProperty("shading-pattern");
		const char* shading_color	= current_block->getProperty("shading-foreground-color");

		if (shading_pattern)
		{
			m_vecProps.addOrReplaceProp("shading-pattern", shading_pattern);
			
			UT_UTF8String pattern_utf8 = shading_pattern;
			setShadingPatternInGUI(pattern_utf8);
		}
		else
		{
			m_vecProps.removeProp("shading-pattern");
			UT_UTF8String pattern_utf8 = BORDER_SHADING_SHADING_DISABLE;
			setShadingPatternInGUI(pattern_utf8);
		}

		if (shading_color)
		{
			m_vecProps.addOrReplaceProp("shading-foreground-color", shading_color);

			UT_RGBColor clr;
			clr.setColor(shading_color);
			setShadingColorInGUI(clr);
		}
		else
		{
			m_vecProps.removeProp("shading-foreground-color");
			setShadingColorInGUI(UT_RGBColor(255, 255, 255));
		}


		// 8/8/2010 Maleesh - TEMP
		UT_DEBUGMSG(("Maleesh =============== Border props %s|%s|%s\n",style_left, color_left, thickness_left));
		UT_DEBUGMSG(("Maleesh =============== Shading props %s|%s\n",shading_pattern, shading_color));

		// draw the preview with the changed properties
		if(m_pBorderShadingPreview)
			m_pBorderShadingPreview->queueDraw();
	}
}

void AP_Dialog_Border_Shading::applyChanges()
{
	// XXX m_vecProps should be turned into a PP_PopertyVector
	UT_DEBUGMSG(("Doing apply changes number props %d \n",m_vecProps.getItemCount()));
	if (m_vecProps.getItemCount() == 0)
		return;

	FV_View * pView = static_cast<FV_View *>(XAP_App::getApp()->getLastFocussedFrame()->getCurrentView());
	PP_PropertyVector propsArray;

	UT_sint32 i = m_vecProps.getItemCount();
	UT_sint32 j;
	for(j= 0; j<i; j=j+2)
	{
		propsArray.push_back(m_vecProps.getNthItem(j));
		propsArray.push_back(m_vecProps.getNthItem(j+1));

		xxx_UT_DEBUGMSG(("Maleesh ======================= %s | %s \n", propsArray[j], propsArray[j + 1]));
	}
	pView->setBlockFormat(propsArray);

	m_bSettingsChanged = false;
}

void AP_Dialog_Border_Shading::finalize(void)
{
	stopUpdater();
	modeless_cleanup();
}

/*!
 Set the color and style and thickness of the toggled button
 */
void AP_Dialog_Border_Shading::toggleLineType(toggle_button btn, bool enabled)
{
	UT_String cTmp = UT_String_sprintf("%02x%02x%02x", m_borderColor.m_red, m_borderColor.m_grn, m_borderColor.m_blu);	
	UT_String sTmp = UT_String_sprintf("%d", (enabled ? m_lineStyle : LS_OFF));

	switch (btn)
	{
		case toggle_left:
		{
			m_vecProps.addOrReplaceProp("left-style", sTmp.c_str());
			m_vecProps.addOrReplaceProp("left-color", cTmp.c_str());
			m_vecProps.addOrReplaceProp("left-thickness",m_sBorderThickness.utf8_str());
		}
		break;
		case toggle_right:
		{	
			m_vecProps.addOrReplaceProp("right-style", sTmp.c_str());
			m_vecProps.addOrReplaceProp("right-color", cTmp.c_str());
			m_vecProps.addOrReplaceProp("right-thickness",m_sBorderThickness.utf8_str());
		}
		break;
		case toggle_top:
		{			
			m_vecProps.addOrReplaceProp("top-style", sTmp.c_str());
			m_vecProps.addOrReplaceProp("top-color", cTmp.c_str());
			m_vecProps.addOrReplaceProp("top-thickness",m_sBorderThickness.utf8_str());
		}
		break;
		case toggle_bottom:
		{			
			m_vecProps.addOrReplaceProp("bot-style", sTmp.c_str());
			m_vecProps.addOrReplaceProp("bot-color", cTmp.c_str());
			m_vecProps.addOrReplaceProp("bot-thickness",m_sBorderThickness.utf8_str());
		}
		break;
	}
	m_bSettingsChanged = true;
	UT_DEBUGMSG(("Maleesh ======================= toggleLineType\n"));
}

void AP_Dialog_Border_Shading::setBorderThickness(UT_UTF8String & sThick)
{
	m_sBorderThickness = sThick;

	m_vecProps.addOrReplaceProp("left-thickness", m_sBorderThickness.utf8_str());
	m_vecProps.addOrReplaceProp("right-thickness", m_sBorderThickness.utf8_str());
	m_vecProps.addOrReplaceProp("top-thickness", m_sBorderThickness.utf8_str());
	m_vecProps.addOrReplaceProp("bot-thickness", m_sBorderThickness.utf8_str());

	guint index			= _findClosestThickness(sThick.utf8_str());
	double space		= m_dThickness[index] + 0.02;	
	UT_String str_space = UT_String_sprintf("%fin", space);	

	m_vecProps.addOrReplaceProp("left-space", str_space.c_str());
	m_vecProps.addOrReplaceProp("right-space", str_space.c_str());
	m_vecProps.addOrReplaceProp("top-space", str_space.c_str());
	m_vecProps.addOrReplaceProp("bot-space", str_space.c_str());

	m_bSettingsChanged = true;
	UT_DEBUGMSG(("Maleesh ======================= setBorderThickness\n"));
}

void AP_Dialog_Border_Shading::setBorderStyle(UT_UTF8String & sStyle)
{
	m_vecProps.addOrReplaceProp("left-style", sStyle.utf8_str());
	m_vecProps.addOrReplaceProp("right-style",sStyle.utf8_str());
	m_vecProps.addOrReplaceProp("top-style",sStyle.utf8_str());
	m_vecProps.addOrReplaceProp("bot-style",sStyle.utf8_str());

	m_bSettingsChanged = true;
	UT_DEBUGMSG(("Maleesh ======================= setBorderStyle %s: \n", sStyle.utf8_str()));
}

void AP_Dialog_Border_Shading::setBorderColor(UT_RGBColor clr)
{
	m_borderColor = clr;

	UT_String s = UT_String_sprintf("%02x%02x%02x", clr.m_red, clr.m_grn, clr.m_blu);	

	m_vecProps.addOrReplaceProp("left-color", s.c_str());
	m_vecProps.addOrReplaceProp("right-color", s.c_str());
	m_vecProps.addOrReplaceProp("top-color", s.c_str());
	m_vecProps.addOrReplaceProp("bot-color", s.c_str());
	
//	m_vecPropsAdjRight.addOrReplaceProp("left-color", s.c_str());
//	m_vecPropsAdjBottom.addOrReplaceProp("top-color", s.c_str());
	
	m_bSettingsChanged = true;
	UT_DEBUGMSG(("Maleesh ======================= setBorderColor\n"));
}

void AP_Dialog_Border_Shading::setShadingPattern(UT_UTF8String & sPattern)
{
	m_vecProps.addOrReplaceProp ("shading-pattern", sPattern.utf8_str());
	m_bSettingsChanged = true;
	UT_DEBUGMSG(("Maleesh ======================= setShadingPattern\n"));
}

void AP_Dialog_Border_Shading::setShadingColor(UT_RGBColor clr)
{
	UT_String bgcol = UT_String_sprintf("%02x%02x%02x", clr.m_red, clr.m_grn, clr.m_blu);

	if (clr.isTransparent ())
	{
		m_vecProps.removeProp ("shading-foreground-color");
	}
	else
	{
		m_vecProps.addOrReplaceProp ("shading-foreground-color", bgcol.c_str ());
	}
	m_bSettingsChanged = true;
	UT_DEBUGMSG(("Maleesh ======================= setShadingColor\n"));
}

void AP_Dialog_Border_Shading::setShadingOffset(UT_UTF8String & /*sOffset*/)
{
	// 7/7/2010 Maleesh  - TODO 
}

void AP_Dialog_Border_Shading::_createPreviewFromGC(GR_Graphics * gc,
											     UT_uint32 width,
											     UT_uint32 height)
{
	UT_return_if_fail (gc);

	delete m_pBorderShadingPreview;
	m_pBorderShadingPreview = new AP_Border_Shading_preview(gc,this);
	UT_return_if_fail (m_pBorderShadingPreview);

	m_pBorderShadingPreview->setWindowSize(width, height);
}

bool AP_Dialog_Border_Shading::_getToggleButtonStatus(const char * lineStyle) const
{
	const gchar * pszStyle = NULL;
	std::string lsOff = UT_std_string_sprintf("%d", LS_OFF);	

	m_vecProps.getProp(lineStyle, pszStyle);

	if ((pszStyle && strcmp(pszStyle, lsOff.c_str())) || 
		!pszStyle)
		return true;
	else
		return false;
}

bool AP_Dialog_Border_Shading::getTopToggled() const
{
	return _getToggleButtonStatus("top-style");
}

bool AP_Dialog_Border_Shading::getBottomToggled() const
{
	return _getToggleButtonStatus("bot-style");
}

bool AP_Dialog_Border_Shading::getRightToggled() const
{
	return _getToggleButtonStatus("right-style");
}

bool AP_Dialog_Border_Shading::getLeftToggled() const
{
	return _getToggleButtonStatus("left-style");
}

guint AP_Dialog_Border_Shading::_findClosestThickness(const char *sthickness) const
{
	double thickness = UT_convertToInches(sthickness);
	guint i = 0;
	guint closest = 0;
	double dClose = 100000000.;
	for(i = 0; i < BORDER_SHADING_NUMTHICKNESS; i++)
	{
		double diff = thickness - m_dThickness[i];
		if(diff < 0)
			diff = -diff;
		if(diff < dClose)
		{
			closest = i;
			dClose = diff;
		}
	}
	return closest;
}


guint AP_Dialog_Border_Shading::_findClosestOffset(const char *sOffset) const
{
	double thickness = UT_convertToInches(sOffset);
	guint i = 0;
	guint closest = 0;
	double dClose = 100000000.;
	for(i = 0; i < BORDER_SHADING_NUMOFFSETS; i++)
	{
		double diff = thickness - m_dShadingOffset[i];
		if(diff < 0)
			diff = -diff;
		if(diff < dClose)
		{
			closest = i;
			dClose = diff;
		}
	}
	return closest;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_Border_Shading_preview::AP_Border_Shading_preview(GR_Graphics * gc, AP_Dialog_Border_Shading * pFormatTable)
	: XAP_Preview(gc)
{
	m_pBorderShading = pFormatTable;
}

AP_Border_Shading_preview::~AP_Border_Shading_preview()
{
}

void AP_Border_Shading_preview::draw(const UT_Rect *clip)
{
	UT_UNUSED(clip);
	GR_Painter painter(m_gc);

	UT_sint32 iWidth = m_gc->tlu (getWindowWidth());
	UT_sint32 iHeight = m_gc->tlu (getWindowHeight());
	UT_Rect pageRect(m_gc->tlu(7), m_gc->tlu(7), iWidth - m_gc->tlu(14), iHeight - m_gc->tlu(14));	
	
	painter.fillRect(GR_Graphics::CLR3D_Background, 0, 0, iWidth, iHeight);
	painter.clearArea(pageRect.left, pageRect.top, pageRect.width, pageRect.height);	
		
	UT_RGBColor tmpCol;
	
	UT_RGBColor black(0, 0, 0);
	m_gc->setLineWidth(m_gc->tlu(1));
	
	int border = m_gc->tlu(20);
	int cornerLength = m_gc->tlu(5);

//
//  Draw the cell background (Shading)
//
	
	const gchar * pszShadingColor	= NULL;
	const gchar * pszShadingPattern = NULL;
	m_pBorderShading->getPropVector().getProp(static_cast<const gchar *>("shading-pattern"), pszShadingPattern);

	if(pszShadingPattern && strcmp(pszShadingPattern, BORDER_SHADING_SHADING_DISABLE))
	{
		m_pBorderShading->getPropVector().getProp(static_cast<const gchar *>("shading-foreground-color"), pszShadingColor);
		if (pszShadingColor && *pszShadingColor)
		{
			UT_parseColor(pszShadingColor, tmpCol);
			painter.fillRect(tmpCol, pageRect.left + border, pageRect.top + border, pageRect.width - 2*border, pageRect.height - 2*border);
		}
	}

//
//  Draw the cell corners
//
	
	m_gc->setColor(UT_RGBColor(127,127,127));
	
	// top left corner
	painter.drawLine(pageRect.left + border - cornerLength, pageRect.top + border,
				   pageRect.left + border, pageRect.top + border);
	painter.drawLine(pageRect.left + border, pageRect.top + border  - cornerLength,
				   pageRect.left + border, pageRect.top + border);

	// top right corner
	painter.drawLine(pageRect.left + pageRect.width - border + cornerLength, pageRect.top + border,
				   pageRect.left + pageRect.width - border, pageRect.top + border);
	painter.drawLine(pageRect.left + pageRect.width - border, pageRect.top + border - cornerLength,
				   pageRect.left + pageRect.width - border, pageRect.top + border);

	// bottom left corner
	painter.drawLine(pageRect.left + border - cornerLength, pageRect.top + pageRect.height - border,
				   pageRect.left + border, pageRect.top + pageRect.height - border);
	painter.drawLine(pageRect.left + border, pageRect.top + pageRect.height - border + cornerLength,
				   pageRect.left + border, pageRect.top + pageRect.height - border);

	// bottom right corner
	painter.drawLine(pageRect.left + pageRect.width - border + cornerLength, pageRect.top + pageRect.height - border,
				   pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border);
	painter.drawLine(pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border + cornerLength,
				   pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border);

//
//  Draw the cell borders
//
	
	// top border
	if (m_pBorderShading->getTopToggled())
	{
		const gchar * pszTopColor = NULL;
		m_pBorderShading->getPropVector().getProp("top-color", pszTopColor);
		if (pszTopColor)
		{
			UT_parseColor(pszTopColor, tmpCol);
			m_gc->setColor(tmpCol);
		}
		else
			m_gc->setColor(black);
		const gchar * pszTopThickness = NULL;
		m_pBorderShading->getPropVector().getProp("top-thickness", pszTopThickness);
		if(pszTopThickness)
		{
			UT_sint32 iTopThickness = UT_convertToLogicalUnits(pszTopThickness);
			m_gc->setLineWidth(iTopThickness);
		}
		else
		{
			m_gc->setLineWidth(m_gc->tlu(1));
		}

		painter.drawLine(pageRect.left + border, pageRect.top + border,
					   pageRect.left + pageRect.width - border, pageRect.top + border);
	}

	// left border
	if (m_pBorderShading->getLeftToggled())
	{
		const gchar * pszLeftColor = NULL;
		m_pBorderShading->getPropVector().getProp("left-color", pszLeftColor);
		if (pszLeftColor)
		{
			UT_parseColor(pszLeftColor, tmpCol);
			m_gc->setColor(tmpCol);
		}
		else
			m_gc->setColor(black);
		const gchar * pszLeftThickness = NULL;
		m_pBorderShading->getPropVector().getProp("left-thickness", pszLeftThickness);
		if(pszLeftThickness)
		{
			UT_sint32 iLeftThickness = UT_convertToLogicalUnits(pszLeftThickness);
			m_gc->setLineWidth(iLeftThickness);
		}
		else
		{
			m_gc->setLineWidth(m_gc->tlu(1));
		}
		painter.drawLine(pageRect.left + border, pageRect.top + border,
					   pageRect.left + border, pageRect.top + pageRect.height - border);
	}

	// right border
	if (m_pBorderShading->getRightToggled())
	{
		const gchar * pszRightColor = NULL;
		m_pBorderShading->getPropVector().getProp("right-color", pszRightColor);
		if (pszRightColor)
		{
			UT_parseColor(pszRightColor, tmpCol);
			m_gc->setColor(tmpCol);
		}
		else
			m_gc->setColor(black);
		const gchar * pszRightThickness = NULL;
		m_pBorderShading->getPropVector().getProp("right-thickness", pszRightThickness);
		if(pszRightThickness)
		{
			UT_sint32 iRightThickness = UT_convertToLogicalUnits(pszRightThickness);
			m_gc->setLineWidth(iRightThickness);
		}
		else
		{
			m_gc->setLineWidth(m_gc->tlu(1));
		}
		painter.drawLine(pageRect.left + pageRect.width - border, pageRect.top + border,
					   pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border);
	}
	
	// bottom border
	if (m_pBorderShading->getBottomToggled())
	{
		const gchar * pszBottomColor = NULL;
		m_pBorderShading->getPropVector().getProp("bot-color", pszBottomColor);
		if (pszBottomColor)
		{
			UT_parseColor(pszBottomColor, tmpCol);
			m_gc->setColor(tmpCol);
		}
		else
			m_gc->setColor(black);
		const gchar * pszBotThickness = NULL;
		m_pBorderShading->getPropVector().getProp("bot-thickness", pszBotThickness);
		if(pszBotThickness)
		{
			UT_sint32 iBotThickness = UT_convertToLogicalUnits(pszBotThickness);
			m_gc->setLineWidth(iBotThickness);
		}
		else
		{
			m_gc->setLineWidth(m_gc->tlu(1));
		}
		painter.drawLine(pageRect.left + border, pageRect.top + pageRect.height - border,
					   pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border);
	}
}
