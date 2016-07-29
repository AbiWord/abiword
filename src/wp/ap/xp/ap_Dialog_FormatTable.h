/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Marc Maurer
 * Copyright (c) 2009-2016 Hubert Figuiere
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

#ifndef AP_DIALOG_FORMATTABLE_H
#define AP_DIALOG_FORMATTABLE_H

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
#define FORMAT_TABLE_NUMTHICKNESS 9

class UT_Timer;
class XAP_Frame;
class fp_TableContainer;
class AP_Dialog_FormatTable;

class ABI_EXPORT AP_FormatTable_preview_drawer
{
public:

	void			draw(GR_Graphics *gc, UT_Rect &rect);
};

class ABI_EXPORT AP_FormatTable_preview : public XAP_Preview
{
public:

	AP_FormatTable_preview(GR_Graphics * gc, AP_Dialog_FormatTable * pFormatTable);
	virtual ~AP_FormatTable_preview(void);

	// data twiddlers
	void			draw(const UT_Rect *clip=NULL);
	GR_Graphics *   getGraphics(void) const { return m_gc;}
private:
	AP_Dialog_FormatTable *  m_pFormatTable;
protected:

};

class ABI_EXPORT AP_Dialog_FormatTable : public XAP_Dialog_Modeless
{
public:
	AP_Dialog_FormatTable(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_FormatTable(void);

	virtual void					runModeless(XAP_Frame * pFrame) = 0;

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

	AP_Dialog_FormatTable::tAnswer		getAnswer(void) const;
	virtual void                        startUpdater(void);
	virtual void                        stopUpdater(void);
	static void                         autoUpdateMC(UT_Worker * pTimer);

	virtual void                        setSensitivity(bool bSens) = 0;
    virtual void                        setActiveFrame(XAP_Frame *pFrame);
	void                                ConstructWindowName(void);
	void                                event_update(void);
	void                                finalize(void);

	void                                setAllSensitivities(void);
	void 								setCurCellProps(void);
	void								setApplyFormatTo(FormatTable applyTo);
	void								applyChanges(void);
	void                                toggleLineType(toggle_button btn, bool enabled);
	void								setBorderColor(const UT_RGBColor & clr);
	void								setBackgroundColor(const UT_RGBColor & clr);
	virtual void						setBackgroundColorInGUI(UT_RGBColor clr) = 0;
	void                                setBorderThickness(const UT_UTF8String & sThick);
	virtual void                        setBorderThicknessInGUI(UT_UTF8String & sThick) = 0;
	void                                clearImage(void);
	void                                askForGraphicPathName(void);
	void                                ShowErrorBox(UT_String & sFile, UT_Error errorCode);
	void								_createPreviewFromGC(GR_Graphics * gc,
															 UT_uint32 width,
															 UT_uint32 height);
	PP_PropertyVector &					getPropVector() { return m_vecProps; }
	const PP_PropertyVector &			getPropVector() const { return m_vecProps; }

	/* We use this in Win32 to know the status of line and to set the push button using this value*/
	bool								getTopToggled() const;
	bool								getBottomToggled() const;
	bool								getRightToggled() const;
	bool								getLeftToggled() const;
	GR_Image *                          getImage(void) const { return m_pImage;}
	FG_Graphic *                        getGraphic(void) const { return m_pGraphic;}

	UT_RGBColor							m_borderColor;
	UT_sint32							m_lineStyle;
	gchar *							m_bgFillStyle;
	PP_PropertyVector                   m_vecProps;
	UT_UTF8String                       m_sBorderThickness;
protected:
	guint                               _findClosestThickness(const char *) const;
	AP_Dialog_FormatTable::tAnswer		m_answer;
	char                                m_WindowName[100];
	AP_FormatTable_preview				*m_pFormatTablePreview;
	AP_FormatTable_preview_drawer		m_previewDrawer;
	double      m_dThickness[FORMAT_TABLE_NUMTHICKNESS];

private:
	bool								_getToggleButtonStatus(const char * lineStyle) const;

	bool								m_bSettingsChanged;

	PP_PropertyVector                   m_vecPropsAdjRight;
	PP_PropertyVector                   m_vecPropsAdjBottom;

	UT_Timer *                          m_pAutoUpdaterMC;

	bool								m_borderToggled;

	FormatTable							m_ApplyTo;

	// Handshake variables
	bool m_bDestroy_says_stopupdating;
	bool m_bAutoUpdate_happening_now;

	PT_DocPosition                      m_iOldPos;
	UT_String                           m_sImagePath;
	IEGraphicFileType                   m_iGraphicType;
	GR_Image *                          m_pImage;
	FG_Graphic *                        m_pGraphic;
};

#endif /* AP_DIALOG_FORMATTABLE_H */
