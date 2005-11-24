/* AbiWord
 * Copyright (C) 2004 Tomas Frydrych <tomasfrydrych@yahoo.co.uk>
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

#include "gr_UnixPangoGraphics.h"
#include "xap_UnixApp.h"
#include "xap_UnixFontManager.h"
#include "ut_debugmsg.h"
#include "ut_misc.h"

#include "xap_App.h"
#include "xap_Prefs.h"

// need this to include what Pango considers 'low-level' api
#define PANGO_ENABLE_ENGINE
#include <pango/pango-item.h>
#include <pango/pangoxft.h>
#include <math.h>

UT_uint32                GR_UnixPangoGraphics::s_iInstanceCount = 0;
UT_VersionInfo           GR_UnixPangoGraphics::s_Version;
int                      GR_UnixPangoGraphics::s_iMaxScript = 0;


class GR_UnixPangoItem: public GR_Item
{
	friend class GR_UnixPangoGraphics;

  public:
	virtual ~GR_UnixPangoItem(){ if (m_pi) {pango_item_free(m_pi);}};
	
	virtual GR_ScriptType getType() const {return (GR_ScriptType)m_iType;}
	
	virtual GR_Item *     makeCopy() const
	    {
			return new GR_UnixPangoItem(pango_item_copy(m_pi));
		}
	
	virtual GRRI_Type     getClassId() const {return GRRI_UNIX_PANGO;}

  protected:
	GR_UnixPangoItem(PangoItem *pi);
	GR_UnixPangoItem() : m_pi(NULL) { }; // just a dummy used to terminate
													 // GR_Itemization list

	PangoItem *m_pi;
	UT_uint32 m_iType;
};

GR_UnixPangoItem::GR_UnixPangoItem(PangoItem *pi):
	m_pi(pi)
{
	// there does not seem to be anything that we could use to identify the items, so we
	// will hash the pointers to the two text engines
	if(!pi)
	{
		m_iType = (UT_uint32)GRScriptType_Void;
	}
	else
	{
		// there does not seem to be anything that we could use to easily identify the script, so we
		// will hash the pointers to the two text engines
		
		void * b[2];
		b[0] = (void*)pi->analysis.shape_engine;
		b[1] = (void*)pi->analysis.lang_engine;

		m_iType = UT_hash32((const char *) &b, 2 * sizeof(void*));
	}
	
}

class GR_UnixPangoRenderInfo : public GR_RenderInfo
{
  public:
	GR_UnixPangoRenderInfo(GR_ScriptType t):
		GR_RenderInfo(t),
		m_pGlyphs(NULL),
		m_pJustify(NULL)
	{
		++s_iInstanceCount;
	};
	
	virtual ~GR_UnixPangoRenderInfo()
	{
		delete [] m_pJustify;

		s_iInstanceCount--;
		if(!s_iInstanceCount)
		{
			delete [] s_pLogAttr;  s_pLogAttr  = NULL;
		}
	};

	virtual GRRI_Type getType() const {return GRRI_UNIX_PANGO;}
	virtual bool      append(GR_RenderInfo &ri, bool bReverse = false);
	virtual bool      split (GR_RenderInfo *&pri, bool bReverse = false);
	virtual bool      cut(UT_uint32 offset, UT_uint32 iLen, bool bReverse = false);
	virtual bool      isJustified() const;

	bool getUTF8Text();
	
	inline bool       allocStaticBuffers(UT_uint32 iSize)
	    {
			if(s_pLogAttr) { delete [] s_pLogAttr; s_pLogAttr = NULL; }
			s_pLogAttr  = new PangoLogAttr[iSize]; // log attr. correspont to characters, not glyphs, but since there are
												   // always at least as many glyphs, this is OK
			UT_return_val_if_fail(s_pLogAttr, false);

			s_iStaticSize = iSize;
			
			s_pOwnerUTF8 = NULL;
			
			return true;
	    }

	PangoGlyphString* m_pGlyphs;
	int *             m_pJustify;

	static UT_UTF8String sUTF8;
	static GR_UnixPangoRenderInfo * s_pOwnerUTF8;
	static PangoLogAttr * s_pLogAttr;
	static UT_uint32  s_iInstanceCount;
	static UT_uint32  s_iStaticSize;  // size of the static buffers
};


GR_UnixPangoRenderInfo * GR_UnixPangoRenderInfo::s_pOwnerUTF8 = NULL;
UT_UTF8String            GR_UnixPangoRenderInfo::sUTF8;
PangoLogAttr *           GR_UnixPangoRenderInfo::s_pLogAttr = NULL;
UT_uint32                GR_UnixPangoRenderInfo::s_iInstanceCount = 0;
UT_uint32                GR_UnixPangoRenderInfo::s_iStaticSize = 0;


bool GR_UnixPangoRenderInfo::getUTF8Text()
{
	if(s_pOwnerUTF8 == this)
		return true;
	
	UT_return_val_if_fail( m_pText, false );

	UT_TextIterator & text = *m_pText;
	sUTF8.clear();
	
	for(; text.getStatus() == UTIter_OK; ++text)
	{
		sUTF8 += text.getChar();
	}

	s_pOwnerUTF8 = this;

	return true;
}

GR_UnixPangoGraphics::GR_UnixPangoGraphics(GdkWindow * win, XAP_UnixFontManager * fontManager, XAP_App *app)
	:GR_UnixGraphics(win, fontManager, app),
	 m_pFontMap(NULL),
	 m_pContext(NULL),
	 m_pPFont(NULL),
	 m_pPFontGUI(NULL)
{
	UT_DEBUGMSG(("GR_UnixPangoGraphics::GR_UnixPangoGraphics using pango (window)\n"));

	GdkDisplay * gDisp = gdk_drawable_get_display(win);
	GdkScreen *  gScreen = gdk_drawable_get_screen(win);
	Display * disp = GDK_DISPLAY_XDISPLAY(gDisp);
	int iScreen = gdk_x11_screen_get_screen_number(gScreen);
	
	m_pContext = gdk_pango_context_get_for_screen(gScreen);
	m_pFontMap = pango_xft_get_font_map(disp, iScreen);
}
		
GR_UnixPangoGraphics::GR_UnixPangoGraphics(GdkPixmap * win, XAP_UnixFontManager * fontManager, XAP_App *app, bool bUsePixmap)
	:GR_UnixGraphics(win, fontManager, app, bUsePixmap),
	 m_pFontMap(NULL),
	 m_pContext(NULL),
	 m_pPFont(NULL),
	 m_pPFontGUI(NULL)
{
	UT_DEBUGMSG(("GR_UnixPangoGraphics::GR_UnixPangoGraphics using pango (pixmap)\n"));
	
	// this is wrong, need context for the printer
	UT_ASSERT_HARMLESS( 0 );

	GdkDisplay * gDisp = gdk_drawable_get_display(win);
	GdkScreen *  gScreen = gdk_drawable_get_screen(win);
	Display * disp = GDK_DISPLAY_XDISPLAY(gDisp);
	int iScreen = gdk_x11_screen_get_screen_number(gScreen);
	
	m_pContext = gdk_pango_context_get_for_screen(gScreen);
	m_pFontMap = pango_xft_get_font_map(disp, iScreen);
}

GR_UnixPangoGraphics::~GR_UnixPangoGraphics()
{
}

GR_Graphics *   GR_UnixPangoGraphics::graphicsAllocator(GR_AllocInfo& info)
{
	UT_return_val_if_fail(info.getType() == GRID_UNIX, NULL);
	UT_DEBUGMSG(("GR_UnixPangoGraphics::graphicsAllocator\n"));
	
	GR_UnixAllocInfo &AI = (GR_UnixAllocInfo&)info;

	//!!!WDG might be right
	if(AI.m_usePixmap || AI.m_pixmap)
	{
		// printer graphics required
		return new GR_UnixPangoGraphics(AI.m_pixmap, AI.m_fontManager,
										XAP_App::getApp(),AI.m_usePixmap);
	}
	else
	{
		// screen graphics required
		return new GR_UnixPangoGraphics(AI.m_win, AI.m_fontManager, XAP_App::getApp());
	}
}

UT_sint32 GR_UnixPangoGraphics::measureUnRemappedChar(const UT_UCSChar c)
{
	// measureString() could be defined in terms of measureUnRemappedChar()
	// but its not (for presumed performance reasons).  Also, a difference
	// is that measureString() uses remapping to get past zero-width
	// character cells.

	UT_UCS4Char newChar;
	double fWidth;

	if(!isSymbol() && !isDingbat())
	{
		newChar = c;
	}
	else if(isSymbol())
	{
		newChar = static_cast<UT_UCSChar>(adobeToUnicode(c));
		xxx_UT_DEBUGMSG(("Measure width of remappedd Symbol %x \n",newChar));
	}
	else
	{
		newChar = c;
	}

	UT_UTF8String s;
	s += c;
	GList* pGL = pango_itemize(m_pContext, s.utf8_str(), 0, s.byteLength(), NULL, NULL);
	UT_return_val_if_fail( pGL, 0 );
	
	PangoItem *pItem = (PangoItem *)g_list_nth(pGL, 0)->data;
	UT_return_val_if_fail( pItem, 0 );

	PangoGlyphString * pGS = pango_glyph_string_new();
	UT_return_val_if_fail( pGS, 0 );
	
	pango_shape(s.utf8_str(), s.byteLength(), &(pItem->analysis), pGS);

	UT_return_val_if_fail( m_pPFont, 0 );
	
	PangoFont * pf = m_pPFont->getPangoFont();
	UT_return_val_if_fail( pf, 0 );
	
	PangoRectangle IR, LR;
	
	pango_glyph_string_extents(pGS, pf, &IR, &LR);

	pango_glyph_string_free(pGS);
	pango_item_free(pItem);
	g_list_free(pGL);
	
	fWidth = IR.width
		* ((double)getResolution() / (double)s_getDeviceResolution());
	return static_cast<UT_uint32>(rint(fWidth));
}

bool GR_UnixPangoGraphics::itemize(UT_TextIterator & text, GR_Itemization & I)
{
	// Performance is not of the highest priorty, as this function gets only called once
	// on each text fragment on load or keyboard entry
	UT_DEBUGMSG(("GR_UnixPangoGraphics::itemize\n"));
	UT_return_val_if_fail( m_pContext, false );
 
	// we need to convert our ucs4 data to utf8 for pango
	UT_UTF8String utf8;
	
	UT_return_val_if_fail(text.getStatus() == UTIter_OK, false);
	UT_uint32 iPosStart = text.getPosition();
	UT_uint32 iPosEnd   = text.getUpperLimit();
	UT_return_val_if_fail(iPosEnd < 0xffffffff && iPosEnd >= iPosStart, false);
	
	UT_uint32 iLen = iPosEnd - iPosStart + 1; // including iPosEnd

	UT_uint32 i;
	for(i = 0; i < iLen; ++i, ++text)
	{
		UT_return_val_if_fail(text.getStatus() == UTIter_OK, false);
		utf8 += text.getChar();
	}

	UT_uint32 iItemCount;
	// PangoAttrList *pAttr = pango_attr_list_new();
  
	GList *gItems = pango_itemize(m_pContext, utf8.utf8_str(), 0, utf8.byteLength(), NULL, NULL);
	iItemCount = g_list_length(gItems);

	//!!!WDG haven't decided what to do about attributes yet
	// We do not want to use these attributes, because the text we draw in a single call
	// is always homogenous
	// pango_attr_list_unref(pAttr);
	
	// now we process the ouptut
	UT_DEBUGMSG(("itemize: number of items %d\n", iItemCount));
	for(i = 0; i < iItemCount; ++i)
	{
		UT_DEBUGMSG(("itemize: creating item %d\n", i));
		PangoItem *pItem = (PangoItem *)g_list_nth(gItems, i)->data;
		GR_UnixPangoItem * pI = new GR_UnixPangoItem(pItem);
		UT_return_val_if_fail(pI, false);

		I.addItem(pItem->offset, pI);
	}

	I.addItem(iPosEnd - iPosStart + 1, new GR_UnixPangoItem());

	g_list_free(gItems);
	
	UT_DEBUGMSG(("itemize succeeded\n"));
	return true;
}

bool GR_UnixPangoGraphics::shape(GR_ShapingInfo & si, GR_RenderInfo *& ri)
{
	UT_DEBUGMSG(("GR_UnixPangoGraphics::shape\n"));
	UT_return_val_if_fail(si.m_pItem && si.m_pItem->getClassId() == GRRI_UNIX_PANGO && si.m_pFont, false);
	GR_UnixPangoItem * pItem = (GR_UnixPangoItem *)si.m_pItem;

	if(!ri)
	{
		// this simply allocates new instance of the RI which this function will fill with
		// meaningful data
		ri = new GR_UnixPangoRenderInfo(pItem->getType());
		UT_return_val_if_fail(ri, false);
	}
	else
	{
		UT_return_val_if_fail(ri->getType() == GRRI_UNIX_PANGO, false);
	}

	GR_UnixPangoRenderInfo * RI = (GR_UnixPangoRenderInfo *)ri;

	UT_UTF8String utf8;
	
	UT_sint32 i;
	for(i = 0; i < si.m_iLength; ++i, ++si.m_Text)
	{
		UT_return_val_if_fail(si.m_Text.getStatus() == UTIter_OK, false);
		utf8 += si.m_Text.getChar();
	}

	if(RI->m_pGlyphs)
	{
		pango_glyph_string_free(RI->m_pGlyphs);
	}
	
	RI->m_pGlyphs = pango_glyph_string_new();

	// !!!WDG where to get gtext and glength from ??? - the GR_ShapingInfo?
	// The text is in pInChars, conversion to utf-8 and removal for m_text from the item
	// is probably desirable

	// before we can call this, we have to set analysis.font
	// Is this the case, or is the font set by pango_itemize()? #TF
	GR_UnixPangoFont * pFont = (GR_UnixPangoFont *) si.m_pFont;
	pItem->m_pi->analysis.font = pFont->getPangoFont();
	
	pango_shape(utf8.utf8_str(), utf8.byteLength(), &(pItem->m_pi->analysis), RI->m_pGlyphs);

	// need to transfer data that we will need later from si to RI
	RI->m_iLength = si.m_iLength;

	RI->m_pItem = si.m_pItem;
	RI->m_pFont = si.m_pFont;

	RI->m_eShapingResult = GRSR_ContextSensitiveAndLigatures;
	
	return true;
}


UT_sint32 GR_UnixPangoGraphics::getTextWidth(GR_RenderInfo & ri)
{
	UT_DEBUGMSG(("GR_UnixPangoGraphics::getTextWidth\n"));
	UT_return_val_if_fail(ri.getType() == GRRI_UNIX_PANGO, 0);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;

	UT_return_val_if_fail( RI.m_pGlyphs, 0 );
	
	GR_UnixPangoFont * pFont = (GR_UnixPangoFont *) RI.m_pFont;
	UT_return_val_if_fail( pFont, 0 );
	
	PangoFont * pf = pFont->getPangoFont();
	UT_return_val_if_fail( pf, 0 );
	
	UT_sint32 iWidth = 0;
	PangoRectangle IR, LR;

	// need to convert the char offset and length to glyph offsets
	UT_uint32 iGlyphCount = RI.m_pGlyphs->num_glyphs;
	UT_sint32 iOffsetStart = -1, iOffsetEnd = -1;
	
	for(UT_uint32 i = 0; i < iGlyphCount; ++i)
	{
		if(iOffsetStart < 0 && RI.m_pGlyphs->log_clusters[i] == RI.m_iOffset)
		{
			iOffsetStart = i;
			continue;
		}

		if(iOffsetEnd < 0 && RI.m_pGlyphs->log_clusters[i] == RI.m_iOffset + RI.m_iLength)
		{
			iOffsetEnd = i;
			break;
		}
	}

	UT_return_val_if_fail( iOffsetStart >= 0, 0 );

	if(iOffsetEnd < 0)
	{
		// to the end
		iOffsetEnd = iGlyphCount;
	}

	pango_glyph_string_extents_range(RI.m_pGlyphs, iOffsetStart, iOffsetEnd, pf, &IR, &LR);

	iWidth = tlu(IR.width) / PANGO_SCALE;

	return iWidth;
}

void GR_UnixPangoGraphics::prepareToRenderChars(GR_RenderInfo & ri)
{
}

/*!
    The offset passed to us as part of ri is a visual offset
*/
void GR_UnixPangoGraphics::renderChars(GR_RenderInfo & ri)
{
	UT_DEBUGMSG(("GR_UnixPangoGraphics::renderChars\n"));
	UT_return_if_fail(ri.getType() == GRRI_UNIX_PANGO);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;
	GR_UnixPangoFont * pFont = (GR_UnixPangoFont *)RI.m_pFont;
	GR_UnixPangoItem * pItem = (GR_UnixPangoItem *)RI.m_pItem;
	UT_return_if_fail(pItem && pFont && pFont->getPangoFont());

	if(RI.m_iLength == 0)
		return;

	UT_DEBUGMSG(("renderChars: xoff %d yoff %d\n", RI.m_xoff, RI.m_yoff));
	UT_sint32 xoff = _tduX(RI.m_xoff);
	UT_sint32 yoff = _tduY(RI.m_yoff);

	

	UT_DEBUGMSG(("about to pango_xft_render xoff %d yoff %d\n", xoff, yoff));
	UT_return_if_fail(m_pXftDraw && RI.m_pGlyphs);
	pango_xft_render(m_pXftDraw, &m_XftColor, pFont->getPangoFont(), RI.m_pGlyphs, xoff, yoff);
}

