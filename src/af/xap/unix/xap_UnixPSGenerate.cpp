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
		m_fp = fopen(m_szFilename, "wb");
	}
	else
	{
		m_bIsFile = false;
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
	bool bSuccess = fputc(static_cast<char>(byte), m_fp) != EOF;
	undoProtectFromPipe();

	return bSuccess;
}

bool ps_Generate::writeBytes(const UT_Byte * pBytes, size_t length)
{
	UT_ASSERT(m_fp);
	UT_ASSERT(pBytes && (length>0));

	doProtectFromPipe();
	bool bSuccess = (fwrite(pBytes,sizeof(UT_Byte),length,m_fp)==length);
	undoProtectFromPipe();

	return bSuccess;
}

bool ps_Generate::formatComment(const char * szCommentName)
{
	UT_String buf = "%%";
	buf += szCommentName;
	buf += "\n";
	return writeBytes(buf.c_str(), buf.size());
}

bool ps_Generate::formatComment(const char * szCommentName, const char * szArg1)
{
	// write out "foo: arg1\n"
	// arg1 may be a list of space separated names,
	// as in: %%BoundingBox: 1 2 3 4\n"
	// we do not PS-escape arg1.
	// return true if successful.

	UT_String buf = "%%";
	buf += szCommentName;
	buf += ": ";
	buf += szArg1;
	buf += "\n";
	return writeBytes(buf.c_str(), buf.size());
}

bool ps_Generate::formatComment(const char * szCommentName, const char **argv, int argc)
{
	// write out a comment line possibly with multiple arguments
	// we PS-escapify each arg as we output it.
	// return true if successful.
	
	UT_String buf = "%%";
	int bufLen;
	
	buf += szCommentName;
	buf += ":";
	for (int k = 0; k < argc; k++)
	{
		UT_String arg(argv[k]);

		bufLen = buf.size();
		if (bufLen + arg.size() < 256)
		{
			// TODO see if we need to PS-style esacpe the string before we add it.
			buf += " ";
			buf += arg;
		}
		else
		{
			buf += "\n";
			if (!writeBytes(buf.c_str(), buf.size()))
				return false;
			buf = "%%+";
		}
	}
	bufLen = buf.size();
	if (bufLen > 3)						// 3==strlen("%%+")
	{
		buf += "\n";
		if (!writeBytes(buf.c_str(), buf.size()))
			return false;
	}
	return true;
}

bool ps_Generate::formatComment(const char * szCommentName, const UT_Vector * pVec)
{
	// write out a comment line possibly with multiple arguments
	// we PS-escapify each arg as we output it.
	// return true if successful.
	
	UT_String buf = "%%";
	buf += szCommentName;
    buf += ":";
	int bufLen;
	UT_uint32 argc = pVec->getItemCount();
	
	for (UT_uint32 k=0; k<argc; k++)
	{
		const char * psz = reinterpret_cast<const char *>(pVec->getNthItem(k));
		UT_String arg(psz);

		bufLen = buf.size();
		if (bufLen + arg.size() < 256)
		{
			// TODO see if we need to PS-style esacpe the string before we add it.
			buf += " ";
			buf += arg;
		}
		else
		{
			buf += "\n";
			if (!writeBytes(buf.c_str(), buf.size()))
				return false;
			buf = "%%+";
		}
	}
	bufLen = buf.size();
	if (bufLen > 3)						// 3==strlen("%%+")
	{
		buf += "\n";
		if (!writeBytes(buf.c_str(), buf.size()))
			return false;
	}
	return true;
}

bool ps_Generate::formatComment(const char * szCommentName, const UT_sint32 iArg)
{
	UT_String buf = "%%";
	buf += szCommentName;
	buf += ": ";
	char temp[30];
	sprintf(temp, "%d\n", iArg);
	buf += temp;
	return writeBytes(buf.c_str(), buf.size());
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
