/* -*- c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*- */
/* AbiWord Graphic importer employing GdkPixbuf
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 Martin Sevior
 *
 * Portions from GdkPixBuf Library 
 * Copyright (C) 1999 The Free Software Foundation
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


#ifndef IE_IMPGRAPHIC_GDKPIXBUF_H
#define IE_IMPGRAPHIC_GDKPIXBUF_H

#include "ut_types.h"
#include "ie_impGraphic.h"
#include "fg_GraphicRaster.h"
#include "ut_bytebuf.h"
#include "png.h"

typedef struct _GdkPixbuf GdkPixbuf;

class ABI_EXPORT IE_ImpGraphicPixbufGraphic : public IE_ImpGraphic
{
public:
	IE_ImpGraphicPixbufGraphic();
	virtual ~IE_ImpGraphicPixbufGraphic();
	virtual UT_Error convertGraphic(UT_ByteBuf* pBB,
							UT_ByteBuf** ppBB);
	virtual UT_Error importGraphic(UT_ByteBuf * pBB, FG_Graphic ** ppfg);
	virtual UT_Error IE_ImpGraphic(const char * szFilename, FG_Graphic ** ppfg);
	void  setXPM(bool b);

private:
	GdkPixbuf * _loadXPM(UT_ByteBuf * pBB);
	UT_Error  Initialize_PNG(void);
	void _createPNGFromPixbuf(GdkPixbuf * pixbuf);

// PNG structures used
	png_structp m_pPNG;				// libpng structure for the PNG Object
	png_infop   m_pPNGInfo;		// libpng structure for info on the PNG Object
	UT_ByteBuf*  m_pPngBB;				// pBB Converted to PNG File
	bool m_bIsXPM;
};


class ABI_EXPORT IE_ImpGraphicPixbufGraphic_Sniffer : public IE_ImpGraphicSniffer
{
public:

	IE_ImpGraphicPixbufGraphic_Sniffer();
	~IE_ImpGraphicPixbufGraphic_Sniffer();
	virtual UT_Confidence_t recognizeSuffix(const char * szSuffix);
	virtual UT_Confidence_t recognizeContents(const char * szBuf, UT_uint32 iNum);
	virtual bool getDlgLabels(const char ** pszDesc,
							  const char ** pszSuffixList,
							  IEGraphicFileType * ft);
	virtual UT_Error constructImporter(IE_ImpGraphic **ppieg);

private:
	bool m_bIsXPM;
};

#endif /*  IE_IMPGRAPHIC_GDKPIXBUF_H */






