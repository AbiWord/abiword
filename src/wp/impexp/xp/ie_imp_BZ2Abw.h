/* AbiWord
 * Copyright (C) 2001 Dom Lachowicz <doml@appligent.com>
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

#ifndef IE_IMP_BZ2ABW_H
#define IE_IMP_BZ2ABW_H

#include <stdio.h>
#include <bzlib.h>

#include "ie_imp_AbiWord_1.h"

class ABI_EXPORT IE_Imp_BZ2AbiWord_Sniffer : public IE_ImpSniffer
{
  friend class IE_Imp;
  
 public:
  
 public:
  IE_Imp_BZ2AbiWord_Sniffer() {}
  virtual ~IE_Imp_BZ2AbiWord_Sniffer() {}
  
  virtual bool recognizeContents (const char * szBuf, 
				  UT_uint32 iNumbytes);
  virtual bool recognizeSuffix (const char * szSuffix);
  virtual bool getDlgLabels (const char ** szDesc,
			     const char ** szSuffixList,
			     IEFileType * ft);
  virtual UT_Error constructImporter (PD_Document * pDocument,
				      IE_Imp ** ppie);	
};

class ABI_EXPORT IE_Imp_BZ2AbiWord : public IE_Imp_AbiWord_1, public UT_XML::Reader
{
 public:
  IE_Imp_BZ2AbiWord(PD_Document * pDocument);
  ~IE_Imp_BZ2AbiWord();

  /* Implementation of UT_XML::Reader
   */
  bool      openFile (const char * szFilename);
  UT_uint32 readBytes (char * buf, UT_uint32 length);
  void      closeFile (void);
  
 private:
  FILE   *m_fp;
  BZFILE *m_bzin;
};

#endif
