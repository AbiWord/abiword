/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2004-6 Tomas Frydrych <dr.tomas@yahoo.co.uk>
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

#include "xap_App.h"
#include "xap_Prefs.h"
#include "xap_EncodingManager.h"

#include "xap_UnixApp.h"
#include "xap_UnixFontManager.h"

#include "ut_debugmsg.h"
#include "ut_misc.h"
#include "ut_vector.h"
#include "ut_locale.h"

// need this to include what Pango considers 'low-level' api
#define PANGO_ENABLE_ENGINE

#include <pango/pango-item.h>
#include <pango/pango-engine.h>
#include <pango/pangoxft.h>

#include <math.h>

#include <gdk/gdk.h>

#ifndef WITHOUT_PRINTING
#include <libgnomeprint/gnome-print-pango.h>
#endif

// found in xap_UnixFont.cpp
extern float fontPoints2float(UT_uint32 iSize,
							  FT_Face pFace,
							  UT_uint32 iFontPoints);

UT_uint32      GR_UnixPangoGraphics::s_iInstanceCount = 0;
UT_VersionInfo GR_UnixPangoGraphics::s_Version;
int            GR_UnixPangoGraphics::s_iMaxScript = 0;


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
	// there does not seem to be anything that we could use to identify the
	// items, so we will hash the pointers to the two text engines
	if(!pi)
	{
		m_iType = (UT_uint32)GRScriptType_Void;
	}
	else
	{
		// there does not seem to be anything that we could use to easily
		// identify the script, so we will hash the pointers to the two text
		// engines
		
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
		m_pLogOffsets(NULL),
		m_pJustify(NULL),
		m_iZoom(0),
		m_iCharCount(0)
	{
		++s_iInstanceCount;
	};
	
	virtual ~GR_UnixPangoRenderInfo()
	{
		delete [] m_pJustify; delete [] m_pLogOffsets;
		pango_glyph_string_free(m_pGlyphs);
		s_iInstanceCount--;

		if(!s_iInstanceCount)
		{
			delete [] s_pLogAttrs;
			s_pLogAttrs = NULL;
		}
	};

	virtual GRRI_Type getType() const {return GRRI_UNIX_PANGO;}
	virtual bool append(GR_RenderInfo &ri, bool bReverse = false);
	virtual bool split (GR_RenderInfo *&pri, bool bReverse = false);
	virtual bool cut(UT_uint32 offset, UT_uint32 iLen, bool bReverse = false);
	virtual bool isJustified() const;

	bool getUTF8Text();
	
	inline bool       allocStaticBuffers(UT_uint32 iSize)
	    {
			if(s_pLogAttrs)
				delete [] s_pLogAttrs;

			s_pLogAttrs = new PangoLogAttr[iSize];

			if(!s_pLogAttrs)
				return false;
			
			s_iStaticSize = iSize;
			return true;
	    }

	PangoGlyphString* m_pGlyphs;
	int *             m_pLogOffsets;
	int *             m_pJustify;
	UT_uint32         m_iZoom;
	UT_uint32         m_iCharCount;
	
	static UT_UTF8String sUTF8;
	static GR_UnixPangoRenderInfo * s_pOwnerUTF8;
	static UT_uint32  s_iInstanceCount;
	static UT_uint32  s_iStaticSize;  // size of the static buffers

	static PangoLogAttr *           s_pLogAttrs;
	static GR_UnixPangoRenderInfo * s_pOwnerLogAttrs;
};


GR_UnixPangoRenderInfo * GR_UnixPangoRenderInfo::s_pOwnerUTF8 = NULL;
UT_UTF8String            GR_UnixPangoRenderInfo::sUTF8;
UT_uint32                GR_UnixPangoRenderInfo::s_iInstanceCount = 0;
UT_uint32                GR_UnixPangoRenderInfo::s_iStaticSize = 0;
GR_UnixPangoRenderInfo * GR_UnixPangoRenderInfo::s_pOwnerLogAttrs = NULL;
PangoLogAttr *           GR_UnixPangoRenderInfo::s_pLogAttrs = NULL;


bool GR_UnixPangoRenderInfo::getUTF8Text()
{
	if(s_pOwnerUTF8 == this)
		return true;
	
	UT_return_val_if_fail( m_pText, false );

	UT_TextIterator & text = *m_pText;
	sUTF8.clear();

	// we intentionally run this as far as the iterator lets us, even if that is
	// past the end of this item
	for(; text.getStatus() == UTIter_OK; ++text)
	{
		sUTF8 += text.getChar();
	}

	s_pOwnerUTF8 = this;

	return true;
}

/* taken from gnomeprint */
static void
xft_substitute_func (FcPattern *pattern, gpointer   data)
{
	FcPatternDel (pattern, FC_HINTING);
	FcPatternAddBool (pattern, FC_HINTING, FALSE);
}

GR_UnixPangoGraphics::GR_UnixPangoGraphics(GdkWindow * win)
	:GR_UnixGraphics(win, NULL),
	 m_pFontMap(NULL),
	 m_pContext(NULL),
	 m_pPFont(NULL),
	 m_pPFontGUI(NULL),
	 m_iDeviceResolution(96)
{
	GdkDisplay * gDisp = gdk_drawable_get_display(win);
	GdkScreen *  gScreen = gdk_drawable_get_screen(win);
	Display * disp = GDK_DISPLAY_XDISPLAY(gDisp);
	int iScreen = gdk_x11_screen_get_screen_number(gScreen);

	m_pContext = pango_xft_get_context(disp, iScreen);
	m_pFontMap = pango_xft_get_font_map(disp, iScreen);

	/* ascertain the real dpi that xft will be using */
	FcPattern *pattern;
	double dpi = 0.0; 
	pattern = FcPatternCreate();
	if (pattern)
	{
		XftDefaultSubstitute (GDK_SCREEN_XDISPLAY (gScreen),
							  iScreen,
							  pattern);
		FcPatternGetDouble (pattern, FC_DPI, 0, &dpi); 
		FcPatternDestroy (pattern);
		UT_DEBUGMSG(("@@@@@@@@@@@@@@ retrieved DPI %f @@@@@@@@@@@@@@@@@@ \n", dpi));

		m_iDeviceResolution = (UT_uint32)round (dpi);
	}	

	_setIsSymbol(false);
	_setIsDingbat(false);
}


GR_UnixPangoGraphics::GR_UnixPangoGraphics()
	:GR_UnixGraphics(NULL, NULL),
	 m_pFontMap(NULL),
	 m_pContext(NULL),
	 m_pPFont(NULL),
	 m_pPFontGUI(NULL),
	 m_iDeviceResolution(96)
{
#ifdef WITHOUT_PRINTING
	UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
#endif
	
	_setIsSymbol(false);
	_setIsDingbat(false);
}


GR_UnixPangoGraphics::~GR_UnixPangoGraphics()
{
	g_object_unref(m_pContext);
	// NB: m_pFontMap is owned by Pango

	_destroyFonts();
	delete m_pPFontGUI;
}

GR_Graphics *   GR_UnixPangoGraphics::graphicsAllocator(GR_AllocInfo& info)
{
	UT_return_val_if_fail(info.getType() == GRID_UNIX, NULL);
	xxx_UT_DEBUGMSG(("GR_UnixPangoGraphics::graphicsAllocator\n"));
	
	GR_UnixAllocInfo &AI = (GR_UnixAllocInfo&)info;

	//!!!WDG might be right
	if(AI.m_usePixmap || AI.m_pixmap)
	{
		// printer graphics required This class does not provide printing
		// services -- we need a separate derrived class for that
		UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
		return NULL;
	}
	else
	{
		// screen graphics required
		return new GR_UnixPangoGraphics(AI.m_win);
	}
}

UT_uint32 GR_UnixPangoGraphics::getDeviceResolution(void) const
{
	// TODO -- we should get this somewhere from the xft lib
	return m_iDeviceResolution;
}


UT_sint32 GR_UnixPangoGraphics::measureUnRemappedChar(const UT_UCSChar c)
{
	/* This function should never be called whe the Pango graphics is in use
	 * -- if you get this assert, please file a bug.
	 * Tomas
	 */
	UT_ASSERT_HARMLESS( UT_NOT_REACHED );
	return 0;
}

bool GR_UnixPangoGraphics::itemize(UT_TextIterator & text, GR_Itemization & I)
{
	// Performance is not of the highest priorty, as this function gets only
	// called once on each text fragment on load or keyboard entry
	xxx_UT_DEBUGMSG(("GR_UnixPangoGraphics::itemize\n"));
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

	// this will result in itemization assuming base direction of 0
	// we set the appropriate embedding level later in shape()
	GList *gItems = pango_itemize(m_pContext,
								  utf8.utf8_str(),
								  0, utf8.byteLength(),
								  NULL, NULL);
	
	iItemCount = g_list_length(gItems);

	//!!!WDG haven't decided what to do about attributes yet We do not want to
	//use these attributes, because the text we draw in a single call is always
	//homogenous pango_attr_list_unref(pAttr);
	
	// now we process the ouptut
	UT_uint32 iOffset = 0;
	xxx_UT_DEBUGMSG(("itemize: number of items %d\n", iItemCount));
	for(i = 0; i < iItemCount; ++i)
	{
		xxx_UT_DEBUGMSG(("itemize: creating item %d\n", i));
		PangoItem *pItem = (PangoItem *)g_list_nth(gItems, i)->data;
		GR_UnixPangoItem * pI = new GR_UnixPangoItem(pItem);
		if(!pI)
		{
			UT_ASSERT(pI);
			g_list_free(gItems);
			return false;
		}

		I.addItem(iOffset, pI);
		iOffset += pItem->num_chars;
	}

	I.addItem(iPosEnd - iPosStart + 1, new GR_UnixPangoItem());

	g_list_free(gItems);
	
	xxx_UT_DEBUGMSG(("itemize succeeded\n"));
	return true;
}

