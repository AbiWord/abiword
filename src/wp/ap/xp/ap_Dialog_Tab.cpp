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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_units.h"
#include "ut_debugmsg.h"

#include "xap_Dialog_Id.h"
#include "xad_Document.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_Prefs.h"
#include "fv_View.h"
#include "fl_DocLayout.h"

#include "ap_Dialog_Tab.h"
#include "ap_Prefs_SchemeIds.h"
#include "ap_Strings.h"
#include "ap_TopRuler.h"		// for AP_TopRulerInfo

AP_Dialog_Tab::AP_Dialog_Tab(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id)
{
	m_answer = a_OK;
	m_pFrame = (XAP_Frame *)0;		// needs to be set from runModal for some of the event_'s to work
}

AP_Dialog_Tab::~AP_Dialog_Tab(void)
{
	UT_VECTOR_PURGEALL(tTabInfo *, m_tabInfo);
}

AP_Dialog_Tab::tAnswer AP_Dialog_Tab::getAnswer(void) const
{
	return m_answer;
}

void AP_Dialog_Tab::_storeWindowData(void)
{

}

void AP_Dialog_Tab::_populateWindowData(void)
{
	FV_View *pView = (FV_View *)m_pFrame->getCurrentView();

	// get the info used in the top ruler
	AP_TopRulerInfo rulerInfo;
	pView->getTopRulerInfo(&rulerInfo);

	UT_DEBUGMSG(("AP_Dialog_Tab::_populateWindowData\n"));
	UT_DEBUGMSG(("iTabStops=%d\tDefaultTabInterval=%d\ttabStops=%s\n",
				rulerInfo.m_iTabStops, 
				rulerInfo.m_iDefaultTabInterval, 
				rulerInfo.m_pszTabStops 
				));
	
	int iTab;
	tTabInfo		*pTabInfo;
	for ( iTab = 0; iTab < rulerInfo.m_iTabStops; iTab++ )
	{
	
		// create new tab info
		pTabInfo = new tTabInfo();
		UT_ASSERT(pTabInfo);

		// get tab information	
		(*rulerInfo.m_pfnEnumTabStops)( rulerInfo.m_pVoidEnumTabStopsData,
						iTab, pTabInfo->iPosition, pTabInfo->iType, pTabInfo->iOffset,
						pTabInfo->iLeader );

		// parse string stuff out
		int i = 0;
		while ( rulerInfo.m_pszTabStops[pTabInfo->iOffset + i] &&  
		        rulerInfo.m_pszTabStops[pTabInfo->iOffset + i] != '/' &&	// remove /...
		        rulerInfo.m_pszTabStops[pTabInfo->iOffset + i] != ',' ) 
			i++;

		// allocate a copy of the buffer (NOTE - we don't use all the space)
		pTabInfo->pszTab = new char[i+1];	
		memcpy( pTabInfo->pszTab, &rulerInfo.m_pszTabStops[pTabInfo->iOffset], i );
		pTabInfo->pszTab[i] = '\0';

		// debug msgs
		UT_DEBUGMSG(("position=%d str=%s\n", pTabInfo->iPosition, pTabInfo->pszTab ));
		m_tabInfo.addItem(pTabInfo);
	}
	
	_setTabList(m_tabInfo);

	// enable/disable controls
	_initEnableControls();
}


void AP_Dialog_Tab::_enableDisableLogic( tControl /* id */ )
{
#if 0
	switch (id)
	{

/*	- Since HIDE_ERRORS is not implemented, no need to toggle it on/off
	case id_CHECK_SPELL_CHECK_AS_TYPE:
		// if we 'check as we type', then enable the 'hide' option
		_controlEnable( id_CHECK_SPELL_HIDE_ERRORS, 
						_gatherSpellCheckAsType() );
		break;
*/

	default:
		// do nothing
		break;
	}	
#endif
}

// The initialize the controls (i.e., disable controls not coded yet)
void AP_Dialog_Tab::_initEnableControls()
{
	// alignment
	_controlEnable( id_ALIGN_BAR,			UT_FALSE );

	// leaders
	_controlEnable( id_LEADER_NONE,			UT_FALSE );
	_controlEnable( id_LEADER_DOT,			UT_FALSE );
	_controlEnable( id_LEADER_DASH,			UT_FALSE );
	_controlEnable( id_LEADER_UNDERLINE,	UT_FALSE );

	// buttons
	_controlEnable( id_BUTTON_SET,			UT_FALSE );
	_controlEnable( id_BUTTON_CLEAR,		UT_FALSE );

	if ( m_tabInfo.getItemCount() > 0 )
		_controlEnable( id_BUTTON_CLEAR_ALL,	UT_FALSE );
}

