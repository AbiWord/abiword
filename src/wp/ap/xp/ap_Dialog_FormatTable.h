/* AbiWord
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
	PT_DocPosition						getCellSource(void);
	PT_DocPosition						getCellDestination(void);
	virtual void                        startUpdater(void);
	virtual void                        stopUpdater(void);
	static void                         autoUpdateMC(UT_Worker * pTimer);
	void								addOrReplaceVecProp(UT_Vector &vec,
															const XML_Char * pszProp,
															const XML_Char * pszVal);
	virtual void                        setSensitivity(bool bSens) = 0;
    virtual void                        setActiveFrame(XAP_Frame *pFrame);
	void                                ConstructWindowName(void);
	void                                setAllSensitivities(void);
	void                                event_update(void);
	void                                finalize(void);
	void								applyChanges(void);
	void                                toggleLineType(toggle_button btn, bool enabled);
	void								setBorderColor(UT_RGBColor clr);
	void								setBackgroundColor(UT_RGBColor clr);
	void								_createPreviewFromGC(GR_Graphics * gc,
															 UT_uint32 width,
															 UT_uint32 height);
															 
	XML_Char *							m_leftColor;
	XML_Char *							m_rightColor;
	XML_Char *							m_topColor;
	XML_Char *							m_bottomColor;															 
	
	UT_sint32							m_lineStyle;
	UT_sint32							m_leftStyle;
	UT_sint32							m_rightStyle;
	UT_sint32							m_topStyle;
	UT_sint32							m_bottomStyle;
	
	XML_Char *							m_bgColor;
	XML_Char *							m_bgFillStyle;
													 
													 
protected:
	AP_Dialog_FormatTable::tAnswer		m_answer;
	char                                m_WindowName[100];
	AP_FormatTable_preview				*m_pFormatTablePreview;
	AP_FormatTable_preview_drawer		m_previewDrawer;
		
private:
	void                                _generateSrcDest(void);
	
	PT_DocPosition                      m_iCellSource;
	PT_DocPosition                      m_iCellDestination;
	UT_Vector                           m_vecProps;
	UT_Vector                           m_vecPropsRight;
	UT_Vector                           m_vecPropsBottom;

	UT_sint32                           m_iLeftStyle;
	UT_sint32                           m_iRightStyle;
	UT_sint32                           m_iTopStyle;
	UT_sint32                           m_iBottomStyle;

	UT_sint32                           m_iNumRows;
	UT_sint32                           m_iNumCols;
	fp_TableContainer *                 m_pTab;
	UT_Timer *                          m_pAutoUpdaterMC;
	
	// Handshake variables
	bool m_bDestroy_says_stopupdating;
	bool m_bAutoUpdate_happening_now;
};

#endif /* AP_DIALOG_FORMATTABLE_H */
