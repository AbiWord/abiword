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
#include <pango/pango-item.h>
#include <pango/pangoxft.h>

UT_uint32 GR_UnixPangoGraphics::s_iInstanceCount = 0;
UT_VersionInfo GR_UnixPangoGraphics::s_Version;
int GR_UnixPangoGraphics::s_iMaxScript = 0;


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
		GR_RenderInfo(t),m_pGlyphs(NULL)
	{
	};
	
	virtual ~GR_UnixPangoRenderInfo()
	    {

	virtual GRRI_Type getType() const {return GRRI_UNIX_PANGO;}
	virtual bool      append(GR_RenderInfo &ri, bool bReverse = false);
	virtual bool      split (GR_RenderInfo *&pri, bool bReverse = false);
	virtual bool      cut(UT_uint32 offset, UT_uint32 iLen, bool bReverse = false);
	virtual bool      isJustified() const;

  public:

	PangoGlyphString* m_pGlyphs;
	
};


GR_UnixPangoGraphics::GR_UnixPangoGraphics(GdkWindow * win, XAP_UnixFontManager * fontManager, XAP_App *app)
	:GR_UnixGraphics(win, fontManager, app), m_pFontMap(NULL),m_pContext(NULL)
{
	UT_DEBUGMSG(("GR_UnixPangoGraphics::GR_UnixPangoGraphics using pango (window)\n"));
	m_pContext = gdk_pango_context_get_for_screen(gdk_screen_get_default());
	m_pFontMap = pango_xft_get_font_map();
}
		
GR_UnixPangoGraphics::GR_UnixPangoGraphics(GdkPixmap * win, XAP_UnixFontManager * fontManager, XAP_App *app, bool bUsePixmap)
	:GR_UnixGraphics(win, fontManager, app, bUsePixmap), m_pFontMap(NULL),m_pContext(NULL)
{
	UT_DEBUGMSG(("GR_UnixPangoGraphics::GR_UnixPangoGraphics using pango (pixmap)\n"));

	// this is wrong, need context for the printer
	m_pContext = gdk_pango_context_get_for_screen(gdk_screen_get_default());
	m_pFontMap = pango_xft_get_font_map();

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
	PangoAttrList *pAttr = pango_attr_list_new();
  
	GList *gItems = pango_itemize(m_pContext, utf8.utf8_str(), 0, utf8.byteLength(), pAttr, NULL);
	iItemCount = g_list_length(gItems);

	//!!!WDG haven't decided what to do about attributes yet
	pango_attr_list_unref(pAttr);
	
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

	// to save time we will use a reasonably sized static buffer and
	// will only allocate one on heap if the static one is too small.
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
	GR_PangoFont * pFont = (GR_PangoFont *) si.m_pFont;
	pItem->m_pi->analysis.font = pFont->m_pf;
	
	pango_shape(utf8.utf8_str(), utf8.byteLength(), &(pItem->m_pi->analysis), RI->m_pGlyphs);

	// need to transfer data that we will need later from si to RI
	RI->m_iLength = si.m_iLength;

	RI->m_pItem = si.m_pItem;
	RI->m_pFont = si.m_pFont;

	RI->m_eShapingResult = GRSR_ContextSensitiveAndLigatures;
	
	return true;
}


