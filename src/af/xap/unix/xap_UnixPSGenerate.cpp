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

#include <string.h>

#include "ut_assert.h"
#include "xap_UnixPSGenerate.h"

ps_Generate::ps_Generate(const char * szFilename)
{
	m_szFilename = szFilename;
	m_fp = 0;
	m_bIsFile = UT_FALSE;
}

ps_Generate::~ps_Generate()
{
	UT_ASSERT(!m_fp);					// somebody didn't close the file
}

UT_Bool ps_Generate::openFile(UT_Bool bIsFile)
{
	if(bIsFile)
	{
		m_bIsFile = UT_TRUE;
		m_fp = fopen(m_szFilename, "w");
	}
	else
	{
		// We should most likely give some thought to the
		// security implications with a popen() (which is
		// a shell out to possibly-priveliged commands).
		// I think we're pretty safe since we're never going
		// to be run setuid root (let's hope).
		//
		// As an alternative, someone should investigate
		// a pipe()/fork()/execve() option, which gets around
		// the /bin/sh problems and is a bit more flexible
		m_bIsFile = UT_FALSE;
		
		m_fp = popen(m_szFilename, "w");
	}

	// This is not sufficient to catch a failed popen(), at least
	// on Linux.  This may be a bug in Linux's popen(), but more likely
	// most Unixes will give you a valid FP, but with all flags and fields
	// invalid.
	if (!m_fp)
		return UT_FALSE;
	
	return writeBytes("%!PS-Adobe-3.0\n");
}

void ps_Generate::closeFile(void)
{
	if (m_fp)
	{
		if(m_bIsFile)
			fclose(m_fp);
		else
			pclose(m_fp);
	}
	m_fp = 0;
}

void ps_Generate::abortFile(void)
{
	// abort the file.
	// TODO close and delete or otherwise cleanup.

	closeFile();
}

UT_Bool	ps_Generate::writeByte(UT_Byte byte)
{
	return fputc((char) byte, m_fp) != EOF;
}

UT_Bool ps_Generate::writeBytes(const char * sz)
{
	return writeBytes((const unsigned char *) sz);
}

UT_Bool ps_Generate::writeBytes(const unsigned char * sz)
{
	// is that strlen correct?  Will we lose sign data on
	// anything?
	return writeBytes((UT_Byte*)sz,strlen((const char *)sz));
}

UT_Bool ps_Generate::writeBytes(UT_Byte * pBytes, UT_uint32 length)
{
	UT_ASSERT(m_fp);
	UT_ASSERT(pBytes && (length>0));

	UT_ASSERT(length<256);				// DSC3.0 requirement
	
	return (fwrite(pBytes,sizeof(UT_Byte),length,m_fp)==length);
}

UT_Bool ps_Generate::formatComment(const char * szCommentName)
{
	char buf[1024];
	sprintf(buf,"%%%%%s\n",szCommentName);
	return writeBytes(buf);
}

UT_Bool ps_Generate::formatComment(const char * szCommentName, const char * szArg1)
{
	// write out "foo: arg1\n"
	// arg1 may be a list of space separated names,
	// as in: %%BoundingBox: 1 2 3 4\n"
	// we do not PS-escape arg1.
	// return true if successful.
	
	char buf[1024];
	sprintf(buf,"%%%%%s: %s\n",szCommentName,szArg1);
	return writeBytes(buf);
}

UT_Bool ps_Generate::formatComment(const char * szCommentName, const char **argv, int argc)
{
	// write out a comment line possibly with multiple arguments
	// we PS-escapify each arg as we output it.
	// return true if successful.
	
	char buf[1024];
	int bufLen;
	
	sprintf(buf,"%%%%%s:",szCommentName);
	for (int k=0; k<argc; k++)
	{
		bufLen = strlen(buf);
		if (bufLen+strlen(argv[k]) < 256)
		{
			// TODO see if we need to PS-style esacpe the string before we add it.
			sprintf(buf+bufLen," %s",argv[k]);
		}
		else
		{
			strcat(buf,"\n");
			if (!writeBytes(buf))
				return UT_FALSE;
			sprintf(buf,"%%%%+");
		}
	}
	bufLen = strlen(buf);
	if (bufLen > 3)						// 3==strlen("%%+")
	{
		strcat(buf,"\n");
		if (!writeBytes(buf))
			return UT_FALSE;
	}
	return UT_TRUE;
}

UT_Bool ps_Generate::formatComment(const char * szCommentName, const UT_Vector * pVec)
{
	// write out a comment line possibly with multiple arguments
	// we PS-escapify each arg as we output it.
	// return true if successful.
	
	char buf[1024];
	int bufLen;
	UT_uint32 argc = pVec->getItemCount();
	
	sprintf(buf,"%%%%%s:",szCommentName);
	for (UT_uint32 k=0; k<argc; k++)
	{
		const char * psz = (const char *)pVec->getNthItem(k);
		bufLen = strlen(buf);
		if (bufLen+strlen(psz) < 256)
		{
			// TODO see if we need to PS-style esacpe the string before we add it.
			sprintf(buf+bufLen," %s",psz);
		}
		else
		{
			strcat(buf,"\n");
			if (!writeBytes(buf))
				return UT_FALSE;
			sprintf(buf,"%%%%+");
		}
	}
	bufLen = strlen(buf);
	if (bufLen > 3)						// 3==strlen("%%+")
	{
		strcat(buf,"\n");
		if (!writeBytes(buf))
			return UT_FALSE;
	}
	return UT_TRUE;
}

			
