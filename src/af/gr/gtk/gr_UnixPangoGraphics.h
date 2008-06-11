/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2004-6 Tomas Frydrych <dr.tomas@yahoo.co.uk>
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

#ifndef GR_UNIXPANGOGRAPHICS_H
#define GR_UNIXPANGOGRAPHICS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vector>

#include "ut_types.h"
#include "ut_string_class.h"
#include "gr_RenderInfo.h"

#include <pango/pango.h>
#include <gtk/gtk.h>

#ifdef ENABLE_PRINT
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-job.h>
#endif

class GR_PangoRenderInfo;
class GR_CairoGraphics;
class XAP_Frame;

class ABI_EXPORT GR_UnixCairoScreenGraphics : public GR_ScreenGraphics
{
protected:
	GdkWindow					*m_pWin;
	GR_ScreenGraphics::Cursor	 m_cursor;
	GdkColor					 m_3dColors[COUNT_3D_COLORS];

public:
	static const char *    graphicsDescriptor() { return "Unix Pango"; }
	static UT_uint32       s_getClassId() { return GRID_UNIX_PANGO; }
	virtual UT_uint32      getClassId() { return s_getClassId(); }

	GR_UnixCairoScreenGraphics(GdkWindow *win);
	~GR_UnixCairoScreenGraphics();

	virtual operator GR_Graphics * () { return this; }

	void                createPixmapFromXPM(char ** pXPM,GdkPixmap *source,
											GdkBitmap * mask);

	virtual void		fillRect(GR_Color3D c,
								 UT_sint32 x, UT_sint32 y,
								 UT_sint32 w, UT_sint32 h);
	virtual void		fillRect(GR_Color3D c, UT_Rect &r);   

	void				init3dColors(GtkStyle * pStyle);

	virtual void		scroll(UT_sint32, UT_sint32);
	virtual void		scroll(UT_sint32 x_dest, UT_sint32 y_dest,
							   UT_sint32 x_src, UT_sint32 y_src,
							   UT_sint32 width, UT_sint32 height);

	virtual bool		getColor3D(GR_Color3D name, UT_RGBColor &color);
	virtual void		setColor3D(GR_Color3D c);

	virtual void		setCursor(GR_ScreenGraphics::Cursor c);
	virtual GR_ScreenGraphics::Cursor getCursor() const;
};

#ifdef ENABLE_PRINT

/*!
    When printing, we need to combine pango with GnomePrint;
    we could do that in a single graphics class, but that would mean
    if(print) test inside each function. In order to avoid slowing the screen
    operations, we will use a derrived class.
*/
class ABI_EXPORT GR_UnixPangoPrintGraphics : public GR_CairoGraphics
{
	friend class GR_PangoFont;
  public:
	
	GR_UnixPangoPrintGraphics(GnomePrintJob *gpm, bool isPreview = false);
	GR_UnixPangoPrintGraphics(GnomePrintContext *ctx, double inWidthDevice,
							  double inHeightDevice);
	
	virtual ~GR_UnixPangoPrintGraphics();

	GnomePrintContext *    getGnomePrintContext() const;
	UT_sint32              scale_ydir (UT_sint32 in) const;
	UT_sint32              scale_xdir (UT_sint32 in) const;
	virtual void           setColor(const UT_RGBColor& clr);
	virtual void           getColor(UT_RGBColor& clr);
	
	virtual void drawChars(const UT_UCSChar* pChars, 
						   int iCharOffset, int iLength,
						   UT_sint32 xoff, UT_sint32 yoff,
						   int * pCharWidths = NULL);
	
	virtual bool shape(GR_ShapingInfo & si, GR_RenderInfo *& ri);
	virtual void renderChars(GR_RenderInfo & ri);

	virtual void drawLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2);
	virtual void setLineWidth(UT_sint32);
	virtual void setLineProperties ( double inWidthPixels,
									 JoinStyle inJoinStyle,
									 CapStyle inCapStyle,
									 LineStyle inLineStyle);

	virtual void fillRect(const UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h);
	virtual void setClipRect(const UT_Rect*);

	virtual void drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest);
   	virtual GR_Image* createNewImage(const char* pszName, const UT_ByteBuf* pBBPNG, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, GR_Image::GRType iType);
	
	virtual bool queryProperties(GR_Graphics::Properties gp) const;
	
	virtual bool startPrint();
	virtual bool startPage(const char * szPagelabel, UT_uint32 pageNumber,
							  bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight);
	virtual bool endPrint();

	virtual void setColorSpace(GR_Graphics::ColorSpace c);
	virtual GR_Graphics::ColorSpace getColorSpace() const;

    virtual GR_Image * genImageFromRectangle(const UT_Rect & /*r*/) { return NULL;}
	virtual void	  saveRectangle(UT_Rect & /*r*/, UT_uint32 /*iIndx*/) {}
	virtual void	  restoreRectangle(UT_uint32 /*iIndx*/) {}

	virtual UT_uint32 getDeviceResolution() const;
	virtual bool      canQuickPrint()
	{ return true;}
	virtual UT_uint32 getFontAscent();
	virtual UT_uint32 getFontDescent();
	virtual UT_uint32 getFontHeight();
	
	virtual UT_uint32 getFontAscent(const GR_Font *);
	virtual UT_uint32 getFontDescent(const GR_Font *);
	virtual UT_uint32 getFontHeight(const GR_Font *);
	virtual double    getResolutionRatio()
	{ return _getResolutionRatio();}
	GnomePrintContext * getGnomePrintContext() { return m_gpc;}

	static GnomePrintConfig * s_setup_config (double mrgnTop,
											  double mrgnBottom,
											  double mrgnLeft,
											  double mrgnRight,
											  double width, double height,
											  int copies, bool portrait);

	static void s_setup_config (GnomePrintConfig *cfg,
								double mrgnTop,
								double mrgnBottom,
								double mrgnLeft,
								double mrgnRight,
								double width, double height,
								int copies, bool portrait);
	
	void   setPdfWorkaround()
	{ m_bPdfLandscapeWorkaround = true;}

  protected:
	double  _getResolutionRatio()
	{
		return m_dResRatio;
	}

	UT_uint32 _getResolution() const;
	void      _drawAnyImage (GR_Image* pImg, UT_sint32 xDest,
							 UT_sint32 yDest, bool rgb);
	bool      _startDocument();
	bool      _startPage(const char * szPageLabel);
	bool      _endPage();
	bool      _endDocument();

  private:
	void      _constructorCommon ();
	
	PangoFontMap *    m_pGPFontMap;
	PangoContext *    m_pGPContext;
	UT_uint32         m_iScreenResolution;
	double            m_dResRatio;

	bool              m_bIsPreview;
	bool			  m_bStartPrint;
	bool			  m_bStartPage;
	bool	     	  m_bNeedStroked;
	double		      m_dLineWidth;
	
	GnomePrintJob     *m_gpm;
	GnomePrintContext *m_gpc;
	double             m_width, m_height;
	bool               m_bPdfLandscapeWorkaround;
	GR_Graphics::ColorSpace	m_cs;
	bool               m_bOwnsFontMap;
};
#endif // ifdef ENABLE_PRINT

#endif
