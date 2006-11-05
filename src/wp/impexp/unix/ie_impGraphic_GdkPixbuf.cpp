/* -*- c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*- */
/* AbiWord Graphic importer employing GdkPixbuf
 * Copyright (C) 2001 Martin Sevior
 * Copyright (C) 2002 Dom Lachowicz
 * Copyright (C) 2005 Marc Maurer
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

#define GDK_PIXBUF_ENABLE_BACKEND
#include "ie_impGraphic_GdkPixbuf.h"

//------------------------------------------------------------------------------------

/*! 
 * This class will import any graphic type supported by gdk-pixbuf
 * into a png buffer for abiword.
 */

static void _write_png( png_structp png_ptr, 
		        png_bytep data, 
		        png_size_t length )
{
	UT_ByteBuf* bb = static_cast<UT_ByteBuf*>(png_get_io_ptr(png_ptr));
	bb->append(data, length);
	return;
}


//static void _write_flush(png_structp png_ptr) { } // Empty Fuction.

//------------------------------------------------------------------------------------

IE_ImpGraphic_GdkPixbuf::IE_ImpGraphic_GdkPixbuf()
	: IE_ImpGraphic(), m_pPngBB(NULL)
{
}

IE_ImpGraphic_GdkPixbuf::~IE_ImpGraphic_GdkPixbuf()
{
	// we likely don't own the m_pPngBB, so don't free it
}

/*!
 * Convert an image data buffer into PNG image buffer.
 */
