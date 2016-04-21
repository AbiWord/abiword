/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "ap_Features.h"

#include "ut_assert.h"
#include "ut_string.h"
#include "ut_units.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xad_Document.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_Prefs.h"
#include "fv_View.h"
#include "fl_DocLayout.h"

#include "ap_Dialog_Tab.h"
#include "ap_Prefs_SchemeIds.h"
#include "ap_TopRuler.h"		// for AP_TopRulerInfo



AP_Dialog_Tab::AP_Dialog_Tab(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialogtabs"),
	  m_answer(a_OK),
	  m_pFrame(0),
	  m_dim(DIM_IN),
	  m_pCallbackFn(0),
	  m_closure(0)
{
}

AP_Dialog_Tab::~AP_Dialog_Tab(void)
{
	UT_VECTOR_PURGEALL(fl_TabStop *, m_tabInfo);
}

void AP_Dialog_Tab::setSaveCallback (TabSaveCallBack pCb, void * closure)
{
	m_pCallbackFn = pCb;
	m_closure = closure;
}

AP_Dialog_Tab::tAnswer AP_Dialog_Tab::getAnswer(void) const
{
	return m_answer;
}

void AP_Dialog_Tab::_storeWindowData()
{
	UT_return_if_fail (m_pFrame); // needs to be set from runModal for some of the event_'s to work

	FV_View *pView = (FV_View *)m_pFrame->getCurrentView();
	UT_return_if_fail (pView && m_pCallbackFn);

	(*m_pCallbackFn)(this, pView, m_pszTabStops.c_str(), _gatherDefaultTabStop(), m_closure);
}

void AP_Dialog_Tab::_populateWindowData(void)
{
	const gchar * szRulerUnits;
	if (getApp()->getPrefsValue(AP_PREF_KEY_RulerUnits,&szRulerUnits))
		m_dim = UT_determineDimension(szRulerUnits);
	else
		m_dim = DIM_IN;

	UT_return_if_fail (m_pFrame); // needs to be set from runModal for some of the event_'s to work

	FV_View *pView = (FV_View *)m_pFrame->getCurrentView();
	UT_return_if_fail (pView);

	// get the info used in the top ruler
	AP_TopRulerInfo rulerInfo;
	pView->getTopRulerInfo(&rulerInfo);

	UT_DEBUGMSG(("AP_Dialog_Tab::_populateWindowData\n"));
	UT_DEBUGMSG(("iTabStops=%d\tDefaultTabInterval=%d\ttabStops=%s\n",
				rulerInfo.m_iTabStops, 
				rulerInfo.m_iDefaultTabInterval, 
				rulerInfo.m_pszTabStops 
				));

	// save the tab string
	m_pszTabStops = rulerInfo.m_pszTabStops;

	int iTab;
	fl_TabStop		*pTabInfo;
	for ( iTab = 0; iTab < rulerInfo.m_iTabStops; iTab++ )
	{
	
		// create new tab info
		pTabInfo = new fl_TabStop();
		UT_return_if_fail (pTabInfo);


		(*rulerInfo.m_pfnEnumTabStops)( rulerInfo.m_pVoidEnumTabStopsData,
						iTab, pTabInfo);

		m_tabInfo.addItem(pTabInfo);
	}
	
	_setTabList(m_tabInfo.getItemCount());
	_setAlignment(FL_TAB_LEFT);

	PP_PropertyVector propsBlock;
	pView->getBlockFormat(propsBlock);

	_setDefaultTabStop((const gchar *)"0");

	if (!propsBlock.empty())
	{
		const std::string & sz = PP_getAttribute("default-tab-interval", propsBlock);
		if(!sz.empty())
		{
			double inches = UT_convertToInches(sz.c_str());

			_setDefaultTabStop((const gchar *)UT_convertInchesToDimensionString(m_dim, inches));
		}

	}
	// enable/disable controls
	_initEnableControls();
}


// The initialize the controls (i.e., disable controls not coded yet)
void AP_Dialog_Tab::_initEnableControls()
{
	// alignment
	_controlEnable( id_ALIGN_BAR,			true );

	// buttons
	// Un-comment this once changes detailed below in something changed are implemented.
	//_controlEnable( id_BUTTON_SET,			false );
	_controlEnable( id_BUTTON_SET,			true );
	_controlEnable( id_BUTTON_CLEAR,		false );

	_controlEnable( id_BUTTON_CLEAR_ALL,	m_tabInfo.getItemCount() == 0 ? false : true );
}

void AP_Dialog_Tab::_event_TabChange(void)
{
	_controlEnable(id_BUTTON_SET, true);
}

void AP_Dialog_Tab::_event_AlignmentChange(void)
{
	_controlEnable(id_BUTTON_SET, true);
}


void AP_Dialog_Tab::_event_TabSelected( UT_sint32 index )
{
	UT_DEBUGMSG(("AP_Dialog_Tab::_event_TabSelected\n"));

	if(index >= 0)
	{
		UT_return_if_fail (index < m_tabInfo.getItemCount());

		fl_TabStop *pTabInfo = (fl_TabStop *)m_tabInfo.getNthItem(index);

		// HACK - iType if 1..5 to be LEFT..BAR in the same order.  We SHOULD make
		// an enumerated type in block layout or something, or atleast define the
		// common set of constants.  ap_TopRuler.cpp defines all the constants
		// again.  Here, since enum is rel 0, i'm subtracting one and doing an
		// ugly type cast
		_setAlignment( pTabInfo->getType() );
		_setLeader( pTabInfo->getLeader() );

		_setTabEdit( _getTabDimensionString(index) );

		// something changed...
		_event_somethingChanged();
	}
}

void AP_Dialog_Tab::_event_Set(void)
{
	UT_String buffer;

	// check the validity of the input
	bool res = buildTab(buffer);
	if (!res)
	{
		// TODO: add a message box here to inform our user - MARCM
		return;
	}

	UT_DEBUGMSG(("DOM: %s\n", buffer.c_str()));

	const char *cbuffer = buffer.c_str();
	int Dimension_size = 0;
	while(cbuffer[Dimension_size] != 0)
	{

		if(cbuffer[Dimension_size] == '/')
		{
			Dimension_size--;
			break;
		}

		Dimension_size++;
	}
	UT_sint32 i;
	// do we have the tab already.

	for (  i = 0; i < m_tabInfo.getItemCount(); i++ )
	{
		fl_TabStop *pTabInfo = (fl_TabStop *)m_tabInfo.getNthItem(i);
		UT_return_if_fail (pTabInfo);

		// if we have a tab at that unit
		if ( memcmp(cbuffer, _getTabString(pTabInfo), Dimension_size) == 0 )
		{
			// Delete the tab.
	
			_deleteTabFromTabString(pTabInfo);

			break;
		}
	}

	// Add tab to list.
	if (!m_pszTabStops.empty()) {
		m_pszTabStops += ",";
	}
	m_pszTabStops += cbuffer;

	UT_return_if_fail (m_pFrame); // needs to be set from runModal for some of the event_'s to work

	FV_View *pView = static_cast<FV_View *>(m_pFrame->getCurrentView());
	UT_return_if_fail(pView);

	buildTabStops(m_pszTabStops.c_str(), m_tabInfo);

	_setTabList(m_tabInfo.getItemCount());

	// Select the new or changed tab in the list.

	for (i = 0; i < m_tabInfo.getItemCount(); i++ )
	{
		fl_TabStop *pTabInfo = m_tabInfo.getNthItem(i);
		UT_return_if_fail (pTabInfo);

		// if we have a tab at that unit
		if ( memcmp(cbuffer, _getTabString(pTabInfo), Dimension_size) == 0 )
		{
			_setSelectTab(i);
			_setTabEdit( _getTabDimensionString(i) );
			break;
		}
	}

	// something changed...
	_event_somethingChanged();

}

/*!
* Update the currently selected tab's properties.
* Ripped off from _event_Set(). This method does auto-apply.
*/
void AP_Dialog_Tab::_event_Update(void)
{
	fl_TabStop *pTabInfo1 = NULL;

	// check the validity of the input
	UT_String buffer;
	bool res = buildTab(buffer);
	if (!res)
	{
		// TODO: add a message box here to inform our user - MARCM
		return;
	}


	// delete tab
	UT_uint32 ndx = _gatherSelectTab();
	pTabInfo1 = m_tabInfo.getNthItem(ndx);
	_deleteTabFromTabString(pTabInfo1);
	m_tabInfo.deleteNthItem(ndx);


	// re-add
	const char *cbuffer = buffer.c_str();
	int Dimension_size = 0;
	while(cbuffer[Dimension_size] != 0)
	{

		if(cbuffer[Dimension_size] == '/')
		{
			Dimension_size--;
			break;
		}

		Dimension_size++;
	}

	// do we have the tab already.
	UT_sint32 i;
	for (i = 0; i < m_tabInfo.getItemCount(); i++ )
	{
		pTabInfo1 = (fl_TabStop *)m_tabInfo.getNthItem(i);
		UT_return_if_fail (pTabInfo1);

		// if we have a tab at that unit
		if ( memcmp(cbuffer, _getTabString(pTabInfo1), Dimension_size) == 0 )
		{
			// Delete the tab.
			_deleteTabFromTabString(pTabInfo1);
			break;
		}
	}

	// Add tab to list.
	if (!m_pszTabStops.empty()) {
		m_pszTabStops += ",";
	}
	m_pszTabStops += cbuffer;

	UT_return_if_fail (m_pFrame); // needs to be set from runModal for some of the event_'s to work

	FV_View *pView = static_cast<FV_View *>(m_pFrame->getCurrentView());
	UT_return_if_fail(pView);

	buildTabStops(m_pszTabStops.c_str(), m_tabInfo);

	_setTabList(m_tabInfo.getItemCount());

	// Select the new or changed tab in the list.

	for (i = 0; i < m_tabInfo.getItemCount(); i++ )
	{
		fl_TabStop *pTabInfo = m_tabInfo.getNthItem(i);
		UT_return_if_fail (pTabInfo);

		// if we have a tab at that unit
		if ( memcmp(cbuffer, _getTabString(pTabInfo), Dimension_size) == 0 )
		{
			_setSelectTab(i);
			_setTabEdit( _getTabDimensionString(i) );
			break;
		}
	}

	// something changed...
	_event_somethingChanged();
	_storeWindowData ();
}

void AP_Dialog_Tab::_event_Clear(void)
{
	UT_DEBUGMSG(("AP_Dialog_Tab::_event_Clear\n"));

	UT_sint32 index = _gatherSelectTab();

	if(index != -1)
	{
		UT_return_if_fail(index < m_tabInfo.getItemCount());

		_deleteTabFromTabString(m_tabInfo.getNthItem(index));

		UT_return_if_fail(m_pFrame); // needs to be set from runModal for some of the event_'s to work

		buildTabStops(m_pszTabStops.c_str(), m_tabInfo);

		_setTabList(m_tabInfo.getItemCount());

		if(m_tabInfo.getItemCount() > 0)
		{
			_setSelectTab(0);
			_event_TabSelected(0);
		}
		else
		{
			_setSelectTab(-1);
		}

		// something changed...
		_event_somethingChanged();
	}
}

void AP_Dialog_Tab::_event_ClearAll(void)
{
	UT_DEBUGMSG(("AP_Dialog_Tab::_event_ClearAll\n"));

	UT_return_if_fail(m_pFrame); // needs to be set from runModal for some of the event_'s to work

	m_pszTabStops.clear();

	buildTabStops(m_pszTabStops.c_str(), m_tabInfo);

	_clearList();

	// something changed...
	_event_somethingChanged();
}

/*static*/ unsigned char AP_Dialog_Tab::AlignmentToChar( eTabType a )
{
	char ch;

	switch ( a )
	{
	case FL_TAB_LEFT:
		ch = 'L';
		break;

	case FL_TAB_RIGHT:
		ch = 'R';
		break;

	case FL_TAB_CENTER:
		ch = 'C';
		break;

	case FL_TAB_DECIMAL:
		ch = 'D';
		break;

	case FL_TAB_BAR:
		ch = 'B';
		break;

	default:
		UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
		ch = 'L';
		break;
	}

	return ch;
}

/*static*/ eTabType AP_Dialog_Tab::CharToAlignment( unsigned char ch )
{
	eTabType a;
	switch ( ch )
	{
	case 'L':
		a = FL_TAB_LEFT;
		break;

	case 'R':
		a = FL_TAB_RIGHT;
		break;

	case 'C':
		a = FL_TAB_CENTER;
		break;

	case 'D':
		a = FL_TAB_DECIMAL;
		break;

	case 'B':					// not implemented, fall though
		a = FL_TAB_BAR;
		break;

	default:
		UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
		a = FL_TAB_LEFT;
	}
	return a;
}

void AP_Dialog_Tab::clearList()
{
	_clearList();

	UT_VECTOR_PURGEALL(fl_TabStop *, m_tabInfo);
}


bool AP_Dialog_Tab::buildTab( UT_String & buffer )
{
	// get current value from member
	const gchar* szOld = _gatherTabEdit();
	bool res = UT_isValidDimensionString(szOld, MAX_TAB_LENGTH);
	if (res)
	{
		const gchar* szNew = UT_reformatDimensionString(m_dim, szOld); 

		UT_String_sprintf( buffer, "%s/%c%c", szNew, AlignmentToChar(_gatherAlignment()),
		 					(static_cast<char>(_gatherLeader()))+'0');
	}
	
	return res;
}

void AP_Dialog_Tab::_event_somethingChanged()
{
	UT_String buffer;

	buildTab( buffer );
	const char *cbuffer = buffer.c_str();
	UT_DEBUGMSG(("AP_Dialog_Tab::_event_somethingChanged  [%s]\n", cbuffer ));

	// check to see if the current tab is in the list
	bool bEnableClear = false;
	bool bEnableSet   = true;		// only disabled if current selection exactly matches current ones
										// or there are no items in the list.

// this just looks broken for the initial tab thingie.
#if 0
	if(m_tabInfo.getItemCount() == 0)
	{
		bEnableSet = false;
	}
#endif

	for ( UT_sint32 i = 0; i < m_tabInfo.getItemCount(); i++ )
	{
		fl_TabStop *pTabInfo = m_tabInfo.getNthItem(i);
		UT_return_if_fail (pTabInfo);

		// if we have a tab at that unit
		if ( !strcmp(cbuffer, _getTabString(pTabInfo)) )
		{
			bEnableClear = true;

			// if everything is the same, disable the set
			if ( pTabInfo->getType() == _gatherAlignment() &&
			     pTabInfo->getLeader() == _gatherLeader() ){
      	// Disabled to fix bug 5143 and match behavior in the remainder of the program.  TODO: Cause focus to shift to OK button here,
				// and beef up the enable/disable routines for the set button.  Then, this can be re-enabled.
				// bEnableSet = false;
			}

		}
	}

	_controlEnable( id_BUTTON_SET, bEnableSet );
	_controlEnable( id_BUTTON_CLEAR, bEnableClear );

	_controlEnable( id_BUTTON_CLEAR_ALL,	m_tabInfo.getItemCount() == 0 ? false : true );

}

char *AP_Dialog_Tab::_getTabDimensionString(UT_sint32 tabIndex)
{

	UT_return_val_if_fail (tabIndex < m_tabInfo.getItemCount(), NULL);

	fl_TabStop *pTabInfo = m_tabInfo.getNthItem(tabIndex);

	const char* pStart = &m_pszTabStops[pTabInfo->getOffset()];
	const char* pEnd = pStart;
	while (*pEnd && (*pEnd != '/'))
	{
		pEnd++;
	}

	UT_uint32 iLen = pEnd - pStart;
	UT_return_val_if_fail (iLen<20, NULL);

	strncpy(buf, pStart, iLen);
	buf[iLen]=0;

	return buf;
}

char *AP_Dialog_Tab::_getTabString(fl_TabStop *pTabInfo)
{
	const char* pStart = &m_pszTabStops[pTabInfo->getOffset()];
	const char* pEnd = pStart;
	while (*pEnd && (*pEnd != ','))
	{
		pEnd++;
	}

	UT_uint32 iLen = pEnd - pStart;

	strncpy(buf, pStart, iLen);
	buf[iLen]=0;

	return buf;
}

void AP_Dialog_Tab::_deleteTabFromTabString(fl_TabStop *pTabInfo)
{
	int Tab_data_size = 0;
	int Offset = pTabInfo->getOffset();

	while(m_pszTabStops[Offset + Tab_data_size] != 0)
	{
		if(m_pszTabStops[Offset + Tab_data_size] == ',')
		{
			break;
		}

		Tab_data_size++;
	}

	if(Offset > 0)
	{
		// include leading comma.
		Offset--;
		Tab_data_size++;
	}

	if(Offset == 0)
	{
		// include trailing comma if there is one.

		if(m_pszTabStops[Offset + Tab_data_size] == ',')
		{
			Tab_data_size++;
		}
	}

	auto iter = m_pszTabStops.begin() + Offset;
	m_pszTabStops.erase(iter, iter + Tab_data_size);
}

#define SPIN_INCR_IN	0.1
#define SPIN_INCR_CM	0.5
#define SPIN_INCR_MM	1.0
#define SPIN_INCR_PI	6.0
#define SPIN_INCR_PT	1.0
#define SPIN_INCR_none	0.1


void AP_Dialog_Tab::_doSpin(tControl id, UT_sint32 amt)
{
	UT_DEBUGMSG (("ROB: _doSpin %d, %d\n", id, amt));

  //	UT_ASSERT(amt); // zero makes no sense
	UT_return_if_fail (id == id_SPIN_DEFAULT_TAB_STOP);
	if(amt == 0 )
	{
	        UT_DEBUGMSG(("AMOUNT = 0 amt = %d \n",amt));
	}
	// get current value from member
	const gchar* szOld = _gatherDefaultTabStop();
	double d = UT_convertDimensionless(szOld);

	// figure out which dimension and units to spin in
	UT_Dimension dimSpin = m_dim;
	double dSpinUnit = SPIN_INCR_PT;
	double dMin = 0.0;
	switch (dimSpin)
	{
	case DIM_IN:	
		dSpinUnit = SPIN_INCR_IN;	
		dMin = 0.1;
		break;

	case DIM_CM:	
		dSpinUnit = SPIN_INCR_CM;	
		dMin = 0.1;
		break;

	case DIM_MM:	
		dSpinUnit = SPIN_INCR_MM;	
		dMin = 1.0;
		break;

	case DIM_PI:	
		dSpinUnit = SPIN_INCR_PI;
		dMin = 6.0;
		break;

	case DIM_PT:	
		dSpinUnit = SPIN_INCR_PT;	
		dMin = 1.0;
		break;
	default:

		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	// figure out spin precision, too
	const char * szPrecision = ".1";
	if ((dimSpin == DIM_PT) || 
		(dimSpin == DIM_PI))
		szPrecision = ".0";

	// if needed, switch unit systems and round off
	UT_Dimension dimOld = UT_determineDimension(szOld, dimSpin);

	if (dimOld != dimSpin)
	{
		double dInches = UT_convertToInches(szOld);
		d = UT_convertInchesToDimension(dInches, dimSpin); 
	}

	// value is now in desired units, so change it
	d += (dSpinUnit * static_cast<double>(amt));
	if (d < dMin)
		d = dMin;

	const gchar* szNew = UT_formatDimensionString(dimSpin, d, szPrecision); 

	_setDefaultTabStop(szNew);
}
