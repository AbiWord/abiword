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
	delete m_pszTabStops;
	UT_VECTOR_PURGEALL(fl_TabStop *, m_tabInfo);
}

AP_Dialog_Tab::tAnswer AP_Dialog_Tab::getAnswer(void) const
{
	return m_answer;
}

void AP_Dialog_Tab::_storeWindowData(void)
{
	UT_ASSERT(m_pFrame); // needs to be set from runModal for some of the event_'s to work

	FV_View *pView = (FV_View *)m_pFrame->getCurrentView();

	const XML_Char * properties[3];
	properties[0] = "tabstops";
	properties[1] = m_pszTabStops;
	properties[2] = 0;
	UT_DEBUGMSG(("AP_Dialog_Tab: Tab Stop [%s]\n",properties[1]));

	pView->setBlockFormat(properties);

	properties[0] = "default-tab-interval";
	properties[1] = _gatherDefaultTabStop();
	properties[2] = 0;
	UT_ASSERT(properties[1]);
	UT_DEBUGMSG(("AP_Dialog_Tab: Default Tab Stop [%s]\n",properties[1]));

	pView->setBlockFormat(properties);

}

void AP_Dialog_Tab::_populateWindowData(void)
{
	const XML_Char * szRulerUnits;
	if (getApp()->getPrefsValue(AP_PREF_KEY_RulerUnits,&szRulerUnits))
		m_dim = UT_determineDimension(szRulerUnits);
	else
		m_dim = DIM_IN;

	UT_ASSERT(m_pFrame); // needs to be set from runModal for some of the event_'s to work

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

	// save the tab string
	m_pszTabStops = new char[strlen(rulerInfo.m_pszTabStops) + 1];
	strcpy(m_pszTabStops, rulerInfo.m_pszTabStops);

	int iTab;
	fl_TabStop		*pTabInfo;
	for ( iTab = 0; iTab < rulerInfo.m_iTabStops; iTab++ )
	{
	
		// create new tab info
		pTabInfo = new fl_TabStop();
		UT_ASSERT(pTabInfo);


		(*rulerInfo.m_pfnEnumTabStops)( rulerInfo.m_pVoidEnumTabStopsData,
						iTab, pTabInfo);

		m_tabInfo.addItem(pTabInfo);
	}
	
	_setTabList(m_tabInfo.getItemCount());
	_setAlignment(FL_TAB_LEFT);

	const XML_Char ** propsBlock = NULL;
	pView->getBlockFormat(&propsBlock);

	_setDefaultTabStop((const XML_Char *)"0");

	if (propsBlock[0])
	{
		const XML_Char * sz;
		
		sz = UT_getAttribute("default-tab-interval", propsBlock);

		if(sz)
		{
			double inches = UT_convertToInches(sz);

			_setDefaultTabStop((const XML_Char *)UT_convertInchesToDimensionString(m_dim, inches));
		}

	}

	// enable/disable controls
	_initEnableControls();
}


// The initialize the controls (i.e., disable controls not coded yet)
void AP_Dialog_Tab::_initEnableControls()
{
	// alignment
	_controlEnable( id_ALIGN_BAR,			false );

	// buttons
	_controlEnable( id_BUTTON_SET,			false );
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
		UT_ASSERT(index < (UT_sint32)m_tabInfo.getItemCount());

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
	char buffer[20];

	buildTab(buffer, 20);

	int Dimension_size = 0;
	while(buffer[Dimension_size] != 0)
	{

		if(buffer[Dimension_size] == '/')
		{
			Dimension_size--;
			break;
		}

		Dimension_size++;
	}
	UT_uint32 i;
	// do we have the tab already.

	for (  i = 0; i < m_tabInfo.getItemCount(); i++ )
	{
		fl_TabStop *pTabInfo = (fl_TabStop *)m_tabInfo.getNthItem(i);
		UT_ASSERT(pTabInfo);

		// if we have a tab at that unit
		if ( memcmp(buffer, _getTabString(pTabInfo), Dimension_size) == 0 )
		{
			// Delete the tab.
	
			_deleteTabFromTabString(pTabInfo);

			break;
		}
	}

	// Add tab to list.

	int NewOffset = strlen(m_pszTabStops);
	char *p_temp = new char[NewOffset + 1 + strlen(buffer) + 1];
	strcpy(p_temp, m_pszTabStops);
	if(m_pszTabStops[0] != 0)
	{
		strcat(p_temp, ",");
	}
	strcat(p_temp, buffer);
	delete m_pszTabStops;
	m_pszTabStops = p_temp;

	UT_ASSERT(m_pFrame); // needs to be set from runModal for some of the event_'s to work

	FV_View *pView = (FV_View *)m_pFrame->getCurrentView();

	buildTabStops(pView->getGraphics(), m_pszTabStops, m_tabInfo);

	_setTabList(m_tabInfo.getItemCount());

	// Select the new or changed tab in the lis.

	for (i = 0; i < m_tabInfo.getItemCount(); i++ )
	{
		fl_TabStop *pTabInfo = (fl_TabStop *)m_tabInfo.getNthItem(i);
		UT_ASSERT(pTabInfo);

		// if we have a tab at that unit
		if ( memcmp(buffer, _getTabString(pTabInfo), Dimension_size) == 0 )
		{
			_setSelectTab(i);
			_setTabEdit( _getTabDimensionString(i) );
			break;
		}
	}

	// something changed...
	_event_somethingChanged();

}

