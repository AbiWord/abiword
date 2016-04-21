/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Marc Maurer
 * Copyright (c) 2009-2016 Hubert Figuiere
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
#include "ut_locale.h"

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

	guint border_style_id = (guint)PP_PropertyMap::linestyle_none - 1;
	m_sDefaultStyle = UT_std_string_sprintf("%d", border_style_id);
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

		if (m_bSettingsChanged || m_iOldPos == pView->getPoint()) {
			return;
		}

		m_iOldPos = pView->getPoint();

		/*
		 * update the border colors
		 */

		fl_BlockLayout* current_block = pView->getCurrentBlock();

		const char* style_left	= current_block->getProperty("left-style");
		const char* style_right	= current_block->getProperty("right-style");
		const char* style_top	= current_block->getProperty("top-style");
		const char* style_bot	= current_block->getProperty("bot-style");


		// Update the border styles.
		std::string active_style  = m_sDefaultStyle;
		std::string default_style = m_sDefaultStyle;

		if (style_left)	{
			PP_addOrSetAttribute("left-style", style_left, m_vecProps);
			if (active_style == default_style) {
				active_style = style_left;
			}
		} else {
			PP_removeAttribute("left-style", m_vecProps);
		}

		if (style_right) {
			PP_addOrSetAttribute("right-style", style_right, m_vecProps);
			if (active_style == default_style) {
				active_style = style_right;
			}
		} else {
			PP_removeAttribute("right-style", m_vecProps);
		}

		if (style_top) {
			PP_addOrSetAttribute("top-style", style_top, m_vecProps);
			if (active_style == default_style) {
				active_style = style_top;
			}
		} else {
			PP_removeAttribute("top-style", m_vecProps);
		}

		if (style_bot) {
			PP_addOrSetAttribute("bot-style", style_bot, m_vecProps);
			if (active_style == default_style) {
				active_style = style_bot;
			}
		} else {
			PP_removeAttribute("bot-style", m_vecProps);
		}

		setBorderStyleInGUI(active_style);

		const char* color_left	= current_block->getProperty("left-color");
		const char* thickness_left	= current_block->getProperty("left-thickness");

		if (color_left)
		{
			PP_addOrSetAttribute("left-color", color_left, m_vecProps);
			PP_addOrSetAttribute("right-color", color_left, m_vecProps);
			PP_addOrSetAttribute("top-color", color_left, m_vecProps);
			PP_addOrSetAttribute("bot-color", color_left, m_vecProps);

			UT_RGBColor clr;
			clr.setColor(color_left);
			setBorderColorInGUI(clr);
		}
		else
		{
			PP_removeAttribute("left-color", m_vecProps);
			PP_removeAttribute("right-color", m_vecProps);
			PP_removeAttribute("top-color", m_vecProps);
			PP_removeAttribute("bot-color", m_vecProps);
		}

		if (thickness_left)
		{
			PP_addOrSetAttribute("left-thickness", thickness_left, m_vecProps);
			PP_addOrSetAttribute("right-thickness", thickness_left, m_vecProps);
			PP_addOrSetAttribute("top-thickness", thickness_left, m_vecProps);
			PP_addOrSetAttribute("bot-thickness", thickness_left, m_vecProps);

			setBorderThicknessInGUI(thickness_left);
		}
		else
		{
			PP_removeAttribute("left-thickness", m_vecProps);
			PP_removeAttribute("right-thickness", m_vecProps);
			PP_removeAttribute("top-thickness", m_vecProps);
			PP_removeAttribute("bot-thickness", m_vecProps);
		}

		// Update shading.

		const char* shading_pattern	= current_block->getProperty("shading-pattern");
		const char* shading_color	= current_block->getProperty("shading-foreground-color");

		if (shading_pattern)
		{
			PP_addOrSetAttribute("shading-pattern", shading_pattern, m_vecProps);
			setShadingPatternInGUI(shading_pattern);
		}
		else
		{
			PP_removeAttribute("shading-pattern", m_vecProps);
			setShadingPatternInGUI(BORDER_SHADING_SHADING_DISABLE);
		}

		if (shading_color)
		{
			PP_addOrSetAttribute("shading-foreground-color", shading_color, m_vecProps);
			UT_RGBColor clr;
			clr.setColor(shading_color);
			setShadingColorInGUI(clr);
		}
		else
		{
			PP_removeAttribute("shading-foreground-color", m_vecProps);
			setShadingColorInGUI(UT_RGBColor(255, 255, 255));
		}


		xxx_UT_DEBUGMSG(("Maleesh =============== Border props %s|%s|%s\n",style_left, color_left, thickness_left));
		xxx_UT_DEBUGMSG(("Maleesh =============== Shading props %s|%s\n",shading_pattern, shading_color));

		// draw the preview with the changed properties
		if(m_pBorderShadingPreview)
			m_pBorderShadingPreview->queueDraw();
	}
}

