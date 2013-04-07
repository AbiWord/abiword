/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998-2003 AbiSource, Inc.
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


#ifndef IE_IMP_WML_H
#define IE_IMP_WML_H

#include "ie_imp_XML.h"
#include "ie_Table.h"

class PD_Document;

// The importer/reader for WML files.

class IE_Imp_WML_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;

public:
	IE_Imp_WML_Sniffer(const char * name);
	virtual ~IE_Imp_WML_Sniffer() {}

	virtual const IE_SuffixConfidence * getSuffixConfidence ();
	virtual const IE_MimeConfidence * getMimeConfidence ();
	virtual UT_Confidence_t recognizeContents (const char * szBuf,
									UT_uint32 iNumbytes);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);

};

class IE_Imp_WML : public IE_Imp_XML
{
public:
	IE_Imp_WML(PD_Document * pDocument);
	virtual ~IE_Imp_WML();

	void			startElement(const gchar *name,
					      const gchar **atts);
	void			endElement(const gchar *name);
	void			charData(const gchar *s, int len);

 private:
	void openTable(const gchar **atts);
	void closeTable(void);
	void openRow(const gchar **atts);
	void closeRow(void);
	void openCell(const gchar **atts);
	void closeCell(void);
	void createImage(const char *name, const gchar **atts);

	bool						m_bOpenedBlock;
	bool						m_bOpenedSection;
	UT_sint32					m_iColumns;
	UT_uint32					m_iImages;
	UT_sint32					m_iOpenedColumns;
	IE_Imp_TableHelperStack *	m_TableHelperStack;
};

#endif /* IE_IMP_WML_H */
