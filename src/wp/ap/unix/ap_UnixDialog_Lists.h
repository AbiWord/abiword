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
        virtual void                    notifyActiveFrame(XAP_Frame *pFrame);
	
	/* CALLBACKS */

        void                            startChanged(void);
        void                            stopChanged(void);
	void                            applyClicked(void);
        void                            startvChanged(void);

	/* Just Plain Useful Functions */

	void                            setAllSensitivity(void);
	void                            updateDialog(void);
	static void                     autoupdateLists(UT_Timer * pTimer);
protected:
	virtual GtkWidget *		_constructWindow(void);
	GtkWidget *				_constructWindowContents(void);
	void					_populateWindowData(void);
	void					_connectSignals(void);

	UT_Bool                         m_bDestroy_says_stopupdating;
	UT_Bool                         m_bAutoUpdate_happening_now;
	UT_Timer *                      m_pAutoUpdateLists;

	GtkWidget *				m_wMainWindow;

	GtkWidget * m_wApply;
	GtkWidget * m_wClose;
	GtkWidget * m_wContents;
	GtkWidget * m_wCheckstartlist;
	GtkWidget * m_wCheckstoplist;
	GtkWidget * m_wNewlisttypel;
        GtkWidget * m_wOption_types;
        GtkWidget * m_wOption_types_menu;
	GtkWidget * m_wNew_startingvaluel;
	GtkWidget * m_wNew_startingvaluev;
	GtkWidget * m_wCur_listtype;
	GtkWidget * m_wCur_listtypev;
	GtkWidget * m_wCur_listlabel;
	GtkWidget * m_wCur_listlabelv;
        GtkWidget * m_wCur_changestart_button;
        GtkWidget * m_wCur_Option_types;
        GtkWidget * m_wCur_Option_types_menu;
	GtkWidget * m_wCur_startingvaluel;
	GtkWidget * m_wCur_startingvaluev;

};

#endif /* AP_UNIXDIALOG_LISTS_H */



