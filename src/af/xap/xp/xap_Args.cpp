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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "xap_Args.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"

/*****************************************************************/

XAP_Args::XAP_Args(int argc, char ** argv)
{
	m_argc = argc;
	m_argv = argv;
	m_szBuf = NULL;
}

#ifdef _WIN32

char *XX_encode(const char *str)
{
	int l=strlen(str)+1;
	const char *c;
	char *d;
	char *result;
	for (c=str;*c;c++) {
		if (*c<0 || *c=='%') l+=2;
	}
	result=(char*)g_malloc(l);
	for (c=str,d=result; *c; c++) {
		if (*c<0 || *c=='%') {
			sprintf(d,"%%%02X",(unsigned char)*c);
			d+=3;
		} else {
			*d++=*c;
		}
	}
	*d=0;
	return result;
}

#endif

XAP_Args::XAP_Args(const char * szCmdLine)
{
	// build an argc,argv for this command line

	m_argc = 0;
	m_argv = NULL;
	m_szBuf = NULL;
	
	if (!szCmdLine || !*szCmdLine)
		return;

	// copy command line into work buffer
	// and put pointers to the tokens in m_argv
	//
	// we support (with apologies to Flex & Bison):
	//
	//    WHITE [ \t]+
	//    DQUOTE '"'
	//    SQUOTE '\''
	//    OTHER [^ \t'"]
	//
	//    T1 := OTHER*
	//    T2 := DQUOTE [^DQUOTE]* DQUOTE
	//    T3 := SQUOTE [^SQUOTE]* SQUOTE
	//
	//    WHITE ({T1|T2|T3}WHITE)* [WHITE]
	
#ifdef _WIN32
	// glib on Windows assumes that command line is in ANSI codepage
	m_szBuf = XX_encode(szCmdLine);
#else
	m_szBuf = g_strdup(szCmdLine);
#endif
	UT_ASSERT(m_szBuf);

	int count = 10;	// start with 10 and g_try_realloc if necessary
	int k = 0;
	char ** argv = (char **)UT_calloc(count,sizeof(char *));

	enum _state { S_START, S_INTOKEN, S_INDQUOTE, S_INSQUOTE } state;
	state = S_START;

#define GrowArrayIfNecessary()								\
	do	{	if (k==count)									\
			{	int newsize = (count+10)*sizeof(char *);	\
				argv = (char **)g_try_realloc(argv,newsize);	\
				count += 10;								\
		}} while (0)

	char * p = m_szBuf;
	while (*p)
	{
		switch (state)
		{
		case S_START:
			if ( (*p==' ') || (*p=='\t') )
			{
				p++;
				break;
			}

			if (*p=='\'')
			{
				state=S_INSQUOTE;
				*p++=0;					// don't include starting quote in token
			}
			else if (*p=='"')
			{
				state=S_INDQUOTE;
				*p++=0;					// don't include starting quote in token
			}
			else
				state=S_INTOKEN;

			GrowArrayIfNecessary();
			argv[k++] = p++;
			break;
			
		case S_INTOKEN:
			if ( (*p==' ') || (*p=='\t') )
			{
				state=S_START;
				*p++=0;
				break;
			}
			
			p++;
			break;
			
		case S_INDQUOTE:
			if ( *p=='"' )
			{
				state=S_START;
				*p++=0;
				break;
			}

			p++;
			break;
			
		case S_INSQUOTE:
			if ( *p=='\'' )
			{
				state=S_START;
				*p++=0;
				break;
			}

			p++;
			break;
		}
	}
	
	if (k==0)
	{
		FREEP(m_szBuf);
		return;
	}

	m_argv = argv;
	m_argc = k;

#ifdef DEBUG
	for (int kk=0; kk<m_argc; kk++)
		UT_DEBUGMSG(("ParsedCommandLine: argv[%d][%s]\n",kk,m_argv[kk]));
#endif

	return;
}

XAP_Args::~XAP_Args(void)
{
	if (m_szBuf)
	{
		FREEP(m_szBuf);
		FREEP(m_argv);
	}
}
