/* AbiSource Application Framework
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

#ifndef XAP_MACCLIPBOARD_H
#define XAP_MACCLIPBOARD_H

#include "ut_types.h"
#include "ut_vector.h"

#include "xap_Clipboard.h"
#include "xap_FakeClipboard.h"

class XAP_MacClipboard : public XAP_FakeClipboard
{
public:
	XAP_MacClipboard();
	
	// virtual bool			openClipboard(void);
	// virtual bool			closeClipboard(void);
	// virtual bool			clearClipboard(void);
	virtual bool			addData(const char* format, void* pData, UT_sint32 iNumBytes);
	virtual bool			hasFormat(const char * format);

protected:
	// virtual UT_uint32		_convertFormatString(const char * format) const;
	// virtual const char *	_convertToFormatString(UT_uint32 fmt) const;

	bool					m_bOpen;
	UT_Vector				m_vecFormat;
	UT_Vector				m_vecCF;
};

#endif /* XAP_MACCLIPBOARD_H */

