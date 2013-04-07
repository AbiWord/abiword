/* AbiWord
 * Copyright (C) 2005 Dom Lachowicz <cinamod@hotmail.com>
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


#ifndef IE_EXP_PDF_H
#define IE_EXP_PDF_H

#include "ie_exp.h"

class ABI_EXPORT IE_Exp_PS_Sniffer : public IE_ExpSniffer
{
  friend class IE_Exp;

public:
  IE_Exp_PS_Sniffer ();
  virtual ~IE_Exp_PS_Sniffer ();

  UT_Confidence_t supportsMIME (const char * szMIME);

  virtual bool recognizeSuffix (const char * szSuffix);
  virtual bool getDlgLabels (const char ** szDesc,
			     const char ** szSuffixList,
			     IEFileType * ft);
  virtual UT_Error constructExporter (PD_Document * pDocument,
				      IE_Exp ** ppie);
};

class ABI_EXPORT IE_Exp_SVG_Sniffer : public IE_ExpSniffer
{
  friend class IE_Exp;

public:
  IE_Exp_SVG_Sniffer ();
  virtual ~IE_Exp_SVG_Sniffer ();

  UT_Confidence_t supportsMIME (const char * szMIME);

  virtual bool recognizeSuffix (const char * szSuffix);
  virtual bool getDlgLabels (const char ** szDesc,
			     const char ** szSuffixList,
			     IEFileType * ft);
  virtual UT_Error constructExporter (PD_Document * pDocument,
				      IE_Exp ** ppie);
};

class ABI_EXPORT IE_Exp_PDF_Sniffer : public IE_ExpSniffer
{
  friend class IE_Exp;

public:
  IE_Exp_PDF_Sniffer ();
  virtual ~IE_Exp_PDF_Sniffer ();

  UT_Confidence_t supportsMIME (const char * szMIME);

  virtual bool recognizeSuffix (const char * szSuffix);
  virtual bool getDlgLabels (const char ** szDesc,
			     const char ** szSuffixList,
			     IEFileType * ft);
  virtual UT_Error constructExporter (PD_Document * pDocument,
				      IE_Exp ** ppie);
};

#endif
