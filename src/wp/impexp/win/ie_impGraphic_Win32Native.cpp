/* -*- c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*- */

/* AbiWord
 *
 * Copyright (C) 2003 Jordi Mas i Hernàndez
 * Copyright (C) 2003 Dom Lachowicz
 * Win32 native plugin based on win32 IPicture interface *  
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
#include <ocidl.h>
#include <olectl.h>
#include "ut_string.h"
#include "ut_bytebuf.h"
#include "fg_GraphicRaster.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "xap_Module.h"
#include "ut_assert.h"

#include "ie_impGraphic_Win32Native.h"

/*******************************************************************/
/*******************************************************************/

/* GDIPLUS interface */
bool isGDIPlusAvailable ();
UT_Error GDIconvertGraphic (UT_ByteBuf * pBB, UT_ByteBuf* pBBOut, std::string& mimetype);
void  shutDownGDIPlus ();



static void _write_png( png_structp png_ptr, 
		        png_bytep data, 
		        png_size_t length )
{
  UT_ByteBuf* bb = static_cast<UT_ByteBuf*>(png_get_io_ptr(png_ptr));
  bb->append(data, length);
}

static void _write_flush(png_structp /*png_ptr*/)
{
  // Empty Function. 
}

//
// Creates a BITMAP file from a handle 
//
static void CreateBMPFile(HWND /*hwnd*/, UT_ByteBuf & pBB, PBITMAPINFO pbi,
			  HBITMAP hBMP, HDC hDC) 
{ 
  BITMAPFILEHEADER hdr;       // bitmap file-header 
  PBITMAPINFOHEADER pbih;     // bitmap info-header 
  LPBYTE lpBits;              // memory pointer 	
  
  if (!hBMP) return;
  
  pbih = (PBITMAPINFOHEADER) pbi; 
  lpBits = (LPBYTE) GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

  if (!lpBits) return;
  
  // Retrieve the color table (RGBQUAD array) and the bits 
  // (array of palette indices) from the DIB. 
  if (!GetDIBits(hDC, hBMP, 0, (WORD) pbih->biHeight, lpBits, pbi, 
		 DIB_RGB_COLORS)) 
    return;
  
  hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M" 
  // Compute the size of the entire file. 
  hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + 
			pbih->biSize + pbih->biClrUsed 
			* sizeof(RGBQUAD) + pbih->biSizeImage); 
  hdr.bfReserved1 = 0; 
  hdr.bfReserved2 = 0; 
  
  // Compute the offset to the array of color indices. 
  hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + 
    pbih->biSize + pbih->biClrUsed 
    * sizeof (RGBQUAD); 
  
  pBB.truncate (0);
  
  // Copy the BITMAPFILEHEADER into the .BMP file. 
  pBB.append ((const UT_Byte *)&hdr, sizeof(BITMAPFILEHEADER));
  pBB.append ((const UT_Byte *)pbih, sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof (RGBQUAD));
  
  // Copy the array of color indices into the .BMP file.         
  pBB.append ((const UT_Byte *)lpBits, (int) pbih->biSizeImage);
  
  GlobalFree((HGLOBAL)lpBits);
}

