/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001, 2003 Hubert Figuiere
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

#ifndef AP_COCOADIALOG_TAB_H
#define AP_COCOADIALOG_TAB_H

#include "ut_types.h"
#include "ap_Dialog_Tab.h"
#include "xap_CocoaDialog_Utilities.h"
#import "xap_Cocoa_NSTableUtils.h"

class XAP_CocoaFrame;
class AP_CocoaDialog_Tab;
class AP_CocoaDialog_Tab_Proxy;


@interface AP_CocoaDialog_TabController : NSWindowController <XAP_CocoaDialogProtocol>
{
    IBOutlet NSBox *_alignmentBox;
    IBOutlet NSCell *_barCell;
    IBOutlet NSButton *_cancelBtn;
    IBOutlet NSCell *_centerCell;
    IBOutlet NSButton *_clearAllBtn;
    IBOutlet NSButton *_clearBtn;
    IBOutlet NSCell *_decimalCell;
    IBOutlet NSTextField *_defaultTabData;
    IBOutlet NSTextField *_defaultTabLabel;
    IBOutlet NSStepper *_defaultTabStepper;
    IBOutlet NSCell *_leaderNoneCell;
    IBOutlet NSCell *_leaderDotCell;
    IBOutlet NSCell *_leaderDashCell;
    IBOutlet NSCell *_leaderUnderlineCell;
    IBOutlet NSBox *_leaderBox;
    IBOutlet NSCell *_leftCell;
    IBOutlet NSButton *_okBtn;
    IBOutlet NSCell *_rightCell;
    IBOutlet NSButton *_setBtn;
    IBOutlet NSTextField *_tabClearLabel;
	IBOutlet NSTableView *_tabList;
    IBOutlet NSTextField *_tabPositionData;
    IBOutlet NSTextField *_tabPositionLabel;
	AP_CocoaDialog_Tab* _xap;
	AP_CocoaDialog_Tab_Proxy*	_proxy;
}
- (IBAction)cancelAction:(id)sender;
- (IBAction)clearAction:(id)sender;
- (IBAction)clearAllAction:(id)sender;
- (IBAction)defaultTabAction:(id)sender;
- (IBAction)defaultTabStepperAction:(id)sender;
- (IBAction)okAction:(id)sender;
- (IBAction)setAction:(id)sender;
- (IBAction)alignmentAction:(id)sender;
- (IBAction)leaderAction:(id)sender;
- (IBAction)positionEditAction:(id)sender;
- (void)tabListAction:(id)sender;
- (id)_lookupWidget:(AP_Dialog_Tab::tControl)ctlid;
@end

/*****************************************************************/
class AP_CocoaDialog_Tab: public AP_Dialog_Tab
{
public:
	AP_CocoaDialog_Tab(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_Tab(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);
	friend class AP_CocoaDialog_Tab_Proxy;
 protected:
	virtual void _controlEnable( tControl ctlid, bool value );
	// we implement these so the XP dialog can set/grab our data
#define SET_GATHER(a,t) virtual t _gather##a(void);  \
 					    virtual void    _set##a( t )
	SET_GATHER			(Alignment,			eTabType);
	SET_GATHER			(Leader,			eTabLeader);
	SET_GATHER			(DefaultTabStop,	const gchar*);


	// to populate the whole list
	virtual void _setTabList(UT_uint32 count);

	// get/set the selected tab
	// the list of n tabs are index 0..(n-1)
	// -1 deselects everything
	SET_GATHER			(SelectTab,			UT_sint32);

	// a pointer to the text in the edit box, MUST BE FREEd on get
	SET_GATHER			(TabEdit,			const char *);
#undef SET_GATHER

	virtual void _clearList();

	// private construction functions
    void event_OK(void);
    void event_Cancel(void);
	void event_Leader(id sender);
	void event_Alignment(id sender);

	eTabType	m_current_alignment;
	eTabLeader	m_current_leader;
	// callbacks can fire these events
private:
	AP_CocoaDialog_TabController*	m_dlg;
	XAP_StringListDataSource*		m_dataSource;
};

class AP_CocoaDialog_Tab_Proxy
{
public:
	AP_CocoaDialog_Tab_Proxy (AP_CocoaDialog_Tab* dialog)
		: m_dlg(dialog) { };
	void cancelAction(void)
		{ m_dlg->event_Cancel(); };
	void clearAction(void)
		{ m_dlg->_event_Clear(); };
	void clearAllAction(void)
		{ m_dlg->_event_ClearAll(); };
	void okAction(void)
		{ m_dlg->event_OK(); };
	void setAction(void)
		{ m_dlg->_event_Set(); };
	void alignmentAction(id sender)
		{ m_dlg->event_Alignment(sender); };
	void leaderAction(id sender)
		{ m_dlg->event_Leader(sender); };
	void positionEditAction(void)
		{ m_dlg->_event_TabChange(); };
	void tabListAction(UT_sint32 index)
		{ m_dlg->_event_TabSelected(index); };
private:
	AP_CocoaDialog_Tab*	m_dlg;
};

#endif /* AP_COCOADIALOG_TAB_H */



