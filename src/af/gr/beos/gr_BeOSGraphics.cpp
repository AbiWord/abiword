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

#include <Debug.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "gr_BeOSGraphics.h"
#include "gr_BeOSImage.h"

#include "xap_BeOSFrame.h"	//For be_DocView 
#include <limits.h>
#include <Font.h>
#include <Debug.h> // Remove me?

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_misc.h"
#include "ut_string.h"

/*
 OPTIMIZATIONS:
 -Make BView callbacks for the scroll code that would
replay a previously recorded BPicture.
 -Get rid of all the sync calls 
*/

#define DPRINTF(x) 	
#if defined(USE_BACKING_BITMAP)
#define UPDATE_VIEW			if (m_pFrontView->Window()->Lock()){	\
					m_pFrontView->DrawBitmapAsync(m_pShadowBitmap);	\
					m_pFrontView->Sync();\
					m_pFrontView->Window()->Unlock();}
#else
//Do a flush instead of a sync
#define UPDATE_VIEW			if (!m_pShadowView->IsPrinting()) { \
					if (m_pShadowView->Window()->Lock()){\
					m_pShadowView->Flush();		\
					m_pShadowView->Window()->Unlock();} \
					}
#endif

GR_BeOSGraphics::GR_BeOSGraphics(BView *docview, XAP_App * app) {
	m_pApp = app;
	m_pShadowView = NULL;
	m_pShadowBitmap = NULL;
	m_pBeOSFont = NULL;
	m_pFontGUI = NULL;
	m_pPrintSettings = NULL;
	m_pPrintJob = NULL;
	m_pFrontView = docview;
 	m_bPrint = FALSE;  
	if (!m_pFrontView)
		return;

	m_cs = GR_Graphics::GR_COLORSPACE_COLOR;
	
#if defined(USE_BACKING_BITMAP)
BRect r;
if (m_pFrontView->Window()->Lock())
	{
		r = m_pFrontView->Bounds();
		m_pFrontView->Window()->Unlock();
	}
	if (!(m_pShadowBitmap = new BBitmap(r, B_RGB32, true, false))) {
		UT_ASSERT(0);
		return;
	}
	if (!(m_pShadowView = new BView(r, "ShadowView", NULL, NULL))) {
		UT_ASSERT(0);
		return; 
	}
	if (m_pShadowBitmap->Lock()) {
		m_pShadowBitmap->AddChild(m_pShadowView);
		m_pShadowBitmap->Unlock();
	}
#else
	m_pShadowView = m_pFrontView;
	m_pShadowView->Window()->Lock();
	m_pShadowView->SetFlags(m_pShadowView->Flags() /*| B_SUBPIXEL_PRECISE*/);
	m_pShadowView->Window()->Unlock();
	m_pFrontView->Window()->Lock();
	m_pFrontView->SetFlags(m_pFrontView->Flags() /*| B_SUBPIXEL_PRECISE*/);
	m_pFrontView->Window()->Unlock();
#endif
/*
 white for _highlight & _bevelup
        black for foreground
        lite gray(192*) for background
        dark gray(128*) for beveldown          
*/

	rgb_color c;
	c.alpha = 255;
	c.red = c.blue = c.green = 0;		//Black
        m_3dColors[CLR3D_Foreground] = c;
	c.red = c.blue = c.green = 192;		//Light Grey
        m_3dColors[CLR3D_Background] = c;
	c.red = c.blue = c.green = 128;		//Dark Grey
        m_3dColors[CLR3D_BevelDown] = c;
	c.red = c.blue = c.green = 255;		//White
        m_3dColors[CLR3D_Highlight] = c;
        m_3dColors[CLR3D_BevelUp] = c;
}		

GR_BeOSGraphics::~GR_BeOSGraphics() {
#if defined(USE_BACKING_BITMAP)
	if (!m_pShadowBitmap)
		return;
if (m_pShadowBitmap->Lock())
{
	m_pShadowBitmap->RemoveChild(m_pShadowView);
	m_pShadowBitmap->Unlock();
}
	delete m_pShadowBitmap;
	delete m_pShadowView;
#endif
	printf("Called GR_BeOSGraphics::~GR_BeOSGraphics()\n");
	DELETEP(m_pFontGUI);
}

