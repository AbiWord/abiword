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


#ifndef AD_DOCUMENT_H
#define AD_DOCUMENT_H

// TODO should the filename be UT_UCSChar rather than char ?

#include "ut_types.h"

class AD_Document
{
public:
	AD_Document();
	void				ref(void);
	void				unref(void);

	const char *			getFilename(void) const;

	virtual UT_Bool			readFromFile(const char * szFilename, int ieft) = 0;
	virtual UT_Bool			newDocument(void) = 0;
	virtual UT_Bool			isDirty(void) const = 0;

	virtual UT_Bool			canDo(UT_Bool bUndo) const = 0;
	virtual UT_Bool			undoCmd(UT_uint32 repeatCount) = 0;
	virtual UT_Bool			redoCmd(UT_uint32 repeatCount) = 0;
	
protected:
	virtual ~AD_Document();		//  Use unref() instead.

	int				m_iRefCount;
	const char *			m_szFilename;
};


#endif /* AD_DOCUMENT_H */
