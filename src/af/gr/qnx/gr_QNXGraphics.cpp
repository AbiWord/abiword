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

#include "xap_QNXApp.h"
#include "gr_QNXGraphics.h"
#include "gr_QNXImage.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_misc.h"
#include "ut_string.h"

#define min(x, y) (((x) > (y)) ? (y) : (x))

#define  MY_ABS(x) 	(((x) < 0) ? -(x) : (x))
/* NOTE: This having to set the focus here sucks ... we shouldn't
         have to do it, but unfortunately there is no way to have
		 some things (like the combo boxes) reject our text input
*/
#if 0
#define DRAW_START	PhPoint_t _ptwin, _ptdraw;					\
			PtGetAbsPosition(m_pWin, &_ptwin.x, &_ptwin.y);		\
			PtGetAbsPosition(m_pDraw, &_ptdraw.x, &_ptdraw.y);	\
			_ptdraw.x -= _ptwin.x;								\
			_ptdraw.y -= _ptwin.y;								\
			PgSetTranslation (&_ptdraw, 0 /* Pg_RELATIVE */);	\
			PhRect_t _rdraw;									\
			PtBasicWidgetCanvas(m_pDraw, &_rdraw);				\
			PgSetUserClip(&_rdraw);								\
			PtTranslateRect(&_rdraw, &_ptdraw);					

#define DRAW_END	_ptdraw.x *= -1; 							\
			_ptdraw.y *= -1; 									\
			PgSetUserClip(NULL);								\
			PgSetTranslation(&_ptdraw, 0); 						\
			PgFlush();										
#else
#define DRAW_START	PhPoint_t  _ptdraw;							\
			PgSetRegion(PtWidgetRid(PtFindDisjoint(m_pDraw)));	\
			PtWidgetOffset(m_pDraw, &_ptdraw);					\
			PgSetTranslation (&_ptdraw, 0 /* Pg_RELATIVE */);	\
			PhRect_t _rdraw;									\
			PtBasicWidgetCanvas(m_pDraw, &_rdraw);				\
			PgSetUserClip(&_rdraw);								

#define DRAW_END	_ptdraw.x *= -1; 							\
			_ptdraw.y *= -1; 									\
			PgSetUserClip(NULL);								\
			PgSetTranslation(&_ptdraw, 0); 						\
			/* Debugging PgFlush(); */

#endif

GR_QNXGraphics::GR_QNXGraphics(PtWidget_t * win, PtWidget_t * draw)
{
	m_pWin = win;
	m_pDraw = draw;
	m_pFont = NULL;
	m_pFontGUI = NULL;
	m_iLineWidth = 1;
	m_currentColor = Pg_BLACK;

	m_FontCount =  PfQueryFonts(' ', PHFONT_ALL_FONTS, NULL, 0) + 1;
	m_FontList = (FontDetails *)malloc(m_FontCount * sizeof(*m_FontList));
	memset(m_FontList, 0, m_FontCount * sizeof(*m_FontList));
	m_FontCount = PfQueryFonts(' ', PHFONT_ALL_FONTS, m_FontList, m_FontCount);


	m_cs = GR_Graphics::GR_COLORSPACE_COLOR;
	m_cursor = GR_CURSOR_INVALID;
	setCursor(GR_CURSOR_DEFAULT);
	init3dColors();
}

GR_QNXGraphics::~GR_QNXGraphics()
{
	DELETEP(m_pFontGUI);
	if (m_FontList) {
		free(m_FontList);
	}
}

UT_Bool GR_QNXGraphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
	{
	case DGP_SCREEN:
		return UT_TRUE;
	case DGP_PAPER:
		return UT_FALSE;
	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
}