void GR_BeOSGraphics::ResizeBitmap(BRect r) {
#if defined(USE_BACKING_BITMAP)
	if (m_pShadowBitmap) {
		if (m_pShadowBitmap->Lock())
		{
			m_pShadowBitmap->RemoveChild(m_pShadowView);
			m_pShadowBitmap->Unlock();
		}
		//Don't really need to nuke the View, just resize
		delete m_pShadowBitmap;	
		delete m_pShadowView;	
	}
	
	if (!(m_pShadowBitmap = new BBitmap(r, B_RGB32, true, false))) {
		UT_ASSERT(0);
		return;
	}
	if (!(m_pShadowView = new BView(r, "ShadowView", NULL, NULL))) {
		UT_ASSERT(0);
		return; 
	}
	if (m_pShadowBitmap->Lock())
	{
		m_pShadowBitmap->AddChild(m_pShadowView);
		m_pShadowBitmap->Unlock();
	}
#endif
}

bool GR_BeOSGraphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
	{
	case DGP_SCREEN:
		return true;
	case DGP_PAPER:			//Not sure what this does
		return true;
		return false;
	default:
		UT_ASSERT(0);
		return false;
	}
}


//Draw this string of characters on the screen in current font
void GR_BeOSGraphics::drawChars(const UT_UCSChar* pChars, int iCharOffset,
							 int iLength, UT_sint32 xoff, UT_sint32 yoff)
{
	int i;
	char *buffer;

	DPRINTF(printf("GR: Draw Chars\n"));
	
	/*Wow, this really sucks here, we are allocating memory/releasing it 
	 * on every drawString. We should find a way around this
	 */

	//We need to convert the string from UCS2 to UTF8 before
	//we use the BeOS string operations on it.
	if (!(buffer = new char[2*(iLength+1)])) {
		return;
	}

	memset(buffer, 0, 2*(iLength+1));
	for (i=0; i<iLength; i++) {
		char * utf8char;
		utf8char =  UT_encodeUTF8char(remapGlyph(pChars[i+iCharOffset], false));
			
//		printf("GR: 0x%x -UCS2 2 UTF8-> ", pChars[i+iCharOffset]);
//		for (int t=0; utf8char[t]; t++) {
//			printf("0x%x ", utf8char[t]);
//		}
//		printf("\n");
		
		strcat(buffer, utf8char);						
	}
	
	if (!m_pShadowView->Window()->Lock()) {
		printf("Lock fail\n");
		DEBUGGER("Lock fail");
		return;
	}

	font_height fh;
	m_pShadowView->GetFontHeight(&fh);

/*DH:
 * I just had a brainstorm on how to fix the jitter problem.
 * Let's measure the string, and draw using the charwidths Abi thinks
 * we are using. This involves moving the pen and using DrawChar, which sucks,
 * but let's see if it works.
 * And it does.
 * Perfectly.
 */
	int offset=(int)(fh.ascent + 0.5);

	BFont viewFont;
	m_pShadowView->GetFont(&viewFont);
	BPoint *escapementArray=new BPoint[iLength];

	escapement_delta tempdelta;
	tempdelta.space=0.0;
	tempdelta.nonspace=0.0;
	float fontsize=viewFont.Size();
	viewFont.GetEscapements(buffer,iLength,&tempdelta,escapementArray);

	// TODO: need remapGlyph() before the following function call?
	m_pShadowView->DrawString(UT_encodeUTF8char(pChars[0+iCharOffset]),
							  BPoint(xoff,yoff+offset));
								  
	for (i=1; i<iLength; i++)
	{
		int widthAbiWants;
		/*Measure the width of the previous character, draw char at new
		 * offset
		 */
		widthAbiWants=(unsigned short int)(escapementArray[i-1].x*fontsize);
		xoff+=widthAbiWants;
		m_pShadowView->DrawString(UT_encodeUTF8char(remapGlyph(pChars[i+iCharOffset], false)),
								  BPoint(xoff,yoff+offset));
	}

	m_pShadowView->Window()->Unlock();

	delete [] escapementArray;
	delete [] buffer;
	UPDATE_VIEW
}

