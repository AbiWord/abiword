/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#ifndef AP_QNXDIALOG_LISTS_H
#define AP_QNXDIALOG_LISTS_H

#include "ap_Dialog_Lists.h"
#include "ut_timer.h"

class XAP_QNXFrame;

/*****************************************************************/

class AP_QNXDialog_Lists: public AP_Dialog_Lists
{
public:
	AP_QNXDialog_Lists(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_QNXDialog_Lists(void);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void			activate();
	virtual void			destroy();
    virtual void            notifyActiveFrame(XAP_Frame *pFrame);

	// CALLBACKS 

    void                            customChanged(void);
	void                            applyClicked(void);
	void                            typeChanged( int type);
	void                            previewExposed(void);
	void                            setMemberVariables(void);
	/* Just Plain Useful Functions */
	void                            fillWidgetFromDialog(void);
	void                            updateDialog(void);
	static void                     autoupdateLists(UT_Timer * pTimer);

protected:
	PtWidget_t *	_constructWindow(void);
	void			_populateWindowData(void);

	void                            _fillNumberedStyleMenu( PtWidget_t *listmenu);
	void                            _fillBulletedStyleMenu( PtWidget_t *listmenu);
	void                            _fillNoneStyleMenu( PtWidget_t *listmenu);
	void                            _setData(void);
	void                            _gatherData(void);

	PtWidget_t *					m_mainWindow;
	GR_QNXGraphics *				m_pPreviewWidget;

	bool                         m_bDestroy_says_stopupdating;
	bool                         m_bAutoUpdate_happening_now;
	bool                         m_bisCustomFrameHidden;
	UT_Timer *                      m_pAutoUpdateLists;

	//List things ...
/*
	PtWidget_t * m_wListStyleNumbered_menu;
	PtWidget_t * m_wListStyleBulleted_menu;
	PtWidget_t * m_wListStyleNone_menu;
*/
    PtWidget_t * m_wListStyle_menu;
	PtWidget_t * m_wListTypeBox;
	PtWidget_t * m_wListType_menu;

	PtWidget_t * m_wMenu_None;
	PtWidget_t * m_wMenu_Bull;
	PtWidget_t * m_wMenu_Num;

	//Custom box
	PtWidget_t * m_wCustomFrame;
	PtWidget_t * m_wCustomLabel;
	//Custom box entries
	PtWidget_t * m_wDelimEntry;
	PtWidget_t * m_wListStyleBox;
	PtWidget_t * m_wLevelSpin;
	PtWidget_t * m_wStartSpin;
	PtWidget_t * m_wAlignListSpin;
	PtWidget_t * m_wIndentAlignSpin;
	PtWidget_t * m_wFontOptions;
	PtWidget_t * m_wFontOptions_menu;

	//Preview pieces
	PtWidget_t * m_wPreviewGroup;
	PtWidget_t * m_wPreviewArea;

	//Radio buttons
	PtWidget_t * m_wStartNewList;
	PtWidget_t * m_wApplyCurrent;
	PtWidget_t * m_wStartSubList;
	PtWidget_t * m_wResumeList;

	//Action buttons
	PtWidget_t * m_wApply;
	PtWidget_t * m_wClose;

	UT_Vector	m_styleVector;

};

#endif /* AP_QNXDIALOG_LISTS_H */






