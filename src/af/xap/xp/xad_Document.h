/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998,1999 AbiSource, Inc.
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

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_string_class.h"
#include "time.h"


// fwd. decl.
class UT_StringPtrMap;

class XAP_ResourceManager;

class ABI_EXPORT AD_Document
{
public:
	AD_Document();
	void				ref(void);
	void				unref(void);

private:
	XAP_ResourceManager *	m_pResourceManager;
public:
	XAP_ResourceManager &	resourceManager () const { return *m_pResourceManager; }

	const char *			getFilename(void) const;
	virtual UT_uint32       getLastSavedAsType() = 0; 
	// TODO - this should be returning IEFileType, 
	// but that's AP stuff, so it's not here

	virtual UT_Error		readFromFile(const char * szFilename, int ieft) = 0;
	virtual UT_Error		importFile(const char * szFilename, int ieft, bool markClean = false) = 0;
	virtual UT_Error		newDocument() = 0;
	virtual bool			isDirty(void) const = 0;

	virtual bool			canDo(bool bUndo) const = 0;
	virtual bool			undoCmd(UT_uint32 repeatCount) = 0;
	virtual bool			redoCmd(UT_uint32 repeatCount) = 0;

	virtual UT_Error		saveAs(const char * szFilename, int ieft) = 0;
	virtual UT_Error		saveAs(const char * szFilename, int ieft, bool cpy) = 0;
	virtual UT_Error		save() = 0;

	/**
	 * Returns the # of seconds since the last save of this file 
	 */
	UT_uint32       getTimeSinceSave () const { return (time(NULL) - m_lastSavedTime); }

   	// "ignore all" list for spell check
   	bool					appendIgnore(const UT_UCSChar * pWord, UT_uint32 len);
   	bool					isIgnore(const UT_UCSChar * pWord, UT_uint32 len) const;
   	bool					enumIgnores(UT_uint32 k, const UT_UCSChar ** pszWord) const;
   	bool					clearIgnores();
   
	void					setEncodingName(const char * szEncodingName);
	const char *			getEncodingName() const;

protected:
	virtual ~AD_Document();		//  Use unref() instead.

	int				m_iRefCount;
	const char *	m_szFilename;
	UT_String		m_szEncodingName;

	UT_StringPtrMap *		m_pIgnoreList;
	time_t          m_lastSavedTime;
};


#endif /* AD_DOCUMENT_H */
