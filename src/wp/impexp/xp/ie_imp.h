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


#ifndef IE_IMP_H
#define IE_IMP_H

#include "ut_types.h"
#include "ie_types.h"

#include "ut_AbiObject.h"
#include "ut_vector.h"
#include "ut_string_class.h"
#include "pt_Types.h"

class PD_Document;

// IE_Imp defines the abstract base class for file importers.

class IE_Imp;

class ABI_EXPORT IE_ImpSniffer : public UT_AbiObject
{
	friend class IE_Imp;
	
public:
	IE_ImpSniffer(const char * name);
	virtual ~IE_ImpSniffer();
	
	// these you get for free
	inline bool supportsFileType (IEFileType type) {return m_type == type;}
	inline IEFileType getFileType() const {return m_type;}
	
	// these you must override these next 4 methods!!!

	/*!
	 * Return a number in the range [0,255] as to your confidence
	 * that you recognize the contents. 0 being the least, 127 being
	 * so-so, 255 being absolutely sure
	 */
	virtual UT_Confidence_t recognizeContents (const char * szBuf, 
						   UT_uint32 iNumbytes) = 0;
	/*!
	 * Return a number in the range [0,255] as to your confidence
	 * that you recognize the suffix. 0 being the least, 127 being
	 * so-so, 255 being absolutely sure
	 */
	virtual UT_Confidence_t recognizeSuffix (const char * szSuffix) = 0;

	/*!
	 * Return a number in the range [0,255] as to your confidence
	 * that you can import this MIME type. 0 being the least, 127 being
	 * so-so, 255 being absolutely sure
	 */
	virtual UT_Confidence_t supportsMIME (const char * szMIME) { return UT_CONFIDENCE_ZILCH; }

	virtual bool getDlgLabels (const char ** szDesc,
				   const char ** szSuffixList,
				   IEFileType * ft) = 0;
	virtual UT_Error constructImporter (PD_Document * pDocument,
					    IE_Imp ** ppie) = 0;
	
	const UT_UTF8String & name () const { return m_name; }

private:
	const UT_UTF8String	m_name;

	// only IE_Imp ever calls this
	inline void setFileType (IEFileType type) {m_type = type;}
	IEFileType m_type;
};

class ABI_EXPORT IE_Imp
{
public:
	
	// constructs an importer of the right type based upon
	// either the filename or sniffing the file.  caller is
	// responsible for destroying the importer when finished
	// with it.

	static IEFileType	fileTypeForContents(const char * szBuf,
						    UT_uint32 iNumbytes);
	
	static IEFileType	fileTypeForSuffix(const char * szSuffix);
	static IEFileType	fileTypeForDescription(const char * szSuffix);

	static IEFileType fileTypeForSuffixes(const char * suffixList);

	static IE_ImpSniffer * snifferForFileType(IEFileType ieft);
	static const char * suffixesForFileType(IEFileType ieft);
	static const char * descriptionForFileType(IEFileType ieft);
	
	static UT_Error	constructImporter(PD_Document * pDocument,
					  const char * szFilename,
					  IEFileType ieft,
					  IE_Imp ** ppie, 
					  IEFileType * pieft = NULL);
	
	static bool	    enumerateDlgLabels(UT_uint32 ndx,
					       const char ** pszDesc,
					       const char ** pszSuffixList,
					       IEFileType * ft);

	static UT_uint32	getImporterCount(void);	
	static void registerImporter (IE_ImpSniffer * sniffer);
	static void unregisterImporter (IE_ImpSniffer * sniffer);
	static void unregisterAllImporters ();

 public:
	IE_Imp(PD_Document * pDocument);
	virtual ~IE_Imp();
	virtual UT_Error	importFile(const char * szFilename) = 0;

	// default impl
	virtual void		pasteFromBuffer(PD_DocumentRange * pDocRange,
						unsigned char * pData, 
						UT_uint32 lenData, 
						const char * szEncoding = 0);
	
 protected:
	PD_Document *           getDoc() const;
	
 private:
	PD_Document *		m_pDocument;
};

class IE_ImpInserter
{
public:

	IE_ImpInserter (PD_Document * pDoc);
	IE_ImpInserter (PD_Document * pDoc, PT_DocPosition dpos);
	~IE_ImpInserter ();

	bool appendStrux (PTStruxType pts, const XML_Char ** attributes);
	bool appendSpan (const UT_UCSChar * p, UT_uint32 length);
	bool appendObject (PTObjectType pto, const XML_Char ** attributes);

	inline bool getIsPaste () const {
		return m_isPaste;
	}

	inline PD_Document * getDoc () const {
		return m_doc;
	}

	inline PT_DocPosition getDocPos () const {
		return m_dpos;
	}

private:

	IE_ImpInserter ();
	IE_ImpInserter (const IE_ImpInserter & other);
	IE_ImpInserter & operator= (const IE_ImpInserter & other);

	PD_Document * m_doc;
	bool m_isPaste;
	PT_DocPosition m_dpos;
};

#endif /* IE_IMP_H */
