/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 Tomas Frydrych, <tomas@frydrych.uklinux.net>
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

#include <ctype.h>
#include <math.h>
#include <string.h>

#include "xap_App.h"
#include "xap_Prefs.h"
#include "gr_Graphics.h"
#include "gr_CharWidths.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_units.h"
#include "ut_sleep.h"
#include "ut_growbuf.h"
#include "ut_debugmsg.h"
#include "ut_OverstrikingChars.h"
#include "ut_TextIterator.h"
#include "gr_ContextGlyph.h"
#include "gr_Caret.h"
#include <fribidi.h>

// static class member initializations
UT_uint32 GR_Font::s_iAllocCount = 0;

GR_Font::GR_Font()
	:m_pCharWidths(NULL)
{
	s_iAllocCount++;
	m_iAllocNo = s_iAllocCount;
}

GR_Font::~GR_Font()
{
	// need this so children can clean up
}

/*!
  Return the hash key used by the cache to fetch the font
  This method may be overridden to compute it in real time if needed
 */
const UT_String & GR_Font::hashKey(void) const
{
	return m_hashKey;
}

/*!
	Return the char width from the cache.
	Compute the width if needed, and cache it.
 */
UT_sint32 GR_Font::getCharWidthFromCache (UT_UCSChar c) const
{
	// first of all, handle 0-width spaces ...
	if(c == 0xFEFF || c == 0x200B || c == UCS_LIGATURE_PLACEHOLDER)
		return 0;
	
	UT_sint32	iWidth = GR_CW_UNKNOWN;

	if (m_pCharWidths == NULL) {
		m_pCharWidths = GR_CharWidthsCache::getCharWidthCache()->getWidthsForFont(this);
	}
	iWidth = m_pCharWidths->getWidth(c);
	if (iWidth == GR_CW_UNKNOWN) {
		iWidth = measureUnremappedCharForCache(c);
		m_pCharWidths->setWidth(c, iWidth);
	}

	return iWidth;
};


bool GR_Font::doesGlyphExist(UT_UCS4Char g)
{
	if(getCharWidthFromCache(g) == GR_CW_ABSENT)
	{
		UT_DEBUGMSG(("GR_Font::doesGlyphExist: glyph 0x%04x absent from font\n",g));
		return false;
	}
	
	return true;
}

/*!
	Implements a GR_CharWidths.
	Override if you which to instanciate a subclass.
 */
GR_CharWidths* GR_Font::newFontWidths(void) const
{
	return new GR_CharWidths();
}


GR_Graphics::GR_Graphics()
	: m_pApp(NULL),
	  m_iZoomPercentage(100),
	  m_iFontAllocNo(0),
	  m_pRect(NULL),
	  m_pCaret(NULL),
	  m_bIsPortrait(true),
	  m_bSpawnedRedraw(false),
	  m_bExposePending(false),
	  m_bIsExposedAreaAccessed(false),
	  m_bDontRedraw(false),
	  m_bDoMerge(false),
	  m_iPrevYOffset(0),
	  m_iPrevXOffset(0),
	  m_hashFontCache(19),
	  m_paintCount(0)
{
}

GR_Font* GR_Graphics::findFont(const char* pszFontFamily,
							   const char* pszFontStyle,
							   const char* pszFontVariant,
							   const char* pszFontWeight,
							   const char* pszFontStretch,
							   const char* pszFontSize)
{
	GR_Font * pFont = NULL;

	// NOTE: we currently favor a readable hash key to make debugging easier
	// TODO: speed things up with a smaller key (the three AP pointers?)
	UT_String key;

	UT_String_sprintf(key,"%s;%s;%s;%s;%s;%s",pszFontFamily, pszFontStyle, pszFontVariant, pszFontWeight, pszFontStretch, pszFontSize);
	const void *pEntry = m_hashFontCache.pick(key.c_str());
	if (!pEntry)
	{
		// TODO -- note that we currently assume font-family to be a single name,
		// TODO -- not a list.  This is broken.

		pFont = _findFont(pszFontFamily, pszFontStyle, pszFontVariant, pszFontWeight, pszFontStretch, pszFontSize);
		UT_ASSERT(pFont);

		// add it to the cache
		m_hashFontCache.insert(key.c_str(),
							   const_cast<void *>(static_cast<const void *>(pFont)));
	}
	else
	{
		pFont = (GR_Font*)(pEntry);
	}
	return pFont;
}

GR_Graphics::~GR_Graphics()
{
	DELETEP(m_pCaret);
}

void GR_Graphics::_destroyFonts ()
{
	UT_HASH_PURGEDATA(GR_Font *, &m_hashFontCache, delete);
	m_hashFontCache.clear ();
}

void GR_Graphics::beginPaint ()
{
	if (m_paintCount == 0)
		_beginPaint ();

	m_paintCount++;
}

void GR_Graphics::endPaint ()
{
	m_paintCount--;

	if (m_paintCount == 0)
		_endPaint ();
}

UT_sint32 GR_Graphics::tdu(UT_sint32 layoutUnits) const
{
	double d = ((double)layoutUnits * static_cast<double>(getDeviceResolution())) * static_cast<double>(getZoomPercentage()) / (100. * static_cast<double>(getResolution()));
	return (UT_sint32)d;
}

