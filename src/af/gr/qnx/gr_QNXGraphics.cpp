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
#include <string.h>
#include <stdio.h>
#include <photon/Pf.h>

#include "xap_QNXApp.h"
#include "gr_QNXGraphics.h"
#include "gr_QNXImage.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_misc.h"
#include "ut_string.h"

#define min(x, y) (((x) > (y)) ? (y) : (x))

#define  MY_ABS(x) 	(((x) < 0) ? -(x) : (x))
/* NOTE: Having to do all of this set up here is a real
         pain, but since we can be drawing outside of an
         expose context, it is just what we have to do.
*/
#define DRAW_START DrawSetup();
#define DRAW_END   DrawTeardown();

/* Global font cache */
struct _fcache {
	struct _fcache *next;
	struct _fcache *prev;
	char 		   name[36];
} *g_pFCache = NULL;	


/***
 Initialization/Teardown for drawing on a widget outside of the normal 
 stream of "damage" events
***/
int GR_QNXGraphics::DrawSetup() {

	//Don't do any of this if you are printing
	if (m_pPrintContext) {
		return 0;
	}

	//Set the region and the draw offset
	PgSetRegion(PtWidgetRid(PtFindDisjoint(m_pDraw)));	
	PtWidgetOffset(m_pDraw, &m_OffsetPoint);					
	PgSetTranslation (&m_OffsetPoint, 0 /* Pg_RELATIVE */);	

	//Always clip to the canvas
	PhRect_t _rdraw;									
	PtBasicWidgetCanvas(m_pDraw, &_rdraw);				
/*
	printf("Widget Rect %d,%d %d,%d \n", 
		_rdraw.ul.x, _rdraw.ul.y, _rdraw.lr.x, _rdraw.lr.y);
*/

	//Add additional user clipping areas (only one for now)
	if (m_pClipList) {
/* This doesn't work right ... so just do the intersect ourselves
		printf("Add Clip Rect %d,%d %d,%d \n", 
			m_pClipList->rect.ul.x, m_pClipList->rect.ul.y, 
			m_pClipList->rect.lr.x, m_pClipList->rect.lr.y);
		PtClipAdd(m_pDraw, &m_pClipList->rect);
*/
		//Instead use this
		if (PtRectIntersect(&_rdraw, &m_pClipList->rect) == 0) {
			//This should never happen!
			UT_DEBUGMSG(("No intersection of widget %d,%d %d,%d ", 
						  _rdraw.ul.x, _rdraw.ul.y, _rdraw.lr.x, _rdraw.lr.y));
			UT_DEBUGMSG(("and clip list %d,%d %d,%d ",
						  m_pClipList->rect.ul.x, m_pClipList->rect.ul.y, 
						  m_pClipList->rect.lr.x, m_pClipList->rect.lr.y));
			//Instead set a 0,0 area
			//PtBasicWidgetCanvas(m_pDraw, &_rdraw);				
			memset(&_rdraw, 0, sizeof(_rdraw));				
		}
	}

	PgSetUserClip(&_rdraw); 
	return 0;
}

int GR_QNXGraphics::DrawTeardown() {

	//Don't do any of this if you are printing
	if (m_pPrintContext) {
		return 0;
	}
	
	//Remove the clipping (only one for now)
	PgSetUserClip(NULL); 
	
	//Reset the translation
	m_OffsetPoint.x *= -1; 							
	m_OffsetPoint.y *= -1; 									
	PgSetTranslation(&m_OffsetPoint, 0); 						

	return 0;
}


GR_QNXGraphics::GR_QNXGraphics(PtWidget_t * win, PtWidget_t * draw, XAP_App *app)
{
	m_pApp = app;
	m_pWin = win;
	m_pDraw = draw;
	m_pFont = NULL;
	m_pFontGUI = NULL;
	m_pClipList = NULL;
	m_iLineWidth = 1;
	m_currentColor = Pg_BLACK;
	m_pPrintContext = NULL;
	m_iAscentCache = m_iDescentCache = -1;

	m_cs = GR_Graphics::GR_COLORSPACE_COLOR;
	m_cursor = GR_CURSOR_INVALID;
	setCursor(GR_CURSOR_DEFAULT);
	init3dColors();
}

