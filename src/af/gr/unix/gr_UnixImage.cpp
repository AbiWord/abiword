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
#include "gr_Graphics.h"
#include "ut_string_class.h"
#include <gdk-pixbuf/gdk-pixbuf-loader.h>

/*!
 * From Martin. I spent a LOT of time in class tracking down terrible
 * Memory leaks because the only way to delete a pixbuf from memory
 * is to unref an object of reference count 1. 
 * Unfortunately I discovered from experience that some operations appear
 * to leave reference count in an undefined state.
 *
 * So a note to future hackers in this class. Please keep a close eye on the
 * reference count on the GdkPixbuf's and keep the liberal asserts I've placed.
 */
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
	GObject * obj = G_OBJECT(m_image);
	UT_ASSERT(obj->ref_count == 1);
	g_object_unref(G_OBJECT(m_image));

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
/*!
 * The idea is to create a
 * new image from the rectangular segment in device units defined by 
 * UT_Rect rec. The Image should be deleted by the calling routine.
 */
GR_Image * GR_UnixImage::createImageSegment(GR_Graphics * pG,const UT_Rect & rec)
{
	UT_sint32 x = pG->tdu(rec.left);
	UT_sint32 y = pG->tdu(rec.top);
	if(x < 0)
	{
		x = 0;
	}
	if(y < 0)
	{
		y = 0;
	}
	UT_sint32 width = pG->tdu(rec.width);
	UT_sint32 height = pG->tdu(rec.height);
	UT_sint32 dH = getDisplayHeight();
	UT_sint32 dW = getDisplayWidth();
	if(height > dH)
	{
		height = dH;
	}
	if(width > dW)
	{
		width = dW;
	}
	if(x + width > dW)
	{
		width = dW - x;
	}
	if(y + height > dH)
	{
		height = dH - y;
	}
	if(width < 0)
	{
		x = dW -1;
		width = 1;
	}
	if(height < 0)
	{
		y = dH -1;
		height = 1;
	}
	UT_String sName("");
	getName(sName);
    UT_String sSub("");
	UT_String_sprintf(sSub,"_segemnt_%d_%d_%d_%d",x,y,width,height);
	sName += sSub;
	GR_UnixImage * pImage = new GR_UnixImage(sName.c_str());
	UT_ASSERT(m_image);
	pImage->m_image = gdk_pixbuf_new_subpixbuf(m_image,x,y,width,height);
//
//  gdk_pixbuf_new_subpixbuf shares pixels with the original pixbuf and
//  so puts a reference on m_image.

	g_object_unref(G_OBJECT(m_image));
	UT_ASSERT(G_OBJECT(m_image)->ref_count == 1);
//
// Make a copy so we don't have to worry about ref counting the orginal.
//
	pImage->m_image = gdk_pixbuf_copy(pImage->m_image);
	return static_cast<GR_Image *>(pImage);
}

/*!
 * Scale our image to rectangle given by rec. The dimensions of rec
 * are calculated in logical units.
 */
void GR_UnixImage::scaleImageTo(GR_Graphics * pG, const UT_Rect & rec)
{
	UT_sint32 width = pG->tdu(rec.width);
	UT_sint32 height = pG->tdu(rec.height);
	if((width == getDisplayWidth()) && (height == getDisplayHeight()))
	{
		return;
	}
	scale(width,height);
	UT_ASSERT(G_OBJECT(m_image)->ref_count == 1);
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
	UT_ASSERT(G_OBJECT(m_image)->ref_count == 1);
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
	UT_ASSERT(G_OBJECT(m_image)->ref_count == 1);
	g_object_unref(G_OBJECT(m_image));
	m_image = image;
	UT_ASSERT(G_OBJECT(m_image)->ref_count == 1);
}

// note that this does take device units, unlike everything else.
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
	
	gdk_pixbuf_loader_set_size(ldr, iDisplayWidth, iDisplayHeight);
	
	if ( FALSE== gdk_pixbuf_loader_write (ldr, static_cast<const guchar *>(pBB->getPointer (0)),
										  static_cast<gsize>(pBB->getLength ()), &err) )
	{
		UT_DEBUGMSG(("DOM: couldn't write to loader: %s\n", err->message));
		g_error_free(err);
		gdk_pixbuf_loader_close (ldr, NULL);
		return false ;
	}
//
// This is just pointer to the buffer in the loader. This can be deleted
// when we close the loader.
//
	m_image = gdk_pixbuf_loader_get_pixbuf (ldr);
	if (!m_image)
	{
		UT_DEBUGMSG (("GdkPixbuf: couldn't get image from loader!\n"));
		gdk_pixbuf_loader_close (ldr, NULL);
		return false;
	}

	G_IS_OBJECT(G_OBJECT(m_image));
//
// Have to put a reference on it to prevent it being deleted during the close.
//
	g_object_ref(G_OBJECT(m_image));
	if ( FALSE == gdk_pixbuf_loader_close (ldr, &err) )
	{
		UT_DEBUGMSG(("DOM: error closing loader. Corrupt image: %s\n", err->message));
		g_error_free(err);
		g_object_unref(G_OBJECT(m_image));
		return false;
	}
//
// Adjust the reference count so it's always 1. Unfortunately 
// the reference count after the close is undefined.
//
	while(G_OBJECT(m_image)->ref_count > 1)
	{
		g_object_unref(G_OBJECT(m_image));
	}
	UT_ASSERT(G_OBJECT(m_image)->ref_count == 1);
	
	// if gdk_pixbuf_loader_set_size was not able to scale the image, then do it manually
	if (iDisplayWidth != gdk_pixbuf_get_width (m_image) || iDisplayHeight != gdk_pixbuf_get_height(m_image))
		scale (iDisplayWidth, iDisplayHeight);
	UT_ASSERT(G_OBJECT(m_image)->ref_count == 1);

	return true;
}
