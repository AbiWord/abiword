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
#include <math.h>
#include <photon/Pf.h>
#include <photon/PhRender.h>

#include "xap_QNXApp.h"
#include "gr_QNXGraphics.h"
#include "gr_QNXImage.h"
#include "gr_CharWidths.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_misc.h"
#include "ut_string.h"
#include "ut_iconv.h"

#define min(x, y) (((x) > (y)) ? (y) : (x))

#define  MY_ABS(x) 	(((x) < 0) ? -(x) : (x))
#define DOUBLE_TO_FIXED(x)	((long)(x * 65536.0 + 0.5)) 
/* NOTE: Having to do all of this set up here is a real
         pain, but since we can be drawing outside of an
         expose context, it is just what we have to do.
*/

#define DRAW_START if(DrawSetup() == -1) return; 
#define DRAW_END   DrawTeardown();

const char* GR_Graphics::findNearestFont(const char* pszFontFamily,
										 const char* pszFontStyle,
										 const char* pszFontVariant,
										 const char* pszFontWeight,
										 const char* pszFontStretch,
										 const char* pszFontSize)
{
	return pszFontFamily;
}
/***
 Initialization/Teardown for drawing on a widget outside of the normal
 stream of "damage" events
***/
int GR_QNXGraphics::DrawSetup() {

	//Don't do any of this if you are printing
	if (m_pPrintContext) {
		return 0;
	}
	//sometimes abi is stupid and calls a raw draw function without making sure we want to draw it (ie, Left ruler in normal view mode), therefor we do a simple check if our widget is realized or not...
	if(PtWidgetIsRealized(m_pDraw) == 0) return -1;

	m_pGC_old=PgSetGC(m_pGC);

	//Set the region and the draw offset
	PgSetRegion(PtWidgetRid(PtFindDisjoint(m_pDraw)));

	PtWidgetOffset(m_pDraw, &m_OffsetPoint);
	m_OffsetPoint.y+=m_pDraw->area.pos.y;
	m_OffsetPoint.x+=m_pDraw->area.pos.x;
	PgSetTranslation (&m_OffsetPoint,0); //replace translation with this one.	

/*	PhPoint_t trans;
	trans.x = -m_OffsetPoint.x;
	trans.y = -m_OffsetPoint.y;
	PdSetOffscreenTranslation(m_pOSC,&trans);*/

	//Always clip to the canvas
	PhRect_t _rdraw;
	PtCalcCanvas(m_pDraw, &_rdraw);
	//Add additional user clipping areas (only one for now)
	if (m_pClipList) {
		//Instead use this
		if (PhRectIntersect(&_rdraw, &m_pClipList->rect) == 0) {
			//This should never happen!
			UT_DEBUGMSG(("No intersection of widget %d,%d %d,%d ",
						  _rdraw.ul.x, _rdraw.ul.y, _rdraw.lr.x, _rdraw.lr.y));
			UT_DEBUGMSG(("and clip list %d,%d %d,%d ",
						  m_pClipList->rect.ul.x, m_pClipList->rect.ul.y,
						  m_pClipList->rect.lr.x, m_pClipList->rect.lr.y));
			//Instead set a 0,0 area
			memset(&_rdraw, 0, sizeof(_rdraw));
		}
	}
/*	PhPoint_t abs;
	PtGetAbsPosition(m_pDraw,&abs.x,&abs.y);
	PhDeTranslateRect(&_rdraw,&abs);*/
	PgSetUserClip(&_rdraw);


	return 0;

}

int GR_QNXGraphics::DrawTeardown() {

	//Don't do any of this if you are printing
	if (m_pPrintContext) {
		return 0;
	}

	//Remove the clipping (only one for now)
//	PtClipRemove();
	//Reset the translation
/*	m_OffsetPoint.x *= -1;
	m_OffsetPoint.y *= -1;
	PgSetTranslation(&m_OffsetPoint, 0);*/


/*	PhRect_t rect;
	rect.ul.x = rect.ul.y = 0;
	rect.lr.x = m_pDraw->area.size.w;
	rect.lr.y = m_pDraw->area.size.h;

	PhRect_t rect2;

	rect2.lr.x = m_OffsetPoint.x + m_pDraw->area.size.w;
	rect2.lr.y = m_OffsetPoint.y + m_pDraw->area.size.h;
	rect2.ul.x = m_OffsetPoint.x;
	rect2.ul.y = m_OffsetPoint.y;

	PgContextBlit(m_pOSC,&rect,NULL,&rect2); */

	PgSetUserClip(NULL);
	PgClearTranslation();

	PgSetGC(m_pGC_old);
//	PhDCSetCurrent(m_pOldDC);

	return 0;
}


