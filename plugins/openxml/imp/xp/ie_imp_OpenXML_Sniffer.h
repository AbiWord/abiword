/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 *
 * Copyright (C) 2007 Philippe Milot <PhilMilot@gmail.com>
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

#ifndef _IE_IMP_OPENXMLSNIFFER_H_
#define _IE_IMP_OPENXMLSNIFFER_H_

#include "ie_imp.h"

class IE_Imp_OpenXML_Sniffer : public IE_ImpSniffer
{
public:
  IE_Imp_OpenXML_Sniffer () ;

  virtual ~IE_Imp_OpenXML_Sniffer ();

  virtual const IE_SuffixConfidence * getSuffixConfidence () override;
  virtual const IE_MimeConfidence * getMimeConfidence () override;
  virtual UT_Confidence_t recognizeContents (GsfInput * input) override;

  virtual UT_Error constructImporter (PD_Document * pDocument,
				      IE_Imp ** ppie) override;

  virtual bool getDlgLabels (const char ** szDesc,
			     const char ** szSuffixList,
			     IEFileType * ft) override;
};

#endif //_IE_IMP_OPENXMLSNIFFER_H_
