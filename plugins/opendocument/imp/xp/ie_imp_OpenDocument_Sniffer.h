/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 *
 * Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
 * Copyright (C) 2004 Robert Staudinger <robsta@stereolyzer.net>
 * Copyright (C) 2005 Daniel d'Andrada T. de Carvalho
 * <daniel.carvalho@indt.org.br>
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

#ifndef _IE_IMP_OPENDOCUMENTSNIFFER_H_
#define _IE_IMP_OPENDOCUMENTSNIFFER_H_

#include <ie_imp.h>

class IE_Imp_OpenDocument_Sniffer : public IE_ImpSniffer
{
public:
  IE_Imp_OpenDocument_Sniffer () ;

  virtual ~IE_Imp_OpenDocument_Sniffer ();

  virtual const IE_SuffixConfidence * getSuffixConfidence ();
  virtual const IE_MimeConfidence * getMimeConfidence ();
  virtual UT_Confidence_t recognizeContents (GsfInput * input);

  virtual UT_Error constructImporter (PD_Document * pDocument,
				      IE_Imp ** ppie) ;

  virtual bool getDlgLabels (const char ** szDesc,
			     const char ** szSuffixList,
			     IEFileType * ft) ;
};

#endif //_IE_IMP_OPENDOCUMENTSNIFFER_H_
