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

#ifndef AP_UNIXDIALOG_TAB_H
#define AP_UNIXDIALOG_TAB_H

#include "ut_types.h"
#include "ap_Dialog_Tab.h"

class XAP_UnixFrame;

/*****************************************************************/
class AP_UnixDialog_Tab: public AP_Dialog_Tab
{
public:
	AP_UnixDialog_Tab(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_Tab(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

 protected:

	GtkWidget *_lookupWidget( tControl id );
	virtual void _controlEnable( tControl id, UT_Bool value );
        void _spinChanged(void);
	// we implement these so the XP dialog can set/grab our data
#define SET_GATHER(a,t) virtual t _gather##a(void);  \
 					    virtual void    _set##a( t )
	SET_GATHER			(Alignment,			eTabType);
	SET_GATHER			(Leader,			eTabLeader);
	SET_GATHER			(DefaultTabStop,	const XML_Char*);


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
	virtual void _clearList();

 protected:
	
	// private construction functions
	virtual GtkWidget * _constructWindow(void);
	virtual void _constructWindowContents(GtkWidget * windowTabs);
	virtual void _constructGnomeButtons(GtkWidget * windowTabs);

	// pointers to widgets we need to query/set
	// there are a ton of them in this dialog

	UT_Vector m_Widgets;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
	// unix specific
	eTabType	m_current_alignment;
	eTabLeader	m_current_leader;
	GtkWidget * m_GnomeButtons;
	GtkWidget * m_buttonOK;
	GtkWidget * m_buttonApply;
	GtkWidget * m_buttonCancel;
	GtkWidget * m_wTable;
	UT_sint32 m_iDefaultSpin;
	GtkObject *  m_oDefaultSpin_adj;
	UT_Bool	   m_bInSetCall;		// a flag set to tell the change routines to ignore this message
	UT_sint32  m_iGtkListIndex;		// the -1, 0.. (N-1) index for the N tabs

protected:
	// Unix call back handlers
	static void s_ok_clicked		( GtkWidget *, gpointer );
	static void s_cancel_clicked		( GtkWidget *, gpointer );
	static void s_apply_clicked		( GtkWidget *, gpointer );

	static void s_set_clicked		( GtkWidget *, gpointer );
	static void s_clear_clicked		( GtkWidget *, gpointer );
	static void s_clear_all_clicked		( GtkWidget *, gpointer );

	static void s_delete_clicked		( GtkWidget *, GdkEvent *, gpointer );

	static void s_list_select		( GtkWidget *, gpointer );
	static void s_list_deselect		( GtkWidget *, gpointer );

	static void s_edit_change		( GtkWidget *, gpointer );
	static void s_spin_default_changed	( GtkWidget *, gpointer );
	static void s_alignment_change		( GtkWidget *, gpointer );
	static void s_leader_change 		( GtkWidget *, gpointer );
	
	// callbacks can fire these events
    void event_OK(void);
    void event_Cancel(void);
    void event_Apply(void);
    void event_WindowDelete(void);
};

#endif /* AP_UNIXDIALOG_TAB_H */