int *
GR_UnixPangoGraphics::_calculateLogicalOffsets (PangoGlyphString * pGlyphs,
												UT_BidiCharType iVisDir,
												const char * pUtf8)
{
	UT_return_val_if_fail( pGlyphs && pUtf8, NULL );
	
	// pGlyphs contains logical cluster info, which is
	// unfortunately indexed to bytes in the utf-8 string, not characters --
	// this is real pain and we have to convert it.
	
	int * pLogOffsets = new int [pGlyphs->num_glyphs];
 
    // See http://www.abisource.com/mailinglists/abiword-dev/2006/Feb/0081.html
    // for insight how this is supposeed to work and possible optimizations.

	// In LTR text, the values in log_clusters are guaranteed to be increasing,
	// in RTL text, the values in log_clusters are decreasing
	
	if (iVisDir == UT_BIDI_LTR ||
		(pGlyphs->num_glyphs > 1 &&
		 pGlyphs->log_clusters[0] < pGlyphs->log_clusters[1]))
	{
		for(int i = 0; i < pGlyphs->num_glyphs; ++i)
		{
			int iOff = pGlyphs->log_clusters[i];
			pLogOffsets[i] =  g_utf8_pointer_to_offset (pUtf8, pUtf8 + iOff);
		}
	}
	else // GR_ShapingInfo.m_iVisDir == UT_BIDI_RTL)
	{
		for(int i = pGlyphs->num_glyphs - 1; i >= 0; --i)
		{
			int iOff = pGlyphs->log_clusters[i];
			pLogOffsets[i] =  g_utf8_pointer_to_offset (pUtf8, pUtf8 + iOff);
		}
	}

	return pLogOffsets;
}

bool GR_UnixPangoGraphics::shape(GR_ShapingInfo & si, GR_RenderInfo *& ri)
{
	xxx_UT_DEBUGMSG(("GR_UnixPangoGraphics::shape, len %d\n", si.m_iLength));
	UT_return_val_if_fail(si.m_pItem &&
						  si.m_pItem->getClassId() == GRRI_UNIX_PANGO &&
						  si.m_pFont, false);
	
	GR_UnixPangoItem * pItem = (GR_UnixPangoItem *)si.m_pItem;

	if(!ri)
	{
		// this simply allocates new instance of the RI which this function will
		// fill with meaningful data
		ri = new GR_UnixPangoRenderInfo(pItem->getType());
		UT_return_val_if_fail(ri, false);
	}
	else
	{
		UT_return_val_if_fail(ri->getType() == GRRI_UNIX_PANGO, false);
	}

	GR_UnixPangoRenderInfo * RI = (GR_UnixPangoRenderInfo *)ri;

	// need this so that isSymbol() and isDingbat() are correct
	setFont(const_cast<GR_Font*>(si.m_pFont));
	
	UT_UTF8String utf8;
	
	UT_sint32 i;
	for(i = 0; i < si.m_iLength; ++i, ++si.m_Text)
	{
		UT_return_val_if_fail(si.m_Text.getStatus() == UTIter_OK, false);
		if(isSymbol())
			utf8 += adobeToUnicode(si.m_Text.getChar());
		else if(isDingbat())
			utf8 += adobeDingbatsToUnicode(si.m_Text.getChar());
		else
			utf8 += si.m_Text.getChar();
	}

	RI->m_iCharCount = si.m_iLength;
	
	if(RI->m_pGlyphs)
	{
		pango_glyph_string_free(RI->m_pGlyphs);
		RI->m_pGlyphs = NULL;
	}
	
	RI->m_pGlyphs = pango_glyph_string_new();

	// before we can call this, we have to set analysis.font
	// Is this the case, or is the font set by pango_itemize()? #TF
	GR_UnixPangoFont * pFont = (GR_UnixPangoFont *) si.m_pFont;

	// We want to do the shaping on a font at it's actual point size
	UT_ASSERT_HARMLESS( !pFont->isGuiFont() );
	
	UT_LocaleTransactor t(LC_NUMERIC, "C");
	UT_String s;
	UT_String_sprintf(s, "%s %f",
					  pFont->getDescription().c_str(),
					  pFont->getPointSize());
		
	PangoFontDescription * pfd = pango_font_description_from_string(s.c_str());
	UT_return_val_if_fail(pfd, false);
	PangoFont * pf = pango_context_load_font(getContext(), pfd);
	pango_font_description_free(pfd);
	
	pItem->m_pi->analysis.font = pf;

	// need to set the embedding level here based on the level of our run
	pItem->m_pi->analysis.level = si.m_iVisDir == UT_BIDI_RTL ? 1 : 0;
	
	pango_shape(utf8.utf8_str(), utf8.byteLength(),
				&(pItem->m_pi->analysis), RI->m_pGlyphs);

	if(RI->m_pLogOffsets)
	{
		delete [] RI->m_pLogOffsets;
	}

	RI->m_pLogOffsets = _calculateLogicalOffsets(RI->m_pGlyphs,
												 si.m_iVisDir,
												 utf8.utf8_str());
	
	// need to transfer data that we will need later from si to RI
	RI->m_iLength = si.m_iLength;

	RI->m_pItem = si.m_pItem;
	RI->m_pFont = si.m_pFont;

	RI->m_eShapingResult = GRSR_ContextSensitiveAndLigatures;

	// remove any justification information -- it will have to be recalculated
	delete[] RI->m_pJustify; RI->m_pJustify = NULL;
	
	// we did our calculations at notional 100%
	RI->m_iZoom = 100;

	return true;
}


UT_sint32 GR_UnixPangoGraphics::getTextWidth(GR_RenderInfo & ri)
{
	xxx_UT_DEBUGMSG(("GR_UnixPangoGraphics::getTextWidth\n"));
	UT_return_val_if_fail(ri.getType() == GRRI_UNIX_PANGO, 0);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;

	UT_return_val_if_fail( RI.m_pGlyphs && RI.m_pLogOffsets, 0 );
	
	GR_UnixPangoFont * pFont = (GR_UnixPangoFont *) RI.m_pFont;
	UT_return_val_if_fail( pFont, 0 );
	
	PangoFont * pf = pFont->getPangoFont();
	UT_return_val_if_fail( pf, 0 );

	UT_sint32 iStart = RI.m_iOffset;
	UT_sint32 iEnd   = RI.m_iOffset + RI.m_iLength;
	
	UT_sint32 iwidth =  _measureExtent (RI.m_pGlyphs, pf, RI.m_iVisDir, NULL,
						   RI.m_pLogOffsets, iStart, iEnd);
	xxx_UT_DEBUGMSG(("TextWidths Pango Font %x height %d text width %d \n",pFont,pFont->getAscent(),iwidth));
	return iwidth;
}

/*!
 * Calculates the extents of string corresponding to glyphstring from
 * *character* offset iStart to iEnd (excluding iEnd);
 *
 * iDir is the visual direction of the text
 * pUtf8 pointer to the corresponding utf8 string; can be NULL if pLogOffsets
 *    is provided
 * pLogOffsets is array of logical offsets (see
 *    gr_UnixPangoRenderInfo::m_pLogOffsets); if NULL, it will be calculated
 *    using the corresponding utf8 string and pointer returned back; the
 *    caller needs to delete[] it when no longer needed.
 *
 * on return iStart and iEnd contain the offset values that correspond to the
 * returned extent (e.g., if the original iStart and/or iEnd are not legal
 * character postions due to clustering rules, these can be different from
 * the requested values).
 */
UT_uint32 GR_UnixPangoGraphics::_measureExtent (PangoGlyphString * pg,
												PangoFont * pf,
												UT_BidiCharType iDir,
												const char * pUtf8,
												int * & pLogOffsets,
												UT_sint32 & iStart,
												UT_sint32 & iEnd)
{
	UT_return_val_if_fail( pg && pf, 0 );
	PangoRectangle LR;

	// need to convert the char offset and length to glyph offsets
	UT_uint32 iGlyphCount = pg->num_glyphs;
	UT_sint32 iOffsetStart = -1, iOffsetEnd = -1;

	if (!pLogOffsets)
	{
		UT_return_val_if_fail( pUtf8, 0 );
		pLogOffsets = _calculateLogicalOffsets (pg, iDir, pUtf8);
	}

	UT_return_val_if_fail( pLogOffsets, 0 );
	
	// loop running in visual plane
	for(UT_uint32 i = 0; i < iGlyphCount; ++i)
	{
		// have to index glyphs in logical plane to hit our logical start
		// offset before the end offset
		UT_uint32 k = (iDir == UT_BIDI_RTL) ? iGlyphCount - i - 1 : i;

		// test for >= -- in case of combining characters, the requested offset
		// might inside the cluster, which is not legal, we take the first
		// offset given to us
		if(iOffsetStart < 0 && pLogOffsets[k] >= iStart)
		{
			iOffsetStart = k;
			iStart = pLogOffsets[k];
			xxx_UT_DEBUGMSG(("::getTextWidth: iOffsetStart == %d\n",
						 iOffsetStart));
			continue;
		}
		

		if(pLogOffsets[k] >= iEnd)
		{
			iOffsetEnd = k;
			iEnd = pLogOffsets[k];
			xxx_UT_DEBUGMSG(("::getTextWidth: iOffsetEnd == %d\n",
							 iOffsetEnd));
			break;
		}
	}

	UT_ASSERT_HARMLESS( iOffsetStart >= 0 );
	
	if(iOffsetEnd < 0 && iDir == UT_BIDI_LTR)
	{
		// to the end
		iOffsetEnd = iGlyphCount;
	}

	if (iDir == UT_BIDI_RTL)
	{
		// in RTL text, the start offset will be higher than the end offset
		// and we will want to measure (iOffsetEnd, iOffsetStart>
		UT_sint32 t  = iOffsetStart;
		iOffsetStart = iOffsetEnd + 1; // + 1 excludes iOffsetEnd
		iOffsetEnd   = t + 1;          // + 1 includes iOffsetStart
	}
	
	pango_glyph_string_extents_range(pg,
									 iOffsetStart,
									 iOffsetEnd, pf, NULL, &LR);

	xxx_UT_DEBUGMSG(("::getTextWidth start %d, end %d, w %d, x %d\n",
				 iOffsetStart, iOffsetEnd, LR.width, LR.x));

	return ptlu(LR.width + LR.x);
}


