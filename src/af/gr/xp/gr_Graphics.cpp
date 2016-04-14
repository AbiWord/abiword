/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include <math.h>
#include <string.h>

#include "xap_App.h"
#include "xap_Prefs.h"
#include "xap_EncodingManager.h"
#include "gr_Graphics.h"
#include "gr_CharWidths.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_std_string.h"
#include "ut_units.h"
#include "ut_sleep.h"
#include "ut_stack.h"
#include "ut_std_map.h"
#include "ut_growbuf.h"
#include "ut_debugmsg.h"
#include "ut_OverstrikingChars.h"
#include "ut_TextIterator.h"
#include "gr_Caret.h"
#include "gr_RenderInfo.h"

// static class member initializations
UT_uint32      GR_Font::s_iAllocCount = 0;
UT_VersionInfo GR_Graphics::s_Version;
UT_UCS4Char    GR_Graphics::s_cDefaultGlyph   = '?';

GR_Font::GR_Font():
	m_eType(GR_FONT_UNSET),
	m_pCharWidths(NULL)
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
const std::string & GR_Font::hashKey(void) const
{
	return m_hashKey;
}

/*!
	Return the char width from the cache.
	Compute the width if needed, and cache it.
 */
UT_sint32 GR_Font::getCharWidthFromCache (UT_UCSChar c) const
{
	// the way GR_CharWidthsCache is implemented will cause problems
	// fro any graphics plugin that wants to use the cache -- we will
	// need to instantiate the cache into a static member of
	// GR_Graphics so that the plugin could get to it without calling
	// the static getCharWidthCache()
#ifndef ABI_GRAPHICS_PLUGIN_NO_WIDTHS
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
#else
	UT_return_val_if_fail(UT_NOT_IMPLEMENTED,0);
#endif
}

bool GR_Font::doesGlyphExist(UT_UCS4Char g) const
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
#ifndef ABI_GRAPHICS_PLUGIN_NO_WIDTHS
	return new GR_CharWidths();
#else
	return NULL;
#endif
}

AllCarets::AllCarets(GR_Graphics * pG,
					 GR_Caret ** pCaret,
					 UT_GenericVector<GR_Caret *>* vecCarets  ):
	m_pG(pG),
	m_pLocalCaret(pCaret),
	m_vecCarets(vecCarets)
{
}
GR_Caret *  AllCarets::getBaseCaret(void)
{
	return *m_pLocalCaret;
}

void	    AllCarets::enable(void)
{
	if(*m_pLocalCaret)
		(*m_pLocalCaret)->enable();
	for(UT_sint32 i =0; i< m_vecCarets->getItemCount();i++)
	{
		m_vecCarets->getNthItem(i)->enable();
	}
			
}

void    AllCarets::JustErase(UT_sint32 xPoint,UT_sint32 yPoint)
{
	if((*m_pLocalCaret))
		(*m_pLocalCaret)->JustErase(xPoint,yPoint);
	for(UT_sint32 i =0; i< m_vecCarets->getItemCount();i++)
	{
		m_vecCarets->getNthItem(i)->JustErase(xPoint,yPoint);
	}
}


void		AllCarets::disable(bool bNoMulti)
{
	if((*m_pLocalCaret))
		(*m_pLocalCaret)->disable(bNoMulti);
	for(UT_sint32 i =0; i< m_vecCarets->getItemCount();i++)
	{
		m_vecCarets->getNthItem(i)->disable(bNoMulti);
	}
}

void		AllCarets::setBlink(bool bBlink)
{
	if((*m_pLocalCaret))
		(*m_pLocalCaret)->setBlink(bBlink);
	for(UT_sint32 i =0; i< m_vecCarets->getItemCount();i++)
	{
		m_vecCarets->getNthItem(i)->setBlink(bBlink);;
	}
}

void        AllCarets::setWindowSize(UT_uint32 width, UT_uint32 height)
{
	if((*m_pLocalCaret))
		(*m_pLocalCaret)->setWindowSize(width, height);
	for(UT_sint32 i =0; i< m_vecCarets->getItemCount();i++)
	{
		m_vecCarets->getNthItem(i)->setWindowSize(width, height);
	}
}

void		AllCarets::setCoords(UT_sint32 x, UT_sint32 y, UT_uint32 h,
						  UT_sint32 x2, UT_sint32 y2, UT_uint32 h2, 
						  bool bPointDirection, 
						  const UT_RGBColor * pClr)
{
	if((*m_pLocalCaret))
		(*m_pLocalCaret)->setCoords(x, y, h, x2, y2, h2, bPointDirection, pClr);
	for(UT_sint32 i =0; i< m_vecCarets->getItemCount();i++)
	{
		m_vecCarets->getNthItem(i)->setCoords(x, y, h, x2, y2, h2, bPointDirection, pClr);
	}
}

void		AllCarets::setInsertMode (bool mode)
{
	if((*m_pLocalCaret))
		(*m_pLocalCaret)->setInsertMode(mode);
	for(UT_sint32 i =0; i< m_vecCarets->getItemCount();i++)
	{
		m_vecCarets->getNthItem(i)->setInsertMode(mode);
	}
}

void		AllCarets::forceDraw(void)
{
	if((*m_pLocalCaret))
		(*m_pLocalCaret)->forceDraw();
	for(UT_sint32 i =0; i< m_vecCarets->getItemCount();i++)
	{
		m_vecCarets->getNthItem(i)->forceDraw();
	}
}




