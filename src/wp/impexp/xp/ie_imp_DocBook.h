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


#ifndef IE_IMP_DOCBOOK_H
#define IE_IMP_DOCBOOK_H

#include 	"ie_imp_XML.h"
#include	"fl_AutoNum.h"

class PD_Document;

class ABI_EXPORT IE_Imp_DocBook_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;

public:
	IE_Imp_DocBook_Sniffer() {}
	virtual ~IE_Imp_DocBook_Sniffer() {}

	virtual UT_Confidence_t recognizeContents (const char * szBuf, 
									UT_uint32 iNumbytes);
	virtual UT_Confidence_t recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);

};

// The importer/reader for DocBook files.

class ABI_EXPORT IE_Imp_DocBook : public IE_Imp_XML
{
public:
	IE_Imp_DocBook(PD_Document * pDocument);
	virtual ~IE_Imp_DocBook();

	static bool		RecognizeContents(const char * szBuf, 
						  UT_uint32 iNumbytes);
	static bool		RecognizeSuffix(const char * szSuffix);
	static UT_Error		StaticConstructor(PD_Document * pDocument,
						  IE_Imp ** ppie);
	static bool		GetDlgLabels(const char ** pszDesc,
					     const char ** pszSuffixList,
					     IEFileType * ft);
	static bool 		SupportsFileType(IEFileType ft);
	

	void			startElement(const XML_Char *name, 
					      const XML_Char **atts);
	void			endElement(const XML_Char *name);
	
	void charData(const XML_Char *s, int len);

protected:
	int m_iInSection;
	int m_iCurListID;
	fl_AutoNum * m_iTitle [7];
	bool m_bMustAddTitle;
	bool m_bTitleAdded;
	bool m_bMustNumber;

	void createList (void);
};

#endif /* IE_IMP_DocBook_H */