/*!
    Do any pre-processing that might be needed for rendering our text (This
    function is guaranteed to be called just before renderChars(), but where
    drawing of a single RI item is done inside of a loop, e.g., drawing the
    different segments of partially selected run, this function can be take out
    of the loop.)
*/
void GR_UnixPangoGraphics::prepareToRenderChars(GR_RenderInfo & ri)
{
	// the only thing we need to do here is to make sure that the glyph metrics
	// are calculated to a correct zoom level.
	UT_return_if_fail(ri.getType() == GRRI_UNIX_PANGO);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;

	if(RI.m_iZoom != getZoomPercentage())
	{
		_scaleCharacterMetrics(RI);
	}
}

/*!
    The offset passed to us as part of ri is a visual offset
*/
void GR_UnixPangoGraphics::renderChars(GR_RenderInfo & ri)
{
	xxx_UT_DEBUGMSG(("GR_UnixPangoGraphics::renderChars\n"));
	UT_return_if_fail(ri.getType() == GRRI_UNIX_PANGO);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;
	GR_UnixPangoFont * pFont = (GR_UnixPangoFont *)RI.m_pFont;
	GR_UnixPangoItem * pItem = (GR_UnixPangoItem *)RI.m_pItem;
	UT_return_if_fail(pItem && pFont && pFont->getPangoFont());

	if(RI.m_iLength == 0)
		return;

	xxx_UT_DEBUGMSG(("Pango renderChars: xoff %d yoff %d\n", RI.m_xoff, RI.m_yoff));
	UT_sint32 xoff = _tduX(RI.m_xoff);
	UT_sint32 yoff = _tduY(RI.m_yoff + getFontAscent(pFont));

	xxx_UT_DEBUGMSG(("about to pango_xft_render xoff %d yoff %d\n", xoff, yoff));
	UT_return_if_fail(m_pXftDraw && RI.m_pGlyphs);

	// TODO -- test here for the endpoint as well
	if(RI.m_iOffset == 0 &&
	   (RI.m_iLength == (UT_sint32)RI.m_iCharCount || !RI.m_iCharCount))
	{
		pango_xft_render(m_pXftDraw, &m_XftColor, pFont->getPangoFont(),
						 RI.m_pGlyphs, xoff, yoff);
	}
	else
	{
		// This is really stupid -- Pango provides no way of drawing substrings,
		// so we need to create a new glyph string, that only contains the
		// subset This is complicated by the fact that all offsets in the Pango
		// api are stupid byte offsets in to utf8 strings, not character offsets
		UT_return_if_fail( RI.m_pText );
		UT_TextIterator & text = *RI.m_pText;
		PangoGlyphString gs;

		UT_UTF8String utf8;
		UT_uint32 i;
		
		for(i = 0; i < RI.m_iCharCount && text.getStatus() == UTIter_OK;
			++i, ++text)
		{
			utf8 += text.getChar();
		}

		if(RI.m_iCharCount > i)
		{
			// it seems the iterator run out on us
			// this should probably not happen
			UT_DEBUGMSG(("gr_UnixPangoGraphics::renderChars: iterator too short\n"));
			return;
		}
		
		UT_sint32 iOffsetStart
			= RI.m_iVisDir == UT_BIDI_RTL ?
			                 RI.m_iCharCount - RI.m_iOffset - 1 : RI.m_iOffset;
		
		const char * pUtf8   = utf8.utf8_str();
		const char * pOffset = g_utf8_offset_to_pointer (pUtf8, iOffsetStart);
		
		if (pOffset)
			iOffsetStart = pOffset - pUtf8;
		
		UT_sint32 iOffsetEnd
			= RI.m_iVisDir == UT_BIDI_RTL ?
			                 RI.m_iCharCount - RI.m_iOffset - RI.m_iLength:
			                 RI.m_iOffset + RI.m_iLength - 1;
		
		pOffset = g_utf8_offset_to_pointer (pUtf8, iOffsetEnd);
		
		if (pOffset)
			iOffsetEnd = pOffset - pUtf8;
		
		// now we need to work out the glyph offsets
		UT_sint32 iGlyphsStart = -1;
		UT_sint32 iGlyphsEnd = -1;
		
		i = 0;
		while(i < RI.m_pGlyphs->num_glyphs)
		{
			if(iGlyphsStart < 0 && RI.m_pGlyphs->log_clusters[i] == iOffsetStart)
				iGlyphsStart = i;

			if(RI.m_pGlyphs->log_clusters[i] == iOffsetEnd)
			{
				iGlyphsEnd = i;
				break;
			}
			
			++i;
		}

		// both of these can be 0 (iGlyphsEnd == 0 => only 1 glyph)
		UT_return_if_fail( iGlyphsStart >= 0 && iGlyphsEnd >= 0 );
		xxx_UT_DEBUGMSG(("Drawing glyph subset from %d to %d (offsets %d, %d)\n",
					 iGlyphsStart, iGlyphsEnd,
					 iOffsetStart, iOffsetEnd));
		
		gs.num_glyphs = iGlyphsEnd - iGlyphsStart + 1; // including the last glyph
		gs.glyphs = RI.m_pGlyphs->glyphs + iGlyphsStart;
		gs.log_clusters = RI.m_pGlyphs->log_clusters + iGlyphsStart;

		pango_xft_render(m_pXftDraw, &m_XftColor, pFont->getPangoFont(),
						 &gs, xoff, yoff);

	}
}

void GR_UnixPangoGraphics::_scaleCharacterMetrics(GR_UnixPangoRenderInfo & RI)
{
	UT_uint32 iZoom = getZoomPercentage();
	if(RI.m_iZoom == iZoom)
		return;

	for(int i = 0; i < RI.m_pGlyphs->num_glyphs; ++i)
	{
		RI.m_pGlyphs->glyphs[i].geometry.x_offset =
			(int)((double)RI.m_pGlyphs->glyphs[i].geometry.x_offset *
				  (double)iZoom / (double)RI.m_iZoom + 0.5) ;

		RI.m_pGlyphs->glyphs[i].geometry.y_offset =
			(int)((double)RI.m_pGlyphs->glyphs[i].geometry.y_offset *
				  (double)iZoom / (double)RI.m_iZoom  + 0.5);

		RI.m_pGlyphs->glyphs[i].geometry.width =
			(int)((double)RI.m_pGlyphs->glyphs[i].geometry.width *
				  (double)iZoom / (double)RI.m_iZoom + 0.5);
	}

	RI.m_iZoom = iZoom;
}


void GR_UnixPangoGraphics::_scaleJustification(GR_UnixPangoRenderInfo & RI)
{
	UT_uint32 iZoom = getZoomPercentage();
	if(RI.m_iZoom == iZoom)
		return;

	for(int i = 0; i < RI.m_pGlyphs->num_glyphs; ++i)
	{
		RI.m_pJustify[i] =
			(int)((double)RI.m_pJustify[i] * (double)iZoom / (double)RI.m_iZoom + 0.5) ;
	}
}


/*!
   This function is called after shaping and before any operations are done on
   the glyphs. Although Pango does not have a separate character placement
   stage, we need to scale the glyph metrics to appropriate zoom level (since we
   shape @100% zoom).

   NB: this is probably not ideal with int arithmetic as moving from one zoom to
   another, and back we are bound to end up with incorrect metrics due to
   rounding errors.

*/
void GR_UnixPangoGraphics::measureRenderedCharWidths(GR_RenderInfo & ri)
{
	UT_return_if_fail(ri.getType() == GRRI_UNIX_PANGO);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;

	_scaleCharacterMetrics(RI);

	if(RI.m_pJustify)
	{
		_scaleJustification(RI);
	}
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

	// the buffer has to have at least one more slot than the number of glyphs
	if(!ri.s_pLogAttrs || ri.s_iStaticSize < ri.sUTF8.length() + 1)
	{
		UT_return_val_if_fail(ri.allocStaticBuffers(ri.sUTF8.length()+1),false);
	}
	
	pango_break(ri.sUTF8.utf8_str(),
				ri.sUTF8.byteLength(),
				&(pItem->m_pi->analysis),
				ri.s_pLogAttrs, ri.s_iStaticSize);

	ri.s_pOwnerLogAttrs = &ri;
	return true;
}

