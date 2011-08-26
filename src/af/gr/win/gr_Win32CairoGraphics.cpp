#include "gr_Win32CairoGraphics.h"

#include <cairo.h>
#include <cairo-win32.h>

#include <xap_Win32App.h>
#include <xap_Win32Res_Cursors.rc2>
#include "ut_Win32OS.h"

#include "gr_CairoGraphics.h"
#include "gr_Win32Image.h"
#include "gr_Painter.h"

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
	
	compute_fXYRatio();

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

GR_Graphics *GR_Win32CairoGraphics::graphicsAllocator(GR_AllocInfo& info)
{
	GR_Win32CairoAllocInfo & allocator = static_cast<GR_Win32CairoAllocInfo&>(info);
	return new GR_Win32CairoGraphics(allocator.m_win, allocator.m_double_buffered);
}

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
	// save the current state and reset clip
	cairo_save(m_cr);
	cairo_reset_clip(m_cr);

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
		m_rectangleCache.resize(iIndex + 1);
	m_rectangleCache[iIndex] = cachedRectangle;

	// restore the current state
	cairo_restore(m_cr);
}

void GR_Win32CairoGraphics::restoreRectangle(UT_uint32 iIndex)
{
	// retrieve rectangle & surface from cache
	cairo_rectangle_t& cachedRectangle = m_rectangleCache[iIndex];
	cairo_surface_t* cachedSurface = m_surfaceCache.getNthItem(iIndex);

	cairo_save(m_cr);
	cairo_reset_clip(m_cr);

	// actuall restore
	cairo_set_source_surface(m_cr, cachedSurface, cachedRectangle.x, cachedRectangle.y);
	cairo_paint(m_cr);

	cairo_restore(m_cr);
}

void GR_Win32CairoGraphics::setCursor(GR_Graphics::Cursor c)
{
	m_cursor = c;

	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(XAP_App::getApp());
	HINSTANCE hinst = pWin32App->getInstance();
	LPCTSTR cursor_name;                //TODO : CHECK

	switch (m_cursor)
	{
	case GR_CURSOR_CROSSHAIR:	
		cursor_name = IDC_CROSS;
		hinst = NULL;
		break;

		/*FALLTHRU*/
	case GR_CURSOR_DEFAULT:
		cursor_name = IDC_ARROW;		// top-left arrow
		hinst = NULL;
		break;

	case GR_CURSOR_LINK:
	case GR_CURSOR_GRAB:
#ifndef IDC_HAND
		cursor_name = MAKEINTRESOURCE(IDC_ABIHAND);
#else
		if (UT_IsWin95())
			cursor_name = MAKEINTRESOURCE(IDC_ABIHAND);
		else
		{
			cursor_name = IDC_HAND;
			hinst = NULL;
		}
#endif
		break;

	case GR_CURSOR_EXCHANGE:
		cursor_name = MAKEINTRESOURCE(IDC_EXCHANGE);
		break;

	case GR_CURSOR_IBEAM:
		cursor_name = IDC_IBEAM;
		hinst = NULL;
		break;	

	case GR_CURSOR_RIGHTARROW:
		cursor_name = MAKEINTRESOURCE(IDC_ABIRIGHTARROW);
		break;

	case GR_CURSOR_LEFTARROW:
		cursor_name = IDC_ARROW;
		hinst = NULL;
		break;

	case GR_CURSOR_DOWNARROW:
		cursor_name = MAKEINTRESOURCE(IDC_ABIDOWNARROW);
		break;

	case GR_CURSOR_IMAGE:
		cursor_name = IDC_SIZEALL;
		hinst = NULL;
		break;

	case GR_CURSOR_IMAGESIZE_NW:
	case GR_CURSOR_IMAGESIZE_SE:
		cursor_name = IDC_SIZENWSE;
		hinst = NULL;
		break;

	case GR_CURSOR_HLINE_DRAG:
	case GR_CURSOR_UPDOWN:
	case GR_CURSOR_IMAGESIZE_N:
	case GR_CURSOR_IMAGESIZE_S:
		cursor_name = IDC_SIZENS;
		hinst = NULL;
		break;

	case GR_CURSOR_IMAGESIZE_NE:
	case GR_CURSOR_IMAGESIZE_SW:
		cursor_name = IDC_SIZENESW;
		hinst = NULL;
		break;

	case GR_CURSOR_VLINE_DRAG:
	case GR_CURSOR_LEFTRIGHT:
	case GR_CURSOR_IMAGESIZE_E:
	case GR_CURSOR_IMAGESIZE_W:
		cursor_name = IDC_SIZEWE;
		hinst = NULL;
		break;

	case GR_CURSOR_WAIT:
		cursor_name = IDC_WAIT;
		hinst = NULL;
		break;
	
	default:
		{
			// this assert makes debugging virtuall impossible !!!
			static bool bDoneThisAlready = false;
			if(!bDoneThisAlready)
			{
				bDoneThisAlready = true;
				UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
			}
			
		}
	}

	HCURSOR hCursor = LoadCursor(hinst,cursor_name); //TODO: Leaking resource
	if (hCursor != NULL)
		SetCursor(hCursor);
}

