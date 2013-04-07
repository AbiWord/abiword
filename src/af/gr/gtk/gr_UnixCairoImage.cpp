/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2008 Dominic Lachowicz
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

#include "gr_UnixCairoImage.h"

static gboolean convertToPng(const gchar *buf,
							 gsize count,
							 GError **error,
							 gpointer byteBuf)
{
	UT_ByteBuf *pBB = reinterpret_cast<UT_ByteBuf *>(byteBuf);
	return pBB->append(reinterpret_cast<const UT_Byte *>(buf), count) ? TRUE : FALSE;
}

/*!
 * Does not keep a reference to @pixbuf
 */
GR_UnixCairoImage::GR_UnixCairoImage(const char *name, GdkPixbuf *pixbuf)
  : GR_CairoImage(name)
{
	UT_ByteBuf *pBB = new UT_ByteBuf();
	
	// TODO: massively inefficient
	if (!gdk_pixbuf_save_to_callback(pixbuf,
									 convertToPng,
									 reinterpret_cast<gpointer>(pBB),
									 "png",
									 NULL, NULL, NULL))
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
	else
		{
			bool result = convertFromBuffer(pBB);
			if (!result)
				{
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				}
		}

	delete pBB;
}

/*!
 * Returns a GdkPixbuf equivalent form of this image. The caller owns the ref to the image,
 * and should call g_object_unref() on it when finished
 */
GdkPixbuf* GR_UnixCairoImage::getPixbuf()
{
	// TODO: massively inefficient
	UT_ByteBuf *pBB = 0;

	if (convertToBuffer(&pBB))
		{
			GdkPixbufLoader *loader = gdk_pixbuf_loader_new();
			GdkPixbuf *pixbuf = 0;

			if (!gdk_pixbuf_loader_write(loader, 
										 static_cast<const guchar *>(pBB->getPointer (0)),
										 static_cast<gsize>(pBB->getLength ()), NULL))
				{
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					goto error;
				}

			if (!gdk_pixbuf_loader_close(loader, NULL))
				{
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					goto error;
				}

			pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);

			if (pixbuf != 0)
				{
					g_object_ref(G_OBJECT(pixbuf));
				}

		error:
			g_object_unref(loader);
			delete pBB;

			return pixbuf;
		}
	
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}