/*!
 * This method converts to device units while taking account of the X-scroll
 * offset. This will always give the exact same logical location on the screen
 * no matter what the X-scroll offset is. This fixes off-by-1-pixel bugs in X.
 */
UT_sint32 GR_Graphics::_tduX(UT_sint32 layoutUnits) const
{
	return tdu(layoutUnits+getPrevXOffset()) - tdu(getPrevXOffset());
}

/*!
 * This method converts to device units while taking account of the Y-scroll
 * offset. This will always give the exact same logical location on the screen
 * no matter what the Y-scroll offset is. This fixes off-by-1-pixel bugs in Y.
 */
UT_sint32 GR_Graphics::_tduY(UT_sint32 layoutUnits) const
{
	return tdu(layoutUnits+getPrevYOffset()) - tdu(getPrevYOffset());
}

/*!
 * This method converts rectangle widths and heights to device units while 
 * taking account rounding down errors.
 * This fixes off-by-1-pixel-bugs in Rectangle widths and heights.
 */
UT_sint32 GR_Graphics::_tduR(UT_sint32 layoutUnits) const
{
	UT_sint32 idh = tdu(layoutUnits);
	if(tlu(idh) < layoutUnits)
	{
		idh += 1;
	}
	return idh;
}

UT_sint32 GR_Graphics::tlu(UT_sint32 deviceUnits) const
{
	return static_cast<UT_sint32>((static_cast<double>(deviceUnits) * static_cast<double>(getResolution()) / static_cast<double>(getDeviceResolution())) * 100. / static_cast<double>(getZoomPercentage()));  //was +0.5
}

double GR_Graphics::tduD(double layoutUnits) const
{
	return (layoutUnits * static_cast<double>(getDeviceResolution()) / static_cast<double>(getResolution())) * static_cast<double>(getZoomPercentage()) / 100.0;
}

double GR_Graphics::tluD(double deviceUnits) const
{
	return (deviceUnits * static_cast<double>(getResolution()) / static_cast<double>(getDeviceResolution())) * 100.0 / static_cast<double>(getZoomPercentage());
}

UT_sint32	GR_Graphics::ftlu(UT_sint32 fontUnits) const
{
	UT_sint32 itmp = fontUnits * (UT_sint32)getResolution();
	return (itmp/ (UT_sint32)getDeviceResolution());
}

double	GR_Graphics::ftluD(double fontUnits) const
{
	return (fontUnits * static_cast<double>(getResolution()) / static_cast<double>(getDeviceResolution()));
}

void GR_Graphics::setLineProperties ( double    inWidthPixels, 
				      JoinStyle inJoinStyle,
				      CapStyle  inCapStyle,
				      LineStyle inLineStyle )
{
  UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

UT_uint32 GR_Graphics::getMaxCharacterWidth(const UT_UCSChar*s, UT_uint32 Length)
{
	UT_GrowBufElement *pWidths = new UT_GrowBufElement[Length];

	measureString(s, 0, Length, pWidths);

	UT_sint32 MaxWidth = 0;

	for(UT_uint32 i = 0; i < Length; i++)
	{
		if(pWidths[i] > MaxWidth)
			MaxWidth = pWidths[i];
	}

	DELETEPV(pWidths);

	return MaxWidth;
}

UT_uint32 GR_Graphics::measureString(const UT_UCSChar* s, int iOffset,
										 int num,  UT_GrowBufElement* pWidths)
{
	// Generic base class version defined in terms of measureUnRemappedChar().
	// Platform versions can roll their own if it makes a performance difference.
	UT_ASSERT(s);

	UT_sint32 stringWidth = 0, charWidth;
	for (int i = 0; i < num; i++)
    {
		UT_UCSChar currentChar = s[i + iOffset];

		{
			charWidth = measureUnRemappedChar(currentChar);

			if(charWidth == GR_CW_UNKNOWN || charWidth ==GR_CW_ABSENT)
				charWidth = 0;
			else if(UT_isOverstrikingChar(currentChar) != UT_NOT_OVERSTRIKING && charWidth > 0)
				charWidth = -charWidth;
			
			// if the widths is < 0 we are dealing with an
			// overstriking character, which does not count for
			// the overall width
			if(charWidth > 0)
				stringWidth += charWidth;
		}

		if (pWidths)
			pWidths[i] = charWidth;
    }
	return stringWidth;
}


void GR_Graphics::setZoomPercentage(UT_uint32 iZoom)
{
	UT_ASSERT(iZoom > 0);

	m_iZoomPercentage = iZoom;

	// invalidate stored font allocation number (change of zoom
	// requires font of different size to be loaded into the device
	// context)
	m_iFontAllocNo = 0xffffffff;
}

UT_uint32 GR_Graphics::getZoomPercentage(void) const
{
	return m_iZoomPercentage;
}

const char * GR_Graphics::invertDimension(UT_Dimension dim, double dValue) const
{
	// return pointer to static buffer -- use it quickly.

	double dInches = dValue / UT_LAYOUT_RESOLUTION;

	return UT_convertInchesToDimensionString( dim, dInches);
}

bool GR_Graphics::scaleDimensions(const char * szLeftIn, const char * szWidthIn,
									 UT_uint32 iWidthAvail,
									 UT_sint32 * piLeft, UT_uint32 * piWidth) const
{
	/* Scale the given left-offset and width using the width available.
	** Compute the actual left-offset and actual width used.
	** We allow the given left-offset to be a number.
	** We allow the given width to be a number or "*"; where "*" indicates
	** we take all remaining space available.
	**
	** NOTE: This routine can also be used for vertical calculations.
	*/

	UT_ASSERT(szLeftIn);
	UT_ASSERT(szWidthIn);

	UT_sint32 iLeft = UT_convertToLogicalUnits(szLeftIn);
	UT_uint32 iWidth;

	if (szWidthIn[0] == '*')
		iWidth = iWidthAvail - iLeft;
	else
		iWidth = UT_convertToLogicalUnits(szWidthIn);

	if (piLeft)
		*piLeft = iLeft;
	if (piWidth)
		*piWidth = iWidth;

	return true;
}

void GR_Graphics::flush(void)
{
	// default implementation does nothing
}
/*!
 * Draw the specified image at the location specified in local units 
 * (xDest,yDest). xDest and yDest are in logical units.
 */
void GR_Graphics::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
   if (pImg)
     pImg->render(this, xDest, yDest);
}