GR_Graphics::GR_Graphics()
	: m_iZoomPercentage(100),
	  m_iFontAllocNo(0),
	  m_pRect(NULL),
	  m_bHave3DColors(false),
	  m_paintCount(0),
	  m_bDoubleBufferingActive(false),
	  m_bDrawingSuspended(false),
	  m_pCaret(NULL),
	  m_bIsPortrait(true),
	  m_bSpawnedRedraw(false),
	  m_bExposePending(false),
	  m_bIsExposedAreaAccessed(false),
	  m_bDontRedraw(false),
	  m_bDoMerge(false),
	  m_iPrevYOffset(0),
	  m_iPrevXOffset(0),
	  m_AllCarets(this,&m_pCaret,&m_vecCarets),
	  m_bAntiAliasAlways(false)
{
}

GR_Font* GR_Graphics::findFont(const char* pszFontFamily,
							   const char* pszFontStyle,
							   const char* pszFontVariant,
							   const char* pszFontWeight,
							   const char* pszFontStretch,
							   const char* pszFontSize,
							   const char* pszLang)
{
	GR_Font * pFont = NULL;

	// NOTE: we currently favor a readable hash key to make debugging easier
	// TODO: speed things up with a smaller key (the three AP pointers?)
	std::string key = UT_std_string_sprintf("%s;%s;%s;%s;%s;%s",pszFontFamily, 
											pszFontStyle, pszFontVariant, 
											pszFontWeight, pszFontStretch, 
											pszFontSize);

	FontCache::const_iterator iter = m_hashFontCache.find(key);
	if (iter == m_hashFontCache.end())
	{
		// TODO -- note that we currently assume font-family to be a single name,
		// TODO -- not a list.  This is broken.

		pFont = _findFont(pszFontFamily, pszFontStyle,
						  pszFontVariant,pszFontWeight,
						  pszFontStretch, pszFontSize,
						  pszLang);
		UT_ASSERT(pFont);
		xxx_UT_DEBUGMSG(("Insert font %x in gr_Graphics cache \n",pFont));
		// add it to the cache
		
		if(pFont)
			m_hashFontCache.insert(std::make_pair(key, pFont));
	}
	else
	{
		pFont = iter->second;
	}
	return pFont;
}

GR_Graphics::~GR_Graphics()
{
	xxx_UT_DEBUGMSG(("Deleting graphics class %x \n",this));
	DELETEP(m_pCaret);
	UT_sint32 i = 0;
	for(i=0; i< m_vecCarets.getItemCount();i++)
	{
		GR_Caret * pCaret = m_vecCarets.getNthItem(i);
		DELETEP(pCaret);
	}
}

bool GR_Graphics::beginDoubleBuffering()
{
	if(m_bDoubleBufferingActive) return false;
	m_DCSwitchManagementStack.push((int)SWITCHED_TO_BUFFER);
	_DeviceContext_SwitchToBuffer();
	m_bDoubleBufferingActive = true;
	return true;
}

void GR_Graphics::endDoubleBuffering(bool token)
{
	// check prerequisites
	if(token == false) return;

	UT_ASSERT(m_DCSwitchManagementStack.getDepth() > 0);
	
	UT_sint32 topMostSwitch;
	m_DCSwitchManagementStack.viewTop(topMostSwitch);
	UT_ASSERT(topMostSwitch == (UT_sint32)SWITCHED_TO_BUFFER);

	_DeviceContext_SwitchToScreen();
	m_DCSwitchManagementStack.pop();
	m_bDoubleBufferingActive = false;
}

bool GR_Graphics::suspendDrawing()
{
	if(m_bDrawingSuspended) return false;
	m_DCSwitchManagementStack.push((int)DRAWING_SUSPENDED);
	_DeviceContext_SuspendDrawing();
	m_bDrawingSuspended = true;
	return true;
}

void GR_Graphics::resumeDrawing(bool token)
{
	// check prerequisites
	if(token == false) return;

	UT_ASSERT(m_DCSwitchManagementStack.getDepth() > 0);
	
	UT_sint32 topMostSwitch;
	m_DCSwitchManagementStack.viewTop(topMostSwitch);
	UT_ASSERT(topMostSwitch == (UT_sint32)DRAWING_SUSPENDED);

	// take action only if the caller has the good token
	_DeviceContext_ResumeDrawing();
	m_DCSwitchManagementStack.pop();
	m_bDrawingSuspended = false;
}

void GR_Graphics::_destroyFonts ()
{
	UT_map_delete_all_second(m_hashFontCache);
	m_hashFontCache.clear ();
}

GR_Caret * GR_Graphics::getNthCaret(UT_sint32 i) const
{
	if (i>= m_vecCarets.getItemCount())
		return NULL;
	return m_vecCarets.getNthItem(i);
}

GR_Caret * GR_Graphics::getCaret(const std::string& sID) const
{
	UT_sint32 i= 0;
	for(i=0; i<m_vecCarets.getItemCount();i++)
	{
		if(m_vecCarets.getNthItem(i)->getID() == sID)
		{
			return m_vecCarets.getNthItem(i);
		}
	}
	return NULL;
}

AllCarets * GR_Graphics::allCarets(void)
{
	return &m_AllCarets;
}

void GR_Graphics::disableAllCarets()
{
	m_AllCarets.disable();
}

void GR_Graphics::enableAllCarets()
{
	m_AllCarets.enable();
}

GR_Caret * GR_Graphics::createCaret(const std::string& sID)
{
	GR_Caret * pCaret = new GR_Caret(this,sID);
	m_vecCarets.addItem(pCaret);
	return pCaret;
}

void GR_Graphics::removeCaret(const std::string& sID)
{
	for(UT_sint32 i = 0; i < m_vecCarets.getItemCount(); i++)
	{
		GR_Caret* pC = m_vecCarets.getNthItem(i);
		if (pC->getID() == sID)
		{			
			DELETEP(pC);
			m_vecCarets.deleteNthItem(i);
		}
	}
}

