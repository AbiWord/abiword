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

#ifndef XAP_DIALOG_FILEOPENSAVEAS_H
#define XAP_DIALOG_FILEOPENSAVEAS_H

#include "xap_Dialog.h"
class XAP_App;

#include <boost/bind.hpp>
#include <boost/function.hpp>


// we return some special values for file types depending
// on how the derived classes do different things for different
// platforms.
#define XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO	-1

/*****************************************************************
** This is the XAP and XP base-class for the file-open and
** file-save-as dialogs.
*****************************************************************/

class ABI_EXPORT XAP_Dialog_FileOpenSaveAs : public XAP_Dialog_AppPersistent
{
public:
	XAP_Dialog_FileOpenSaveAs(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog_FileOpenSaveAs(void);

	virtual void						useStart(void);
	virtual void						runModal(XAP_Frame * pFrame) = 0;
	virtual void						useEnd(void);

	typedef enum { a_VOID, a_OK, a_CANCEL }	tAnswer;

	void							setCurrentPathname(const std::string & pathname);
	void								setSuggestFilename(bool);
	XAP_Dialog_FileOpenSaveAs::tAnswer	getAnswer(void) const;
	const std::string &					getPathname(void) const;

	// the caller supplies three lists of equal length of descriptions for menu
	// labels, suffixes if the platform dialog uses them for filters, and
	// enumerated (any encoding, really) values for each type.
	void								setFileTypeList(const char ** szDescriptions,
														const char ** szSuffixes,
														const UT_sint32 * nTypeList);
	void								setDefaultFileType(UT_sint32 nType);

	// this dialog reserves the negative number space to return an
	// "automatically detected" type, so the caller should NOT supply one
	// in the list.  This is done because each platform has a different notion
	// of auto-detect (Windows is strictly by extension, Unix can be anything,
	// etc.) and XP, AP-level code shouldn't have to know
	// what type of suffix (ala "*.*" or "*") is appropriate.
	UT_sint32							getFileType(void) const;

    typedef boost::function<std::string (std::string,UT_sint32)> m_appendDefaultSuffixFunctor_t;
    void setAppendDefaultSuffixFunctor( m_appendDefaultSuffixFunctor_t f );

  protected:
	std::string					m_persistPathname;
	std::string					m_initialPathname;
	std::string					m_finalPathname;

	const char **						m_szDescriptions;
	const char ** 						m_szSuffixes;
	const UT_sint32 *					m_nTypeList;

	// derived classes set this for query
	UT_sint32							m_nFileType;
	UT_sint32							m_nDefaultFileType;

	bool								m_bSuggestName;
	XAP_Dialog_FileOpenSaveAs::tAnswer	m_answer;

    m_appendDefaultSuffixFunctor_t m_appendDefaultSuffixFunctor;
};

ABI_EXPORT XAP_Dialog_FileOpenSaveAs::m_appendDefaultSuffixFunctor_t
getAppendDefaultSuffixFunctorUsing_IE_Exp_preferredSuffixForFileType();

#endif /* XAP_DIALOG_FILEOPENSAVEAS_H */
