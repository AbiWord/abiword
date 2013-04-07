/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2011 Ben Martin
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


#ifndef IE_IMP_RDF_H
#define IE_IMP_RDF_H

#include <stdio.h>
#include "ie_imp.h"
#include "pd_Document.h"

class pf_Frag_Strux;

/**
 * Importer/Reader classes for various types that can become RDF objects
 * in abiword
 */

class ABI_EXPORT IE_Imp_RDF_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;
	friend class IE_Imp_RDF;

  protected:

  public:
	IE_Imp_RDF_Sniffer( const char * name );
	virtual ~IE_Imp_RDF_Sniffer();

	virtual UT_Confidence_t recognizeContents ( const char * szBuf,
                                                UT_uint32 iNumbytes );


};


class ABI_EXPORT IE_Imp_RDF_VCard_Sniffer : public IE_Imp_RDF_Sniffer
{
  public:
	IE_Imp_RDF_VCard_Sniffer();
	virtual ~IE_Imp_RDF_VCard_Sniffer();

	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);

    const IE_SuffixConfidence* getSuffixConfidence ();
    const IE_MimeConfidence*   getMimeConfidence ();

};


class ABI_EXPORT IE_Imp_RDF_Calendar_Sniffer : public IE_Imp_RDF_Sniffer
{
  public:
	IE_Imp_RDF_Calendar_Sniffer();
	virtual ~IE_Imp_RDF_Calendar_Sniffer();

	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);

    const IE_SuffixConfidence* getSuffixConfidence ();
    const IE_MimeConfidence*   getMimeConfidence ();

};




class ABI_EXPORT IE_Imp_RDF : public IE_Imp
{
  public:
	IE_Imp_RDF(PD_Document * pDocument, bool bEncoded=false);
	IE_Imp_RDF(PD_Document * pDocument, const char * encoding);
	virtual ~IE_Imp_RDF();

	virtual bool pasteFromBuffer( PD_DocumentRange * pDocRange,
                                  const unsigned char * pData, UT_uint32 lenData,
                                  const char * szEncoding = 0 );

  protected:
	virtual UT_Error	_loadFile(GsfInput * fp);

    virtual bool pasteFromBufferSS( PD_DocumentRange * pDocRange,
                                    std::stringstream& ss,
                                    const char * szEncoding );

    std::pair< PT_DocPosition, PT_DocPosition > insertTextWithXMLID( const std::string& text,
                                                                     const std::string& xmlid );


  private:

};

class ABI_EXPORT IE_Imp_RDF_VCard : public IE_Imp_RDF
{
  public:
	IE_Imp_RDF_VCard(PD_Document * pDocument, bool bEncoded=false);
	IE_Imp_RDF_VCard(PD_Document * pDocument, const char * encoding);
	virtual ~IE_Imp_RDF_VCard();

    virtual bool pasteFromBufferSS( PD_DocumentRange * pDocRange,
                                    std::stringstream& ss,
                                    const char * szEncoding );

};


class ABI_EXPORT IE_Imp_RDF_Calendar : public IE_Imp_RDF
{
  public:
	IE_Imp_RDF_Calendar(PD_Document * pDocument, bool bEncoded=false);
	IE_Imp_RDF_Calendar(PD_Document * pDocument, const char * encoding);
	virtual ~IE_Imp_RDF_Calendar();

    virtual bool pasteFromBufferSS( PD_DocumentRange * pDocRange,
                                    std::stringstream& ss,
                                    const char * szEncoding );

};


#endif
