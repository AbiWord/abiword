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
#include "ut_string.h"

#include "ie_impGraphic_BMP.h"
#include "fg_GraphicRaster.h"

static void _write_png( png_structp png_ptr, 
					    png_bytep data, 
			            unsigned int length )
{
	UT_ByteBuf* bb = (UT_ByteBuf*) png_get_io_ptr(png_ptr);
	bb->append(data, length);
}

static void _write_flush(png_structp png_ptr) { } // Empty Fuction.


UT_Bool IE_ImpGraphic_BMP::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".bmp") == 0);
}

UT_Bool IE_ImpGraphic_BMP::RecognizeContents(const char * szBuf, UT_uint32 iNumbytes)
{
	return ( !(strncmp(szBuf, "BM", 2)) );
}

UT_Bool IE_ImpGraphic_BMP::GetDlgLabels(const char ** pszDesc,
									   const char ** pszSuffixList,
									   IEGraphicFileType * ft)
{
	*pszDesc = "Window's Bitmap (.bmp)";
	*pszSuffixList = "*.bmp";
	*ft = IEGFT_BMP;
	return UT_TRUE;
}

UT_Bool IE_ImpGraphic_BMP::SupportsFileType(IEGraphicFileType ft)
{
	return (IEGFT_BMP == ft);
}

UT_Error IE_ImpGraphic_BMP::StaticConstructor(IE_ImpGraphic **ppieg)
{
	*ppieg = new IE_ImpGraphic_BMP();
	if (*ppieg == NULL)
	  return UT_IE_NOMEMORY;

	return UT_OK;
}


//  This actually creates our FG_Graphic object for a PNG
UT_Error IE_ImpGraphic_BMP::importGraphic(UT_ByteBuf* pBB, 
										  FG_Graphic ** ppfg)
{
	UT_Error err;
	InitializePrivateClassData();

	/* Read Header Data */
	if ((err = Read_BMP_Header(pBB))) return err;
	if ((err = Initialize_PNG()))     return err;

	/* Read Palette, if no palette set Header accordingly */
	if(m_iBitsPerPlane < 24) 
	{
		if ((err = Convert_BMP_Pallet(pBB))) return err;
	}
	else
	{
		png_set_IHDR ( m_pPNG,
			           m_pPNGInfo,
					   m_iWidth,
					   m_iHeight,
					   m_iBitsPerPlane,
					   PNG_COLOR_TYPE_RGB,
					   PNG_INTERLACE_NONE,
					   PNG_COMPRESSION_TYPE_DEFAULT,
					   PNG_FILTER_TYPE_DEFAULT );

	}
	if ((err = Convert_BMP(pBB))) return err;

	/* Clean Up Memory Used */
		
	FREEP(m_pPNGInfo->palette);
	DELETEP(pBB);
	png_destroy_write_struct(&m_pPNG, &m_pPNGInfo);

	/* Send Data back to AbiWord as PNG */
	FG_GraphicRaster *pFGR;
	pFGR = new FG_GraphicRaster();

	if(pFGR == NULL)
		return UT_IE_NOMEMORY;

	if(!pFGR->setRaster_PNG(m_pBB)) {
		DELETEP(pFGR);	
		return UT_IE_BOGUSDOCUMENT;
	}

	*ppfg = (FG_Graphic *) pFGR;

	return UT_OK;
}

