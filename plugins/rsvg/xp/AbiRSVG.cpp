/* -*- c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*- */

/* AbiWord SVG Graphic importer employing LibRSVG and GdkPixbuf
 * Copyright (C) 2002 Dom Lachowicz
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

#ifdef _WIN32
#include <stddef.h>
#endif
#include <string.h>
#include <glib.h>
#include <librsvg/rsvg.h>
#include <png.h>

#include "ut_debugmsg.h"
#include "ut_types.h"
#include "ut_string.h"
#include "xap_Module.h"
#include "ie_impGraphic.h"
#include "fg_GraphicRaster.h"

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_librsvg_register
#define abi_plugin_unregister abipgn_librsvg_unregister
#define abi_plugin_supports_version abipgn_librsvg_supports_version
// dll exports break static linking
#define ABI_BUILTIN_FAR_CALL extern "C"
#else
#define ABI_BUILTIN_FAR_CALL ABI_FAR_CALL
ABI_PLUGIN_DECLARE("AbiRSVG")
#endif

// Workaround for __jmpbuf #define on AIX
#undef jmpbuf

//------------------------------------------------------------------------------------

/*! 
 * This class will import SVGs into PNG byte buffers for AbiWord
 */

static void _write_png( png_structp png_ptr, 
						png_bytep data, 
						png_size_t length )
{
	UT_ByteBuf* bb = static_cast<UT_ByteBuf*>(png_get_io_ptr(png_ptr));
	bb->append(data, length);
}

static void _write_flush(png_structp /*png_ptr*/) { } // Empty Fuction.

//------------------------------------------------------------------------------------

class IE_RSVGBitmapGraphic : public IE_ImpGraphic
{
public:

	IE_RSVGBitmapGraphic()
		: IE_ImpGraphic(), m_pPngBB(0)
	{
	}
	
	virtual ~IE_RSVGBitmapGraphic()
	{
		// we likely don't own the m_pPngBB, so don't free it
	}
	
	/*!
	 * Convert an image data buffer into PNG image buffer.
	 */
	virtual UT_Error importGraphic(UT_ByteBuf * pBB, FG_ConstGraphicPtr & pfg)
	{
		UT_Error err = _importGraphic(pBB);
		if ( err != UT_OK )
			return err ;

		FG_GraphicRasterPtr pFGR(new FG_GraphicRaster);
		if(pFGR == NULL)
			{
				return UT_IE_NOMEMORY;
			}
		if(!pFGR->setRaster_PNG(m_pPngBB)) 
			{
				DELETEP(pFGR);			
				return UT_IE_FAKETYPE;
			}
		
		pfg = std::move(pFGR);
		return UT_OK;
	}
  
private:

	UT_Error _importGraphic(UT_ByteBuf * pBB)
	{
		GdkPixbuf * pixbuf = NULL;		
		GError * err = NULL;
		
		RsvgHandle * rsvg = rsvg_handle_new ();
		if ( FALSE == rsvg_handle_write ( rsvg, static_cast<const guchar *>(pBB->getPointer (0)),
										  static_cast<gsize>(pBB->getLength ()), &err ) )
			{
				UT_DEBUGMSG(("DOM: couldn't write to loader: %s\n", err->message));
				g_error_free(err);
				return UT_ERROR ;
			}
		
		if ( FALSE == rsvg_handle_close ( rsvg, &err ) )
			{
				UT_DEBUGMSG(("DOM: couldn't write to loader: %s\n", err->message));
				g_error_free(err);
				g_object_unref(G_OBJECT(rsvg));
				return UT_ERROR ;
			}
		
		pixbuf = rsvg_handle_get_pixbuf ( rsvg ) ;
		g_object_unref(G_OBJECT(rsvg));
		
		if (!pixbuf)
			{
				return UT_ERROR;
			}
		
		g_object_ref (pixbuf);
		
		// Initialize stuff to create our PNG.
		UT_Error error = Initialize_PNG();
		if (error)
			{
				return error;
			}
		
		if (setjmp(png_jmpbuf(m_pPNG)))
			{
				g_object_unref(G_OBJECT(pixbuf));
				png_destroy_write_struct(&m_pPNG, &m_pPNGInfo);
				return UT_ERROR;
			}
		
		//
		// Build the png member variables.
		//
		_createPNGFromPixbuf(pixbuf);

		//
		// Get rid of these now that they are no longer needed
		//
		g_object_unref(G_OBJECT(pixbuf));
		png_destroy_write_struct(&m_pPNG, &m_pPNGInfo);

		return UT_OK;
	}
	
	void _createPNGFromPixbuf(GdkPixbuf * pixbuf)
	{
		int colorType = PNG_COLOR_TYPE_RGB;

		//
		// OK define the PNG header from the info in GdkPixbuf
		//

		UT_uint32 width =  gdk_pixbuf_get_width(pixbuf);
		UT_uint32 height = gdk_pixbuf_get_height(pixbuf);
		int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
		guchar * pBuf = gdk_pixbuf_get_pixels(pixbuf);

		if(gdk_pixbuf_get_has_alpha(pixbuf))
		{
			colorType =  PNG_COLOR_TYPE_RGB_ALPHA;
		}

		//
		// Abi only has 8 bits per sample
		//
		static const int bitsPerSampleAbi = 8;
		png_set_IHDR ( m_pPNG,  m_pPNGInfo,
					   width,			       
					   height,			       
					   bitsPerSampleAbi,
					   colorType,
					   PNG_INTERLACE_NONE,
					   PNG_COMPRESSION_TYPE_DEFAULT,
					   PNG_FILTER_TYPE_DEFAULT );
		
		png_write_info(m_pPNG, m_pPNGInfo);
		UT_Byte* pngScanline = new UT_Byte[rowstride];

		for(UT_uint32 i =0; i < height; i++)
		{
			memmove(pngScanline,pBuf,rowstride);
			png_write_row(m_pPNG, pngScanline);
			pBuf += rowstride;
		}

		DELETEP (pngScanline);		
		png_write_end(m_pPNG, m_pPNGInfo);
	}

