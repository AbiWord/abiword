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

#ifndef XAP_QNXCLIPBOARD_H
#define XAP_QNXCLIPBOARD_H

#include "ut_types.h"
#include "ut_vector.h"
#include "ut_bytebuf.h"
#include "xap_Clipboard.h"
#include "xap_FakeClipboard.h"
#include "xap_QNXApp.h"

//////////////////////////////////////////////////////////////////

class XAP_QNXClipboard : public XAP_FakeClipboard
{
public:
	XAP_QNXClipboard(XAP_QNXApp * pQNXApp);
	virtual ~XAP_QNXClipboard();

	void				initialize(void);

	virtual bool		clearClipboard(void);

	virtual bool		addData(const char* format, void* pData, UT_sint32 iNumBytes);
	virtual bool		getClipboardData(const char* format, void** ppData, UT_uint32 *pLen);

	virtual bool		hasFormat(const char *format);

protected:
	XAP_QNXApp 			*m_pQNXApp;
};

#endif /* XAP_QNXCLIPBOARD_H */