GR_QNXGraphics::~GR_QNXGraphics()
{
	DELETEP(m_pFontGUI);
}

/***
 Miscellaneous functions 
***/
bool GR_QNXGraphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
	{
	case DGP_SCREEN:
		return (m_pPrintContext) ? false : true;
	case DGP_PAPER:
		return (m_pPrintContext) ? true : false;
	case DGP_OPAQUEOVERLAY:
		return true;
	default:
		UT_ASSERT(0);
		return false;
	}
}

UT_uint32 GR_QNXGraphics::_getResolution(void) const
{
	/*Unix uses a fixed value of 100dpi for this value,
      Windows does a call to get the context resolution.
      Photon old fonts were 72dpi, the new ones are 96dpi
      so we will use that.

      In reality we should use a value which doesn't change 
      and is based on something other than the changing font
      values.  Also should something different be used for
      the printing values vs the screen values?
	*/

	/*
	return 96;
	*/
	return 96;
}

void GR_QNXGraphics::flush(void)
{
	PgFlush();
}

/***
 Character Operations 
***/
/*
 This function gets called a lot ... ideally it should be as fast as
possible, but due to the fact that we call it outside of the normal 
draw stream, we can't always assume that other things are properly 
set (ie font, colour etc) so we end up doing it again ourselves. It
also licks that we have to do the translation at this layer.
*/
void GR_QNXGraphics::drawChars(const UT_UCSChar* pChars, int iCharOffset,
				 int iLength, UT_sint32 xoff, UT_sint32 yoff)
{
	PhPoint_t pos;
	int i;
	
	pos.x = xoff; 
	pos.y = yoff + getFontAscent();

	DRAW_START

	PgSetFont(m_pFont->getFont());
	PgSetTextColor(m_currentColor);

	/* We have no idea in advance how big this result is going 
	   to convert into, so we will guestimate with the MB_CUR_MAX,
       which in theory is the max number of characters a wc will expand to.
	*/
	char *pNChars, utb[MB_CUR_MAX * 150];  // arbitrary biggish size for utb
	int  blen, ipos;

	if ((iLength * MB_CUR_MAX) < (int)(sizeof(utb) / sizeof(utb[0]))) {
		blen = sizeof(utb) / sizeof(utb[0]);
		pNChars = utb;
	} else {
		blen = iLength * MB_CUR_MAX;
		pNChars = new char[blen];
	}

	for (i = ipos = 0; i < iLength; i++) {
		int tlen;

		UT_ASSERT((ipos + MB_CUR_MAX) < blen);
		tlen = wctomb(&pNChars[ipos], remapGlyph(pChars[i + iCharOffset], false));
		UT_ASSERT(tlen > 0);
		ipos += tlen;
	}

	//Faster to copy and not flush or to not copy and flush?
/*
	Each flush causes two messages to be sent.  Since AbiWord is stupid about sending
    entire runs of text, we don't do this but do the copy instead if it is required.  
	PgDrawTextmx(pNChars, ipos, &pos, 0);
	PgFlush();
*/
	PgDrawText(pNChars, ipos, &pos, 0);

	if (pNChars != utb) {
		delete [] pNChars;
	}

	DRAW_END
}

void GR_QNXGraphics::drawChar(UT_UCSChar Char, UT_sint32 xoff, UT_sint32 yoff)
{
	drawChars(&Char, 0, 1, xoff, yoff);
}

/*
 This function is also called a ton of times.  We need to make it 
 as fast a possible.
*/
UT_uint32 GR_QNXGraphics::measureUnRemappedChar(const UT_UCSChar c)
{
	const char *font;
	FontRender metrics;
if (!m_pFont || !(font = m_pFont->getFont())) {
		return 0;
	}
    PfGlyph(font,c,&metrics,NULL,NULL,NULL);
    return metrics.width;
}

/*** 
 Font Operations 
***/
GR_Font * GR_QNXGraphics::getGUIFont(void)
{
	if (!m_pFontGUI) {
		m_pFontGUI = new QNXFont("helv10");
		UT_ASSERT(m_pFontGUI);
	}

	return m_pFontGUI;
}


