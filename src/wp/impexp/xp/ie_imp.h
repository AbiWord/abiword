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

#include "ut_vector.h"

class PD_Document;
class PD_DocumentRange;

// IE_Imp defines the abstract base class for file importers.

class IE_Imp;

class IE_ImpSniffer
{
	friend class IE_Imp;
	
public:
	IE_ImpSniffer();
	virtual ~IE_ImpSniffer();
	
	// these you get for free
	inline bool supportsFileType (IEFileType type) {return m_type == type;}
	inline IEFileType getFileType() const {return m_type;}
	
	// these you must override these
	virtual bool recognizeContents (const char * szBuf, 
									UT_uint32 iNumbytes) = 0;
	virtual bool recognizeSuffix (const char * szSuffix) = 0;
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft) = 0;
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie) = 0;
	
private:
	// only IE_Imp ever calls this
	inline void setFileType (IEFileType type) {m_type = type;}
	IEFileType m_type;
};

class IE_Imp
{
public:
	
	// constructs an importer of the right type based upon
	// either the filename or sniffing the file.  caller is
	// responsible for destroying the importer when finished
	// with it.

	static IEFileType	fileTypeForContents(const char * szBuf,
											UT_uint32 iNumbytes);

	static IEFileType	fileTypeForSuffix(const char * szSuffix);
	
	static UT_Error		constructImporter(PD_Document * pDocument,
										  const char * szFilename,
										  IEFileType ieft,
										  IE_Imp ** ppie, 
										  IEFileType * pieft = NULL);

	static bool		    enumerateDlgLabels(UT_uint32 ndx,
										   const char ** pszDesc,
										   const char ** pszSuffixList,
										   IEFileType * ft);
	static UT_uint32	getImporterCount(void);
	static void init();

	static void registerImporter (IE_ImpSniffer * sniffer);
	static void unregisterImporter (IE_ImpSniffer * sniffer);

 public:
	IE_Imp(PD_Document * pDocument);
	virtual ~IE_Imp();
	virtual UT_Error	importFile(const char * szFilename) = 0;
	virtual void		pasteFromBuffer(PD_DocumentRange * pDocRange,
						unsigned char * pData, UT_uint32 lenData) = 0;

protected:
	PD_Document *		m_pDocument;
};

#endif /* IE_IMP_H */
