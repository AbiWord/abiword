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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef XAP_FAKECLIPBOARD_H
#define XAP_FAKECLIPBOARD_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_vector.h"

/* #include "xap_Clipboard.h" */

struct _ClipboardItem;

class ABI_EXPORT XAP_FakeClipboard
{
public:
	XAP_FakeClipboard();
	virtual ~XAP_FakeClipboard();

	virtual bool			clearClipboard(void);

	virtual bool			addData(const char* format, const void* pData, UT_sint32 iNumBytes);
	virtual bool			getClipboardData(const char* format, void ** ppData, UT_uint32 * pLen);
	virtual bool			hasFormat(const char* format);

protected:
	_ClipboardItem*			_findFormatItem(const char*);

	UT_GenericVector<_ClipboardItem*> m_vecData;
};

#endif /* XAP_FAKECLIPBOARD_H */