GR_Font * GR_QNXGraphics::findFont(const char* pszFontFamily, 
									const char* pszFontStyle, 
									const char* /*pszFontVariant*/, 
									const char* pszFontWeight, 
									const char* /*pszFontStretch*/, 
									const char* pszFontSize)
{
	UT_ASSERT(pszFontFamily);
	UT_ASSERT(pszFontStyle);
	UT_ASSERT(pszFontWeight);
	UT_ASSERT(pszFontSize);
	
	UT_DEBUGMSG(("GR: findFont [%s] [%s] [%s] [%s] ", 
			pszFontFamily, pszFontStyle, pszFontWeight, pszFontSize));

	char fname[MAX_FONT_TAG];
	int size = convertDimension(pszFontSize);
	int style = 0;

	// Only check for bold weight and italic style
	if (UT_strcmp(pszFontWeight, "bold") == 0) {
		style |= PF_STYLE_BOLD;
	}
	if (UT_strcmp(pszFontStyle, "italic") == 0) {
		style |= PF_STYLE_ITALIC;
	}
	style|=PF_STYLE_ANTIALIAS; 

//	printf("Looking for font [%s]@%d w/0x%x\n", pszFontFamily, size, style); 
	if (PfGenerateFontName((const char *)pszFontFamily, 
							style, size, (char *)fname) == NULL) {
		//Punt ... give us another chance with a default font
		sprintf(fname, "%s%d", "helv", size); 
		switch (style & (PF_STYLE_BOLD | PF_STYLE_ITALIC)) {
		case (PF_STYLE_BOLD | PF_STYLE_ITALIC):
			strcat(fname, "bi"); break;
		case PF_STYLE_ITALIC:
			strcat(fname, "i"); break;
		case PF_STYLE_BOLD:
			strcat(fname, "b"); break;
		default:
			break;
		}
	}
	UT_DEBUGMSG(("Setting to font name [%s] ", fname));
	if(PfLoadMetrics(fname)!=0)
	{
		PfUnloadMetrics(fname);
	}
	return(new QNXFont(fname));
}

void GR_QNXGraphics::setFont(GR_Font * pFont)
{
	QNXFont *qnxFont = (QNXFont *)pFont;
	const char *backupfont = { "helv10" };
	const char *font;

	if (!qnxFont || !(font = qnxFont->getFont())) {
		UT_DEBUGMSG(("No font found, using helv10 as default"));
		UT_ASSERT(0);
		font = backupfont;
	}

	if (m_pFont && strcmp(m_pFont->getFont(), font) == 0) {
		return;
	} else if (m_pFont) {
		delete(m_pFont);
	}

	m_pFont = new QNXFont(font);

	/* At the same time, load the font metrics into
       a local cache to speed up access as we use them! */
	struct _fcache *tmp;
	for (tmp = g_pFCache; tmp; tmp = tmp->next) {
		if (strcmp(tmp->name, font) == 0) {
			break;
		}
	}
	if (tmp == NULL &&
		(tmp = (struct _fcache *)malloc(sizeof(*tmp))) != NULL) {
		strcpy(tmp->name, font);
		tmp->next = g_pFCache;
		tmp->prev = NULL;
		g_pFCache = tmp;	

//		PfLoadMetrics(font);
	} else if (tmp && tmp->prev) {
		if ((tmp->prev->next = tmp->next)) {
			tmp->next->prev = tmp->prev;
		}
		if ((tmp->next = g_pFCache)) {
			g_pFCache->prev = tmp;
		}
		tmp->prev = NULL;
	}
	m_iAscentCache = m_iDescentCache = -1;
}

UT_uint32 GR_QNXGraphics::getFontAscent()
{
	if(m_iAscentCache == -1) {
		m_iAscentCache = getFontAscent(m_pFont);
	}

	return m_iAscentCache;
}

UT_uint32 GR_QNXGraphics::getFontDescent()
{
	if (m_iDescentCache == -1) {
		m_iDescentCache = getFontDescent(m_pFont);
	}

	return m_iDescentCache;
}

UT_uint32 GR_QNXGraphics::getFontHeight()
{
	return getFontAscent() + getFontDescent();
}

