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

#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "ut_assert.h"
#include "xap_UnixPSGenerate.h"
#include "ut_string_class.h"

ps_Generate::ps_Generate(const char * szFilename)
{
	m_szFilename = szFilename;
	m_fp = 0;
	m_bIsFile = false;
	m_pfOldSIGPIPEHandler = NULL;
}

ps_Generate::~ps_Generate()
{
	UT_ASSERT(!m_fp);					// somebody didn't close the file
}

bool ps_Generate::openFile(bool bIsFile)
{
	if(bIsFile)
	{
		m_bIsFile = true;
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
		m_bIsFile = false;
		
		// This is not sufficient to catch a failed popen(), at least
		// on Linux.  This may be a bug in Linux's popen(), but more likely
		// most Unixes will give you a valid FP, but with all flags and fields
		// invalid.
		m_fp = NULL;
		if ((m_fp = popen(m_szFilename, "w")) == NULL)
			return false;
	}

	
	return writeBytes("%!PS-Adobe-2.0\n");
}

void ps_Generate::closeFile(void)
{
	if (m_fp)
	{
		doProtectFromPipe();
		if(m_bIsFile)
			fclose(m_fp);
		else
			pclose(m_fp);
		undoProtectFromPipe();
	}
	m_fp = 0;
}

void ps_Generate::abortFile(void)
{
	// abort the file.
	// TODO close and delete or otherwise cleanup.

	closeFile();
}

bool	ps_Generate::writeByte(UT_Byte byte)
{
	doProtectFromPipe();
	bool bSuccess = fputc((char) byte, m_fp) != EOF;
	undoProtectFromPipe();

	return bSuccess;
}

bool ps_Generate::writeBytes(const char * sz)
{
	return writeBytes((const unsigned char *) sz);
}

bool ps_Generate::writeBytes(const unsigned char * sz)
{
	// is that strlen correct?  Will we lose sign data on
	// anything?
	return writeBytes((UT_Byte*)sz,strlen((const char *)sz));
}

bool ps_Generate::writeBytes(UT_Byte * pBytes, size_t length)
{
	UT_ASSERT(m_fp);
	UT_ASSERT(pBytes && (length>0));

//	UT_ASSERT(length<256);				// DSC3.0 requirement
	
	doProtectFromPipe();
	bool bSuccess = (fwrite(pBytes,sizeof(UT_Byte),length,m_fp)==length);
	undoProtectFromPipe();

	return bSuccess;
}

bool ps_Generate::formatComment(const char * szCommentName)
{
	UT_String buf = "%%%%";
	buf += szCommentName;
	buf += "\n";
	return writeBytes((UT_Byte*)buf.c_str(), buf.size());
}

bool ps_Generate::formatComment(const char * szCommentName, const char * szArg1)
{
	// write out "foo: arg1\n"
	// arg1 may be a list of space separated names,
	// as in: %%BoundingBox: 1 2 3 4\n"
	// we do not PS-escape arg1.
	// return true if successful.

	UT_String buf = "%%%%";
	buf += szCommentName;
	buf += ": ";
	buf += szArg1;
	buf += "\n";
	return writeBytes((UT_Byte*)buf.c_str(), buf.size());
}

bool ps_Generate::formatComment(const char * szCommentName, const char **argv, int argc)
{
	// write out a comment line possibly with multiple arguments
	// we PS-escapify each arg as we output it.
	// return true if successful.
	
        UT_String buf = "%%%%";
	int bufLen;
	
        buf += szCommentName;
	for (int k=0; k<argc; k++)
	{
		bufLen = buf.size();
		if (bufLen+strlen(argv[k]) < 256)
		{
			// TODO see if we need to PS-style esacpe the string before we add it.
			buf += " ";
			buf += argv[k];
		}
		else
		{
			buf += "\n";
			if (!writeBytes((UT_Byte*)buf.c_str(), buf.size()))
				return false;
			buf = "%%%%+";
		}
	}
	bufLen = buf.size();
	if (bufLen > 3)						// 3==strlen("%%+")
	{
		buf += "\n";
		if (!writeBytes((UT_Byte*)buf.c_str(), buf.size()))
			return false;
	}
	return true;
}

bool ps_Generate::formatComment(const char * szCommentName, const UT_Vector * pVec)
{
	// write out a comment line possibly with multiple arguments
	// we PS-escapify each arg as we output it.
	// return true if successful.
	
	UT_String buf = "%%%%";
	buf += szCommentName;

	int bufLen;
	UT_uint32 argc = pVec->getItemCount();
	
	for (UT_uint32 k=0; k<argc; k++)
	{
		const char * psz = (const char *)pVec->getNthItem(k);
		bufLen = buf.size();
		if (bufLen+strlen(psz) < 256)
		{
			// TODO see if we need to PS-style esacpe the string before we add it.
			buf += " ";
			buf += psz;
		}
		else
		{
			buf += "\n";
			if (!writeBytes((UT_Byte*)buf.c_str(), buf.size()))
				return false;
			buf = "%%%%+";
		}
	}
	bufLen = buf.size();
	if (bufLen > 3)						// 3==strlen("%%+")
	{
		buf += "\n";
		if (!writeBytes((UT_Byte*)buf.c_str(), buf.size()))
			return false;
	}
	return true;
}

void ps_Generate::doProtectFromPipe(void)
{
	UT_ASSERT(m_pfOldSIGPIPEHandler == NULL);

	/*  We want to ignore SIGPIPE signals and have fwrite return failure
		with errno set to EPIPE rather than crashing when we get 
		a SIGPIPE.  

	    (We would get a SIGPIPE when the child process created with popen
		has already exited and we are still trying to write to it.  It
		is an error, but returning failure here is more useful than trying
		to deal with it in a SIGPIPE handler).  */

	m_pfOldSIGPIPEHandler = signal(SIGPIPE, pipeSignalHandler);
}

void ps_Generate::undoProtectFromPipe(void)
{
	signal(SIGPIPE, m_pfOldSIGPIPEHandler);
	m_pfOldSIGPIPEHandler = NULL;
}
			
/*  We need a signal handler to ignore SIGPIPE's.  */
void ps_Generate::pipeSignalHandler(int signum)
{
	/*  Each time we get the signal, we need to reset the signal
		handler for Linux.  Otherwise it will return to the default
		behavior -- exiting the process.  */
    signal(signum, pipeSignalHandler);
}