void GR_UnixPangoGraphics::measureRenderedCharWidths(GR_RenderInfo & ri)
{
	// !!!WDG pango doesn't do scriptplace - I think it is part of pango_shape
	// not sure what is to be done here?
	// OK, in that case we do nothing, the measure call is always subsequent to the shape
	// call #TF
}

void GR_UnixPangoGraphics::appendRenderedCharsToBuff(GR_RenderInfo & ri, UT_GrowBuf & buf) const
{
	UT_return_if_fail( UT_NOT_IMPLEMENTED );
}

/*!
    returns true on success
 */
bool GR_UnixPangoGraphics::_scriptBreak(GR_UnixPangoRenderInfo &ri)
{
	UT_return_val_if_fail(ri.m_pText && ri.m_pGlyphs && ri.m_pItem, false);

	GR_UnixPangoItem * pItem = (GR_UnixPangoItem*)ri.m_pItem;

	// fill the static buffer with UTF8 text
	UT_return_val_if_fail(ri.getUTF8Text(), false);

	UT_return_val_if_fail(ri.allocStaticBuffers(ri.m_pGlyphs->num_glyphs), false);
		
	pango_break(ri.sUTF8.utf8_str(), ri.sUTF8.byteLength(),&(pItem->m_pi->analysis), ri.s_pLogAttr, ri.m_pGlyphs->num_glyphs);

	return true;
}

