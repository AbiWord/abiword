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
#include "gr_Caret.h"

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

			if(charWidth == GR_CW_UNKNOWN)
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