UT_Error IE_ImpGraphic_GdkPixbuf::importGraphic(UT_ByteBuf * pBB, FG_Graphic ** ppfg)
{
	GdkPixbuf * pixbuf = pixbufForByteBuf ( pBB );

	if (!pixbuf)
	{
		UT_DEBUGMSG (("GdkPixbuf: couldn't get image from loader!\n"));
		return UT_ERROR;
	}

	// Initialize stuff to create our PNG.
	UT_Error err = Initialize_PNG();
	if (err)
	{
		g_object_unref(G_OBJECT(pixbuf));
		return err;
	}

	if (setjmp(m_pPNG->jmpbuf))
	{
		DELETEP(m_pPngBB);
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

	FG_GraphicRaster * pFGR = new FG_GraphicRaster();
	if(pFGR == NULL)
	{
		DELETEP(m_pPngBB);
		return UT_IE_NOMEMORY;
	}

	if(!pFGR->setRaster_PNG(m_pPngBB)) 
	{
		DELETEP(pFGR);
		DELETEP(m_pPngBB);
		return UT_IE_FAKETYPE;
	}

	*ppfg = static_cast<FG_Graphic *>(pFGR);
	return UT_OK;
}

/*!
 * Convert an image byte buffer into a PNG byte buffer
 */
UT_Error IE_ImpGraphic_GdkPixbuf::convertGraphic(UT_ByteBuf* pBB,
								UT_ByteBuf** ppBB)
{
	GdkPixbuf * pixbuf = pixbufForByteBuf (pBB);

	if (!pixbuf)
	{			
		return UT_ERROR;
	}

	// Initialize stuff to create our PNG.
	UT_Error err =Initialize_PNG();
	if (err)
	{
		g_object_unref(G_OBJECT(pixbuf));
		return err;
	}

	if (setjmp(m_pPNG->jmpbuf))
	{
		DELETEP(m_pPngBB);
		png_destroy_write_struct(&m_pPNG, &m_pPNGInfo);
		g_object_unref(G_OBJECT(pixbuf));
		return UT_ERROR;
	}

	//		
	// Build the png member variables.
	//
	_createPNGFromPixbuf(pixbuf);

	// cleanup
	g_object_unref(G_OBJECT(pixbuf));
	png_destroy_write_struct(&m_pPNG, &m_pPNGInfo);

	*ppBB =  m_pPngBB;
	return UT_OK;
}

/*!
 * This method fills the m_pPNG byte buffer with a PNG representation of 
 * of the supplied gdk-pixbuf.
 * This can be saved in the PT as a data-item and recreated.
 * ppBB is a pointer to a pointer of a byte buffer. It's the callers
 * job to delete it.
 */
void IE_ImpGraphic_GdkPixbuf::_createPNGFromPixbuf(GdkPixbuf * pixbuf)
{
	int colorType = PNG_COLOR_TYPE_RGB;

	if(gdk_pixbuf_get_has_alpha(pixbuf))
	{
		colorType =  PNG_COLOR_TYPE_RGB_ALPHA;
	}

	//
	// OK define the PNG header from the info in GdkPixbuf
	//

	UT_uint32 width =  gdk_pixbuf_get_width(pixbuf);
	UT_uint32 height = gdk_pixbuf_get_height(pixbuf);
	int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	guchar * pBuf = gdk_pixbuf_get_pixels(pixbuf);
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
	png_set_compression_level(m_pPNG,3); // 3 is much faster and keeps a reasonable size
	UT_Byte* pngScanline = new UT_Byte[rowstride];
	UT_DEBUGMSG(("COnverting pixbuf to png \n"));
	for(UT_uint32 i =0; i < height; i++)
	{
		memmove(pngScanline,pBuf,rowstride);
		png_write_row(m_pPNG, pngScanline);
		pBuf += rowstride;
	}
	UT_DEBUGMSG(("Conversion over \n"));

	DELETEPV (pngScanline);		
	png_write_end(m_pPNG, m_pPNGInfo);
}

/*!
 * Massage the byte buffer into an array of strings that can be loaded by 
 * gdk-pixbuf
 */
GdkPixbuf * IE_ImpGraphic_GdkPixbuf::_loadXPM(UT_ByteBuf * pBB)
{
	GdkPixbuf * pixbuf = NULL;
	const char * pBC = reinterpret_cast<const char *>(pBB->getPointer(0));

	UT_GenericVector<char*> vecStr;
	UT_sint32 k =0;
	UT_sint32 iBase =0;

	//
	// Find dimension line to start with.
	//
	UT_sint32 length = static_cast<UT_sint32>(pBB->getLength());
	for(k =0; (*(pBC+k) != '"') &&( k < length); k++)
		;

	if(k >= length)
	{
		return NULL;
	}

	k++;
	iBase = k;
	for(k =k; (*(pBC+k) != '"') && (k < length); k++)
		;
	if(k >= length)
	{
		return NULL;
	}

	char * sz = NULL;
	UT_sint32 kLen = k-iBase+1;
	sz = static_cast<char *>(UT_calloc(kLen,sizeof(char)));
	UT_sint32 i =0;

	for(i=0; i< (kLen -1); i++)
	{
		*(sz+i) = *(pBC+iBase+i);
	}
	*(sz+i) = 0;
	vecStr.addItem(sz);

	//
	// Now loop through all the lines until we get to "}" outside the
	// '"'
	while((*(pBC+k) != '}')  && (k < length) )
	{
		k++;

		//
		// Load a single string of data into our vector.
		// 
		if(*(pBC+k) =='"')
		{
			//
			// Start of a line
			//
			k++;
			iBase = k;
			for(k =k; (*(pBC+k) != '"') && (k < length); k++) {}
			if(k >= length)
			{
				return NULL;
			}
			sz = NULL;
			kLen = k-iBase+1;
			sz = static_cast<char *>(UT_calloc(kLen,sizeof(char)));
			for(i=0; i<(kLen -1); i++)
			{
				*(sz+i) = *(pBC+iBase+i);
			}
			*(sz +i) = 0;
			vecStr.addItem(sz);
		}
	}

	if(k >= length)
	{
		for(i=0; i< static_cast<UT_sint32>(vecStr.getItemCount()); i++)
		{
			char * psz = vecStr.getNthItem(i);
			FREEP(psz);
		}
		return NULL;
	}

	const char ** pszStr = static_cast<const char **>(UT_calloc(vecStr.getItemCount(),sizeof(char *)));
	for(i=0; i< static_cast<UT_sint32>(vecStr.getItemCount()); i++)
		pszStr[i] = vecStr.getNthItem(i);
	pixbuf = gdk_pixbuf_new_from_xpm_data(pszStr);
	DELETEP(pszStr);
	return pixbuf;
}

GdkPixbuf * IE_ImpGraphic_GdkPixbuf::pixbufForByteBuf (UT_ByteBuf * pBB)
{
	if ( !pBB || !pBB->getLength() )
		return NULL;

	GdkPixbuf * pixbuf = NULL;

	bool bIsXPM = false;
	const char * szBuf = reinterpret_cast<const char *>(pBB->getPointer(0));
	if((pBB->getLength() > 9) && (strncmp (szBuf, "/* XPM */", 9) == 0))
	{
		bIsXPM = true;
	}

	if(bIsXPM)
	{
		pixbuf = _loadXPM(pBB);
	}
	else
	{
		GError * err = 0;
		GdkPixbufLoader * ldr = 0;

		ldr = gdk_pixbuf_loader_new ();
		if (!ldr)
			{
				UT_DEBUGMSG (("GdkPixbuf: couldn't create loader! WTF?\n"));
				UT_ASSERT (ldr);
				return NULL ;
			}

		if ( FALSE== gdk_pixbuf_loader_write (ldr, static_cast<const guchar *>(pBB->getPointer (0)),
											  static_cast<gsize>(pBB->getLength ()), &err) )
			{
				UT_DEBUGMSG(("DOM: couldn't write to loader: %s\n", err->message));
				g_error_free(err);
				gdk_pixbuf_loader_close (ldr, NULL);
				g_object_unref (G_OBJECT(ldr));
				return NULL ;
			}
		
		gdk_pixbuf_loader_close (ldr, NULL);
		pixbuf = gdk_pixbuf_loader_get_pixbuf (ldr);

		// ref before closing the loader
		if ( pixbuf )
			g_object_ref (G_OBJECT(pixbuf));

		g_object_unref (G_OBJECT(ldr));
	}

	return pixbuf;
}
	
UT_Error IE_ImpGraphic_GdkPixbuf::Initialize_PNG(void)
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
	if (setjmp(m_pPNG->jmpbuf))
		{
			/* Free all of the memory associated with the png_ptr and info_ptr */
			png_destroy_write_struct(&m_pPNG, &m_pPNGInfo);
			
			/* If we get here, we had a problem reading the file */
			return UT_ERROR;
		}
	m_pPngBB = new UT_ByteBuf;  /* Byte Buffer for Converted Data */
	
	/* Setting up the Data Writing Function */
	png_set_write_fn(m_pPNG, static_cast<void *>(m_pPngBB), static_cast<png_rw_ptr>(_write_png), NULL);
	
	return UT_OK;
}