UT_Error IE_ImpGraphic_BMP::Read_BMP_Header(UT_ByteBuf* pBB)
{
	/* Stepping Through the Header Data first all the File Info
	 * Then the Image Info until reached the end of the image Header Size
	 * Note some simple checks for data out of bounds are included
	 */

	/* File Info Starts Here */
	m_iBytesRead  = 0;
	m_iFileType   = Read2Bytes(pBB,m_iBytesRead);
	 if (m_iFileType != 0x4D42) return UT_IE_BOGUSDOCUMENT;
	m_iFileSize   = Read4Bytes(pBB,m_iBytesRead);
	m_iXHotspot   = Read2Bytes(pBB,m_iBytesRead);
	m_iYHotspot   = Read2Bytes(pBB,m_iBytesRead);		
	m_iOffset     = Read4Bytes(pBB,m_iBytesRead);

	/* Image Info Starts Here */
	m_iHeaderSize = Read4Bytes(pBB,m_iBytesRead);
		if (m_bHeaderDone) return UT_IE_BOGUSDOCUMENT; /* More Header Info Needed */
	m_bOldBMPFormat = (m_iHeaderSize <=12) ? UT_TRUE : UT_FALSE;
	m_iWidth  = (m_bOldBMPFormat) ?
				(UT_sint32) Read2Bytes(pBB,m_iBytesRead) :
	            (UT_sint32) Read4Bytes(pBB,m_iBytesRead) ;
	m_iHeight = (m_bOldBMPFormat) ?
				(UT_sint32) Read2Bytes(pBB,m_iBytesRead) :
	            (UT_sint32) Read4Bytes(pBB,m_iBytesRead) ;
		if (m_bHeaderDone) return UT_IE_BOGUSDOCUMENT; /* More Header Info Needed */
	m_iPlanes		    = Read2Bytes(pBB,m_iBytesRead);
		if (m_bHeaderDone) return UT_IE_BOGUSDOCUMENT; /* More Header Info Needed */
		if (m_iPlanes != 1) return UT_IE_BOGUSDOCUMENT;
	m_iBitsPerPlane     = Read2Bytes(pBB,m_iBytesRead);
		if (m_bHeaderDone) return UT_OK;

	/* This rest of the header is read but not normally required */
	m_iCompression      = Read4Bytes(pBB,m_iBytesRead);
		if (m_iCompression != 0) return UT_IE_BOGUSDOCUMENT;
		if (m_bHeaderDone) return UT_OK;
	m_iImageSize		= Read4Bytes(pBB,m_iBytesRead);
		if(m_bHeaderDone) return UT_OK;
	m_iXResolution		= Read4Bytes(pBB,m_iBytesRead);
		if (m_bHeaderDone) return UT_OK;
	m_iYResolution		= Read4Bytes(pBB,m_iBytesRead);
		if (m_bHeaderDone) return UT_OK;
	m_iClrUsed			= Read4Bytes(pBB,m_iBytesRead);
		if (m_bHeaderDone) return UT_OK;
	m_iClrImportant		= Read4Bytes(pBB,m_iBytesRead);
		if (m_bHeaderDone) return UT_OK;
	m_iResolutionUnits	= Read2Bytes(pBB,m_iBytesRead);
 		if (m_bHeaderDone) return UT_OK;
	m_iPadding			= Read2Bytes(pBB,m_iBytesRead);
		if (m_bHeaderDone) return UT_OK;
	m_iOrigin			= Read2Bytes(pBB,m_iBytesRead);
		if (m_bHeaderDone) return UT_OK;
	m_iHalfToning		= Read2Bytes(pBB,m_iBytesRead);
		if (m_bHeaderDone) return UT_OK;
	m_iHalfToningParam1 = Read4Bytes(pBB,m_iBytesRead);
		if (m_bHeaderDone) return UT_OK;
	m_iHalfToningParam2 = Read4Bytes(pBB,m_iBytesRead);
		if (m_bHeaderDone) return UT_OK;
	m_iClrEncoding		= Read4Bytes(pBB,m_iBytesRead);
		if (m_bHeaderDone) return UT_OK;
	m_iIdentifier		= Read4Bytes(pBB,m_iBytesRead);
		if (m_bHeaderDone) return UT_OK;
	/* Document Using non-standard HeaderSize Assume OK */
	return UT_OK;
}

UT_Error IE_ImpGraphic_BMP::Initialize_PNG()
{
	/* Set up png structures for writing */
	m_pPNG = png_create_write_struct( PNG_LIBPNG_VER_STRING, 
		                              (void*) NULL,
									  NULL, 
									  NULL );
	if( m_pPNG == NULL )
	{
		return UT_FALSE;
	}

	m_pPNGInfo = png_create_info_struct(m_pPNG);
	if ( m_pPNGInfo == NULL )
	{
		png_destroy_write_struct(&m_pPNG, (png_infopp) NULL);
		return UT_FALSE;
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
		return UT_FALSE;
	}
	m_pBB = new UT_ByteBuf;  /* Byte Buffer for Converted Data */

	/* Setting up the Data Writing Function */
	png_set_write_fn(m_pPNG, (void *)m_pBB, _write_png, _write_flush);

	return UT_OK;
}

