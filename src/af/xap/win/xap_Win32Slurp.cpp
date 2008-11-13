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

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "xap_Frame.h"
#include "xap_Win32App.h"
#include "xap_Win32Slurp.h"
#include "xap_Prefs.h"

//////////////////////////////////////////////////////////////////
// "Slurp" (aka "Leech") refers to the ability of an application
// to receive DDE messages to open a document.  For example, the
// user can double-click on a foo.abw document in Windows Explorer
// and (if we're properly registered with the system) it will hand
// the document to us (either to an existing process or launch an
// instance and then hand it to us).
//
// The less-than-polite name for this file reflects that amount of
// pain endured in writing the code -- and the amount of time
// required to find any documentation on the subject (search MSDN
// for: "SAMPLE: How to Use File Associations", Article ID Q1222787,
// August 5, 1996.
//
// It appears that the above cited document is not quite correct
// (or rather is not aging well)...  The examples show setting
// on "HKEY_CLASSES_ROOT\<foo>" to "File_assoc", but this string
// is the value used by WindowsExplorer and the FileOpen dialog
// to describe the document type.
//////////////////////////////////////////////////////////////////

#define MY_DDE_TOPICNAME				"System"

// DDE does not provide us with an instance handle in the callback
// to get back to our class member data.  Therefore (and since we
// are only creating one of these classes), we make a static reference
// to the only instance we create.

static XAP_Win32Slurp * s_Slurp = NULL;

//////////////////////////////////////////////////////////////////

XAP_Win32Slurp::XAP_Win32Slurp(XAP_Win32App * pApp)
{
	UT_ASSERT(!s_Slurp);
	
	m_pApp = pApp;
	m_bInitialized = false;
	s_Slurp = this;
	m_hszServerName = NULL;
	m_hszTopic = NULL;
}

XAP_Win32Slurp::~XAP_Win32Slurp(void)
{
	UT_ASSERT_HARMLESS(s_Slurp);
	s_Slurp = NULL;
}

static HDDEDATA CALLBACK s_DdeServerCallback(UINT uType, UINT uFmt, HCONV hConv, 
											 HSZ hsz1, HSZ hsz2, HDDEDATA hData, 
											 DWORD dwData1, DWORD dwData2)
{
	return s_Slurp->doCallback(uType,uFmt,hConv,hsz1,hsz2,hData,dwData1,dwData2);
}

HDDEDATA CALLBACK XAP_Win32Slurp::doCallback(UINT uType, UINT /*uFmt*/, HCONV /*hConv*/,
											 HSZ hsz1, HSZ hsz2, HDDEDATA hData,
											 DWORD /*dwData1*/, DWORD /*dwData2*/)
{
#ifdef DEBUG
	UINT xtypf	= (uType & 0x000f);
	UINT xtyp	= (uType & XTYP_MASK);
	UINT xclass	= (uType & XCLASS_MASK);

	UT_DEBUGMSG(("DDEML callback received [0x%x 0x%x 0x%x]\n",xclass,xtyp,xtypf));
#endif

	switch (uType)
	{
	case XTYP_DISCONNECT:
		return 0;

	case XTYP_CONNECT:
		{
			// grant connection request iff it matches my [ServerName,Topic]
			bool bAccept = ( (hsz2==m_hszServerName) && (hsz1==m_hszTopic) );
			UT_DEBUGMSG(("DDEML connect [accepted %d]\n",bAccept));
			return (HDDEDATA)bAccept;
		}

	case XTYP_EXECUTE:
		processCommand(hData);
		return (HDDEDATA)DDE_FACK;			// we handled the command

	case XTYP_ERROR:
		UT_DEBUGMSG(("DDEML error in callback\n"));
		return 0;

	default:
		return 0;
	}
}