BFont *findClosestFont(const char* pszFontFamily, 
		 	const char* pszFontStyle, 
			const char* pszFontWeight) {

	BFont 	*aFont = new BFont();
	font_family family;
	font_style style; 
	uint32 flags, i; 

	//Try and get a good family ... or one that matches
	int32 numFamilies = count_font_families(); 
   	for (i = 0; i < numFamilies; i++ ) { 
       uint32 flags; 
       if ((get_font_family(i, &family, &flags) == B_OK) &&  
           (strcmp(family, pszFontFamily) == 0)) {
           DPRINTF(printf("Font Match %s and %s \n", family, pszFontFamily));
           break;
       }
	}
	if (i >= numFamilies) {
		strcpy(family, "Dutch801 Rm BT");
		DPRINTF(printf("Didn't find font to match %s \n", pszFontFamily));
	}

#define REGULAR_BIT	0x1
#define ROMAN_BIT	0x2
#define BOLD_BIT	0x4
#define ITALIC_BIT	0x8
#define BOLD_ITALIC_BIT	0x10
	
	//Then try and match the styles 
	//(sub normal for Regular if possible, Roman if required)
	int32 numStyles = count_font_styles(family); 
	int32 stylemask = 0;
	for ( int32 j = 0; j < numStyles; j++ ) { 
		if ( get_font_style(family, j, &style, &flags) == B_OK ) {
			if (strcmp(style, "Regular") == 0) 
				stylemask |= REGULAR_BIT;
			else if (strcmp(style, "Roman") == 0)
				stylemask |= ROMAN_BIT;
			else if (strcmp(style, "Italic") == 0)
				stylemask |= ITALIC_BIT;
			else if (strcmp(style, "Bold") == 0)
				stylemask |= BOLD_BIT;
			else if (strcmp(style, "Bold Italic") == 0)
				stylemask |= BOLD_ITALIC_BIT;			
		} 
	}
	
	int32 targetstyle = 0;								 
	if ((strcmp(pszFontStyle, "italic") == 0) &&
	    (strcmp(pszFontWeight, "bold") == 0)) {
	    targetstyle |= BOLD_BIT | ITALIC_BIT | BOLD_ITALIC_BIT;
	}
	else if (strcmp(pszFontStyle, "italic") == 0) 
		targetstyle |= ITALIC_BIT;
	else if (strcmp(pszFontWeight, "bold") == 0) 
		targetstyle |= BOLD_BIT;
	else 
		targetstyle |= ROMAN_BIT | REGULAR_BIT;
		
	//Search order preference
	//Bold Italic --> Bold Italic; 
	//Bold    --> Bold
	//Italic  --> Italic
	//Regular --> Normal/Roman
	if (targetstyle & stylemask & BOLD_ITALIC_BIT) 
		strcpy(style, "Bold Italic");
	else if (targetstyle & stylemask & ITALIC_BIT) 
		strcpy(style, "Italic");
	else if (targetstyle & stylemask & BOLD_BIT) 
		strcpy(style, "Bold");
	else if (targetstyle & stylemask & ROMAN_BIT) 
		strcpy(style, "Roman");
	else if (targetstyle & stylemask & REGULAR_BIT) 
		strcpy(style, "Regular");

	DPRINTF(printf("Setting Style %s \n", style));
	
	aFont->SetFamilyAndStyle((strlen(family) == 0) ? NULL : family, 
	                         (strlen(style) == 0) ? NULL : style);
	return(aFont);
}

GR_Font* GR_BeOSGraphics::getGUIFont(void)
{
	if (!m_pFontGUI)
	{
		BFont *aFont = new BFont();
		m_pFontGUI = new BeOSFont(aFont);
	}
	return m_pFontGUI;
}

GR_Font* GR_BeOSGraphics::findFont(const char* pszFontFamily, 
								const char* pszFontStyle, 
								const char* /*pszFontVariant*/, 
								const char* pszFontWeight, 
								const char* /*pszFontStretch*/, 
								const char* pszFontSize)
{
	BFont 	*aFont;
	//int 	size = atoi(pszFontSize);
	//UT_sint32 iHeight = convertDimension(pszFontSize);
	int		size = convertDimension(pszFontSize);

	DPRINTF(printf("GR Find Font:\n\tFamily: %s ", pszFontFamily));
	DPRINTF(printf("\n\tStyle: %s ", pszFontStyle));
	DPRINTF(printf("\n\tWeight: %s ", pszFontWeight));
	DPRINTF(printf("\n\tSize: %s (%d) ", pszFontSize, size));
	
	aFont = findClosestFont(pszFontFamily, pszFontStyle, pszFontWeight);
	aFont->SetSize(size);
	DPRINTF(printf("GR: -- Located Font: \n"));
	DPRINTF(aFont->PrintToStream());
	m_pBeOSFont = new BeOSFont(aFont);
	return(m_pBeOSFont);
}

