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
#include <stdio.h>
#include <string.h>
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "xap_App.h"
#include "xap_Prefs.h"
#include "xap_Dlg_FileOpenSaveAs.h"

/*****************************************************************
** The file-open and file-save-as dialogs have
** app-persistence, but the persistence is
** independent of each other.  This code supports
** both dialogs; one instance of this object will
** be created for each.  The object, once created,
** will be recycled (reused) throughout the session.
**
** We observe the following basic policy:
** [] we are to be given the pathname of the document
**    in the frame from which we were requested.  if
**    it is 'untitled', give us a null pathname.
** [] if the document has a pathname, we will start
**    the dialog in directory(pathname).
** [] if the document does not have a pathname, we
**    will start in the directory we were in when
**    last OK'd.  (we ignore previous CANCEL's.)
** [] if the document does not have a pathname and
**    we have no previous OK'd use, we start the
**    dialog in the current directory (probably from
**    where the application was invoked from).
** [] we do not let the current working directory
**    change.

******************************************************************/

XAP_Dialog_FileOpenSaveAs::XAP_Dialog_FileOpenSaveAs(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_AppPersistent(pDlgFactory,id, "interface/dialogopenlinux"),
	  m_szPersistPathname(NULL),
	  m_szInitialPathname(NULL),
	  m_szFinalPathname(NULL),
	  m_szDescriptions(NULL),
	  m_szSuffixes(NULL),
	  m_nTypeList(NULL),
	  m_nFileType(XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO),
	  m_nDefaultFileType(XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO),
	  m_bSuggestName(false),
	  m_answer(a_VOID)
{
  const gchar * savedir = 0;
  if (getApp()->getPrefsValue(XAP_PREF_KEY_DefaultSaveDirectory, &savedir) && strlen(savedir))
    {
      m_szPersistPathname = g_strdup(savedir);
    }
}

XAP_Dialog_FileOpenSaveAs::~XAP_Dialog_FileOpenSaveAs(void)
{
	UT_ASSERT(!m_bInUse);
	FREEP(m_szPersistPathname);
	FREEP(m_szInitialPathname);
	FREEP(m_szFinalPathname);
}

void XAP_Dialog_FileOpenSaveAs::useStart(void)
{
	XAP_Dialog_AppPersistent::useStart();
	
	FREEP(m_szInitialPathname);
	FREEP(m_szFinalPathname);
	m_answer = a_VOID;
	m_bSuggestName = false;
}

void XAP_Dialog_FileOpenSaveAs::useEnd(void)
{
	XAP_Dialog_AppPersistent::useEnd();

	FREEP(m_szInitialPathname);
	if (m_answer == a_OK)
	{
		FREEP(m_szPersistPathname);
		m_szPersistPathname = m_szFinalPathname;
		m_szFinalPathname = NULL;
	}
}

void XAP_Dialog_FileOpenSaveAs::setCurrentPathname(const char * szPathname)
{
	// this lets us know the pathname of the document
	// in the frame from which we were invoked.

	FREEP(m_szInitialPathname);
	if (szPathname && *szPathname)
		m_szInitialPathname = g_strdup(szPathname);
}

void XAP_Dialog_FileOpenSaveAs::setSuggestFilename(bool bSuggestName)
{
	m_bSuggestName = bSuggestName;
}

XAP_Dialog_FileOpenSaveAs::tAnswer XAP_Dialog_FileOpenSaveAs::getAnswer(void) const
{
	// let our caller know if user hit ok or cancel.
	
	return m_answer;
}

const char * XAP_Dialog_FileOpenSaveAs::getPathname(void) const
{
	// give our caller a temporary string (valid until the next
	// use of this dialog) containing the pathname the user
	// chose.

	return m_szFinalPathname;
}

void XAP_Dialog_FileOpenSaveAs::setFileTypeList(const char ** szDescriptions,
												const char ** szSuffixes,
												const UT_sint32 * nTypeList)
{
	m_szDescriptions = szDescriptions;
	m_szSuffixes = szSuffixes;
	m_nTypeList = nTypeList;
}

void XAP_Dialog_FileOpenSaveAs::setDefaultFileType(UT_sint32 nType)
{
	m_nDefaultFileType = nType;
}


UT_sint32 XAP_Dialog_FileOpenSaveAs::getFileType(void) const
{
	return m_nFileType;
}