//These versions take the font that we have provided
UT_uint32 GR_QNXGraphics::getFontAscent(GR_Font * fnt)
{
	QNXFont *pQNXFont = (QNXFont *)fnt;
	UT_ASSERT(pQNXFont);
		
	FontQueryInfo info;
	if (PfQueryFontInfo(pQNXFont->getFont(), &info) == -1) {
		UT_ASSERT(0);
		return(0);
	}

	return MY_ABS(info.ascender);
}

UT_uint32 GR_QNXGraphics::getFontDescent(GR_Font * fnt)
{
	QNXFont *pQNXFont = (QNXFont *)fnt;
	UT_ASSERT(pQNXFont);
		
	FontQueryInfo info;
	if (PfQueryFontInfo(pQNXFont->getFont(), &info) == -1) {
		UT_ASSERT(0);
		return(0);
	}

	return MY_ABS(info.descender);
}

UT_uint32 GR_QNXGraphics::getFontHeight(GR_Font * fnt)
{
/*
	return getFontAscent(fnt)+getFontDescent(fnt);
*/
	QNXFont *pQNXFont = (QNXFont *)fnt;
	UT_ASSERT(pQNXFont);
		
	FontQueryInfo info;
	if (PfQueryFontInfo(pQNXFont->getFont(), &info) == -1) {
		UT_ASSERT(0);
		return(0);
	}
	return MY_ABS(info.descender) + MY_ABS(info.ascender);
}


//////////////////////////////////////////////////////////////////
// This is a static method in the GR_Font base class implemented
// in platform code.
//////////////////////////////////////////////////////////////////
void GR_Font::s_getGenericFontProperties(const char *szFontName, FontFamilyEnum * pff, FontPitchEnum * pfp, bool * pbTrueType)
{

#if 0
   switch (xx & 0xf0)
   {
   default:
   case FF_DONTCARE:   *pff = FF_Unknown;      break;
   case FF_ROMAN:      *pff = FF_Roman;        break;
   case FF_SWISS:      *pff = FF_Swiss;        break;
   case FF_MODERN:     *pff = FF_Modern;       break;
   case FF_SCRIPT:     *pff = FF_Script;       break;
   case FF_DECORATIVE: *pff = FF_Decorative;   break;
   }

   if ((xx & TMPF_FIXED_PITCH) == TMPF_FIXED_PITCH)
       *pfp = FP_Variable;             // yes these look backwards
   else                                // but that is how windows
       *pfp = FP_Fixed;                // defines the bits...

   *pbTrueType = ((xx & TMPF_TRUETYPE) == TMPF_TRUETYPE);
#endif
	*pff = FF_Roman;
	*pfp = FP_Variable;
	*pbTrueType = 0;
//XXX.
}


/***
 Draw Operations
***/
void GR_QNXGraphics::setColor(const UT_RGBColor& clr)
{
	//printf("GR: setColor %d %d %d \n", clr.m_red, clr.m_blu, clr.m_grn);
	m_currentColor = PgRGB(clr.m_red, clr.m_grn, clr.m_blu);
	//Defer actually setting the color until we stroke something
}


void GR_QNXGraphics::drawLine(UT_sint32 x1, UT_sint32 y1,
			      UT_sint32 x2, UT_sint32 y2)
{
	// TODO set the line width according to m_iLineWidth
	DRAW_START
	PgSetFillColor(m_currentColor);
	PgSetStrokeColor(m_currentColor);
	PgSetStrokeWidth(m_iLineWidth);
	PgDrawILine(x1, y1, x2, y2);
	DRAW_END
}

void GR_QNXGraphics::setLineWidth(UT_sint32 iLineWidth)
{
	m_iLineWidth = iLineWidth;
}

void GR_QNXGraphics::xorLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2,
			    UT_sint32 y2)
{
	int old;
	old = PgSetDrawMode(Pg_DRAWMODE_XOR);
	drawLine(x1, y1, x2, y2);	
	PgSetDrawMode(old);
}

