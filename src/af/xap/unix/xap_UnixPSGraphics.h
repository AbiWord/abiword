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

#ifndef XAP_UNIXPSGRAPHICS_H
#define XAP_UNIXPSGRAPHICS_H

#include "ut_vector.h"
#include "gr_Graphics.h"
#include "ut_misc.h"

#include "xap_UnixPSFont.h"
#include "xap_UnixPSGenerate.h"

class UT_ByteBuf;
class XAP_UnixFontManager;

/*****************************************************************/
/*****************************************************************/
class PS_GraphicsAllocInfo : public GR_AllocInfo
{
public:
	PS_GraphicsAllocInfo(const char * szFilename,
				const char * szTitle,
				const char * szSoftwareNameAndVersion,
				XAP_UnixFontManager * fontManager,
				bool		 bIsFile,
				XAP_App *app)
		: m_fileName(szFilename),
		  m_title(szTitle),
		  m_softwareName(szSoftwareNameAndVersion),
		  m_fontManager(fontManager),
		  m_isFile(bIsFile), m_app(app) {};

	virtual GR_GraphicsId getType() const {return GRID_UNIX_PS;};
	virtual bool isPrinterGraphics() const {return true; };

	const char * m_fileName;
	const char  * m_title;
	const char * m_softwareName;
	XAP_UnixFontManager * m_fontManager;
	bool m_isFile;
	XAP_App * m_app;
};



class PS_Graphics : public GR_Graphics
{
	// all constructors are protected; instances must be created via
	// GR_GraphicsFactory
public:
	virtual ~PS_Graphics();

	static UT_uint32 s_getClassId() {return GRID_UNIX_PS;}
	virtual UT_uint32 getClassId() {return s_getClassId();}
	
	virtual GR_Capability getCapability() {return GRCAP_PRINTER_ONLY;}
	static const char *    graphicsDescriptor(void) { return "Unix PostScript Graphics";}
	static GR_Graphics *   graphicsAllocator(GR_AllocInfo&);

	virtual void drawGlyph(UT_uint32 Char, double xoff, double yoff);
	virtual void drawChars(const UT_UCSChar* pChars, 
						   int iCharOffset, int iLength,
						   double xoff, double  yoff,
						   int * pCharWidths = NULL);
	virtual void setFont(GR_Font* pFont);
	virtual void clearFont(void) {m_pCurrentFont = NULL;}
	virtual double getFontAscent();
	virtual double getFontDescent();
	virtual double getFontHeight();

	virtual void getCoverage(UT_NumberVector& coverage);
	
	virtual double getFontAscent(GR_Font *);
	virtual double getFontDescent(GR_Font *);
	virtual double getFontHeight(GR_Font *);

	// virtual UT_uint32 measureString(const UT_UCSChar*s, int iOffset, int num, unsigned short* pWidths);
	virtual UT_sint32 measureUnRemappedChar(const UT_UCSChar c);
	
	virtual void setColor(const UT_RGBColor& clr);
      virtual void getColor(UT_RGBColor& clr);
	virtual GR_Font* getGUIFont();

	virtual void drawLine(double x1, double y1, double x2, double y2);
	virtual void setLineWidth(double);
	virtual void setLineProperties ( double inWidthPixels,
					 JoinStyle inJoinStyle,
					 CapStyle inCapStyle,
					 LineStyle inLineStyle);
	virtual void xorLine(double, double, double, double);
	virtual void polyLine(UT_Point * pts, UT_uint32 nPoints);
	virtual void fillRect(const UT_RGBColor& c, double x, double y, double w, double h);
	virtual void invertRect(const UT_Rect*);
	virtual void setClipRect(const UT_Rect*);
	virtual void scroll(double, double);
	virtual void scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						UT_sint32 x_src, UT_sint32 y_src,
						UT_sint32 width, UT_sint32 height);
	virtual void clearArea(double, double, double, double);

	virtual void drawImage(GR_Image* pImg, double xDest, double yDest);
	virtual void drawRGBImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest);
	virtual void drawGrayImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest);
	virtual void drawBWImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest);	
   	virtual GR_Image* createNewImage(const char* pszName, const UT_ByteBuf* pBBPNG, double iWidth, double iHeight, GR_Image::GRType iType);
	
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
	virtual void fillRect(GR_Color3D c, double x, double y, double w, double h);
	virtual void fillRect(GR_Color3D c, UT_Rect &r);
	virtual void setPageSize(char* pageSizeName, UT_uint32 iwidth = 0, UT_uint32 iheight=0);
	virtual void setPageCount(UT_uint32 iCount) { m_iPageCount = iCount;}

    virtual GR_Image * genImageFromRectangle(const UT_Rect & r) { return NULL;}
	virtual void	  saveRectangle(UT_Rect & r, UT_uint32 iIndx) {}
	virtual void	  restoreRectangle(UT_uint32 iIndx) {}


protected:
	// all instances have to be created via GR_GraphicsFactory; see gr_Graphics.h
	/* TODO add other constructors to route to lp/lpr rather than a file */
	PS_Graphics(const char * szFilename,
				const char * szTitle,
				const char * szSoftwareNameAndVersion,
				XAP_UnixFontManager * fontManager,
				bool		 bIsFile,
				XAP_App *pApp);
	
	virtual GR_Font* _findFont(const char* pszFontFamily, 
							   const char* pszFontStyle, 
							   const char* pszFontVariant, 
							   const char* pszFontWeight, 
							   const char* pszFontStretch, 
							   const char* pszFontSize);

	bool			_startDocument(void);
	bool			_startPage(const char * szPageLabel, UT_uint32 pageNumber,
							   bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight);
	bool			_endPage(void);
	bool			_endDocument(void);
	void			_emit_DocumentNeededResources(void);
	void			_emit_IncludeResource(void);
	void			_emit_PrologMacros(void);
	void			_emit_FontMacros(void);
	void			_emit_SetFont(void);
	void                    _emit_SetFont(PSFont *pFont);
	void			_emit_SetLineWidth(void);
	void 			_emit_SetColor(void);
	virtual UT_uint32 getDeviceResolution(void) const;
	
	void 			_drawCharsUTF8(const UT_UCSChar*	pChars,
									UT_uint32			iCharOffset,
								 	UT_uint32			iLength,
								 	UT_sint32 			xoff,
								    UT_sint32 			yoff); 
								 		
	UT_GenericVector<PSFont *>	m_vecFontList;
	PSFont *		m_pCurrentFont;
	UT_RGBColor		m_currentColor;
	ps_Generate *	m_ps;
	const char *	m_szFilename;
	const char *	m_szTitle;
	const char *	m_szSoftwareNameAndVersion;
	bool			m_bStartPrint;
	bool			m_bStartPage;
	bool			m_bNeedStroked;
	bool			m_bIsFile;
	UT_sint32		m_iLineWidth;
	UT_uint32		m_iWidth;
	UT_uint32		m_iHeight;
	char*			m_szPageSizeName;
	UT_uint32       m_iPageCount;
	

	GR_Graphics::ColorSpace	m_cs;
	
	XAP_UnixFontManager *	m_fm;
};

#endif /* XAP_UNIXPSGRAPHICS_H */
