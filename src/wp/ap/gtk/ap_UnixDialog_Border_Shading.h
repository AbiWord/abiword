/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#pragma once

#include "xap_UnixDialog.h"
#include "ap_Dialog_Border_Shading.h"


class XAP_UnixFrame;
class GR_UnixCairoGraphics;

/*****************************************************************/

class AP_UnixDialog_Border_Shading
	: public AP_Dialog_Border_Shading
	, public XAP_UnixDialog
{
public:
	AP_UnixDialog_Border_Shading(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_Border_Shading(void);

	virtual void			runModeless(XAP_Frame * pFrame) override;

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events
	virtual void			event_Close(void);
	void event_previewInvalidate(void);
	void event_previewDraw(void);
	void                    event_BorderThicknessChanged(void);
	void                    event_BorderStyleChanged(void);
	void                    event_ShadingOffsetChanged(void);
	void					event_shadingPatternChange(void);

	virtual void           	setBorderThicknessInGUI(const std::string & sThick) override;
	virtual void            setBorderColorInGUI(const UT_RGBColor & clr) override;
	virtual void            setBorderStyleInGUI(const std::string & sStyle) override;
	virtual void		setShadingColorInGUI(const UT_RGBColor & clr) override;
	virtual void		setShadingPatternInGUI(const std::string & sPattern) override;
	virtual void		setShadingOffsetInGUI(const std::string & sOffset) override;

	virtual void           	setSensitivity(bool bsens) override;
	virtual void           	destroy(void) override;
	virtual void            activate(void) override;
	virtual void            notifyActiveFrame(XAP_Frame * pFrame) override;
	const GtkWidget 	  * getWindow (void) const { return m_windowMain; }

protected:
	typedef enum
	{
	  BUTTON_APPLY = GTK_RESPONSE_APPLY,
	  BUTTON_CLOSE = GTK_RESPONSE_CLOSE
	} ResponseId ;

	virtual GtkWidget *		_constructWindow(void);
	void					_populateWindowData(void);
	void					_connectSignals(void);
	void					_setShadingEnable(bool enable);
	GR_UnixCairoGraphics	* 		m_pPreviewWidget;

	// pointers to widgets we need to query/set
	GtkWidget * m_wApplyButton;
	GtkWidget * m_wCloseButton;

	GtkWidget * m_wBorderColorButton;
	GtkWidget * m_wShadingColorLabel;
	GtkWidget * m_wShadingColorButton;
	GtkWidget * m_wLineLeft;
	GtkWidget * m_wLineRight;
	GtkWidget * m_wLineTop;
	GtkWidget * m_wLineBottom;

	GtkWidget * m_wPreviewArea;
	GtkWidget * m_wBorderThickness;
	GtkWidget * m_wBorderStyle;
	GtkWidget * m_wShadingOffsetLabel;
	GtkWidget * m_wShadingOffset;
	GtkWidget * m_wShadingEnable;

	guint       m_iBorderThicknessConnect;
	guint       m_iBorderStyleConnect;
	guint       m_iShadingOffsetConnect;
	guint 		m_iShadingEnableConnect;
	guint		m_iLineLeftConnect;
	guint		m_iLineRightConnect;
	guint		m_iLineTopConnect;
	guint		m_iLineBotConnect;
};
