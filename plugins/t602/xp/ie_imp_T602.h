/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2001 Petr Tomasek <tomasek@etf.cuni.cz>
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


#ifndef IE_IMP_T602_H
#define IE_IMP_T602_H

#include <stdio.h>
#include "ie_imp.h"
#include "ut_string_class.h"

// The importer for T602 documents.

class IE_Imp_T602_Sniffer : public IE_ImpSniffer
{
   friend class IE_Imp;
   friend class IE_Imp_T602;

public:
   IE_Imp_T602_Sniffer();
   virtual ~IE_Imp_T602_Sniffer() {}

   virtual const IE_SuffixConfidence * getSuffixConfidence ();
   virtual const IE_MimeConfidence * getMimeConfidence () { return NULL; }
   virtual UT_Confidence_t recognizeContents (const char * szBuf, UT_uint32 iNumbytes);
   virtual bool getDlgLabels (const char ** szDesc, const char ** szSuffixList, IEFileType * ft);
   virtual UT_Error constructImporter (PD_Document * pDocument, IE_Imp ** ppie);
};

class IE_Imp_T602 : public IE_Imp
{
public:
   IE_Imp_T602(PD_Document * pDocument);
   ~IE_Imp_T602();

protected:
   virtual UT_Error	_loadFile(GsfInput * input);

   UT_uint16 _conv(unsigned char c);
   bool _getbyte(unsigned char &c);
   UT_Error _writeheader();
   UT_Error _writeTP();
   UT_Error _writePP();
   UT_Error _writeSP();
   UT_Error _write_fh(UT_String & fh,UT_uint32 id, bool hea);
   UT_Error _ins(UT_uint16);
   UT_Error _dotcom(unsigned char ch);
   UT_Error _inschar(unsigned char c, bool eol);

 private:
   GsfInput *m_importFile;
// T602 Document attributes...
   int m_charset;
   UT_String m_family;
   UT_String m_basefamily;
   bool m_softcr;
   int m_basesize;
   int m_size;
   UT_String m_lmargin;
   UT_String m_rmargin;
   int m_bold;
   int m_italic;
   int m_underline;
   int m_tpos;
   int m_big;
   UT_String m_color;
   int m_sfont;
// .dot commands
   bool m_eol;
// paragraph attributes
   int m_lheight; // line height: 1==1, 2==1.5, 3==2
// section attributes..
   UT_uint32 m_footer;
   UT_uint32 m_header;
   UT_uint32 m_fhc; // footer/header counter...
   UT_String m_fbuff;
   UT_String m_hbuff;
// write (section) headers..
   bool m_writeheader;

};

#endif /* IE_IMP_T602_H */
