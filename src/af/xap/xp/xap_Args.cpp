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

#include <stdlib.h>
#include <string.h>
#include "xap_Args.h"
#include "ut_string.h"

/*****************************************************************/

XAP_Args::XAP_Args(int argc, char ** argv)
{
	m_argc = argc;
	m_argv = argv;
	m_szBuf = NULL;
}

XAP_Args::XAP_Args(const char * szCmdLine)
{
	// build an argc,argv for this command line

	m_argc = 0;
	m_argv = NULL;
	m_szBuf = NULL;
	
	if (!szCmdLine || !*szCmdLine)
		return;

	// copy command line into work buffer
	// and count the tokens
	
	UT_cloneString(m_szBuf,szCmdLine);
	int k;
	char * p;
	for (k=0, p=strtok(m_szBuf," "); (p); k++, p=strtok(NULL," "))
		;

	// if no tokens, do nothing.
	
	if (k==0)
	{
		FREEP(m_szBuf);
		return;
	}

	// build an array of tokens.  we just let them
	// point back into our work buffer.
	
	m_argc = k;
	m_argv = (char **)calloc(m_argc,sizeof(char *));
	strcpy(m_szBuf,szCmdLine);
	for (k=0, p=strtok(m_szBuf," "); (p); k++, p=strtok(NULL," "))
		m_argv[k] = p;
}

XAP_Args::~XAP_Args(void)
{
	if (m_szBuf)
	{
		FREEP(m_szBuf);
		FREEP(m_argv);
	}
}