//Set the font to something (I guess we set pFont to be like BFont somewhere)
void GR_BeOSGraphics::setFont(GR_Font* pFont)
{
	BeOSFont *tmpFont;
	
	DPRINTF(printf("GR: Set Font\n"));
	tmpFont = static_cast<BeOSFont*> (pFont);
	UT_ASSERT(tmpFont);
	
	m_pBeOSFont = tmpFont;
	m_pBeOSFont->get_font()->SetSpacing(B_BITMAP_SPACING);
	if (m_pShadowView->Window()->Lock())
	{
	if (m_pBeOSFont)
		m_pShadowView->SetFont(m_pBeOSFont->get_font());
	else
		printf("HEY! NO FONT INFORMATION AVAILABLE!\n");

	m_pShadowView->Window()->Unlock();
	}
}

//Get the height of the font
UT_uint32 GR_BeOSGraphics::getFontHeight()
{
	font_height fh;
	
	if(m_pShadowView->Window()->Lock())
	{
		m_pShadowView->GetFontHeight(&fh);
		m_pShadowView->Window()->Unlock();
	}
	//printf("GR: Get Font Height %d\n",(int)(fh.ascent + fh.descent + fh.leading + 0.5));
	//Gives ascent, descent, leading
	return((UT_uint32)(fh.ascent + fh.descent + fh.leading + 0.5));
}

UT_uint32 GR_BeOSGraphics::getFontAscent()
{
	font_height fh;

	if (m_pShadowView->Window()->Lock())
	{
		m_pShadowView->GetFontHeight(&fh);
		m_pShadowView->Window()->Unlock();
	}
	//Gives ascent, descent, leading
	//printf("GR: Font Ascent %d\n",(int)(fh.ascent + 0.5));
	return((UT_uint32)(fh.ascent + 0.5));
}

UT_uint32 GR_BeOSGraphics::getFontDescent()
{
	font_height fh;
	if(m_pShadowView->Window()->Lock())
	{
		m_pShadowView->GetFontHeight(&fh);
		m_pShadowView->Window()->Unlock();
	}
	//Gives ascent, descent, leading
	//printf("GR: Font Descent %d\n",(int)(fh.descent + 0.5));
	return((UT_uint32)(fh.descent + 0.5));
}

UT_uint32 GR_BeOSGraphics::measureUnRemappedChar(const UT_UCSChar c)
{
	//We need to convert the string from UCS2 to UTF8 before
	//we use the BeOS string operations on it.
	char buffer[10];

	BFont viewFont;
	BPoint escapementArray[1];
	
	char * utf8char;
	utf8char =  UT_encodeUTF8char(c);
	strcpy(buffer, utf8char);						

	escapement_delta tempdelta;
	tempdelta.space=0.0;
	tempdelta.nonspace=0.0;
	float fontsize=0.0f;
	
//	m_pShadowView->GetFont(&viewFont);
//	viewFont.SetSpacing(B_BITMAP_SPACING); // moved inside of lock call..

	if (m_pShadowView->Window()->Lock()) {
		
		m_pShadowView->GetFont(&viewFont);
		viewFont.SetSpacing(B_BITMAP_SPACING);

		m_pShadowView->SetFont(&viewFont);
		
		//Hope this works on UTF8 characters buffers
		viewFont.GetEscapements(buffer,1,&tempdelta,escapementArray);
		fontsize=viewFont.Size();

		m_pShadowView->Window()->Unlock();

		return (escapementArray[0].x *fontsize);
	}
	
	return 1.0f; // Shouldn't happen.
}

