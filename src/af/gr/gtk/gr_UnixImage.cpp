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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <stdlib.h>

#include "gr_UnixImage.h"
#include "ut_assert.h"
#include "ut_bytebuf.h"
#include "ut_debugmsg.h"
#include "gr_Graphics.h"
#include "ut_string_class.h"
#include <gdk-pixbuf/gdk-pixbuf-loader.h>

/* quick implementation of cropped images, thecrop rectangle should go 
 * to GR_Image in next versions. */
class GR_UnixCroppedImage: public GR_UnixImage
{
public:
	GR_UnixCroppedImage(const char* pszName);
	GR_UnixCroppedImage(const char* pszName, GdkPixbuf * pPixbuf);

	virtual void cairoSetSource(cairo_t *);
	void crop(double left, double top, double right, double bottom);
private:
	 double m_CropLeft, m_CropRight, m_CropTop, m_CropBottom;
};

GR_UnixCroppedImage::GR_UnixCroppedImage(const char* pszName):
	GR_UnixImage(pszName),
	m_CropLeft(0.),
	m_CropRight(0.),
	m_CropTop(0.),
	m_CropBottom(0.)
{
}

GR_UnixCroppedImage::GR_UnixCroppedImage(const char* pszName, GdkPixbuf * pPixbuf):
	GR_UnixImage(pszName, pPixbuf),
	m_CropLeft(0.),
	m_CropRight(0.),
	m_CropTop(0.),
	m_CropBottom(0.)
{
}

void GR_UnixCroppedImage::cairoSetSource(cairo_t *cr)
{
	GdkPixbuf const *image = getData();
	UT_return_if_fail(image);
	double w, h;
	w = gdk_pixbuf_get_width(image);
	h = gdk_pixbuf_get_height(image);
	double scaleX = (double)getDisplayWidth() / w / (1 - m_CropLeft - m_CropRight);
	double scaleY = (double)getDisplayHeight() / h / (1 - m_CropTop - m_CropBottom);
	cairo_scale(cr, scaleX, scaleY);
	cairo_rectangle(cr, 0., 0.,
	                (1 - m_CropLeft - m_CropRight) * w,
	                (1 - m_CropTop - m_CropBottom) * h);
	cairo_clip(cr);
	gdk_cairo_set_source_pixbuf(cr, image, -m_CropLeft * w,
	                            -m_CropTop * h);
}

void GR_UnixCroppedImage::crop(double left, double top, double right, double bottom)
{
	m_CropLeft = left;
	m_CropTop = top;
	m_CropRight = right;
	m_CropBottom = bottom;
}

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

GR_UnixImage::GR_UnixImage(const char* szName,GdkPixbuf * pPixbuf )
  : m_image(pPixbuf)
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
  if(m_image)
      setDisplaySize (gdk_pixbuf_get_width (pPixbuf), gdk_pixbuf_get_height (pPixbuf));
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
	if(m_image != NULL)
	{
		UT_ASSERT(G_OBJECT(m_image)->ref_count == 1);
		g_object_unref(G_OBJECT(m_image));
	}
}

GR_Image::GRType GR_UnixImage::getType(void) const
{
	return  m_ImageType;
}


GR_UnixImage *GR_UnixImage::makeSubimage(const std::string & name,
                           UT_sint32 x, UT_sint32 y,
                           UT_sint32 width, UT_sint32 height) const
{
    if(m_image == NULL)
	    return NULL;
	GR_UnixCroppedImage * pImage = new GR_UnixCroppedImage(name.c_str());

    pImage->m_image = gdk_pixbuf_copy(m_image);
	if (pImage->m_image == NULL)
	{
		delete pImage;
		return NULL;
	}
	pImage->setDisplaySize (getDisplayWidth(), getDisplayHeight());

	pImage->crop((double)x / (double)getDisplayWidth(),
	             (double)y / (double)getDisplayHeight(),
	             1. - ((double)x + width) / getDisplayWidth(),
	             1. - ((double)y + height) / getDisplayHeight());
    return pImage;
}


/*!
 * Scale our image to rectangle given by rec. The dimensions of rec
 * are calculated in logical units.
 */
void GR_UnixImage::scaleImageTo(GR_Graphics * pG, const UT_Rect & rec)
{
	UT_sint32 width = pG->tdu(rec.width);
	UT_sint32 height = pG->tdu(rec.height);
	scale(width,height);
}

static gboolean convCallback(const gchar *buf,
			     gsize count,
				 GError ** /*error*/,
			     gpointer byteBuf)
{
  UT_ByteBuf * pBB = reinterpret_cast<UT_ByteBuf *>(byteBuf);
  pBB->append(reinterpret_cast<const UT_Byte *>(buf),count);
  return TRUE;
}

/*!
 * This method fills a byte buffer with a PNG representation of itself.
 * This can be saved in the PT as a data-item and recreated.
 * ppBB is a pointer to a pointer of a byte buffer. It's the callers
 * job to delete it.
 */
