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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef AP_DIALOG_FORMATFRAME_H
#define AP_DIALOG_FORMATFRAME_H

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_PropVector.h"
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
#define FORMAT_FRAME_NUMTHICKNESS 9

class UT_Timer;
class XAP_Frame;
class fp_TableContainer;
class AP_Dialog_FormatFrame;

class AP_FormatFrame_preview_drawer
{
public:

	void			draw(GR_Graphics *gc, UT_Rect &rect);
};

class AP_FormatFrame_preview : public XAP_Preview
{
public:

	AP_FormatFrame_preview(GR_Graphics * gc, AP_Dialog_FormatFrame * pFormatFrame);
	virtual ~AP_FormatFrame_preview(void);

	// data twiddlers
	void			draw(void);
	GR_Graphics *   getGraphics(void) const { return m_gc;} 
	/*void			set(UT_uint32 iColumns, bool bLines)
					{
						m_iColumns = iColumns;
						m_bLineBetween = bLines;
						draw();
					}*/
private:
	AP_FormatFrame_preview_drawer	m_previewDrawer;
	AP_Dialog_FormatFrame *  m_pFormatFrame;
protected:

};

class AP_Dialog_FormatFrame : public XAP_Dialog_Modeless
{
public:
	AP_Dialog_FormatFrame(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_FormatFrame(void);

	virtual void					runModeless(XAP_Frame * pFrame) = 0;

	typedef enum { a_OK, a_CLOSE } tAnswer;
	typedef enum { toggle_left, toggle_right, toggle_top, toggle_bottom } toggle_button;
	
	AP_Dialog_FormatFrame::tAnswer		getAnswer(void) const;
	virtual void                        startUpdater(void);
	virtual void                        stopUpdater(void);
	static void                         autoUpdateMC(UT_Worker * pTimer);

	virtual void                        setSensitivity(bool bSens) = 0;
    virtual void                        setActiveFrame(XAP_Frame *pFrame);
	void                                ConstructWindowName(void);
	void                                event_update(void);
	void                                finalize(void);
	void                                setWrapping(bool bWrap)
		{m_bSetWrapping = bWrap;}
	bool                                getWrapping(void) const
		{return m_bSetWrapping;}
	void                                setBorderThickness(UT_UTF8String & sThick);
	virtual void                        setBorderThicknessInGUI(UT_UTF8String & sThick) = 0;
	
	void                                setAllSensitivities(void);
	void 								setCurFrameProps(void);	
	void								applyChanges(void);
	void                                toggleLineType(toggle_button btn, bool enabled);
	void								setBorderColor(UT_RGBColor clr);
	void								setBGColor(UT_RGBColor clr);
	void                                clearImage(void);
	void                                askForGraphicPathName(void);
	void                                ShowErrorBox(UT_String & sFile, UT_Error errorCode);
	void								_createPreviewFromGC(GR_Graphics * gc,
															 UT_uint32 width,
															 UT_uint32 height);
	UT_PropVector &						getPropVector() { return m_vecProps; }

	/* We use this in Win32 to know the status of line and to set the push button using this value*/
	bool								getTopToggled();
	bool								getBottomToggled();
	bool								getRightToggled();
	bool								getLeftToggled();
	GR_Image *                          getImage(void) { return m_pImage;}
	FG_Graphic *                        getGraphic(void) { return m_pGraphic;}
				
	UT_RGBColor							m_borderColor;
	UT_sint32							m_lineStyle;
	XML_Char *							m_bgFillStyle;
	UT_PropVector                           m_vecProps;									
	UT_UTF8String                       m_sBorderThickness;
					 
protected:
	AP_Dialog_FormatFrame::tAnswer		m_answer;
	char                                m_WindowName[100];
	AP_FormatFrame_preview				*m_pFormatFramePreview;
	AP_FormatFrame_preview_drawer		m_previewDrawer;
private:
	bool								_getToggleButtonStatus(const char * lineStyle);

	bool								m_bSettingsChanged;

	UT_PropVector                           m_vecPropsAdjRight;
	UT_PropVector                           m_vecPropsAdjBottom;

	UT_Timer *                          m_pAutoUpdaterMC;
	
	bool								m_borderToggled;
	
	// Handshake variables
	bool m_bDestroy_says_stopupdating;
	bool m_bAutoUpdate_happening_now;

	PT_DocPosition                      m_iOldPos;
	UT_String                           m_sImagePath;
	IEGraphicFileType                   m_iGraphicType;
	GR_Image *                          m_pImage;
	FG_Graphic *                        m_pGraphic;

    bool                                m_bSetWrapping;
};

#endif /* AP_DIALOG_FORMATFRAME_H */