void GR_QNXGraphics::drawChars(const UT_UCSChar* pChars, int iCharOffset,
				 int iLength, UT_sint32 xoff, UT_sint32 yoff)
{
	PhPoint_t pos;
	
	pos.x = xoff; 
	pos.y = yoff + getFontAscent();

	DRAW_START

	PgFlush();		/* Just to clear out any drawing to be done ... */
	PgSetFont(m_pFont->getFont());
	PgSetTextColor(m_currentColor);

#if 0
	char      *buffer;

	if (!(buffer = (char *)malloc(iLength + 1))) {
		printf("Failed buffer allocation \n");
		return;
	}

	int i;
	for (i = 0; i<iLength; i++) {
		buffer[i] = (char)pChars[i];	//Bad I know convert from UTC later
	}
	buffer[i] = '\0';
	
	
	//For some reason, I _NEED_ to set the BACK_FILL flag, and
	//if I don't set the fill color things just work fine because
	//the last fill that happened was to wipe out the text area
	//with the appropriate color. 

	//This method is no more than a stop gap.  It fails for use
	//on the rulers and it fails for use when scrolling text.
	//A new method must be found!
 
	//This is the prefered way
	//PgDrawTextmx(buffer, iLength, &pos, 0);

	//This way would be okay if it worked
	//PgSetFillColor(Pg_TRANSPARENT);	
	//PgDrawTextmx(buffer, iLength, &pos, Pg_BACK_FILL);

	//This way is a total kludge (assumes last draw was for background)
	//PgSetFillColor(Pg_PURPLE);
	//PgSetStrokeColor(Pg_PURPLE);
	//PgDrawTextmx(buffer, iLength, &pos, Pg_BACK_FILL);

	//This way is even more of a kludge
	//PgSetFillColor(Pg_CYAN);
	//PgSetStrokeColor(Pg_CYAN);
	//PgChromaOn();
	//PgSetChroma(Pg_CYAN, Pg_CHROMA_DEST_MATCH | Pg_CHROMA_NODRAW); 
	//PgDrawTextmx(buffer, iLength, &pos, Pg_BACK_FILL);
	//PgChromaOff();


	//This way seems to work ok, good to use
	PgDrawTextmx(buffer, iLength, &pos, 0);
	printf("Drawing %d,%d [%s] \n", pos.x, pos.y, buffer);

	//We always need to flush the buffer since we free it
	PgFlush();
	free(buffer);	
#else
	PgDrawTextmx((char *)(&pChars[iCharOffset]), iLength*sizeof(UT_UCSChar), 
		   		  &pos, Pg_TEXT_WIDECHAR);

	PgFlush();
#endif

	DRAW_END
}

void GR_QNXGraphics::setFont(GR_Font * pFont)
{
	/*
  	Fonts listed in /usr/photon/font and take
	the form of face size [style] passed in
	to PgSetFont(char *font);
	*/
	QNXFont *qnxFont = (QNXFont *)pFont;
	const char *font;

	if (!qnxFont || !(font = qnxFont->getFont())) {
		fprintf(stderr, "No font found, using helv10 as default \n");
		font = "helv10";
	}

	PgSetFont((char *)font);
	if (m_pFont) {
		delete(m_pFont);
	}

	/*
 	 We need to copy the font here, not just save the pointer
	 m_pFont = qnxFont;
	*/
	m_pFont = new QNXFont(font);
}

UT_uint32 GR_QNXGraphics::getFontHeight()
{
	PhRect_t rect;
	const char *font;

	if (!m_pFont || !(font = m_pFont->getFont())) {
		return(0);
	}

	PfExtentText(&rect, NULL, font, "A", 0);
	//printf("GR: Height:%d + %d \n", MY_ABS(rect.lr.y), MY_ABS(rect.ul.y));
	return(MY_ABS(rect.ul.y) + MY_ABS(rect.lr.y) + 1);
}

