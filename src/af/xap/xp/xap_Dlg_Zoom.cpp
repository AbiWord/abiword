/* AbiSource Application Framework
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
#include "ut_debugmsg.h"

#include "xap_Dlg_Zoom.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"

XAP_Dialog_Zoom::XAP_Dialog_Zoom(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id)
	: AP_Dialog_NonPersistent(pDlgFactory,id)
{
	m_answer = a_OK;

	// this should really never appear, since setZoomPercent()
	// should always be called before the dialog is shown
	m_zoomPercent = 100;
}

XAP_Dialog_Zoom::~XAP_Dialog_Zoom(void)
{
}

XAP_Dialog_Zoom::tAnswer XAP_Dialog_Zoom::getAnswer(void) const
{
	// let our caller know if user hit ok, cancel, etc.
	return m_answer;
}

void XAP_Dialog_Zoom::setZoomPercent(UT_uint32 zoom)
{
	// If the percentage is "special", make the dialog read
	// the "special" case, otherwise use the custom percentage
	// setting.  We can't detect the Page Width and Whole Page
	// settings, although we could store those if we really wanted.

	switch(zoom)
	{
	case 200:
		m_zoomType = XAP_Dialog_Zoom::z_200;
		break;
	case 100:
		m_zoomType = XAP_Dialog_Zoom::z_100;
		break;
	case 75:
		m_zoomType = XAP_Dialog_Zoom::z_75;
		break;
	// can't detect PageWidth and WholePage
	default:
		m_zoomType = XAP_Dialog_Zoom::z_PERCENT;
	}

	// store the percentage
	m_zoomPercent = zoom;
}

XAP_Dialog_Zoom::zoomType XAP_Dialog_Zoom::getZoomType(void)
{
	return m_zoomType;
}

UT_uint32 XAP_Dialog_Zoom::getZoomPercent(void)
{
	// we deliver based on special cases first, then the custom percentage
	switch(m_zoomType)
	{
	case XAP_Dialog_Zoom::z_200:
		return 200;
	case XAP_Dialog_Zoom::z_100:
		return 100;
	case XAP_Dialog_Zoom::z_75:
		return 75;
    // we can't really do anything with these,
	// since it's up to the application to query for these two
	// types and do something special with them
	case XAP_Dialog_Zoom::z_PAGEWIDTH:
		return 0;
	case XAP_Dialog_Zoom::z_WHOLEPAGE:
		return 0;
	case XAP_Dialog_Zoom::z_PERCENT:
		// fall through
	default:
		if (m_zoomPercent > 1)
			return m_zoomPercent;
		else
			return 1;
	}
}