bool GR_UnixPangoGraphics::canBreak(GR_RenderInfo & ri, UT_sint32 &iNext, bool bAfter)
{
	UT_return_val_if_fail(ri.getType() == GRRI_UNIX_PANGO &&
						  ri.m_iOffset < ri.m_iLength, false);
	
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;
	iNext = -1;

	if(!RI.s_pLogAttrs || RI.s_pOwnerLogAttrs != &ri)
	{
		if(!_scriptBreak(RI))
			return false;
	}
	
	UT_uint32 iDelta  = 0;
	if(bAfter)
	{
		// the caller wants to know if break can occur on the (logically) right
		// edge of the given character
		
		if(ri.m_iOffset + 1 >= (UT_sint32)RI.s_iStaticSize)
		{
			// we are quering past what have data for
			return false;
		}

		// we will examine the next character, since Pango tells us about
		// breaking on the left edge
		iDelta = 1;
	}

	if(RI.s_pLogAttrs[ri.m_iOffset + iDelta].is_line_break)
		return true;

	// find the next break
	for(UT_sint32 i = ri.m_iOffset + iDelta + 1; i < RI.m_iLength; ++i)
	{
		if(RI.s_pLogAttrs[i].is_line_break)
		{
			iNext = i - iDelta;
			break;
		}
	}
		
	if(iNext == -1)
	{
		// we have not found any breaks in this run -- signal this to the caller
		iNext = -2;
	}
	
	return false;
}


bool GR_UnixPangoGraphics::needsSpecialCaretPositioning(GR_RenderInfo & ri)
{
	// something smarter is needed here, so we do not go through this for
	// langugages that do not need it.
	return true;
}

UT_uint32 GR_UnixPangoGraphics::adjustCaretPosition(GR_RenderInfo & ri, bool bForward)
{
	UT_return_val_if_fail(ri.getType() == GRRI_UNIX_PANGO, 0);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;
	
	if(!RI.s_pLogAttrs || RI.s_pOwnerLogAttrs != &ri)
		_scriptBreak(RI);

	UT_return_val_if_fail( RI.s_pLogAttrs, RI.m_iOffset );
	
	UT_sint32 iOffset = ri.m_iOffset;

	if(bForward)
		while(!RI.s_pLogAttrs[iOffset].is_cursor_position && iOffset < RI.m_iLength)
			iOffset++;
	else
		while(!RI.s_pLogAttrs[iOffset].is_cursor_position && iOffset > 0)
			iOffset--;
	
	return iOffset;
}

void GR_UnixPangoGraphics::adjustDeletePosition(GR_RenderInfo & ri)
{
	UT_return_if_fail(ri.getType() == GRRI_UNIX_PANGO);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;

	if(ri.m_iOffset + ri.m_iLength >= (UT_sint32)RI.m_iCharCount)
		return;
	
	if(!RI.s_pLogAttrs || RI.s_pOwnerLogAttrs != &ri)
		_scriptBreak(RI);

	UT_return_if_fail( RI.s_pLogAttrs);
	
	// deletion can start anywhere, but can only end on cluster boundary if the
	// base character is included in the deletion
	
	// get the offset of the character that follows the deleted segment
	UT_sint32 iNextOffset = (UT_sint32)ri.m_iOffset + ri.m_iLength;

	if(RI.s_pLogAttrs[iNextOffset].is_cursor_position)
	{
		// the next char is a valid caret position, so we are OK
		return;
	}

	// If we got this far, we were asked to end the deletion before a character
	// that is not a valid caret position. We need to determine if the segment
	// we are asked to delete contains the character's base character; if it
	// does, we have to expand the seletion to delete the entire cluster.

	UT_sint32 iOffset = iNextOffset - 1;
	while(iOffset > 0 && iOffset > ri.m_iOffset && !RI.s_pLogAttrs[iOffset].is_cursor_position)
		iOffset--;

	if(RI.s_pLogAttrs[iOffset].is_cursor_position)
	{
		// our delete segment includes the base character, so we have to delete
		// the entire cluster
		iNextOffset = iOffset + 1;
		
		while(iNextOffset < (UT_sint32)RI.s_iStaticSize - 1 // -1 because iLogBuffSize is char count +1
			  && !RI.s_pLogAttrs[iNextOffset].is_cursor_position)
			iNextOffset++;

		
		ri.m_iLength = iNextOffset - ri.m_iOffset;
		return;
	}
	
	// two posibilities: we are deleting only a cluster appendage or the run
	// does not contain base character. The latter should probably not happen,
	// but in both cases we will let the delete proceed as is
}