UT_uint32 GR_QNXGraphics::measureString(const UT_UCSChar* s, int iOffset,
					  int num,  unsigned short* pWidths)
{
	int  i, charWidth = 0;
	const char *font;

	if (!m_pFont || !(font = m_pFont->getFont())) {
		return(0);
	}
	//printf("GR: Measure string font %s \n", (font) ? font : "NULL");

	//CHANGE THIS TO MAKE IT UNICODE
	char *buffer;
	if (!(buffer = (char *)malloc(num + 1))) {
		fprintf(stderr, "Can't allocate memory \n");
		return(0);
	}
	memset(buffer, 0, num + 1);

#if 0
	for (i = 0; i < num; i++) {
		PhRect_t rect;

		buffer[i] = (char)(s[i+iOffset]);
		PfExtentText(&rect, NULL, font, &buffer[i], 0);
		pWidths[i] = (rect.lr.x - min(rect.ul.x, 0) + 1) - rect.ul.x;
		charWidth += pWidths[i];
		//printf("Width of %s is %d \n", &buffer[i], pWidths[i]);
	}
#else
	PhRect_t rect;
	int indices, penpos;

	for (i = 0; i < num; i++) {

		buffer[i] = (char)(s[i+iOffset]);
		memset(&rect, 0, sizeof(rect));
		indices = 1;
		penpos = 0;

		PfExtentTextCharPositions(&rect, 			/* Rect extent */
							 		NULL,			/* Position offset */
									&buffer[i],		/* Buffer to hit */
									font, 			/* Font buffer uses */
									&indices,		/* Where to get pen pos from */
									&penpos, 		/* Where to store pen pos */
									1,				/* Number of indices */
									0,				/* Flags */
									0,				/* Length of buffer (use strlen) */
									0, 				/* Number of characters to skip */
									NULL);			/* Clipping rectangle? */
		pWidths[i] = penpos;
		charWidth += penpos;
		//printf("Width of %s is %d (%d)\n", &buffer[i], pWidths[i], charWidth);
	}

#endif

	free(buffer);

	//printf("GR: Width:%d \n", charWidth);
	return charWidth;
}

UT_uint32 GR_QNXGraphics::_getResolution(void) const
{
	// this is hard-coded at 100 for X now, since 75 (which
	// most X servers return when queried for a resolution)
	// makes for tiny fonts on modern resolutions.

	return 100;
}

void GR_QNXGraphics::setColor(UT_RGBColor& clr)
{
	//printf("GR: setColor %d %d %d \n", clr.m_red, clr.m_blu, clr.m_grn);
	m_currentColor = PgRGB(clr.m_red, clr.m_grn, clr.m_blu);
	//Defer actually setting the color until we stroke something
}

GR_Font * GR_QNXGraphics::getGUIFont(void)
{
	if (!m_pFontGUI) {
		m_pFontGUI = new QNXFont("helv10");
		UT_ASSERT(m_pFontGUI);
	}

	return m_pFontGUI;
}


#include <ctype.h>
//We want to get a match of the first n characters
//here and the longest match will win.
int GR_QNXGraphics::GetClosestName(const char *pszFontFamily, 
								   char full_font_name[50], 
								   char abrv_font_name[50]) {
	int i, j, longest, closest;

	longest = 0; closest = -1;	
	for (i=0; i<m_FontCount; i++) {
		//printf("Comparing [%s] to [%s] ... ", pszFontFamily, m_FontList[i].desc);
		for (j=0; pszFontFamily[j] && m_FontList[i].desc[j]; j++) {
			if (tolower(pszFontFamily[j]) != tolower(m_FontList[i].desc[j])) {
				break;
			}
		} 
		if (j > longest) {
			longest = j;
			closest = i;
		}
	}

	if (closest >= 0) {
		strcpy(full_font_name, m_FontList[closest].desc);
		strcpy(abrv_font_name, m_FontList[closest].stem);
	}
	else {
		UT_DEBUGMSG(("No match found, using default \n"));
		strcpy(full_font_name, "Times");
		strcpy(abrv_font_name, "time");
	}
}