bool GR_UnixPangoGraphics::canBreak(GR_RenderInfo & ri, UT_sint32 &iNext, bool bAfter)
{
	UT_return_val_if_fail(ri.getType() == GRRI_UNIX_PANGO && ri.m_iOffset < ri.m_iLength, false);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;
	iNext = -1;

	if(!_scriptBreak(RI))
		return false;

	if(ri.m_iLength > (UT_sint32)RI.s_iStaticSize)
	{
		UT_return_val_if_fail( RI.allocStaticBuffers(ri.m_iLength),false );
	}
	
	UT_uint32 iDelta  = 0;
	if(bAfter)
	{
		// the caller wants to know if break can occur on the (logically) right edge of the given
		// character
		if(ri.m_iOffset + 1 == ri.m_iLength)
		{
			// we are quering the last char of a run, for which we do not have the info
			// we will return false, which should force the next run to be examined ...
			return false;
		}

		// we will examine the next character, since USP tells us about breaking on the left edge
		iDelta = 1;
	}

	if(RI.s_pLogAttr[ri.m_iOffset + iDelta].is_char_break)
		return true;

	// find the next break
	for(UT_sint32 i = ri.m_iOffset + iDelta + 1; i < RI.m_iLength; ++i)
	{
		if(RI.s_pLogAttr[i].is_char_break)
		{
			iNext = i - iDelta;
			break;
		}
	}
	
	return false;
}



