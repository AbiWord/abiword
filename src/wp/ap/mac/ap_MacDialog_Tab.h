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

#ifndef AP_MacDialog_Tab_H
#define AP_MacDialog_Tab_H

#include "ap_Dialog_Tab.h"

class XAP_BeOSFrame;

/*****************************************************************/

class AP_MacDialog_Tab: public AP_Dialog_Tab
{
	
public:
	AP_MacDialog_Tab(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_MacDialog_Tab(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	
protected:
	virtual void                    _controlEnable( tControl id, bool value );
	virtual void                    _setTabList( UT_uint32 count );
	virtual void                    _clearList();
	eTabType                        _gatherAlignment();
        void                            _setAlignment( eTabType a );
	eTabLeader                      _gatherLeader();
	void                            _setLeader( eTabLeader a );
	const XML_Char *                _gatherDefaultTabStop();
	void                            _setDefaultTabStop( const XML_Char* default_tab );
	UT_sint32                       _gatherSelectTab();
	void                            _setSelectTab( UT_sint32 v );
	const char *                    _gatherTabEdit();
	void                            _setTabEdit( const char *pszStr );
	
	class TabWindow* newwin;
};

#endif /* AP_MacDialog_Tab_H */
