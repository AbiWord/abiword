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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef IE_IMP_XSL_FO_H
#define IE_IMP_XSL_FO_H

#include "ie_imp_XML.h"
#include "ie_Table.h"

class PD_Document;

// The importer/reader for XSL-FO files.

class IE_Imp_XSL_FO_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;

public:
	IE_Imp_XSL_FO_Sniffer(const char * name);
	virtual ~IE_Imp_XSL_FO_Sniffer() {}

	virtual const IE_SuffixConfidence * getSuffixConfidence ();
	virtual UT_Confidence_t recognizeContents (const char * szBuf,
									UT_uint32 iNumbytes);
	virtual const IE_MimeConfidence * getMimeConfidence () { return NULL; }
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);

};

class IE_Imp_XSL_FO : public IE_Imp_XML
{
public:
	IE_Imp_XSL_FO(PD_Document * pDocument);
	virtual ~IE_Imp_XSL_FO();

	void			    startElement(const gchar *name,
									  const gchar **atts);
	void			    endElement(const gchar *name);

	void				charData(const gchar *s, int len);

protected:

	bool				_isInListTag(void);
	UT_uint32			_tagTop(void);
	void createImage (const char *name, const gchar **atts);

private:

	UT_sint32			m_iBlockDepth;
	UT_sint32			m_iListDepth;
	UT_sint32			m_iListBlockDepth;
	UT_sint32			m_iTableDepth;
	UT_uint32			m_iFootnotes;
	UT_uint32			m_iImages;
	bool				m_bOpenedLink;
	bool				m_bPendingFootnote;
	bool				m_bInFootnote;
	bool				m_bIgnoreFootnoteBlock;

	UT_NumberStack				m_utnsTagStack;
	IE_Imp_TableHelperStack *	m_TableHelperStack;
};

#endif /* IE_IMP_XSL_FO_H */