UT_sint32 GR_UnixPangoGraphics::resetJustification(GR_RenderInfo & ri, bool bPermanent)
{
	UT_return_val_if_fail(ri.getType() == GRRI_UNIX_PANGO, 0);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;

	if(!RI.m_pJustify)
		return 0;
	
	UT_sint32 iWidth2 = 0;
	for(UT_sint32 i = 0; i < RI.m_pGlyphs->num_glyphs; ++i)
	{
		iWidth2 += RI.m_pJustify[i];

		// TODO here we need to substract the amount from pango metrics
		UT_ASSERT_HARMLESS( UT_NOT_IMPLEMENTED );
	}

	if(bPermanent)
	{
		delete [] RI.m_pJustify;
		RI.m_pJustify = NULL;
	}
	else
	{
		memset(RI.m_pJustify, 0, RI.m_pGlyphs->num_glyphs * sizeof(int));
	}
	
	return -iWidth2;
}


UT_sint32 GR_UnixPangoGraphics::countJustificationPoints(const GR_RenderInfo & ri) const
{
	UT_return_val_if_fail(ri.getType() == GRRI_UNIX_PANGO, 0);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;

	UT_return_val_if_fail(RI.m_pText, 0);
	UT_TextIterator & text = *RI.m_pText;
	UT_uint32 iPosEnd   = text.getUpperLimit();

	text.setPosition(iPosEnd);
	UT_return_val_if_fail( text.getStatus()== UTIter_OK, 0 );
	

	UT_sint32 iCount = 0;
	bool bNonBlank = false;

	for(; text.getStatus() == UTIter_OK; --text)
	{
		UT_UCS4Char c = text.getChar();
		
		if(c != UCS_SPACE)
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
    We take the same approach as with Uniscribe; we store the justification amount in a
    separate array of the ri and add it to the offsets before we draw. We will probably
    need some static buffers to speed things up
 */
void GR_UnixPangoGraphics::justify(GR_RenderInfo & ri)
{
	UT_return_if_fail(ri.getType() == GRRI_UNIX_PANGO);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &) ri;
	if(!RI.m_iJustificationPoints || !RI.m_iJustificationAmount || !RI.m_pGlyphs)
		return;
	
	if(!RI.m_pJustify)
		RI.m_pJustify = new int[RI.m_pGlyphs->num_glyphs];
	
	UT_return_if_fail(RI.m_pJustify);
	memset(RI.m_pJustify, 0, RI.m_pGlyphs->num_glyphs * sizeof(int));
	
	UT_uint32 iExtraSpace = RI.m_iJustificationAmount;
	UT_uint32 iPoints     = RI.m_iJustificationPoints;

	UT_return_if_fail( RI.m_pText );
	UT_TextIterator & text = *RI.m_pText;
	
	for(UT_sint32 i = 0; text.getStatus() == UTIter_OK && i < RI.m_iLength; ++text, ++i)
	{
		UT_UCS4Char c = text.getChar();
		
		if(c == UCS_SPACE)
		{
			UT_uint32 iSpace = iExtraSpace/iPoints;
			iExtraSpace -= iSpace;
			iPoints--;

			RI.m_pJustify[i] = iSpace;

			// TODO here we need to add this amount the pango metrics
			UT_ASSERT_HARMLESS( UT_NOT_IMPLEMENTED );
			
			if(!iPoints)
				break;
		}
	}

	UT_ASSERT_HARMLESS( !iExtraSpace );
}

UT_uint32 GR_UnixPangoGraphics::XYToPosition(const GR_RenderInfo & ri, UT_sint32 x, UT_sint32 y) const
{
	UT_return_val_if_fail(ri.getType() == GRRI_UNIX_PANGO, 0);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &) ri;
	GR_UnixPangoItem * pItem = (GR_UnixPangoItem *)RI.m_pItem;
	UT_return_val_if_fail(pItem, 0);

	UT_return_val_if_fail( UT_NOT_IMPLEMENTED, 0 );
}