void AP_Dialog_Tab::_event_TabChange(void)
{
	UT_DEBUGMSG(("AP_Dialog_Tab::_event_TabChange\n"));
}

void AP_Dialog_Tab::_event_TabSelected( tTabInfo *pTabInfo )
{
	UT_DEBUGMSG(("AP_Dialog_Tab::_event_TabSelected\n"));

	m_CurrentTab = pTabInfo;

	// HACK - iType if 1..5 to be LEFT..BAR in the same order.  We SHOULD make
	// an enumerated type in block layout or something, or atleast define the
	// common set of constants.  ap_TopRuler.cpp defines all the constants
	// again.  Here, since enum is rel 0, i'm subtracting one and doing an
	// ugly type cast
	_setAlignment( (tAlignment)(pTabInfo->iType - 1) );

	// set the edit box's text
	_setTabEdit( pTabInfo->pszTab );

	// something changed...
	_event_somethingChanged();
}

void AP_Dialog_Tab::_event_Set(void)
{
	UT_DEBUGMSG(("AP_Dialog_Tab::_event_Set\n"));
}

void AP_Dialog_Tab::_event_Clear(void)
{
	UT_DEBUGMSG(("AP_Dialog_Tab::_event_Clear\n"));
}

void AP_Dialog_Tab::_event_ClearAll(void)
{
	UT_DEBUGMSG(("AP_Dialog_Tab::_event_ClearAll\n"));
}

/*static*/ unsigned char AP_Dialog_Tab::AlignmentToChar( AP_Dialog_Tab::tAlignment a )
{
	char ch;

	switch ( a )
	{
	case AP_Dialog_Tab::align_LEFT:
		ch = 'L';
		break;

	case AP_Dialog_Tab::align_RIGHT:
		ch = 'R';
		break;

	case AP_Dialog_Tab::align_CENTER:
		ch = 'C';
		break;

	case AP_Dialog_Tab::align_DECIMAL:
		ch = 'D';
		break;

	case AP_Dialog_Tab::align_BAR:
		// fall through

	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		ch = 'L';
		break;
	}

	return ch;
}

/*static*/ AP_Dialog_Tab::tAlignment AP_Dialog_Tab::CharToAlignment( unsigned char ch )
{
	tAlignment a;
	switch ( ch )
	{
	case 'L':
		a = align_LEFT;
		break;

	case 'R':
		a = align_RIGHT;
		break;

	case 'C':
		a = align_CENTER;
		break;

	case 'D':
		a = align_DECIMAL;
		break;

	case 'B':					// not implemented, fall though
		// a = align_BAR;
		// break;

	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		a = align_LEFT;

	}
	return a;
}

void AP_Dialog_Tab::clearList()
{
	_clearList();

	UT_VECTOR_PURGEALL(tTabInfo *, m_tabInfo);
}


void AP_Dialog_Tab::buildTab( char *buffer, int bufflen )
{
	// TODO - use snprintf
	sprintf( buffer, "%s/%c", _gatherTabEdit(), AlignmentToChar(_gatherAlignment()));
}

void AP_Dialog_Tab::_event_somethingChanged()
{
	char buffer[0x100];

	buildTab( buffer, 0x100 );

	UT_DEBUGMSG(("AP_Dialog_Tab::_event_somethingChanged  [%s]\n", buffer ));

	// check to see if the current tab is in the list
	UT_Bool bEnableClear = UT_FALSE;
	UT_Bool bEnableSet   = UT_TRUE;		// only disabled if current selection exactly matches current ones
	strcpy( buffer, _gatherTabEdit() );

	for ( UT_uint32 i = 0; i < m_tabInfo.getItemCount(); i++ )
	{
		tTabInfo *pTabInfo = (tTabInfo *)m_tabInfo.getNthItem(i);
		UT_ASSERT(pTabInfo);

		// if we have a tab at that unit
		if ( !strcmp(buffer, pTabInfo->pszTab) )
		{
			bEnableClear = UT_TRUE;

			// if everything is the same, disable the set
			if ( pTabInfo->iType == _gatherAlignment() &&
			     pTabInfo->iLeader == _gatherLeader() )
				bEnableSet = UT_FALSE;

		}
	}

	_controlEnable( id_BUTTON_SET, bEnableSet );
	_controlEnable( id_BUTTON_CLEAR, bEnableClear );

	// ???
	_setSelectTab( -1 );
}
