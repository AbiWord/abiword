/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
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

#ifndef IE_IMPEXP_OPENWRITER_H
#define IE_IMPEXP_OPENWRITER_H

#include "ie_imp.h"
#include "ie_exp.h"

class IE_Imp_OpenWriter_Sniffer : public IE_ImpSniffer
{
public:
  IE_Imp_OpenWriter_Sniffer () ;

  virtual ~IE_Imp_OpenWriter_Sniffer ();

  virtual const IE_SuffixConfidence * getSuffixConfidence ();

  virtual const IE_MimeConfidence * getMimeConfidence () { return NULL; }

	virtual UT_Confidence_t recognizeContents (GsfInput * input);

  virtual UT_Error constructImporter (PD_Document * pDocument,
				      IE_Imp ** ppie) ;

  virtual bool getDlgLabels (const char ** szDesc,
			     const char ** szSuffixList,
			     IEFileType * ft) ;
};

class IE_Exp_OpenWriter_Sniffer : public IE_ExpSniffer
{
public:
  IE_Exp_OpenWriter_Sniffer();

  virtual ~IE_Exp_OpenWriter_Sniffer() ;

  virtual bool recognizeSuffix(const char * szSuffix) ;

  virtual UT_Error constructExporter(PD_Document * pDocument,
				     IE_Exp ** ppie) ;

  virtual bool getDlgLabels(const char ** pszDesc,
			    const char ** pszSuffixList,
			    IEFileType * ft) ;
};

#endif // IE_IMPEXP_OPENWRITER_H