/*!
 * Create a new image from the Raster rgba byte buffer defined by pBB.
 * The dimensions of iWidth and iHeight are in logical units but the image
 * doesn't scale if the resolution or zoom changes. Instead you must create
 * a new image.
 */
GR_Image* GR_Graphics::createNewImage(const char* pszName, const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, GR_Image::GRType iType)
{
   GR_VectorImage * vectorImage = NULL;

   if (iType == GR_Image::GRT_Unknown) {
      if (GR_Image::getBufferType(pBB) == GR_Image::GRT_Vector)
	vectorImage = new GR_VectorImage(pszName);
   }
   else if (iType == GR_Image::GRT_Vector) {
      vectorImage = new GR_VectorImage(pszName);
   }

   if (vectorImage) {
      vectorImage->convertFromBuffer(pBB, iDisplayWidth, iDisplayHeight);
   }

   return vectorImage;
}

bool GR_Graphics::_PtInPolygon(UT_Point * pts,UT_uint32 nPoints,UT_sint32 x,UT_sint32 y)
{
    UT_uint32 i,j;
    bool bResult = false;
    for (i = 0,j = nPoints - 1;i < nPoints;j = i++){
        if ((((pts[i].y <= y) && (y < pts[j].y)) || ((pts[j].y <= y) && (y < pts[i].y))) &&
            (x < (pts[j].x - pts[i].x) * (y - pts[i].y) / (pts[j].y - pts[i].y) + pts[i].x))
        {
            bResult = !bResult;
        }
    }
    return (bResult);
}

void GR_Graphics::polygon(UT_RGBColor& c,UT_Point *pts,UT_uint32 nPoints)
{
    UT_sint32 minX,maxX,minY,maxY,x,y;
    minX = maxX = pts[0].x;
    minY = maxY = pts[0].y;
    for(UT_uint32 i = 0;i < nPoints - 1;i++){
        minX = UT_MIN(minX,pts[i].x);
        maxX = UT_MAX(maxX,pts[i].x);
        minY = UT_MIN(minY,pts[i].y);
        maxY = UT_MAX(maxY,pts[i].y);
    }
    for(x = minX;x <= maxX;x++){
        for(y = minY;y <= maxY;y++){
            if(_PtInPolygon(pts,nPoints,x,y)){
                fillRect(c,x,y,1,1);
            }
        }
    }
 }

/*!
 * Hand shaking variables to let the App know when expose events are being
 * handled.
 * Returns true if the exposed redraw method is running.
 */
const bool  GR_Graphics::isSpawnedRedraw(void) const
{
	return m_bSpawnedRedraw;
}

/*!
 * sets/clears the redraw running variable
 */
void  GR_Graphics::setSpawnedRedraw( bool exposeState)
{
	m_bSpawnedRedraw = exposeState;
}

/*!
 * Informs if there are unprocessed expose information present
 * Returns true is so.
 */
const bool GR_Graphics::isExposePending(void) const
{
	return m_bExposePending;
}

/*!
 * sets the exposed Pending state.
 */
void GR_Graphics::setExposePending(bool exposeState)
{
	m_bExposePending = exposeState;
	if(!exposeState)
	{
		m_PendingExposeArea = m_RecentExposeArea;
	}
}

/*!
 * Sets the Most recent expose rectangle to something sane after repaint.
 *
 */
void GR_Graphics::setRecentRect(UT_Rect * pRect)
{
	m_RecentExposeArea = *pRect;
}


/*!
 * Informs if a process is accessing the global merged expose area.
 * Returns true is so.
 */
const bool GR_Graphics::isExposedAreaAccessed(void) const
{
	return m_bIsExposedAreaAccessed;
}

/*!
 * sets the varaible explaining the state of the Merged area
 */
void GR_Graphics::setExposedAreaAccessed(bool exposeState)
{
	m_bIsExposedAreaAccessed = exposeState;
}

/*!
 * Methods to manipulate the expose rectangle.
 */

