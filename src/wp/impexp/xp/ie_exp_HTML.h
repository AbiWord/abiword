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

#include "ie_exp.h"

/* NOTE: I'm trying to keep the code similar across versions,
 *       and therefore features are enabled/disabled here:
 */

/* Define if the base unicode char is UCS-4
 */
#define HTML_UCS4

/* Define if the sniffers need to pass export name to parent
 */
#define HTML_NAMED_CONSTRUCTORS

/* Define if the [P/X]HTML export options dialog is implemented
 */
#define HTML_DIALOG_OPTIONS

/* Define if the tables are supported
 */
#define HTML_TABLES_SUPPORTED

/* Define if meta information is supported
 */
#define HTML_META_SUPPORTED

/* TODO: Rather than having separate sniffers to differentiate
 *       the exporter's behaviour, should have a dialog box
 *       with options.
 */
#ifndef HTML_DIALOG_OPTIONS
#define HTML_ENABLE_HTML4 
#define HTML_ENABLE_PHTML
#endif
#define HTML_ENABLE_MHTML


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

	virtual UT_Confidence_t supportsMIME (const char * szMimeType);
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

	virtual UT_Confidence_t supportsMIME (const char * szMimeType);
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

#ifdef HTML_DIALOG_OPTIONS
#include "xap_Dlg_HTMLOptions.h"
#else

/* This struct is defined in xap_Dlg_HTMLOptions.h, but that file does not
 * exist in AbiWord-1.0.x so a definition is provided here for the moment.
 */
struct XAP_Exp_HTMLOptions
{
	bool	bIs4;
	bool	bIsAbiWebDoc;
	bool	bDeclareXML;
	bool	bAllowAWML;
	bool	bEmbedCSS;
	bool	bLinkCSS;
	bool	bEmbedImages;
	bool    bClassOnly;
	bool    bAbsUnits;
	bool	bScaleUnits;
	UT_uint32 iCompact;
	/* other options, not set/saved/restore by options dialog
	 */
	bool	bMultipart;
};

#endif /* HTML_DIALOG_OPTIONS */

class s_StyleTree;

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

	static void printStyleTree(PD_Document *pDocument, UT_ByteBuf& tree);

private:
	UT_Error            _doOptions ();
	void				_buildStyleTree ();
protected:
	virtual UT_Error	_writeDocument ();
public:
	virtual UT_Error	_writeDocument (bool bClipBoard, bool bTemplateBody);
private:
	s_StyleTree *		m_style_tree;
	bool				m_bSuppressDialog;
	XAP_Exp_HTMLOptions	m_exp_opt;
	UT_UTF8String       m_sLinkCSS;
	UT_UTF8String       m_sTitle;
};

#endif /* IE_EXP_HTML_H */
