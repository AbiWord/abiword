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

#ifndef IE_EXP_MSWORD_97_H
#define IE_EXP_MSWORD_97_H

#include "ie_exp.h"
#include "ut_vector.h"
#include "ut_misc.h"
#include "pl_Listener.h"

#include "exporter/wvexporter.h"

// forward declarations
//
class PD_Document;
class s_MsWord_97_Listener;

// The exporter/writer for the MsWord 97 format

class IE_Exp_MsWord_97 : public IE_Exp
{
		friend class s_MsWord_97_Listener;
        s_MsWord_97_Listener* m_pListener;

 public:
        IE_Exp_MsWord_97(PD_Document * pDocument);
        ~IE_Exp_MsWord_97();

        static UT_Bool          RecognizeSuffix(const char * szSuffix);
        static UT_Error         StaticConstructor(PD_Document * pDocument,
                                                                                   IE_Exp ** ppie);
        static UT_Bool          GetDlgLabels(const char ** pszDesc,
                                             const char ** pszSuffixList,
                                             IEFileType * ft);

        static UT_Bool          SupportsFileType(IEFileType ft);

		void            		write(const char * sz);
		void            		write(const char * sz, UT_uint32 length);

 protected:
		char * fileName;
 
        wvDocument* m_pWordDocument;
       
        wvStream* m_pMainStream;
        wvStream* m_pTableStream;
	wvStream* m_pSummaryStream;
        wvStream* m_pDataStream;

		// these are all overridden methods of the base class
		// see comments in ie_exp_MsWord_97.cpp for explanation

        UT_Error        		_writeDocument(void);
		UT_Bool 				_openFile(const char * szFileName);
		UT_uint32				_writeBytes(const UT_Byte * pBytes, UT_uint32 length);
		UT_Bool 				_writeBytes(const UT_Byte * pBytes);
		UT_Bool 				_closeFile(void);
		void 					_abortFile(void);
};

#endif