/*!
 * Set values inside the PendingArea Rectangle
\param x the x-coord of the upper left corner.
\param y the y-coord of the upper left corner.
\param width the width of the rectangle.
\param height the height of the rectangle.
*/
void  GR_Graphics::setPendingRect( UT_sint32 x, UT_sint32 y, UT_sint32 width, UT_sint32 height)
{
	m_PendingExposeArea.set(x,y,width,height);
}

/*!
 * Do a union of the current rectangle with the one presented in the
 * parameter list. The makes the new rectangle the smallest possible that
 * covers both rectangles.
\param  UT_Rect * pRect pointer to the rectangle to merge with.
*/
void  GR_Graphics::unionPendingRect( UT_Rect * pRect)
{
	m_PendingExposeArea.unionRect(pRect);
}

/*!
\returns a const pointer to the PendingExposeArea.
*/
const UT_Rect * GR_Graphics::getPendingRect(void) const
{
	return & m_PendingExposeArea;
}

/*
 * Hand shaking fail safe variable sto make sure we don'tr repaint till
 * we're absolueltely ready.
 */
void  GR_Graphics::setDontRedraw(bool bDontRedraw)
{
	m_bDontRedraw = bDontRedraw;
}

/*
 * Hand shaking fail safe variable sto make sure we don'tr repaint till
 * we're absolueltely ready.
 */
bool GR_Graphics::isDontRedraw(void)
{
	return m_bDontRedraw;
}

/*!
 * Alternate method to tell the doRepaint to merge the next expose so that
 * expands of exposed area due to scrolls can be merged without doing a
 * display update.
 \returns the doMerge boolean (set from scroll)
*/
bool GR_Graphics::doMerge(void) const
{
	return m_bDoMerge;
}

/*!
 * Sets the do merge boolean
 * Set for scroll, clear from doRepaint
 */
void GR_Graphics::setDoMerge( bool bMergeState)
{
	m_bDoMerge = bMergeState;
}

/*!
 * Method to handle expose events with a background repainter. Events that
 * Pass through here are rapidly delt with by either expanding an
 * already existing expose rectangle to cover the expose rectangle of
 *  the current event or if there is no pending expose rectangle, because
 *  the background repainter has cleared it, set a new expose rectangle.
\params UT_Rect *rClip the rectangle of the expose event.
*/
void GR_Graphics::doRepaint( UT_Rect * rClip)
{
//
// Look if we have a pending expose left over.
//
	xxx_UT_DEBUGMSG(("SEVIOR: Starting doRepaint \n"));
	while(isSpawnedRedraw())
	{
		UT_usleep(100);
	}
//
// Stop the repainter
//
	setDontRedraw(true);
//
// Get a lock on the expose rectangle
//
	while(isExposedAreaAccessed())
	{
		UT_usleep(10); // 10 microseconds
	}
	setExposedAreaAccessed(true);
	if(isExposePending() || doMerge())
	{
		//
        // If so merge in the current expose area
        //
		xxx_UT_DEBUGMSG(("Doing a union in expose handler\n"));
		unionPendingRect( rClip);
		setRecentRect(rClip);
		setDoMerge(false);
	}
	else
	{
//
// Otherwise Load the current expose area into the redraw area.
//
		xxx_UT_DEBUGMSG(("Setting Exposed Area in expose handler \n"));
		setPendingRect(rClip->left,rClip->top,rClip->width,rClip->height);
		setRecentRect(rClip);
  	}
//
// Release expose rectangle lock
//
	setExposedAreaAccessed(false);
//
// Tell the repainter there is something to repaint.
//
	setExposePending(true);
//
// Allow the repainter to paint
//
	setDontRedraw(false);
	xxx_UT_DEBUGMSG(("SEVIOR: Finished doRepaint \n"));
//
// OK this event is handled.
//
}

/*!
 * This method fills the distination rectangle with a piece of the image pImg.
 * The size and location of the piece of the image is defined by src. 
 * src and dest are in logical units.
*/
void GR_Graphics::fillRect(GR_Image * pImg, const UT_Rect & src, const UT_Rect & dest)
{
	GR_Image * pImageSection = pImg->createImageSegment(this, src);
	UT_return_if_fail(pImageSection);
	drawImage(pImageSection,dest.left,dest.top);
	delete pImageSection;
}
/*!
 * Fill the specified rectangle with color defined by "c". The dimensions
 * of UT_Rect are in logical units.
 */
void GR_Graphics::fillRect(const UT_RGBColor& c, const UT_Rect &r)
{
	fillRect(c, r.left, r.top, r.width, r.height);
}
#if XAP_DONTUSE_XOR
#else
void GR_Graphics::xorRect(UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
	xorLine(x,     y,     x + w, y);
	xorLine(x + w, y,     x + w, y + h);
	xorLine(x + w, y + h, x,     y + h);
	xorLine(x,     y + h, x,     y);
}

void GR_Graphics::xorRect(const UT_Rect& r)
{
	xorRect(r.left, r.top, r.width, r.height);
}

#endif

/*
    this is a default implementation that only deals with bidi
    reordering

    derrived classes can either provide entirely different
    implementation, or call the default implementation first and then
    further divide the results into items with same shaping needs
 */