//
// Creates a Bitmap info struct from a handle
//
static PBITMAPINFO CreateBitmapInfoStruct(HBITMAP hBmp)
{ 
  BITMAP 	bmp; 
  PBITMAPINFO 	pbmi; 
  WORD    	cClrBits; 
  
  // Retrieve the bitmap's color format, width, and height. 
  if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp)) 
    return NULL;
  
  if (bmp.bmBitsPixel==16) 
    bmp.bmBitsPixel = 24;	// 16 bit BMPs are not supported by all programs
  
  // Convert the color format to a count of bits. 
  cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel); 
  
  if (cClrBits == 1) 
    cClrBits = 1; 
  else if (cClrBits <= 4) 
    cClrBits = 4;
  else if (cClrBits <= 8) 
    cClrBits = 8; 
  else if (cClrBits <= 16) 
    cClrBits = 16;
  else if (cClrBits <= 24) 
    cClrBits = 24;
  else cClrBits = 32; 
  
  // Allocate memory for the BITMAPINFO structure. (This structure 
  // contains a BITMAPINFOHEADER structure and an array of RGBQUAD 
  // data structures.) 
  
  if (cClrBits != 24) 
    pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
				    sizeof(BITMAPINFOHEADER) + 
				    sizeof(RGBQUAD) * (1 << cClrBits)); 
  else // There is no RGBQUAD array for the 24-bit-per-pixel format. 	
    pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
				    sizeof(BITMAPINFOHEADER)); 
  
  // Initialize the fields in the BITMAPINFO structure. 
  
  pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
  pbmi->bmiHeader.biWidth = bmp.bmWidth; 
  pbmi->bmiHeader.biHeight = bmp.bmHeight; 
  pbmi->bmiHeader.biPlanes = bmp.bmPlanes; 
  pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel; 
  if (cClrBits < 24) 
    pbmi->bmiHeader.biClrUsed = (1<<cClrBits); 
  
  // If the bitmap is not compressed, set the BI_RGB flag. 
  pbmi->bmiHeader.biCompression = BI_RGB; 
  
  // Compute the number of bytes in the array of color 
  // indices and store the result in biSizeImage. 
  // For Windows NT/2000, the width must be DWORD aligned unless 
  // the bitmap is RLE compressed.
  pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits +31) & ~31) /8
    * pbmi->bmiHeader.biHeight; 
  // Set biClrImportant to 0, indicating that all of the 
  // device colors are important. 
  pbmi->bmiHeader.biClrImportant = 0; 
  return pbmi; 
} 

//  This actually creates our FG_Graphic object for a PNG
UT_Error IE_ImpGraphic_Win32Native::importGraphic(UT_ByteBuf* pBB, 
												  FG_ConstGraphicPtr & pfg)
{
	std::string mimetype;
    UT_Error err = _convertGraphic(pBB, mimetype); 
    if (err != UT_OK) return err;
    
    /* Send Data back to AbiWord as PNG */
    FG_GraphicRasterPtr pFGR(new FG_GraphicRaster);
    
    if(pFGR == NULL)
		return UT_IE_NOMEMORY;
    
	if (mimetype == "image/jpeg")
	{
		if(!pFGR->setRaster_JPEG(m_pBB))
		{
			return UT_IE_FAKETYPE;
		}
	}
	else
	{
		if(!pFGR->setRaster_PNG(m_pBB))
		{
			return UT_IE_FAKETYPE;
		}
	}
    pfg = std::move(pFGR);
    return UT_OK;
}
  
  //
  // Entry point for conversion
  //
