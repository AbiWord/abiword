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
 
#ifndef XAP_UNIXPSGENERATE_H
#define XAP_UNIXPSGENERATE_H

#include <stdio.h>
#include "ut_types.h"
#include "ut_vector.h"

class ps_Generate
{
public:
	ps_Generate(const char * szFilename);
	~ps_Generate();
	
	bool		openFile(bool bIsFile);
	void		closeFile(void);
	void		abortFile(void);
	bool		writeByte(UT_Byte byte);
	bool		writeBytes(const char * sz);
	bool		writeBytes(const unsigned char * sz);	
	bool		writeBytes(UT_Byte * pBytes, size_t length);
	bool		formatComment(const char * szCommentName);
	bool		formatComment(const char * szCommentName, const char * szArg1);
	bool		formatComment(const char * szCommentName, const char **argv, int argc);
	bool		formatComment(const char * szCommentName, const UT_Vector * pVec);

protected:
 	void 		doProtectFromPipe(void);
	void 		undoProtectFromPipe(void);

	const char *	m_szFilename;
	FILE *			m_fp;
	bool			m_bIsFile;
	void			(*m_pfOldSIGPIPEHandler)(int);

private:
	static void	pipeSignalHandler(int signum);
};

#endif /* XAP_UNIXPSGENERATE_H */