GR_Font * GR_QNXGraphics::findFont(const char* pszFontFamily, 
									const char* pszFontStyle, 
									const char* /*pszFontVariant*/, 
									const char* pszFontWeight, 
									const char* /*pszFontStretch*/, 
									const char* pszFontSize)
{
	FontQueryInfo fqinfo;
	char fname[50], closest[50], temp[50];

	UT_ASSERT(pszFontFamily);
	UT_ASSERT(pszFontStyle);
	UT_ASSERT(pszFontWeight);
	UT_ASSERT(pszFontSize);
	
	//printf("GR: findFont [%s] [%s] [%s] [%s] \n", pszFontFamily, pszFontStyle, pszFontWeight, pszFontSize);

	GetClosestName(pszFontFamily, closest, fname);
	//printf("GR: Closest font is [%s] [%s] \n", closest, fname);

	//The size is in xxpt so just cat it and back up two
	//strncat(fname, pszFontSize, strlen(pszFontSize) - 2);
	//OR: keep scaling ... the return from this is inches?
	//int size = convertDimension(pszFontSize);
	//OR: just do it yourself!
	double size = UT_convertToPoints(pszFontSize);
	size = (size * (double)getZoomPercentage() / 100.0) + 0.5;
	sprintf(temp, "%s%d", fname, (int)size);
	strcpy(fname, temp);
	//printf("GR: Font w/ size is [%s] (%s -> %d)  \n", fname, pszFontSize, size);
	
	
	// this is kind of sloppy
	if (!UT_stricmp(pszFontStyle, "normal") &&
		!UT_stricmp(pszFontWeight, "normal"))
	{
	}
	else if (!UT_stricmp(pszFontStyle, "normal") &&
			 !UT_stricmp(pszFontWeight, "bold"))
	{
		strcat(fname, "b");
	}
	else if (!UT_stricmp(pszFontStyle, "italic") &&
			 !UT_stricmp(pszFontWeight, "normal"))
	{
		strcat(fname, "i");
	}
	else if (!UT_stricmp(pszFontStyle, "italic") &&
			 !UT_stricmp(pszFontWeight, "bold"))
	{
		strcat(fname, "bi");
	}
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	//printf("GR: Set font to [%s] \n", fname);

	return(new QNXFont(fname));
}

UT_uint32 GR_QNXGraphics::getFontAscent()
{
	FontQueryInfo info;

	if (!m_pFont || !m_pFont->getFont()) {
		return(0);
	}

	if (PfQueryFont(m_pFont->getFont(), &info) == -1) {
		return(0);
	}
		
	return(MY_ABS(info.ascender));
}

UT_uint32 GR_QNXGraphics::getFontDescent()
{
	FontQueryInfo info;

	if (!m_pFont || !m_pFont->getFont()) {
		return(0);
	}

	if (PfQueryFont(m_pFont->getFont(), &info) == -1) {
		return(0);
	}
		
	return(MY_ABS(info.descender));
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
	GdkPoint * points = (GdkPoint *)calloc(nPoints, sizeof(GdkPoint));
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
	if (pRect)
	{
		PhRect_t r;

		r.ul.x = pRect->left;
		r.ul.y = pRect->top;
		r.lr.x = r.ul.x + pRect->width;
		r.lr.y = r.ul.y + pRect->height;

		//printf("GR: Set clipping %d,%d %d/%d \n", 
		//	pRect->left, pRect->top, pRect->width, pRect->height);
		PgSetUserClip(&r);							
		//PtClipAdd(m_pDraw, &r);
	}
	else
	{
		//printf("GR: Clear Clipping \n");
		PgSetUserClip(NULL);							
		//PtClipRemove();
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
	//printf("GR Fill Rect %d,%d %d/%d 0x%x\n", x, y, w, h, newc);
/*
	if ((w < 0) || (h < 0) || x < 0 || y < 0) {
		//printf("BAILING on the fill \n");
		return;
	}
*/
	w = (w < 0) ? -1*w : w;
	h = (h < 0) ? -1*h : h;

	newc = PgRGB(c.m_red, c.m_grn, c.m_blu);
	DRAW_START
	PgSetFillColor(newc);
	PgSetStrokeColor(newc);
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

	//PtWidgetCanvas(m_pDraw, &rect);
	PtBasicWidgetCanvas(m_pDraw, &rect);
	//PtLabelWidgetCanvas(m_pDraw, &rect);

	offset.x = -1*dx;
	offset.y = -1*dy;
	
	/*
	printf("GR Scroll %d,%d %d,%d  by %d,%d\n",
			rect.ul.x, rect.ul.y, rect.lr.x, rect.lr.y, offset.x, offset.y);
	*/

	//This generates an expose event on the area uncovered by the blit
	//which the framework draws over anyway and so it flickers a bit.
	//PtBlit(m_pDraw, &rect, &offset);
	
	//OR: This does the blit on the region, so the rect must be in
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
	PhBlit(PtWidgetRid(PtFindDisjoint(m_pDraw)), &rect, &offset);
	//to get an expose call PtDamageExtent(region_widget, damage_rect)
}

void GR_QNXGraphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						  UT_sint32 x_src, UT_sint32 y_src,
						  UT_sint32 width, UT_sint32 height)
{
	UT_ASSERT(0);
}