#if 0
UT_uint32 GR_BeOSGraphics::measureString(const UT_UCSChar* s, int iOffset,
									  int num,  unsigned short* pWidths)
{
	DPRINTF(printf("GR: Measure String\n"));
	UT_uint32	size, i;	
	char 		*buffer;

	//We need to convert the string from UCS2 to UTF8 before
	//we use the BeOS string operations on it.
	buffer = new char[2*(num+1)];
	if (!buffer) {
		return(0);
	}

	//Set the character, then set the length of the character
	size=0;
	memset(buffer, 0, 2*(num+1));

	BFont viewFont;
	BPoint *escapementArray=new BPoint[num];

	m_pShadowView->GetFont(&viewFont);
	viewFont.SetSpacing(B_BITMAP_SPACING);
	if (m_pShadowView->Window()->Lock()) {
		m_pShadowView->SetFont(&viewFont);
		m_pShadowView->Window()->Unlock();
	}

	for (i=0; i<num; i++) {
		char * utf8char;
		UT_UCSChar *currentChar;
		currentChar = s[i+Offset];
		// TODO: next line might be performance hit
		currentChar = remapGlyph(currentChar, false);
		utf8char =  UT_encodeUTF8char(currentChar);
		strcat(buffer, utf8char);						
	}

	escapement_delta tempdelta;
	tempdelta.space=0.0;
	tempdelta.nonspace=0.0;
	//Hope this works on UTF8 characters buffers
	viewFont.GetEscapements(buffer,num,&tempdelta,escapementArray);
	float fontsize=viewFont.Size();

	for (i=0; i<num; i++)
	{
		pWidths[i]=(short unsigned int) ceil(escapementArray[i].x*fontsize) ;
/*
		printf("Escapement for %d is %d (%f)\n",i,
			(short unsigned int) ceil(escapementArray[i].x*fontsize),
			ceil(escapementArray[i].x*fontsize));
*/
		size+= ceil(escapementArray[i].x *fontsize);
	}

	delete [] buffer;
	return(size);
}
#endif
UT_uint32 GR_BeOSGraphics::_getResolution() const
{
	return 72;
}

void GR_BeOSGraphics::setColor(UT_RGBColor& clr)
{
	DPRINTF(printf("GR: setColor\n"));
	if (m_pShadowView->Window()->Lock())
	{
		m_pShadowView->SetHighColor(clr.m_red, clr.m_grn, clr.m_blu);
		m_pShadowView->Window()->Unlock();
	}

}

void GR_BeOSGraphics::drawLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2,
							UT_sint32 y2)
{
	DPRINTF(printf("GR: Draw Line\n"));
	if (m_pShadowView->Window()->Lock())
	{
		m_pShadowView->StrokeLine(BPoint(x1, y1), BPoint(x2, y2));
		m_pShadowView->Window()->Unlock();
	}
	UPDATE_VIEW
}

void GR_BeOSGraphics::setLineWidth(UT_sint32 iLineWidth)
{
	DPRINTF(printf("GR: Set Line Width %d \n", iLineWidth));
	//m_iLineWidth = iLineWidth;
	if(m_pShadowView->Window()->Lock())
	{
		m_pShadowView->SetPenSize(iLineWidth);
		m_pShadowView->Window()->Unlock();
	}
	//UPDATE_VIEW
}

/*Poly line is only used during drawing squiggles*/
void GR_BeOSGraphics::polyLine(UT_Point * pts, UT_uint32 nPoints)
{
	DPRINTF(printf("GR: Poly Line \n"));
	if (m_pShadowView->Window()->Lock())
        {
	for (UT_uint32 k=1; k<nPoints; k++)
		drawLine(pts[k-1].x,pts[k-1].y, pts[k].x,pts[k].y); 
	
		m_pShadowView->Window()->Unlock();
	}
	
	UPDATE_VIEW
}


void GR_BeOSGraphics::xorLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2,
			    UT_sint32 y2)
{
	DPRINTF(printf("GR: XOR Line\n"));
	if (m_pShadowView->Window()->Lock())
	{	
		drawing_mode oldmode = m_pShadowView->DrawingMode();
		m_pShadowView->SetDrawingMode(B_OP_INVERT);//or B_OP_BLEND
		m_pShadowView->StrokeLine(BPoint(x1, y1), BPoint(x2, y2));
		m_pShadowView->SetDrawingMode(oldmode);
		m_pShadowView->Window()->Unlock();
	}

	UPDATE_VIEW
}