void GR_QNXGraphics::polyLine(UT_Point * pts, UT_uint32 nPoints)
{
#if 0
	GdkPoint * points = (GdkPoint *)UT_calloc(nPoints, sizeof(GdkPoint));
	UT_ASSERT(points);

	for (UT_uint32 i = 0; i < nPoints; i++)
	{
		points[i].x = pts[i].x;
		// It seems that Windows draws each pixel along the the Y axis
		// one pixel beyond where GDK draws it (even though both coordinate
		// systems start at 0,0 (?)).  Subtracting one clears this up so
		// that the poly line is in the correct place relative to where
		// the rest of GR_QNXGraphics:: does things (drawing text, clearing
		// areas, etc.).
		points[i].y = pts[i].y - 1;	
	}

	gdk_draw_lines(m_pWin, m_pGC, points, nPoints);

	FREEP(points);
#else
	for (UT_uint32 k=1; k<nPoints; k++)
		drawLine(pts[k-1].x,pts[k-1].y, pts[k].x,pts[k].y);
#endif
}

void GR_QNXGraphics::invertRect(const UT_Rect* pRect)
{
	UT_ASSERT(pRect);
	
	int old;
	old = PgSetDrawMode(Pg_DRAWMODE_XOR);
	UT_RGBColor c;
	c.m_red = PgRedValue(m_currentColor);
	c.m_blu = PgBlueValue(m_currentColor);
	c.m_grn = PgGreenValue(m_currentColor);
	fillRect(c, pRect->left, pRect->top, pRect->width, pRect->height);
	PgSetDrawMode(old);
}

void GR_QNXGraphics::setClipRect(const UT_Rect* pRect)
{
	m_pRect = pRect;
	if (pRect)
	{
		PhRect_t r;

		r.ul.x = pRect->left;
		r.ul.y = pRect->top;
		r.lr.x = r.ul.x + pRect->width;
		r.lr.y = r.ul.y + pRect->height;

//		UT_ASSERT(!m_pClipList);		//Only one item for now	
	
		if (m_pClipList || (m_pClipList = PhGetTile())) {
			m_pClipList->rect = r;
			m_pClipList->next = NULL; //One item list for now
		}
	}
	else
	{
		if (m_pClipList) {
			PhFreeTiles(m_pClipList);
			m_pClipList = NULL;
		}
	}
}

void GR_QNXGraphics::fillRect(UT_RGBColor& c, UT_Rect &r)
{
	fillRect(c, r.left, r.top, r.width, r.height);
}

void GR_QNXGraphics::fillRect(UT_RGBColor& c, UT_sint32 x, UT_sint32 y,
							   UT_sint32 w, UT_sint32 h)
{
	PgColor_t newc;

	//Why do we have to do this, why can't the xap code figure it right?
	w = (w < 0) ? -1*w : w;
	h = (h < 0) ? -1*h : h;

	newc = PgRGB(c.m_red, c.m_grn, c.m_blu);
	DRAW_START
	PgSetFillColor(newc);
	PgSetStrokeColor(newc);
	//printf("fillRect RGB %d,%d %d/%d w/ %08x\n", x, y, w, h, newc);
	PgDrawIRect(x, y, x+w, y+h, Pg_DRAW_FILL_STROKE);
	DRAW_END
}

inline void adjust_rect(PhRect_t *rect, PhPoint_t *offset) {
	//Adjust the sides inwards to the rect (-x -> left side else right)
	if (offset->x < 0) {
		rect->ul.x -= offset->x;
	}
	else {
		rect->lr.x -= offset->x;
	}

	//Adjust the sides inwards to the rect (-y -> left side else right)
	if (offset->y < 0) {
		rect->ul.y -= offset->y;
	}
	else {
		rect->lr.y -= offset->y;
	}
}