bool XAP_Win32Slurp::connectSlurper(void)
{
	if (m_bInitialized)					// only do this once !!
		return false;
	
	// Register us with the DDE Management Library (DDEML).

	m_idDdeServerInst = 0;
	UINT nDdeRtnStatus = DdeInitialize(&m_idDdeServerInst,
									   (PFNCALLBACK)&s_DdeServerCallback,
									   (  APPCMD_FILTERINITS
										| CBF_SKIP_CONNECT_CONFIRMS
										| CBF_FAIL_SELFCONNECTIONS
										| CBF_FAIL_POKES),
									   0);
	if (nDdeRtnStatus != DMLERR_NO_ERROR)
	{
		UT_DEBUGMSG(("Unable to initialize DDEML [%d]\n",nDdeRtnStatus));
		return false;
	}

	// create two registered strings.
	// the "ServerName" must match the value in the registry under the key:
	// HKEY_CLASSES_ROOT\<xxx>\shell\open\ddeexec\application
	//
	// the "TopicName" must match the value in the key:
	// HKEY_CLASSES_ROOT\<xxx>\shell\open\ddeexec\topic
	
	m_hszServerName = DdeCreateStringHandle(m_idDdeServerInst, const_cast<char *>(m_pApp->getApplicationName()), CP_WINANSI);
	m_hszTopic = DdeCreateStringHandle(m_idDdeServerInst, MY_DDE_TOPICNAME, CP_WINANSI);

	// register the server Name
	HDDEDATA bRegistered = DdeNameService(m_idDdeServerInst, m_hszServerName, NULL, DNS_REGISTER);
	if (!bRegistered)
	{
		UT_DEBUGMSG(("Unable to register NameService with DDEML.\n"));
		return false;
	}

	m_bInitialized = true;
	return true;
}

bool XAP_Win32Slurp::disconnectSlurper(void)
{
	if (m_hszServerName)
		DdeFreeStringHandle(m_idDdeServerInst,m_hszServerName);
	if (m_hszTopic)
		DdeFreeStringHandle(m_idDdeServerInst,m_hszTopic);
	
	DdeUninitialize(m_idDdeServerInst);
	m_idDdeServerInst = 0;

	m_bInitialized = false;
	return true;
}

void XAP_Win32Slurp::processCommand(HDDEDATA hData)
{
	DWORD bufSize = DdeGetData(hData,NULL,0,0);

	char * pBuf = (char *)UT_calloc(sizeof(char),bufSize+100);
	if (!pBuf)
	{
		UT_DEBUGMSG(("No memory to allocate DDE buffer [size %d]\n",bufSize));
		return;
	}
	
	DdeGetData(hData,(LPBYTE)pBuf,bufSize+99,0);
	UT_DEBUGMSG(("DDEML received command '%s'\n",pBuf));
	
	// we expect something of the form:
	//     [Open("<pathname>")]
	// if anything more complicated is needed, it may be a
	// good idea to use a regex library
	// TODO failures just goto Finished. Some error reporting
	// TODO would be nice

	// pointer to work through the incoming string
	char * next = pBuf;
	
	// pointer used to copy into command and pathname
	char * dest = 0;
	
	// chomp the [
	if ( *next++ != '[' ) goto Finished;
	
	// find the next sequence of non ( characters
	// this will be the dde command
	char command[1024];
	dest = command;
	for ( ; *next != '('; ++next )
	{
		*dest++ = *next;
	}
	*dest = 0;

	// chomp the ( and the "
	if ( *next++ != '(' ) goto Finished;
	if ( *next++ != '"' ) goto Finished;
	// go until the next " to get the parameter
	// " are not allowed in filenames, so we should be safe here
	char pathname[4096];
	dest = pathname;
	for ( ; *next != '"'; ++next )	
	{
		*dest++ = *next;
	}
	*dest = 0;
	
	// chomp the ", ), and ]
	if ( *next++ != '"' ) goto Finished;
	if ( *next++ != ')' ) goto Finished;
	if ( *next++ != ']' ) goto Finished;
	
	// now do something useful with the command and its parameter
	if (g_ascii_strcasecmp(command,"open") == 0)
		{
			if (!pathname || !*pathname)
			{
				UT_DEBUGMSG(("No pathname given in DDE Open command.\n"));
				goto Finished;
			}

			// ask the application to load this document into a window....

			XAP_Win32App *p_app = (XAP_Win32App *)XAP_App::getApp();
			UT_Error error = p_app->fileOpen(p_app->getLastFocussedFrame(), pathname);
			
			if(error != UT_OK)
			{
				UT_DEBUGMSG(("Could not load document given in DDE Open command [%s].\n",pathname));
			}

			goto Finished;
		}

Finished:
	FREEP(pBuf);
}

///////////////////////////////////////////////////////////////////
// Stuff to deal with loading the registry with our file
// associations and DDE commands.
///////////////////////////////////////////////////////////////////

typedef enum { X_Error, X_CreatedKey, X_ExistingKey } XX_Status;

