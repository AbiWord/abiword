/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
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


#ifndef IE_EXP_HTML_H
#define IE_EXP_HTML_H

#include "xap_Dlg_HTMLOptions.h"

#include "ie_exp.h"

/* NOTE: I'm trying to keep the code similar across versions,
 *       and therefore features are enabled/disabled here:
 */

/* Define if the base unicode char is UCS-4
 */
/* #undef HTML_UCS4 */

/* Define if the sniffers need to pass export name to parent
 */
/* #undef HTML_NAMED_CONSTRUCTORS */

/* Define if the [P/X]HTML export options dialog is implemented
 */
/* #undef HTML_DIALOG_OPTIONS */

/* Define if the tables are supported
 */
/* #undef HTML_TABLES_SUPPORTED */

/* Define if meta information is supported
 */
/* #undef HTML_META_SUPPORTED */

/* TODO: Rather than having separate sniffers to differentiate
 *       the exporter's behaviour, should have a dialog box
 *       with options.
 */
#ifndef HTML_DIALOG_OPTIONS
#define HTML_ENABLE_HTML4 
/* #undef HTML_ENABLE_PHTML */
#endif
#ifdef DEBUG
#define HTML_ENABLE_MHTML
#endif

class PD_Document;

// The exporter/writer for HTML

class ABI_EXPORT IE_Exp_HTML_Sniffer : public IE_ExpSniffer
{
	friend class IE_Exp;

public:
	IE_Exp_HTML_Sniffer ();
	virtual ~IE_Exp_HTML_Sniffer () {}

	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructExporter (PD_Document * pDocument,
										IE_Exp ** ppie);
};

#ifdef HTML_ENABLE_HTML4

class ABI_EXPORT IE_Exp_HTML4_Sniffer : public IE_ExpSniffer
{
	friend class IE_Exp;

public:
	IE_Exp_HTML4_Sniffer ();
	virtual ~IE_Exp_HTML4_Sniffer () {}

	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructExporter (PD_Document * pDocument,
										IE_Exp ** ppie);
};

#endif /* HTML_ENABLE_HTML4 */

#ifdef HTML_ENABLE_PHTML

class ABI_EXPORT IE_Exp_PHTML_Sniffer : public IE_ExpSniffer
{
	friend class IE_Exp;

public:
	IE_Exp_PHTML_Sniffer ();
	virtual ~IE_Exp_PHTML_Sniffer () {}

	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructExporter (PD_Document * pDocument,
										IE_Exp ** ppie);
};

#endif /* HTML_ENABLE_PHTML */

#ifdef HTML_ENABLE_MHTML

class ABI_EXPORT IE_Exp_MHTML_Sniffer : public IE_ExpSniffer
{
	friend class IE_Exp;

public:
	IE_Exp_MHTML_Sniffer ();
	virtual ~IE_Exp_MHTML_Sniffer () {}

	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructExporter (PD_Document * pDocument,
										IE_Exp ** ppie);
};

#endif /* HTML_ENABLE_MHTML */

class ABI_EXPORT IE_Exp_HTML : public IE_Exp
{
public:
	IE_Exp_HTML (PD_Document * pDocument);
	virtual ~IE_Exp_HTML ();

	static bool			RecognizeSuffix (const char * szSuffix);
	static UT_Error		StaticConstructor (PD_Document * pDocument,
										   IE_Exp ** ppie);
	static bool			GetDlgLabels (const char ** pszDesc,
									  const char ** pszSuffixList,
									  IEFileType * ft);
	static bool 		SupportsFileType (IEFileType ft);

	inline void			suppressDialog (bool disable = true) { m_bSuppressDialog = disable; }

	inline void			set_HTML4 (bool enable = true) { m_exp_opt.bIs4 = enable; }
	inline void			set_PHTML (bool enable = true) { m_exp_opt.bIsAbiWebDoc = enable; }
	inline void			set_MHTML (bool enable = true) { m_exp_opt.bMultipart = enable; }

protected:
	virtual bool		_openFile (const char * szFilename);
	virtual UT_Error	_writeDocument ();

private:
	bool				m_bSuppressDialog;
	XAP_Exp_HTMLOptions	m_exp_opt;
};

#endif /* IE_EXP_HTML_H */