GR_QNXGraphics::GR_QNXGraphics(PtWidget_t * win, PtWidget_t * draw, XAP_App *app)
{
	m_pApp = app;
	m_pWin = win;
	m_pDraw = draw;
	m_pGC = PgCreateGC(0);
	m_pFont = NULL;
	m_pFontGUI = NULL;
	m_pClipList = NULL;
	m_iLineWidth = 1;
	m_currentColor = Pg_BLACK;
	m_pPrintContext = NULL;
	m_iAscentCache = m_iDescentCache = m_iHeightCache -1;

//	m_pOSC = PdCreateOffscreenContext(0,draw->area.size.w,draw->area.size.h,NULL);
	m_pFontCx = PfAttachCx("/dev/phfont",240000);
	
	m_cs = GR_Graphics::GR_COLORSPACE_COLOR;
	m_cursor = GR_CURSOR_INVALID;
	setCursor(GR_CURSOR_DEFAULT);
	init3dColors();
}

GR_QNXGraphics::~GR_QNXGraphics()
{
	_destroyFonts ();

	UT_VECTOR_PURGEALL(UT_Rect*, m_vSaveRect);

	for (UT_uint32 i = 0; i < m_vSaveRectBuf.size (); i++) {
	  PhImage_t * pImg = (PhImage_t	*)m_vSaveRectBuf.getNthItem (i);
	  PgShmemDestroy(pImg);
	}
	PgDestroyGC(m_pGC);
//	PhDCRelease(m_pOSC);
	PfDetach(m_pFontCx);
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

void GR_QNXGraphics::setLineProperties ( double inWidthPixels, 
					  GR_Graphics::JoinStyle inJoinStyle,
					  GR_Graphics::CapStyle inCapStyle,
					  GR_Graphics::LineStyle inLineStyle )
{
int capstyle,joinstyle;
switch(inJoinStyle)
{
	case JOIN_MITER:
		joinstyle = Pg_MITER_JOIN;
		break;
	case JOIN_ROUND:
		joinstyle = Pg_ROUND_JOIN;
		break;
	case JOIN_BEVEL:
		joinstyle = Pg_BEVEL_JOIN;
		break;
}
switch (inCapStyle)
{
	case CAP_BUTT:
		capstyle=Pg_BUTT_CAP;
		break;
	case CAP_ROUND:
		capstyle=Pg_ROUND_CAP;
		break;
	case CAP_PROJECTING:
		capstyle=Pg_SQUARE_CAP;
		break;
}

DRAW_START
if(inLineStyle == LINE_SOLID)
	PgSetStrokeDash(NULL,0,0x10000);
if(inLineStyle == LINE_ON_OFF_DASH)
	PgSetStrokeDash((const unsigned char *)"\10\4",2,0x10000); //Not 110% if this is same as in GTK.
if(inLineStyle == LINE_DOUBLE_DASH)
	PgSetStrokeDash((const unsigned char *)"\10\4",2,0x10000);  //Same as gtk?

PgSetStrokeWidth((int)inWidthPixels);
PgSetStrokeCap(capstyle);
PgSetStrokeJoin(joinstyle);
DRAW_END
}

UT_uint32 GR_QNXGraphics::getDeviceResolution(void) const
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

	return 72;
}

void GR_QNXGraphics::flush(void)
{
	DRAW_START
	PgFlush();
	DRAW_END
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
							   int iLength, UT_sint32 xoff, UT_sint32 yoff,
							   int * pCharWidths)
{
if(pCharWidths == NULL && iLength > 1) {
	static int alloclen;
	static UT_UCS2Char *ucs2buffer;

	if(iLength > (alloclen/2)) {
		alloclen = iLength*2;
		ucs2buffer = (UT_UCS2Char *)realloc(ucs2buffer,alloclen);
	}

	for(int i=0;i<iLength;i++)
		ucs2buffer[i] = (UT_UCS2Char)pChars[iCharOffset+i];

	PhPoint_t pos = {tdu(xoff),tdu(getFontAscent()) + tdu(yoff)};
	DRAW_START
	GR_CaretDisabler caretDisabler(getCaret());
//	PgSetTextColor(m_currentColor);	
//	PgSetFont(m_pFont->getDisplayFont());
	PgDrawText((const char *)ucs2buffer,iLength*2,&pos,Pg_TEXT_WIDECHAR|Pg_TEXT_LEFT);
	DRAW_END
}
else
	for(int i=0;i<iLength;i++) {
		drawChar(pChars[iCharOffset + i],xoff,yoff);
		if(pCharWidths && i < iLength - 1)
			xoff += pCharWidths[i];
}
}