void GR_UnixPangoGraphics::positionToXY(const GR_RenderInfo & ri,
										  UT_sint32& x, UT_sint32& y,
										  UT_sint32& x2, UT_sint32& y2,
										  UT_sint32& height, bool& bDirection) const
{
	UT_return_if_fail(ri.getType() == GRRI_UNIX_PANGO);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &) ri;
	GR_UnixPangoItem * pItem = (GR_UnixPangoItem *)RI.m_pItem;
  
	if(!pItem)
		return;

	// the code in fp_TextRun that calls this function does not construct the iterator !!!
	UT_ASSERT_HARMLESS( 0 );
	
	// TODO: this is very inefficient: to cache or not to cache ?
	UT_UTF8String utf8;
	
	UT_sint32 i;
	for(i = 0; i < RI.m_iLength; ++i, ++(*(RI.m_pText)))
	{
		UT_return_if_fail(RI.m_pText->getStatus() == UTIter_OK);
		utf8 += RI.m_pText->getChar();
	}
	
	pango_glyph_string_index_to_x (RI.m_pGlyphs,
								   (char*)utf8.utf8_str(), // do not like this ...
								   utf8.byteLength(),
								   &(pItem->m_pi->analysis), 
								   RI.m_iOffset,
								   RI.m_bLastOnLine,
								   &x);

	x = x;
	x2 = x;
}

