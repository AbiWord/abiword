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

#ifndef AP_QNXDIALOG_TAB_H
#define AP_QNXDIALOG_TAB_H

#include "ap_Dialog_Tab.h"
#include <Pt.h>

class XAP_QNXFrame;

/*****************************************************************/

class AP_QNXDialog_Tab: public AP_Dialog_Tab
{
public:
	AP_QNXDialog_Tab(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_QNXDialog_Tab(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	
protected:
	/*** Start inherited ***/

	virtual void _controlEnable( tControl id, UT_Bool value );

	// we implement these so the XP dialog can set/grab our data
#define SET_GATHER(a,t) virtual t _gather##a(void);  \
 					    virtual void    _set##a( t )
	SET_GATHER			(Alignment,			eTabType);
	SET_GATHER			(Leader,			eTabLeader);
	SET_GATHER			(DefaultTabStop,	const XML_Char *);

	// to populate the whole list
	virtual void _setTabList(UT_uint32 count);

	// get/set the selected tab
	// the list of n tabs are index 0..(n-1)
	// -1 deselects everything
	SET_GATHER			(SelectTab,			UT_sint32);

	// a pointer to the text in the edit box, MUST BE FREEd on get
	SET_GATHER			(TabEdit,			const char *);
#undef SET_GATHER

	// clear all the items from the tab list - only gui side
	virtual void			_clearList();

	/*** End inherited ***/
	static int s_ok_clicked			(PtWidget_t *w, void *data, PtCallbackInfo_t *info); 
	static int s_cancel_clicked		(PtWidget_t *w, void *data, PtCallbackInfo_t *info);
	static int s_apply_clicked			(PtWidget_t *w, void *data, PtCallbackInfo_t *info);

	static int s_set_clicked			(PtWidget_t *w, void *data, PtCallbackInfo_t *info);
	static int s_clear_clicked			(PtWidget_t *w, void *data, PtCallbackInfo_t *info);
	static int s_clear_all_clicked		(PtWidget_t *w, void *data, PtCallbackInfo_t *info);

	static int s_delete_clicked		(PtWidget_t *w, void *data, PtCallbackInfo_t *info);

	static int s_list_select			(PtWidget_t *w, void *data, PtCallbackInfo_t *info);
	static int s_list_deselect			(PtWidget_t *w, void *data, PtCallbackInfo_t *info);

	static int s_edit_change			(PtWidget_t *w, void *data, PtCallbackInfo_t *info);
	static int s_alignment_change		(PtWidget_t *w, void *data, PtCallbackInfo_t *info);
	static int s_leader_change 		(PtWidget_t *w, void *data, PtCallbackInfo_t *info);
	
	PtWidget_t *			_lookupWidget ( tControl id );
	PtWidget_t *			_constructWindow(void);
    void 					event_OK(void);
    void 					event_Cancel(void);
    void 					event_Apply(void);
    void 					event_WindowDelete(void);

	UT_Vector 				m_Widgets;
	eTabType				m_current_alignment;
	eTabLeader				m_current_leader;
	PtWidget_t				*m_mainWindow;
	int 					done;
};

#endif /* AP_QNXDIALOG_TAB_H */