void GR_BeOSGraphics::invertRect(const UT_Rect* pRect)
{
	DPRINTF(printf("GR: Invert Rect\n"));
	if (m_pShadowView->Window()->Lock())
	{
		/*Dan thinks StrokeRect is only getting the outer edges, when we don't really want that. we'll see in a sec*/
		drawing_mode oldmode = m_pShadowView->DrawingMode();
		//printf("Inverting rect\n");
		m_pShadowView->SetDrawingMode(B_OP_INVERT);	//or B_OP_BLEND
		m_pShadowView->/*Stroke*/FillRect(BRect(pRect->left, pRect->top,
									pRect->left + pRect->width,
									pRect->top + pRect->height));
		m_pShadowView->SetDrawingMode(oldmode);
		m_pShadowView->Window()->Unlock();
	}
	UPDATE_VIEW
}

void GR_BeOSGraphics::fillRect(UT_RGBColor& c, UT_Rect &r) {
	fillRect(c,r.left,r.top,r.width,r.height);
}


void GR_BeOSGraphics::fillRect(UT_RGBColor& c, UT_sint32 x, UT_sint32 y,
						UT_sint32 w, UT_sint32 h)
{
	DPRINTF(printf("GR: Flll Rect\n"));
	if (m_pShadowView->Window()->Lock())
	{	
		rgb_color old_colour = m_pShadowView->HighColor();
		m_pShadowView->SetHighColor(c.m_red, c.m_grn, c.m_blu);
		m_pShadowView->FillRect(BRect(x, y, x+w, y+h));
//		m_pShadowView->Invalidate(BRect(x,y,x+w,y+h));
		m_pShadowView->SetHighColor(old_colour);
		m_pShadowView->Window()->Unlock();
	}
	UPDATE_VIEW
}

void GR_BeOSGraphics::setClipRect(const UT_Rect* pRect)
{
	BRegion region;
	BRegion *r = NULL;
	if (pRect) {
		DPRINTF(printf("GR: Set Clip Rect: %d-%d -> %d-%d\n", 
				pRect->left, pRect->top, 
				pRect->left+pRect->width,
				pRect->top+pRect->height));
		region.Set(BRect(pRect->left, pRect->top, 
				 pRect->left+pRect->width,
				pRect->top+pRect->height));
		r = &region;
	}	
	if (m_pShadowView->Window()->Lock())
	{
		m_pShadowView->ConstrainClippingRegion(r);
		m_pShadowView->Window()->Unlock();
	}
}

void GR_BeOSGraphics::scroll(UT_sint32 dx, UT_sint32 dy)
{

	DPRINTF(printf("GR: Scroll dx %d dy %d\n", dx, dy));

#if defined(METHOD_PRE_R4)
	//This is slow and crappy method, but it works
	//when you don't have a CopyBits function
	BRegion region;
	if (m_pShadowView->Window()->Lock())
	{

	//If we are moving down, right offset positive
		BRect r = m_pShadowView->Bounds();
		(dy < 0) ? (r.top -= dy) : (r.bottom -= dy);
		(dx < 0) ? (r.left -= dx) : (r.right -= dx);
		printf("Invalidating "); r.PrintToStream();
		region.Set(BRect(pRect->left, pRect->top, 
				 pRect->left+pRect->width,
				pRect->top+pRect->height));
		m_pShadowView->ConstrainClippingRegion(&region);
		m_pShadowView->Invalidate(r);
		m_pShadowView->Window()->Unlock();
	}
#endif

	//This method lets the app server draw for us
	if(m_pShadowView->Window()->Lock())
	{
		BRect src, dest;
		dest = src = m_pShadowView->Bounds();
		dest.OffsetBy(-1*dx, -1*dy);
		//printf("Scroll SRC "); src.PrintToStream();
		//printf("Scroll DST "); dest.PrintToStream();
		m_pShadowView->CopyBits(src, dest);
		m_pShadowView->Window()->Unlock();
	}
}

void GR_BeOSGraphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
			  UT_sint32 x_src, UT_sint32 y_src,
			  UT_sint32 width, UT_sint32 height)
{
//	DEBUGGER("GR: MoveArea\n");
//	printf("GR: Move Area\n");
	UT_ASSERT(0);
}