void GR_QNXGraphics::clearArea(UT_sint32 x, UT_sint32 y,
				 UT_sint32 width, UT_sint32 height)
{
	UT_RGBColor clrWhite(255,255,255);
	fillRect(clrWhite, x, y, width, height);
}

UT_Bool GR_QNXGraphics::startPrint(void)
{
	UT_ASSERT(0);
	return UT_FALSE;
}

UT_Bool GR_QNXGraphics::startPage(const char * /*szPageLabel*/, UT_uint32 /*pageNumber*/,
								UT_Bool /*bPortrait*/, UT_uint32 /*iWidth*/, UT_uint32 /*iHeight*/)
{
	UT_ASSERT(0);
	return UT_FALSE;
}

UT_Bool GR_QNXGraphics::endPrint(void)
{
	UT_ASSERT(0);
	return UT_FALSE;
}

GR_Image* GR_QNXGraphics::createNewImage(const char* pszName, const UT_ByteBuf* pBBPNG, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
	printf("GR: convertNewImage \n");

	GR_QNXImage* pImg = new GR_QNXImage(NULL, pszName);

	pImg->convertFromPNG(pBBPNG, iDisplayWidth, iDisplayHeight);

	return pImg;
}

void GR_QNXGraphics::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
	UT_ASSERT(pImg);
	
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

void GR_QNXGraphics::flush(void)
{
	PgFlush();
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
#if 0
	if (m_cursor == c)
		return;
	
	m_cursor = c;
	
	GdkCursorType cursor_number;
	
	switch (c)
	{
	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		/*FALLTHRU*/
	case GR_CURSOR_DEFAULT:
		cursor_number = GDK_TOP_LEFT_ARROW;
		break;
		
	case GR_CURSOR_IBEAM:
		cursor_number = GDK_XTERM;
		break;

	case GR_CURSOR_RIGHTARROW:
		cursor_number = GDK_ARROW;
		break;
		
	case GR_CURSOR_IMAGE:
		cursor_number = GDK_FLEUR;
		break;
		
	case GR_CURSOR_IMAGESIZE_NW:
		cursor_number = GDK_TOP_LEFT_CORNER;
		break;
		
	case GR_CURSOR_IMAGESIZE_N:
		cursor_number = GDK_TOP_SIDE;
		break;
		
	case GR_CURSOR_IMAGESIZE_NE:
		cursor_number = GDK_TOP_RIGHT_CORNER;
		break;
		
	case GR_CURSOR_IMAGESIZE_E:
		cursor_number = GDK_RIGHT_SIDE;
		break;
		
	case GR_CURSOR_IMAGESIZE_SE:
		cursor_number = GDK_BOTTOM_RIGHT_CORNER;
		break;
		
	case GR_CURSOR_IMAGESIZE_S:
		cursor_number = GDK_BOTTOM_SIDE;
		break;
		
	case GR_CURSOR_IMAGESIZE_SW:
		cursor_number = GDK_BOTTOM_LEFT_CORNER;
		break;
		
	case GR_CURSOR_IMAGESIZE_W:
		cursor_number = GDK_LEFT_SIDE;
		break;
	}

	GdkCursor * cursor = gdk_cursor_new(cursor_number);
	gdk_window_set_cursor(m_pWin, cursor);
	gdk_cursor_destroy(cursor);
#endif
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
	PgDrawIRect(x, y, x+w, y+h, Pg_DRAW_FILL_STROKE);
	DRAW_END
}

void GR_QNXGraphics::fillRect(GR_Color3D c, UT_Rect &r)
{
	UT_ASSERT(c < COUNT_3D_COLORS);
	fillRect(c, r.left, r.top, r.width, r.height);
}

//////////////////////////////////////////////////////////////////
// This is a static method in the GR_Font base class implemented
// in platform code.
//////////////////////////////////////////////////////////////////

void GR_Font::s_getGenericFontProperties(const char * /*szFontName*/,
										 FontFamilyEnum * pff,
										 FontPitchEnum * pfp,
										 UT_Bool * pbTrueType)
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
	printf("SET GENERIC FONT ... WHY HERE?\n");
	*pff = FF_Unknown;
	*pfp = FP_Variable;
	*pbTrueType = 0;
}
