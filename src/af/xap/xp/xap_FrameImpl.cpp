/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998-2002 AbiSource, Inc.
 * Copyright (C) 2002 
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

#include "xap_Frame.h"
#include "xap_FrameImpl.h"
#include "xap_App.h"
#include "xad_Document.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "ut_string.h"
#include "ev_Keyboard.h"
#include "ev_Mouse.h"
#include "ev_Toolbar.h"

#if defined(XP_UNIX_TARGET_GTK) || (defined(__APPLE__) && defined(__MACH__)) || defined(WIN32) || defined(__QNXNTO__)

/*** Also look at xap_Frame.cpp, half the XAP_FrameImpl seems to be there!!! ***/

XAP_FrameImpl::XAP_FrameImpl(XAP_Frame *pFrame) :
	m_pMouse(0),
	m_pKeyboard(0),
	m_iFrameMode(XAP_NormalFrame),
	m_ViewAutoUpdaterID(0),
	m_ViewAutoUpdater(NULL),
	m_szToolbarLabelSetName(0),
	m_szToolbarAppearance(0),
	m_szMenuLayoutName(0),
	m_szMenuLabelSetName(0),
	m_pFrame(pFrame)
{
}

XAP_FrameImpl::~XAP_FrameImpl(void)
{
  	DELETEP(m_pKeyboard);
	DELETEP(m_pMouse);

	if(m_ViewAutoUpdaterID != 0) {
		m_ViewAutoUpdater->stop();
	}

	DELETEP(m_ViewAutoUpdater);

	FREEP(m_szMenuLayoutName);
	FREEP(m_szMenuLabelSetName);

	UT_VECTOR_FREEALL(char *,m_vecToolbarLayoutNames);	
	FREEP(m_szToolbarLabelSetName);
	FREEP(m_szToolbarAppearance);
	UT_VECTOR_PURGEALL(EV_Toolbar *, m_vecToolbars);

}

UT_RGBColor XAP_FrameImpl::getColorSelBackground () const
{
	return UT_RGBColor(192, 192, 192);
}

UT_RGBColor XAP_FrameImpl::getColorSelForeground () const
{
	return UT_RGBColor(255, 255, 255);
}

#define MAX_TITLE_LENGTH 256
bool XAP_FrameImpl::_updateTitle()
{
	/*
		The document title for this window has changed, so we need to:

		1. Update m_szTitle accordingly.	(happens here)
		2. Update the window title.		(happens in subclass)

		Note that we don't need to update the contents of the Window menu, 
		because that happens dynamically at menu pop-up time.  
	*/

	const char* szName = m_pFrame->m_pDoc->getFilename();

	if (szName && *szName) 
	{
		UT_UTF8String sUntruncatedString = szName;
		// WL_FIXME: we probably need a string truncation function, in the ut_utf8string class..
		UT_UTF8Stringbuf::UTF8Iterator iter = sUntruncatedString.getIterator ();
		iter = iter.start ();
		for (int currentSize = sUntruncatedString.size(); currentSize > MAX_TITLE_LENGTH; currentSize--)
			iter.advance();
		m_pFrame->m_sTitle = iter.current();
	}
	else
	{
		UT_ASSERT(m_pFrame->m_iUntitled);
		const XAP_StringSet * pSS = m_pFrame->m_pApp->getStringSet();
		m_pFrame->m_sTitle = UT_UTF8String_sprintf(m_pFrame->m_sTitle, 
							   pSS->getValueUTF8(XAP_STRING_ID_UntitledDocument).c_str(), 
							   m_pFrame->m_iUntitled);
	}

	m_pFrame->m_sNonDecoratedTitle = m_pFrame->m_sTitle;
	
	if (m_pFrame->m_nView)
	{
		// multiple top-level views, so append : & view number
		UT_UTF8String sBuf;
		UT_ASSERT(m_pFrame->m_nView < 10000);
		UT_UTF8String_sprintf(sBuf, ":%d", m_pFrame->m_nView);
		m_pFrame->m_sTitle += sBuf;
	}

	// only for non-untitled documents
	if (m_pFrame->m_pDoc->isDirty())
		m_pFrame->m_sTitle += " *";

	return true;
}
#endif