void GR_QNXGraphics::scroll(UT_sint32 dx, UT_sint32 dy)
{

	PhRect_t  rect;
	PhPoint_t offset;

	PtBasicWidgetCanvas(m_pDraw, &rect);

	offset.x = -1*dx;
	offset.y = -1*dy;

/*
	UT_DEBUGMSG(("GR Scroll1 Before %d,%d %d,%d  by %d,%d",
			rect.ul.x, rect.ul.y, rect.lr.x, rect.lr.y, offset.x, offset.y));
*/
	
	//This does the blit on the region, so the rect must be in
	//the windows co-ordinates.  But it doesn't automatically
	//generate a damage event like PtBlit does so we only re-draw once.
	PhPoint_t shift;
	PtWidgetOffset(m_pDraw, &shift);
	PtTranslateRect(&rect, &shift);					

	//The problem here is with clipping ... can I clip the the rect?
	//Alternately, I should be able to adjust the rect by the offset
	//on the opposite side that it is scrolling ... clipping would be
	//way easier though.
	adjust_rect(&rect, &offset);

/*
	UT_DEBUGMSG(("GR Scroll1 %d,%d %d,%d  by %d,%d",
			rect.ul.x, rect.ul.y, rect.lr.x, rect.lr.y, offset.x, offset.y));
*/
	PhBlit(PtWidgetRid(PtFindDisjoint(m_pDraw)), &rect, &offset);
	//to get an expose call PtDamageExtent(region_widget, damage_rect)
}

void GR_QNXGraphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						  UT_sint32 x_src, UT_sint32 y_src,
						  UT_sint32 width, UT_sint32 height)
{
	PhRect_t 	rect, widgetrect;
	PhPoint_t 	offset;

	PtBasicWidgetCanvas(m_pDraw, &widgetrect);

/*
	UT_DEBUGMSG(("GR Scroll2 dest:%d,%d src:%d,%d %dx%d ", 
					x_dest, y_dest, x_src, y_src, width, height));
*/
	if (width < 0) {
		UT_ASSERT(0);
		x_src -= width;
		width = -1*width;
	}
	if (height < 0) {
		UT_ASSERT(0);
		y_src -= height;
		height = -1*height;
	}

	rect.ul.x = rect.lr.x = x_src;
	rect.ul.y = rect.lr.y = y_src;
	rect.lr.x += width;
	rect.lr.y += height;

	offset.x = x_dest - x_src;
	offset.y = y_dest - y_src;

/*
	UT_DEBUGMSG(("GR Scroll2 Before %d,%d %d,%d  by %d,%d",
			rect.ul.x, rect.ul.y, rect.lr.x, rect.lr.y, offset.x, offset.y));
*/

	if (PtRectIntersect(&widgetrect, &rect) == 0) {
		UT_DEBUGMSG(("No intersection! "));
		UT_ASSERT(0);
	}
	else {
		rect = widgetrect;
	}

	PhPoint_t shift;
	PtWidgetOffset(m_pDraw, &shift);
	PtTranslateRect(&rect, &shift);					

/*
	UT_DEBUGMSG(("GR Scroll2 %d,%d %d,%d  by %d,%d",
			rect.ul.x, rect.ul.y, rect.lr.x, rect.lr.y, offset.x, offset.y));
*/

	PhBlit(PtWidgetRid(PtFindDisjoint(m_pDraw)), &rect, &offset);
}

void GR_QNXGraphics::clearArea(UT_sint32 x, UT_sint32 y,
				 UT_sint32 width, UT_sint32 height)
{
	UT_RGBColor clrWhite(255,255,255);
	//printf("clearArea %d,%d %d/%d w/ %08x\n", x, y, width, height, clrWhite);
	fillRect(clrWhite, x, y, width, height);
}

/***
 Image operations
***/
GR_Image* GR_QNXGraphics::createNewImage(const char* pszName, const UT_ByteBuf* pBBPNG, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, GR_Image::GRType iType)
{
	/* GR_QNXImage* pImg = NULL; */
	GR_Image* pImg = NULL;
   	if (iType == GR_Image::GRT_Raster)
     		pImg = new GR_QNXImage(pszName);
   	else
     		pImg = new GR_VectorImage(pszName);
   
	pImg->convertFromBuffer(pBBPNG, iDisplayWidth, iDisplayHeight);
	return pImg;
}