void GR_UnixPangoGraphics::drawChars(const UT_UCSChar* pChars,
									int iCharOffset, int iLength,
									UT_sint32 xoff, UT_sint32 yoff,
									int * pCharWidth)
{
	UT_return_if_fail(m_pXftDraw);

	UT_UTF8String utf8(pChars + iCharOffset, iLength);

	// this function expect indexes in bytes !!! (stupid)
	GList * pItems = pango_itemize(getContext(), utf8.utf8_str(), 0, utf8.byteLength(), NULL, NULL);
	int iItemCount = g_list_length(pItems);
	PangoGlyphString * pGstring = pango_glyph_string_new();

	UT_sint32 xoffD = _tduX(xoff);
	UT_sint32 yoffD = _tduY(yoff);

	PangoFont * pf = m_pPFont->getPangoFont();
	PangoRectangle IR, LR;
	
	for(int i = 0; i < iItemCount; ++i)
	{
		PangoItem *pItem = (PangoItem *)g_list_nth(pItems, i)->data;
		UT_return_if_fail( pItem );
		pItem->analysis.font = pf;

		pango_shape(utf8.utf8_str()+ pItem->offset, pItem->length, &(pItem->analysis), pGstring);

		pango_xft_render(m_pXftDraw, &m_XftColor, pf, pGstring, xoffD, yoffD);

		// now advance xoff
		pango_glyph_string_extents(pGstring, pf, &IR, &LR);
		xoffD += PANGO_PIXELS(IR.width);
	}

	pango_glyph_string_free(pGstring);
}

void GR_UnixPangoGraphics::setFont(GR_Font * pFont)
{
	UT_return_if_fail( pFont && pFont->getType() == GR_FONT_UNIX_PANGO);

	//PangoFont * pf = (PangoFont*) pFont;
	m_pPFont = static_cast<GR_UnixPangoFont*>(pFont);
}