UT_sint32 GR_UnixPangoGraphics::resetJustification(GR_RenderInfo & ri, bool bPermanent)
{
	UT_return_val_if_fail(ri.getType() == GRRI_UNIX_PANGO, 0);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;

	if(!RI.m_pJustify)
		return 0;

	if(RI.m_iZoom != getZoomPercentage())
		_scaleCharacterMetrics(RI);
	
	UT_sint32 iWidth2 = 0;
	for(UT_sint32 i = 0; i < RI.m_pGlyphs->num_glyphs; ++i)
	{
		iWidth2 += RI.m_pJustify[i];

		// TODO here we need to substract the amount from pango metrics
		RI.m_pGlyphs->glyphs[i].geometry.width -= RI.m_pJustify[i];
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
	
	return ptlu(-iWidth2);
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
	UT_sint32 iLen = RI.m_iLength;
	
	for(; iLen > 0 && text.getStatus() == UTIter_OK; --text, --iLen)
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
    We take the same approach as with Uniscribe; we store the justification
    amount in a separate array of the ri and add it to the offsets before we
    draw. We will probably need some static buffers to speed things up
 */
void GR_UnixPangoGraphics::justify(GR_RenderInfo & ri)
{
	UT_return_if_fail(ri.getType() == GRRI_UNIX_PANGO);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &) ri;
	if(!RI.m_iJustificationPoints || !RI.m_iJustificationAmount || !RI.m_pGlyphs)
		return;

	// make sure that we are not adding apples to oranges
	if(RI.m_iZoom != getZoomPercentage())
		_scaleCharacterMetrics(RI);
	
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

			RI.m_pJustify[i] = ltpu(iSpace);

			// TODO here we need to add this amount the pango metrics
			RI.m_pGlyphs->glyphs[i].geometry.width += RI.m_pJustify[i];
			
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

	// TODO: this is very inefficient: to cache or not to cache ?
	UT_UTF8String utf8;
	
	UT_sint32 i;
	for(i = 0; i < RI.m_iLength; ++i, ++(*(RI.m_pText)))
	{
		UT_return_val_if_fail(RI.m_pText->getStatus() == UTIter_OK, 0);
		utf8 += RI.m_pText->getChar();
	}
	
	int x_pos = ltpu(x);
	int len = utf8.byteLength();
	int iPos = len;
	int iTrailing;
	const char * pUtf8 = utf8.utf8_str();

	/* Another jolly pango function:
	 * if x is greater than the width of the string, it will happily read
	 * pass the end of it.
	 */
	pango_glyph_string_x_to_index(RI.m_pGlyphs,
								  (char*)pUtf8, // do not like this ...
								  len,
								  &(pItem->m_pi->analysis), 
								  x_pos,
								  &iPos,
								  &iTrailing);

	/* if at the end (or pass) the end of the string, just return the length*/
	if (iPos >= len)
	{
		return RI.m_iLength;
	}
	
	i = g_utf8_pointer_to_offset(pUtf8, pUtf8 + iPos);
	
	if(iTrailing)
		i++;

	return i;
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

	// TODO: this is very inefficient: to cache or not to cache ?
	UT_UTF8String utf8;

	UT_sint32 i;
	for(i = 0; i < RI.m_iLength; ++i, ++(*(RI.m_pText)))
	{
		UT_return_if_fail(RI.m_pText->getStatus() == UTIter_OK);
		utf8 += RI.m_pText->getChar();
	}

	UT_sint32 iByteOffset;
	gboolean  bTrailing = TRUE;
	const char * pUtf8 = utf8.utf8_str();
	const char * pOffset = NULL;
	
	if(RI.m_iOffset < 0)
	{
		// we translate negative offsets into leading edge of the first char
		iByteOffset = 0;
		bTrailing = FALSE;
	}
	else if(RI.m_iOffset == 0)
	{
		// trailing edge of the first char
		iByteOffset = 0;
	}
	else if( i > RI.m_iOffset)
	{
		// withing range of our string
		pOffset = g_utf8_offset_to_pointer (pUtf8, RI.m_iOffset);
	}
	else if(i > 1)
	{
		// this is the case where the requested offset is past the end
		// of our string; we will use the last char; as we have more than one
		// character we can use g_utf8_prev_char (); 
		pOffset = g_utf8_prev_char (pUtf8 + utf8.byteLength());
	}
	else
	{
		// something utterly wrong ...
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		iByteOffset = 0;
	}

	if (pOffset)
		iByteOffset = pOffset - pUtf8;
	
	pango_glyph_string_index_to_x (RI.m_pGlyphs,
								   (char*)pUtf8, // do not like this ...
								   utf8.byteLength(),
								   &(pItem->m_pi->analysis), 
								   iByteOffset,
								   bTrailing,
								   &x);

	x = ptlu(x);
	x2 = x;
}

void GR_UnixPangoGraphics::drawChars(const UT_UCSChar* pChars,
									int iCharOffset, int iLength,
									UT_sint32 xoff, UT_sint32 yoff,
									int * pCharWidth)
{
	UT_return_if_fail(m_pXftDraw);

	UT_UTF8String utf8;

	if(isSymbol())
	{
		for(int i = iCharOffset; i < iCharOffset + iLength; ++i)
		{
			utf8 += adobeToUnicode(pChars[i]);
		}
	}
	else if(isDingbat())
	{
		for(int i = iCharOffset; i < iCharOffset + iLength; ++i)
		{
			utf8 += adobeDingbatsToUnicode(pChars[i]);
		}
	}
	else
	{
		utf8.appendUCS4(pChars + iCharOffset, iLength);
	}

	// this function expect indexes in bytes !!! (stupid)
	GList * pItems = pango_itemize(getContext(),
								   utf8.utf8_str(),
								   0, utf8.byteLength(),
								   NULL, NULL);
	
	int iItemCount = g_list_length(pItems);
	PangoGlyphString * pGstring = pango_glyph_string_new();

	UT_sint32 xoffD = _tduX(xoff);
	UT_sint32 yoffD = _tduY(yoff+getFontAscent());

	PangoFont * pf = m_pPFont->getPangoFont();
	PangoRectangle LR;
	
	for(int i = 0; i < iItemCount; ++i)
	{
		PangoItem *pItem = (PangoItem *)g_list_nth(pItems, i)->data;

		if(!pItem)
		{
			UT_ASSERT(pItem);
			pango_glyph_string_free(pGstring);
			return;
		}

		pItem->analysis.font = pf;

		pango_shape(utf8.utf8_str()+ pItem->offset,
					pItem->length,
					&(pItem->analysis),
					pGstring);

		pango_xft_render(m_pXftDraw, &m_XftColor, pf, pGstring, xoffD, yoffD);

		// now advance xoff
		pango_glyph_string_extents(pGstring, pf, NULL, &LR);
		xoffD += PANGO_PIXELS(LR.width);
	}

	pango_glyph_string_free(pGstring);
}

UT_uint32 GR_UnixPangoGraphics::measureString(const UT_UCSChar * pChars,
											  int iCharOffset,
											  int iLength,
											  UT_GrowBufElement* pWidths)
{
	UT_UTF8String utf8;
	UT_uint32 iWidth = 0;

	if(isSymbol())
	{
		for(int i = iCharOffset; i < iCharOffset + iLength; ++i)
		{
			utf8 += adobeToUnicode(pChars[i]);
		}
	}
	else if(isDingbat())
	{
		for(int i = iCharOffset; i < iCharOffset + iLength; ++i)
		{
			utf8 += adobeDingbatsToUnicode(pChars[i]);
		}
	}
	else
	{
		utf8.appendUCS4(pChars + iCharOffset, iLength);
	}

	// this function expect indexes in bytes !!! (stupid)
	GList * pItems = pango_itemize(getContext(),
								   utf8.utf8_str(),
								   0, utf8.byteLength(),
								   NULL, NULL);
	
	int iItemCount = g_list_length(pItems);
	PangoGlyphString * pGstring = pango_glyph_string_new();

	PangoFont * pf = m_pPFont->getPangoFont();
	PangoRectangle LR;
	UT_uint32 iOffset = 0;
	
	for(int i = 0; i < iItemCount; ++i)
	{
		PangoItem *pItem = (PangoItem *)g_list_nth(pItems, i)->data;

		if(!pItem)
		{
			UT_ASSERT(pItem);
			pango_glyph_string_free(pGstring);
			return 0;
		}

		pItem->analysis.font = pf;

		pango_shape(utf8.utf8_str()+ pItem->offset,
					pItem->length,
					&(pItem->analysis),
					pGstring);

		pango_glyph_string_extents(pGstring, pf, NULL, &LR);
		iWidth += ptlu(LR.width + LR.x);

		int * pLogOffsets = NULL;

		/* this is rather involved, fortunately the width array is not
		 * needed most of the time we use this function in abi
		 */
		if (pWidths)
		{
			int charLength = g_utf8_strlen (utf8.utf8_str()+ pItem->offset,
											-1);
			for (int j = 0; j < charLength; /*increment manually in loop*/)
			{
				UT_sint32 iStart = j;
				UT_sint32 iEnd = j + 1;
				UT_BidiCharType iDir = pItem->analysis.level % 2 ?
					UT_BIDI_RTL : UT_BIDI_LTR;
				
				UT_uint32 iMyWidth =
					_measureExtent (pGstring, pf, iDir,
									utf8.utf8_str()+pItem->offset,
									pLogOffsets, iStart, iEnd);

				if (iEnd == j + 1)
				{
					/* this should be the case most of the time */
					pWidths[iOffset++] = iMyWidth;
				}
				else if (iEnd > j+1)
				{
					for (UT_uint32 k = iOffset;
						 k < iOffset + (iEnd - (j + 1)) + 1;
						 ++k)
					{
						pWidths[iOffset++] = iMyWidth / (iEnd - (j + 1) + 1);
					}
				}
				else
				{
					// iEnd < j+1 -- something badly wrong
					UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
					pWidths[iOffset++] = 0;
					++j;
					continue;
				}
				
				j = iEnd;
			}
		}

		delete [] pLogOffsets;
	}

	pango_glyph_string_free(pGstring);
	return iWidth;
}


void GR_UnixPangoGraphics::setFont(GR_Font * pFont)
{
	UT_return_if_fail( pFont && pFont->getType() == GR_FONT_UNIX_PANGO);

	//PangoFont * pf = (PangoFont*) pFont;
	m_pPFont = static_cast<GR_UnixPangoFont*>(pFont);

	_setIsSymbol(false);
	_setIsDingbat(false);

	char * szUnixFontName = UT_strdup(m_pPFont->getFamily());
	const char * szLCFontName = UT_lowerString(szUnixFontName);

	if (szLCFontName)
	{
		xxx_UT_DEBUGMSG(("GR_UnixPangoGraphics::setFont: %s\n", szLCFontName));
		if(strstr(szLCFontName,"symbol") != NULL)
		{
			if(!strstr(szLCFontName,"starsymbol") &&
			   !strstr(szLCFontName, "opensymbol"))
				_setIsSymbol(true);
		}
		
		if(strstr(szLCFontName,"dingbat"))
			_setIsDingbat(true);
	}
	FREEP(szLCFontName);
	
	if(!m_pPFont->isGuiFont() && m_pPFont->getZoom() != getZoomPercentage())
	{
		m_pPFont->reloadFont(this);
	}
}

void GR_UnixPangoGraphics::setZoomPercentage(UT_uint32 iZoom)
{
	// not sure if we should not call GR_UnixGraphics::setZoomPercentage() here
	// instead
	GR_Graphics::setZoomPercentage (iZoom); // chain up

	if(m_pPFont && !m_pPFont->isGuiFont() && m_pPFont->getZoom() != iZoom)
	{
		m_pPFont->reloadFont(this);
	}
}

GR_Font* GR_UnixPangoGraphics::getDefaultFont(UT_String& fontFamily, const char * pLang)
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
	return pFP->getAscent();
}

UT_uint32 GR_UnixPangoGraphics::getFontDescent(GR_Font *pFont)
{
	UT_return_val_if_fail( pFont, 0 );

	GR_UnixPangoFont * pFP = (GR_UnixPangoFont*) pFont;
	return pFP->getDescent();
}

UT_uint32 GR_UnixPangoGraphics::getFontHeight(GR_Font *pFont)
{
	UT_return_val_if_fail( pFont, 0 );

	GR_UnixPangoFont * pFP = (GR_UnixPangoFont*) pFont;
	return pFP->getAscent() + pFP->getDescent();
}
	
const char* GR_UnixPangoGraphics::findNearestFont(const char* pszFontFamily,
												  const char* pszFontStyle,
												  const char* pszFontVariant,
												  const char* pszFontWeight,
												  const char* pszFontStretch,
												  const char* pszFontSize,
												  const char* pszLang)
{
	UT_String s;
	UT_String_sprintf(s, "%s, %s %s %s %s %s",
					  pszFontFamily,
					  pszFontStyle,
					  pszFontVariant,
					  pszFontWeight,
					  pszFontStretch,
					  pszFontSize);

	const char * cs = s.c_str() + s.length() - 2;

	if(!UT_strcmp(cs, "pt"))
	   s[s.length()-2] = 0;

	UT_DEBUGMSG(("---FinfFont size %s \n",pszFontSize));

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
										 const char* pszFontSize,
										 const char* pszLang)
{
	double dPointSize = UT_convertToPoints(pszFontSize);
	UT_String s;

	// Pango is picky about the string we pass to it -- it cannot handle any 'normal'
	// values, and it will stop parsing when it encounters one.
	const char * pStyle = pszFontStyle;
	const char * pVariant = pszFontVariant;
	const char * pWeight = pszFontWeight;
	const char * pStretch = pszFontStretch;

	if(pszFontFamily && !strcmp(pszFontFamily, "Symbol"))
	{
		pszFontFamily = "Standard Symbols L";
	}
	   
	if(pszFontStyle && *pszFontStyle == 'n')
		pStyle = "";
	else if(pszFontStyle == NULL)
	        pStyle = "";

	if(pszFontVariant && *pszFontVariant == 'n')
		pVariant = "";
	else if(pszFontVariant == NULL)
	        pVariant = "";

	if(pszFontWeight && *pszFontWeight == 'n')
		pWeight = "";
	else if(pszFontWeight == NULL)
	        pWeight = "";

	if(pszFontStretch && *pszFontStretch == 'n')
		pStretch = "";
	else if(pszFontStretch == NULL)
	        pStretch = "";

	if(!pszLang || !*pszLang)
		pszLang = "en-US";
	
	UT_String_sprintf(s, "%s, %s %s %s %s",
					  pszFontFamily,
					  pStyle,
					  pVariant,
					  pWeight,
					  pStretch);
	
	return new GR_UnixPangoFont(s.c_str(), dPointSize, this, pszLang);
}