// ---------------------------------------------------------------------------

IE_ImpGraphicGdkPixbuf_Sniffer::IE_ImpGraphicGdkPixbuf_Sniffer()
  : IE_ImpGraphicSniffer()
{
}

IE_ImpGraphicGdkPixbuf_Sniffer::~IE_ImpGraphicGdkPixbuf_Sniffer() 
{
}

/**** CODE STOLEN FROM gdk-pixbuf-io.c ****/

static gint 
format_check (GdkPixbufFormat *info, const guchar *buffer, int size)
{
	int i, j;
	gchar m;
	GdkPixbufModulePattern *pattern;
	gboolean anchored;
	guchar *prefix;
	gchar *mask;

	for (pattern = info->signature; pattern->prefix; pattern++) {
		if (pattern->mask && pattern->mask[0] == '*') {
			prefix = (guchar *)pattern->prefix + 1;
			mask = (gchar *)pattern->mask + 1;
			anchored = FALSE;
		}
		else {
			prefix = (guchar *)pattern->prefix;
			mask = (gchar *)pattern->mask;
			anchored = TRUE;
		}
		for (i = 0; i < size; i++) {
			for (j = 0; i + j < size && prefix[j] != 0; j++) {
				m = mask ? mask[j] : ' ';
				if (m == ' ') {
					if (buffer[i + j] != prefix[j])
						break;
				}
				else if (m == '!') {
					if (buffer[i + j] == prefix[j])
						break;
				}
				else if (m == 'z') {
					if (buffer[i + j] != 0)
						break;
				}
				else if (m == 'n') {
					if (buffer[i + j] == 0)
						break;
				}
			} 

			if (prefix[j] == 0) 
				return pattern->relevance;

			if (anchored)
				break;
		}
	}
	return 0;
}