	UT_Error Initialize_PNG(void)
	{
		/* Set up png structures for writing */
		m_pPNG = png_create_write_struct( PNG_LIBPNG_VER_STRING, 
										  static_cast<void*>(NULL),
										  NULL, 
										  NULL );
		if( m_pPNG == NULL )
			{
				return UT_ERROR;
			}
		
		m_pPNGInfo = png_create_info_struct(m_pPNG);
		if ( m_pPNGInfo == NULL )
			{
				png_destroy_write_struct(&m_pPNG, static_cast<png_infopp>(NULL));
				return UT_ERROR;
		}
		
		/* Set error handling if you are using the setjmp/longjmp method (this is
		 * the normal method of doing things with libpng).  REQUIRED unless you
		 * set up your own error handlers in the png_create_read_struct() earlier.
		 */
		if (setjmp(png_jmpbuf(m_pPNG)))
			{
				/* Free all of the memory associated with the png_ptr and info_ptr */
				png_destroy_write_struct(&m_pPNG, &m_pPNGInfo);
				
				/* If we get here, we had a problem reading the file */
				return UT_ERROR;
			}
		m_pPngBB = new UT_ByteBuf;  /* Byte Buffer for Converted Data */
		
		/* Setting up the Data Writing Function */
		png_set_write_fn(m_pPNG, static_cast<void *>(m_pPngBB), static_cast<png_rw_ptr>(_write_png), static_cast<png_flush_ptr>(_write_flush));
		
		return UT_OK;
	}
  
  // PNG structures used
  png_structp  m_pPNG;				// libpng structure for the PNG Object
  png_infop    m_pPNGInfo;			// libpng structure for info on the PNG Object
  UT_ByteBuf*  m_pPngBB;			// pBB Converted to PNG File
};

//------------------------------------------------------------------------------------

// supported suffixes
static IE_SuffixConfidence IE_RSVGGraphic_Sniffer__SuffixConfidence[] = {
	{ "svg", 	UT_CONFIDENCE_PERFECT 	},
	{ "", 	UT_CONFIDENCE_ZILCH 	}
};

class IE_RSVGGraphic_Sniffer : public IE_ImpGraphicSniffer
{
public:

  IE_RSVGGraphic_Sniffer() 
	  : IE_ImpGraphicSniffer()
	{
	}

  virtual ~IE_RSVGGraphic_Sniffer() 
	{
	}

  const IE_SuffixConfidence * getSuffixConfidence ()
	{
		return IE_RSVGGraphic_Sniffer__SuffixConfidence;
	}

  virtual UT_Confidence_t recognizeContents(const char * szBuf, UT_uint32 /*iNum*/)
	{
		// todo: make me better
		if ( strstr ( szBuf, "xml" ) != NULL && 
			 strstr ( szBuf, "svg" ) != NULL )
			return UT_CONFIDENCE_PERFECT;
		return UT_CONFIDENCE_ZILCH;
	}
	
	virtual const IE_MimeConfidence * getMimeConfidence () 
	{
		/* Aliases once this plugin supports mime types (list courtesy of gdk-pixbuf)
		image/svg+xml
		image/svg
		image/svg-xml
		image/vnd.adobe.svg+xml
		text/xml-svg
		*/
		return NULL;
	}

	virtual bool getDlgLabels(const char ** pszDesc,
							  const char ** pszSuffixList,
							  IEGraphicFileType * ft)
	{
		// TODO add a more complete list of suffixes
		*pszDesc = "SVG Images (.svg)";
		*pszSuffixList = "*.svg";
		*ft = getType ();
		return true;
	}
	
	virtual UT_Error constructImporter(IE_ImpGraphic **ppieg)
	{
		*ppieg = new IE_RSVGBitmapGraphic();
		if (*ppieg == NULL)
			return UT_IE_NOMEMORY;
		return UT_OK;
	}

} ;

// -----------------------------------------------------------------------


static IE_RSVGGraphic_Sniffer * m_sniffer = 0;

ABI_BUILTIN_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{
  mi->name = "LibRSVG SVG image loader plugin";
    mi->desc = "This will enable AbiWord to read SVG files";
    mi->version = ABI_VERSION_STRING;
    mi->author = "Dom Lachowicz <cinamod@hotmail.com>";
    mi->usage = "No Usage";

    if (!m_sniffer)
    {
      // create a new sniffer
      m_sniffer = new IE_RSVGGraphic_Sniffer ();
	}

    IE_ImpGraphic::registerImporter ( m_sniffer );

    return 1;
}

ABI_BUILTIN_FAR_CALL
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
    mi->name = 0;
    mi->desc = 0;
    mi->version = 0;
    mi->author = 0;
    mi->usage = 0;

    if ( m_sniffer )
	{
		IE_ImpGraphic::unregisterImporter ( m_sniffer );
		delete m_sniffer;
		m_sniffer = 0;
	}

    return 1;
}

ABI_BUILTIN_FAR_CALL
int abi_plugin_supports_version (UT_uint32 /*major*/, UT_uint32 /*minor*/, UT_uint32 /*release*/)
{
	return isCurrentAbiVersion(major, minor, release);
}