UT_sint32 GR_UnixPangoGraphics::getTextWidth(const GR_RenderInfo & ri) const
{
	UT_DEBUGMSG(("GR_UnixPangoGraphics::getTextWidth\n"));
	UT_return_val_if_fail(ri.getType() == GRRI_UNIX_PANGO, 0);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;

	// !!!WDG How to do this for pango?
	UT_sint32 iWidth = 0;
	#if 0
	//UT_uint32 iZoom = getZoomPercentage();
	
	if(!RI.m_pJustify && ri.m_iOffset == 0 && ri.m_iLength == RI.m_iCharCount)
		return (RI.m_ABC.abcA + RI.m_ABC.abcB + RI.m_ABC.abcC);

	UT_return_val_if_fail(ri.m_iOffset + ri.m_iLength <= RI.m_iCharCount, 0);
	
	
	GR_UnixPangoItem & I = (GR_UnixPangoItem &)*ri.m_pItem;
	bool bReverse = I.m_si.a.fRTL != 0;

	for(UT_uint32 i = ri.m_iOffset; i < ri.m_iLength + ri.m_iOffset; ++i)
	{
		if(!bReverse)
		{
			UT_uint32 iMax = RI.m_iCharCount;
			
			if(i < RI.m_iCharCount - 1)
				iMax = RI.m_pClust[i+1];

			for(UT_uint32 j = RI.m_pClust[i]; j < iMax; ++j)
			{
				iWidth += RI.m_pAdvances[j];

				if(RI.m_pJustify)
					iWidth += RI.m_pJustify[j];
			}
		}
		else
		{
			UT_sint32 iMin = -1;
			
			// The offset is a logical offset, and clusters are in logical order.  Indices, however,
			// are in visual order, and the clusters reference glyph indices, so that clust[i] >
			// clust[i+1]

			if(i < RI.m_iCharCount - 1)
				iMin = RI.m_pClust[i+1];

			for(UT_sint32 j = (UT_sint32)RI.m_pClust[i]; j > iMin; --j)
			{
				iWidth += RI.m_pAdvances[j];

				if(RI.m_pJustify)
					iWidth += RI.m_pJustify[j];
			}
		}
	}
	#endif

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
	UT_return_if_fail(pItem && pFont && pFont->m_pf);

	if(RI.m_iLength == 0)
		return;

	UT_DEBUGMSG(("renderChars: xoff %d yoff %d\n", RI.m_xoff, RI.m_yoff));
	UT_sint32 xoff = _tduX(RI.m_xoff);
	UT_sint32 yoff = _tduY(RI.m_yoff);

	

	UT_DEBUGMSG(("about to pango_xft_render xoff %d yoff %d\n", xoff, yoff));
	UT_return_if_fail(m_pXftDraw && &m_XftColor && RI.m_pGlyphs);
	UT_return_if_fail(PANGO_XFT_IS_FONT (pFont->m_pf));
	pango_xft_render(m_pXftDraw, &m_XftColor, pFont->m_pf, RI.m_pGlyphs, xoff, yoff);
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

bool GR_UnixPangoGraphics::canBreak(GR_RenderInfo & ri, UT_sint32 &iNext)
{
	UT_return_val_if_fail( UT_NOT_IMPLEMENTED, true );
}



UT_sint32 GR_UnixPangoGraphics::resetJustification(GR_RenderInfo & ri, bool bPermanent)
{
	UT_return_val_if_fail(ri.getType() == GRRI_UNIX_PANGO, 0);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;

	if(!RI.m_pJustify)
		return 0;

	UT_return_val_if_fail( UT_NOT_IMPLEMENTED, 0 );
}

UT_sint32 GR_UnixPangoGraphics::countJustificationPoints(const GR_RenderInfo & ri) const
{
	UT_return_val_if_fail( UT_NOT_IMPLEMENTED, 0 );
}

void GR_UnixPangoGraphics::justify(GR_RenderInfo & ri)
{
	UT_return_if_fail( UT_NOT_IMPLEMENTED );
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
	
	pango_glyph_string_index_to_x (RI.m_pGlyphs,
								   pItem->m_text.utf8_str(),
								   pItem->m_text.byteLength(),
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
	// for now used the base class function -- this is only used for drawing static
	// strings
	GR_UnixGraphics::drawChars(pChars, iCharOffset, iLength, xoff, yoff, pCharWidth);
}


GR_Font* GR_UnixPangoGraphics::getDefaultFont(UT_String& fontFamily)
{
	static GR_PangoFont fontHandle(m_pFontManager->getDefaultFont(), 12);
	fontFamily = fontHandle.getUnixFont()->getName();
	
	return &fontHandle;
}

GR_UnixPangoFont(XAP_UnixFont * font, UT_uint32 size)
{
	// somehow need to create the PangoFont here
	// we do have an xft font created by the base class, but that seems to be of no use to
	// us ... we have to describe the font, and then load it via the map
	// I really wish there was a way to create PangoFont from xft font
	m_pf = pango_font_map_load_font(GR_UnixPangoGraphics::getFontMap(), GR_UnixPangoGraphics::getContext(),
									);
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
	return (m_pJustify != NULL);
}


GR_UnixPangoFont::~GR_UnixPangoFont()
{
};


