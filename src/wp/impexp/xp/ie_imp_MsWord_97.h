/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2001 Dom Lachowicz <dominicl@seas.upenn.edu> 
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

#ifndef IE_IMP_MSWORD_H
#define IE_IMP_MSWORD_H

// The importer/reader for Microsoft Word Documents

#include "ie_imp.h"
#include "ut_string_class.h"

//
// forward decls so that we don't have to #include "wv.h" here
//
typedef struct _wvParseStruct wvParseStruct;
typedef struct _Blip Blip;
typedef struct _CHP CHP;
class PD_Document;

//
// The Sniffer/Manager/Creator Class for DOC
//
class IE_Imp_MsWord_97_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;

public:
	IE_Imp_MsWord_97_Sniffer() {}
	virtual ~IE_Imp_MsWord_97_Sniffer() {}

	virtual bool recognizeContents (const char * szBuf, 
									UT_uint32 iNumbytes);
	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);
};

// how many chars to buffer in our fields implementation
#define FLD_SIZE 40000

//
// The import class for the MSFT Word DOC format
//
class IE_Imp_MsWord_97 : public IE_Imp
{
public:
	IE_Imp_MsWord_97 (PD_Document * pDocument);
	~IE_Imp_MsWord_97 ();

	UT_Error			importFile (const char * szFilename);
	void                pasteFromBuffer (PD_DocumentRange *, 
										 unsigned char *, unsigned int, const char * szEncoding = 0);

   	// wv's callbacks need access to these, so they have to be public
	int             _specCharProc (wvParseStruct *ps, UT_uint16 eachchar, 
								   CHP * achp);
	int             _charProc (wvParseStruct *ps, UT_uint16 eachchar,
							   UT_Byte chartype,  UT_uint16 lid);
	int 			_docProc  (wvParseStruct *ps, UT_uint32 tag);
	int 			_eleProc  (wvParseStruct *ps, UT_uint32 tag,
							   void *props, int dirty);	

private:

	int        _beginSect (wvParseStruct *ps, UT_uint32 tag,
						   void *props, int dirty);
	int        _endSect (wvParseStruct *ps, UT_uint32 tag,
						 void *props, int dirty);

	int        _beginPara (wvParseStruct *ps, UT_uint32 tag,
						   void *props, int dirty);
	int        _endPara (wvParseStruct *ps, UT_uint32 tag,
						 void *props, int dirty);

	int        _beginChar (wvParseStruct *ps, UT_uint32 tag,
						   void *props, int dirty);
	int        _endChar (wvParseStruct *ps, UT_uint32 tag,
						 void *props, int dirty);

   	UT_Error   _handleImage (Blip *, long width, long height);
	bool       _handleCommandField (char *command);
	int        _fieldProc (wvParseStruct *ps, UT_uint16 eachchar, 
						   UT_Byte chartype, UT_uint16 lid);
	void       _appendChar (UT_UCSChar ch);
	void       _flush ();

private:

	UT_UCS2String       m_pTextRun;
   	UT_uint32           m_iImageCount;
	UT_uint32           m_nSections;

	UT_UCSChar m_command [FLD_SIZE];
	UT_UCSChar m_argument [FLD_SIZE];
};

#endif /* IE_IMP_MSWORD_H */
