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

#if 0
// for some reason Dom's box is acting wierd since martin put this in the
// main tree. leave this in please for him until the real problem gets
// worked out
#include "ut_types.h"
typedef struct _GHashTable      GHashTable;
typedef struct _GMutex          GMutex;
typedef void (*GDestroyNotify) (void *data);
typedef struct _GData           GData;
typedef UT_uint32          GQuark;
#endif

#include <string.h>
#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>
#include "ie_impGraphic_GdkPixbuf.h"

#include "ut_debugmsg.h"

/*! 
 * This class will import any graphic type supported by gdk-pixbuf
 * into a png buffer for abiword.
 */

static void _write_png( png_structp png_ptr, 
		        png_bytep data, 
		        png_size_t length )
{
	UT_ByteBuf* bb = (UT_ByteBuf*) png_get_io_ptr(png_ptr);
	bb->append(data, length);
}


static void _write_flush(png_structp png_ptr) { } // Empty Fuction.

//------------------------------------------------------------------------------------

IE_ImpGraphicPixbufGraphic::IE_ImpGraphicPixbufGraphic()
{
	m_bIsXPM = false;
}
	
IE_ImpGraphicPixbufGraphic::~IE_ImpGraphicPixbufGraphic()
{
}

void  IE_ImpGraphicPixbufGraphic::setXPM(bool b)
{
		m_bIsXPM = b;
}

void IE_ImpGraphicPixbufGraphic::_createPNGFromPixbuf(GdkPixbuf * pixbuf)
{
	int colorType;

	//
	// OK define the PNG header from the info in GdkPixbuf
	//

	UT_uint32 width =  gdk_pixbuf_get_width(pixbuf);
	UT_uint32 height = gdk_pixbuf_get_height(pixbuf);
	int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	guchar * pBuf = gdk_pixbuf_get_pixels(pixbuf);
	colorType =  PNG_COLOR_TYPE_RGB;
	if(gdk_pixbuf_get_has_alpha(pixbuf))
	{
		colorType =  PNG_COLOR_TYPE_RGB_ALPHA;
	}
	//
	// Abi only has 8 bits per sample
	//
	int bitsPerSampleAbi =8;
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

/*!
 * Massage the byte buffer into an array of strings that can be loaded by 
 * gdk-pixbuf
 */
GdkPixbuf * IE_ImpGraphicPixbufGraphic::_loadXPM(UT_ByteBuf * pBB)
{
	GdkPixbuf * pixbuf = NULL;
	char * pBC = (char *) pBB->getPointer(0);

	UT_Vector vecStr;
	UT_sint32 k =0;
	UT_sint32 iBase =0;
	//
	// Find dimension line to start with.
	//
	UT_sint32 length = (UT_sint32) pBB->getLength();
	for(k =0; (*(pBC+k) != '"') &&( k < length); k++) {}
	if(k >= length)
	{
		return NULL;
	}
	k++;
	iBase = k;
	for(k =k; (*(pBC+k) != '"') && (k < length); k++) {}
	if(k >= length)
	{
		return NULL;
	}
	char * sz = NULL;
	UT_sint32 kLen = k-iBase+1;
	sz = (char *) UT_calloc(kLen,sizeof(char));
	UT_sint32 i =0;

	for(i=0; i< (kLen -1); i++)
	{
		*(sz+i) = *(pBC+iBase+i);
	}
	*(sz+i) = 0;
	vecStr.addItem((void *) sz);
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
			sz = (char *) UT_calloc(kLen,sizeof(char));
			for(i=0; i<(kLen -1); i++)
			{
				*(sz+i) = *(pBC+iBase+i);
			}
			*(sz +i) = 0;
			vecStr.addItem((void *) sz);
		}
	}
	if(k>=length)
	{
		for(i=0; i< (UT_sint32) vecStr.getItemCount(); i++)
		{
			char * psz = (char *) vecStr.getNthItem(i);
			FREEP(psz);
		}
		return NULL;
	}
	const char ** pszStr = (const char **) UT_calloc(vecStr.getItemCount(),sizeof(char *));
	for(i=0; i< (UT_sint32) vecStr.getItemCount(); i++)
	{
		pszStr[i] = (const char *) vecStr.getNthItem(i);
	}
	pixbuf = gdk_pixbuf_new_from_xpm_data(pszStr);
	DELETEP(pszStr);
//		for(i=0; i<(UT_sint32)vecStr.getItemCount(); i++)
//		{
//			char * psz = (char *) vecStr.getNthItem(i);
//		    FREEP(psz);
//		}
	return pixbuf;
}

/*!
 * Convert an image data buffer into PNG image buffer.
 */