void GR_UnixPangoGraphics::setZoomPercentage(UT_uint32 iZoom)
{
	// not sure if we should not call GR_UnixGraphics::setZoomPercentage() here instead
	GR_Graphics::setZoomPercentage (iZoom); // chain up

	DELETEP(m_pPFontGUI);
	GR_Font * pFont = getGUIFont();
	UT_return_if_fail( pFont->getType()== GR_FONT_UNIX_PANGO );
	
	m_pPFontGUI = NULL;
	m_pPFontGUI = static_cast<GR_UnixPangoFont*>(pFont);
}

GR_Font* GR_UnixPangoGraphics::getDefaultFont(UT_String& fontFamily)
{
	UT_return_val_if_fail( UT_NOT_IMPLEMENTED, NULL );
}

UT_uint32 GR_UnixPangoGraphics::getFontAscent()
{
	return getFontAscent(m_pPFont);
}

UT_uint32 GR_UnixPangoGraphics::getFontDescent()
{
	return getFontDescent(m_pPFont);
}

UT_uint32 GR_UnixPangoGraphics::getFontHeight()
{
	return getFontHeight(m_pPFont);
}

UT_uint32 GR_UnixPangoGraphics::getFontAscent(GR_Font * pFont)
{
	UT_return_val_if_fail( pFont, 0 );

	GR_UnixPangoFont * pFP = (GR_UnixPangoFont*) pFont;
	PangoFont * pf = pFP->getPangoFont();

	// FIXME: we probably want the real language from somewhere
	PangoFontMetrics * pfm = pango_font_get_metrics(pf, pango_language_from_string("en-US"));
	UT_return_val_if_fail( pfm, 0 );
	
	UT_uint32 i = (UT_uint32) pango_font_metrics_get_descent(pfm);
	pango_font_metrics_unref(pfm);
	return i;
}

UT_uint32 GR_UnixPangoGraphics::getFontDescent(GR_Font *pFont)
{
	UT_return_val_if_fail( pFont, 0 );

	GR_UnixPangoFont * pFP = (GR_UnixPangoFont*) pFont;
	PangoFont * pf = pFP->getPangoFont();

	// FIXME: we probably want the real language from somewhere
	PangoFontMetrics * pfm = pango_font_get_metrics(pf, pango_language_from_string("en-US"));
	UT_return_val_if_fail( pfm, 0 );

	UT_uint32 i = (UT_uint32) pango_font_metrics_get_descent(pfm);
	pango_font_metrics_unref(pfm);
	return i;
}

UT_uint32 GR_UnixPangoGraphics::getFontHeight(GR_Font *pFont)
{
	UT_return_val_if_fail( pFont, 0 );

	GR_UnixPangoFont * pFP = (GR_UnixPangoFont*) pFont;
	PangoFont * pf = pFP->getPangoFont();

	// FIXME: we probably want the real language from somewhere
	PangoFontMetrics * pfm = pango_font_get_metrics(pf, pango_language_from_string("en-US"));
	UT_return_val_if_fail( pfm, 0 );
	
	UT_uint32 i = (UT_uint32) pango_font_metrics_get_descent(pfm);
	pango_font_metrics_unref(pfm);
	return i;
}
	
const char* GR_UnixPangoGraphics::findNearestFont(const char* pszFontFamily,
												  const char* pszFontStyle,
												  const char* pszFontVariant,
												  const char* pszFontWeight,
												  const char* pszFontStretch,
												  const char* pszFontSize)
{
	UT_String s = "'";
	s += pszFontFamily;
	s += "' ";
	s += pszFontStyle;
	s += " ";
	s += pszFontVariant;
	s += " ";
	s += pszFontWeight;
	s += " ";
	s += pszFontStretch;
	s += " ";
	s += pszFontSize;

	const char * cs = s.c_str() + s.length() - 2;

	if(!UT_strcmp(cs, "pt"))
	   s[s.length()-2] = 0;

	PangoFontDescription * pfd = pango_font_description_from_string(s.c_str());
	UT_return_val_if_fail( pfd, NULL );

	PangoFont *pf = pango_context_load_font(getContext(), pfd);

	pango_font_description_free(pfd);
	pfd = pango_font_describe(pf);
	
	const char* face = pango_font_description_get_family(pfd);

	static char buffer[100];
	strncpy(buffer, face, 99);
	buffer[99] = 0;
	
	pango_font_description_free(pfd);
	
	return buffer;
}


