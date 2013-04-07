/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2005 Robert Staudinger <robsta@stereolyzer.net>
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

#ifndef AP_UNIXDIALOG_TAB_H
#define AP_UNIXDIALOG_TAB_H

#include <gtk/gtk.h>

#include "ut_types.h"
#include "ap_Dialog_Tab.h"

class XAP_UnixFrame;

class AP_UnixDialog_Tab: public AP_Dialog_Tab
{
public:
	AP_UnixDialog_Tab		   (XAP_DialogFactory *pDlgFactory,
								XAP_Dialog_Id 	   id);
	virtual ~AP_UnixDialog_Tab (void);

	static XAP_Dialog *		static_constructor (XAP_DialogFactory *pDlgFactory,
												XAP_Dialog_Id 	   id);

	virtual void			runModal		   (XAP_Frame *pFrame);

	// Event-Handler
	void onDefaultTabChanged  (double value);
	void onDefaultTabFocusOut (void);
	void onTabSelected 		  (void);
	void onPositionChanged 	  (double value);
	void onPositionFocusOut	  (void);
	void onAlignmentChanged   (void);
	void onLeaderChanged 	  (void);
	void onAddTab 			  (void);
	void onDeleteTab 		  (void);

	GtkWidget *getWindow (void) { return m_wDialog; }

 protected:

	GtkWidget *_lookupWidget( tControl id );
	virtual void _controlEnable( tControl id, bool value );
        void _spinChanged(void);
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

	// clear all the items from the tab list - only gui side
	virtual void _clearList();

	// private construction functions
	virtual GtkWidget * _constructWindow(void);

private:

	void 	   _connectSignals 	 (GtkBuilder *builder);
	UT_sint32  _getSelectedIndex (void);

	GtkBuilder *m_pBuilder;
	GtkWidget *m_wDialog;
	GtkWidget *m_sbDefaultTab;
	GtkWidget *m_exUserTabs;
	GtkWidget *m_lvTabs;
	GtkWidget *m_btDelete;
	GtkWidget *m_sbPosition;
	GtkWidget *m_cobAlignment;
	GtkWidget *m_cobLeader;
	GtkTreeSelection *m_tsSelection;

	gchar *m_AlignmentMapping[__FL_TAB_MAX];
	gchar *m_LeaderMapping[__FL_LEADER_MAX];

	guint m_hSigDefaultTabChanged;
	guint m_hSigPositionChanged;
	guint m_hSigAlignmentChanged;
	guint m_hSigLeaderChanged;
	guint m_hTabSelected;
};

#endif /* AP_UNIXDIALOG_TAB_H */



