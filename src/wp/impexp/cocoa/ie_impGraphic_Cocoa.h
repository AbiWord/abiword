/* -*- c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001, 2003 Hubert Figuiere
 *
 * Portions from Nisus Software and Apple documentation
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

#ifndef __IE_IMPGRAPHIC_COCOA_H__
#define __IE_IMPGRAPHIC_COCOA_H__

// WARNING: this file should conform to C++

#include "ut_bytebuf.h"
#include "ie_impGraphic.h"

class IE_ImpGraphicCocoa_Sniffer : public IE_ImpGraphicSniffer
{
 public:
	virtual UT_Confidence_t recognizeContents (const char * szBuf,
					UT_uint32 iNumbytes);
	virtual const IE_SuffixConfidence * getSuffixConfidence ();
	virtual const IE_MimeConfidence * getMimeConfidence ();
	virtual bool getDlgLabels (const char ** szDesc,
				   const char ** szSuffixList,
				   IEGraphicFileType * ft);
	virtual UT_Error constructImporter (IE_ImpGraphic ** ppieg);
};

class IE_ImpGraphic_Cocoa : public IE_ImpGraphic
{
public:
	virtual UT_Error    importGraphic(const UT_ConstByteBufPtr & pBB,
                                          FG_ConstGraphicPtr & pfg);
	virtual UT_Error    convertGraphic(const UT_ConstByteBufPtr & pBB,
                                       UT_ConstByteBufPtr & ppBB);
 private:
	UT_Error _convertGraphic(const UT_ConstByteBufPtr & pBB);
	UT_ConstByteBufPtr  m_pPngBB; 		// pBB Converted to PNG
};

#endif