GR_Font* GR_UnixPangoGraphics::_findFont(const char* pszFontFamily,
										 const char* pszFontStyle,
										 const char* pszFontVariant,
										 const char* pszFontWeight,
										 const char* pszFontStretch,
										 const char* pszFontSize)
{
	UT_String s = "'";
	s += pszFontFamily;
	s += "' ";
	s += pszFontStyle;
	s += " ";
	s += pszFontVariant;
	s += " ";
	s += pszFontWeight;
	s += " ";
	s += pszFontStretch;
	s += " ";
	s += pszFontSize;

	const char * cs = s.c_str() + s.length() - 2;

	if(!UT_strcmp(cs, "pt"))
	   s[s.length()-2] = 0;

	PangoFontDescription * pfd = pango_font_description_from_string(s.c_str());
	UT_return_val_if_fail( pfd, NULL );
	
	return new GR_UnixPangoFont(pfd, this);
}

	
void GR_UnixPangoGraphics::getCoverage(UT_NumberVector& coverage)
{
	// I am not sure if this is really used for anything, my grep indicates no
	UT_ASSERT_HARMLESS( UT_NOT_IMPLEMENTED );
}

GR_Font * GR_UnixPangoGraphics::getGUIFont(void)
{
	if (!m_pPFontGUI)
	{
		// get the font resource
		GtkStyle *tempStyle = gtk_style_new();
		const char *guiFontName = pango_font_description_get_family(tempStyle->font_desc);
		if (!guiFontName)
			guiFontName = "Times New Roman";

		UT_String s;
		UT_String_sprintf(s, "'%s' %f", guiFontName,12*100.0/getZoomPercentage()); 
		PangoFontDescription * pfd = pango_font_description_from_string(s.c_str());
		UT_return_val_if_fail( pfd, NULL );
		g_object_unref(G_OBJECT(tempStyle));
	
		m_pPFontGUI = new GR_UnixPangoFont(pfd, this);
		
		UT_ASSERT(m_pPFontGUI);
	}

	return m_pPFontGUI;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GR_UnixPangFont implementation
//
GR_UnixPangoFont::GR_UnixPangoFont(PangoFontDescription * pDesc, GR_UnixPangoGraphics * pG)
	:m_pf(NULL)
{
	m_eType = GR_FONT_UNIX_PANGO;
	
	UT_return_if_fail( pDesc && pG );
	
	m_pf = pango_context_load_font(pG->getContext(), pDesc);
}

/*!
	Measure the unremapped char to be put into the cache.
	That means measuring it for a font size of 120
*/
UT_sint32 GR_UnixPangoFont::measureUnremappedCharForCache(UT_UCS4Char cChar) const
{
	// this is not implemented because we do not use the width cache (when shaping, it is
	// not possible to measure characters, only glyphs)
	UT_ASSERT_HARMLESS( UT_NOT_IMPLEMENTED );
	return 0;
}

/*
  NB: it is essential that this function is fast
*/
bool GR_UnixPangoFont::doesGlyphExist(UT_UCS4Char g)
{
	UT_return_val_if_fail( m_pf, false );

	// FIXME: need to pass the real language down to this function eventually, but since
	// we only expect answer in yes/no terms, this will do quite well for now
	PangoCoverage* pCoverage = pango_font_get_coverage(m_pf, pango_language_from_string("en-US"));
	UT_return_val_if_fail( pCoverage, false );

	PangoCoverageLevel eLevel = pango_coverage_get(pCoverage, g);

	if(PANGO_COVERAGE_NONE == eLevel)
		return false;

	return true;
}

bool GR_UnixPangoFont::glyphBox(UT_UCS4Char g, UT_Rect & rec, GR_Graphics * pG)
{
	UT_ASSERT_HARMLESS( UT_NOT_IMPLEMENTED );
	return false;
}


//////////////////////////////////////////////////////////////////////////////
//
// GR_UnixPangoRenderInfo Implementation
//

bool GR_UnixPangoRenderInfo::append(GR_RenderInfo &ri, bool bReverse)
{
	//UT_return_val_if_fail( UT_NOT_IMPLEMENTED, false );
	return false;
}

bool GR_UnixPangoRenderInfo::split (GR_RenderInfo *&pri, bool bReverse)
{
	UT_return_val_if_fail(m_pGraphics && m_pFont, false);

	UT_ASSERT_HARMLESS(!pri);

	// create a new RI and make a copy of item into
	if(!pri)
	{
		pri = new GR_UnixPangoRenderInfo(m_eScriptType);
		UT_return_val_if_fail(pri,false);
	}

	pri->m_pItem = m_pItem->makeCopy();
	UT_return_val_if_fail(pri->m_pItem, false);

	return false;
}

bool GR_UnixPangoRenderInfo::cut(UT_uint32 offset, UT_uint32 iLen, bool bReverse)
{
	return false;
}


bool GR_UnixPangoRenderInfo::isJustified() const
{
	UT_return_val_if_fail( UT_NOT_IMPLEMENTED,false );
}


GR_UnixPangoFont::~GR_UnixPangoFont()
{
}