UT_Error IE_ImpGraphicPixbufGraphic::importGraphic(UT_ByteBuf * pBB, FG_Graphic ** ppfg)
{
	GdkPixbuf * pixbuf = NULL;
	GdkPixbufLoader * ldr = NULL;
	if(m_bIsXPM)
	{
		pixbuf = _loadXPM(pBB);
	}
	else
	{
		ldr = gdk_pixbuf_loader_new ();
		UT_ASSERT (ldr);

#ifdef HAVE_GTK_2_0
		// we're GTK+ 2.0 ready :)
		GError * err = g_error_new (G_FILE_ERROR, G_FILE_ERROR_NOENT, "foobar");
	
		gdk_pixbuf_loader_write (ldr, (const guchar *)pBB->getPointer (0),
								 (gsize)pBB->getLength (), &err);
#else
		gdk_pixbuf_loader_write (ldr, (const guchar *)pBB->getPointer (0),
								 (gsize)pBB->getLength ());
#endif
	
		pixbuf = gdk_pixbuf_loader_get_pixbuf (ldr);
	}

	if (!pixbuf)
	{
		return false;
	}

	gdk_pixbuf_ref (pixbuf);

#ifdef HAVE_GTK_2_0
	g_error_free (err);
#endif
	if(ldr)
		gdk_pixbuf_loader_close (ldr);

		// Initialize stuff to create our PNG.
	UT_Error err =Initialize_PNG();
	if (err)
	{
		return err;
	}

	if (setjmp(m_pPNG->jmpbuf))
	{
		png_destroy_write_struct(&m_pPNG, &m_pPNGInfo);
		return UT_ERROR;
	}
//
// Build the png member variables.
//
	_createPNGFromPixbuf(pixbuf);
	FG_GraphicRaster * pFGR = new FG_GraphicRaster();
	if(pFGR == NULL)
	{
		return UT_IE_NOMEMORY;
	}
	if(!pFGR->setRaster_PNG(m_pPngBB)) 
	{
		DELETEP(pFGR);
		
		return UT_IE_FAKETYPE;
	}

	*ppfg = (FG_Graphic *) pFGR;
	return UT_OK;
}

/*!
 * Convert the data contained in the file to into a PNG-based 
 * FG_Graphic type
 */
UT_Error IE_ImpGraphicPixbufGraphic::IE_ImpGraphic(const char * szFilename, FG_Graphic ** ppfg)
{
	//
	// Construct a pixbuf.
	//
	GdkPixbuf * pixbuf = gdk_pixbuf_new_from_file (szFilename);
	if(pixbuf == NULL)
	{
		return UT_ERROR;
	}
		//printf("AbiGdkpixbuf - load from file %s into FG_Grapic * \n",szFilename);

		// Initialize stuff to create our PNG.
	UT_Error err =Initialize_PNG();
	if (err)
	{
		return err;
	}

	if (setjmp(m_pPNG->jmpbuf))
	{
		png_destroy_write_struct(&m_pPNG, &m_pPNGInfo);
		return UT_ERROR;
	}
//
// Build the png member variables.
//
	_createPNGFromPixbuf(pixbuf);
	FG_GraphicRaster * pFGR = new FG_GraphicRaster();
	if(pFGR == NULL)
	{
		return UT_IE_NOMEMORY;
	}
	if(!pFGR->setRaster_PNG(m_pPngBB)) 
	{
		DELETEP(pFGR);
		
		return UT_IE_FAKETYPE;
	}

	*ppfg = (FG_Graphic *) pFGR;
	return UT_OK;
}

/*!
 * Convert an image byte buffer into a PNG byte buffer
 */
UT_Error IE_ImpGraphicPixbufGraphic::convertGraphic(UT_ByteBuf* pBB,
													UT_ByteBuf** ppBB)
{
	GdkPixbuf * pixbuf = NULL;
	GdkPixbufLoader * ldr = NULL;
	if(m_bIsXPM)
	{
		pixbuf = _loadXPM(pBB);
	}
	else
	{
		ldr = gdk_pixbuf_loader_new ();
		UT_ASSERT (ldr);

#ifdef HAVE_GTK_2_0
		// we're GTK+ 2.0 ready :)
		GError * err = g_error_new (G_FILE_ERROR, G_FILE_ERROR_NOENT, "foobar");
	
		gdk_pixbuf_loader_write (ldr, (const guchar *)pBB->getPointer (0),
								 (gsize)pBB->getLength (), &err);
#else
		gdk_pixbuf_loader_write (ldr, (const guchar *)pBB->getPointer (0),
								 (gsize)pBB->getLength ());
#endif
	
		pixbuf = gdk_pixbuf_loader_get_pixbuf (ldr);
	}

	if (!pixbuf)
	{
		return false;
	}

	gdk_pixbuf_ref (pixbuf);

#ifdef HAVE_GTK_2_0
	g_error_free (err);
#endif
	if(ldr)
		gdk_pixbuf_loader_close (ldr);

	UT_Error err =Initialize_PNG();
	if (err)
	{
		return err;
	}

	if (setjmp(m_pPNG->jmpbuf))
	{
		png_destroy_write_struct(&m_pPNG, &m_pPNGInfo);
		return UT_ERROR;
	}
//
// Build the png member variables.
//
	_createPNGFromPixbuf(pixbuf);
	*ppBB =  m_pPngBB;
	return UT_OK;
}


