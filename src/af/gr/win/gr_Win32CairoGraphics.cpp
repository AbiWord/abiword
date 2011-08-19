#include "gr_Win32CairoGraphics.h"

#include <cairo.h>
#include <cairo-win32.h>

#include "gr_CairoGraphics.h"
#include "gr_Win32Image.h"

GR_Image* GR_Win32CairoGraphicsBase::createNewImage(const char* pszName,
													const UT_ByteBuf* pBB,
                                                    const std::string& mimetype,
													UT_sint32 iWidth,
													UT_sint32 iHeight,
													GR_Image::GRType iType)
{
   	GR_Image* pImg = NULL;

	if (iType == GR_Image::GRT_Raster) 
		pImg = new GR_Win32Image(pszName);
	else if (iType == GR_Image::GRT_Vector) 
		pImg = new GR_VectorImage(pszName);
	else 
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	
	if(pImg) pImg->convertFromBuffer(pBB, mimetype, tdu(iWidth), tdu(iHeight));
   	return pImg;
}

GR_Win32CairoGraphics::GR_Win32CairoGraphics(HWND win, bool bDoubleBuffered)
	: GR_Win32CairoGraphicsBase(),
      m_hwnd(win),
      m_bDoubleBuffered(bDoubleBuffered)
{
	m_cr = NULL;
	setCursor(GR_CURSOR_DEFAULT);	
}

cairo_t *GR_Win32CairoAllocInfo::createCairo()
{
	if(m_win) 
		return GR_Win32CairoGraphics::_createCairo(m_win);
	else 
		return NULL;
}

GR_Win32CairoGraphics::~GR_Win32CairoGraphics()
{

}

cairo_t *GR_Win32CairoGraphics::_createCairo(HWND win)
{
	UT_ASSERT(win);

	HDC dc = GetDC(win);
	cairo_surface_t *surface = cairo_win32_surface_create(dc);
	cairo_t *cr = cairo_create(surface);
	
	return cr;
}

void GR_Win32CairoGraphics::_beginPaint()
{
	GR_CairoGraphics::_beginPaint(); // needed?
	m_cr = _createCairo(m_hwnd);
	_initCairo();
}


void GR_Win32CairoGraphics::_endPaint()
{
	cairo_destroy(m_cr);
	m_cr = NULL;
	GR_CairoGraphics::_endPaint(); // needed?
}

/*

// leaving it commented for now, we need to implement all virtual methods before this,
// otherwise GR_Win32CairoGraphics becomes abstract and cannot be instantiated

GR_Graphics *GR_Win32CairoGraphics::graphicsAllocator(GR_AllocInfo& info)
{
	GR_Win32CairoAllocInfo & allocator = static_cast<GR_Win32CairoAllocInfo&>(info);
	return new GR_Win32CairoGraphics(allocator.m_win, allocator.m_double_buffered);
}
*/

GR_Image * GR_Win32CairoGraphics::genImageFromRectangle(const UT_Rect & r)
{
	// TODO: adapted from Win32Graphics, should get more cairo into this

	// set up the rectangle
	UT_sint32 iX = _tduX(r.left);
	UT_sint32 iY = _tduY(r.top);
	UT_sint32 iWidth = _tduR(r.width);
	UT_sint32 iHeight = _tduR(r.height);
	
	cairo_surface_flush(cairo_get_target(m_cr));
	HDC screenDC = cairo_win32_surface_get_dc(cairo_get_target(m_cr));

	UT_ASSERT(m_cr);

	BITMAPINFO bmi; 
	BYTE *imagedata;
	HDC hMemDC = CreateCompatibleDC(screenDC);
		
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 	
	bmi.bmiHeader.biWidth = iWidth;
	bmi.bmiHeader.biHeight = iHeight;
	bmi.bmiHeader.biPlanes = 1; 
	bmi.bmiHeader.biBitCount = 24; // as we want true-color
	bmi.bmiHeader.biCompression = BI_RGB; // no compression
	bmi.bmiHeader.biSizeImage = (((iWidth * bmi.bmiHeader.biBitCount + 31) & ~31) >> 3) * iHeight; 
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0; 
	bmi.bmiHeader.biClrImportant = 0;
	bmi.bmiHeader.biClrUsed = 0; // we are not using palette
		
	HBITMAP hBit = CreateDIBSection(hMemDC,&bmi,DIB_RGB_COLORS,(void**)&imagedata,0,0);
	
	GdiFlush();

	HBITMAP hOld = (HBITMAP) SelectObject(hMemDC, hBit);
	BitBlt(hMemDC, 0, 0, iWidth, iHeight, screenDC, iX, iY, SRCCOPY);
	hBit =  (HBITMAP)SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);

	GR_Win32Image *img = new GR_Win32Image("Screenshot");
	img->setDIB((BITMAPINFO *)ConvertDDBToDIB(hBit, NULL, BI_RGB, screenDC));
	return img;
}