bool  GR_UnixImage::convertToBuffer(UT_ByteBuf** ppBB) const
{
  if (!m_image)
  {
    UT_ASSERT(m_image);
    *ppBB = 0;
    return false;
  }
  
  
  UT_ByteBuf * pBB = 0;
  const guchar * pixels = gdk_pixbuf_get_pixels(m_image);

  if (pixels)
  {
    GError    * error =NULL;
    pBB = new UT_ByteBuf();		
    gdk_pixbuf_save_to_callback(m_image,
				convCallback,
				reinterpret_cast<gpointer>(pBB),
				"png",
				&error,NULL,NULL);
    if(error != NULL)
      {
	g_error_free (error);
      }
  }
  
  *ppBB = pBB;
  // UT_ASSERT(G_OBJECT(m_image)->ref_count == 1);
  return true;
}

bool GR_UnixImage::saveToPNG(const char * szFile)
{
        UT_return_val_if_fail(m_image,false);

	GError * error = NULL;
	gboolean res = gdk_pixbuf_save (m_image, szFile, "png", &error,NULL);
	if(res != FALSE)
	{
		return true;
	}
	delete error;
	return false;

}

/*! 
 * Returns true if pixel at point (x,y) in device units is transparent.
 */
bool GR_UnixImage::isTransparentAt(UT_sint32 x, UT_sint32 y)
{
  if(!hasAlpha())
  {
    return false;
  }
  UT_return_val_if_fail(m_image,false);
  // UT_sint32 iBitsPerPixel = gdk_pixbuf_get_bits_per_sample(m_image);
  UT_sint32 iRowStride = gdk_pixbuf_get_rowstride(m_image);
  UT_sint32 iWidth =  gdk_pixbuf_get_width(m_image);
  UT_sint32 iHeight =  gdk_pixbuf_get_height(m_image);
  UT_ASSERT(iRowStride/iWidth == 4);
  UT_return_val_if_fail((x>= 0) && (x < iWidth), false);
  UT_return_val_if_fail((y>= 0) && (y < iHeight), false);
  guchar * pData = gdk_pixbuf_get_pixels(m_image);
  xxx_UT_DEBUGMSG(("BitsPerPixel = %d \n",iBitsPerPixel));
  UT_sint32 iOff = iRowStride*y;
  guchar pix0 = pData[iOff+ x*4];
  guchar pix1 = pData[iOff+ x*4 +1];
  guchar pix2 = pData[iOff+ x*4 +2];
  guchar pix3 = pData[iOff+ x*4 +3];
  if((pix3 | pix0 | pix1 | pix2) == 0)
  {
    return true;
  }
  UT_DEBUGMSG(("x %d y %d pix0 %d pix1 %d pix2 %d pix3 %d \n",x,y,pix0,pix1,pix2,pix3));
  return false;
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
	
	// don't scale if passed -1 for either, or either is 0
	if (iDisplayWidth <= 0 || iDisplayHeight <= 0)
		return;

	setDisplaySize (iDisplayWidth, iDisplayHeight);
}

/*!
Loads an image from from a byte buffer. Note: If the specified width and/or
height does not match the size of the image contained in the byte buffer,
some image information will be lost. WARNING: TODO
@param iDisplayWidth the width to which the image needs to be scaled. Values
are in device units. Setting this to -1 will cause the image not to be scaled.
@param iDisplayheight the height to which the image needs to be scaled. Values
are in device units. Setting this to -1 will cause the image not to be scaled.
@return true if successful, false otherwise
*/
bool GR_UnixImage::convertFromBuffer(const UT_ByteBuf* pBB, 
                                     const std::string & /*mimetype */,
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

	// We explicitely do not scale the image on load, since cairo can do
	// that during rendering. Scaling during loading will result in lost
	// image information, which in turns results in bugs like
	// in bugs like #12183.
	// So simply remember the display size for drawing later.
	setDisplaySize(iDisplayWidth, iDisplayHeight);
	
	if ( !gdk_pixbuf_loader_write (ldr, static_cast<const guchar *>(pBB->getPointer (0)),static_cast<gsize>(pBB->getLength ()), &err) )
	{
		if(err != NULL)
		{
			UT_DEBUGMSG(("DOM: couldn't write to loader:%s \n", err->message));
			g_error_free(err);
		}
		gdk_pixbuf_loader_close (ldr, NULL);
		g_object_unref(G_OBJECT(ldr));
		return false;
	}

	if ( !gdk_pixbuf_loader_close (ldr, &err) )
	{
		if(err != NULL)
		{
			UT_DEBUGMSG(("DOM: couldn't close loader:%s \n", err->message));
			g_error_free(err);
		}
		g_object_unref(G_OBJECT(ldr));
		return false;
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
		g_object_unref(G_OBJECT(ldr));
		return false;
	}

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
	g_object_unref(G_OBJECT(ldr));
//
// Adjust the reference count so it's always 1. Unfortunately 
// the reference count after the close is undefined.
//
	while(G_OBJECT(m_image)->ref_count > 1)
	{
		g_object_unref(G_OBJECT(m_image));
	}
	UT_ASSERT(G_OBJECT(m_image)->ref_count == 1);

	UT_ASSERT(G_OBJECT(m_image)->ref_count == 1);
	
	return true;
}


void GR_UnixImage::cairoSetSource(cairo_t * cr)
{
	UT_return_if_fail(m_image);
	double scaleX = (double)getDisplayWidth() / (double)gdk_pixbuf_get_width (m_image);
	double scaleY = (double)getDisplayHeight() / (double)gdk_pixbuf_get_height(m_image);
	cairo_scale(cr, scaleX, scaleY);
	gdk_cairo_set_source_pixbuf(cr, m_image, 0, 0);
}