bool GR_Graphics::itemize(UT_TextIterator & text, GR_Itemization & I)
{
	UT_return_val_if_fail(text.getStatus() == UTIter_OK, false);
	
	I.clear();
	UT_uint32 iCurOffset = 0, iLastOffset = 0;
	UT_uint32 iPosStart = text.getPosition();

	// the main loop that will span the whole text of the iterator
	while(text.getStatus() == UTIter_OK)
	{
		FriBidiCharType iPrevType, iNextType, iLastStrongType = FRIBIDI_TYPE_UNSET, iType;
		
		UT_UCS4Char c = text.getChar();
		
		UT_return_val_if_fail(text.getStatus() == UTIter_OK, false);
		
		iType = fribidi_get_type(static_cast<FriBidiChar>(c));

		UT_uint32 i = 1;

		//we have to break the text into chunks that each will go into a
		//separate run in a manner that will ensure that the text will
		//be correctly processed later. The most obvious way is to
		//break every time we encounter a change of directional
		//properties. Unfortunately that means breaking at each white
		//space, which adds a huge amount of processing due to
		//allocating and deleting runs when loading a
		//document. The following code tries to catch out the obvious
		//cases when the span can remain intact. Tomas, Jan 28, 2003

		// remember where we are ...
		iCurOffset = iLastOffset = text.getPosition();
		++text;
		
		// this loop will cover a single homogenous item
		while(text.getStatus() == UTIter_OK)
		{
			iPrevType = iType;
			if(FRIBIDI_IS_STRONG(iType))
				iLastStrongType = iType;
			
			c = text.getChar();
			UT_return_val_if_fail(text.getStatus() == UTIter_OK, false);

			// remember the offset
			iLastOffset = text.getPosition();
			++text;
			
			iType = fribidi_get_type(static_cast<FriBidiChar>(c));
			if(iType != iPrevType)
			{
				// potential direction boundary see if we can ignore
				// it
				bool bIgnore = false;
#if 0
				// this assumption is not true; for instance in the
				// sequence ") " the parenthesis and the space can
				// resolve to different directions
				// 
				// I am leaving it here so that I do not add it one
				// day again (Tomas, Apr 10, 2003)
				
				if(FRIBIDI_IS_NEUTRAL(iPrevType) && FRIBIDI_IS_NEUTRAL(iType))
				{
					// two neutral characters in a row will have the same
					// direction
					xxx_UT_DEBUGMSG(("GR_Graphics::itemize: ntrl->ntrl (c=0x%04x)\n",c));
					bIgnore = true;
				}
				else
#endif
				if(FRIBIDI_IS_STRONG(iPrevType) && FRIBIDI_IS_NEUTRAL(iType))
				{
					// we can ignore a neutral character following a
					// strong one if it is followed by a strong
					// character of identical type to the previous one
					xxx_UT_DEBUGMSG(("GR_Graphics::itemize: strong->ntrl (c=0x%04x)\n",c));
					
					// take a peek at what follows
					UT_uint32 iOldPos = text.getPosition();
					
					while(text.getStatus() == UTIter_OK)
					{
						UT_UCS4Char c = text.getChar();
						UT_return_val_if_fail(text.getStatus() == UTIter_OK, false);

						++text;
						
						iNextType = fribidi_get_type(static_cast<FriBidiChar>(c));
						xxx_UT_DEBUGMSG(("GR_Graphics::itemize: iNextType 0x%04x\n", iNextType));
						
						if(iNextType == iPrevType)
						{
							bIgnore = true;
							break;
						}

						// if the next character is strong, we cannot
						// ignore the boundary
						if(FRIBIDI_IS_STRONG(iNextType))
							break;
					}

					// restore position
					text.setPosition(iOldPos);
				}
				else if(FRIBIDI_IS_NEUTRAL(iPrevType) && FRIBIDI_IS_STRONG(iType))
				{
					// a neutral character followed by a strong one -- we
					// can ignore it, if the neutral character was
					// preceeded by a strong character of the same
					// type
					if(iType == iLastStrongType)
					{
						bIgnore = true;
					}
					xxx_UT_DEBUGMSG(("GR_Graphics::itemize: ntrl->strong (c=0x%04x)\n",c));
				}
				else
				{
					// in all other cases we will split
					xxx_UT_DEBUGMSG(("GR_Graphics::itemize: other (c=0x%04x)\n",pSpan[i]));
				}

				xxx_UT_DEBUGMSG(("GR_Graphics::itemize: bIgnore %d\n",static_cast<UT_uint32>(bIgnore)));
				if(!bIgnore)
					break;
			}
			
		}

		I.addItem(iCurOffset - iPosStart, GRScriptType_Undefined);
	}

	// add an extra record of type Void to allow for calculation of
	// length of the last item
	// iLastOffset is the offset of the last valid character; the Void
	// offset is one beyond that
	I.addItem(iLastOffset - iPosStart + 1, GRScriptType_Void);
	return true;
}