BITMAPINFO * GR_Win32CairoGraphics::ConvertDDBToDIB(HBITMAP bitmap, HPALETTE hPal, DWORD dwCompression, HDC m_hdc) 
{
	BITMAP				bm;	
	BITMAPINFOHEADER	bi;	
	LPBITMAPINFOHEADER 	lpbi;	
	DWORD				dwLen;
	HANDLE				hDIB;	
	HANDLE				handle;	
	HDC 				hDC;	
	
	if (dwCompression == BI_BITFIELDS)
		return NULL;

	if (hPal == NULL)
		hPal = (HPALETTE)GetStockObject(DEFAULT_PALETTE);	
		
	// Get bitmap information
	::GetObjectW(bitmap, sizeof(bm),(LPSTR)&bm);	
	
	// Initialize the bitmapinfoheader
	bi.biSize			= sizeof(BITMAPINFOHEADER);	
	bi.biWidth			= bm.bmWidth;
	bi.biHeight 		= bm.bmHeight;	
	bi.biPlanes 		= 1;
	bi.biBitCount		= bm.bmPlanes * bm.bmBitsPixel;
	bi.biCompression	= dwCompression;	
	bi.biSizeImage		= 0;	
	bi.biXPelsPerMeter	= 0;
	bi.biYPelsPerMeter	= 0;	
	bi.biClrUsed		= 0;	
	bi.biClrImportant	= 0;
	
	// Compute the size of the  infoheader and the color table
	int nColors = (1 << bi.biBitCount);	
	if( nColors > 256 ) 		
		nColors = 0;
	dwLen  = bi.biSize + nColors * sizeof(RGBQUAD);
	
	// We need a device context to get the DIB from	
	hDC = CreateCompatibleDC(m_hdc);
	hPal = SelectPalette(hDC, hPal, TRUE);	
	RealizePalette(hDC);
	
	// Allocate enough memory to hold bitmapinfoheader and color table
	hDIB = g_try_malloc(dwLen);	
	if (!hDIB){
		SelectPalette(hDC, hPal, TRUE);		
		DeleteDC(hDC);
		return NULL;	
		}
	lpbi = (LPBITMAPINFOHEADER)hDIB;	
	*lpbi = bi;
	
	// Call GetDIBits with a NULL lpBits param, so the device driver 
	// will calculate the biSizeImage field 
	GetDIBits(
		hDC, 
		bitmap, 
		0L, 
		(DWORD)bi.biHeight,
		(LPBYTE)NULL, 
		(LPBITMAPINFO)lpbi, 
		(DWORD)DIB_RGB_COLORS
		);	
	bi = *lpbi;
	
	// If the driver did not fill in the biSizeImage field, then compute it
	// Each scan line of the image is aligned on a DWORD (32bit) boundary
	if (bi.biSizeImage == 0) {
		bi.biSizeImage = ((((bi.biWidth * bi.biBitCount) + 31) & ~31) / 8) * bi.biHeight;
	
		// If a compression scheme is used the result may infact be larger
		// Increase the size to account for this.		
		if (dwCompression != BI_RGB)
			bi.biSizeImage = (bi.biSizeImage * 3) / 2;
		}
	
	// Realloc the buffer so that it can hold all the bits	
	dwLen += bi.biSizeImage;
	handle = g_try_realloc(hDIB, dwLen);
	if (handle)
		hDIB = handle;	
	else{
		g_free(hDIB);		
		// Reselect the original palette
		SelectPalette(hDC,hPal,TRUE);		
		DeleteDC(hDC);
		return NULL;	
		}
	
	// Get the bitmap bits	
	lpbi = (LPBITMAPINFOHEADER)hDIB;	
	
	// FINALLY get the DIB
	BOOL bGotBits = GetDIBits( 
		hDC, 
		bitmap, 
		0L,							// Start scan line				
		(DWORD)bi.biHeight,			// # of scan lines
		(LPBYTE)lpbi 				// address for bitmap bits
		+ (bi.biSize + nColors * sizeof(RGBQUAD)),
		(LPBITMAPINFO)lpbi,			// address of bitmapinfo
		(DWORD)DIB_RGB_COLORS		// Use RGB for color table
		);
	
	if (!bGotBits)	{
		g_free(hDIB);				
		SelectPalette(hDC,hPal,TRUE);		
		DeleteDC(hDC);
		return NULL;	
		}	
		
	SelectPalette(hDC,hPal,TRUE);	
	DeleteDC(hDC);

	return (BITMAPINFO*)hDIB;
}

void GR_Win32CairoGraphics::saveRectangle(UT_Rect & r, UT_uint32 iIndex)
{
	// set up the rectangle
	cairo_rectangle_t cachedRectangle;
	cachedRectangle.x = static_cast<float>(_tduX(r.left));
	cachedRectangle.y = static_cast<float>(_tduY(r.top ));
	cachedRectangle.width  = static_cast<float>(_tduR(r.width ));
	cachedRectangle.height = static_cast<float>(_tduR(r.height));

	// set the new surface and free the old one
	// TODO: should check things before blindly destroying the old surface
	cairo_surface_t *oldCachedSurface = NULL;
	cairo_surface_t *cachedSurface = _getCairoSurfaceFromContext(m_cr, cachedRectangle);
	m_surfaceCache.setNthItem(iIndex, cachedSurface, &oldCachedSurface);
	cairo_surface_destroy(oldCachedSurface);

	// do the same for the rectangle
	if(m_rectangleCache.size() <= iIndex)
		m_rectangleCache.resize(iIndex);
	m_rectangleCache[iIndex] = cachedRectangle;
}

void GR_Win32CairoGraphics::restoreRectangle(UT_uint32 iIndex)
{
	// retrieve rectangle & surface from cache
	cairo_rectangle_t& cachedRectangle = m_rectangleCache[iIndex];
	cairo_surface_t* cachedSurface = m_surfaceCache.getNthItem(iIndex);

	// actuall restore
	cairo_set_source_surface(m_cr, cachedSurface, cachedRectangle.x, cachedRectangle.y);
	cairo_paint(m_cr);
}