UT_Error	IE_ImpGraphicPixbufGraphic::Initialize_PNG(void)
{
	/* Set up png structures for writing */
	m_pPNG = png_create_write_struct( PNG_LIBPNG_VER_STRING, 
		                              (void*) NULL,
									  NULL, 
									  NULL );
	if( m_pPNG == NULL )
	{
		return UT_ERROR;
	}

	m_pPNGInfo = png_create_info_struct(m_pPNG);
	if ( m_pPNGInfo == NULL )
	{
		png_destroy_write_struct(&m_pPNG, (png_infopp) NULL);
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
	png_set_write_fn(m_pPNG, (void *)m_pPngBB, (png_rw_ptr)_write_png, (png_flush_ptr)_write_flush);

	return UT_OK;
}

// ---------------------------------------------------------------------------

IE_ImpGraphicPixbufGraphic_Sniffer::IE_ImpGraphicPixbufGraphic_Sniffer() 
{
	m_bIsXPM = false;
}

IE_ImpGraphicPixbufGraphic_Sniffer:: ~IE_ImpGraphicPixbufGraphic_Sniffer() 
{
}

/*!
 * Sniff the byte buffer to see if it contains vaild image data recognized
 * by gdk-pixbuf
 */
UT_Confidence_t IE_ImpGraphicPixbufGraphic_Sniffer::recognizeContents(const char * szBuf, UT_uint32 iNum)
{

//
//  Handle xpm differently coz the standard gdk-pixbuf loader scheme doesn't
// work for these!
//
	if((iNum > 9) && (strncmp (szBuf, "/* XPM */", 9) == 0))
	{
		m_bIsXPM = true;
		return UT_CONFIDENCE_PERFECT;
	}
	GdkPixbufLoader * ldr = gdk_pixbuf_loader_new ();
	UT_ASSERT (ldr);
	gdk_pixbuf_loader_write (ldr, (const guchar *) szBuf,(gsize)iNum);
	GdkPixbuf * pixbuf =  gdk_pixbuf_loader_get_pixbuf (ldr);
	if(pixbuf)
	{
		m_bIsXPM = false;
		gdk_pixbuf_loader_close (ldr);
		return UT_CONFIDENCE_PERFECT;
	}
	else
	{
		return UT_CONFIDENCE_ZILCH;
	}
}

UT_Confidence_t IE_ImpGraphicPixbufGraphic_Sniffer::recognizeSuffix(const char * szSuffix)
{
	m_bIsXPM = false;
	if(UT_stricmp(szSuffix,".jpg") == 0)
	{
		return UT_CONFIDENCE_PERFECT;
	}
	else if(UT_stricmp(szSuffix,".jepg") == 0)
	{
		return UT_CONFIDENCE_PERFECT;
	}
	else if(UT_stricmp(szSuffix,".png") == 0)
	{
		return UT_CONFIDENCE_PERFECT;
	}
	else if(UT_stricmp(szSuffix,".tiff") == 0)
	{
		return UT_CONFIDENCE_PERFECT;
	}
	else if(UT_stricmp(szSuffix,".gif") == 0)
	{
		return UT_CONFIDENCE_PERFECT;
	}
	else if(UT_stricmp(szSuffix,".xpm") == 0)
	{
		m_bIsXPM = true;
		return UT_CONFIDENCE_PERFECT;
	}
	else if(UT_stricmp(szSuffix,".pnm") == 0)
	{
		return UT_CONFIDENCE_PERFECT;
	}
	else if(UT_stricmp(szSuffix,".ras") == 0)
	{
		return UT_CONFIDENCE_PERFECT;
	}
	else if(UT_stricmp(szSuffix,".ico") == 0)
	{
		return UT_CONFIDENCE_PERFECT;
	}
	else if(UT_stricmp(szSuffix,".bmp") == 0)
	{
		return UT_CONFIDENCE_PERFECT;
	}
	else if(UT_stricmp(szSuffix,".xbm") == 0)
	{
		return UT_CONFIDENCE_PERFECT;
	}
	return UT_CONFIDENCE_ZILCH;
}

bool IE_ImpGraphicPixbufGraphic_Sniffer::getDlgLabels(const char ** pszDesc,
													  const char ** pszSuffixList,
													  IEGraphicFileType * ft)
{
	// TODO add a more complete list of suffixes
	*pszDesc = "All Gnome images";
	*pszSuffixList = "*.jpg; *.jpeg; *.png; *.tiff; *.gif; *.xpm; *.pnm; *.ras; *.ico; *.bmp; *.xbm";
	*ft = getType ();
	return true;
}

UT_Error IE_ImpGraphicPixbufGraphic_Sniffer::constructImporter(IE_ImpGraphic **ppieg)
{
	*ppieg = new IE_ImpGraphicPixbufGraphic();
	if (*ppieg == NULL)
		return UT_IE_NOMEMORY;
	static_cast<IE_ImpGraphicPixbufGraphic *>( *ppieg)->setXPM(m_bIsXPM);
	m_bIsXPM = false;
	return UT_OK;
}







