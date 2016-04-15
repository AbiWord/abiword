/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
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
	  m_szDescriptions(NULL),
	  m_szSuffixes(NULL),
	  m_nTypeList(NULL),
	  m_nFileType(XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO),
	  m_nDefaultFileType(XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO),
	  m_bSuggestName(false),
	  m_answer(a_VOID),
      m_appendDefaultSuffixFunctor( getAppendDefaultSuffixFunctorUsing_IE_Exp_preferredSuffixForFileType() )

{
	const gchar * savedir = nullptr;
	if (getApp()->getPrefsValue(XAP_PREF_KEY_DefaultSaveDirectory, &savedir) && strlen(savedir)) {
		m_persistPathname = savedir;
	}
}

XAP_Dialog_FileOpenSaveAs::~XAP_Dialog_FileOpenSaveAs(void)
{
	UT_ASSERT(!m_bInUse);
}

void XAP_Dialog_FileOpenSaveAs::useStart(void)
{
	XAP_Dialog_AppPersistent::useStart();

	m_initialPathname.clear();
	m_finalPathname.clear();
	m_answer = a_VOID;
	m_bSuggestName = false;
}

void XAP_Dialog_FileOpenSaveAs::useEnd(void)
{
	XAP_Dialog_AppPersistent::useEnd();

	m_initialPathname.clear();
	if (m_answer == a_OK)
	{
		m_persistPathname = std::move(m_finalPathname);
		m_finalPathname.clear();
	}
}

void XAP_Dialog_FileOpenSaveAs::setCurrentPathname(const std::string & pathname)
{
	// this lets us know the pathname of the document
	// in the frame from which we were invoked.

	m_initialPathname = pathname;
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

const std::string & XAP_Dialog_FileOpenSaveAs::getPathname(void) const
{
	// give our caller a temporary string (valid until the next
	// use of this dialog) containing the pathname the user
	// chose.

	return m_finalPathname;
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

void
XAP_Dialog_FileOpenSaveAs::setAppendDefaultSuffixFunctor( m_appendDefaultSuffixFunctor_t f )
{
    m_appendDefaultSuffixFunctor = f;
}

#include "../../../wp/impexp/xp/ie_types.h"
#include "../../../wp/impexp/xp/ie_exp.h"
#include <sstream>

std::string
AppendDefaultSuffixFunctorUsing_IE_Exp_preferredSuffixForFileType( std::string dialogFilename, UT_sint32 n )
{
    UT_UTF8String suffix( IE_Exp::preferredSuffixForFileType( n ));
    std::stringstream ss;
    ss << dialogFilename << suffix.utf8_str();
    return ss.str();
}

XAP_Dialog_FileOpenSaveAs::m_appendDefaultSuffixFunctor_t
getAppendDefaultSuffixFunctorUsing_IE_Exp_preferredSuffixForFileType()
{
    return AppendDefaultSuffixFunctorUsing_IE_Exp_preferredSuffixForFileType;
}