void GR_QNXGraphics::drawChar(UT_UCSChar Char, UT_sint32 xoff, UT_sint32 yoff)
{
PhPoint_t pos = {tdu(xoff),tdu(yoff)+ tdu(getFontAscent())};
UT_UCS2Char ucs2buffer[2] = {(UT_UCS2Char)Char,0};

GR_CaretDisabler caretDisabler(getCaret());
DRAW_START

//PgSetTextColor(m_currentColor);	
//PgSetFont(m_pFont->getDisplayFont());
PgDrawText((const char *)&ucs2buffer,2,&pos,Pg_TEXT_WIDECHAR|Pg_TEXT_LEFT);
DRAW_END
}

UT_uint32 GR_QNXGraphics::measureUnRemappedChar(const UT_UCSChar c)
{
float fWidth;
//All other platforms do this, so why not..
	if(c == 0x200B || c == 0xFEFF) { // 0-with spaces
		return 0;
	}

fWidth = m_pFont->getCharWidthFromCache(c);
fWidth *= ((float)m_pFont->getSize() / (float)GR_CharWidthsCache::CACHE_FONT_SIZE);
fWidth = ftluD(fWidth);
return static_cast<UT_uint32>(rint(fWidth));
}

/***
 Font Operations
***/
GR_Font * GR_QNXGraphics::getGUIFont(void)
{
	if (!m_pFontGUI) {
		char font[20];
		FontID *fid;
		fid = PfFindFont("Helvetica",0,(int)(10.0 * 100.0) / getZoomPercentage());
		m_pFontGUI = new QNXFont(fid);
		UT_ASSERT(m_pFontGUI);
	}

	return m_pFontGUI;
}


