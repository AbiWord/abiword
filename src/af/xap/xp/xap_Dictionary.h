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


#ifndef XAP_DICTIONARY_H
#define XAP_DICTIONARY_H

#include <stdio.h>

#include "ut_types.h"
#include "ut_alphahash.h"

/*
	A simple custom dictionary class, which allows the user to add words 
	which aren't in the standard dictionary.  The contents are stored as
	UTF8 plaintext, with one word per line. 
*/
class XAP_Dictionary
{
public:
	XAP_Dictionary(const char * szFilename);
	~XAP_Dictionary();

	const char *		getShortName(void) const;

	bool				load(void);
	bool				save(void);

	bool				addWord(const UT_UCSChar * pWord, UT_uint32 len);
	bool				isWord(const UT_UCSChar * pWord, UT_uint32 len) const;

protected:
	bool				_openFile(const char * mode);
	UT_uint32			_writeBytes(const UT_Byte * pBytes, UT_uint32 length);
	bool				_writeBytes(const UT_Byte * sz);
	bool				_closeFile(void);
	void				_abortFile(void);

	bool				_parseUTF8(void);
	void				_outputUTF8(const UT_UCSChar * data, UT_uint32 length);

	const char *		m_szFilename;

	bool				m_bDirty;
	UT_AlphaHashTable	m_hashWords;

private:
	FILE *				m_fp;
};


#endif /* XAP_DICTIONARY_H */