void GR_QNXGraphics::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
	UT_ASSERT(pImg);

   	if (pImg->getType() != GR_Image::GRT_Raster) {
      		pImg->render(this, xDest, yDest);
      		return;
   	}
   
	GR_QNXImage * pQNXImage = (GR_QNXImage *)(pImg);
	Fatmap * image = pQNXImage->getData();
	PhPoint_t pos;
	PhDim_t   size;

	size.w = pQNXImage->getDisplayWidth();
	size.h = pQNXImage->getDisplayHeight();

	DRAW_START 

	pos.x = xDest; pos.y = yDest;

	UT_ASSERT(image->data);
	PgDrawImagemx(image->data,			/* Data */
				Pg_IMAGE_DIRECT_888,  	/* Type */ 
				&pos,					/* Position */
				&size,					/* Size */
				3 /* 24 bit image */ * size.w,	/* BPL */
				0);						/* tag (CRC) */
	PgFlush();

	DRAW_END
}

void GR_QNXGraphics::setColorSpace(GR_Graphics::ColorSpace /* c */)
{
	// we only use ONE color space here now (GdkRGB's space)
	// and we don't let people change that on us.
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

GR_Graphics::ColorSpace GR_QNXGraphics::getColorSpace(void) const
{
	return m_cs;
}

void GR_QNXGraphics::setCursor(GR_Graphics::Cursor c)
{
unsigned short cursor_number=0;
	if (m_cursor == c)
		return;
	
	m_cursor = c;

	switch (c)
	{
	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		/*FALLTHRU*/
	case GR_CURSOR_DEFAULT:
		cursor_number = Ph_CURSOR_POINTER;
		break;
		
	case GR_CURSOR_IBEAM:
		cursor_number = Ph_CURSOR_POINTER; //XXX: Wtf is IBEAM ?
		break;

	case GR_CURSOR_RIGHTARROW:
		cursor_number = Ph_CURSOR_DRAG_RIGHT; 
		break;
		
#ifdef BIDI_ENABLED
	case GR_CURSOR_LEFTARROW:
		cursor_number = Ph_CURSOR_DRAG_LEFT; //GDK_ARROW;
		break;
#endif

	case GR_CURSOR_IMAGE:
		cursor_number = Ph_CURSOR_POINTER; //XXX: ???
		break;
		
	case GR_CURSOR_IMAGESIZE_NW:
		cursor_number = Ph_CURSOR_DRAG_TL;
		break;
		
	case GR_CURSOR_IMAGESIZE_N:
		cursor_number = Ph_CURSOR_DRAG_TOP;
		break;
		
	case GR_CURSOR_IMAGESIZE_NE:
		cursor_number = Ph_CURSOR_DRAG_TR;
		break;
		
	case GR_CURSOR_IMAGESIZE_E:
		cursor_number = Ph_CURSOR_DRAG_RIGHT;
		break;
		
	case GR_CURSOR_IMAGESIZE_SE:
		cursor_number = Ph_CURSOR_DRAG_BR;
		break;
		
	case GR_CURSOR_IMAGESIZE_S:
		cursor_number = Ph_CURSOR_DRAG_BOTTOM;
		break;
		
	case GR_CURSOR_IMAGESIZE_SW:
		cursor_number = Ph_CURSOR_DRAG_BL;
		break;
		
	case GR_CURSOR_IMAGESIZE_W:
		cursor_number = Ph_CURSOR_DRAG_LEFT;
		break;

	case GR_CURSOR_LEFTRIGHT:
		cursor_number=Ph_CURSOR_DRAG_HORIZONTAL;
		break;
	case GR_CURSOR_UPDOWN:
		cursor_number=Ph_CURSOR_DRAG_VERTICAL;
		break;
	case GR_CURSOR_EXCHANGE:
		cursor_number=Ph_CURSOR_POINTER;
		break;
	case GR_CURSOR_GRAB:
		cursor_number=Ph_CURSOR_MOVE;
		break;
	case GR_CURSOR_LINK:
		cursor_number=Ph_CURSOR_FINGER;
		break;	
	case GR_CURSOR_WAIT:
		cursor_number= Ph_CURSOR_WAIT;
		break;
	}

	PtSetResource(m_pWin,Pt_ARG_CURSOR_TYPE,cursor_number,0);
}

GR_Graphics::Cursor GR_QNXGraphics::getCursor(void) const
{
	return m_cursor;
}

void GR_QNXGraphics::setColor3D(GR_Color3D c)
{
	UT_ASSERT(c < COUNT_3D_COLORS);
	m_currentColor = m_3dColors[c];
	PgSetStrokeColor(m_currentColor);
	PgSetFillColor(m_currentColor);
}

void GR_QNXGraphics::init3dColors()
{
	m_3dColors[CLR3D_Foreground] =  PgRGB(0, 0, 0);			//Black
	m_3dColors[CLR3D_Background] =  PgRGB(192, 192, 192);	//Light Grey
	m_3dColors[CLR3D_BevelUp] =  PgRGB(255, 255, 255);		//White
	m_3dColors[CLR3D_BevelDown] =  PgRGB(128, 128, 128);	//Dark Grey
	m_3dColors[CLR3D_Highlight] =  PgRGB(255, 255, 255);	//White
}

void GR_QNXGraphics::fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
	UT_ASSERT(c < COUNT_3D_COLORS);

	DRAW_START
	PgSetFillColor(m_3dColors[c]);
	PgSetStrokeColor(m_3dColors[c]);
	//printf("FillRect 3D %d,%d %d/%d w %08x\n", x, y, x+w, y+w, m_3dColors[c]);
	PgDrawIRect(x, y, x+w, y+h, Pg_DRAW_FILL_STROKE);
	DRAW_END
}