void AP_Dialog_Tab::_event_Clear(void)
{
	UT_DEBUGMSG(("AP_Dialog_Tab::_event_Clear\n"));

	UT_sint32 index = _gatherSelectTab();

	if(index != -1)
	{
		UT_ASSERT(index < (UT_sint32)m_tabInfo.getItemCount());

		_deleteTabFromTabString((fl_TabStop *)m_tabInfo.getNthItem(index));

		UT_ASSERT(m_pFrame); // needs to be set from runModal for some of the event_'s to work

		FV_View *pView = (FV_View *)m_pFrame->getCurrentView();

		buildTabStops(pView->getGraphics(), m_pszTabStops, m_tabInfo);

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

	UT_ASSERT(m_pFrame); // needs to be set from runModal for some of the event_'s to work

	FV_View *pView = (FV_View *)m_pFrame->getCurrentView();

	delete m_pszTabStops;
	m_pszTabStops = new char [1]; m_pszTabStops[0] = 0;
	buildTabStops(pView->getGraphics(), m_pszTabStops, m_tabInfo);

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
		// fall through

	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
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
		//a = FL_TAB_BAR;
		//break;

	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		a = FL_TAB_LEFT;
	}
	return a;
}

void AP_Dialog_Tab::clearList()
{
	_clearList();

	UT_VECTOR_PURGEALL(fl_TabStop *, m_tabInfo);
}


void AP_Dialog_Tab::buildTab( char *buffer, UT_uint32 bufflen )
{
	// get current value from member
	const XML_Char* szOld = _gatherTabEdit();
	const XML_Char* szNew = UT_reformatDimensionString(m_dim, szOld); 

	snprintf( buffer, bufflen, "%s/%c%c", szNew, AlignmentToChar(_gatherAlignment()),
		 				((char)_gatherLeader())+'0');
}

void AP_Dialog_Tab::_event_somethingChanged()
{
	char buffer[0x100];

	buildTab( buffer, 0x100 );

	UT_DEBUGMSG(("AP_Dialog_Tab::_event_somethingChanged  [%s]\n", buffer ));

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

	for ( UT_uint32 i = 0; i < m_tabInfo.getItemCount(); i++ )
	{
		fl_TabStop *pTabInfo = (fl_TabStop *)m_tabInfo.getNthItem(i);
		UT_ASSERT(pTabInfo);

		// if we have a tab at that unit
		if ( !strcmp(buffer, _getTabString(pTabInfo)) )
		{
			bEnableClear = true;

			// if everything is the same, disable the set
			if ( pTabInfo->getType() == _gatherAlignment() &&
			     pTabInfo->getLeader() == _gatherLeader() )
				bEnableSet = false;

		}
	}

	_controlEnable( id_BUTTON_SET, bEnableSet );
	_controlEnable( id_BUTTON_CLEAR, bEnableClear );

	_controlEnable( id_BUTTON_CLEAR_ALL,	m_tabInfo.getItemCount() == 0 ? false : true );

}

char *AP_Dialog_Tab::_getTabDimensionString(UT_uint32 tabIndex)
{

	UT_ASSERT(tabIndex < m_tabInfo.getItemCount());

	fl_TabStop *pTabInfo = (fl_TabStop *)m_tabInfo.getNthItem(tabIndex);

	const char* pStart = &m_pszTabStops[pTabInfo->getOffset()];
	const char* pEnd = pStart;
	while (*pEnd && (*pEnd != '/'))
	{
		pEnd++;
	}

	UT_uint32 iLen = pEnd - pStart;
	UT_ASSERT(iLen<20);

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
	UT_ASSERT(iLen<20);

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



	memmove(m_pszTabStops + Offset, 
				m_pszTabStops + Offset + Tab_data_size,
				strlen(m_pszTabStops) - (Offset + Tab_data_size));

	m_pszTabStops[strlen(m_pszTabStops) - Tab_data_size] = 0;
}

#define SPIN_INCR_IN	0.1
#define SPIN_INCR_CM	0.5
#define SPIN_INCR_MM	1.0
#define SPIN_INCR_PI	6.0
#define SPIN_INCR_PT	1.0
#define SPIN_INCR_none	0.1


void AP_Dialog_Tab::_doSpin(tControl id, UT_sint32 amt)
{
  //	UT_ASSERT(amt); // zero makes no sense
	UT_ASSERT(id = id_SPIN_DEFAULT_TAB_STOP);
	if(amt == 0 )
	{
	        UT_DEBUGMSG(("AMOUNT = 0 amt = %d \n",amt));
	}
	// get current value from member
	const XML_Char* szOld = _gatherDefaultTabStop();
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

		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
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
	d += (dSpinUnit * (double) amt);
	if (d < dMin)
		d = dMin;

	const XML_Char* szNew = UT_formatDimensionString(dimSpin, d, szPrecision); 

	_setDefaultTabStop(szNew);
}

//TODO: Roll this function and the above into one function.
//      Most things will increment for us so we just need to bound
//      limit and make sure the settings are correct.
void AP_Dialog_Tab::_doSpinValue(tControl id, double value)
{
  //	UT_ASSERT(amt); // zero makes no sense
	UT_ASSERT(id = id_SPIN_DEFAULT_TAB_STOP);

	double d = value;

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

		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	// figure out spin precision, too
	const char * szPrecision = ".1";
	if ((dimSpin == DIM_PT) || 
		(dimSpin == DIM_PI))
		szPrecision = ".0";

#if 0
	// if needed, switch unit systems and round off
	UT_Dimension dimOld = dimSpin;

	if (dimOld != dimSpin)
	{
		double dInches = UT_convertToInches(szOld);
		d = UT_convertInchesToDimension(dInches, dimSpin); 
	}
#endif

	// value is now in desired units, so change it
	if (d < dMin)
		d = dMin;

	const XML_Char* szNew = UT_formatDimensionString(dimSpin, d, szPrecision); 

	_setDefaultTabStop(szNew);
}