UT_Error IE_ImpGraphic_BMP::Convert_BMP_Pallet(UT_ByteBuf* pBB)
{
	/* Reset error handling for libpng */
	if (setjmp(m_pPNG->jmpbuf))
	{
		png_destroy_write_struct(&m_pPNG, &m_pPNGInfo);
		return UT_FALSE;
	}

	png_set_IHDR ( m_pPNG,
		           m_pPNGInfo,
				   m_iWidth,
				   m_iHeight,
				   m_iBitsPerPlane,
				   PNG_COLOR_TYPE_PALETTE,
				   PNG_INTERLACE_NONE,
				   PNG_COMPRESSION_TYPE_DEFAULT,
				   PNG_FILTER_TYPE_DEFAULT );

	UT_uint32 iOffset = m_iHeaderSize + 14;
	UT_uint32 numClrs = (m_iClrUsed > 0) ?
                        m_iClrUsed :
		                (m_iOffset - iOffset)/((m_bOldBMPFormat)?3:4);

	png_colorp palette = (png_colorp)png_malloc(m_pPNG, numClrs * sizeof(png_color));

	for (UT_uint32 i=0; i < numClrs; i++)
	{
		palette[i].blue  = ReadByte(pBB,iOffset++);
		palette[i].green = ReadByte(pBB,iOffset++);
		palette[i].red   = ReadByte(pBB,iOffset++);
		if(!m_bOldBMPFormat) iOffset++;
	}
	if (iOffset > m_iOffset) return UT_IE_BOGUSDOCUMENT;

	png_set_PLTE( m_pPNG, m_pPNGInfo, palette, numClrs );

	return UT_OK;
}

UT_Error IE_ImpGraphic_BMP::Convert_BMP(UT_ByteBuf* pBB)
{
	/* Reset error handling for libpng */
	if (setjmp(m_pPNG->jmpbuf))
	{
		png_destroy_write_struct(&m_pPNG, &m_pPNGInfo);
		return UT_FALSE;
	}
	png_write_info(m_pPNG,m_pPNGInfo);

	UT_Byte*  row_data;
	UT_sint32 row;
	UT_uint32 position;
	UT_uint32 row_width = m_iWidth;
	while ((row_width & 3) != 0) row_width++;

	switch (m_iBitsPerPlane)
	{
	case 1:
	case 4:
	case 8:
	case 16:
	case 24:
		for (row=m_iHeight-1; row >= 0; row--)
		{
			/* Calculatin the start of each row */
			position=m_iOffset + row*row_width*m_iBitsPerPlane/8;
			row_data = (unsigned char *) pBB->getPointer(position);
			png_write_rows(m_pPNG,&row_data,1);
		}	
		break;
	default:
		return UT_IE_BOGUSDOCUMENT;
		break;
	}

	png_write_end(m_pPNG,m_pPNGInfo);
	return UT_OK;
}

UT_Byte IE_ImpGraphic_BMP::ReadByte  (UT_ByteBuf* pBB, 
									    UT_uint32 offset)
{
	return ( (UT_Byte)  ReadBytes(pBB,offset,1) );
}

UT_uint16 IE_ImpGraphic_BMP::Read2Bytes(UT_ByteBuf* pBB, 
									    UT_uint32 offset)
{
	return ( (UT_uint16) ReadBytes(pBB,offset,2) );
}


UT_uint32 IE_ImpGraphic_BMP::Read4Bytes(UT_ByteBuf* pBB, 
									    UT_uint32 offset)
{
	return ( ReadBytes(pBB,offset,4) );
}


UT_uint32 IE_ImpGraphic_BMP::ReadBytes(UT_ByteBuf* pBB, 
									   UT_uint32 offset,
									   UT_uint32 num_bytes)
{
	UT_ASSERT(num_bytes <= 4);
	m_iBytesRead+=num_bytes;

	if (m_iHeaderSize)
	{
		m_bHeaderDone = (m_iBytesRead >= m_iHeaderSize + 14) ?
		                UT_TRUE :
	                    UT_FALSE;
	}

	UT_uint32 result = 0;
	const UT_Byte*  pByte;
	for (UT_uint32 i=0; i<num_bytes; i++)
	{
		pByte   =  pBB->getPointer(offset+i);
		result |=  *pByte << (i*8);
	}
	return (result);
}

void IE_ImpGraphic_BMP::InitializePrivateClassData()
{
	m_iFileType=0;
	m_iFileSize=0;
	m_iXHotspot=0;
	m_iYHotspot=0;
	m_iOffset=0;
	m_iHeaderSize=0;	
	m_iWidth=0;			
	m_iHeight=0;		
	m_iPlanes=0;		
	m_iBitsPerPlane=0;	
	m_iCompression=0;	
	m_iImageSize=0;		
	m_iXResolution=0;	
	m_iYResolution=0;	
	m_iClrUsed=0;		
	m_iClrImportant=0;	
	m_iResolutionUnits=0;
	m_iPadding=0;		
	m_iOrigin=0;		
	m_iHalfToning=0;	
	m_iHalfToningParam1=0;
	m_iHalfToningParam2=0;
	m_iClrEncoding=0;	
	m_iIdentifier=0;	
	m_iBytesRead=0;		
	m_bOldBMPFormat=UT_FALSE;
	m_bHeaderDone=UT_FALSE;
}