void GR_Win32CairoGraphics::compute_fXYRatio()
{
	HDC hdc = GetDC(m_hwnd);
	
	int nLogPixelsX = GetDeviceCaps(hdc, LOGPIXELSX);
	int nLogPixelsY = GetDeviceCaps(hdc, LOGPIXELSY);
	
	if(nLogPixelsY)
	{
		m_fXYRatio = (double)nLogPixelsX  / (double)nLogPixelsY;
	}
	else
	{
		UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
		m_fXYRatio = 1;
	}
}

void GR_Win32CairoGraphics::scroll(UT_sint32 dx, UT_sint32 dy)
{
	UT_sint32 oldDY = tdu(getPrevYOffset());
	UT_sint32 oldDX = (UT_sint32)((double)tdu(getPrevXOffset()) * m_fXYRatio);
	UT_sint32 newY = getPrevYOffset() + dy;
	UT_sint32 newX = getPrevXOffset() + dx;
	UT_sint32 ddx = -(UT_sint32)((double)(tdu(newX) - oldDX) * m_fXYRatio);
	UT_sint32 ddy = -(tdu(newY) - oldDY);
	setPrevYOffset(newY);
	setPrevXOffset(newX);
	if(ddx == 0 && ddy == 0)
	{
		return;
	}
	GR_Painter caretDisablerPainter(this); // not an elegant way to disable all carets, but it works beautifully - MARCM

	ScrollWindowEx(m_hwnd, ddx, ddy, NULL, NULL, NULL, 0, SW_INVALIDATE);
}

void GR_Win32CairoGraphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest, UT_sint32 x_src, UT_sint32 y_src, UT_sint32 width, UT_sint32 height)
{
	x_dest = (UT_sint32)((double)tdu(x_dest) * m_fXYRatio);
	y_dest = tdu(y_dest);
	x_src = (UT_sint32)((double)tdu(x_src) * m_fXYRatio);
	y_src = tdu(y_src);
	width = (UT_sint32)((double)tdu(width) * m_fXYRatio);
	height = tdu(height);
	RECT r;
	r.left = x_src;
	r.top = y_src;
	r.right = r.left + width;
	r.bottom = r.top + height;

	GR_Painter caretDisablerPainter(this); // not an elegant way to disable all carets, but it works beautifully - MARCM

	ScrollWindowEx(m_hwnd, (x_dest - x_src), (y_dest - y_src), &r, NULL, NULL, NULL, SW_ERASE);
}

GR_Font* GR_Win32CairoGraphics::getGUIFont(void)
{
	return NULL;
}

UT_RGBColor GR_Win32CairoGraphics::translateWinColor(DWORD c)
{
	BYTE R = GetRValue(c);
	BYTE G = GetGValue(c);
	BYTE B = GetBValue(c);
	return UT_RGBColor(
		(unsigned char)R, 
		(unsigned char)G, 
		(unsigned char)B);
}

void GR_Win32CairoGraphics::init3dColors()
{
	m_3dColors[CLR3D_Foreground] = translateWinColor(GetSysColor(COLOR_BTNTEXT));
	m_3dColors[CLR3D_Background] = translateWinColor(GetSysColor(COLOR_3DFACE));
	m_3dColors[CLR3D_BevelUp]    = translateWinColor(GetSysColor(COLOR_3DHIGHLIGHT));
	m_3dColors[CLR3D_BevelDown]  = translateWinColor(GetSysColor(COLOR_3DSHADOW));
	m_3dColors[CLR3D_Highlight]  = translateWinColor(GetSysColor(COLOR_WINDOW));	
}