// process information encapsulated by si and store results in ri
bool GR_Graphics::shape(GR_ShapingInfo & si, GR_RenderInfo *& pri)
{
	if(si.m_Type == GRScriptType_Void)
		return false;

	if(!pri)
	{
		pri = new GR_XPRenderInfo(si.m_Type);
		UT_return_val_if_fail(pri, false);
	}

	GR_XPRenderInfo * pRI = (GR_XPRenderInfo *)pri;
	GR_ContextGlyph  cg;

	
	// make sure that the buffers are of sufficient size ...
	if(si.m_iLength > pRI->m_iBufferSize) //buffer too small, reallocate
	{
		delete[] pRI->m_pChars;
		delete[] pRI->m_pAdvances;
			
		pRI->m_pChars = new UT_UCS4Char[si.m_iLength + 1];
		UT_return_val_if_fail(pRI->m_pChars, false);

		pRI->m_pAdvances = new UT_sint32[si.m_iLength + 1];
		UT_return_val_if_fail(pRI->m_pAdvances, false);

		pRI->m_iBufferSize = si.m_iLength + 1;
	}

	pRI->m_iLength = si.m_iLength;
	pRI->m_eScriptType = si.m_Type;
	
	if(si.m_eShapingRequired == GRSR_None)
	{
		// our run only contains non-shaping, non-ligating
		// characters, we will process it using the much faster
		// copyString()
		pRI->m_eShapingResult = cg.copyString(si.m_Text,pRI->m_pChars, si.m_iLength, si.m_pLang,
											  si.m_iVisDir,
											  si.m_isGlyphAvailable, si.m_param);
	}
	else
	{
		pRI->m_eShapingResult = cg.renderString(si.m_Text,pRI->m_pChars, si.m_iLength,si.m_pLang,
												si.m_iVisDir,
												si.m_isGlyphAvailable, si.m_param);
	}

	pRI->m_eState = GRSR_BufferClean;
	
	return (pRI->m_eShapingResult != GRSR_Error);
}

void GR_Graphics::appendRenderedCharsToBuff(GR_RenderInfo & ri, UT_GrowBuf & buf) const
{
	UT_return_if_fail(ri.getType() == GRRI_XP);
	
	GR_XPRenderInfo & RI = (GR_XPRenderInfo &) ri;
	buf.append(reinterpret_cast<UT_GrowBufElement *>(RI.m_pChars),RI.m_iLength);
}

void GR_Graphics::measureRenderedCharWidths(GR_RenderInfo & ri, UT_GrowBufElement* pCharWidths) 
{
	UT_return_if_fail(ri.getType() == GRRI_XP && pCharWidths);

	GR_XPRenderInfo & RI = (GR_XPRenderInfo &) ri;

	bool bReverse = (RI.m_iVisDir == FRIBIDI_TYPE_RTL);

	UT_sint32 j,k;
	UT_uint32 i;

	for (i = 0; i < RI.m_iLength; i++)
	{
		// this is a bit tricky, since we want the resulting width array in
		// logical order, so if we reverse the draw buffer ourselves, we
		// have to address the draw buffer in reverse
		j = bReverse ? RI.m_iLength - i - 1 : i;
		//k = (!bReverse && iVisDirection == FRIBIDI_TYPE_RTL) ? getLength() - i - 1: i;
		k = i + RI.m_iOffset;

		if(k > 0 && *(RI.m_pChars + j) == UCS_LIGATURE_PLACEHOLDER)
		{
			pCharWidths[k]   = pCharWidths[k - 1]/2;
			UT_uint32 mod    = pCharWidths[k-1]%2;
			pCharWidths[k-1] = pCharWidths[k] + mod;
		}
		else
		{

			measureString(RI.m_pChars + j, 0, 1,
										 static_cast<UT_GrowBufElement*>(pCharWidths) + k);

			UT_uint32 iCW = pCharWidths[k] > 0 ? pCharWidths[k] : 0;
		}
	}
	
}


