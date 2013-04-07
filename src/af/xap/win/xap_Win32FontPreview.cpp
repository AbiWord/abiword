/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
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


#include "ut_debugmsg.h"
#include "xap_Win32FontPreview.h"
#include "gr_Win32Graphics.h"
#include "xap_Win32FrameImpl.h"


XAP_Win32FontPreview::XAP_Win32FontPreview(XAP_Frame * /*pFrame*/, UT_sint32 left, UT_uint32 top)
	: XAP_FontPreview()
{
	m_left = left;
	m_top = top;
	
	// create the preview window here
}

XAP_Win32FontPreview::~XAP_Win32FontPreview(void)
{
	// and fill in the destructor
}