GR_Font * GR_QNXGraphics::_findFont(const char* pszFontFamily,
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

	int size = (int)UT_convertToPoints(pszFontSize);

	UT_ASSERT(size);

	int style = 0;
	FontID *fid;
	// Only check for bold weight and italic style
	if (UT_strcmp(pszFontWeight, "bold") == 0) {
		style |= PF_STYLE_BOLD;
	}
	if (UT_strcmp(pszFontStyle, "italic") == 0) {
		style |= PF_STYLE_ITALIC;
	}
	fid = PfFindFont((const char *)pszFontFamily, style, size);
	if (fid == NULL) { //couldn't find that font, try default.
		fid = PfFindFont("helvetica",style,size);
	}
	if(PfLoadMetricsCx(m_pFontCx,PfConvertFontID(fid))!=0)
	{
		UT_DEBUGMSG(("PfLoadMetrics failed!"));
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	QNXFont *font = new QNXFont(fid);
	return font;
}

void GR_QNXGraphics::drawGlyph(UT_uint32 Char,UT_sint32 xoff,UT_sint32 yoff)
{
	UT_ASSERT(UT_TODO);
}

void GR_QNXGraphics::setFont(GR_Font * pFont)
{
	QNXFont *qnxFont = static_cast<QNXFont *>(pFont);
	if(m_pFont == NULL || pFont->getAllocNumber() != m_iFontAllocNo)
	{
		//change size of font depending on zoom...
		UT_uint32 size = (UT_uint32)rint((qnxFont->getSize() * getZoomPercentage()) / 100.0);
		qnxFont->createDisplayFont(size);
		m_pFont = qnxFont;
		m_iFontAllocNo= pFont->getAllocNumber();
		m_iAscentCache = m_iDescentCache = m_iHeightCache = -1;

	}
	//even if they are the same, make sure you set it to the new ptr, as the caller expect.
	else 
	{ 
		m_pFont = qnxFont;
	}
	DRAW_START
	PgSetFont(qnxFont->getDisplayFont());
	DRAW_END
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
	if(m_iHeightCache == -1)
		m_iHeightCache = getFontHeight(m_pFont);
return m_iHeightCache;
}

//These versions take the font that we have provided
UT_uint32 GR_QNXGraphics::getFontAscent(GR_Font * fnt)
{
QNXFont *font = (QNXFont *) fnt;
FontQueryInfo qinfo;
	if(PfQueryFontInfoCx(m_pFontCx,font->getFont(),&qinfo) == -1)
		return 0; 
return (abs(qinfo.ascender) * getResolution() / getDeviceResolution());
}

UT_uint32 GR_QNXGraphics::getFontDescent(GR_Font * fnt)
{
QNXFont *font = (QNXFont *) fnt;
FontQueryInfo qinfo;
	if(PfQueryFontInfoCx(m_pFontCx,font->getFont(),&qinfo) == -1)
		return 0; 
return (qinfo.descender * getResolution() /getDeviceResolution());

}

UT_uint32 GR_QNXGraphics::getFontHeight(GR_Font * fnt)
{
QNXFont *font = (QNXFont *) fnt;
FontQueryInfo qinfo;
	if(PfQueryFontInfoCx(m_pFontCx,font->getFont(),&qinfo) == -1)
		return 0; 
return ((qinfo.descender + abs(qinfo.ascender))) * getResolution() / getDeviceResolution();
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
void GR_QNXGraphics::getColor(UT_RGBColor& clr)
{
	clr.m_red = PgRedValue(m_currentColor);
	clr.m_blu = PgBlueValue(m_currentColor);
	clr.m_grn = PgGreenValue(m_currentColor);
}


void GR_QNXGraphics::setColor(const UT_RGBColor& clr)
{
	m_currentColor = PgRGB(clr.m_red, clr.m_grn, clr.m_blu);
	DRAW_START
	PgSetFillColor(m_currentColor);
	PgSetTextColor(m_currentColor);
	PgSetStrokeColor(m_currentColor);
	DRAW_END
}


void GR_QNXGraphics::drawLine(UT_sint32 x1, UT_sint32 y1,
			      UT_sint32 x2, UT_sint32 y2)
{
	if(getCaret() && getCaret()->isEnabled())
		GR_CaretDisabler caretDisabler(getCaret());
		DRAW_START


	x1 = tdu(x1);
	x2 = tdu(x2);
	y1 = tdu(y1);
	y2 = tdu(y2);

//	PgSetFillColor(m_currentColor);
//	PgSetStrokeColor(m_currentColor);
//	PgSetStrokeWidth(m_iLineWidth);
	PgDrawILine(x1, y1, x2, y2);
	DRAW_END
}

void GR_QNXGraphics::getCoverage(UT_Vector &coverage)
{
FontQueryInfo info;
const char *font;
if(!m_pFont || !(font = m_pFont->getFont())) {
return;
}
coverage.clear();
memset(&info,0,sizeof(info));
PfQueryFontInfoCx(m_pFontCx,font,&info);
coverage.push_back((void*)info.lochar);
coverage.push_back((void*)(info.hichar - info.lochar));
}

void GR_QNXGraphics::setLineWidth(UT_sint32 iLineWidth)
{
	m_iLineWidth = tdu(iLineWidth);
	DRAW_START
		PgSetStrokeWidth(m_iLineWidth);
	DRAW_END
}

void GR_QNXGraphics::xorLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2,
			    UT_sint32 y2)
{
	int old;

	GR_CaretDisabler caretDisabler(getCaret());
	DRAW_START
	old = PgSetDrawMode(Pg_DrawModeSRCXOR);

	x1 = tdu(x1);
	x2 = tdu(x2);
	y1 = tdu(y1);
 	y2 = tdu(y2);
	PgSetStrokeXORColor(Pg_WHITE,Pg_BLACK);
//	PgSetStrokeWidth(m_iLineWidth);
	PgDrawILine(x1, y1, x2, y2);
	PgFlush();
	PgSetDrawMode(old);
	DRAW_END
}

void GR_QNXGraphics::polyLine(UT_Point * pts, UT_uint32 nPoints)
{
	GR_CaretDisabler caretDisabler(getCaret());
	for (UT_uint32 k=1; k<nPoints; k++)
	{
		drawLine(pts[k-1].x,pts[k-1].y, pts[k].x,pts[k].y);
	}
}

void GR_QNXGraphics::invertRect(const UT_Rect* pRect)
{
	int old;
	UT_ASSERT(pRect);
	GR_CaretDisabler caretDisabler(getCaret());

	DRAW_START

	old = PgSetDrawMode(Pg_DrawModeSRCXOR);
	PgSetFillXORColor(Pg_WHITE,m_currentColor);
	PgDrawIRect(tdu(pRect->left), tdu(pRect->top), tdu(pRect->left)+tdu(pRect->width), tdu(pRect->top)+tdu(pRect->height), Pg_DRAW_FILL);
	PgSetDrawMode(old);

	DRAW_END
}

void GR_QNXGraphics::setClipRect(const UT_Rect* pRect)
{
	m_pRect = pRect;
	if (pRect)
	{
		PhRect_t r;
		r.ul.x = tdu(pRect->left);
		r.ul.y = tdu(pRect->top);
		r.lr.x = r.ul.x + tdu(pRect->width);
		r.lr.y = r.ul.y + tdu(pRect->height);
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

void GR_QNXGraphics::fillRect(const UT_RGBColor & c, UT_sint32 x, UT_sint32 y,
			      UT_sint32 w, UT_sint32 h)
{
	PgColor_t newc;
	x = tdu(x);
	y = tdu(y);
	w = tdu(w);
	h = tdu(h);

	newc = PgRGB(c.m_red, c.m_grn, c.m_blu);
	GR_CaretDisabler caretDisabler(getCaret());
	DRAW_START
	
	PgSetFillColor(newc);
//	printf("fillRect RGB %d,%d %d/%d w/ %08x\n", x, y, w, h, newc);
	PgDrawIRect(x, y, x+w, y+h, Pg_DRAW_FILL);
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

	dx = tdu(dx);
	dy = tdu(dy);
	
	GR_CaretDisabler caretDisabler(getCaret());
	PtCalcCanvas(m_pDraw, &rect);

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
	PhTranslateRect(&rect, &shift);

	//The problem here is with clipping ... can I clip the the rect?
	//Alternately, I should be able to adjust the rect by the offset
	//on the opposite side that it is scrolling ... clipping would be
	//way easier though.
	adjust_rect(&rect, &offset);


	xxx_UT_DEBUGMSG(("GR Scroll1 %d,%d %d,%d  by %d,%d",
			rect.ul.x, rect.ul.y, rect.lr.x, rect.lr.y, offset.x, offset.y));

	PhBlit(PtWidgetRid(PtFindDisjoint(m_pDraw)), &rect, &offset);
	//to get an expose call PtDamageExtent(region_widget, damage_rect)
}

void GR_QNXGraphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						  UT_sint32 x_src, UT_sint32 y_src,
						  UT_sint32 width, UT_sint32 height)
{
	PhRect_t 	rect, widgetrect;
	PhPoint_t 	offset;

	x_dest = tdu(x_dest);
	y_dest = tdu(y_dest);
	x_src = tdu(x_src);
	y_src = tdu(y_src);
	width = tdu(width);
	height = tdu(height);
	GR_CaretDisabler caretDisabler(getCaret());

	PtCalcCanvas(m_pDraw, &widgetrect);

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
	PhTranslateRect(&rect, &shift);


	xxx_UT_DEBUGMSG(("GR Scroll2 %d,%d %d,%d  by %d,%d",
			rect.ul.x, rect.ul.y, rect.lr.x, rect.lr.y, offset.x, offset.y));


	PhBlit(PtWidgetRid(PtFindDisjoint(m_pDraw)), &rect, &offset);
}

void GR_QNXGraphics::clearArea(UT_sint32 x, UT_sint32 y,
				 UT_sint32 width, UT_sint32 height)
{
	UT_RGBColor clrWhite(255,255,255);
	fillRect(clrWhite, x, y, width, height);
}

/***
 Image operations
***/
GR_Image* GR_QNXGraphics::createNewImage(const char* pszName, const UT_ByteBuf* pBBPNG, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, GR_Image::GRType iType)
{
	iDisplayWidth = tdu(iDisplayWidth);
	iDisplayHeight = tdu(iDisplayHeight);
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
	xDest = tdu(xDest);
	yDest = tdu(yDest);


   	if (pImg->getType() != GR_Image::GRT_Raster) {
      		pImg->render(this, xDest, yDest);
      		return;
   	}

	GR_QNXImage * pQNXImage = (GR_QNXImage *)(pImg);
	PhImage_t * image = pQNXImage->getData();
	PhPoint_t pos;

	GR_CaretDisabler caretDisabler(getCaret());
	DRAW_START

	pos.x = xDest; pos.y = yDest;
	PgDrawPhImage(&pos,image,NULL);
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

	case GR_CURSOR_VLINE_DRAG:
		cursor_number	=	Ph_CURSOR_DRAG_HORIZONTAL;
		break;

	case GR_CURSOR_HLINE_DRAG:
		cursor_number = Ph_CURSOR_DRAG_VERTICAL;
		break;

	case GR_CURSOR_RIGHTARROW:
		cursor_number = Ph_CURSOR_DRAG_RIGHT;
		break;

	case GR_CURSOR_LEFTARROW:
		cursor_number = Ph_CURSOR_DRAG_LEFT; //GDK_ARROW;
		break;

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
	case GR_CURSOR_CROSSHAIR:
		cursor_number = Ph_CURSOR_CROSSHAIR;
		break;
	}

	PtSetResource(PtGetParent(m_pWin,PtWindow),Pt_ARG_CURSOR_TYPE,cursor_number,0);
}

GR_Graphics::Cursor GR_QNXGraphics::getCursor(void) const
{
	return m_cursor;
}

void GR_QNXGraphics::setColor3D(GR_Color3D c)
{
	UT_ASSERT(c < COUNT_3D_COLORS);
	m_currentColor = m_3dColors[c];
	DRAW_START

	GR_CaretDisabler caretDisabler(getCaret());
	PgSetStrokeColor(m_currentColor);
	PgSetFillColor(m_currentColor);
	PgSetTextColor(m_currentColor); //do we need this?

	DRAW_END
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

	x = tdu(x);
	y = tdu(y);
	w = tdu(w);
	h = tdu(h);

	GR_CaretDisabler caretDisabler(getCaret());
	DRAW_START
	PgSetFillColor(m_3dColors[c]);
//	fprintf(stderr,"FillRect 3D %d,%d %d/%d w %08x\n", x, y, x+w, y+w, m_3dColors[c]);
	PgDrawIRect(x, y, x+w, y+h, Pg_DRAW_FILL);
	DRAW_END
}

void GR_QNXGraphics::fillRect(GR_Color3D c, UT_Rect &r)
{
	UT_ASSERT(c < COUNT_3D_COLORS);
	fillRect(c, r.left, r.top, r.width, r.height);
}

void GR_QNXGraphics::fillRect(const UT_RGBColor & c,UT_Rect &r)
{
fillRect(c,r.left,r.top,r.width,r.height);
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
		PtFlush();
		PtHold(); //increment "flux" count. no widgets will be redrawn until after print!
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

	iWidth = tdu(iWidth);
	iHeight = tdu(iHeight);

	if (!m_pPrintContext) {
		return false;
	}

	if(!m_bPrintNextPage) {
		//do some setup things..
		m_bPrintNextPage=true;
	} else {
		PgFlush();
		PpPrintNewPage(m_pPrintContext);
	}
	return true;
}

bool GR_QNXGraphics::endPrint(void) {
	UT_DEBUGMSG(("GR: End print"));
	UT_ASSERT(m_pPrintContext);

	if (m_pPrintContext) {
		PgFlush();
		PpSuspendJob(m_pPrintContext);
		PpEndJob(m_pPrintContext);
		PtRelease();
		PtDamageWidget(m_pWin);
		PgFlush();
	}
	return true;
}

void GR_QNXGraphics::saveRectangle(UT_Rect &r, UT_uint32 iIndx)
{
	
  PhRect_t rect;
  short int x,y;

	r.left =  tdu(r.left);
	r.top =   tdu(r.top);
	r.width = tdu(r.width);
	r.height =tdu(r.height); 
#if 0
	// had to turn this off -- the fact the the rectangle coordinances
	// are the same does not guarantee that the bitmap is (e.g., in a
	// right-aligned paragraph the text moves while the caret remains
	// static. Tomas, Jan 18, 2003
  
  if (m_vSaveRect.getItemCount()>iIndx && m_vSaveRect.getNthItem(iIndx) && 
      ((UT_Rect *)(m_vSaveRect.getNthItem(iIndx)))->left == r.left &&
      ((UT_Rect *)(m_vSaveRect.getNthItem(iIndx)))->top == r.top &&
      ((UT_Rect *)(m_vSaveRect.getNthItem(iIndx)))->width == r.width && 
      ((UT_Rect *)(m_vSaveRect.getNthItem(iIndx)))->height == r.height) {
    return;
  }
#endif
		
  void * oldR = NULL;
  m_vSaveRect.setNthItem(iIndx, (void*)new UT_Rect(r),&oldR);
  if(oldR)
    delete (UT_Rect*)oldR;
  
  PhImage_t	*pImg;

  PtGetAbsPosition(m_pDraw,&x,&y);
  rect.ul.x=x + r.left;
  rect.ul.y=y + r.top;
  rect.lr.x= rect.ul.x + r.width;
  rect.lr.y= rect.ul.y + r.height;
  pImg =PgReadScreen(&rect,NULL);

  void * oldC = NULL;
  m_vSaveRectBuf.setNthItem(iIndx, (void*) pImg, &oldC);
  if(oldC)
    PgShmemDestroy((PhImage_t *)oldC);
  
  return;
}

void GR_QNXGraphics::restoreRectangle(UT_uint32 iIndx)
{
  UT_Rect * r = (UT_Rect*)m_vSaveRect.getNthItem(iIndx);
  PhImage_t	*pImg = (PhImage_t*)m_vSaveRectBuf.getNthItem(iIndx);

  if((r && pImg))
    {
      PhPoint_t pos;
      
      DRAW_START
	
	pos.x=r->left;
        pos.y=r->top;
      
	PgDrawPhImage(&pos,pImg,0);
      DRAW_END
    }
  return;
}


/* FONT ROUTINES */

const char *QNXFont::getFont()
{
return PfConvertFontID(m_fontID);
}
const int QNXFont::getSize() {
	return PfFontSize(m_fontID);
}

QNXFont::QNXFont(FontID *aFont)
:
m_fontID(NULL),
m_120ptFontID(NULL),
m_displayFontID(NULL)
{
	UT_ASSERT(aFont);
	m_fontID = aFont;
	UT_String_sprintf(m_hashKey,"%s-%d-%d",getFont(),getSize(),PfFontFlags(m_fontID));	
//set size to cache_font_size(120) 
	m_120ptFontID = PfFindFont(PfFontDescription(m_fontID),PfFontFlags(m_fontID),GR_CharWidthsCache::CACHE_FONT_SIZE);
}

QNXFont::~QNXFont() {
	if(m_fontID)
		PfFreeFont(m_fontID);
	if(m_120ptFontID)
		PfFreeFont(m_120ptFontID);
	if(m_displayFontID) {
		PfFreeFont(m_displayFontID);
		m_displayFontID = NULL;
	}
}

//measure the character at a fontsize of 120.

void QNXFont::createDisplayFont(UT_uint32 size) {
	UT_ASSERT(size);
	if(m_displayFontID) 
		PfFreeFont(m_displayFontID);
	m_displayFontID = PfFindFont(PfFontDescription(m_fontID),PfFontFlags(m_fontID),size);	
}

const int QNXFont::getDisplayFontSize() {
	UT_ASSERT(m_displayFontID);
	PfFontSize(m_displayFontID);
}
const char * QNXFont::getDisplayFont() {
	UT_ASSERT(m_displayFontID);

	return (PfConvertFontID(m_displayFontID));
}
void QNXFont::deleteDisplayFont() {

	if(m_displayFontID) {
		PfFreeFont(m_displayFontID);
		m_displayFontID = NULL;
	}
}

UT_sint32 QNXFont::measureUnremappedCharForCache(UT_UCSChar cChar) const
{
PhRect_t rect;
UT_UCSChar ucs4str[2] = {cChar,NULL};


if(PfExtent(&rect,NULL,PfConvertFontID(m_120ptFontID),NULL,NULL,(char *)ucs4str,NULL,PF_WIDE_CHAR32,NULL) != 0) {
	UT_DEBUGMSG(("PfExtent failed %s",strerror(errno)));
	return 0;
}

return (rect.lr.x - min(rect.ul.x,0));
}
void GR_QNXGraphics::setZoomPercentage(UT_uint32 iZoom)
{
	UT_DEBUGMSG(("Set Zoom!!!! deleting: 0x%x",m_pFontGUI));
	DELETEP (m_pFontGUI);
	if(m_pFont)
		m_pFont->deleteDisplayFont();

	GR_Graphics::setZoomPercentage (iZoom); // chain up
}
