/* AbiSource Application Framework
 * Copyright (C) 2009 J.M. Maurer <uwog@uwog.net>
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

#include <windows.h>

#include "ap_Win32App.h"
#include "xap_Frame.h"
#include "ap_Win32Frame.h"
#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ut_debugmsg.h"
#include "ap_Win32Preview_Annotation.h"
#include "xap_Win32DialogHelper.h"

AP_Win32Preview_Annotation::AP_Win32Preview_Annotation(XAP_DialogFactory * pDlgFactory,XAP_Dialog_Id id)
	: AP_Preview_Annotation(pDlgFactory,id),
	m_hToolTip(NULL)
{
	UT_DEBUGMSG(("AP_Win32Preview_Annotation: Preview annotation for Unix platform\n"));
}

AP_Win32Preview_Annotation::~AP_Win32Preview_Annotation(void)
{
	UT_DEBUGMSG(("Preview Annotation deleted %p \n",this));
	destroy();
}

void AP_Win32Preview_Annotation::runModeless(XAP_Frame * pFrame)
{
	UT_DEBUGMSG(("Preview Annotation runModeless %p \n",this));
	UT_return_if_fail(pFrame);

	setActiveFrame(pFrame);

	if (!m_hToolTip)
	{
		AP_Win32Frame* pWin32Frame = static_cast<AP_Win32Frame*>(pFrame);
		AP_Win32FrameImpl* pWin32FrameImpl = pWin32Frame->getAPWin32FrameImpl();
		UT_return_if_fail(pWin32FrameImpl);
		_createToolTip(pWin32FrameImpl->getHwndDocument());
	}
}

void AP_Win32Preview_Annotation::_createToolTip(HWND hwndParent)
{
	UT_return_if_fail(!m_hToolTip);

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(XAP_App::getApp());
	HINSTANCE hinst = pWin32App->getInstance();

	// Create a tooltip.
    m_hToolTip = CreateWindowEx(WS_EX_TOPMOST,
        TOOLTIPS_CLASS, NULL,
        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_BALLOON,		
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        hwndParent, NULL, hinst, NULL);

    SetWindowPos(m_hToolTip, HWND_TOPMOST,
        0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    // Set up "tool" information. We can simply use the full document
	// window as the "tool": if one moves the mouse outside the
	// annotation, the tooltip will be destroyed and thus
	// won't pop up on an area with no annotation in it.
    TOOLINFO ti = { 0 };
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_SUBCLASS;
    ti.hwnd = hwndParent;
    ti.hinst = hinst;
    ti.lpszText = (LPSTR)getDescription().c_str();
    GetClientRect (hwndParent, &ti.rect);
    SendMessage(m_hToolTip, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);

	if (!getTitle().empty() || !getAuthor().empty())
	{
		// Set the title and/or author as the tooltip title
		std::string title;
		if (!getAuthor().empty())
		{
			title += getAuthor();
			if (!getTitle().empty())
				title += ": ";
		}
		title += getTitle();

		// The title can't exceed 100 chars (including the terminating \0 character)
		// according to http://msdn.microsoft.com/en-us/library/bb760414(VS.85).aspxs
		if (title.size() > 99)
			title = title.substr(0, 99);

		SendMessage(m_hToolTip, TTM_SETTITLE, (WPARAM)TTI_NONE, (LPARAM)title.c_str());
	}

	// We don't want to auto-hide the popup after is has been shown, but the maximum popup
	// time is 30 seconds. We use this long delay since people might want to carefully read 
	// the remarks, and it is annoying when the popup disappears while doing that.
	SendMessage(m_hToolTip, TTM_SETDELAYTIME, (WPARAM)TTDT_AUTOPOP, (LPARAM)MAKELONG(30*1000, 0));
}

void AP_Win32Preview_Annotation::activate(void)
{
	// stubbed out
}

void AP_Win32Preview_Annotation::draw(void)
{
	// stubbed out
}

XAP_Dialog * AP_Win32Preview_Annotation::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return new AP_Win32Preview_Annotation(pFactory,id);
}

void AP_Win32Preview_Annotation::destroy(void)
{
	if (m_hToolTip)
	{
		DestroyWindow(m_hToolTip);
		m_hToolTip = NULL;
	}
}