void AP_Dialog_Border_Shading::applyChanges()
{
	// XXX m_vecProps should be turned into a PP_PopertyVector
	UT_DEBUGMSG(("Doing apply changes number props %lu \n",m_vecProps.size()));
	if (m_vecProps.empty()) {
		return;
	}

	FV_View * pView = static_cast<FV_View *>(XAP_App::getApp()->getLastFocussedFrame()->getCurrentView());

	pView->setBlockFormat(m_vecProps);

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
	std::string cTmp = UT_std_string_sprintf("%02x%02x%02x", m_borderColor.m_red, m_borderColor.m_grn, m_borderColor.m_blu);
	std::string sTmp = UT_std_string_sprintf("%d", (enabled ? m_lineStyle : LS_OFF));

	switch (btn)
	{
		case toggle_left:
			PP_addOrSetAttribute("left-style", sTmp, m_vecProps);
			PP_addOrSetAttribute("left-color", cTmp, m_vecProps);
			PP_addOrSetAttribute("left-thickness", m_sBorderThickness, m_vecProps);
			break;
		case toggle_right:
			PP_addOrSetAttribute("right-style", sTmp, m_vecProps);
			PP_addOrSetAttribute("right-color", cTmp, m_vecProps);
			PP_addOrSetAttribute("right-thickness", m_sBorderThickness, m_vecProps);
			break;
		case toggle_top:
			PP_addOrSetAttribute("top-style", sTmp, m_vecProps);
			PP_addOrSetAttribute("top-color", cTmp, m_vecProps);
			PP_addOrSetAttribute("top-thickness",m_sBorderThickness, m_vecProps);
			break;
		case toggle_bottom:
			PP_addOrSetAttribute("bot-style", sTmp, m_vecProps);
			PP_addOrSetAttribute("bot-color", cTmp, m_vecProps);
			PP_addOrSetAttribute("bot-thickness", m_sBorderThickness, m_vecProps);
			break;
	}
	m_bSettingsChanged = true;
	xxx_UT_DEBUGMSG(("Maleesh ======================= toggleLineType\n"));
}

void AP_Dialog_Border_Shading::setBorderThickness(const std::string & sThick)
{
	m_sBorderThickness = sThick;

	PP_addOrSetAttribute("left-thickness", m_sBorderThickness, m_vecProps);
	PP_addOrSetAttribute("right-thickness", m_sBorderThickness, m_vecProps);
	PP_addOrSetAttribute("top-thickness", m_sBorderThickness, m_vecProps);
	PP_addOrSetAttribute("bot-thickness", m_sBorderThickness, m_vecProps);

	guint index			= _findClosestThickness(sThick.c_str());
	double space		= m_dThickness[index] + 0.02;
	std::string str_space;
	{
		UT_LocaleTransactor l(LC_NUMERIC, "C");
		str_space = UT_std_string_sprintf("%fin", space);
	}

	PP_addOrSetAttribute("left-space", str_space, m_vecProps);
	PP_addOrSetAttribute("right-space", str_space, m_vecProps);
	PP_addOrSetAttribute("top-space", str_space, m_vecProps);
	PP_addOrSetAttribute("bot-space", str_space, m_vecProps);

	m_bSettingsChanged = true;
	xxx_UT_DEBUGMSG(("Maleesh ======================= setBorderThickness\n"));
}

void AP_Dialog_Border_Shading::setBorderStyle(const std::string & sStyle)
{
	PP_addOrSetAttribute("left-style", sStyle, m_vecProps);
	PP_addOrSetAttribute("right-style", sStyle, m_vecProps);
	PP_addOrSetAttribute("top-style", sStyle, m_vecProps);
	PP_addOrSetAttribute("bot-style", sStyle, m_vecProps);

	m_bSettingsChanged = true;
//	UT_DEBUGMSG(("Maleesh ======================= setBorderStyle %s: \n", sStyle.c_str()));
}

void AP_Dialog_Border_Shading::setBorderColor(const UT_RGBColor& clr)
{
	m_borderColor = clr;

	std::string s = UT_std_string_sprintf("%02x%02x%02x", clr.m_red, clr.m_grn, clr.m_blu);

	PP_addOrSetAttribute("left-color", s, m_vecProps);
	PP_addOrSetAttribute("right-color", s, m_vecProps);
	PP_addOrSetAttribute("top-color", s, m_vecProps);
	PP_addOrSetAttribute("bot-color", s, m_vecProps);

	m_bSettingsChanged = true;
	xxx_UT_DEBUGMSG(("Maleesh ======================= setBorderColor\n"));
}