static bool
_gdk_pixbuf_get_module (const guchar *buffer, guint size)
{
	GSList *formats, *format_ptr;

	gint score, best = 0;
	GdkPixbufFormat *selected = NULL;

	format_ptr = gdk_pixbuf_get_formats ();
	for (formats = format_ptr; formats; formats = g_slist_next (formats)) {
		GdkPixbufFormat *info = (GdkPixbufFormat *)formats->data;

#if 0
		if (info->disabled)
			continue;
#endif

		score = format_check (info, buffer, size);
		if (score > best) {
			best = score; 
			selected = info;
		}
		if (score >= 100) 
			break;
	}

	g_slist_free(format_ptr);
	if (selected != NULL)
		return true;

	return false;
}

/**** END CODE STOLEN FROM gdk-pixbuf-io.c ****/

/*!
 * Sniff the byte buffer to see if it contains vaild image data recognized
 * by gdk-pixbuf
 */
UT_Confidence_t IE_ImpGraphicGdkPixbuf_Sniffer::recognizeContents(const char * szBuf, UT_uint32 iNum)
{

	//
	//  Handle xpm differently coz the standard gdk-pixbuf loader scheme doesn't
	// work for these!
	//
	xxx_UT_DEBUGMSG(("gdk-pixbuf sniff happenning data %s \n",szBuf));
	if((iNum > 9) && (strncmp (szBuf, "/* XPM */", 9) == 0))
	{
		return UT_CONFIDENCE_PERFECT;
	}

	return _gdk_pixbuf_get_module((guchar *)szBuf, iNum);
}

UT_Confidence_t IE_ImpGraphicGdkPixbuf_Sniffer::recognizeSuffix(const char * szSuffix)
{
	// FIXME: get the supported formats from GdkPixbuf somehow
	static const char * suffixes[] =  
	{
		".jpg",
		".jpeg",
		".png",
		".tif",
		".tiff",
		".gif",
		".xpm",
		".pnm",
		".ras",
		".ico",
		".bmp",
		".xbm",
		".wmf"
	} ;

	for ( unsigned int i = 0; i < NrElements(suffixes); i++ )
		if ( UT_stricmp(szSuffix, suffixes[i] ) == 0 )
			return UT_CONFIDENCE_PERFECT;

	return UT_CONFIDENCE_ZILCH;
}

bool IE_ImpGraphicGdkPixbuf_Sniffer::getDlgLabels(const char ** pszDesc,
						  const char ** pszSuffixList,
						  IEGraphicFileType * ft)
{
	// TODO add a more complete list of suffixes
	*pszDesc = "All platform supported image formats";
	*pszSuffixList = "*.jpg; *.jpeg; *.png; *.tiff; *.gif; *.xpm; *.pnm; *.ras; *.ico; *.bmp; *.xbm; *.wmf";
	*ft = getType ();
	return true;
}
	
UT_Error IE_ImpGraphicGdkPixbuf_Sniffer::constructImporter(IE_ImpGraphic **ppieg)
{
	*ppieg = new IE_ImpGraphic_GdkPixbuf();
	if (*ppieg == NULL)
		return UT_IE_NOMEMORY;
	return UT_OK;
}
