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

#ifndef AP_DIALOG_FORMATTABLE_H
#define AP_DIALOG_FORMATTABLE_H

#include "ut_types.h"
#include "ut_misc.h"
#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"
#include "pt_Types.h"
#include "xap_Preview.h"

class UT_Timer;
class XAP_Frame;
class fp_TableContainer;
class AP_Dialog_FormatTable;

class AP_FormatTable_preview_drawer
{
public:

	void			draw(GR_Graphics *gc, UT_Rect &rect);
};

class AP_FormatTable_preview : public XAP_Preview
{
public:

	AP_FormatTable_preview(GR_Graphics * gc, AP_Dialog_FormatTable * pFormatTable);
	virtual ~AP_FormatTable_preview(void);

	// data twiddlers
	void			draw(void);

	/*void			set(UT_uint32 iColumns, bool bLines)
					{
						m_iColumns = iColumns;
						m_bLineBetween = bLines;
						draw();
					}*/

private:
	AP_FormatTable_preview_drawer	m_previewDrawer;
	AP_Dialog_FormatTable *  m_pFormatTable;
protected:

};

class AP_Dialog_FormatTable : public XAP_Dialog_Modeless
{
public:
	AP_Dialog_FormatTable(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_FormatTable(void);

	virtual void					runModeless(XAP_Frame * pFrame) = 0;

	typedef enum { a_OK, a_CLOSE } tAnswer;
	typedef enum { toggle_left, toggle_right, toggle_top, toggle_bottom } toggle_button;
	
	AP_Dialog_FormatTable::tAnswer		getAnswer(void) const;
	virtual void                        startUpdater(void);
	virtual void                        stopUpdater(void);
	static void                         autoUpdateMC(UT_Worker * pTimer);

	virtual void                        setSensitivity(bool bSens) = 0;
    virtual void                        setActiveFrame(XAP_Frame *pFrame);
	void                                ConstructWindowName(void);
	void                                event_update(void);
	void                                finalize(void);
	
	void								addOrReplaceVecProp(UT_Vector &vec,
															const XML_Char * pszProp,
															const XML_Char * pszVal);
	void								getVecProp(UT_Vector &vec,
												   const XML_Char * pszProp,
												   const XML_Char * &pszVal);
	void								removeVecProp(UT_Vector &vec, const XML_Char * pszProp);
	
	void                                setAllSensitivities(void);
	void 								setCurCellProps(void);	
	void								applyChanges(void);
	void                                toggleLineType(toggle_button btn, bool enabled);
	void								setBorderColor(UT_RGBColor clr);
	void								setBGColor(UT_RGBColor clr);
	
	void								_createPreviewFromGC(GR_Graphics * gc,
															 UT_uint32 width,
															 UT_uint32 height);

	/* We use this in Win32 to know the status of line and to set the push button using this value*/
	bool								getTopToggled();
	bool								getBottomToggled();
	bool								getRightToggled();
	bool								getLeftToggled();
				
	UT_RGBColor							m_borderColor;
	UT_sint32							m_lineStyle;
	XML_Char *							m_bgFillStyle;
										 
	UT_Vector                           m_vecProps;													 
protected:
	AP_Dialog_FormatTable::tAnswer		m_answer;
	char                                m_WindowName[100];
	AP_FormatTable_preview				*m_pFormatTablePreview;
	AP_FormatTable_preview_drawer		m_previewDrawer;
		
private:
	bool								_getToggleButtonStatus(const char * lineStyle);

	bool								m_bSettingsChanged;

	UT_Vector                           m_vecPropsAdjRight;
	UT_Vector                           m_vecPropsAdjBottom;

	UT_Timer *                          m_pAutoUpdaterMC;
	
	bool								m_borderToggled;
	
	// Handshake variables
	bool m_bDestroy_says_stopupdating;
	bool m_bAutoUpdate_happening_now;
	PT_DocPosition                      m_iOldPos;
};

#endif /* AP_DIALOG_FORMATTABLE_H */
