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

#include "ut_vector.h"
#include "gr_Graphics.h"
#include "ut_misc.h"

#include "xap_UnixPSFont.h"
#include "xap_UnixFontManager.h"

// hack
extern "C" {
#include <gnome.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-master.h>
#include <libgnomeprint/gnome-font.h>
#include <libgnomeprint/gnome-font-face.h>
}

// forward declaration
class UT_ByteBuf;

/*****************************************************************/
/*   Take notice that we don't derive from xap_UnixPSGraphics    */
/*****************************************************************/

class XAP_UnixGnomePrintGraphics : public GR_Graphics
{
 public:

	XAP_UnixGnomePrintGraphics(GnomePrintMaster *gpm,
				   const char * pageSize,
				   XAP_UnixFontManager * fontManager,
				   XAP_App *pApp, bool isPreview);

	virtual ~XAP_UnixGnomePrintGraphics();

	virtual void drawChars(const UT_UCSChar* pChars, 
			       int iCharOffset, int iLength,
			       UT_sint32 xoff, UT_sint32 yoff);
	virtual void setFont(GR_Font* pFont);

	virtual UT_uint32 getFontAscent();
	virtual UT_uint32 getFontDescent();
	virtual UT_uint32 getFontHeight();
	
	virtual UT_uint32 getFontAscent(GR_Font *);
	virtual UT_uint32 getFontDescent(GR_Font *);
	virtual UT_uint32 getFontHeight(GR_Font *);

	virtual UT_uint32 measureUnRemappedChar(const UT_UCSChar c);
	
	virtual void setColor(UT_RGBColor& clr);
	virtual GR_Font* getGUIFont();
	virtual GR_Font* findFont(const char* pszFontFamily, 
				  const char* pszFontStyle, 
				  const char* pszFontVariant, 
				  const char* pszFontWeight, 
				  const char* pszFontStretch, 
				  const char* pszFontSize);
	virtual void drawLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2);
	virtual void setLineWidth(UT_sint32);
	virtual void xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
	virtual void polyLine(UT_Point * pts, UT_uint32 nPoints);
	virtual void fillRect(UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h);
	virtual void fillRect(UT_RGBColor& c, UT_Rect &r);
	virtual void invertRect(const UT_Rect*);
	virtual void setClipRect(const UT_Rect*);
	virtual void scroll(UT_sint32, UT_sint32, XAP_Frame *);
	virtual void scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						UT_sint32 x_src, UT_sint32 y_src,
						UT_sint32 width, UT_sint32 height);
	virtual void clearArea(UT_sint32, UT_sint32, UT_sint32, UT_sint32);

	virtual void drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest);
	virtual void drawAnyImage (GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest, bool rgb);
   	virtual GR_Image* createNewImage(const char* pszName, const UT_ByteBuf* pBBPNG, 
					 UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, 
					 GR_Image::GRType iType);
	
	virtual bool queryProperties(GR_Graphics::Properties gp) const;
	
	virtual bool startPrint(void);
	virtual bool startPage(const char * szPagelabel);
	virtual bool startPage (const char *szPageLabel, 
				UT_uint32, bool, 
				UT_uint32, UT_uint32);
	virtual bool endPrint();

	virtual void setColorSpace(GR_Graphics::ColorSpace c);
	virtual GR_Graphics::ColorSpace getColorSpace(void) const;
	
	virtual void setCursor(GR_Graphics::Cursor c);
	virtual GR_Graphics::Cursor getCursor(void) const;

	virtual void					setColor3D(GR_Color3D c);
	virtual UT_RGBColor *			getColor3D(GR_Color3D c);
	virtual void fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h);
	virtual void fillRect(GR_Color3D c, UT_Rect &r);

 protected:

	bool			_startDocument(void);
	bool			_startPage(const char * szPageLabel);
	bool			_endPage(void);
	bool			_endDocument(void);
	virtual UT_uint32       _getResolution(void) const;

	GnomeFont * _allocGnomeFont(PSFont* pFont);
	
	// if we need to rotate a page for landscape mode, this'll do it
	void _setup_rotation (void);

	double _get_height (void);

	/* ugly scaling functions */	
	double _scale_factor_get(void);
	double _scale_factor_get_inverse(void);
	double _scale_x_dir(int x);
	double _scale_y_dir(int y);

	bool                 m_bIsPreview;
	GnomePrintMaster        *m_gpm;
	GnomePrintContext       *m_gpc;
	const GnomePaper        *m_paper;

	// some small variables stolen from UnixPSGraphics
	bool			m_bStartPrint;
	bool			m_bStartPage;
	bool			m_bNeedStroked;
	double		        m_dLineWidth;

	GR_Graphics::ColorSpace	m_cs;
	UT_RGBColor		m_currentColor;

	// temporary
	GnomeFont *m_pCurrentFont;

	XAP_UnixFontManager *	m_fm;
};


#endif /* XAP_UNIXGNOMEPRINTGRAPHCS_H */
