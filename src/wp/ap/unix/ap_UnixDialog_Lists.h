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

#ifndef AP_UNIXDIALOG_LISTS_H
#define AP_UNIXDIALOG_LISTS_H

#include "ap_Dialog_Lists.h"
#include "ut_timer.h"
#include "gr_UnixGraphics.h"

class XAP_UnixFrame;

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
	
	/* CALLBACKS */

	void					customChanged(void);
	void					applyClicked(void);
	void					styleChanged( gint style);
	void					previewExposed(void);
	void					setXPFromLocal(void);
	/* Just Plain Useful Functions */
	void					updateFromDocument(void);
	void					setAllSensitivity(void);
	void					updateDialog(void);
	bool                                    dontUpdate(void);
	static void				autoupdateLists(UT_Timer * pTimer);

 protected:
	virtual GtkWidget *		_constructWindow(void);
	GtkWidget *				_constructWindowContents(void);
	void					_setRadioButtonLabels(void);
	void					_connectSignals(void);
	void					_fillNumberedStyleMenu( GtkWidget *listmenu);
	void					_fillBulletedStyleMenu( GtkWidget *listmenu);
	void					_fillNoneStyleMenu( GtkWidget *listmenu);
	void					_loadXPDataIntoLocal(void);
	void					_gatherData(void);
	GList *					_getGlistFonts (void);
	void					_fillFontMenu(GtkWidget* menu);

	inline GtkWidget *		_getCloseButton(void) { return m_wClose; }
	inline GtkWidget *		_getApplyButton(void) { return m_wApply; }
	inline GtkWidget *		_getMainWindow(void) { return m_wMainWindow; }

	inline void				_setCloseButton(GtkWidget *w) { m_wClose = w; }
	inline void				_setApplyButton(GtkWidget *w) { m_wApply = w; }
	inline void				_setMainWindow(GtkWidget *w) { m_wMainWindow = w; }

 private:
	GList *					m_glFonts;
	GR_UnixGraphics *		m_pPreviewWidget;

	bool					m_bManualListStyle;
	bool					m_bDestroy_says_stopupdating;
	bool					m_bAutoUpdate_happening_now;
	bool					m_bisCustomFrameHidden;
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
	GtkObject * m_oAlignList_adj;
	GtkWidget * m_wAlignListSpin;
	GtkObject * m_oIndentAlign_adj;
	GtkWidget * m_wIndentAlignSpin;
	GtkObject * m_oLevelSpin_adj;
	GtkWidget * m_wFontOptions;
	GtkWidget * m_wFontOptions_menu;
	GtkWidget * m_wCustomFrame;
	GtkWidget * m_wCustomTable;
	GtkWidget * m_wCustomLabel;
	GtkWidget * m_wListStyleBox;
	GtkWidget * m_wListStyleNumbered_menu;
	GtkWidget * m_wListStyleBulleted_menu;
	GtkWidget * m_wListStyleNone_menu;
	GtkWidget * m_wListStyle_menu;
	GtkWidget * m_wListTypeBox;
	GtkWidget * m_wListType_menu;
	GtkObject * m_oStartSpin_adj;
	GtkWidget * m_wStartSpin;
	GtkWidget * m_wMenu_None;
	GtkWidget * m_wMenu_Bull;
	GtkWidget * m_wMenu_Num;
	GtkWidget * m_wStartSub_label;
	GtkWidget * m_wStartNew_label;
	gint m_iDelimEntryID;
	gint m_iDecimalEntryID;
	gint m_iStyleBoxID;
	gint m_iAlignListSpinID;
	gint m_iIndentAlignSpinID;
	gint m_iFontOptionsID;
};

#endif /* AP_UNIXDIALOG_LISTS_H */