void AP_Dialog_Border_Shading::setShadingPattern(const std::string & sPattern)
{
	PP_addOrSetAttribute ("shading-pattern", sPattern, m_vecProps);
	m_bSettingsChanged = true;
	xxx_UT_DEBUGMSG(("Maleesh ======================= setShadingPattern\n"));
}

void AP_Dialog_Border_Shading::setShadingColor(const UT_RGBColor & clr)
{
	if (clr.isTransparent ()) {
		PP_removeAttribute ("shading-foreground-color", m_vecProps);
	} else {
		PP_addOrSetAttribute ("shading-foreground-color",
							  UT_std_string_sprintf("%02x%02x%02x", clr.m_red,
													clr.m_grn, clr.m_blu),
							  m_vecProps);
	}
	m_bSettingsChanged = true;
	xxx_UT_DEBUGMSG(("Maleesh ======================= setShadingColor\n"));
}

void AP_Dialog_Border_Shading::setShadingOffset(const std::string & /*sOffset*/)
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
	std::string lsOff = UT_std_string_sprintf("%d", LS_OFF);

	const std::string & sStyle = PP_getAttribute(lineStyle, m_vecProps);

	if (sStyle != lsOff) {
		return true;
	} else {
		return false;
	}
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

	const PP_PropertyVector & props = m_pBorderShading->getPropVector();
//
//  Draw the cell background (Shading)
//
	const std::string & shadingPattern = PP_getAttribute("shading-pattern", props);

	if(shadingPattern != BORDER_SHADING_SHADING_DISABLE) {
		const std::string & shadingColor = PP_getAttribute("shading-foreground-color", props);
		if (!shadingColor.empty())
		{
			UT_parseColor(shadingColor.c_str(), tmpCol);
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
		const std::string & topColor = PP_getAttribute("top-color", props);
		if (!topColor.empty()) {
			UT_parseColor(topColor.c_str(), tmpCol);
			m_gc->setColor(tmpCol);
		} else {
			m_gc->setColor(black);
		}
		const std::string & topThickness = PP_getAttribute("top-thickness", props);
		if(!topThickness.empty()) {
			UT_sint32 iTopThickness = UT_convertToLogicalUnits(topThickness.c_str());
			m_gc->setLineWidth(iTopThickness);
		} else {
			m_gc->setLineWidth(m_gc->tlu(1));
		}

		painter.drawLine(pageRect.left + border, pageRect.top + border,
					   pageRect.left + pageRect.width - border, pageRect.top + border);
	}

	// left border
	if (m_pBorderShading->getLeftToggled())
	{
		const std::string & leftColor = PP_getAttribute("left-color", props);
		if (!leftColor.empty())	{
			UT_parseColor(leftColor.c_str(), tmpCol);
			m_gc->setColor(tmpCol);
		} else {
			m_gc->setColor(black);
		}
		const std::string & leftThickness = PP_getAttribute("left-thickness", props);
		if(!leftThickness.empty()) {
			UT_sint32 iLeftThickness = UT_convertToLogicalUnits(leftThickness.c_str());
			m_gc->setLineWidth(iLeftThickness);
		} else {
			m_gc->setLineWidth(m_gc->tlu(1));
		}
		painter.drawLine(pageRect.left + border, pageRect.top + border,
					   pageRect.left + border, pageRect.top + pageRect.height - border);
	}

	// right border
	if (m_pBorderShading->getRightToggled())
	{
		const std::string & rightColor = PP_getAttribute("right-color", props);
		if (!rightColor.empty()) {
			UT_parseColor(rightColor.c_str(), tmpCol);
			m_gc->setColor(tmpCol);
		} else {
			m_gc->setColor(black);
		}
		const std::string & rightThickness = PP_getAttribute("right-thickness", props);
		if(!rightThickness.empty()) {
			UT_sint32 iRightThickness = UT_convertToLogicalUnits(rightThickness.c_str());
			m_gc->setLineWidth(iRightThickness);
		} else {
			m_gc->setLineWidth(m_gc->tlu(1));
		}
		painter.drawLine(pageRect.left + pageRect.width - border, pageRect.top + border,
					   pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border);
	}

	// bottom border
	if (m_pBorderShading->getBottomToggled())
	{
		const std::string & bottomColor = PP_getAttribute("bot-color", props);
		if (!bottomColor.empty()) {
			UT_parseColor(bottomColor.c_str(), tmpCol);
			m_gc->setColor(tmpCol);
		} else {
			m_gc->setColor(black);
		}
		const std::string & botThickness = PP_getAttribute("bot-thickness", props);
		if(!botThickness.empty()) {
			UT_sint32 iBotThickness = UT_convertToLogicalUnits(botThickness.c_str());
			m_gc->setLineWidth(iBotThickness);
		} else {
			m_gc->setLineWidth(m_gc->tlu(1));
		}
		painter.drawLine(pageRect.left + border, pageRect.top + pageRect.height - border,
					   pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border);
	}
}