void GR_BeOSGraphics::clearArea(UT_sint32 x, UT_sint32 y,
			     UT_sint32 width, UT_sint32 height)
{
	DPRINTF(printf("GR: Clear Area %d-%d -> %d-%d\n", 
					x, y, x+width, y+height));
	if(m_pShadowView->Window()->Lock())
	{
		rgb_color old_colour = m_pShadowView->HighColor();
		m_pShadowView->SetHighColor(m_pShadowView->ViewColor());
		m_pShadowView->FillRect(BRect(x, y, x+width, y+height));
		m_pShadowView->SetHighColor(old_colour);
		m_pShadowView->Window()->Unlock();
	}
	UPDATE_VIEW
}

bool GR_BeOSGraphics::startPrint(void)
{
	if (!m_pPrintJob) {
		printf("Creating a new print job \n");
		m_pPrintJob = new BPrintJob("ADD A NAME HERE");
	}
	if (!m_pPrintJob) {
		printf("No print job ... exiting \n");
		return(false);
	}

	if (!m_pPrintSettings) {
		printf("I SHOULD NEVER BE HERE! \n");
		return(false);
		if (m_pPrintJob->ConfigPage() != B_OK) {
			return(false);
		}

		if (m_pPrintJob->ConfigJob() != B_OK) {
       		         return(false);
       		}
		m_pPrintSettings = m_pPrintJob->Settings();
	}
	m_pPrintJob->SetSettings(m_pPrintSettings);

	printf("Paper Rect: "); m_pPrintJob->PaperRect().PrintToStream();
	printf("Print Rect: "); m_pPrintJob->PrintableRect().PrintToStream();

	m_pPrintJob->BeginJob();

	//Make sure that we start spooling at the right time
	m_bPrint = FALSE;
	return(true);
}

bool GR_BeOSGraphics::startPage(const char * /*szPageLabel*/, 
				   UT_uint32 /*pageNumber*/,
				   bool /*bPortrait*/, 
				   UT_uint32 /*iWidth*/, 
				   UT_uint32 /*iHeight*/) {

	if (!m_pPrintJob || !m_pPrintJob->CanContinue() || !m_pShadowView) {
		printf("GR: Start page something amiss \n");
		return(false);
	}

	if (m_bPrint) {
		BPicture *tmppic;
		BRect     r;

		if(m_pShadowView->Window()->Lock())
		{	
			r = m_pShadowView->Bounds();
			tmppic = m_pShadowView->EndPicture();
			m_pShadowView->Window()->Unlock();
		}
		((be_DocView *)m_pShadowView)->SetPrintPicture(tmppic);
		m_pPrintJob->DrawView(m_pShadowView, 
				      //BRect(0, 0, 600, 600), 
				      BRect(0, 0, SHRT_MAX, SHRT_MAX), 
				      BPoint(0,0));
		
		//Commit this page and move to the next one
		m_pPrintJob->SpoolPage();
		delete(tmppic);
	}

	m_bPrint = TRUE;
	m_pShadowView->BeginPicture(new BPicture());

	return(true);
}

bool GR_BeOSGraphics::endPrint(void) {
	if (!m_pPrintJob || !m_pPrintJob->CanContinue()) {
		return(false);
	}

	if (m_bPrint) {
		BPicture *tmppic;
		BRect     r;

		if (m_pShadowView->Window()->Lock())
		{
			tmppic = m_pShadowView->EndPicture();
			m_pShadowView->Window()->Unlock();
		}
		((be_DocView *)m_pShadowView)->SetPrintPicture(tmppic);
		m_pPrintJob->DrawView(m_pShadowView, 
				      //BRect(0, 0, FLT_MAX, FLT_MAX), 
				      BRect(0, 0, SHRT_MAX, SHRT_MAX), 
                                      BPoint(0,0));
		
		//Commit this page and move to the next one
		m_pPrintJob->SpoolPage();
		delete(tmppic);
	}

	((be_DocView *)m_pShadowView)->SetPrintPicture(NULL);
	m_pPrintJob->CommitJob();
	delete(m_pPrintJob);
	m_pPrintJob = NULL;
	return(true);
}