UT_Error IE_ImpGraphic_Win32Native::_convertGraphic(UT_ByteBuf * pBB, std::string& mimetype)
{	
    IPicture* pPicture = NULL;
    IStream* stream;	
    HGLOBAL hG;		
    HBITMAP hBitmap;
    OLE_HANDLE* hB;
    PBITMAPINFO	bi;
    UT_ByteBuf bBufBMP;
    UT_Error err;	

	/* If the system has GDI+, use it*/
	if (isGDIPlusAvailable())
	{
		m_pBB = new UT_ByteBuf();
		return GDIconvertGraphic(pBB, m_pBB, mimetype);		
	}

	// the code below always writes out PNG's for now; we could update it to support
	// native JPEG images as well, or just delete it and always use GDI+.
	mimetype = "image/png";

    // We need to store the incoming bytebuffer in a Windows global heap	
    size_t nBlockLen = pBB->getLength();   	
    hG = GlobalAlloc(GPTR, nBlockLen);
    if (!hG) 
		return UT_IE_NOMEMORY;	
    
    CopyMemory(hG, pBB->getPointer(0), nBlockLen);   	
    
    // Create a stream from heap
    HRESULT hr = CreateStreamOnHGlobal(hG,false,&stream);
    if (!SUCCEEDED(hr) || !stream)
	{
		GlobalFree(hG);
		return UT_IE_NOMEMORY;
	}
    
    hr = OleLoadPicture(stream,0,false,IID_IPicture,(void**)&pPicture);	

    stream->Release();
    GlobalFree(hG);
    
    if (!SUCCEEDED(hr) || !pPicture)
	{
		return UT_IE_UNKNOWNTYPE;
	}

    pPicture->get_Handle((unsigned int*)&hB);
    
    hBitmap = (HBITMAP)CopyImage(hB,IMAGE_BITMAP,0,0,LR_COPYRETURNORG);
    
    HWND hWnd = GetDesktopWindow();
    
    // Create a BMP file from a BITMAP	
    bi = CreateBitmapInfoStruct(hBitmap);						
    CreateBMPFile(hWnd, bBufBMP, bi, hBitmap, GetDC(hWnd));
    LocalFree ((HLOCAL)bi);
    
    InitializePrivateClassData();
    
    /* Read Header Data */
    err = Read_BMP_Header(&bBufBMP);
	
	/* 
	   It's not a bitmap, then we have to rendered it into a device
	   context and get a bitmap from there. Case wmf graphics
	*/
	if (err) 
	{
		if (err!=UT_IE_BOGUSDOCUMENT) 
		{
			pPicture->Release();
			return err;		
		}
		
        long nWidth  = 0;
        long nHeight = 0;
		long nScaleToWidth= 500;
		long nScaleToHeight= 500;		
		RECT rc, rect;				
		BYTE *imagedata;
		HBITMAP hBit;
		HBITMAP hOld;
		BITMAPINFO bmi; 		
		HDC hWndDC = GetDC(hWnd);
		HDC	hMemDC = CreateCompatibleDC(hWndDC);
		HBRUSH hBrush = (HBRUSH)GetCurrentObject(hMemDC, OBJ_BRUSH);
		
		pPicture->get_Width (&nWidth);
		pPicture->get_Height(&nHeight);

		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 	
		bmi.bmiHeader.biWidth = nScaleToWidth;
		bmi.bmiHeader.biHeight = nScaleToHeight;
		bmi.bmiHeader.biPlanes = 1; 
		bmi.bmiHeader.biBitCount = 24; // as we want true-color
		bmi.bmiHeader.biCompression = BI_RGB; // no compression
		bmi.bmiHeader.biSizeImage = (((bmi.bmiHeader.biWidth * bmi.bmiHeader.biBitCount + 31) & ~31) >> 3) * bmi.bmiHeader.biHeight; 
		bmi.bmiHeader.biXPelsPerMeter = 0;
		bmi.bmiHeader.biYPelsPerMeter = 0; 
		bmi.bmiHeader.biClrImportant = 0;
		bmi.bmiHeader.biClrUsed = 0; // we are not using palette
			
		hBit = CreateDIBSection(hMemDC,&bmi,DIB_RGB_COLORS,(void**)&imagedata,0,0);						
		hOld = (HBITMAP) SelectObject(hMemDC, hBit);			

		
		rect.left = 0;
        rect.top = nScaleToHeight;
        rect.right = nScaleToWidth;
        rect.bottom = 0;

		FillRect(hMemDC, &rect,  hBrush);
		pPicture->Render(hMemDC, 0,0,  nScaleToWidth,	nScaleToHeight, 0,  nHeight, 
						 nWidth, -nHeight, &rc);	
		
		hBit =  (HBITMAP)SelectObject(hMemDC, hOld);
		
		bi =  CreateBitmapInfoStruct(hBit);						
		CreateBMPFile(hWnd, bBufBMP, &bmi, hBit, hMemDC);
		LocalFree ((HLOCAL)bi);

		
		DeleteDC(hMemDC);
		DeleteDC(hWndDC);
	    DeleteObject(hBrush); 
		DeleteObject(hBit);	  
    
		err = Read_BMP_Header(&bBufBMP);
		if (err) 
		{
			pPicture->Release();
			return err;				
		}
		
	}

	pPicture->Release();


    if ((err = Initialize_PNG()))
	{   
		return err;
	}	
    
    /* Read Palette, if no palette set Header accordingly */
    if(m_iBitsPerPlane < 24) 
	{
		if ((err = Convert_BMP_Palette(&bBufBMP))) 
			return err;
	}
    else
	{
		UT_uint16 bitsPerChannel;
		UT_uint16 colorType;

		if (m_iBitsPerPlane == 24) {
			bitsPerChannel = 8;
			colorType = PNG_COLOR_TYPE_RGB;
		} else if (m_iBitsPerPlane == 32) {
			bitsPerChannel = 8;
			colorType = PNG_COLOR_TYPE_RGB_ALPHA;
		} else if (m_iBitsPerPlane == 48) {
			bitsPerChannel = 16;
			colorType = PNG_COLOR_TYPE_RGB;
		} else if (m_iBitsPerPlane == 64) {
			bitsPerChannel = 16;
			colorType = PNG_COLOR_TYPE_RGB_ALPHA;
		} else {		   
			return UT_ERROR;
		}
	
		png_set_IHDR ( m_pPNG,
					   m_pPNGInfo,
					   m_iWidth,
					   m_iHeight,
					   bitsPerChannel,
					   colorType,
					   PNG_INTERLACE_NONE,
					   PNG_COMPRESSION_TYPE_DEFAULT,
					   PNG_FILTER_TYPE_DEFAULT );
	
	}
    if ((err = Convert_BMP(&bBufBMP))) 
	{
		return err;
	}
    
    /* Clean Up Memory Used */
		
    //FREEP(m_pPNGInfo->palette);
    png_destroy_write_struct(&m_pPNG, &m_pPNGInfo);
    
    return UT_OK;  	  	
}
  
