/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2009 Hubert Figuiere
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef AP_UNIXDIALOG_LISTS_H
#define AP_UNIXDIALOG_LISTS_H

#include <vector>
#include <string>

#include "ap_Dialog_Lists.h"
#include "ut_timer.h"
#include "xap_GtkObjectHolder.h"

class XAP_UnixFrame;
class GR_CairoGraphics;

/*****************************************************************/

class AP_UnixDialog_Lists: public AP_Dialog_Lists
{
 public:
	AP_UnixDialog_Lists(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_Lists(void);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void			destroy(void);
	virtual void			activate(void);
	virtual void			notifyActiveFrame(XAP_Frame *pFrame);
	virtual void            runModal(XAP_Frame * pFrame);
	/* CALLBACKS */

	void					customChanged(void);
	void					applyClicked(void);
	void closeClicked(void);
	void					styleChanged( gint style);
	void					previewExposed(void);
	void                    setFoldLevel(UT_sint32 iLevel,bool bSet);

	/* Just Plain Useful Functions */

	void                    setListTypeFromWidget(void);
	void					setXPFromLocal(void);
	void					loadXPDataIntoLocal(void);
	void					updateFromDocument(void);
	void					setAllSensitivity(void);
	void					updateDialog(void);
	bool                                    dontUpdate(void);
	static void				autoupdateLists(UT_Worker * pTimer);
    virtual bool            isPageLists(void);
	virtual void            setFoldLevelInGUI(void);
 protected:
	virtual GtkWidget *		_constructWindow(void);
	GtkWidget *				_constructWindowContents(void);
	void					_setRadioButtonLabels(void);
	void					_connectSignals(void);
	void					_fillNumberedStyleMenu( GtkListStore *listmenu);
	void					_fillBulletedStyleMenu( GtkListStore *listmenu);
	void					_fillNoneStyleMenu( GtkListStore *listmenu);
	void					_gatherData(void);
	void					_getGlistFonts (std::vector<std::string> & glFonts);
	void					_fillFontMenu(GtkListStore* store);

	inline GtkWidget *		_getCloseButton(void) { return m_wClose; }
	inline GtkWidget *		_getApplyButton(void) { return m_wApply; }
	inline GtkWidget *		_getMainWindow(void) { return m_wMainWindow; }

	inline void				_setCloseButton(GtkWidget *w) { m_wClose = w; }
	inline void				_setApplyButton(GtkWidget *w) { m_wApply = w; }
	inline void				_setMainWindow(GtkWidget *w) { m_wMainWindow = w; }

 private:
	typedef enum
	{
		BUTTON_OK = GTK_RESPONSE_OK,
		BUTTON_CANCEL = GTK_RESPONSE_CANCEL,
		BUTTON_CLOSE = GTK_RESPONSE_CLOSE,
		BUTTON_APPLY = GTK_RESPONSE_APPLY,
		BUTTON_RESET
	} ResponseId ;

    std::vector<std::string>  m_glFonts;
	GR_CairoGraphics *		        m_pPreviewWidget;

	bool					m_bManualListStyle;
	bool					m_bDestroy_says_stopupdating;
	bool					m_bAutoUpdate_happening_now;
	bool                                    m_bDontUpdate;
	UT_Timer *				m_pAutoUpdateLists;

	GtkWidget *				m_wMainWindow;

	GtkWidget * m_wApply;
	GtkWidget * m_wClose;
	GtkWidget * m_wContents;
	GtkWidget * m_wStartNewList;
	GtkWidget * m_wApplyCurrent;
	GtkWidget * m_wStartSubList;
	GSList    * m_wRadioGroup;
	GtkWidget * m_wPreviewArea;
	GtkWidget * m_wDelimEntry;
	GtkWidget * m_wDecimalEntry;
	GtkAdjustment * m_oAlignList_adj;
	GtkWidget * m_wAlignListSpin;
	GtkAdjustment * m_oIndentAlign_adj;
	GtkWidget * m_wIndentAlignSpin;
	GtkComboBox * m_wFontOptions;
	GtkListStore * m_wFontOptions_menu;
	GtkWidget * m_wCustomFrame;
	GtkWidget * m_wCustomTable;
	GtkWidget * m_wCustomLabel;
	GtkComboBox * m_wListStyleBox;
	XAP_GtkObjectHolder<GtkListStore> m_wListStyleNumbered_menu;
	XAP_GtkObjectHolder<GtkListStore> m_wListStyleBulleted_menu;
	XAP_GtkObjectHolder<GtkListStore> m_wListStyleNone_menu;
	XAP_GtkObjectHolder<GtkListStore> m_wListStyle_menu;
	GtkComboBox * m_wListTypeBox;
	XAP_GtkObjectHolder<GtkListStore> m_wListType_menu;
	GtkAdjustment * m_oStartSpin_adj;
	GtkWidget * m_wStartSpin;
	GtkWidget * m_wStartSub_label;
	GtkWidget * m_wStartNew_label;
	gint m_iDelimEntryID;
	gint m_iDecimalEntryID;
	gint m_iStyleBoxID;
	gint m_iAlignListSpinID;
	gint m_iIndentAlignSpinID;
	UT_sint32  m_iPageLists;
	UT_sint32  m_iPageFold;
	UT_GenericVector<GtkWidget*>  m_vecFoldCheck;
	UT_NumberVector  m_vecFoldID;
};

#endif /* AP_UNIXDIALOG_LISTS_H */







