/* AbiWord
 * Copyright (C) 2001-2002 Dom Lachowicz
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

#include "gr_UnixImage.h"
#include "ut_assert.h"
#include "ut_bytebuf.h"
#include "ut_debugmsg.h"

#include <gdk-pixbuf/gdk-pixbuf-loader.h>

GR_UnixImage::GR_UnixImage(const char* szName)
  : m_image(NULL)
{
  if (szName)
  {
    setName ( szName );
  }
  else
  {
    setName ( "GdkPixbufImage" );
  }
  m_ImageType = GR_Image::GRT_Raster;
}


GR_UnixImage::GR_UnixImage(const char* szName, GR_Image::GRType imageType)
  : m_image(NULL)
{
  if (szName)
  {
    setName ( szName );
  }
  else
  {
    setName ( "GdkPixbufImage" );
  }
  m_ImageType = imageType;
}

GR_UnixImage::~GR_UnixImage()
{
	UT_return_if_fail(m_image);
	g_object_unref (G_OBJECT(m_image));
}

GR_Image::GRType GR_UnixImage::getType(void) const
{
	return  m_ImageType;
}

UT_sint32  GR_UnixImage::getDisplayWidth(void) const
{
	UT_return_val_if_fail(m_image, 0);
	return gdk_pixbuf_get_width (m_image);
}

UT_sint32  GR_UnixImage::getDisplayHeight(void) const
{
	UT_return_val_if_fail(m_image, 0);

	return gdk_pixbuf_get_height (m_image);
}

bool  GR_UnixImage::convertToBuffer(UT_ByteBuf** ppBB) const
{
  if (!m_image)
    {
		UT_ASSERT(m_image);
		*ppBB = 0;
		return false;
    }
  
	const guchar * pixels = gdk_pixbuf_get_pixels(m_image);
	UT_ByteBuf * pBB = 0;

	if (pixels)
	{
		// length is height * rowstride
		UT_uint32 len = gdk_pixbuf_get_height (m_image) * 
			gdk_pixbuf_get_rowstride (m_image);
		pBB = new UT_ByteBuf();		
		pBB->append(static_cast<const UT_Byte *>(pixels), len);		
	}

	*ppBB = pBB;
	return true;
}

bool GR_UnixImage::hasAlpha (void) const
{
	UT_return_val_if_fail(m_image, false);
	return (gdk_pixbuf_get_has_alpha (m_image) ? true : false);
}

UT_sint32 GR_UnixImage::rowStride (void) const
{
	UT_return_val_if_fail(m_image, 0);
	return static_cast<UT_sint32>(gdk_pixbuf_get_rowstride (m_image));
}

// note that this does take device units, unlike everything else.
void GR_UnixImage::scale (UT_sint32 iDisplayWidth, 
						  UT_sint32 iDisplayHeight)
{
	UT_return_if_fail(m_image);
	
	// don't scale if passed -1 for either
	if (iDisplayWidth < 0 || iDisplayHeight < 0)
		return;

	GdkPixbuf * image = 0;

	image = gdk_pixbuf_scale_simple (m_image, iDisplayWidth, 
					 iDisplayHeight, GDK_INTERP_BILINEAR);

	g_object_unref (G_OBJECT(m_image));
	m_image = image;
}

bool GR_UnixImage::convertFromBuffer(const UT_ByteBuf* pBB, 
				     UT_sint32 iDisplayWidth, 
				     UT_sint32 iDisplayHeight)
{
	// assert no image loaded yet
	UT_ASSERT(!m_image);

	GError * err = 0;
	GdkPixbufLoader * ldr = gdk_pixbuf_loader_new ();	

	if (!ldr)
	{
		UT_DEBUGMSG (("GdkPixbuf: couldn't create loader! WTF?\n"));
		UT_ASSERT (ldr);
		return false;
	}
	
	if ( FALSE== gdk_pixbuf_loader_write (ldr, static_cast<const guchar *>(pBB->getPointer (0)),
										  static_cast<gsize>(pBB->getLength ()), &err) )
	{
		UT_DEBUGMSG(("DOM: couldn't write to loader: %s\n", err->message));
		g_error_free(err);
		gdk_pixbuf_loader_close (ldr, NULL);
		return false ;
	}

	m_image = gdk_pixbuf_loader_get_pixbuf (ldr);
	if (!m_image)
	{
		UT_DEBUGMSG (("GdkPixbuf: couldn't get image from loader!\n"));
		gdk_pixbuf_loader_close (ldr, NULL);
		return false;
	}

	scale (iDisplayWidth, iDisplayHeight);
	g_object_ref (G_OBJECT(m_image));
   
	if ( FALSE == gdk_pixbuf_loader_close (ldr, &err) )
	{
		UT_DEBUGMSG(("DOM: error closing loader. Corrupt image: %s\n", err->message));
		g_error_free(err);
		g_object_unref(G_OBJECT(m_image));
		return false;
	}

	return true;
}