GR_Image* GR_BeOSGraphics::createNewImage(const char* pszName, 
					  const UT_ByteBuf* pBB,
					  UT_sint32 iDisplayWidth, 
					  UT_sint32 iDisplayHeight,
					  GR_Image::GRType iType)
{
	DPRINTF(printf("GR: Create new image %s \n", pszName));
	GR_Image* pImg = NULL;
   	if (iType == GR_Image::GRT_Raster)
	   	pImg = new GR_BeOSImage(pszName);
   	else
     		pImg = new GR_VectorImage(pszName);
   
	pImg->convertFromBuffer(pBB, iDisplayWidth, iDisplayHeight);
	return pImg;
}

void GR_BeOSGraphics::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
	UT_ASSERT(pImg);
	
   	if (pImg->getType() != GR_Image::GRT_Raster) {
      		pImg->render(this, xDest, yDest);
      		return;
   	}
   
	GR_BeOSImage * pBeOSImage = static_cast<GR_BeOSImage *>(pImg);
	BBitmap* image = pBeOSImage->getData();
	if (!image)
		return;
	//UT_sint32 iImageWidth = pUnixImage->getDisplayWidth();
	//UT_sint32 iImageHeight = pUnixImage->getDisplayHeight();

	if(m_pShadowView->Window()->Lock())
	{
		m_pShadowView->DrawBitmapAsync(image, BPoint(xDest, yDest)); 
		m_pShadowView->Window()->Unlock();
	}
	UPDATE_VIEW
}

void GR_BeOSGraphics::flush(void)
{
	UPDATE_VIEW
}

void GR_BeOSGraphics::setColorSpace(GR_Graphics::ColorSpace c)
{
	// TODO:  maybe? 
	//UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

GR_Graphics::ColorSpace GR_BeOSGraphics::getColorSpace(void) const
{
	return m_cs;
}

void GR_BeOSGraphics::setCursor(GR_Graphics::Cursor c)
{
/*
	if (m_cursor == c)
		return;
	
	m_cursor = c;
	
	enum GdkCursorType cursor_number;
	
	switch (c)
	{
	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		//FALLTHRU
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
*/
}

GR_Graphics::Cursor GR_BeOSGraphics::getCursor(void) const
{
	return m_cursor;
}

void GR_BeOSGraphics::setColor3D(GR_Color3D c)
{
	DPRINTF(printf("Set color 3D %d \n", c));
	if (m_pShadowView->Window()->Lock())
	{
		m_pShadowView->SetHighColor(m_3dColors[c]);
		m_pShadowView->Window()->Unlock();
	}
}

void GR_BeOSGraphics::fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32
 w, UT_sint32 h)
{
	DPRINTF(printf("GR:FillRect 3D %d!\n", c));
	if(m_pShadowView->Window()->Lock())
	{	
		rgb_color old_colour = m_pShadowView->HighColor();
		drawing_mode oldmode=m_pShadowView->DrawingMode();
		m_pShadowView->SetHighColor(m_3dColors[c]);
		m_pShadowView->SetDrawingMode(B_OP_COPY);
		m_pShadowView->FillRect(BRect(x, y, x+w, y+h));
		m_pShadowView->SetHighColor(old_colour);
		m_pShadowView->SetDrawingMode(oldmode);
		m_pShadowView->Window()->Unlock();
	}
	UPDATE_VIEW
}

void GR_BeOSGraphics::fillRect(GR_Color3D c, UT_Rect &r)
{
        UT_ASSERT(c < COUNT_3D_COLORS);
        fillRect(c,r.left,r.top,r.width,r.height);
}                               

//////////////////////////////////////////////////////////////////
// This is a static method in the GR_Font base class implemented
// in platform code.
//////////////////////////////////////////////////////////////////
void GR_Font::s_getGenericFontProperties(const char * szFontName,
										 FontFamilyEnum * pff,
										 FontPitchEnum * pfp,
										 bool * pbTrueType)
{
	// describe in generic terms the named font.

	// Note: most of the unix font handling code is in abi/src/af/xap/unix
	// Note: rather than in the graphics class.  i'm not sure this matters,
	// Note: but it is just different....

	// TODO add code to map the given font name into one of the
	// TODO enums in GR_Font and set *pff and *pft.

	*pff = FF_Unknown;
	*pfp = FP_Unknown;
	*pbTrueType = true;
}