/*!
 *  This is a very ugly hack, but Pango gives me no public API to find out the
 *  upper extent of the font coverage, so I either have to do this, or have to
 *  iterate over the entire UCS-4 space (which is a non-starter)
 *
 *  The struct definition below must match _PangoCoverage !!!
 *
 *  each block represents 256 characters, so the maximum possible character
 *  value is n_blocks * 256;
 */

struct _MyPangoCoverage
{
  guint ref_count;
  int n_blocks;
  int data_size;
  
  void *blocks;
};

typedef _MyPangoCoverage MyPangoCoverage;


void GR_UnixPangoGraphics::getCoverage(UT_NumberVector& coverage)
{
	UT_return_if_fail(m_pPFont);
	
	PangoCoverage * pc = m_pPFont->getPangoCoverage();
	
	if(!pc)
		return;

	MyPangoCoverage * mpc = (MyPangoCoverage*) pc;
	UT_uint32 iMaxChar = mpc->n_blocks * 256;

	UT_DEBUGMSG(("GR_UnixPangoGraphics::getCoverage: iMaxChar %d\n", iMaxChar));
	
	bool bInRange = false;
	
	for(UT_uint32 i = 0; i < iMaxChar; ++i)
	{
		PangoCoverageLevel pl = pango_coverage_get(pc, i);

		if(PANGO_COVERAGE_NONE == pl || PANGO_COVERAGE_FALLBACK == pl)
		{
			if(bInRange)
			{
				// according to the code in XAP_UnixFont::getCoverage(), the
				// range is of type <x,y)
				coverage.push_back(i);
				bInRange = false;
			}
		}
		else
		{
			if(!bInRange)
			{
				coverage.push_back(i);
				bInRange = true;
			}
		}
	}
}

UT_GenericVector<const char *> *  GR_UnixPangoGraphics::getAllFontNames(void)
{
	FcFontSet* fs;
	fs = FcConfigGetFonts(FcConfigGetCurrent(), FcSetSystem);

	// we know how many fonts there are, so we tell the vector constructor
	UT_GenericVector<const char *> * pVec =
		new UT_GenericVector<const char *>(fs->nfont,fs->nfont);

	UT_return_val_if_fail( pVec, NULL );
	
	for(UT_sint32 i = 0; i < fs->nfont; ++i)
	{
		unsigned char *family;
		FcPatternGetString(fs->fonts[i], FC_FAMILY, 0, &family);
		pVec->addItem((const char *)family);
	}

	return pVec;
}

UT_uint32 GR_UnixPangoGraphics::getAllFontCount()
{
	FcFontSet* fs;
	fs = FcConfigGetFonts(FcConfigGetCurrent(), FcSetSystem);
	return fs->nfont;
}