/*!
 * WARNING! WARNING! WARNING!
 * Only gr_UnixGraphics should call this!
 * Because xap_UnixFontManager "owns" it's fonts, keeps it's own cache of them
 * and will happily destroy them. If it does this, the cache of pointers here
 * is no longer valid.
 * It would be better to simply remove the font from this cache too but the
 * key used in the xap_UnixFontManager class is different fromt the key used
 * here.
 * Until a better solution appears I'll leave this here. Other classes should
 * use _destroyFonts to remove the cache. 
 */
void GR_Graphics::invalidateCache(void)
{
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
	double d = (static_cast<double>(layoutUnits) * static_cast<double>(getDeviceResolution()) * static_cast<double>(getZoomPercentage())) * (1./100. / static_cast<double>(getResolution())) + 0.1;
	return static_cast<UT_sint32>(d);
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
	return static_cast<UT_sint32>((static_cast<double>(deviceUnits) * static_cast<double>(getResolution()) * 100.) / (static_cast<double>(getDeviceResolution()) * static_cast<double>(getZoomPercentage())));
}

double GR_Graphics::tduD(double layoutUnits) const
{
	return (layoutUnits * static_cast<double>(getDeviceResolution()) * static_cast<double>(getZoomPercentage())) / (100.0 * static_cast<double>(getResolution()));
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

void GR_Graphics::setLineProperties ( double    /*inWidthPixels*/, 
									  JoinStyle /*inJoinStyle*/,
									  CapStyle  /*inCapStyle*/,
									  LineStyle /*inLineStyle*/ )
{
  UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void GR_Graphics::getMaxCharacterDimension(const UT_UCSChar*s, UT_uint32 length, UT_uint32 &width, UT_uint32 &height)
{
	UT_GrowBufElement *pWidths = new UT_GrowBufElement[length];


	UT_uint32 maxHeight = 0;
	measureString(s, 0, length, pWidths, &maxHeight);

	UT_sint32 maxWidth = 0;

	for(UT_uint32 i = 0; i < length; i++)
	{
		if(pWidths[i] > maxWidth)
			maxWidth = pWidths[i];
	}

	DELETEPV(pWidths);

	width = maxWidth;
	if (maxHeight > 0) { 
		height = maxHeight;
	}
}

UT_uint32 GR_Graphics::measureString(const UT_UCSChar* s, int iOffset,
										 int num,  UT_GrowBufElement* pWidths, 
									 UT_uint32 * /*height*/)
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
    This method is just like drawChars() except it treats yoff as position of the font
    baseline. The default implementation simply subtracts the ascent of the current font
    from yoff and calls drawChars(), which should work on all platforms except for win32.

    On win32 because of the trickery we use to achieve wysiwyg layout the acent of the
    font we work with is slightly smaller than that of the actual font the system uses to
    draw on the screen. As a result, the characters end up positioned slightly higher than
    they should and this has proved a problem in the math plugin (see screen shots in #9500)
*/
void GR_Graphics::drawCharsRelativeToBaseline(const UT_UCSChar* pChars,
								 int iCharOffset,
								 int iLength,
								 UT_sint32 xoff,
								 UT_sint32 yoff,
								 int* pCharWidths)
{
	drawChars(pChars, iCharOffset, iLength, xoff, yoff - getFontAscent(), pCharWidths);
}


/*!
 * Create a new image from the Raster rgba byte buffer defined by pBB.
 * The dimensions of iWidth and iHeight are in logical units but the image
 * doesn't scale if the resolution or zoom changes. Instead you must create
 * a new image.
 */
GR_Image* GR_Graphics::createNewImage(const char* pszName, const UT_ByteBuf* pBB, const std::string& mimetype,
									  UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, GR_Image::GRType iType)
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
      vectorImage->convertFromBuffer(pBB, mimetype, iDisplayWidth, iDisplayHeight);
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
 * This method fills the distination rectangle with a piece of the image pImg.
 * The size and location of the piece of the image is defined by src. 
 * src and dest are in logical units.
*/
void GR_Graphics::fillRect(GR_Image * pImg, const UT_Rect & src, const UT_Rect & dest)
{
	UT_return_if_fail(pImg);
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

/////////////////////////////////////////////////////////////////////////////////
//
//  COMPLEX SCRIPT PROCESSING FUNCTIONS
//
//

/*!
    itemize() analyses text represented by text, notionally dividing
    it into segments that from the point of the shaper are uniform;
    this notional division is stored in GR_Itemization I.
    
    The default implementation that only deals with bidi reordering

    derrived classes can either provide entirely different
    implementation, or call the default implementation first and then
    further divide the results into items with same shaping needs
 */
#ifndef ABI_GRAPHICS_PLUGIN
bool GR_Graphics::itemize(UT_TextIterator & text, GR_Itemization & I)
{
	UT_return_val_if_fail(text.getStatus() == UTIter_OK, false);
	
	I.clear();
	UT_uint32 iCurOffset = 0, iLastOffset = 0;
	UT_uint32 iPosStart = text.getPosition();

	// the main loop that will span the whole text of the iterator
	while(text.getStatus() == UTIter_OK)
	{
		UT_BidiCharType iPrevType, iLastStrongType = UT_BIDI_UNSET, iType;
		
		UT_UCS4Char c = text.getChar();
		
		UT_return_val_if_fail(text.getStatus() == UTIter_OK, false);

		iType = UT_bidiGetCharType(c);
#if 0
		// this branch of code breaks at all direction bounaries
		// it is disabled because doing that causes bug 8099
		iCurOffset = iLastOffset = text.getPosition();
		++text;
		
		// this loop will cover a single homogenous item
		while(text.getStatus() == UTIter_OK)
		{
			iPrevType = iType;

			c = text.getChar();
			UT_return_val_if_fail(text.getStatus() == UTIter_OK, false);

			// remember the offset
			iLastOffset = text.getPosition();
			
			iType = UT_bidiGetCharType(c);
			if(iType != iPrevType)
			{
				break;
			}

			++text;
		}
#else
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

		UT_BidiCharType iNextType;
		
		// this loop will cover a single homogenous item
		while(text.getStatus() == UTIter_OK)
		{
			iPrevType = iType;
			if(UT_BIDI_IS_STRONG(iType))
				iLastStrongType = iType;
			
			c = text.getChar();
			UT_return_val_if_fail(text.getStatus() == UTIter_OK, false);

			// remember the offset
			iLastOffset = text.getPosition();
			++text;
			
			iType = UT_bidiGetCharType(c);
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
				
				if(UT_BIDI_IS_NEUTRAL(iPrevType) && UT_BIDI_IS_NEUTRAL(iType))
				{
					// two neutral characters in a row will have the same
					// direction
					xxx_UT_DEBUGMSG(("GR_Graphics::itemize: ntrl->ntrl (c=0x%04x)\n",c));
					bIgnore = true;
				}
				else
#endif
				if(UT_BIDI_IS_STRONG(iPrevType) && UT_BIDI_IS_NEUTRAL(iType))
				{
					// we can ignore a neutral character following a
					// strong one if it is followed by a strong
					// character of identical type to the previous one
					xxx_UT_DEBUGMSG(("GR_Graphics::itemize: strong->ntrl (c=0x%04x)\n",c));
					
					// take a peek at what follows
					UT_uint32 iOldPos = text.getPosition();
					
					while(text.getStatus() == UTIter_OK)
					{
						UT_UCS4Char c2 = text.getChar();
						UT_return_val_if_fail(text.getStatus() == UTIter_OK, false);

						++text;
						
						iNextType = UT_bidiGetCharType(c2);
						xxx_UT_DEBUGMSG(("GR_Graphics::itemize: iNextType 0x%04x\n", iNextType));
						
						if(iNextType == iPrevType)
						{
							bIgnore = true;
							break;
						}

						// if the next character is strong, we cannot
						// ignore the boundary
						if(UT_BIDI_IS_STRONG(iNextType))
							break;
					}

					// restore position
					text.setPosition(iOldPos);
				}
				else if(UT_BIDI_IS_NEUTRAL(iPrevType) && UT_BIDI_IS_STRONG(iType))
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
#endif
		
		I.addItem(iCurOffset - iPosStart, new GR_XPItem(GRScriptType_Undefined));
	}

	// add an extra record of type Void to allow for calculation of
	// length of the last item
	// iLastOffset is the offset of the last valid character; the Void
	// offset is one beyond that
	I.addItem(iLastOffset - iPosStart + 1, new GR_XPItem(GRScriptType_Void));
	return true;
}

/*!
    this function remaps a number of specialised glyphs to reasonable
    alternatives
*/
static UT_UCS4Char s_remapGlyph(UT_UCS4Char g)
{
	// various hyphens
	if(g >= 0x2010 && g <= 0x2015) return '-';

	// various quotes
	if(g >= 0x2018 && g <= 0x201b) return '\'';
	if(g == 0x2039) return '<';
	if(g == 0x203a) return '>';
	if(g >= 0x201c && g <= 0x201f) return '\"';

	// bullets
	if(g == 0x2022 || g == 0x2023) return '*';

	// miscell. punctuation
	if(g == 0x2044) return '/';
	if(g == 0x2045) return '[';
	if(g == 0x2046) return ']';
	if(g == 0x2052) return '%';
	if(g == 0x2053) return '~';

	// currency symbols
	if(g == 0x20a3) return 'F';
	if(g == 0x20a4) return 0x00a3;
	if(g == 0x20ac) return 'E';

	// letter like symbols
	if(g == 0x2103) return 'C';
	if(g == 0x2109) return 'F';
	if(g == 0x2117) return 0x00a9;
	if(g == 0x2122) return 'T'; // TM symbol, this is not satisfactory, but ...
	if(g == 0x2126) return 0x03a9;
	if(g == 0x212a) return 'K';

	// dingbats
	if(g >= 0x2715 && g <= 0x2718) return 0x00d7;
	if(g >= 0x2719 && g <= 0x2720) return '+';
	if(g == 0x271) return '*';
	if(g >= 0x2722 && g <= 0x2727) return '+';
	if(g >= 0x2728 && g <= 0x274b) return '*';
	if(g >= 0x2758 && g <= 0x275a) return '|';
	if(g == 0x275b || g == 0x275c) return '\'';
	if(g == 0x275d || g == 0x275e) return '\"';
	if(g == 0x2768 || g == 0x276a) return '(';
	if(g == 0x2769 || g == 0x276b) return ')';
	if(g == 0x276c || g == 0x276e || g == 0x2770) return '<';
	if(g == 0x276d || g == 0x276f || g == 0x2771) return '>';
	if(g == 0x2772) return '[';
	if(g == 0x2773) return ']';
	if(g == 0x2774) return '{';
	if(g == 0x2775) return '}';
	if(g >= 0x2776 && g <= 0x2793) return ((g-0x2775)%10 + '0');

	return g;
}

// if the character has a mirror form, returns the mirror form,
// otherwise returns the character itself
static UT_UCSChar s_getMirrorChar(UT_UCSChar c)
{
	//got to do this, otherwise bsearch screws up
	UT_UCS4Char mc;

	if (UT_bidiGetMirrorChar(c,mc))
		return mc;
	else
		return c;
}

/*!
    shape() processes the information encapsulated by GR_ShapingInfo
    si and stores results in GR_*RenderInfo* pri.

    If the contents of pri are NULL the function must create a new
    instance of GR_*RenderInfo of the appropriate type and store the
    pointer in pri; it also must store pointer to this graphics
    instance in pri->m_pGraphics.

    If ri indicates that the text is justified, appropriate processing
    needs to be done

    This function is tied closely together to a class derrived from
    GR_RenderInfo which may contain caches of various data that will
    speed subsequent calls to prepareToRenderChars() and renderChars()
*/
bool GR_Graphics::shape(GR_ShapingInfo & si, GR_RenderInfo *& pri)
{
	if(!si.m_pItem || si.m_pItem->getType() == GRScriptType_Void || !si.m_pFont)
		return false;

	if(!pri)
	{
		pri = new GR_XPRenderInfo(si.m_pItem->getType());
		UT_return_val_if_fail(pri, false);
		pri->m_pGraphics = this;
	}

	GR_XPRenderInfo * pRI = (GR_XPRenderInfo *)pri;

	const GR_Font *pFont = si.m_pFont;
	
	// make sure that the buffers are of sufficient size ...
	if(si.m_iLength > pRI->m_iBufferSize) //buffer too small, reallocate
	{
		delete[] pRI->m_pChars;
		delete[] pRI->m_pWidths;
			
		pRI->m_pChars = new UT_UCS4Char[si.m_iLength + 1];
		UT_return_val_if_fail(pRI->m_pChars, false);

		pRI->m_pWidths = new UT_sint32[si.m_iLength + 1];
		UT_return_val_if_fail(pRI->m_pWidths, false);

		pRI->m_iBufferSize = si.m_iLength + 1;
	}

	pRI->m_iLength = si.m_iLength;
	pRI->m_iTotalLength = si.m_iLength;
	pRI->m_eScriptType = si.m_pItem->getType();
	pRI->m_pItem = si.m_pItem;

	UT_UCS4Char glyph, current;
	UT_UCS4Char * dst_ptr = pRI->m_pChars;
	bool previousWasSpace = si.m_previousWasSpace;

	for(UT_sint32 i = 0; i < si.m_iLength; ++i, ++si.m_Text)
	{
		UT_return_val_if_fail(si.m_Text.getStatus() == UTIter_OK, false);
		current = si.m_Text.getChar();

		if (si.m_TextTransform == GR_ShapingInfo::LOWERCASE)
			current = g_unichar_tolower(current);
		else if (si.m_TextTransform == GR_ShapingInfo::UPPERCASE)
			current = g_unichar_toupper(current);
		else if (si.m_TextTransform == GR_ShapingInfo::CAPITALIZE) {
				if (previousWasSpace) {
					current = g_unichar_toupper(current);
				}
		} // else si.m_TextTransform == GR_ShapingInfo::NONE

		previousWasSpace = g_unichar_isspace(current);

		if(si.m_iVisDir == UT_BIDI_RTL)
			glyph = s_getMirrorChar(current);
		else
			glyph = current;

		if(pFont->doesGlyphExist(glyph))
			*dst_ptr++ = glyph;
		else
		{
			UT_UCS4Char t = s_remapGlyph(glyph);
			if(pFont->doesGlyphExist(t))
			{
				*dst_ptr++ = t;
			}
			else
			{
				*dst_ptr++ = s_cDefaultGlyph;
			}
		}
	}
	
	pRI->m_eState = GRSR_BufferClean;
	
	if(pRI->isJustified())
		justify(*pRI);

	// make sure that we invalidate the static buffers if we own them
	if(pRI->s_pOwner == pRI)
		pRI->s_pOwner = NULL;
	
	return true;
}

void GR_Graphics::appendRenderedCharsToBuff(GR_RenderInfo & ri, UT_GrowBuf & buf) const
{
	UT_return_if_fail(ri.getType() == GRRI_XP);
	
	GR_XPRenderInfo & RI = (GR_XPRenderInfo &) ri;
	buf.append(reinterpret_cast<UT_GrowBufElement *>(RI.m_pChars),RI.m_iLength);
}

UT_sint32 GR_Graphics::getTextWidth(GR_RenderInfo & ri)
{
	UT_return_val_if_fail(ri.getType() == GRRI_XP, 0);
	GR_XPRenderInfo & RI = (GR_XPRenderInfo &) ri;

	// NB: the width array is in VISUAL order, but offset is a logical offset
	bool bReverse = (ri.m_iVisDir == UT_BIDI_RTL);
	
	UT_sint32 iWidth = 0;
	for (UT_sint32 i = ri.m_iOffset; i < ri.m_iLength + ri.m_iOffset; ++i)
	{
		UT_uint32 k = i;

		if(bReverse)
		{
			if((static_cast<UT_sint32>(RI.m_iTotalLength) - i - 1) < 0)
			{
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
				continue;
			}

			k = RI.m_iTotalLength - i - 1;
		}

		UT_uint32 iCW = RI.m_pWidths[k] > 0 ? RI.m_pWidths[k] : 0;
		iWidth += iCW;
	}

	return iWidth;
}


void GR_Graphics::measureRenderedCharWidths(GR_RenderInfo & ri) 
{
	UT_return_if_fail(ri.getType() == GRRI_XP);
	GR_XPRenderInfo & RI = (GR_XPRenderInfo &) ri;
	UT_return_if_fail(RI.m_pWidths);
	
	//bool bReverse = (RI.m_iVisDir == UT_BIDI_RTL);

	UT_sint32 i;

	for (i = 0; i < RI.m_iLength; i++)
	{
		if(i > 0 && *(RI.m_pChars + i) == UCS_LIGATURE_PLACEHOLDER)
		{
			RI.m_pWidths[i]   = RI.m_pWidths[i - 1]/2;
			UT_uint32 mod     = RI.m_pWidths[i-1]%2;
			RI.m_pWidths[i-1] = RI.m_pWidths[i] + mod;
		}
		else
		{

			measureString(RI.m_pChars + i, 0, 1,
					 static_cast<UT_GrowBufElement*>(RI.m_pWidths) + i);
		}
	}

	if(RI.isJustified())
	{
		justify(RI);
	}
	
	// make sure that we invalidate the static buffers if we own them
	if(RI.s_pOwner == &RI)
		RI.s_pOwner = NULL;
	
}

/*!
   prepareToRenderChars() does any preprocessing necessary immediately
   prior to the actual output on screen (which is done by
   renderChars()), and must be always called before renderChars().

   What this function does is entirely dependend of the specific
   shaping engine (and in some cases might not do anthing at all). For
   example, this function might refresh any temporary buffers,
   etc.

   The reason for dividing the actual drawing into two steps (prepare
   and render) is to limit the amount of processing in cases where the
   caller needs to make several calls to renderChars() with the same
   GR_RenderInfo. For example, fp_TextRun::_draw() draws the text in
   one, two or three segments depending whether there is a selection
   and where in the run it is. It will make one call to
   prepareToRenderChars() and then 1-3 calls to renderChars() chaning
   the background and text colour in between those calls.
*/
void GR_Graphics::prepareToRenderChars(GR_RenderInfo & ri)
{
	UT_return_if_fail(ri.getType() == GRRI_XP);
	GR_XPRenderInfo & RI = (GR_XPRenderInfo &)ri;
	RI.prepareToRenderChars();
}

/*!
   renderChars() outputs textual data represented by ri onto the device (screen,
   priter, etc.).

   The output starts at ri.m_iOffset, is ri.m_iLength
   long and the drawing starts at ri.m_xoff, ri.m_yoff
*/
void GR_Graphics::renderChars(GR_RenderInfo & ri)
{
	UT_return_if_fail(ri.getType() == GRRI_XP);
	GR_XPRenderInfo & RI = (GR_XPRenderInfo &)ri;

	drawChars(RI.s_pCharBuff,RI.m_iOffset,RI.m_iLength,RI.m_xoff,RI.m_yoff,RI.s_pAdvances);

	
}

/*!
    return true if linebreak at character c is permissible
*/
bool GR_Graphics::canBreak(GR_RenderInfo & ri, UT_sint32 &iNext, bool bAfter)
{
	UT_UCS4Char c[2];

	// Default to -1.
	iNext = -1; 

	// Check the iterator is OK.
	UT_return_val_if_fail(ri.m_pText && ri.m_pText->getStatus() == UTIter_OK, false);

	// Advance the iterator by the given offset.
	*(ri.m_pText) += ri.m_iOffset;
	// Check that we haven't run off the end of the iterator.
	UT_return_val_if_fail(ri.m_pText->getStatus() == UTIter_OK, false);

	// Fetch a pointer to the Encoding manager.
	UT_return_val_if_fail(XAP_App::getApp(), false);
	const XAP_EncodingManager *encMan =  XAP_App::getApp()->getEncodingManager();
	UT_return_val_if_fail(encMan, false);

	// Set up c[] appropriately depending on whether we're looking
	// for break before or after.
	if (bAfter)
	{
		c[1] = ri.m_pText->getChar();
	}
	else
	{
		--(*ri.m_pText);  
		c[1] = ri.m_pText->getChar();
	}

	// Make sure we managed to get the character we wanted.
	if (c[1] == UT_IT_ERROR)
		return false;

	UT_uint32 iCount = ri.m_iOffset;
	do
	{
		++(*ri.m_pText);
		c[0] = c[1];
		c[1] = ri.m_pText->getChar();

		// If we reach the end of the document then return false
		// (and leave iNext set to -1).
		if (c[1] == UT_IT_ERROR)
			return false;
		iCount++;
	}
	while (!encMan->canBreakBetween(c));

	// Set iNext.
	iNext = iCount - 1;

	// If a break was possible between the first character pair
	// then return true. 
	if (iNext == ri.m_iOffset)
		return true;
	// ...otherwise, return false.
	return false;
}

/*!
   resetJustification() makes the data represented by ri unjustified
   and returns value by which the total width changed as a result such
   that OriginalWidth + ReturnValue = NewWidth (i.e., the return
   value should normally be negative).

   The parameter bPermanent indicates that the resetting is permanent
   and any buffers used to hold justification information can be
   remove, e.g., the paragraph alignment has changed from justified to
   left (in some circumstance the reset can be only temporary and we
   will be asked to subsequently recalculate the justification
   information; in such case it makes sense to keep the buffers in place)
   
*/
UT_sint32 GR_Graphics::resetJustification(GR_RenderInfo & ri, bool /* bPermanent*/)
{
	UT_return_val_if_fail(ri.getType() == GRRI_XP, 0);
	GR_XPRenderInfo & RI = (GR_XPRenderInfo &)ri;

	UT_return_val_if_fail(RI.m_pChars && RI.m_pWidths, 0);
	
	UT_sint32 iAccumDiff = 0;

	if(RI.isJustified())
	{
		UT_sint32 iSpaceWidthBefore = RI.m_iSpaceWidthBeforeJustification;

		if(RI.m_pWidths == NULL)
		{
			return 0;
		}

		for(UT_sint32 i = 0; i < RI.m_iLength; ++i)
		{
			if(RI.m_pChars[i] != UCS_SPACE)
				continue;

			if(RI.m_pWidths[i] != iSpaceWidthBefore)
			{
				iAccumDiff += iSpaceWidthBefore - RI.m_pWidths[i];
				RI.m_pWidths[i] = iSpaceWidthBefore;
			}
		}
		
		RI.m_iSpaceWidthBeforeJustification = 0xfffffff; // note one less 'f'
		RI.m_iJustificationPoints = 0;
		RI.m_iJustificationAmount = 0;

		if(RI.s_pOwner == &RI)
			RI.s_pOwner = NULL;

	}

	return iAccumDiff;
}

/*!
   countJustificationPoints() counts the number of points between
   which any extra justification width could be distributed (in Latin
   text these are typically spaces).spaces in the text;

   The function returns the count as negative value if
   the run contains only blank data (i.e., only spaces in Latin text).
*/
UT_sint32 GR_Graphics::countJustificationPoints(const GR_RenderInfo & ri) const
{
	UT_return_val_if_fail(ri.getType() == GRRI_XP, 0);
	const GR_XPRenderInfo & RI = static_cast<const GR_XPRenderInfo &>(ri);

	UT_return_val_if_fail(RI.m_pChars, 0);

	UT_sint32 iCount = 0;
	bool bNonBlank = false;

	for(UT_sint32 i = (UT_sint32)RI.m_iLength-1; i >= 0; --i)
	{
		if(RI.m_pChars[i] != UCS_SPACE)
		{
			bNonBlank = true;
			continue;
		}

		// only count this space if this is not last run, or if we
		// have found something other than spaces
		if(!ri.m_bLastOnLine || bNonBlank)
			iCount++;
	}

	if(!bNonBlank)
	{
		return -iCount;
	}
	else
	{
		return iCount;
	}
}

/*!
   justify() distributes justification information into the text.

   The justification information consists of ri.m_iJustificationPoints and
   m_iJustificationAmount

   NB: This function must not modify the original values in
   ri.m_iJustificationAmount and ri.m_iJustificationPoints
*/
void GR_Graphics::justify(GR_RenderInfo & ri)
{
	UT_return_if_fail(ri.getType() == GRRI_XP);
	GR_XPRenderInfo & RI = static_cast<GR_XPRenderInfo &>(ri);

	UT_return_if_fail(RI.m_pChars && RI.m_pWidths);

	// need to leave the original values alone ...
	UT_uint32 iPoints = ri.m_iJustificationPoints;
	UT_sint32 iAmount = ri.m_iJustificationAmount;
	
	
	if(!iAmount)
	{
		// this can happend near the start of the line (the line is
		// processed from back to front) due to rounding errors in
		// the  algorithm; we simply mark the run as one that does not
		// use justification

		// not needed, since we are always called after ::resetJustification()
		// resetJustification();
		return;
	}

	if(iPoints)
	{
		for(UT_sint32 i = 0; i < RI.m_iLength; ++i)
		{
			if(RI.m_pChars[i] != UCS_SPACE)
				continue;
			
			RI.m_iSpaceWidthBeforeJustification = RI.m_pWidths[i];
		
			UT_sint32 iThisAmount = iAmount / iPoints;

			RI.m_pWidths[i] += iThisAmount;

			xxx_UT_DEBUGMSG(("Space at loc %d new width %d given extra width %d \n",
							 i,pCharWidths[i],iThisAmount));
		
			iAmount -= iThisAmount;

			iPoints--;

			if(!iPoints)
				break;
		}

		if(RI.s_pOwner == &RI)
			RI.s_pOwner = NULL;
	}
}

UT_uint32 GR_Graphics::XYToPosition(const GR_RenderInfo & ri, UT_sint32 /*x*/, 
									UT_sint32 /*y*/) const
{
	UT_return_val_if_fail(ri.getType() == GRRI_XP, 0);
	const GR_XPRenderInfo & RI = static_cast<const GR_XPRenderInfo &>(ri);
	UT_return_val_if_fail(RI.m_pWidths, 0);

	UT_return_val_if_fail(UT_NOT_IMPLEMENTED,0 );
	return 0;
}

void GR_Graphics::positionToXY(const GR_RenderInfo & ri,
							   UT_sint32& /*x*/, UT_sint32& /*y*/,
							   UT_sint32& /*x2*/, UT_sint32& /*y2*/,
							   UT_sint32& /*height*/, bool& /*bDirection*/) const
{
	UT_return_if_fail(ri.getType() == GRRI_XP);
	const GR_XPRenderInfo & RI = static_cast<const GR_XPRenderInfo &>(ri);
	UT_return_if_fail(RI.m_pWidths);

	UT_return_if_fail(UT_NOT_IMPLEMENTED);
}

UT_uint32 GR_Graphics::adjustCaretPosition(GR_RenderInfo & ri, bool /*bForward*/)
{
	return ri.m_iOffset;
}

void GR_Graphics::adjustDeletePosition(GR_RenderInfo & )
{
	return;
}

#endif // #ifndef ABI_GRAPHICS_PLUGIN

///////////////////////////////////////////////////////////////////////////////
//
//  IMPLEMNATION OF GR_GraphicsFactory
//
//  GR_GraphicsFactory allows parallel existence of differnt
//  implementations of GR_Graphics class, so that the graphics
//  class used by the application can be changed at runtime.
//
//  The factory is accessed via XAP_App functions.
//
//  Each derrived class needs to be registered with the factory using
//  the registerClass() function; new instances of the graphics class
//  are obtained by newGraphics()
//
//  
//  


/*!
   Registers the class allocator and descriptor functions with the
   factory.

   allocator is a static intermediary to the graphics constructor; it
   takes a parameter of type GR_AllocInfo*, which should point to any
   data the allocator needs to pass onto the constructor.

   descriptor is a static function that returns a string that
   describes the graphics class in human-readable manner (e.g., to be
   shown in dialogues).
   
   Returns true on success, false if requested id is already
   registered (except for IDs < GRID_LAST_DEFAULT, which will
   automatically be reassigned).

   This function is to be used by built-in classes; plugins
   should use registerPluginClass() instead.

   The default platform implementation of the graphics class should
   register itself twice, once with its predefined id and once as the
   default class.
*/
bool GR_GraphicsFactory::registerClass(GR_Allocator allocator, GR_Descriptor descriptor,
									   UT_uint32 iClassId)
{
	UT_return_val_if_fail(allocator && descriptor && iClassId > GRID_LAST_DEFAULT, false);
	
	UT_sint32 indx = m_vClassIds.findItem(iClassId);

	if(indx >= 0)
	{
		return false;
	}
	
	m_vAllocators.addItem(allocator);
	m_vDescriptors.addItem(descriptor);
	m_vClassIds.addItem((UT_sint32)iClassId);

	return true;
}


/*!
   As registerClass() but to be used by plugins; it automatically
   allocates an id for the class by which it will be identified.

   \return id > 0 on success, 0 on failure
*/
UT_uint32 GR_GraphicsFactory::registerPluginClass(GR_Allocator allocator, GR_Descriptor descriptor)
{
	UT_return_val_if_fail(allocator && descriptor, 0);

	static UT_uint32 iLastId = GRID_LAST_EXTENSION;
	iLastId++;

	while(iLastId < GRID_UNKNOWN && !registerClass(allocator,descriptor, iLastId))
		iLastId++;

	if(iLastId != GRID_UNKNOWN)
		return iLastId;

	return 0;
}


/*!
    Unregisteres class with the given id; this function is only to be
    used by plugins.

    Returns true on success.

    The caller must never try to unregister any other class that
    itself; the sole exception to this is GRID_DEFAULT

    The built-in graphics classes cannot be unregistered.
*/
bool GR_GraphicsFactory::unregisterClass(UT_uint32 iClassId)
{
	// cannot unregister built-in classes
	UT_return_val_if_fail(iClassId > GRID_LAST_BUILT_IN, false);

	// cannot unregister the default graphics class, hopefully the
	// rogue plugin will pay attention to the return value
	UT_return_val_if_fail(iClassId == m_iDefaultScreen || iClassId == m_iDefaultPrinter, false);
	
	UT_sint32 indx = m_vClassIds.findItem(iClassId);

	if(indx < 0)
		return false;

	m_vClassIds.deleteNthItem(indx);
	m_vAllocators.deleteNthItem(indx);
	m_vDescriptors.deleteNthItem(indx);

	return true;
}

/*!
   Creates an instance of the graphics class represented by iClassId,
   passing param to the class allocator.
*/
GR_Graphics * GR_GraphicsFactory::newGraphics(UT_uint32 iClassId, GR_AllocInfo &param) const
{
	if(iClassId == GRID_DEFAULT)
		iClassId = m_iDefaultScreen;

	if(iClassId == GRID_DEFAULT_PRINT)
		iClassId = m_iDefaultPrinter;
	
	UT_sint32 indx = m_vClassIds.findItem(iClassId);

	if(indx < 0)
		return NULL;

	GR_Allocator alloc = m_vAllocators.getNthItem(indx);
				
	if(!alloc)
		return NULL;

	return alloc(param);
}

const char *  GR_GraphicsFactory::getClassDescription(UT_uint32 iClassId) const
{
	if(iClassId == GRID_DEFAULT)
		iClassId = m_iDefaultScreen;

	if(iClassId == GRID_DEFAULT_PRINT)
		iClassId = m_iDefaultPrinter;

	UT_sint32 indx = m_vClassIds.findItem(iClassId);

	if(indx < 0)
		return NULL;
					
	GR_Descriptor descr = m_vDescriptors.getNthItem(indx);
				
	if(!descr)
		return NULL;

	return descr();
				
}

bool GR_GraphicsFactory::isRegistered(UT_uint32 iClassId) const
{
	UT_sint32 indx = m_vClassIds.findItem(iClassId);

	if(indx < 0)
		return false;

	return true;
}

#if defined(WITH_CAIRO)
#include "gr_CairoNullGraphics.h"
#elif defined(TOOLKIT_WIN)
#include "gr_Win32Graphics.h"
#else
#warning un-handled case
#endif

/**
 * Creates an offscreen graphics context. Only used for measuring font metrics and whatnot, not actually for drawing
 */
/* static */
GR_Graphics* GR_Graphics::newNullGraphics()
{
	// todo: support other platforms when possible
	
#if defined(WITH_CAIRO)
	GR_CairoNullGraphicsAllocInfo ai;
	return XAP_App::getApp()->newGraphics(GRID_CAIRO_NULL, (GR_AllocInfo&)ai);
#elif defined(TOOLKIT_WIN)
	GR_Win32AllocInfo ai (GR_Win32Graphics::createbestmetafilehdc(), GR_Win32Graphics::getDocInfo(), NULL);
	return XAP_App::getApp()->newGraphics(GRID_WIN32, (GR_AllocInfo&)ai);
#else
#endif
	
	return NULL;
}
