/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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


#ifndef IE_IMP_KWORD_1_H
#define IE_IMP_KWORD_1_H

#include "ut_string_class.h"
#include "ie_imp_XML.h"

class PD_Document;

// The importer/reader for KWord 1.0 files.

class ABI_EXPORT IE_Imp_KWord_1_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;

public:
	IE_Imp_KWord_1_Sniffer() {}
	virtual ~IE_Imp_KWord_1_Sniffer() {}

	virtual bool recognizeContents (const char * szBuf, 
									UT_uint32 iNumbytes);
	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);

};

class ABI_EXPORT IE_Imp_KWord_1 : public IE_Imp_XML
{
public:
  IE_Imp_KWord_1(PD_Document * pDocument);
  virtual ~IE_Imp_KWord_1();

  virtual void _startElement(const XML_Char *name, const XML_Char **atts);
  virtual void _endElement(const XML_Char *name);
  virtual void _charData(const XML_Char*, int len); 

private:

  void _appendText ();

  UT_UCS2String m_szTextBuffer;
  UT_String m_szProps;
  bool m_bInText;
};

#endif /* IE_IMP_KWORD_1_H */
