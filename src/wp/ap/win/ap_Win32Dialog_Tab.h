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

#ifndef AP_WIN32DIALOG_TAB_H
#define AP_WIN32DIALOG_TAB_H

#include "ap_Dialog_Tab.h"
class XAP_Win32Frame;

/*****************************************************************/

class AP_Win32Dialog_Tab: public AP_Dialog_Tab
{
public:
	AP_Win32Dialog_Tab(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_Tab(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	
protected:

	void _controlEnable( tControl id, UT_Bool value );

#define SET_GATHER(a,t) virtual t _gather##a(void);  \
 					    virtual void    _set##a( t )
	SET_GATHER			(Alignment,			eTabType);
	SET_GATHER			(Leader,			eTabLeader);
	SET_GATHER			(DefaultTabStop,	UT_sint32);


	// to populate the whole list
	SET_GATHER			(TabList,			const UT_Vector &);

	// get/set the selected tab
	// the list of n tabs are index 0..(n-1)
	// -1 deselects everything
	SET_GATHER			(SelectTab,			UT_sint32);

	// a pointer to the text in the edit box, MUST BE FREEd on get
	SET_GATHER			(TabEdit,			const char *);
#undef SET_GATHER

	virtual void _clearList();

};

#endif /* AP_WIN32DIALOG_TAB_H */



