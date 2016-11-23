/* -*- c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*- */

/* AbiWord Graphic importer employing GdkPixbuf
 * Copyright (C) 2001 Martin Sevior
 * Copyright (C) 2002 Dom Lachowicz
 * Copyright (C) 2005 Marc Maurer
 * Copyright (C) 2009 Hubert Figuiere
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include <string.h>

#include <string>

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>
#include <gdk-pixbuf/gdk-pixbuf-io.h>

#include "ut_debugmsg.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "ut_string.h"
#include "xap_Module.h"
#include "ie_impGraphic.h"
#include "fg_GraphicRaster.h"
#include <png.h>

#ifndef IE_IMPGRAPHIC_GDKPIXBUF_H
#define IE_IMPGRAPHIC_GDKPIXBUF_H

class ABI_EXPORT IE_ImpGraphic_GdkPixbuf : public IE_ImpGraphic
{
public:

	IE_ImpGraphic_GdkPixbuf();
	virtual ~IE_ImpGraphic_GdkPixbuf();

	virtual UT_Error importGraphic(UT_ByteBuf * pBB, FG_ConstGraphicPtr & pfg);

private:

	GdkPixbuf * pixbufForByteBuf (UT_ByteBuf * pBB, std::string & mimetype);
	void _createPNGFromPixbuf(GdkPixbuf * pixbuf);
	UT_Error _png_write(GdkPixbuf * pixbuf);

	GdkPixbuf * _loadXPM(UT_ByteBuf * pBB);
	UT_Error Initialize_PNG(void);

	// PNG structures used
	png_structp m_pPNG;				// libpng structure for the PNG Object
	png_infop   m_pPNGInfo;			// libpng structure for info on the PNG Object
	UT_ByteBuf*  m_pPngBB;			// pBB Converted to PNG File
};

class ABI_EXPORT IE_ImpGraphicGdkPixbuf_Sniffer : public IE_ImpGraphicSniffer
{
public:
	IE_ImpGraphicGdkPixbuf_Sniffer();
	virtual ~IE_ImpGraphicGdkPixbuf_Sniffer();

	virtual const IE_SuffixConfidence * getSuffixConfidence ();
	virtual const IE_MimeConfidence * getMimeConfidence ();
	virtual UT_Confidence_t recognizeContents(const char * szBuf, UT_uint32 iNum);
	virtual bool getDlgLabels(const char ** pszDesc,
							  const char ** pszSuffixList,
							  IEGraphicFileType * ft);
	virtual UT_Error constructImporter(IE_ImpGraphic **ppieg);
};

#endif // IE_IMPGRAPHIC_GDKPIXBUF_H