void GR_QNXGraphics::fillRect(GR_Color3D c, UT_Rect &r)
{
	UT_ASSERT(c < COUNT_3D_COLORS);
	fillRect(c, r.left, r.top, r.width, r.height);
}

/*** 
 Printing operations
***/
PpPrintContext_t * GR_QNXGraphics::getPrintContext() {
	return m_pPrintContext;
}

void GR_QNXGraphics::setPrintContext(PpPrintContext_t *context) {
	if (context != NULL) {
		m_iShadowZoomPercentage = getZoomPercentage();
		setZoomPercentage(100);
	} else {
		setZoomPercentage(m_iShadowZoomPercentage);
		m_iShadowZoomPercentage = 100;
	}
		
	 m_pPrintContext = context;
}

bool GR_QNXGraphics::startPrint(void) {
	UT_DEBUGMSG(("GR: Start print"));
	UT_ASSERT(m_pPrintContext);

	if (m_pPrintContext) {
		PpStartJob(m_pPrintContext);	
		PpContinueJob(m_pPrintContext);	
	}

 	m_bPrintNextPage = false;    
	return true;
}

bool GR_QNXGraphics::startPage(const char * szPageLabel, UT_uint32 pageNumber,
									bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight) {

	UT_DEBUGMSG(("GR: Start Page %d W/H %d/%d (portrait:%d label:%s nextpage:%d)", 
		pageNumber, iWidth, iHeight, bPortrait, (szPageLabel) ? szPageLabel : "", m_bPrintNextPage));
	UT_ASSERT(m_pPrintContext);

	if (!m_pPrintContext) {
		return false;
	}

	if (!m_bPrintNextPage) {		/* First page do setup */
#if 0
		PhDim_t 	size;

#define DPI_LEVEL 92 
		/* Adjust for non-printable margins? */
		size.w = ((iWidth) * DPI_LEVEL) / 1000;
		size.h = ((iHeight) * DPI_LEVEL) / 1000;
		UT_DEBUGMSG(("Source size %d/%d \n", size.w, size.h));
		PpPrintSetPC(m_pPrintContext, INITIAL_PC, 0, Pp_PC_SOURCE_SIZE, &size);

		PpPrintOpen(m_pPrintContext);	
		PpPrintStart(m_pPrintContext);	
#endif
	} else {
		PpPrintNewPage(m_pPrintContext);
	}

	m_bPrintNextPage = true;
	return true;	
}

bool GR_QNXGraphics::endPrint(void) {
	UT_DEBUGMSG(("GR: End print"));
	UT_ASSERT(m_pPrintContext);

	if (m_pPrintContext) {
		PpSuspendJob(m_pPrintContext);
		PpEndJob(m_pPrintContext);
		PtDamageWidget(m_pWin);		
		PtFlush();	
	}
	return true;
}