GR_Font * GR_UnixPangoGraphics::getDefaultFont(GR_Font::FontFamilyEnum f,
											   const char * pszLang)
{
	const char* pszFontFamily = NULL;
	const char* pszFontStyle = "normal";
	const char* pszFontVariant = "normal";
	const char* pszFontWeight = "normal";
	const char* pszFontStretch = "normal";
	const char* pszFontSize = "12pt";

	if(!pszLang)
		pszLang = "en-US";

	switch (f)
	{
		case GR_Font::FF_Roman:
			pszFontFamily = "Times";
			break;

		case GR_Font::FF_Swiss:
			pszFontFamily = "Helvetica";
			break;

		case GR_Font::FF_Modern:
			pszFontFamily = "Courier";
			break;

		case GR_Font::FF_Script:
			pszFontFamily = "Cursive";
			break;

		case GR_Font::FF_Decorative:
			pszFontFamily = "Old English";
			break;

		case GR_Font::FF_Technical:
		case GR_Font::FF_BiDi:
			pszFontFamily = "Arial";
			break;
			
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	return findFont(pszFontFamily,
					pszFontStyle,
					pszFontVariant,
					pszFontWeight,
					pszFontStretch,
					pszFontSize,
					pszLang);
}


GR_Font * GR_UnixPangoGraphics::getGUIFont(void)
{
	if (!m_pPFontGUI)
	{
		// get the font resource
		GtkStyle *tempStyle = gtk_style_new();
		const char *guiFontName = pango_font_description_get_family(tempStyle->font_desc);
		if (!guiFontName)
			guiFontName = "'Times New Roman'";

		UT_UTF8String s = XAP_EncodingManager::get_instance()->getLanguageISOName();

		const char * pCountry
			= XAP_EncodingManager::get_instance()->getLanguageISOTerritory();
		
		if(pCountry)
		{
			s += "-";
			s += pCountry;
		}
		
		m_pPFontGUI = new GR_UnixPangoFont(guiFontName, 11.0, this, s.utf8_str(), true);

		g_object_unref(G_OBJECT(tempStyle));
		
		UT_ASSERT(m_pPFontGUI);
	}

	return m_pPFontGUI;
}

/*!
    Convert device units to pango units
*/
inline int GR_UnixPangoGraphics::dtpu(int d) const
{
	return d * PANGO_SCALE;
}

/*!
    Convert pango units to device units
*/
inline int GR_UnixPangoGraphics::ptdu(int p) const
{
	return PANGO_PIXELS(p);
}

/*!
    Convert pango units to layout units
*/
inline int GR_UnixPangoGraphics::ptlu(int p) const
{
	double d = (double)p * 100.0 * (double) getResolution()/
		((double)getDeviceResolution()*(double)getZoomPercentage()*(double) PANGO_SCALE) + .5;

	return (int) d;
}

/*!
    Convert layout units to pango units
*/
inline int GR_UnixPangoGraphics::ltpu(int l) const
{
	double d = (double)l *
		(double)getDeviceResolution() * (double)PANGO_SCALE * (double)getZoomPercentage()/
		((double)getResolution() * 100.0) + .5;
	
	return (int) d;
}
	

/*!
    Convert pango font units to layout units

    (Pango font units == point size * PANGO_SCALE, hence at zoom of 100% there
    are 20/PANGO_SCALE layout units to each pango font unit.)
*/
inline int GR_UnixPangoGraphics::pftlu(int pf) const
{
	double d = (double)pf * 2000.0 / ((double)getZoomPercentage() * (double)PANGO_SCALE);
	return (int) d;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GR_UnixPangFont implementation
//
GR_UnixPangoFont::GR_UnixPangoFont(const char * pDesc, double dSize,
								   GR_UnixPangoGraphics * pG,
								   const char * pLang,
								   bool bGuiFont):
	m_dPointSize(dSize),
	m_iZoom(0), // forces creation of font by reloadFont()
	m_pf(NULL),
	m_bGuiFont(bGuiFont),
	m_pCover(NULL),
	m_pfd(NULL),
	m_pPLang(NULL),
	m_iAscent(0),
	m_iDescent(0)
{
	m_eType = GR_FONT_UNIX_PANGO;
	UT_return_if_fail( pDesc && pG && pLang);

	m_sDesc = pDesc;
	
	setLanguage(pLang);
	reloadFont(pG);
}

GR_UnixPangoFont::~GR_UnixPangoFont()
{
	if(m_pCover)
		pango_coverage_unref(m_pCover);
	pango_font_description_free(m_pfd);
}

void GR_UnixPangoFont::setLanguage(const char * pLang)
{
	UT_return_if_fail( pLang );

	m_pPLang = pango_language_from_string(pLang); 
}

/*!
    Reloads the Pango font associated with this font, taking into account the
    current level of zoom
*/
void GR_UnixPangoFont::reloadFont(GR_UnixPangoGraphics * pG)
{
	UT_return_if_fail( pG );

	UT_uint32 iZoom = pG->getZoomPercentage();
	if(m_pf && (m_bGuiFont || m_iZoom == iZoom))
		return;
	
	m_iZoom = iZoom;
	
	UT_LocaleTransactor t(LC_NUMERIC, "C");
	UT_String s;
	if(!m_bGuiFont && pG->queryProperties(GR_Graphics::DGP_SCREEN))
		UT_String_sprintf(s, "%s %f", m_sDesc.c_str(), m_dPointSize * (double)m_iZoom / 100.0);
	else
		UT_String_sprintf(s, "%s %f", m_sDesc.c_str(), m_dPointSize);
		

	if(m_pfd)
	{
		pango_font_description_free(m_pfd);
		m_pfd = NULL;
	}
	
	m_pfd = pango_font_description_from_string(s.c_str());
	UT_return_if_fail(m_pfd);
	
	m_pf = pango_context_load_font(pG->getContext(), m_pfd);

	UT_return_if_fail( m_pf );
	// FIXME: we probably want the real language from somewhere
	PangoFontMetrics * pfm = pango_font_get_metrics(m_pf, m_pPLang);
	UT_return_if_fail( pfm);

	// pango_metrics_ functions return in points * PANGO_SCALE (points * 1024)
	m_iAscent = (UT_uint32) pG->ptlu(pango_font_metrics_get_ascent(pfm));
	m_iDescent = (UT_uint32) pG->ptlu(pango_font_metrics_get_descent(pfm));
	UT_DEBUGMSG(("Font Ascent %d point size %f zoom %d \n",m_iAscent, m_dPointSize, m_iZoom));
	pango_font_metrics_unref(pfm);
}


/*!
	Measure the unremapped char to be put into the cache.
	That means measuring it for a font size of 120
*/
UT_sint32 GR_UnixPangoFont::measureUnremappedCharForCache(UT_UCS4Char cChar) const
{
	// this is not implemented because we do not use the width cache (when
	// shaping, it is not possible to measure characters, only glyphs)
	UT_ASSERT_HARMLESS( UT_NOT_IMPLEMENTED );
	return 0;
}

PangoCoverage * GR_UnixPangoFont::getPangoCoverage() const
{
	if(!m_pCover)
	{
		m_pCover = pango_font_get_coverage(m_pf, m_pPLang);
		UT_return_val_if_fail(m_pCover, NULL);
	}

	return m_pCover;
}


/*!
    Determine if character g exists in this font.  We assume here that coverage
    is not affected by font size -- since we only operate with single fonts and
    assume scalable fonts, this should be OK.
    
    NB: it is essential that this function is fast
*/
bool GR_UnixPangoFont::doesGlyphExist(UT_UCS4Char g)
{
	UT_return_val_if_fail( m_pf, false );

	PangoCoverage * pc = getPangoCoverage();
	UT_return_val_if_fail(pc, false);
	
	PangoCoverageLevel eLevel = pango_coverage_get(pc, g);

	if(PANGO_COVERAGE_NONE == eLevel || PANGO_COVERAGE_FALLBACK == eLevel)
		return false;

	return true;
}

bool GR_UnixPangoFont::glyphBox(UT_UCS4Char g, UT_Rect & rec, GR_Graphics * pG)
{
	UT_return_val_if_fail( m_pf, false );
	
	guint iGlyphIndx = pango_fc_font_get_glyph (PANGO_FC_FONT(m_pf), g);
	FT_Face pFace = pango_fc_font_lock_face(PANGO_FC_FONT(m_pf));

	FT_Error error = FT_Load_Glyph(pFace, iGlyphIndx,
								   FT_LOAD_LINEAR_DESIGN |
								   FT_LOAD_IGNORE_TRANSFORM |
								   FT_LOAD_NO_BITMAP |
								   FT_LOAD_NO_SCALE);

	
	if (error)
	{
		pango_fc_font_unlock_face(PANGO_FC_FONT(m_pf));
		return false;
	}

	UT_uint32 iSize = (UT_uint32)(m_dPointSize * (double)pG->getResolution() /
		(double)pG->getDeviceResolution());
	
	rec.left   = static_cast<UT_sint32>(fontPoints2float(iSize, pFace,
														 pFace->glyph->metrics.horiBearingX));
	
	rec.width  = static_cast<UT_sint32>(fontPoints2float(iSize, pFace,
														 pFace->glyph->metrics.width));
	
	rec.top    = static_cast<UT_sint32>(fontPoints2float(iSize, pFace,
														 pFace->glyph->metrics.horiBearingY));
	
	rec.height = static_cast<UT_sint32>(fontPoints2float(iSize, pFace,
														 pFace->glyph->metrics.height));
	
	UT_DEBUGMSG(("GlyphBox: %c [l:%d, w:%d, t:%d, h:%d\n",
				 (char)g, rec.left,rec.width,rec.top,rec.height));

	pango_fc_font_unlock_face(PANGO_FC_FONT(m_pf));
	
	return true;
}

const char* GR_UnixPangoFont::getFamily() const
{
	UT_return_val_if_fail( m_pfd, NULL );
	
	return pango_font_description_get_family(m_pfd);
}


//////////////////////////////////////////////////////////////////////////////
//
// GR_UnixPangoRenderInfo Implementation
//

bool GR_UnixPangoRenderInfo::append(GR_RenderInfo &ri, bool bReverse)
{
	if(s_pOwnerUTF8 == this)
		s_pOwnerUTF8 = NULL;

	if(s_pOwnerLogAttrs == this)
		s_pOwnerLogAttrs = NULL;
	
	delete [] m_pLogOffsets; m_pLogOffsets = NULL;

	// will be set when shaping
	m_iCharCount = 0;
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

	if(s_pOwnerUTF8 == this)
		s_pOwnerUTF8 = NULL;

	if(s_pOwnerLogAttrs == this)
		s_pOwnerLogAttrs = NULL;
	
	delete [] m_pLogOffsets; m_pLogOffsets = NULL;

	// will be set when shaping
	m_iCharCount = 0;
	
	return false;
}

bool GR_UnixPangoRenderInfo::cut(UT_uint32 offset, UT_uint32 iLen, bool bReverse)
{

	if(s_pOwnerUTF8 == this)
		s_pOwnerUTF8 = NULL;

	if(s_pOwnerLogAttrs == this)
		s_pOwnerLogAttrs = NULL;
	
	delete [] m_pLogOffsets; m_pLogOffsets = NULL;
	
	// will be set when shaping
	m_iCharCount = 0;
	return false;
}


bool GR_UnixPangoRenderInfo::isJustified() const
{
    return (m_pJustify != NULL);
}

#ifndef WITHOUT_PRINTING
GR_UnixPangoPrintGraphics::GR_UnixPangoPrintGraphics(XAP_UnixGnomePrintGraphics * pGPG):
	GR_UnixPangoGraphics(),
	m_pGnomePrint(pGPG)
{
#if 0
	/* ascertain the real dpi that xft will be using, so we can match that
	 * for our
	 * gnome-print font map
	 */
	GdkScreen *  gScreen = gdk_screen_get_default();
	int iScreen = gdk_x11_screen_get_screen_number(gScreen);

	FcPattern *pattern;
	double dpi = 0.0; 
	pattern = FcPatternCreate();
	if (pattern)
	{
		XftDefaultSubstitute (GDK_SCREEN_XDISPLAY (gScreen),
							  iScreen,
							  pattern);
		FcPatternGetDouble (pattern, FC_DPI, 0, &dpi); 
		FcPatternDestroy (pattern);
		UT_DEBUGMSG(("@@@@@@@@@@@@@@ retrieved DPI %f @@@@@@@@@@@@@@@@@@ \n",
					 dpi));
		m_iDeviceResolution = (UT_uint32)round (dpi);
	}	
	m_pFontMap = gnome_print_pango_font_map_new_for_dpi(96, FALSE);
#else
	m_pFontMap = gnome_print_pango_get_default_font_map ();
#endif
	m_pContext = gnome_print_pango_create_context(m_pFontMap);
	gnome_print_scale (m_pGnomePrint->getGnomePrintContext(), 72./96., 72./96.);
	
}


GR_UnixPangoPrintGraphics::~GR_UnixPangoPrintGraphics()
{
	delete m_pGnomePrint;
}

GR_Graphics * GR_UnixPangoPrintGraphics::graphicsAllocator(GR_AllocInfo& info)
{
	UT_return_val_if_fail(info.getType() == GRID_UNIX, NULL);
	xxx_UT_DEBUGMSG(("GR_UnixPangoGraphics::graphicsAllocator\n"));
	
	GR_UnixAllocInfo &AI = (GR_UnixAllocInfo&)info;

	//!!!WDG might be right
	if(AI.m_pGnomePrint)
	{
		// printer graphics required
		// This class does not provide printing services -- we need a separate derrived
		// class for that
		return new GR_UnixPangoPrintGraphics(AI.m_pGnomePrint);
	}
	else
	{
		// screen graphics required
		UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
		return NULL;
	}
}

GnomePrintContext * GR_UnixPangoPrintGraphics::getGnomePrintContext() const
{
	return m_pGnomePrint->getGnomePrintContext();
}


UT_sint32 GR_UnixPangoPrintGraphics::scale_ydir (UT_sint32 in) const
{
	return m_pGnomePrint->scale_ydir (in);
}

UT_sint32 GR_UnixPangoPrintGraphics::scale_xdir (UT_sint32 in) const
{
	return m_pGnomePrint->scale_xdir (in);
}


void GR_UnixPangoPrintGraphics::drawChars(const UT_UCSChar* pChars, 
										   int iCharOffset, int iLength,
										   UT_sint32 xoff, UT_sint32 yoff,
										   int * pCharWidths)
{
	// we cannot call XAP_UnixGnomePrintGraphic::drawChars() here because the fonts this
	// class uses are pango fonts, not the PS fonts the class expects

	UT_UTF8String utf8(pChars + iCharOffset, iLength);
	
	GList * pLogItems = pango_itemize(m_pContext, utf8.utf8_str(),
									  0, utf8.byteLength(),
									  NULL, NULL);

	GList * pItems = pango_reorder_items(pLogItems);
	g_list_free(pLogItems);
	
	xoff = _tduX(xoff);
	yoff = m_pGnomePrint->scale_ydir(_tduY(yoff + getFontAscent(m_pPFont)));

	GnomePrintContext * gpc = m_pGnomePrint->getGnomePrintContext();
	UT_return_if_fail( gpc );

	gnome_print_gsave(gpc);
	gnome_print_moveto(gpc, xoff, yoff);
	
	PangoFont * pf = m_pPFont->getPangoFont();
	
	for(unsigned int i = 0; i < g_list_length(pItems); ++i)
	{
		PangoGlyphString * pGlyphs = pango_glyph_string_new();
		PangoItem *pItem = (PangoItem *)g_list_nth(pItems, i)->data;
		pItem->analysis.font = pf;

		pango_shape(utf8.utf8_str() + pItem->offset, pItem->length,
					& pItem->analysis, pGlyphs);

		gnome_print_pango_glyph_string(gpc, pf, pGlyphs);

		pango_glyph_string_free(pGlyphs);
	}

	gnome_print_grestore (gpc);
	g_list_free(pItems);
}


void GR_UnixPangoPrintGraphics::renderChars(GR_RenderInfo & ri)
{
	xxx_UT_DEBUGMSG(("GR_UnixPangoGraphics::renderChars\n"));
	UT_return_if_fail(ri.getType() == GRRI_UNIX_PANGO);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;
	GR_UnixPangoFont * pFont = (GR_UnixPangoFont *)RI.m_pFont;
	GR_UnixPangoItem * pItem = (GR_UnixPangoItem *)RI.m_pItem;
	UT_return_if_fail(pItem && pFont && pFont->getPangoFont() && m_pGnomePrint);

	if(RI.m_iLength == 0)
		return;

	xxx_UT_DEBUGMSG(("PangoPrint renderChars: xoff %d yoff %d\n", RI.m_xoff, RI.m_yoff));
	UT_sint32 xoff = _tduX(RI.m_xoff);
	UT_sint32 yoff = m_pGnomePrint->scale_ydir(_tduY(RI.m_yoff + getFontAscent(pFont)));

	xxx_UT_DEBUGMSG(("about to gnome_print_pango_gplyph_string render xoff %d yoff %d\n",
				 xoff, yoff));

	GnomePrintContext * gpc = m_pGnomePrint->getGnomePrintContext();
	UT_return_if_fail( gpc );

	gnome_print_gsave(gpc);
	gnome_print_moveto(gpc, xoff, yoff);
	
	gnome_print_pango_glyph_string(gpc, pFont->getPangoFont(), RI.m_pGlyphs);

	gnome_print_grestore (gpc);
}

void GR_UnixPangoPrintGraphics::drawLine (UT_sint32 x1, UT_sint32 y1,
										   UT_sint32 x2, UT_sint32 y2)
{
	UT_return_if_fail( m_pGnomePrint );
	m_pGnomePrint->drawLine(x1, y1, x2, y2);
}

bool GR_UnixPangoPrintGraphics::queryProperties(GR_Graphics::Properties gp) const
{
	UT_return_val_if_fail( m_pGnomePrint, false );
	return m_pGnomePrint->queryProperties(gp);
}

void GR_UnixPangoPrintGraphics::getColor(UT_RGBColor& clr)
{
	UT_return_if_fail( m_pGnomePrint );
	m_pGnomePrint->getColor(clr);
}

void GR_UnixPangoPrintGraphics::setColor(const UT_RGBColor& clr)
{
	UT_return_if_fail( m_pGnomePrint );
	m_pGnomePrint->setColor(clr);
}

void GR_UnixPangoPrintGraphics::setLineWidth(UT_sint32 iLineWidth)
{
	UT_return_if_fail( m_pGnomePrint );
	m_pGnomePrint->setLineWidth(iLineWidth);
}

bool GR_UnixPangoPrintGraphics::startPrint(void)
{
	UT_return_val_if_fail( m_pGnomePrint, false );
	return m_pGnomePrint->startPrint();
}

bool GR_UnixPangoPrintGraphics::startPage (const char *szPageLabel,
											UT_uint32 pageNo, bool portrait, 
											UT_uint32 width, UT_uint32 height)
{
	UT_return_val_if_fail( m_pGnomePrint, false );
	return m_pGnomePrint->startPage(szPageLabel, pageNo, portrait, width, height);	
}

bool GR_UnixPangoPrintGraphics::endPrint()
{
	UT_return_val_if_fail( m_pGnomePrint, false );
	return m_pGnomePrint->endPrint();
}

void GR_UnixPangoPrintGraphics::setColorSpace(GR_Graphics::ColorSpace c)
{
	UT_return_if_fail( m_pGnomePrint );
	m_pGnomePrint->setColorSpace(c);
}

GR_Graphics::ColorSpace GR_UnixPangoPrintGraphics::getColorSpace(void) const
{
	UT_return_val_if_fail( m_pGnomePrint, getColorSpace() );
	return m_pGnomePrint->getColorSpace();
}

UT_uint32 GR_UnixPangoPrintGraphics::getDeviceResolution(void) const
{
	UT_return_val_if_fail( m_pGnomePrint, 0 );
	return m_pGnomePrint->getDeviceResolution();
}

void GR_UnixPangoPrintGraphics::drawImage(GR_Image* pImg, UT_sint32 xDest, 
										   UT_sint32 yDest)
{
	UT_return_if_fail( m_pGnomePrint );
	m_pGnomePrint->drawImage(pImg, xDest, yDest);
}

GR_Image* GR_UnixPangoPrintGraphics::createNewImage(const char* pszName, 
													 const UT_ByteBuf* pBB, 
													 UT_sint32 iDisplayWidth,
													 UT_sint32 iDisplayHeight, 
													 GR_Image::GRType iType)
{
	UT_return_val_if_fail( m_pGnomePrint, NULL );
	return m_pGnomePrint->createNewImage(pszName, pBB, iDisplayWidth, iDisplayHeight, iType);	
}

void GR_UnixPangoPrintGraphics::fillRect(const UT_RGBColor& c, 
										  UT_sint32 x, UT_sint32 y, 
										  UT_sint32 w, UT_sint32 h)
{
	UT_return_if_fail( m_pGnomePrint );
	m_pGnomePrint->fillRect(c, x, y, w, h);
}

void GR_UnixPangoPrintGraphics::setCursor(GR_Graphics::Cursor c)
{
	UT_return_if_fail( m_pGnomePrint);
	m_pGnomePrint->setCursor(c);
}

GR_Graphics::Cursor GR_UnixPangoPrintGraphics::getCursor(void) const
{
	UT_return_val_if_fail( m_pGnomePrint, GR_CURSOR_INVALID );
	return m_pGnomePrint->getCursor();	
}

void GR_UnixPangoPrintGraphics::xorLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, 
										 UT_sint32 y2)
{
	UT_return_if_fail( m_pGnomePrint );
	m_pGnomePrint->xorLine(x1, y1, x2, y2);
}

void GR_UnixPangoPrintGraphics::polyLine(UT_Point * pts, 
										  UT_uint32 nPoints)
{
	UT_return_if_fail( m_pGnomePrint );
	m_pGnomePrint->polyLine(pts, nPoints);
}

void GR_UnixPangoPrintGraphics::invertRect(const UT_Rect* pRect)
{
	UT_return_if_fail( m_pGnomePrint );
	m_pGnomePrint->invertRect(pRect);
}

void GR_UnixPangoPrintGraphics::clearArea(UT_sint32 x, UT_sint32 y,
										   UT_sint32 width, UT_sint32 height)
{
	UT_return_if_fail( m_pGnomePrint );
	m_pGnomePrint->clearArea(x, y, width, height);
}

void GR_UnixPangoPrintGraphics::scroll(UT_sint32 x, UT_sint32 y)
{
	UT_return_if_fail( m_pGnomePrint );
	m_pGnomePrint->scroll(x,y);
}

void GR_UnixPangoPrintGraphics::scroll(UT_sint32 x_dest,
										UT_sint32 y_dest,
										UT_sint32 x_src,
										UT_sint32 y_src,
										UT_sint32 width,
										UT_sint32 height)
{
	UT_return_if_fail( m_pGnomePrint );
	m_pGnomePrint->scroll(x_dest, y_dest, x_src, y_src, width, height);
}

UT_RGBColor * GR_UnixPangoPrintGraphics::getColor3D(GR_Color3D c)
{
	UT_return_val_if_fail( m_pGnomePrint, NULL );
	return m_pGnomePrint->getColor3D(c);
}

void GR_UnixPangoPrintGraphics::setColor3D(GR_Color3D c)
{
	UT_return_if_fail( m_pGnomePrint );
	m_pGnomePrint->setColor3D(c);
}

GR_Font* GR_UnixPangoPrintGraphics::getGUIFont()
{
	UT_return_val_if_fail( m_pGnomePrint, NULL );
	return m_pGnomePrint->getGUIFont();
}

void GR_UnixPangoPrintGraphics::fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y,
										  UT_sint32 w, UT_sint32 h)
{
	UT_return_if_fail( m_pGnomePrint );
	m_pGnomePrint->fillRect(c, x, y, w, h);
}

void GR_UnixPangoPrintGraphics::fillRect(GR_Color3D c, UT_Rect &r)
{
	UT_return_if_fail( m_pGnomePrint );
	m_pGnomePrint->fillRect(c, r);
}

void GR_UnixPangoPrintGraphics::setPageSize(char* pageSizeName,
											 UT_uint32 iwidth, UT_uint32 iheight)
{
	UT_return_if_fail( m_pGnomePrint );
	m_pGnomePrint->setPageSize(pageSizeName, iwidth, iheight);
}

void GR_UnixPangoPrintGraphics::setClipRect(const UT_Rect* pRect)
{
	UT_return_if_fail( m_pGnomePrint );
	m_pGnomePrint->setClipRect(pRect);
}

void GR_UnixPangoPrintGraphics::setLineProperties (double inWidth,
												   JoinStyle inJoinStyle,
												   CapStyle inCapStyle,
												   LineStyle inLineStyle)
{
	UT_return_if_fail( m_pGnomePrint );
	m_pGnomePrint->setLineProperties(inWidth, inJoinStyle, inCapStyle, inLineStyle);
}

#endif
