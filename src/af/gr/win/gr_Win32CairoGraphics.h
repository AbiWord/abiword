#ifndef _GR_WIN32CAIROGRAPHICS_H_
#define _GR_WIN32CAIROGRAPHICS_H_

#include <windows.h>

#include "gr_CairoGraphics.h"

class ABI_EXPORT GR_Win32CairoAllocInfo 
	: public GR_CairoAllocInfo
{
public:
	GR_Win32CairoAllocInfo(HWND win, bool bDoubleBuffered = false)
		: GR_CairoAllocInfo(false, false, bDoubleBuffered),
		  m_win(win)
	{ }

	GR_Win32CairoAllocInfo(bool bPreview)
		: GR_CairoAllocInfo(bPreview, true, false),
		  m_win(0)
	{ }
	
	virtual cairo_t *createCairo();

	HWND m_win;
};

class ABI_EXPORT GR_Win32CairoGraphicsBase 
	: public GR_CairoGraphics
{
public:
	virtual GR_Image* createNewImage(const char* pszName,
									 const UT_ByteBuf* pBB,
	                                 const std::string & mimeType,
									 UT_sint32 iDisplayWidth,
									 UT_sint32 iDisplayHeight,
									 GR_Image::GRType = GR_Image::GRT_Raster);
protected:
	GR_Win32CairoGraphicsBase() 
		: GR_CairoGraphics()
	{ }

	GR_Win32CairoGraphicsBase(cairo_t *cr, UT_uint32 iDeviceResolution)
		: GR_CairoGraphics(cr, iDeviceResolution)
	{ }
};

class ABI_EXPORT GR_Win32CairoGraphics 
	: public GR_Win32CairoGraphicsBase
{
public:
	~GR_Win32CairoGraphics();
	static UT_uint32       s_getClassId() { return GRID_WIN32_PANGO; }
	virtual UT_uint32      getClassId() { return s_getClassId(); }

	static const char *    graphicsDescriptor() { return "Win32 Cairo Pango"; }
	static GR_Graphics *   graphicsAllocator(GR_AllocInfo&);

	HWND getWindow () { return m_hwnd; }

	virtual void           setCursor(GR_Graphics::Cursor c);

	virtual void		   scroll(UT_sint32, UT_sint32);
	virtual void		   scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						   UT_sint32 x_src, UT_sint32 y_src,
						   UT_sint32 width, UT_sint32 height);
	virtual void		   saveRectangle(UT_Rect & r, UT_uint32 iIndx);
	virtual void		   restoreRectangle(UT_uint32 iIndx);
	virtual GR_Image *	   genImageFromRectangle(const UT_Rect & r);

	virtual void _beginPaint();
	virtual void _endPaint();

protected:
	GR_Win32CairoGraphics(HWND win = 0, bool double_buffered = false);

private:
	HWND m_hwnd;
	bool m_bDoubleBuffered;

};

#endif
