/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (c) 2016 Hubert Figuiere
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

#ifndef AP_DIALOG_TAB_H
#define AP_DIALOG_TAB_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"
#include "ut_units.h"
#include "fl_BlockLayout.h"

class XAP_Frame;
class AP_Dialog_Tab;
class FV_View;

#define MAX_TAB_LENGTH 15 // the maximum length the the tab input can have

typedef void (*TabSaveCallBack)(AP_Dialog_Tab * pDlg,
								FV_View *pView, const char * szTabStops,
								const char * szDflTabStop, void * closure);

class ABI_EXPORT AP_Dialog_Tab : public XAP_Dialog_NonPersistent
{
 public:

	AP_Dialog_Tab(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_Tab(void);

	virtual void	runModal(XAP_Frame * pFrame) = 0;

	// answer from dialog
	typedef enum { a_OK, a_CANCEL } tAnswer;

	// control ids
	typedef enum { id_EDIT_TAB = 0, id_LIST_TAB,
				   id_SPIN_DEFAULT_TAB_STOP,

				   // should be in same order as eTabType (fl_BlockLayout.h)
				   id_ALIGN_LEFT, id_ALIGN_CENTER, id_ALIGN_RIGHT, id_ALIGN_DECIMAL, id_ALIGN_BAR,

				   // should be in same order as eTabLeader (fl_BlockLayout.h)
				   id_LEADER_NONE, id_LEADER_DOT, id_LEADER_DASH, id_LEADER_UNDERLINE,

				   id_BUTTON_SET, id_BUTTON_CLEAR, id_BUTTON_CLEAR_ALL,
				   id_BUTTON_OK, id_BUTTON_CANCEL,

				   id_last } tControl;

	AP_Dialog_Tab::tAnswer	getAnswer(void) const;

	// clear the tab list
	void clearList();

	static unsigned char AlignmentToChar( eTabType a );
	static eTabType		 CharToAlignment( unsigned char ch );

	void setSaveCallback (TabSaveCallBack, void * closure);

 protected:

	// to enable/disable a control
	virtual void _controlEnable( tControl id, bool value )=0;

	// disable controls appropriately
	void _initEnableControls();

	// initial population / final storage of window data
	void _populateWindowData(void);
	void _storeWindowData(void);

	// grab tab from the current text/align/leader controls
	bool buildTab( UT_String & buffer );

	UT_Dimension _getDimension () { return m_dim; }
	char *_getTabDimensionString(UT_sint32 tabIndex);
	char *_getTabString(fl_TabStop *pTabInfo);
	void _deleteTabFromTabString(fl_TabStop *pTabInfo);
	void _doSpin( tControl id, UT_sint32 amount);

	// the actual access functions
#define SET_GATHER(a,u) virtual u _gather##a(void) = 0; \
					 	virtual void    _set##a( u ) = 0
	SET_GATHER			(Alignment,			eTabType);
	SET_GATHER			(Leader,			eTabLeader);
	SET_GATHER			(DefaultTabStop,	const gchar*);

	// to populate the whole list
	virtual void _setTabList(UT_uint32 count) = 0;

	// get/set the selected tab
	// the list of n tabs are index 0..(n-1)
	// -1 deselects everything
	SET_GATHER			(SelectTab,			UT_sint32);

	// a pointer to the text in the edit box, MUST BE FREEd on get
	SET_GATHER			(TabEdit,			const char *);
#undef SET_GATHER

	// clear all the items from the tab list - only gui side
	virtual void _clearList()=0;

 protected:
	tAnswer				m_answer;
	XAP_Frame *			m_pFrame;
	UT_Dimension		m_dim;

	std::string		m_pszTabStops;	// from rulerInfo
	UT_GenericVector<fl_TabStop*>	m_tabInfo;		// list of fl_TabStop *

	// AP level handlers
	void _event_TabChange(void);		// when the edit box changes
	void _event_TabSelected( UT_sint32 index);	// when a list item is selected
	void _event_AlignmentChange(void);

	void _event_Set(void);				// buttons
	void _event_Update(void);
	void _event_Clear(void);
	void _event_ClearAll(void);

	void _event_somethingChanged();			// when anything changes - text, align, or leader

private:
	TabSaveCallBack m_pCallbackFn;
	void * m_closure;

	char buf[20];

};

#endif /* AP_DIALOG_PARAGRAPH_H */