UT_Error IE_ImpGraphic_Win32Native::Read_BMP_Header(UT_ByteBuf* pBB)
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
    m_bOldBMPFormat = (m_iHeaderSize <=12) ? true : false;
    m_iWidth  = (m_bOldBMPFormat) ?
		static_cast<UT_sint32>(Read2Bytes(pBB,m_iBytesRead) ):
		static_cast<UT_sint32>(Read4Bytes(pBB,m_iBytesRead) );
    m_iHeight = (m_bOldBMPFormat) ?
		static_cast<UT_sint32>(Read2Bytes(pBB,m_iBytesRead) ):
		static_cast<UT_sint32>(Read4Bytes(pBB,m_iBytesRead) );
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
    if (m_bHeaderDone) return UT_OK;
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

UT_Error IE_ImpGraphic_Win32Native::Initialize_PNG()
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
    m_pBB = new UT_ByteBuf;  /* Byte Buffer for Converted Data */
    
    /* Setting up the Data Writing Function */
    png_set_write_fn(m_pPNG, static_cast<void *>(m_pBB), static_cast<png_rw_ptr>(_write_png), static_cast<png_flush_ptr>(_write_flush));

    return UT_OK;
}
  
UT_Error IE_ImpGraphic_Win32Native::Convert_BMP_Palette(UT_ByteBuf* pBB)
{
    /* Reset error handling for libpng */
    if (setjmp(png_jmpbuf(m_pPNG)))
	{
		png_destroy_write_struct(&m_pPNG, &m_pPNGInfo);
		return UT_ERROR;
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
    
    png_colorp palette = static_cast<png_colorp>(png_malloc(m_pPNG, numClrs * sizeof(png_color)));
    
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
  
UT_Error IE_ImpGraphic_Win32Native::Convert_BMP(UT_ByteBuf* pBB)
{
    /* Reset error handling for libpng */
	if (setjmp(png_jmpbuf(m_pPNG)))
	{
		png_destroy_write_struct(&m_pPNG, &m_pPNGInfo);
		return UT_ERROR;
	}
    png_write_info(m_pPNG,m_pPNGInfo);
    
    const UT_Byte*  row_data;
    UT_sint32 row;
    UT_uint32 position;
    UT_uint32 row_width = m_iWidth * m_iBitsPerPlane / 8;
    while ((row_width & 3) != 0) row_width++;
    UT_Byte* row_transformed_data = new UT_Byte[row_width];

    switch (m_iBitsPerPlane)
	{
	case 1:
	case 4:
	case 8:
	case 16:
		for (row=m_iHeight-1; row >= 0; row--)
		{
			/* Calculating the start of each row */
			position=m_iOffset + row*row_width;
			row_data = reinterpret_cast<const unsigned char *>(pBB->getPointer(position));
			png_write_rows(m_pPNG,const_cast<png_byte **>(reinterpret_cast<const png_byte **>(&row_data)),1);
		}	
		break;
	case 24:
	case 48:
		for (row=m_iHeight-1; row >= 0; row--)
		{
			/* Calculating the start of each row */
			position=m_iOffset + row*row_width;
			/* Transforming the b/r to r/b */
			for (UT_sint32 i=0, col=0; i < m_iWidth; i++,col+=3)
			{
				row_transformed_data[col+0] = (UT_Byte)*pBB->getPointer(position+col+2);
				row_transformed_data[col+1] = (UT_Byte)*pBB->getPointer(position+col+1);
				row_transformed_data[col+2] = (UT_Byte)*pBB->getPointer(position+col+0);
			}
			png_write_rows(m_pPNG,&row_transformed_data,1);
		}	
		break;
	case 32: 
	case 64:
		for (row=m_iHeight-1; row >= 0; row--)
		{
			/* Calculating the start of each row */
			position=m_iOffset + row*row_width;
			/* Transforming the b/r to r/b */
			for (UT_sint32 i=0, col=0; i < m_iWidth; i++,col+=4)
			{
				row_transformed_data[col+0] = (UT_Byte)*pBB->getPointer(position+col+2);
				row_transformed_data[col+1] = (UT_Byte)*pBB->getPointer(position+col+1);
				row_transformed_data[col+2] = (UT_Byte)*pBB->getPointer(position+col+0);
				row_transformed_data[col+3] = (UT_Byte)*pBB->getPointer(position+col+3);				
			}
			png_write_rows(m_pPNG,&row_transformed_data,1);
		}	
		break;			
	default:
		return UT_IE_BOGUSDOCUMENT;
		break;
	}
    delete [] row_transformed_data;
    
    png_write_end(m_pPNG,m_pPNGInfo);
    return UT_OK;
}


UT_Byte IE_ImpGraphic_Win32Native::ReadByte  (UT_ByteBuf* pBB, 
											  UT_uint32 offset)
{
    return ( static_cast<const UT_Byte>(ReadBytes(pBB,offset,1) ));
}
  
UT_uint16 IE_ImpGraphic_Win32Native::Read2Bytes(UT_ByteBuf* pBB, 
												UT_uint32 offset)
{
    return ( static_cast<const UT_uint16>(ReadBytes(pBB,offset,2) ));
}
    
UT_uint32 IE_ImpGraphic_Win32Native::Read4Bytes(UT_ByteBuf* pBB, 
												UT_uint32 offset)
{
    return ( ReadBytes(pBB,offset,4) );
}  
  
UT_uint32 IE_ImpGraphic_Win32Native::ReadBytes(UT_ByteBuf* pBB, 
											   UT_uint32 offset,
											   UT_uint32 num_bytes)
{
    UT_return_val_if_fail (num_bytes <= 4, 0);
    UT_return_val_if_fail ((m_iBytesRead + num_bytes) <= pBB->getLength(), 0);

    m_iBytesRead+=num_bytes;
    
    if (m_iHeaderSize)
	{
		m_bHeaderDone = (m_iBytesRead >= m_iHeaderSize + 14) ?
			true :
			false;
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
  
void IE_ImpGraphic_Win32Native::InitializePrivateClassData()
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
    m_bOldBMPFormat=false;
    m_bHeaderDone=false;
}


/*******************************************************************/
/*******************************************************************/

// supported suffixes
static IE_SuffixConfidence IE_ImpGraphicWin32Native_Sniffer__SuffixConfidence[] = {
	{ "bmp", 	UT_CONFIDENCE_PERFECT 	},
	{ "emf", 	UT_CONFIDENCE_PERFECT 	},
	{ "gif", 	UT_CONFIDENCE_PERFECT 	},
	{ "ico", 	UT_CONFIDENCE_PERFECT 	},
	{ "jpg", 	UT_CONFIDENCE_PERFECT 	},
	{ "jpeg", 	UT_CONFIDENCE_PERFECT 	},
	{ "wmf", 	UT_CONFIDENCE_PERFECT 	},
	{ "", 	UT_CONFIDENCE_ZILCH 	}
};

// supported mimetypes
static IE_MimeConfidence IE_ImpGraphicWin32_Sniffer__MimeConfidence[] = {
    { IE_MIME_MATCH_FULL,   "image/bmp",        UT_CONFIDENCE_PERFECT  },
	{ IE_MIME_MATCH_FULL,   "image/x-emf",      UT_CONFIDENCE_PERFECT  },
	{ IE_MIME_MATCH_FULL,   "image/gif",        UT_CONFIDENCE_PERFECT  },
    { IE_MIME_MATCH_FULL,   "image/x-icon",     UT_CONFIDENCE_PERFECT  },
    { IE_MIME_MATCH_FULL,   "image/jpeg",       UT_CONFIDENCE_PERFECT  },
    { IE_MIME_MATCH_FULL,   "windows/metafile", UT_CONFIDENCE_PERFECT  },
    { IE_MIME_MATCH_FULL,   "image/x-wmf",      UT_CONFIDENCE_PERFECT  },
    { IE_MIME_MATCH_BOGUS,  "",               UT_CONFIDENCE_ZILCH    }
};

const IE_MimeConfidence * IE_ImpGraphicWin32Native_Sniffer::getMimeConfidence ()
{
	return IE_ImpGraphicWin32_Sniffer__MimeConfidence;
}
 
 
const IE_SuffixConfidence * IE_ImpGraphicWin32Native_Sniffer::getSuffixConfidence ()
{
	return IE_ImpGraphicWin32Native_Sniffer__SuffixConfidence;
}

UT_Confidence_t IE_ImpGraphicWin32Native_Sniffer::recognizeContents (const char * /*szBuf*/,
																	 UT_uint32 /*iNumbytes*/)
{
    return UT_CONFIDENCE_SOSO;
}

bool IE_ImpGraphicWin32Native_Sniffer::getDlgLabels (const char ** pszDesc,
															 const char ** pszSuffixList,
															 IEGraphicFileType * ft)
{
    *pszDesc = "BMP, EMF, GIF, ICO, JPEG, WMF Images";
    *pszSuffixList = "*.bmp; *.emf; *.gif; *.ico; *.jpg; *.jpeg; *.wmf";
    *ft = getType ();
    return true;
}

UT_Error IE_ImpGraphicWin32Native_Sniffer::constructImporter (IE_ImpGraphic ** ppieg)
{
    *ppieg = new IE_ImpGraphic_Win32Native();
    if (*ppieg == NULL)
		return UT_IE_NOMEMORY;
    
    return UT_OK;
}

