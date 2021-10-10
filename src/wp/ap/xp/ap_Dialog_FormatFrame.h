/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Marc Maurer
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

#ifndef AP_DIALOG_FORMATFRAME_H
#define AP_DIALOG_FORMATFRAME_H

#include <string>

#include "ut_types.h"
#include "ut_misc.h"
#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"
#include "pt_Types.h"
#include "pp_Property.h"
#include "xap_Preview.h"
#include "fv_View.h"
#include "ie_imp.h"
#include "ie_impGraphic.h"
#include "ie_exp.h"
#include "ie_types.h"
#include "fg_Graphic.h"
#include "xap_Dlg_FileOpenSaveAs.h"
#define FORMAT_FRAME_NUMTHICKNESS 9

class UT_Timer;
class XAP_Frame;
class fp_TableContainer;
class AP_Dialog_FormatFrame;

class ABI_EXPORT AP_FormatFrame_preview_drawer
{
public:

	void			draw(GR_Graphics *gc, UT_Rect &rect);
};

class ABI_EXPORT AP_FormatFrame_preview : public XAP_Preview
{
public:

	AP_FormatFrame_preview(GR_Graphics * gc, AP_Dialog_FormatFrame * pFormatFrame);
	virtual ~AP_FormatFrame_preview(void);

	// data twiddlers
	void drawImmediate(const UT_Rect* clip = nullptr) override;
	/*void			set(UT_uint32 iColumns, bool bLines)
					{
						m_iColumns = iColumns;
						m_bLineBetween = bLines;
						draw();
					}*/
private:
	AP_Dialog_FormatFrame *  m_pFormatFrame;
protected:

};

