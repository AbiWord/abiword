/* AbiSource Application Framework
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

#ifndef XAP_UNIXGNOMEPRINTGRAPHICS_H
#define XAP_UNIXGNOMEPRINTGRAPHICS_H

#include "gr_Graphics.h"

#include "xap_UnixPSFont.h"
#include "xap_UnixFontManager.h"
#include "xap_UnixFont.h"

#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-job.h>
#include <libgnomeprint/gnome-print-paper.h>

// forward declaration
class UT_ByteBuf;

/*****************************************************************/
/*   Take notice that we don't derive from xap_UnixPSGraphics    */
/*****************************************************************/

class XAP_UnixGnomePrintGraphics : public GR_Graphics
{
 public:

	explicit XAP_UnixGnomePrintGraphics(GnomePrintJob *gpm,
										bool isPreview = false);

	virtual ~XAP_UnixGnomePrintGraphics();

	virtual void drawGlyph(UT_uint32 Char, UT_sint32 xoff, UT_sint32 yoff);
	virtual void drawChars(const UT_UCSChar* pChars, 
						   int iCharOffset, int iLength,
						   UT_sint32 xoff, UT_sint32 yoff,
						   int * pCharWidths = NULL);
	virtual void setFont(GR_Font* pFont);

	virtual UT_uint32 getFontAscent();
	virtual UT_uint32 getFontDescent();
	virtual UT_uint32 getFontHeight();
	
	virtual UT_uint32 getFontAscent(GR_Font *);
	virtual UT_uint32 getFontDescent(GR_Font *);
	virtual UT_uint32 getFontHeight(GR_Font *);
	virtual UT_uint32 measureUnRemappedChar(const UT_UCSChar c);
	
	virtual void getCoverage(UT_Vector& coverage);

	virtual void setColor(const UT_RGBColor& clr);
	virtual void getColor(UT_RGBColor& clr);
	virtual GR_Font* findFont(const char* pszFontFamily, 
							  const char* pszFontStyle, 
							  const char* pszFontVariant, 
							  const char* pszFontWeight, 
							  const char* pszFontStretch, 
							  const char* pszFontSize);
	virtual void drawLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2);
	virtual void setLineWidth(UT_sint32);
	virtual void setLineProperties ( double inWidthPixels,
					 JoinStyle inJoinStyle,
					 CapStyle inCapStyle,
					 LineStyle inLineStyle);

	virtual GR_Font* getGUIFont();
	virtual void xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
	virtual void polyLine(UT_Point * pts, UT_uint32 nPoints);
	virtual void fillRect(const UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h);
	virtual void invertRect(const UT_Rect*);
	virtual void setClipRect(const UT_Rect*);
	virtual void scroll(UT_sint32, UT_sint32);
	virtual void scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						UT_sint32 x_src, UT_sint32 y_src,
						UT_sint32 width, UT_sint32 height);
	virtual void clearArea(UT_sint32, UT_sint32, UT_sint32, UT_sint32);

	virtual void drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest);
   	virtual GR_Image* createNewImage(const char* pszName, const UT_ByteBuf* pBBPNG, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, GR_Image::GRType iType);
	
	virtual bool queryProperties(GR_Graphics::Properties gp) const;
	
	virtual bool startPrint(void);
	virtual bool startPage(const char * szPagelabel, UT_uint32 pageNumber,
							  bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight);
	virtual bool endPrint(void);

	virtual void setColorSpace(GR_Graphics::ColorSpace c);
	virtual GR_Graphics::ColorSpace getColorSpace(void) const;
	
	virtual void setCursor(GR_Graphics::Cursor c);
	virtual GR_Graphics::Cursor getCursor(void) const;

	virtual void					setColor3D(GR_Color3D c);
	virtual UT_RGBColor *			getColor3D(GR_Color3D c);
	virtual void fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h);
	virtual void fillRect(GR_Color3D c, UT_Rect &r);
	virtual void setPageSize(char* pageSizeName, UT_uint32 iwidth = 0, UT_uint32 iheight=0);
	virtual void	  saveRectangle(UT_Rect & r, UT_uint32 iIndx) {}
	virtual void	  restoreRectangle(UT_uint32 iIndx) {}

	virtual UT_uint32 getDeviceResolution(void) const;

	static const guchar * s_map_page_size (const char * abi);
	static GnomePrintConfig * s_setup_config (XAP_Frame * pFrame);

protected:

	virtual UT_uint32 _getResolution() const;

private:

	UT_sint32 scale_ydir (UT_sint32 in);

	GnomeFont * _allocGnomeFont(PSFont* pFont);
	void _drawAnyImage (GR_Image* pImg, 
						UT_sint32 xDest, 
						UT_sint32 yDest, bool rgb);
	bool _startDocument(void);
	bool _startPage(const char * szPageLabel);
	bool _endPage(void);
	bool _endDocument(void);

	bool                     m_bIsPreview;
	GnomePrintJob           *m_gpm;
	GnomePrintContext       *m_gpc;
	double                   m_width, m_height;

	// some small variables stolen from UnixPSGraphics
	bool			    m_bStartPrint;
	bool			    m_bStartPage;
	bool	     		m_bNeedStroked;
	double		        m_dLineWidth;

	GR_Graphics::ColorSpace	m_cs;
	UT_RGBColor		        m_currentColor;
	GnomeFont *             m_pCurrentFont;
	PSFont *                m_pCurrentPSFont;

	XAP_UnixFontManager *	m_fm;

	// not implemented, not possible
	XAP_UnixGnomePrintGraphics ();
	XAP_UnixGnomePrintGraphics (const XAP_UnixGnomePrintGraphics & other);
	XAP_UnixGnomePrintGraphics& operator=(const XAP_UnixGnomePrintGraphics & other);
};


#endif /* XAP_UNIXGNOMEPRINTGRAPHCS_H */



