/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 1999 John Brewer DBA Jera Design
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

#include "ap_FrameData.h"
#include "ap_MacFrame.h"
#include "ap_Prefs.h"

AP_MacFrame::AP_MacFrame(XAP_MacApp * app)
	: XAP_MacFrame(app)
{
}

AP_MacFrame::AP_MacFrame(AP_MacFrame * f)
	: XAP_MacFrame(static_cast<XAP_MacFrame *>(f))
{
}

AP_MacFrame::~AP_MacFrame(void)
{
}

bool AP_MacFrame::initialize()
{
	if (!initFrameData())
		return false;

	if (!XAP_MacFrame::initialize(AP_PREF_KEY_KeyBindings,AP_PREF_DEFAULT_KeyBindings,
									AP_PREF_KEY_MenuLayout, AP_PREF_DEFAULT_MenuLayout,
									AP_PREF_KEY_MenuLabelSet, AP_PREF_DEFAULT_MenuLabelSet,
									AP_PREF_KEY_ToolbarLayouts, AP_PREF_DEFAULT_ToolbarLayouts,
									AP_PREF_KEY_ToolbarLabelSet, AP_PREF_DEFAULT_ToolbarLabelSet))
		return false;

	_createTopLevelWindow();
	return true;
}

XAP_Frame *	AP_MacFrame::cloneFrame(void)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED); 
	return 0;
}

void AP_MacFrame::setStatusMessage(const char * /*szMsg*/)
{
	//TODO
	UT_ASSERT (UT_NOT_IMPLEMENTED); 
}                                                                        

UT_Error AP_MacFrame::loadDocument(const char * /*szFilename*/, int /*ieft*/)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED); 
	return true;
}

bool AP_MacFrame::initFrameData(void)
{
	UT_ASSERT(!((AP_FrameData*)m_pData));

	UT_ASSERT(m_app);
	
	AP_FrameData* pData = new AP_FrameData(m_app);
	m_pData = (void*) pData;
	
	return (pData ? true : false);
}

void AP_MacFrame::killFrameData(void)
{
	DELETEP(m_pData);
	m_pData = NULL;
}

bool	AP_MacFrame::close(void)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED); 
	return true;
}

bool	AP_MacFrame::raise(void)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED); 
	return true;
}

bool	AP_MacFrame::show(void)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED); 
	return true;
}

XAP_DialogFactory *AP_MacFrame::getDialogFactory(void)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED); 
	return 0;
}

void AP_MacFrame::setXScrollRange(void)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED); 
}

void AP_MacFrame::setYScrollRange(void)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED); 
}