class ABI_EXPORT AP_Dialog_FormatFrame : public XAP_Dialog_Modeless
{
public:
	AP_Dialog_FormatFrame(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_FormatFrame(void);

	virtual void runModeless(XAP_Frame * pFrame) override = 0;

	typedef enum { a_OK, a_CLOSE } tAnswer;
	typedef enum { toggle_left, toggle_right, toggle_top, toggle_bottom } toggle_button;

	AP_Dialog_FormatFrame::tAnswer		getAnswer(void) const;
	virtual void                        startUpdater(void);
	virtual void                        stopUpdater(void);
	static void                         autoUpdateMC(UT_Worker * pTimer);

	virtual void                        setSensitivity(bool bSens) = 0;
    virtual void setActiveFrame(XAP_Frame *pFrame) override;
	void                                ConstructWindowName(void);
	void                                event_update(void);
	void                                finalize(void);

	void                                setWrapping(bool bWrap);
	bool                                getWrapping(void) const { return m_bSetWrapping; }

	void                                setAllSensitivities(void);
	void 								setCurFrameProps(void);
	void								applyChanges(void);
	void                                toggleLineType(toggle_button btn, bool enabled);
	void                                clearImage(void);
	void                                askForGraphicPathName(void);
	void                                ShowErrorBox(const std::string & sFile,
                                                     UT_Error errorCode);
	void								_createPreviewFromGC(GR_Graphics * gc,
															 UT_uint32 width,
															 UT_uint32 height);
	PP_PropertyVector &					getPropVector() { return m_vecProps; }

	GR_Image *                          getImage(void) { return m_pImage;}
	const FG_ConstGraphicPtr &          getGraphic(void) const { return m_pGraphic;}

	UT_RGBColor							m_borderColor;
	UT_sint32							m_lineStyle;

	PP_PropertyVector                   m_vecProps;

	void					setBGColor (const UT_RGBColor & clr);

	const UT_RGBColor &		backgroundColor () const { return m_backgroundColor; }

	void					setBorderColor (UT_RGBColor clr);

	void					setBorderColorAll (UT_RGBColor clr);

	void					setBorderColorRight  (const UT_RGBColor & rgb);
	void					setBorderColorLeft   (const UT_RGBColor & rgb);
	void					setBorderColorTop    (const UT_RGBColor & rgb);
	void					setBorderColorBottom (const UT_RGBColor & rgb);

	const UT_RGBColor &		borderColorRight ()  const { return m_borderColorRight;  }
	const UT_RGBColor &		borderColorLeft ()   const { return m_borderColorLeft;   }
	const UT_RGBColor &		borderColorTop ()    const { return m_borderColorTop;    }
	const UT_RGBColor &		borderColorBottom () const { return m_borderColorBottom; }

	void					setBorderLineStyleRight  (UT_sint32 linestyle);
	void					setBorderLineStyleLeft   (UT_sint32 linestyle);
	void					setBorderLineStyleTop    (UT_sint32 linestyle);
	void					setBorderLineStyleBottom (UT_sint32 linestyle);

	UT_sint32				borderLineStyleRight ()  const { return m_borderLineStyleRight;  }
	UT_sint32				borderLineStyleLeft ()   const { return m_borderLineStyleLeft;   }
	UT_sint32				borderLineStyleTop ()    const { return m_borderLineStyleTop;    }
	UT_sint32				borderLineStyleBottom () const { return m_borderLineStyleBottom; }

	bool					getRightToggled ()  const { return (m_borderLineStyleRight  ? true : false); }
	bool					getLeftToggled ()   const { return (m_borderLineStyleLeft   ? true : false); }
	bool					getTopToggled ()    const { return (m_borderLineStyleTop    ? true : false); }
	bool					getBottomToggled () const { return (m_borderLineStyleBottom ? true : false); }

	virtual void			setBorderThicknessInGUI (UT_UTF8String & sThick) = 0;

	void					setBorderThickness (const UT_UTF8String & sThick);

	void					setBorderThicknessAll (const UT_UTF8String & sThick);

	void					setBorderThicknessRight  (const UT_UTF8String & sThick);
	void					setBorderThicknessLeft   (const UT_UTF8String & sThick);
	void					setBorderThicknessTop    (const UT_UTF8String & sThick);
	void					setBorderThicknessBottom (const UT_UTF8String & sThick);

	const UT_UTF8String &	getBorderThicknessRight ()  const { return m_sBorderThicknessRight;  }
	const UT_UTF8String &	getBorderThicknessLeft ()   const { return m_sBorderThicknessLeft;   }
	const UT_UTF8String &	getBorderThicknessTop ()    const { return m_sBorderThicknessTop;    }
	const UT_UTF8String &	getBorderThicknessBottom () const { return m_sBorderThicknessBottom; }

	void					setBorderThicknessAll (float thickness); // border line thickness in pt [0.01pt .. 99.99pt]

	void					setBorderThicknessRight  (float thickness);
	void					setBorderThicknessLeft   (float thickness);
	void					setBorderThicknessTop    (float thickness);
	void					setBorderThicknessBottom (float thickness);

	float					borderThicknessRight ()  const { return m_borderThicknessRight;  }
	float					borderThicknessLeft ()   const { return m_borderThicknessLeft;   }
	float					borderThicknessTop ()    const { return m_borderThicknessTop;    }
	float					borderThicknessBottom () const { return m_borderThicknessBottom; }

	void					setPositionMode (FL_FrameFormatMode mode);

	FL_FrameFormatMode		positionMode () const { return m_iFramePositionTo; }

protected:
	AP_Dialog_FormatFrame::tAnswer		m_answer;
	char                                m_WindowName[100];
	AP_FormatFrame_preview				*m_pFormatFramePreview;
	AP_FormatFrame_preview_drawer		m_previewDrawer;
private:
    GR_Image * _makeImageForRaster(const std::string & name,
                                   GR_Graphics * pGraphics,
                                   const FG_ConstGraphicPtr & pG);
	bool					_getToggleButtonStatus(const char * lineStyle);

	bool					m_bSettingsChanged;

	UT_RGBColor				m_backgroundColor;

	UT_RGBColor				m_borderColorRight;
	UT_RGBColor				m_borderColorLeft;
	UT_RGBColor				m_borderColorTop;
	UT_RGBColor				m_borderColorBottom;

	UT_sint32				m_borderLineStyleRight;
	UT_sint32				m_borderLineStyleLeft;
	UT_sint32				m_borderLineStyleTop;
	UT_sint32				m_borderLineStyleBottom;

	float					m_borderThicknessRight;
	float					m_borderThicknessLeft;
	float					m_borderThicknessTop;
	float					m_borderThicknessBottom;

	UT_UTF8String			m_sBorderThickness;

	UT_UTF8String			m_sBorderThicknessRight;
	UT_UTF8String			m_sBorderThicknessLeft;
	UT_UTF8String			m_sBorderThicknessTop;
	UT_UTF8String			m_sBorderThicknessBottom;

	UT_Timer *                          m_pAutoUpdaterMC;

	// Handshake variables
	bool m_bDestroy_says_stopupdating;
	bool m_bAutoUpdate_happening_now;

	PT_DocPosition                      m_iOldPos;
    std::string                         m_sImagePath;
	IEGraphicFileType                   m_iGraphicType;
	GR_Image *                          m_pImage;
	FG_ConstGraphicPtr                  m_pGraphic;

	bool					m_bSensitive;
    bool					m_bSetWrapping;
    bool					m_bLineToggled;

	FL_FrameFormatMode		m_iFramePositionTo;
};

#endif /* AP_DIALOG_FORMATFRAME_H */