bool GR_XPRenderInfo::append(GR_RenderInfo &ri, bool bReverse)
{
	GR_XPRenderInfo & RI = (GR_XPRenderInfo &) ri;
	
	if((m_iBufferSize <= m_iLength + RI.m_iLength) || (bReverse && (m_iLength > RI.m_iLength)))
	{
		xxx_UT_DEBUGMSG(("GR_RenderInfo::append: reallocating span buffer\n"));
		m_iBufferSize = m_iLength + RI.m_iLength + 1;
		UT_UCS4Char * pSB = new UT_UCS4Char[m_iBufferSize];
		UT_UCS4Char * pAB = new UT_UCS4Char[m_iBufferSize];
		
		UT_return_val_if_fail(pSB && pAB, false);
		
		if(bReverse)
		{
			UT_UCS4_strncpy(pSB, RI.m_pChars, RI.m_iLength);
			UT_UCS4_strncpy(pSB + RI.m_iLength, m_pChars, m_iLength);

			UT_UCS4_strncpy(pAB, (UT_UCS4Char*)RI.m_pAdvances, RI.m_iLength);
			UT_UCS4_strncpy(pAB + RI.m_iLength, (UT_UCS4Char*)m_pAdvances, m_iLength);
		}
		else
		{
			UT_UCS4_strncpy(pSB,m_pChars, m_iLength);
			UT_UCS4_strncpy(pSB + m_iLength, RI.m_pChars, RI.m_iLength);

			UT_UCS4_strncpy(pAB,(UT_UCS4Char*)m_pAdvances, m_iLength);
			UT_UCS4_strncpy(pAB + m_iLength, (UT_UCS4Char*)RI.m_pAdvances, RI.m_iLength);
		}

		*(pSB + m_iLength + RI.m_iLength) = 0;
		delete [] m_pChars;
		delete [] m_pAdvances;
		
		m_pChars = pSB;
		m_pAdvances = (UT_sint32*)pAB;
	}
	else
	{
		UT_DEBUGMSG(("fp_TextRun::mergeWithNext: reusing existin span buffer\n"));
		if(bReverse)
		{
			// can only shift the text directly in the existing buffer if
			// getLength() <= pNext->getLength()
			UT_return_val_if_fail(m_iLength <= RI.m_iLength, false);
			UT_UCS4_strncpy(m_pChars + RI.m_iLength, m_pChars, m_iLength);
			UT_UCS4_strncpy(m_pChars, RI.m_pChars, RI.m_iLength);

			UT_UCS4_strncpy((UT_UCS4Char*)m_pAdvances + RI.m_iLength,
							(UT_UCS4Char*)m_pAdvances, m_iLength);
			
			UT_UCS4_strncpy((UT_UCS4Char*)m_pAdvances,
							(UT_UCS4Char*)RI.m_pAdvances, RI.m_iLength);
		}
		else
		{
			UT_UCS4_strncpy(m_pChars + m_iLength, RI.m_pChars, RI.m_iLength);

			UT_UCS4_strncpy((UT_UCS4Char*)m_pAdvances + m_iLength,
							(UT_UCS4Char*)RI.m_pAdvances, RI.m_iLength);
		}
		*(m_pChars + m_iLength + RI.m_iLength) = 0;
	}

	return true;
}

bool  GR_XPRenderInfo::split (GR_RenderInfo *&pri, UT_uint32 offset, bool bReverse)
{
	UT_ASSERT( !pri );
	pri = new GR_XPRenderInfo(m_eScriptType);
	UT_return_val_if_fail(pri, false);

	GR_XPRenderInfo * pRI = (GR_XPRenderInfo *)pri;
	
	UT_uint32 iPart2Len = m_iLength - offset;
	UT_uint32 iPart1Len = m_iLength - iPart2Len;

	m_iLength = iPart1Len;
	pRI->m_iLength = iPart2Len;

	UT_UCS4Char * pSB = new UT_UCS4Char[m_iLength + 1];
	UT_UCS4Char * pAB = new UT_UCS4Char[m_iLength + 1];
	UT_return_val_if_fail(pSB && pAB, false);
	m_iBufferSize = iPart1Len;
	
	pRI->m_pChars = new UT_UCS4Char[iPart2Len + 1];
	pRI->m_pAdvances = (UT_sint32*)new UT_UCS4Char[iPart2Len + 1];
	UT_return_val_if_fail(pRI->m_pChars && pRI->m_pAdvances, false);
	pRI->m_iBufferSize = iPart2Len;
	
	
	if(bReverse)
	{
		UT_UCS4_strncpy(pSB, m_pChars + pRI->m_iLength, m_iLength);
		UT_UCS4_strncpy(pRI->m_pChars, m_pChars, pRI->m_iLength);

		UT_UCS4_strncpy(pAB, (UT_UCS4Char*)m_pAdvances + pRI->m_iLength, m_iLength);
		UT_UCS4_strncpy((UT_UCS4Char*)pRI->m_pAdvances,
						(UT_UCS4Char*)m_pAdvances, pRI->m_iLength);
	}
	else
	{
		UT_UCS4_strncpy(pSB, m_pChars, m_iLength);
		UT_UCS4_strncpy(pRI->m_pChars, m_pChars + m_iLength, pRI->m_iLength);

		UT_UCS4_strncpy(pAB,(UT_UCS4Char*)m_pAdvances, m_iLength);
		UT_UCS4_strncpy((UT_UCS4Char*)pRI->m_pAdvances,
						(UT_UCS4Char*)m_pAdvances + m_iLength, pRI->m_iLength);
	}

	pSB[m_iLength] = 0;
	
	pRI->m_pChars[pRI->m_iLength] = 0;

	delete[] m_pChars;
	m_pChars = pSB;

	delete[] m_pAdvances;
	m_pAdvances = (UT_sint32*)pAB;

	pRI->m_eShapingResult = m_eShapingResult;
	
	return true;
}

