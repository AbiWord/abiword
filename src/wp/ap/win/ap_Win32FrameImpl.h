/* AbiWord
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

#ifndef AP_WIN32FRAMEIMPL_H
#define AP_WIN32FRAMEIMPL_H

#include "ap_Frame.h"
#include "xap_Win32FrameImpl.h"

class ABI_EXPORT AP_Win32FrameImpl : public XAP_Win32FrameImpl
{
 public:
	AP_Win32FrameImpl(AP_Frame *pFrame); 
	virtual XAP_FrameImpl * createInstance(XAP_Frame *pFrame, XAP_App *pApp);

 protected:
	virtual void _createToolbars();
	virtual void _refillToolbarsInFrameData();
	virtual void _rebuildToolbar(UT_uint32 ibar);

/*** Win32 helper functions ***/
	void _translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y);

 private:
	HWND						m_hwndDocument;	/* the actual document window */
};

#endif /* AP_WIN32FRAMEIMPL_H */
