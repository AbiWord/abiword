/* AbiSource Program Utilities
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
 


#ifndef PS_GENERATE_H
#define PS_GENERATE_H

#include <stdio.h>
#include "ut_types.h"
#include "ut_vector.h"

class ps_Generate
{
public:
	ps_Generate(const char * szFilename);
	~ps_Generate();
	
	UT_Bool		openFile(UT_Bool bIsFile);
	void		closeFile(void);
	void		abortFile(void);
	UT_Bool		writeBytes(const char * sz);
	UT_Bool		writeBytes(UT_Byte * pBytes, UT_uint32 length);
	UT_Bool		formatComment(const char * szCommentName);
	UT_Bool		formatComment(const char * szCommentName, const char * szArg1);
	UT_Bool		formatComment(const char * szCommentName, const char **argv, int argc);
	UT_Bool		formatComment(const char * szCommentName, const UT_Vector * pVec);

protected:
	const char *	m_szFilename;
	FILE *			m_fp;
	UT_Bool			m_bIsFile;
};

#endif /* PS_GENERATE_H */