/*
   remove section of legth iLen starting at offset from any chaches ...
   return value false indicates that simple removal was not possible
   and the caller needs to re-shape.
*/
bool GR_XPRenderInfo::cut(UT_uint32 offset, UT_uint32 iLen, bool bReverse)
{
	UT_return_val_if_fail(m_pText, false);
	
	// ascertain the state of the buffer and our shaping requirenments ...
	bool bRefresh = (((UT_uint32)m_eState & (UT_uint32)m_eShapingResult ) != 0);

	if(bRefresh)
		return false;
	
	bool bLigatures = (((UT_uint32)m_eShapingResult & (UT_uint32) GRSR_Ligatures) != 0);
	bool bContext = (((UT_uint32)m_eShapingResult & (UT_uint32) GRSR_ContextSensitive) != 0);

	GR_ContextGlyph cg;
	UT_UCS4Char c;

	UT_uint32 pos = m_pText->getPosition();

	if(!bRefresh && bLigatures)
	{
		// we need to recalculate the draw buffer if the character
		// left of the deleted section is susceptible to ligating or
		// if the two characters around the right edge of the deletion
		// form a ligagure

		// start with the right boundary, as that is computationally
		// easier
		if(offset + iLen < m_iLength)
		{
			// the easiest way of checking for presence of ligature
			// glyph is to check for the presence of the placeholder
			UT_uint32 off2  = offset + iLen;

			if(m_iVisDir == FRIBIDI_TYPE_RTL)
			{
				off2 = m_iLength - off2 - 1;
			}

			bRefresh |= (m_pChars[off2] == UCS_LIGATURE_PLACEHOLDER);
		}

		// now the left boundary
		if(!bRefresh && offset > 0)
		{
			m_pText->setPosition(pos + offset - 1);
			if(m_pText->getStatus() == UTIter_OK)
			{
				c = m_pText->getChar();
				bRefresh |= !cg.isNotFirstInLigature(c);
			}
		}
	}

	if(!bRefresh && bContext)
	{
		// we need to retrieve the characters left and right of the
		// deletion
		if(offset > 0)
		{
			m_pText->setPosition(pos + offset - 1);
			if(m_pText->getStatus() == UTIter_OK)
			{
				c = m_pText->getChar();
				bRefresh |= !cg.isNotContextSensitive(c);
			}
		}

		if(!bRefresh && offset + iLen < m_iLength)
		{
			// this function is called in response to the PT being
			// already changed, i.e., the character that used to be at
			// offset + iLen is now at offset
			m_pText->setPosition(pos + offset);
			if(m_pText->getStatus() == UTIter_OK)
			{
				c = m_pText->getChar();
				bRefresh |= !cg.isNotContextSensitive(c);
			}
		}
	}

	if(bRefresh)
	{
		return false;
	}
	else
	{
		// if we got here, we just need to cut out a bit of the draw
		// buffer
		UT_uint32 iLenToCopy = m_iLength - offset - iLen;

		if(iLenToCopy)
		{
			UT_UCS4Char * d = m_pChars+offset;
			UT_UCS4Char * s = m_pChars+offset+iLen;

			if(m_iVisDir == FRIBIDI_TYPE_RTL)
			{
				d = m_pChars + (m_iLength - (offset + iLen - 1));
				s = m_pChars + (m_iLength - offset);
			}

			UT_UCS4_strncpy(d, s, iLenToCopy);
			m_pChars[m_iLength - iLen] = 0;

			d = (UT_UCS4Char *) m_pAdvances+offset;
			s = (UT_UCS4Char *) m_pAdvances+offset+iLen;

			if(m_iVisDir == FRIBIDI_TYPE_RTL)
			{
				d = (UT_UCS4Char *) m_pAdvances + (m_iLength - offset + iLen - 1);
				s = (UT_UCS4Char *) m_pAdvances + (m_iLength - offset);
			}

			UT_UCS4_strncpy(d, s, iLenToCopy);
			m_pAdvances[m_iLength - iLen] = 0;
			
		}
	}

	return true;
}

/*
   Registers the class allocator and descriptor functions with the
   factory. Returns true on success, false if requested id is already
   registered.

   In case of failure the plugin should try with a different id.

   The default platform implementation of the graphics class should
   register itself twice, once with its predefined id and once with GRID_DEFAULT.
*/
bool GR_GraphicsFactory::registerClass(GR_Graphics * (*allocator)(bool),
									   const char *  (*descriptor)(void),
									   UT_uint32 iClassId)
{
	UT_sint32 indx = m_vClassIds.findItem(iClassId);

	if(indx >= 0)
		return false;

	m_vAllocators.addItem((void*)allocator);
	m_vDescriptors.addItem((void*)descriptor);
	m_vClassIds.addItem((UT_sint32)iClassId);

	return true;
}

void GR_GraphicsFactory::unregisterClass(UT_uint32 iClassId)
{
	UT_sint32 indx = m_vClassIds.findItem(iClassId);

	if(indx < 0)
		return;

	m_vClassIds.deleteNthItem(indx);
	m_vAllocators.deleteNthItem(indx);
	m_vDescriptors.deleteNthItem(indx);
}
	
GR_Graphics * GR_GraphicsFactory::newGraphics(UT_uint32 iClassId, bool bPrint) const
{
	UT_sint32 indx = m_vClassIds.findItem(iClassId);

	if(indx < 0)
		return NULL;
				
					
	GR_Graphics * (*alloc)(bool)
		= (GR_Graphics * (*)(bool))m_vAllocators.getNthItem(indx);
				
	if(!alloc)
		return NULL;

	return alloc(bPrint);
}

const char *  GR_GraphicsFactory::getClassDescription(UT_uint32 iClassId) const
{
	UT_sint32 indx = m_vClassIds.findItem(iClassId);

	if(indx < 0)
		return NULL;
				
					
	const char * (*descr)(void)
		= (const char * (*)(void))m_vDescriptors.getNthItem(indx);
				
	if(!descr)
		return NULL;

	return descr();
				
}
