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

#include <stdlib.h>

#include "gr_UnixGnomeImage.h"
#include "ut_assert.h"
#include "ut_bytebuf.h"
#include "ut_debugmsg.h"

#include <gdk-pixbuf/gdk-pixbuf-loader.h>

GR_UnixGnomeImage::GR_UnixGnomeImage(const char* szName) 
	: m_image(NULL)
{
	if (szName)
    {
		strcpy(m_szName, szName);
    }
	else
    {
		strcpy(m_szName, "UnixGnomeImage");
    }
}

GR_UnixGnomeImage::~GR_UnixGnomeImage()
{
	gdk_pixbuf_unref (m_image);
}

UT_sint32	GR_UnixGnomeImage::getDisplayWidth(void) const
{
    return gdk_pixbuf_get_width (m_image);
}

UT_sint32	GR_UnixGnomeImage::getDisplayHeight(void) const
{
    return gdk_pixbuf_get_height (m_image);
}

bool		GR_UnixGnomeImage::convertToBuffer(UT_ByteBuf** ppBB) const
{
	const guchar * pixels = gdk_pixbuf_get_pixels(m_image);
	UT_ByteBuf * pBB = 0;

	if (pixels)
	{
		// length is height * rowstride
		UT_uint32 len = gdk_pixbuf_get_height (m_image) * 
			gdk_pixbuf_get_rowstride (m_image);
		pBB = new UT_ByteBuf();		
		pBB->append((const UT_Byte *)pixels, len);		
	}

	*ppBB = pBB;
	return true;
}

bool	GR_UnixGnomeImage::convertFromBuffer(const UT_ByteBuf* pBB, 
											 UT_sint32 iDisplayWidth, 
											 UT_sint32 iDisplayHeight)
{
	GdkPixbufLoader * ldr = gdk_pixbuf_loader_new ();
	UT_ASSERT (ldr);
	if (!ldr)
	{
		UT_DEBUGMSG (("GdkPixbuf: couldn't create loader! WTF?\n"));
		return false;
	}

#ifdef HAVE_GTK_2_0
	// we're GTK+ 2.0 ready :)
	GError * err = g_error_new (G_FILE_ERROR, G_FILE_ERROR_NOENT, "foobar");
	
	gdk_pixbuf_loader_write (ldr, (const guchar *)pBB->getPointer (0),
							 (gsize)pBB->getLength (), &err);
#else
	gdk_pixbuf_loader_write (ldr, (const guchar *)pBB->getPointer (0),
							 (gsize)pBB->getLength ());
#endif
	
	GdkPixbuf * image = gdk_pixbuf_loader_get_pixbuf (ldr);
	UT_ASSERT (image);
	if (!image)
	{
		UT_DEBUGMSG (("GdkPixbuf: couldn't get image from loader!\n"));
		return false;
	}

	m_image = gdk_pixbuf_scale_simple (image, iDisplayWidth, 
									   iDisplayHeight, GDK_INTERP_NEAREST);
	UT_ASSERT (m_image);
	if (!m_image)
	{
		UT_DEBUGMSG(("GdkPixbuf: couldn't scale image!\n"));
		return false;
	}

	gdk_pixbuf_ref (m_image);
	gdk_pixbuf_unref (image);

#ifdef HAVE_GTK_2_0
	g_error_free (err);
#endif

	gdk_pixbuf_loader_close (ldr);

	return true;
}
