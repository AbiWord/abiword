/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2000 Hubert Figuière
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

// $Id$

#ifndef IE_IMP_MSWRITE_H
#define IE_IMP_MSWRITE_H

#include <stdio.h>
#include "ie_imp.h"
#include "ie_impexp_MSWrite.h"
class PD_Document;


// The importer/reader for MS Write Files.

class IE_Imp_MSWrite : public IE_Imp
{
 public:
   IE_Imp_MSWrite(PD_Document * pDocument);
   ~IE_Imp_MSWrite();
   
   virtual UT_Error     importFile(const char * szFilename);
   virtual void		pasteFromBuffer(PD_DocumentRange * pDocRange,
                                        unsigned char * pData, 
                                        UT_uint32 lenData);
   
   static UT_Bool		RecognizeContents(const char * szBuf, 
                                                  UT_uint32 iNumbytes);
   static UT_Bool		RecognizeSuffix(const char * szSuffix);
   static UT_Error		StaticConstructor(PD_Document * pDocument,
                                                  IE_Imp ** ppie);
   static UT_Bool		GetDlgLabels(const char ** pszDesc,
                                             const char ** pszSuffixList,
                                             IEFileType * ft);
   static UT_Bool 		SupportsFileType(IEFileType ft);
   
 protected:
   UT_Error			_parseFile(FILE * fp);
   UT_Error			_writeHeader(FILE * fp);
 private:
   static UT_uint32 ReadFileDesc(FILE * file, WRI_write_file_desc_t & desc);
   static UT_uint32 ReadText(FILE * file, const WRI_write_file_desc_t & desc, 
                             UT_uint16 pageNumber,	/* start at 1 */
                             UT_Byte * buf, UT_uint32 & bufLen);
   static UT_uint32 ReadFormatPage (FILE * file, int & pos, 
                                    WRI_format_page_t * aPage);

   static XML_Char * MakeProperties (XML_Char * buf, const size_t bufSize,
                                     XML_Char * attr, XML_Char * value);
   UT_Error ReadTextRangeWithFormat (FILE * fp, const UT_uint32 end, 
                                     const WRI_format_page_t & fmtData);
   
   //   static UT_uint32 ReadStyle(FILE * file, 
   //                         const write_file_desc & desc, 
   //                         UT_uint16 pageNumber,/* start at 1 */
   //                         text_run_array * & buf, size_t textLen);
   //   static UT_uint32 ReadPageStyle(text_run_array * & buf, 
   //                             UT_uint16 currentPage, 
   //                             size_t & remainingLen);
};

#endif /* IE_IMP_MSWRITE_H */