static XX_Status _fetchKey(HKEY k1, const char * szSubKey, HKEY * pkNew)
{
	DWORD dwDisposition;
	LONG eResult = RegCreateKeyEx(k1,szSubKey,0,"",
								  REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL, 
								  pkNew, &dwDisposition);
	if (eResult != ERROR_SUCCESS)
	{
		UT_DEBUGMSG(("Could not open key [%s]\n",szSubKey));
		return X_Error;
	}

	switch (dwDisposition)
	{
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		if (*pkNew)
			RegCloseKey(*pkNew);
		*pkNew = 0;
		return X_Error;

	case REG_CREATED_NEW_KEY:
		return X_CreatedKey;

	case REG_OPENED_EXISTING_KEY:
		return X_ExistingKey;
	}
}

void XAP_Win32Slurp::stuffRegistry(const char * szSuffix,
								   const char * szApplicationName,
								   const char * szExePathname,
								   const char * szContentType)
{
	// load the system registry if there's no info
	// for us already present for the given suffix.
	// we assume that the string contains the dot.
	//
	// TODO consider raising a dialog asking if they
	// TODO want us to override any existing settings
	// TODO if they currently don't point to us.  like
	// TODO MSFT and NSCP currently (politely) fight
	// TODO for the .html suffix....
	//
	// we construct the following pattern in the registry:
	//
	// HKEY_CLASSES_ROOT\<suffix> = <foo>
	// HKEY_CLASSES_ROOT\<suffix>\Content Type = <content_type>
	// HKEY_CLASSES_ROOT\<foo> = <application_name> ## " Document"
	// HKEY_CLASSES_ROOT\<foo>\shell\open\command = <exe_pathname>
	// HKEY_CLASSES_ROOT\<foo>\shell\open\ddeexec = [Open("%1")]
	// HKEY_CLASSES_ROOT\<foo>\shell\open\ddeexec\application = <application_name>
	// HKEY_CLASSES_ROOT\<foo>\shell\open\ddeexec\topic = System
	// HKEY_CLASSES_ROOT\<foo>\DefaultIcon = <exe_pathname,2>

#define VALUE_DDEEXEC_OPEN			"[Open(\"%1\")]"
#define FORMAT_OUR_INDIRECTION		"AbiSuite.%s"
#define CONTENT_TYPE_KEY			"Content Type"
#define DOCUMENT_ICON_POSITION	2
#define xx(s)					((LPBYTE)(s)),(strlen(s)+1)
	
	char buf[1024];
	char bufOurFoo[1024];
	char bufOurFooValue[1024];
	char bufDefaultIconValue[1024];
	DWORD dType;
	LONG eResult;
	ULONG len;
	bool bUpdateContentType;
	bool bCreateOrOverwrite = false;

	HKEY hKeyFoo = 0;
	HKEY hKeySuffix = 0;
	HKEY hKeyShell = 0;
	HKEY hKeyCommand = 0;
	HKEY hKeyOpen = 0;
	HKEY hKeyDdeExec = 0;
	HKEY hKeyApplication = 0;
	HKEY hKeyTopic = 0;
	HKEY hKeyDefaultIcon = 0;
	
	sprintf(bufOurFoo,"AbiSuite.%s",szApplicationName);
	strtok(bufOurFoo," ");				// trim key at first whitespace

	sprintf(bufOurFooValue,"%s Document",szApplicationName);
	
	///////////////////////////////////////////////////////////////////
	// See if someone has claimed this suffix.
	// HKEY_CLASSES_ROOT\<suffix> = <foo>
	///////////////////////////////////////////////////////////////////

	switch ( _fetchKey(HKEY_CLASSES_ROOT,szSuffix,&hKeySuffix) )
	{
	case X_Error:
		goto CleanupMess;

	case X_CreatedKey:					// we are g_free to create what we want.
		bCreateOrOverwrite = true;
		break;
		
	case X_ExistingKey:					// see what's already there 
		{
			///////////////////////////////////////////////////////////////////
			// HKEY_CLASSES_ROOT\<suffix> = <foo>
			// was already present.  Verify the value.
			///////////////////////////////////////////////////////////////////

			len = G_N_ELEMENTS(buf);
			eResult = RegQueryValueEx(hKeySuffix,NULL,NULL,&dType,(LPBYTE)buf,&len);

			if ((eResult != ERROR_SUCCESS) || (dType != REG_SZ) || (len==0))
				break;					// bogus data, overwrite it.
			
			UT_DEBUGMSG(("Registry: suffix [HKEY_CLASSES_ROOT\\%s] --> [%s]\n",
						 szSuffix,buf));

			if (g_ascii_strcasecmp(buf,bufOurFoo) != 0)	// we didn't create this so ask first.
			{
				if (!_askForStealFromAnotherApplication())
					goto CleanupMess;

				bCreateOrOverwrite = true;
				break;
			}
		}
	}

	if (bCreateOrOverwrite)
	{
		///////////////////////////////////////////////////////////////////
		// Set the value <foo> in 
		// HKEY_CLASSES_ROOT\<suffix> = <foo>
		///////////////////////////////////////////////////////////////////

		UT_ASSERT(hKeySuffix);
		eResult = RegSetValueEx(hKeySuffix,NULL,0,REG_SZ,xx(bufOurFoo));

		UT_DEBUGMSG(("Register: HKEY_CLASSES_ROOT\\%s <-- %s [error %d]\n",
					 szSuffix,bufOurFoo,eResult));

		if (eResult != ERROR_SUCCESS)
			goto CleanupMess;
	}

	///////////////////////////////////////////////////////////////////
	// See if "HKEY_CLASSES_ROOT\<suffix>\Content Type" is present
	// as a value under the current key.
	///////////////////////////////////////////////////////////////////

	bUpdateContentType = true;
	len = G_N_ELEMENTS(buf);
	eResult = RegQueryValueEx(hKeySuffix,CONTENT_TYPE_KEY,NULL,&dType,(LPBYTE)buf,&len);
	if ((eResult == ERROR_SUCCESS) && (dType == REG_SZ))
	{
		UT_DEBUGMSG(("Registry: Existing ContentType [%s]\n",buf));
		if (g_ascii_strcasecmp(buf,szContentType) == 0)
			bUpdateContentType = false;
		else							// we didn't create this so ask first.
			bUpdateContentType = (_askForStealMimeFromAnotherApplication());
	}

	if (bUpdateContentType)				// bogus or stale data, overwrite it.
	{
		///////////////////////////////////////////////////////////////////
		// Set the value <content_type> in 
		// HKEY_CLASSES_ROOT\<suffix>\Content Type = <content_type>
		///////////////////////////////////////////////////////////////////

		UT_ASSERT(hKeySuffix);
		eResult = RegSetValueEx(hKeySuffix,CONTENT_TYPE_KEY,0,REG_SZ,xx(szContentType));

		UT_DEBUGMSG(("Register: HKEY_CLASSES_ROOT\\%s\\Content Type <-- %s [error %d]\n",
					 szSuffix,szContentType,eResult));

		// content-type is not a critical field, so if it fails we just go on.
		// if (eResult != ERROR_SUCCESS) goto CleanupMess;
	}
	
	///////////////////////////////////////////////////////////////////
	// Verify that the suffix indirection is defined.
	// HKEY_CLASSES_ROOT\<foo> = ...
	///////////////////////////////////////////////////////////////////

	switch ( _fetchKey(HKEY_CLASSES_ROOT,bufOurFoo,&hKeyFoo) )
	{
	case X_Error:
		goto CleanupMess;

	case X_ExistingKey:
		UT_ASSERT(hKeyFoo);
		len = G_N_ELEMENTS(buf);
		eResult = RegQueryValueEx(hKeyFoo,NULL,0,&dType,(LPBYTE)buf,&len);
		if ((eResult==ERROR_SUCCESS) && (dType==REG_SZ) && (g_ascii_strcasecmp(buf,bufOurFooValue)==0))
			break;					// already has correct value, no need to overwrite.

		/* otherwise, replace the value */
		/* fall thru intended */

	case X_CreatedKey:
		UT_ASSERT(hKeyFoo);
		eResult = RegSetValueEx(hKeyFoo,NULL,0,REG_SZ,xx(bufOurFooValue));
		if (eResult != ERROR_SUCCESS)
			goto CleanupMess;
		break;
	}
		
	///////////////////////////////////////////////////////////////////
	// Inspect the command path
	// HKEY_CLASSES_ROOT\<foo>\shell\open\command = <exe_pathname>
	///////////////////////////////////////////////////////////////////
	char commandPathWithParam[1024];
	strcpy ( commandPathWithParam, szExePathname );
	strcat ( commandPathWithParam, " \"%1\"" );

	if (_fetchKey(hKeyFoo,"shell",&hKeyShell) == X_Error)
		goto CleanupMess;
	if (_fetchKey(hKeyShell,"open",&hKeyOpen) == X_Error)
		goto CleanupMess;
	switch ( _fetchKey(hKeyOpen,"command",&hKeyCommand) )
	{
	case X_Error:
		goto CleanupMess;

	case X_ExistingKey:	
		UT_ASSERT(hKeyCommand);
		len = G_N_ELEMENTS(buf);
		eResult = RegQueryValueEx(hKeyCommand,NULL,0,&dType,(LPBYTE)buf,&len);
		if ((eResult==ERROR_SUCCESS) && (dType==REG_SZ))
		{
			if (g_ascii_strcasecmp(buf,commandPathWithParam) == 0)
				break;					// already has correct value, no need to overwrite.
			
			if(memcmp(buf, commandPathWithParam, strlen(commandPathWithParam)) == 0)
			{
				// Path name is the same but has extra at the end.
				// Probably "%1"

				// Fall throught to update path name.
			}
			else
			{
				
				if (!_askForUpdateExePathname())
					goto CleanupMess;
			}
		}

		/* otherwise, replace the value */
		/* fall thru intended */
		
	case X_CreatedKey:
		UT_ASSERT(hKeyCommand);
		if (eResult != ERROR_SUCCESS)
			eResult = RegSetValueEx(hKeyCommand,NULL,0,REG_SZ,xx(commandPathWithParam));
		goto CleanupMess;
		break;
	}

	///////////////////////////////////////////////////////////////////
	// Inspect the ddeexec key
	// HKEY_CLASSES_ROOT\<foo>\shell\open\ddeexec = [Open(%1)]
	///////////////////////////////////////////////////////////////////

	switch ( _fetchKey(hKeyOpen,"ddeexec",&hKeyDdeExec) )
	{
	case X_Error:
		goto CleanupMess;

	case X_ExistingKey:	
		UT_ASSERT(hKeyDdeExec);
		len = G_N_ELEMENTS(buf);
		eResult = RegQueryValueEx(hKeyDdeExec,NULL,0,&dType,(LPBYTE)buf,&len);
		if ((eResult==ERROR_SUCCESS) && (dType==REG_SZ) && (g_ascii_strcasecmp(buf,VALUE_DDEEXEC_OPEN)==0))
			break;						// already has correct value, no need to overwrite.

		/* otherwise, replace the value */
		/* fall thru intended */
		
	case X_CreatedKey:
		UT_ASSERT(hKeyDdeExec);
		eResult = RegSetValueEx(hKeyDdeExec,NULL,0,REG_SZ,xx(VALUE_DDEEXEC_OPEN));
		if (eResult != ERROR_SUCCESS)
			goto CleanupMess;
		break;
	}

	///////////////////////////////////////////////////////////////////
	// Inspect the application key
	// HKEY_CLASSES_ROOT\<foo>\shell\open\ddeexec\application = <application_name>
	///////////////////////////////////////////////////////////////////

	switch ( _fetchKey(hKeyDdeExec,"application",&hKeyApplication) )
	{
	case X_Error:
		goto CleanupMess;

	case X_ExistingKey:	
		UT_ASSERT(hKeyApplication);
		len = G_N_ELEMENTS(buf);
		eResult = RegQueryValueEx(hKeyApplication,NULL,0,&dType,(LPBYTE)buf,&len);
		if ((eResult==ERROR_SUCCESS) && (dType==REG_SZ) && (g_ascii_strcasecmp(buf,szApplicationName)==0))
			break;						// already has correct value, no need to overwrite.

		/* otherwise, replace the value */
		/* fall thru intended */
		
	case X_CreatedKey:
		UT_ASSERT(hKeyApplication);
		eResult = RegSetValueEx(hKeyApplication,NULL,0,REG_SZ,xx(szApplicationName));
		if (eResult != ERROR_SUCCESS)
			goto CleanupMess;
		break;
	}

	///////////////////////////////////////////////////////////////////
	// Inspect the topic key
	// HKEY_CLASSES_ROOT\<foo>\shell\open\ddeexec\topic = System
	///////////////////////////////////////////////////////////////////

	switch ( _fetchKey(hKeyDdeExec,"topic",&hKeyTopic) )
	{
	case X_Error:
		goto CleanupMess;

	case X_ExistingKey:	
		UT_ASSERT(hKeyTopic);
		len = G_N_ELEMENTS(buf);
		eResult = RegQueryValueEx(hKeyTopic,NULL,0,&dType,(LPBYTE)buf,&len);
		if ((eResult==ERROR_SUCCESS) && (dType==REG_SZ) && (g_ascii_strcasecmp(buf,MY_DDE_TOPICNAME)==0))
			break;						// already has correct value, no need to overwrite.

		/* otherwise, replace the value */
		/* fall thru intended */
		
	case X_CreatedKey:
		UT_ASSERT(hKeyTopic);
		eResult = RegSetValueEx(hKeyTopic,NULL,0,REG_SZ,xx(MY_DDE_TOPICNAME));
		if (eResult != ERROR_SUCCESS)
			goto CleanupMess;
		break;
	}

	///////////////////////////////////////////////////////////////////
	// Set the default icon for the suffix (this is for Explorer et al.
	// HKEY_CLASSES_ROOT\<foo>\DefaultIcon = <exe_pathname,1>
	///////////////////////////////////////////////////////////////////

	sprintf(bufDefaultIconValue,"%s,%d",szExePathname,DOCUMENT_ICON_POSITION);
	switch ( _fetchKey(hKeyFoo,"DefaultIcon",&hKeyDefaultIcon) )
	{
	case X_Error:
		goto CleanupMess;

	case X_ExistingKey:	
		UT_ASSERT(hKeyDefaultIcon);
		len = G_N_ELEMENTS(buf);
		eResult = RegQueryValueEx(hKeyDefaultIcon,NULL,0,&dType,(LPBYTE)buf,&len);
		if ((eResult==ERROR_SUCCESS) && (dType==REG_SZ) && (g_ascii_strcasecmp(buf,bufDefaultIconValue)==0))
			break;						// already has correct value, no need to overwrite.

		/* otherwise, replace the value */
		/* fall thru intended */
		
	case X_CreatedKey:
		UT_ASSERT(hKeyDefaultIcon);
		eResult = RegSetValueEx(hKeyDefaultIcon,NULL,0,REG_SZ,xx(bufDefaultIconValue));
		if (eResult != ERROR_SUCCESS)
			goto CleanupMess;
		break;
	}
		
	///////////////////////////////////////////////////////////////////
	// Success.
	///////////////////////////////////////////////////////////////////

	UT_DEBUGMSG(("Successfully stuffed the registry for suffix [%s].\n",szSuffix));
	
CleanupMess:
		
#define KILLKEY(k)		do { if (k) RegCloseKey(k); (k) = 0; } while (0)

	KILLKEY(hKeyFoo);
	KILLKEY(hKeySuffix);
	KILLKEY(hKeyShell);
	KILLKEY(hKeyCommand);
	KILLKEY(hKeyOpen);
	KILLKEY(hKeyDdeExec);
	KILLKEY(hKeyApplication);
	KILLKEY(hKeyTopic);
	KILLKEY(hKeyDefaultIcon);

	return;
}

bool XAP_Win32Slurp::_askForStealFromAnotherApplication(void) const
{
	// TODO install a real dialog that asks the user and
	// TODO allows us to set preference value for never
	// TODO asking again or always steal or whatever...

	UT_DEBUGMSG(("Registry: Suffix not ours.\n"));
	bool bResult = false;

#ifdef DEBUG	
	bResult = true;
#endif
	
	return bResult;
}

bool XAP_Win32Slurp::_askForStealMimeFromAnotherApplication(void) const
{
	// TODO install a real dialog that asks the user and
	// TODO allows us to set preference value for never
	// TODO asking again or always steal or whatever...
	
	UT_DEBUGMSG(("Registry: MIME type not ours.\n"));
	bool bResult = false;

#ifdef DEBUG	
	bResult = true;
#endif

	return bResult;
}

bool XAP_Win32Slurp::_askForUpdateExePathname(void) const
{
	// TODO install a real dialog that asks the user if
	// TODO we want to change the pathname of the exe in
	// TODO the association and to set preference values
	// TODO to never asking again or always set it or
	// TODO whatever...

	UT_DEBUGMSG(("Registry: Need to update EXE pathname...\n"));
	bool bResult = false;

	return bResult;
}

