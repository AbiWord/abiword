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

#ifndef AP_DIALOG_BORDER_SHADING_H
#define AP_DIALOG_BORDER_SHADING_H

#include "ut_types.h"
#include "ut_misc.h"
#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"
#include "pt_Types.h"
#include "xap_Preview.h"
#include "fv_View.h"
#include "ie_imp.h"
#include "ie_impGraphic.h"
#include "ie_exp.h"
#include "ie_types.h"
#include "fg_Graphic.h"
#include "xap_Dlg_FileOpenSaveAs.h"

#define BORDER_SHADING_NUMTHICKNESS 9
#define BORDER_SHADING_NUMOFFSETS 9
#define BORDER_SHADING_NUMOFSTYLES 4

#define BORDER_SHADING_SHADING_DISABLE	"0"
#define BORDER_SHADING_SHADING_ENABLE	"1"

class UT_Timer;
class XAP_Frame;
class fp_TableContainer;
class AP_Dialog_Border_Shading;

class ABI_EXPORT AP_Border_Shading_preview_drawer
{
public:

	void			draw(GR_Graphics *gc, UT_Rect &rect);
};

class ABI_EXPORT AP_Border_Shading_preview : public XAP_Preview
{
public:

	AP_Border_Shading_preview(GR_Graphics * gc, AP_Dialog_Border_Shading * pFormatTable);
	virtual ~AP_Border_Shading_preview(void);

	// data twiddlers
	void drawImmediate(const UT_Rect* clip = nullptr) override;

private:
	AP_Dialog_Border_Shading *  m_pBorderShading;
protected:

};

class ABI_EXPORT AP_Dialog_Border_Shading : public XAP_Dialog_Modeless
{
public:
	AP_Dialog_Border_Shading(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_Border_Shading(void);

	virtual void runModeless(XAP_Frame * pFrame) override = 0;

	enum tAnswer {
          a_OK,
          a_CLOSE
        };
	enum toggle_button {
          toggle_left,
          toggle_right,
          toggle_top,
          toggle_bottom
        };

	AP_Dialog_Border_Shading::tAnswer	getAnswer(void) const;
	virtual void                        startUpdater(void);
	virtual void                        stopUpdater(void);
	static void                         autoUpdateMC(UT_Worker * pTimer);

	virtual void                        setSensitivity(bool bSens) = 0;
    virtual void                        setActiveFrame(XAP_Frame *pFrame) override;
	void                                ConstructWindowName(void);
	void                                event_update(void);
	void                                finalize(void);
	void                                setAllSensitivities(void);
	void 								setCurBlockProps(void);
	void								applyChanges(void);
	void                                toggleLineType(toggle_button btn, bool enabled);

	void                                setBorderColor(const UT_RGBColor & clr);
	void                                setBorderThickness(const std::string & sThick);
	void                                setBorderStyle(const std::string & sStyle);
	void                                setShadingColor(const UT_RGBColor & clr);
	void                                setShadingPattern(const std::string & sPattern);
	void                                setShadingOffset(const std::string & sOffset);

	virtual void                        setBorderStyleInGUI(const std::string & sStyle) = 0;
	virtual void                        setBorderThicknessInGUI(const std::string & sThick) = 0;
	virtual void                        setBorderColorInGUI(const UT_RGBColor & clr) = 0;
	virtual void						setShadingColorInGUI(const UT_RGBColor & clr) = 0;
	virtual void						setShadingPatternInGUI(const std::string & sPattern) = 0;
	virtual void						setShadingOffsetInGUI(const std::string & sOffset) = 0;

	void								_createPreviewFromGC(GR_Graphics * gc,
															 UT_uint32 width,
															 UT_uint32 height);
	PP_PropertyVector &					getPropVector()
        { return m_vecProps; }
	const PP_PropertyVector &			getPropVector() const
        { return m_vecProps; }

	bool								getTopToggled() const;
	bool								getBottomToggled() const;
	bool								getRightToggled() const;
	bool								getLeftToggled() const;

protected:
	guint                               _findClosestThickness(const char *) const;
	guint                               _findClosestOffset(const char *) const;
	AP_Dialog_Border_Shading::tAnswer	m_answer;
	char                                m_WindowName[100];
	AP_Border_Shading_preview			*m_pBorderShadingPreview;
	AP_Border_Shading_preview_drawer	m_previewDrawer;

	UT_RGBColor							m_borderColor;
	UT_sint32							m_lineStyle;
	gchar *								m_bgFillStyle;
	PP_PropertyVector                       m_vecProps;
	std::string                       m_sBorderThickness;

	double      						m_dThickness[BORDER_SHADING_NUMTHICKNESS];
	double      						m_dShadingOffset[BORDER_SHADING_NUMOFFSETS];

private:
	bool								_getToggleButtonStatus(const char * lineStyle) const;

	bool								m_bSettingsChanged;
	PT_DocPosition                      m_iOldPos;
	std::string							m_sDefaultStyle;

	UT_Timer *                          m_pAutoUpdaterMC;

	// Handshake variables
	bool m_bDestroy_says_stopupdating;
	bool m_bAutoUpdate_happening_now;
};

#endif /* AP_DIALOG_BORDER_SHADING_H */
