/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 2004-2007 Tomas Frydrych
 * Copyright (C) 2009 Hubert Figuiere
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

#include <string>

#include "gr_CairoGraphics.h"
#include "gr_Painter.h"

#include "xap_App.h"
#include "xap_Prefs.h"
#include "xap_Strings.h"
#include "xap_Frame.h"

#include "ut_debugmsg.h"
#include "ut_misc.h"
#include "ut_locale.h"
#include "ut_std_string.h"
#include "ut_std_vector.h"

// need this to include what Pango considers 'low-level' api
#define PANGO_ENABLE_ENGINE

#include <pango/pango-item.h>
#include <pango/pango-engine.h>
#include <pango/pangocairo.h>

#include <math.h>

// only became "public" in pango 1.20. see http://bugzilla.gnome.org/show_bug.cgi?id=472303
#ifndef PANGO_GLYPH_EMPTY
#define PANGO_GLYPH_EMPTY ((PangoGlyph)0x0FFFFFFF)
#endif

#if !PANGO_VERSION_CHECK(1,22,0)
// stuff deprecated in 1.22....
PangoContext* pango_font_map_create_context(PangoFontMap* fontmap)
{
	return pango_cairo_font_map_create_context(PANGO_CAIRO_FONT_MAP(fontmap));
}
#endif

UT_uint32 adobeDingbatsToUnicode(UT_uint32 iAdobe);
UT_uint32 adobeToUnicode(UT_uint32 iAdobe);

UT_uint32      GR_CairoGraphics::s_iInstanceCount = 0;
UT_VersionInfo GR_CairoGraphics::s_Version;
int            GR_CairoGraphics::s_iMaxScript = 0;



/*!
 * The idea is to create a
 * new image from the rectangular segment in device units defined by 
 * UT_Rect rec. The Image should be deleted by the calling routine.
 */
GR_Image * GR_CairoRasterImage::createImageSegment(GR_Graphics * pG,const UT_Rect & rec)
{
	UT_sint32 x = pG->tdu(rec.left);
	UT_sint32 y = pG->tdu(rec.top);
	if(x < 0)
	{
		x = 0;
	}
	if(y < 0)
	{
		y = 0;
	}
	UT_sint32 width = pG->tdu(rec.width);
	UT_sint32 height = pG->tdu(rec.height);
	UT_sint32 dH = getDisplayHeight();
	UT_sint32 dW = getDisplayWidth();
	if(height > dH)
	{
		height = dH;
	}
	if(width > dW)
	{
		width = dW;
	}
	if(x + width > dW)
	{
		width = dW - x;
	}
	if(y + height > dH)
	{
		height = dH - y;
	}
	if(width <= 0)
	{
	        x = dW -1;
		width = 1;
	}
	if(height <= 0)
	{
		y = dH -1;
		height = 1;
	}
	std::string sName("");
	getName(sName);
	sName += UT_std_string_sprintf("_segemnt_%d_%d_%d_%d",x,y,width,height);

	GR_CairoRasterImage * pImage = makeSubimage(sName, x, y, width, height);
	if(pImage) 
	{
		pImage->setDisplaySize(width,height);
	}
	return pImage;
}



GR_CairoPatternImpl::GR_CairoPatternImpl(const char * fileName)
	: m_pattern(NULL)
{
	cairo_surface_t * surface = cairo_image_surface_create_from_png(fileName);
	m_pattern = cairo_pattern_create_for_surface(surface);
	cairo_pattern_set_extend(m_pattern, CAIRO_EXTEND_REPEAT);
	cairo_surface_destroy(surface);
}

GR_CairoPatternImpl::GR_CairoPatternImpl(cairo_surface_t * surf)
	: m_pattern(cairo_pattern_create_for_surface(surf))
{
	cairo_pattern_set_extend(m_pattern, CAIRO_EXTEND_REPEAT);
}

GR_CairoPatternImpl::GR_CairoPatternImpl(const GR_CairoPatternImpl & p)
	: UT_ColorPatImpl(p)
	, m_pattern(cairo_pattern_reference(p.m_pattern))
{
}

GR_CairoPatternImpl::~GR_CairoPatternImpl()
{
	cairo_pattern_destroy(m_pattern);
}


UT_ColorPatImpl * GR_CairoPatternImpl::clone() const
{
	return new GR_CairoPatternImpl(*this);
}


static void _pango_item_list_free(GList * items) 
{
	GList * l;
	for( l = items; l ; l = l->next) {
		if(l->data) {
			pango_item_free(static_cast<PangoItem*>(l->data));
			l->data = NULL;
		}
	}
	g_list_free(items);
}



class GR_CairoPangoItem: public GR_Item
{
	friend class GR_CairoGraphics;
	friend class GR_UnixPangoPrintGraphics;
	friend class GR_PangoRenderInfo;
	
  public:
	virtual ~GR_CairoPangoItem(){ if (m_pi) {pango_item_free(m_pi);}};
	
	virtual GR_ScriptType getType() const {return (GR_ScriptType)m_iType;}
	
	virtual GR_Item *     makeCopy() const
	    {
			return new GR_CairoPangoItem(pango_item_copy(m_pi));
		}
	
	virtual GRRI_Type     getClassId() const {return GRRI_CAIRO_PANGO;}

  protected:
	GR_CairoPangoItem(PangoItem *pi);
	GR_CairoPangoItem() : m_pi(NULL) { }; // just a dummy used to terminate
	                                     // GR_Itemization list

	PangoItem *m_pi;
	UT_uint32 m_iType;
};

GR_CairoPangoItem::GR_CairoPangoItem(PangoItem *pi):
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

class GR_PangoRenderInfo : public GR_RenderInfo
{
  public:
	GR_PangoRenderInfo(GR_ScriptType t):
		GR_RenderInfo(t),
		m_pGlyphs(NULL),
		m_pScaledGlyphs(NULL),
		m_pLogOffsets(NULL),
		m_pJustify(NULL),
		m_iZoom(0),
		m_iCharCount(0),
		m_iShapingAllocNo(0)
	{
		++s_iInstanceCount;
		if(sUTF8 == NULL)
			sUTF8 = new UT_UTF8String("");
	};

	virtual ~GR_PangoRenderInfo()
	{
		delete [] m_pJustify; delete [] m_pLogOffsets;
		if(m_pGlyphs)
			pango_glyph_string_free(m_pGlyphs);
		if(m_pScaledGlyphs)
			pango_glyph_string_free(m_pScaledGlyphs);
		s_iInstanceCount--;

		if(!s_iInstanceCount)
		{
			delete [] s_pLogAttrs;
			s_pLogAttrs = NULL;
			DELETEP(sUTF8);
		}
	};

	virtual GRRI_Type getType() const {return GRRI_CAIRO_PANGO;}
	virtual bool append(GR_RenderInfo &ri, bool bReverse = false);
	virtual bool split (GR_RenderInfo *&pri, bool bReverse = false);
	virtual bool cut(UT_uint32 offset, UT_uint32 iLen, bool bReverse = false);
	virtual bool isJustified() const;
	virtual bool canAppend(GR_RenderInfo &ri) const;

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
	PangoGlyphString* m_pScaledGlyphs;
	int *             m_pLogOffsets;
	int *             m_pJustify;
	UT_uint32         m_iZoom;
	UT_uint32         m_iCharCount;
	UT_uint32         m_iShapingAllocNo;
	
	static UT_UTF8String * sUTF8;
	static GR_PangoRenderInfo * s_pOwnerUTF8;
	static UT_uint32  s_iInstanceCount;
	static UT_uint32  s_iStaticSize;  // size of the static buffers

	static PangoLogAttr *           s_pLogAttrs;
	static GR_PangoRenderInfo * s_pOwnerLogAttrs;
};


GR_PangoRenderInfo * GR_PangoRenderInfo::s_pOwnerUTF8 = NULL;
UT_UTF8String *          GR_PangoRenderInfo::sUTF8 = NULL;
UT_uint32                GR_PangoRenderInfo::s_iInstanceCount = 0;
UT_uint32                GR_PangoRenderInfo::s_iStaticSize = 0;
GR_PangoRenderInfo * GR_PangoRenderInfo::s_pOwnerLogAttrs = NULL;
PangoLogAttr *           GR_PangoRenderInfo::s_pLogAttrs = NULL;


bool GR_PangoRenderInfo::getUTF8Text()
{
	if(s_pOwnerUTF8 == this)
		return true;
	
	UT_return_val_if_fail( m_pText && m_pText->getStatus() == UTIter_OK, false );

	UT_TextIterator & text = *m_pText;
	sUTF8->clear();
	sUTF8->reserve( text.getUpperLimit());
	// we intentionally run this as far as the iterator lets us, even if that is
	// past the end of this item
	for(; text.getStatus() == UTIter_OK; ++text)
	{
		*sUTF8 += text.getChar();
	}

	s_pOwnerUTF8 = this;

	return true;
}

UT_uint32 GR_CairoGraphics::getDefaultDeviceResolution()
{
	PangoFontMap * pFontMap = pango_cairo_font_map_get_default();
	return (UT_uint32) pango_cairo_font_map_get_resolution(PANGO_CAIRO_FONT_MAP(pFontMap));
	// The default font map must not be freed.
}

// TODO maybe consolidate a common constructor again?
GR_CairoGraphics::GR_CairoGraphics(cairo_t *cr, UT_uint32 iDeviceResolution)
  :	m_pFontMap(NULL),
	m_pContext(NULL),
	m_pLayoutFontMap(NULL),
	m_pLayoutContext(NULL),
	m_pPFont(NULL),
	m_pPFontGUI(NULL),
	m_pAdjustedPangoFont(NULL),
	m_pAdjustedPangoFontDescription(NULL),
	m_iAdjustedPangoFontSize(0),
	m_pAdjustedLayoutPangoFont(NULL),
	m_pAdjustedLayoutPangoFontDescription(NULL),
	m_iAdjustedLayoutPangoFontSize(0),
	m_iDeviceResolution(iDeviceResolution),
	m_cr(cr),
	m_cursor(GR_CURSOR_INVALID),
	m_cs(GR_Graphics::GR_COLORSPACE_COLOR),
	m_curColorDirty(false),
	m_clipRectDirty(false),
	m_lineWidth(1.0),
	m_joinStyle(JOIN_MITER),
	m_capStyle(CAP_BUTT),
	m_lineStyle(LINE_SOLID),
	m_linePropsDirty(false),
	m_bIsSymbol(false),
	m_bIsDingbat(false),
	m_iPrevX1(0),
	m_iPrevX2(0),
	m_iPrevY1(0),
	m_iPrevY2(0),
	m_iPrevRect(1000), // arbitary number that leaves room for plenty of carets
	m_iXORCount(0)
{
	_initPango();
}

GR_CairoGraphics::GR_CairoGraphics()
  :	m_pFontMap(NULL),
	m_pContext(NULL),
	m_pLayoutFontMap(NULL),
	m_pLayoutContext(NULL),
	m_pPFont(NULL),
	m_pPFontGUI(NULL),
	m_pAdjustedPangoFont(NULL),
	m_pAdjustedPangoFontDescription(NULL),
	m_iAdjustedPangoFontSize(0),
	m_pAdjustedLayoutPangoFont(NULL),
	m_pAdjustedLayoutPangoFontDescription(NULL),
	m_iAdjustedLayoutPangoFontSize(0),
	m_iDeviceResolution(getDefaultDeviceResolution()),
	m_cr(NULL),
	m_cursor(GR_CURSOR_INVALID),
	m_cs(GR_Graphics::GR_COLORSPACE_COLOR),
	m_curColorDirty(false),
	m_clipRectDirty(false),
	m_lineWidth(1.0),
	m_joinStyle(JOIN_MITER),
	m_capStyle(CAP_BUTT),
	m_lineStyle(LINE_SOLID),
	m_linePropsDirty(false),
	m_bIsSymbol(false),
	m_bIsDingbat(false),
	m_iPrevX1(0),
	m_iPrevX2(0),
	m_iPrevY1(0),
	m_iPrevY2(0),
	m_iPrevRect(1000),
	m_iXORCount(0)

{
	_initPango();
}


void GR_CairoGraphics::_initCairo()
{
	UT_ASSERT(m_cr);
	cairo_translate(m_cr, 0.5, 0.5);
	cairo_set_line_width (m_cr, 1);
}

void GR_CairoGraphics::_initPango()
{
	m_pFontMap =  pango_cairo_font_map_new();
	pango_cairo_font_map_set_resolution(PANGO_CAIRO_FONT_MAP(m_pFontMap), m_iDeviceResolution);
	m_pContext = pango_font_map_create_context(PANGO_FONT_MAP(m_pFontMap));

	m_pLayoutFontMap = pango_cairo_font_map_new();
	pango_cairo_font_map_set_resolution(PANGO_CAIRO_FONT_MAP(m_pLayoutFontMap), getResolution());	
	m_pLayoutContext = pango_font_map_create_context(PANGO_FONT_MAP(m_pLayoutFontMap));

	UT_DEBUGMSG(("Created LayoutFontMap %p Layout Context %p resolution %d device resolution %d \n", 
				 m_pLayoutFontMap,	m_pLayoutContext, getResolution(),
				 m_iDeviceResolution));
}

GR_CairoGraphics::~GR_CairoGraphics()
{
	xxx_UT_DEBUGMSG(("Deleting UnixPangoGraphics %x \n",this));

	// free m_vSaveRect & m_vSaveRectBuf elements
	UT_std_vector_sparsepurgeall(m_vSaveRect);
	UT_std_vector_freeall(m_vSaveRectBuf, cairo_surface_destroy);

	cairo_destroy(m_cr);
	m_cr = NULL;

	if(m_pAdjustedPangoFont!= NULL)
	{
		g_object_unref(m_pAdjustedPangoFont);
	}
	if(m_pAdjustedPangoFontDescription)
	{
		pango_font_description_free(m_pAdjustedPangoFontDescription);
	}
	if(m_pAdjustedLayoutPangoFont!= NULL)
	{
		g_object_unref(m_pAdjustedLayoutPangoFont);
	}
	if(m_pAdjustedLayoutPangoFontDescription)
	{
		pango_font_description_free(m_pAdjustedLayoutPangoFontDescription);
	}
	if (m_pContext != NULL)
	{
		g_object_unref(m_pContext);
	}

	_destroyFonts();
	delete m_pPFontGUI;
	if(m_pLayoutContext) {
		g_object_unref(m_pLayoutContext);
	}
	if(m_pFontMap) {
		g_object_unref(m_pFontMap);
	}

	// MES After much reading and playing I discovered that the
	// FontMap gets unreferenced after every font that uses it is 
	// removed provied Context is also unrefed. Leaving the unref of
	// the FontMap causes an intermitent crash on exit, particularly on 
	// documents with lots of Math. This fixes those crashes and checks with
	// valgrind show no measureable increase in leacked memory.
	// 
	// Hub: but we still leak and the doc say to unref.
	if (m_pLayoutFontMap) {
		g_object_unref(m_pLayoutFontMap);
		m_pLayoutFontMap = NULL;
	}
}

void GR_CairoGraphics::resetFontMapResolution(void)
{
	pango_cairo_font_map_set_resolution(PANGO_CAIRO_FONT_MAP(m_pFontMap), m_iDeviceResolution);	
}

bool GR_CairoGraphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
	{
		case DGP_SCREEN:
		case DGP_OPAQUEOVERLAY:
			return true;
		case DGP_PAPER:
			return false;
		default:
			UT_ASSERT(0);
			return false;
	}
}


bool GR_CairoGraphics::startPrint(void)
{
	UT_ASSERT(0);
	return false;
}

bool GR_CairoGraphics::startPage(const char * /*szPageLabel*/, UT_uint32 /*pageNumber*/,
								bool /*bPortrait*/, UT_uint32 /*iWidth*/, UT_uint32 /*iHeight*/)
{
	UT_ASSERT(0);
	return false;
}

bool GR_CairoGraphics::endPrint(void)
{
	UT_ASSERT(0);
	return false;
}

void GR_CairoGraphics::drawGlyph(UT_uint32 Char, UT_sint32 xoff, UT_sint32 yoff)
{
	drawChars(&Char, 0, 1, xoff, yoff, NULL);
}

void GR_CairoGraphics::setColorSpace(GR_Graphics::ColorSpace /* c */)
{
	// we only use ONE color space here now (GdkRGB's space)
	// and we don't let people change that on us.
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

GR_Graphics::ColorSpace GR_CairoGraphics::getColorSpace(void) const
{
	return m_cs;
}


GR_Graphics::Cursor GR_CairoGraphics::getCursor(void) const
{
	return m_cursor;
}

void GR_CairoGraphics::setColor3D(GR_Color3D c)
{
	UT_ASSERT(c < COUNT_3D_COLORS);

	setColor(m_3dColors[c]);
}

bool GR_CairoGraphics::getColor3D(GR_Color3D name, UT_RGBColor &color)
{
	if (m_bHave3DColors) {
		color = m_3dColors[name];
		return true;
	}
	return false;
}


UT_uint32 GR_CairoGraphics::getDeviceResolution(void) const
{
	return m_iDeviceResolution;
}


UT_sint32 GR_CairoGraphics::measureUnRemappedChar(const UT_UCSChar c, UT_uint32 * height)
{
        if (height) { 
		*height = 0;
	}
	UT_sint32 w = measureString(&c, 0, 1, NULL, height);
	return w;
}

bool GR_CairoGraphics::itemize(UT_TextIterator & text, GR_Itemization & I)
{
	// Performance is not of the highest priorty, as this function gets only
	// called once on each text fragment on load or keyboard entry
	xxx_UT_DEBUGMSG(("GR_CairoGraphics::itemize\n"));
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

	UT_uint32 iByteLength = utf8.byteLength();
	
	PangoAttrList *pAttrList = pango_attr_list_new();
	PangoAttrIterator *pIter = pango_attr_list_get_iterator (pAttrList);
	const GR_PangoFont * pFont = (const GR_PangoFont *) I.getFont();

	if (pFont)
	{
		const PangoFontDescription * pfd = pFont->getPangoDescription();
		PangoAttribute * pAttr = pango_attr_font_desc_new (pfd);
		pAttr->start_index = 0;
		pAttr->end_index = iByteLength;
		pango_attr_list_insert(pAttrList, pAttr);		
	}

	const char * pLang = I.getLang();

	if (pLang)
	{
		PangoLanguage  * pl = pango_language_from_string(pLang);
		PangoAttribute * pAttr = pango_attr_language_new (pl);		
		pAttr->start_index = 0;
		pAttr->end_index = iByteLength;
		pango_attr_list_insert(pAttrList, pAttr);		
	}
	
	UT_uint32 iItemCount;

	// this will result in itemization assuming base direction of 0
	// we set the appropriate embedding level later in shape()
	GList *gItems = pango_itemize(m_pContext,
								  utf8.utf8_str(),
								  0, iByteLength,
								  pAttrList, pIter);

	pango_attr_iterator_destroy (pIter);
	pango_attr_list_unref (pAttrList);
	
	iItemCount = g_list_length(gItems);

	// now we process the ouptut
	UT_uint32 iOffset = 0;
	xxx_UT_DEBUGMSG(("itemize: number of items %d\n", iItemCount));
	for(i = 0; i < iItemCount; ++i)
	{
		xxx_UT_DEBUGMSG(("itemize: creating item %d\n", i));
		PangoItem *pItem = (PangoItem *)g_list_nth(gItems, i)->data;
		GR_CairoPangoItem * pI = new GR_CairoPangoItem(pItem);

#if 0 //def DEBUG
		PangoFont * pf = pI->m_pi->analysis.font;
		PangoFontDescription * pfd = pango_font_describe (pf);
		char * pfds = pango_font_description_to_string (pfd);
		
		PangoLanguage * lang = pI->m_pi->analysis.language;

		UT_DEBUGMSG(("@@@@ ===== Item [%s] [%s] =====\n",
					 pfds, pango_language_to_string(lang)));
		
		pango_font_description_free (pfd);
		g_free (pfds);
#endif
		
		I.addItem(iOffset, pI);
		iOffset += pItem->num_chars;
	}

	I.addItem(iPosEnd - iPosStart + 1, new GR_CairoPangoItem());

	g_list_free(gItems);
	
	xxx_UT_DEBUGMSG(("itemize succeeded\n"));
	return true;
}

int *
GR_CairoGraphics::_calculateLogicalOffsets (PangoGlyphString * pGlyphs,
												UT_BidiCharType iVisDir,
												const char * pUtf8)
{
	UT_return_val_if_fail( pGlyphs && pUtf8, NULL );
	
	// pGlyphs contains logical cluster info, which is
	// unfortunately indexed to bytes in the utf-8 string, not characters --
	// this is real pain and we have to convert it.
	
	int * pLogOffsets = new int [pGlyphs->num_glyphs];
 
	// In LTR text, the values in log_clusters are guaranteed to be increasing,
	// in RTL text, the values in log_clusters are decreasing
	
	glong offset = 0;
	const gchar *s = pUtf8;
	
	if (iVisDir == UT_BIDI_LTR ||
		(pGlyphs->num_glyphs > 1 &&
		 pGlyphs->log_clusters[0] < pGlyphs->log_clusters[1]))
	{
		for(int i = 0; i < pGlyphs->num_glyphs; ++i)
		{

			int iOff = pGlyphs->log_clusters[i];
			
		//      below is equivalent to 
		//	pLogOffsets[i] =  g_utf8_pointer_to_offset (pUtf8, pUtf8 + iOff);
		// 	but avoids quadratic behavior because we use s and offset from 
		//      previous iteration

    		        while (s <  pUtf8 + iOff)
		        {
				s = g_utf8_next_char (s);
				offset++;
		        }
			pLogOffsets[i] =  offset;
		}
	}
	else // GR_ShapingInfo.m_iVisDir == UT_BIDI_RTL)
	{
		for(int i = pGlyphs->num_glyphs - 1; i >= 0; --i)
		{
			int iOff = pGlyphs->log_clusters[i];
  		        while (s <  pUtf8 + iOff)
		        {
				s = g_utf8_next_char (s);
				offset++;
		        }			
			pLogOffsets[i] =  offset;
		}
	}

	return pLogOffsets;
}

bool GR_CairoGraphics::shape(GR_ShapingInfo & si, GR_RenderInfo *& ri)
{
	xxx_UT_DEBUGMSG(("GR_CairoGraphics::shape, len %d\n", si.m_iLength));
	UT_return_val_if_fail(si.m_pItem &&
						  si.m_pItem->getClassId() == GRRI_CAIRO_PANGO &&
						  si.m_pFont, false);
	
	const GR_CairoPangoItem * pItem =
		static_cast<const GR_CairoPangoItem *>(si.m_pItem);

	PangoFontset * pfs = NULL;
	PangoFont    * pFontSubst = NULL;
	
	if(!ri)
	{
		// this simply allocates new instance of the RI which this function
		// will fill with meaningful data
		ri = new GR_PangoRenderInfo(pItem->getType());
		UT_return_val_if_fail(ri, false);
	}
	else
	{
		UT_return_val_if_fail(ri->getType() == GRRI_CAIRO_PANGO, false);
	}

	GR_PangoRenderInfo * RI = (GR_PangoRenderInfo *)ri;

	// need this so that isSymbol() and isDingbat() are correct
	setFont(si.m_pFont);

	/*
	 * Pango does a royally bad job of the font substitution in
	 * pango_itemize(): it will happily return 'Times New Roman' as
	 * font when we have requested 'Arial', even though the latter is
	 * present and has the necessary coverage. Consequently we have to
	 * do the font substitution manually even on the first shapping.
	 *
	 * If the font has changed from the one for which we previously shapped
	 * (or have not shaped, in which case alloc no is 0), we load a fontset
	 * for the requested font description. Later on, we pick the best font
	 * for each character in this run.
	 */
	if(RI->m_iShapingAllocNo != si.m_pFont->getAllocNumber())
	{
		//UT_DEBUGMSG(("@@@@ ===== Font change %d -> %d\n",
		//			 RI->m_iShapingAllocNo,
		//			 si.m_pFont->getAllocNumber()));
			
		const GR_PangoFont * pFont =
			static_cast<const GR_PangoFont*>(si.m_pFont);

		pfs = pango_font_map_load_fontset (getFontMap(),
										   getContext(),
										   pFont->getPangoDescription(),
										   pItem->m_pi->analysis.language);
	}
	
	UT_UTF8String utf8;
	utf8.reserve(si.m_iLength);
	bool previousWasSpace = si.m_previousWasSpace;

	UT_sint32 i;
	for(i = 0; i < si.m_iLength; ++i, ++si.m_Text)
	{
		UT_return_val_if_fail(si.m_Text.getStatus() == UTIter_OK, false);
		UT_UCS4Char c = si.m_Text.getChar();
		if(isSymbol())
			utf8 += adobeToUnicode(c);
		else if(isDingbat())
			utf8 += adobeDingbatsToUnicode(c);
		else {
			
			if (si.m_TextTransform == GR_ShapingInfo::LOWERCASE)
				c = g_unichar_tolower(c);
			else if (si.m_TextTransform == GR_ShapingInfo::UPPERCASE)
				c = g_unichar_toupper(c);
			else if (si.m_TextTransform == GR_ShapingInfo::CAPITALIZE) {
				if (previousWasSpace) {
					c = g_unichar_toupper(c);
				}
			} // else si.m_TextTransform == GR_ShapingInfo::NONE
			
			utf8 += c;
			previousWasSpace = g_unichar_isspace(c);
		}

		if (pfs)
		{
			/*
			 * A font change; get the best font for this character
			 */
			PangoFont * font = pango_fontset_get_font (pfs, c);
			
			if (!font)
			{
				/*
				 * We did not find a suitable font -- nothing we can do.
				 */
				UT_DEBUGMSG(("@@@@ ===== Failed to find font for u%04x\n", c));
			}
			else if (pFontSubst && (pFontSubst != font))
			{
				/*
			     * Ok, the font we got for this character does not match
			     * the one we got the for the preceding characters.
			     *
			     * What we could do is to split the run before this character
			     * so we might use two different fonts, but we currently
			     * do not have the infrastructure to do this. Also, doing this
			     * breaks when the missing glyph is a combining character.
			     *
			     * Alternatively, we would need to maintain an internal list
			     * of fonts for each section, but that would mean also to
			     * maintain separate glyph strings, which would be a nightmare.
			     *
			     * We can limit this from happening by preventing items that
			     * will use different font from merging, which I have now done,
			     * but again, this does not work when combining characters are
			     * involved, because we cannot draw the combining character on
			     * it's own.
			     *
			     * TODO -- devise a sensible way of handling this.
			     */
#if DEBUG
				PangoFontDescription * pfd = pango_font_describe (pFontSubst);
				char * sFontSubst = pango_font_description_to_string (pfd);
				pango_font_description_free (pfd);
				pfd = pango_font_describe (font);
				char * sFont = pango_font_description_to_string (pfd);
				pango_font_description_free (pfd);
				UT_DEBUGMSG(("@@@@ ===== Font for u%04x (%s) does not match "
							 "earlier font %s\n", c, sFont, sFontSubst));
				g_free (sFontSubst);
				g_free (sFont);
#endif
				g_object_unref (G_OBJECT (pFontSubst));
				pFontSubst = font;
			}
			else if (pFontSubst == font)
			{
				/* We now have two references to this font, rectify */
				g_object_unref (G_OBJECT (font));
			}
			else
			{
				pFontSubst = font;
			}

		}
	}

	if(pfs) 
	{
		g_object_unref((GObject*)pfs);
		pfs = NULL;
	}
	if (pFontSubst)
	{
		/*
		 * We are doing font substitution -- release the font previously
		 * stored in the PangoAnalysis and replace it with this one.
		 */
		if (pItem->m_pi->analysis.font)
			g_object_unref (G_OBJECT (pItem->m_pi->analysis.font));
		
		pItem->m_pi->analysis.font = (PangoFont*)pFontSubst;
	}
	
	RI->m_iCharCount = si.m_iLength;
	
	if(RI->m_pGlyphs)
	{
		pango_glyph_string_free(RI->m_pGlyphs);
		RI->m_pGlyphs = NULL;
	}
	
	if(RI->m_pScaledGlyphs)
	{
		pango_glyph_string_free(RI->m_pScaledGlyphs);
		RI->m_pScaledGlyphs = NULL;
	}
	
	RI->m_pGlyphs = pango_glyph_string_new();
	RI->m_pScaledGlyphs = pango_glyph_string_new();

	/*
	 * We want to do the shaping on a font at it's actual point size, so we
	 * cannot use the font in our PangoAnalysis structure, which we will
	 * later use for drawing, and which will be adjusted for the current
	 * zoom.
	 */
	UT_LocaleTransactor t(LC_NUMERIC, "C");
	UT_String              s;
	PangoFont            * pPangoFontOrig = pItem->m_pi->analysis.font;
	const GR_PangoFont * pFont = static_cast<const GR_PangoFont *>(si.m_pFont);
	PangoFontDescription * pfd;
	
	if (PANGO_IS_FONT(pPangoFontOrig))
	{
		pfd = pango_font_describe (pPangoFontOrig);
		double dSize = (double)PANGO_SCALE * pFont->getPointSize();
		pango_font_description_set_size (pfd, (gint)dSize);

#if 0 //def DEBUG
		char * s = pango_font_description_to_string (pfd);
		UT_DEBUGMSG(("@@@@ ===== Shaping with font [%s]\n", s));
		g_free (s);
#endif
	}
	else
	{
		UT_ASSERT_HARMLESS( !pFont->isGuiFont() );
		UT_String_sprintf(s, "%s %f",
						  pFont->getDescription().c_str(),
						  pFont->getPointSize());
		
		pfd = pango_font_description_from_string(s.c_str());
	}

	UT_return_val_if_fail(pfd, false);
	PangoFont * pf = pango_context_load_font(getLayoutContext(), pfd);
	pango_font_description_free(pfd);
	
	// no need to ref pf because it will replaced right after
	pItem->m_pi->analysis.font = pf;

	// need to set the embedding level here based on the level of our run
	pItem->m_pi->analysis.level = si.m_iVisDir == UT_BIDI_RTL ? 1 : 0;
	
	pango_shape(utf8.utf8_str(), utf8.byteLength(),
				&(pItem->m_pi->analysis), RI->m_pGlyphs);
	pango_shape(utf8.utf8_str(), utf8.byteLength(),
				&(pItem->m_pi->analysis), RI->m_pScaledGlyphs);

	pItem->m_pi->analysis.font = pPangoFontOrig;

	g_object_unref(pf);
		
	if(RI->m_pLogOffsets)
	{
		delete [] RI->m_pLogOffsets;
	}

	RI->m_pLogOffsets = _calculateLogicalOffsets(RI->m_pGlyphs,
												 si.m_iVisDir,
												 utf8.utf8_str());
	
	// need to transfer data that we will need later from si to RI
	RI->m_iLength = si.m_iLength;
	RI->m_pItem   = si.m_pItem;
	RI->m_pFont   = si.m_pFont;
	RI->m_iShapingAllocNo = si.m_pFont->getAllocNumber();
	
	RI->m_eShapingResult = GRSR_ContextSensitiveAndLigatures;

	// remove any justification information -- it will have to be recalculated
	delete[] RI->m_pJustify; RI->m_pJustify = NULL;
	
	// we did our calculations at notional 100%
	RI->m_iZoom = 100;

	// Make sure that s_pOwnerUTF8 and s_pOwnerLogAttrs are not referencing RI
	if (RI->s_pOwnerLogAttrs == RI)
		RI->s_pOwnerLogAttrs = NULL;
	if (RI->s_pOwnerUTF8 == RI)
		RI->s_pOwnerUTF8 = NULL;

	return true;
}


UT_sint32 GR_CairoGraphics::getTextWidth(GR_RenderInfo & ri)
{
	xxx_UT_DEBUGMSG(("GR_CairoGraphics::getTextWidth\n"));
	UT_return_val_if_fail(ri.getType() == GRRI_CAIRO_PANGO, 0);
	GR_PangoRenderInfo & RI = (GR_PangoRenderInfo &)ri;
	const GR_CairoPangoItem * pItem =
		static_cast<const GR_CairoPangoItem *>(RI.m_pItem);

	UT_return_val_if_fail( RI.m_pGlyphs && RI.m_pLogOffsets && pItem, 0 );

	const GR_PangoFont * pFont =
		static_cast<const GR_PangoFont *>(RI.m_pFont);
	UT_return_val_if_fail( pFont, 0 );

	//
	// Actually want the layout font here
	//
	PangoFont * pf = _adjustedLayoutPangoFont(pFont, pItem->m_pi->analysis.font);

	xxx_UT_DEBUGMSG(("Adjusted Layout font %x Adjusted font %x \n",pf));
	UT_return_val_if_fail( pf, 0 );

	UT_sint32 iStart = RI.m_iOffset;
	UT_sint32 iEnd   = RI.m_iOffset + RI.m_iLength;
	
	UT_sint32 iWidth =  _measureExtent (RI.m_pGlyphs, pf, RI.m_iVisDir, NULL,
						   RI.m_pLogOffsets, iStart, iEnd);
	xxx_UT_DEBUGMSG(("TextWidths Pango Font %x height %d text width %d \n",
					 pFont, pFont->getAscent(), iWidth));
	return iWidth;
}

/*!
 * Calculates the extents of string corresponding to glyphstring from
 * *character* offset iStart to iEnd (excluding iEnd);
 *
 * iDir is the visual direction of the text
 * 
 * pUtf8 pointer to the corresponding utf8 string; can be NULL if pLogOffsets
 *    is provided
 *    
 * pLogOffsets is array of logical offsets (see
 *    gr_UnixPangoRenderInfo::m_pLogOffsets); if NULL, it will be calculated
 *    using the corresponding utf8 string and pointer returned back; the
 *    caller needs to delete[] it when no longer needed.
 *
 * On return iStart and iEnd contain the offset values that correspond to the
 * returned extent (e.g., if the original iStart and/or iEnd are not legal
 * character postions due to clustering rules, these can be different from
 * the requested values).
 */
UT_uint32 GR_CairoGraphics::_measureExtent (PangoGlyphString * pg,
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

	xxx_UT_DEBUGMSG(("Font size in _measureExtents %d \n", 
						pango_font_description_get_size(pango_font_describe (pf))));

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

	UT_return_val_if_fail( iOffsetStart >= 0, 0 );
	
	pango_glyph_string_extents_range(pg,
									 iOffsetStart,
									 iOffsetEnd, pf, NULL, &LR);

	xxx_UT_DEBUGMSG(("::getTextWidth start %d, end %d, w %d, x %d\n",
				 iOffsetStart, iOffsetEnd, LR.width, LR.x));

	return ptlunz(LR.width + LR.x);
}


/*!
    Do any pre-processing that might be needed for rendering our text (This
    function is guaranteed to be called just before renderChars(), but where
    drawing of a single RI item is done inside of a loop, e.g., drawing the
    different segments of partially selected run, this function can be take out
    of the loop.)
*/
void GR_CairoGraphics::prepareToRenderChars(GR_RenderInfo & ri)
{
	// the only thing we need to do here is to make sure that the glyph metrics
	// are calculated to a correct zoom level.
	UT_return_if_fail(ri.getType() == GRRI_CAIRO_PANGO);
	GR_PangoRenderInfo & RI = (GR_PangoRenderInfo &)ri;

	if(RI.m_iZoom != getZoomPercentage())
	{
		_scaleCharacterMetrics(RI);
	}
}

/*
 * This is used to get device zoomed PangoFont that is correct for present zoom level.
 * pFont is the font that we are supposed to be using (the user-selected font)
 * pf is the PangoFont that we are actually using (possibly a different,
 * substituted font).
 */
PangoFont *  GR_CairoGraphics::_adjustedPangoFont (const GR_PangoFont * pFont, PangoFont * pf)
{
	UT_return_val_if_fail(pFont, NULL);
	
	if (!pf)
		return pFont->getPangoFont();

	/*
	 * When Pango is doing font substitution for us, the substitute font
	 * we are getting always has size 12pt, so we have to use the size of
	 * our own font to fix this.
	 */
	PangoFontDescription * pfd = pango_font_describe (pf);
	UT_sint32 dSize = (gint)(pFont->getPointSize() * (double)PANGO_SCALE * (double)getZoomPercentage() / 100.0);
	pango_font_description_set_size (pfd, dSize);

	// Check if we have already cached a font with this description and size
	if (m_pAdjustedPangoFontDescription && pango_font_description_equal(m_pAdjustedPangoFontDescription, pfd) && m_iAdjustedPangoFontSize == dSize)
	{
		pango_font_description_free(pfd);
		return m_pAdjustedPangoFont;
	}
	
	/* Create and cache this font to avoid all this huha if we can */
	if (m_pAdjustedPangoFont) 
		g_object_unref(m_pAdjustedPangoFont);
	if (m_pAdjustedPangoFontDescription)
		pango_font_description_free(m_pAdjustedPangoFontDescription);
	
	m_pAdjustedPangoFont = pango_context_load_font(getContext(), pfd);
	m_pAdjustedPangoFontDescription = pfd;
	m_iAdjustedPangoFontSize = dSize;
	
	return m_pAdjustedPangoFont;
}


/*
 * This is used to get Layout PangoFont (that is independent from the zoom level).
 * pFont is the font that we are supposed to be using (the user-selected font)
 * pf is the PangoFont that we are actually using (possibly a different,
 * substituted font).
 */
PangoFont *  GR_CairoGraphics::_adjustedLayoutPangoFont (const GR_PangoFont * pFont, PangoFont * pf)
{
	UT_return_val_if_fail(pFont, NULL);
	
	if (!PANGO_IS_FONT(pf))
		return pFont->getPangoLayoutFont();

	/*
	 * When Pango is doing font substitution for us, the substitute font
	 * we are getting always has size 12pt, so we have to use the size of
	 * our own font to fix this.
	 */
	PangoFontDescription * pfd = pango_font_describe (pf);
	UT_sint32 dSize = (gint)(pFont->getPointSize()*(double)PANGO_SCALE);
	pango_font_description_set_size (pfd, dSize);

	// Check if we have already cached a font with this description and size
	if (m_pAdjustedLayoutPangoFontDescription && pango_font_description_equal(m_pAdjustedLayoutPangoFontDescription, pfd) && m_iAdjustedLayoutPangoFontSize == dSize)
	{
		pango_font_description_free(pfd);
		return m_pAdjustedLayoutPangoFont;
	}
   
	/* Create and cache this font to avoid all this huha if we can */
	if (m_pAdjustedLayoutPangoFont) 
		g_object_unref(m_pAdjustedLayoutPangoFont);
	if (m_pAdjustedLayoutPangoFontDescription)
		pango_font_description_free(m_pAdjustedLayoutPangoFontDescription);
	
	m_pAdjustedLayoutPangoFont = pango_context_load_font(getLayoutContext(), pfd);
	m_pAdjustedLayoutPangoFontDescription = pfd;
	m_iAdjustedLayoutPangoFontSize = dSize;
	
	return m_pAdjustedLayoutPangoFont;
}


/*!
    The offset passed to us as part of ri is a visual offset
*/
void GR_CairoGraphics::renderChars(GR_RenderInfo & ri)
{
	if (m_cr == NULL)
		return;
	UT_return_if_fail(ri.getType() == GRRI_CAIRO_PANGO);
	GR_PangoRenderInfo & RI = (GR_PangoRenderInfo &)ri;
	const GR_PangoFont * pFont = static_cast<const GR_PangoFont *>(RI.m_pFont);
	const GR_CairoPangoItem * pItem =
		static_cast<const GR_CairoPangoItem *>(RI.m_pItem);
	UT_return_if_fail(pItem && pFont && pFont->getPangoFont());
	xxx_UT_DEBUGMSG(("GR_CairoGraphics::renderChars length %d \n",
					 RI.m_iLength));

	if(RI.m_iLength == 0)
		return;

	_setProps();
	//
	// Actually want the zoomed device font here
	//
	PangoFont * pf = _adjustedPangoFont(pFont, pItem->m_pi->analysis.font);

	xxx_UT_DEBUGMSG(("Pango renderChars: xoff %d yoff %d\n",
					 RI.m_xoff, RI.m_yoff));

	double xoff = _tdudX(RI.m_xoff);
	double yoff = _tdudY(RI.m_yoff + getFontAscent(pFont));

	UT_return_if_fail(RI.m_pScaledGlyphs);

	// TODO -- test here for the endpoint as well
	if(RI.m_iOffset == 0 &&
	   (RI.m_iLength == (UT_sint32)RI.m_iCharCount || !RI.m_iCharCount))
	{
		xxx_UT_DEBUGMSG(("Doing Cairo Render now.\n")); 
		cairo_save(m_cr);
		cairo_translate(m_cr, xoff, yoff);
		pango_cairo_show_glyph_string(m_cr, pf, RI.m_pScaledGlyphs);
		cairo_restore(m_cr);
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
			xxx_UT_DEBUGMSG(("gr_UnixPangoGraphics::renderChars: iterator too short\n"));
			return;
		}
		
		UT_sint32 iOffsetStart
			= RI.m_iVisDir == UT_BIDI_RTL ?
			                 RI.m_iCharCount - RI.m_iOffset - RI.m_iLength : RI.m_iOffset;
		xxx_UT_DEBUGMSG(("\n\niOffsetStart (in chars): %d\n", iOffsetStart));
		
		const char * pUtf8   = utf8.utf8_str();
		const char * pOffset = g_utf8_offset_to_pointer (pUtf8, iOffsetStart);
		
		if (pOffset)
			iOffsetStart = pOffset - pUtf8;
		xxx_UT_DEBUGMSG(("iOffsetStart (in bytes): %d\n", iOffsetStart));
		
		UT_sint32 iOffsetEnd
			= RI.m_iVisDir == UT_BIDI_RTL ?
			                 RI.m_iCharCount - RI.m_iOffset:
			                 RI.m_iOffset + RI.m_iLength;
		xxx_UT_DEBUGMSG(("iOffsetEnd (in chars): %d\n", iOffsetEnd));
		
		pOffset = g_utf8_offset_to_pointer (pUtf8, iOffsetEnd);

		xxx_UT_DEBUGMSG(("RI.m_iCharCount: %d, RI.m_iOffset: %d, RI.m_iLength: %d, rtl: %s\n", RI.m_iCharCount, RI.m_iOffset, RI.m_iLength, RI.m_iVisDir == UT_BIDI_RTL ? "yes" : "no"));
			
		if (pOffset)
			iOffsetEnd = pOffset - pUtf8;
		xxx_UT_DEBUGMSG(("iOffsetEnd (in bytes): %d\n", iOffsetEnd));

		//	
		// now we need to work out the glyph offsets given the start and end offsets in bytes
		//
			
		// the glyph end offset should point to the offset of the first glyph that is to be rendered
		UT_sint32 iGlyphsStart = -1;
		// the glyph end offset should point to the offset of the first glyph that is NOT to be rendered anymore
		// (which in the special case of "render everything from iGlyphsStart to the end" for rtl text means
		// offset -1, as index 0 points the the "last" glyph of the string. For ltr text this simply means 
		// offset "num_glyphs".
		UT_sint32 iGlyphsEnd 
			= RI.m_iVisDir == UT_BIDI_RTL ?
							-1 : RI.m_pScaledGlyphs->num_glyphs;

		// count downwards for RTL text, so we include the full character clusters
		i = RI.m_iVisDir == UT_BIDI_RTL ? RI.m_pScaledGlyphs->num_glyphs - 1 : 0;
		while(i < (UT_uint32)RI.m_pScaledGlyphs->num_glyphs)
		{
			xxx_UT_DEBUGMSG(("RI.m_pScaledGlyphs->log_clusters[%d] == %d\n", i, RI.m_pScaledGlyphs->log_clusters[i]));
			if(iGlyphsStart < 0 && RI.m_pScaledGlyphs->log_clusters[i] == iOffsetStart)
				iGlyphsStart = i;

			if(RI.m_pScaledGlyphs->log_clusters[i] == iOffsetEnd)
			{
				iGlyphsEnd = i;
				break;
			}
			
			RI.m_iVisDir == UT_BIDI_RTL ? --i : ++i;
		}
		if (RI.m_iVisDir == UT_BIDI_RTL)
		{
			// Swap the start and end offset for rtl text, so start <= end again.
			// Note that this means that iGlyphsStart now points to the offset of
			// the "last  glyph that should not rendered yet". The glyphs starting from
			// this offset + 1 should be rendered. iGlyphsEnd on the other hand now 
			// points to the offset of the last glyph that should be rendered.
			std::swap(iGlyphsStart, iGlyphsEnd);
		}

		xxx_UT_DEBUGMSG(("iGlyphsStart: %d, iGlyphsEnd: %d\n", iGlyphsStart, iGlyphsEnd));
		UT_return_if_fail(iGlyphsStart <= iGlyphsEnd);

		// both of these can be 0 (iGlyphsEnd == 0 => only 1 glyph)
		xxx_UT_DEBUGMSG(("Drawing glyph subset from %d to %d (byte offsets %d, %d)\n",
					 iGlyphsStart, iGlyphsEnd,
					 iOffsetStart, iOffsetEnd));
		
		gs.num_glyphs = iGlyphsEnd - iGlyphsStart;
		gs.glyphs
			= RI.m_iVisDir == UT_BIDI_RTL ?
				RI.m_pScaledGlyphs->glyphs + iGlyphsStart + 1 :
				RI.m_pScaledGlyphs->glyphs + iGlyphsStart;
		gs.log_clusters
			= RI.m_iVisDir == UT_BIDI_RTL ?
				RI.m_pGlyphs->log_clusters + iGlyphsStart + 1 :
				RI.m_pGlyphs->log_clusters + iGlyphsStart;

		// finally we can render the substring
		cairo_save(m_cr);
		cairo_translate(m_cr, xoff, yoff);
		pango_cairo_show_glyph_string(m_cr, pf, &gs);
		cairo_restore(m_cr);
	}
}


cairo_surface_t * GR_CairoGraphics::_getCairoSurfaceFromContext(cairo_t *cr, 
                                                                     const cairo_rectangle_t & rect)
{
	cairo_surface_t * surface = cairo_surface_create_similar(cairo_get_target(cr), 
	                                       CAIRO_CONTENT_COLOR_ALPHA, 
	                                       rect.width, rect.height);

	cairo_surface_t * source = cairo_get_target(cr);
	cairo_surface_flush(source);

	cairo_t * dest = cairo_create(surface);
	cairo_set_source_surface(dest, source, rect.x, rect.y);
	cairo_paint(dest);
	cairo_destroy(dest);
	return surface;
}


void GR_CairoGraphics::_setSource(cairo_t *cr, const UT_RGBColor &clr)
{
	const GR_CairoPatternImpl * pat 
		= NULL;//dynamic_cast<const GR_CairoPatternImpl *>(clr.pattern());
	if(pat) {
		cairo_set_source(cr, pat->getPattern());
	}
	else {
		cairo_set_source_rgb(cr, clr.m_red/255., clr.m_grn/255., clr.m_blu/255.);
	}
}


static cairo_line_cap_t mapCapStyle(GR_Graphics::CapStyle in)
{
	switch (in) {
	case GR_Graphics::CAP_ROUND:
		return CAIRO_LINE_CAP_ROUND;
	case GR_Graphics::CAP_PROJECTING:
		return CAIRO_LINE_CAP_SQUARE;
	case GR_Graphics::CAP_BUTT:
	default:
		return CAIRO_LINE_CAP_BUTT;
    }
}

/*
 * \param width line width
 * \param dashes Array for dash pattern
 * \param n_dashes IN: length of dashes, OUT: number of needed dash pattern fields
 */
static void mapDashStyle(GR_Graphics::LineStyle in, double width, double *dashes, int *n_dashes)
{
	switch (in) {
	case GR_Graphics::LINE_ON_OFF_DASH:
	case GR_Graphics::LINE_DOUBLE_DASH:				// see GDK_LINE_DOUBLE_DASH, but it was never used
		UT_ASSERT(*n_dashes > 0);
		dashes[0] = 4 * width;
		*n_dashes = 1;
		break;
	case GR_Graphics::LINE_DOTTED:
		UT_ASSERT(*n_dashes > 0);
		dashes[0] = 2 * width;
		*n_dashes = 1;
		break;
	case GR_Graphics::LINE_SOLID:
	default:
		*n_dashes = 0;
	}
}

static cairo_line_join_t mapJoinStyle(GR_Graphics::JoinStyle in)
{
	switch ( in ) {
	case GR_Graphics::JOIN_ROUND:
		return CAIRO_LINE_JOIN_ROUND;
	case GR_Graphics::JOIN_BEVEL:
		return CAIRO_LINE_JOIN_BEVEL;
	case GR_Graphics::JOIN_MITER:
	default:
		return CAIRO_LINE_JOIN_MITER;
    }
}

void GR_CairoGraphics::_resetClip(void)
{
	if (m_cr == NULL)
		return;
	xxx_UT_DEBUGMSG(("Reset clip in cairo xp!!! \n"));
	cairo_reset_clip(m_cr);
}

void GR_CairoGraphics::_setProps()
{
	if (m_cr == NULL)
		return;

	if(m_curColorDirty) 
	{
		_setSource(m_cr, m_curColor);

		m_curColorDirty = false;
	}
	if(m_clipRectDirty)
	{
		_resetClip();
		if (m_pRect)
		{
			double x, y, width, height;
			x = _tdudX(m_pRect->left);
			y = _tdudY(m_pRect->top);
			width = _tduR(m_pRect->width);
			height = _tduR(m_pRect->height);
			cairo_rectangle(m_cr, x, y, width, height);
			cairo_clip(m_cr);
		}
		m_clipRectDirty = false;
	}
	if(m_linePropsDirty)
	{
		double dashes[2];
		double width;
		int n_dashes;
		width = tduD(m_lineWidth);
		if(width < 1.0)
			width = 1.0;
		cairo_set_line_width (m_cr, width);
		cairo_set_line_join (m_cr, mapJoinStyle(m_joinStyle));
		cairo_set_line_cap (m_cr, mapCapStyle(m_capStyle));

		width = cairo_get_line_width(m_cr);
		n_dashes = G_N_ELEMENTS(dashes);
		mapDashStyle(m_lineStyle, width, dashes, &n_dashes);
		cairo_set_dash(m_cr, dashes, n_dashes, 0);

		m_linePropsDirty = false;
	}
}

void GR_CairoGraphics::_scaleCharacterMetrics(GR_PangoRenderInfo & RI)
{
	UT_uint32 iZoom = getZoomPercentage();

	xxx_UT_DEBUGMSG(("_scaleCharacterMetrics... \n"));
	for(int i = 0; i < RI.m_pGlyphs->num_glyphs; ++i)
	{
		RI.m_pScaledGlyphs->glyphs[i].geometry.x_offset =
			_tduX(RI.m_pGlyphs->glyphs[i].geometry.x_offset);


		RI.m_pScaledGlyphs->glyphs[i].geometry.y_offset = _tduY(RI.m_pGlyphs->glyphs[i].geometry.y_offset);

		RI.m_pScaledGlyphs->glyphs[i].geometry.width =_tduX(RI.m_pGlyphs->glyphs[i].geometry.width );
	}
	RI.m_iZoom = iZoom;
}


void GR_CairoGraphics::_scaleJustification(GR_PangoRenderInfo & RI)
{
	RI.m_iZoom = getZoomPercentage();
	return;
}


/*!
   This function is called after shaping and before any operations are done on
   the glyphs. Although Pango does not have a separate character placement
   stage, we need to scale the glyph metrics to appropriate zoom level (since
   we shape @100% zoom).

   NB: this is probably not ideal with int arithmetic as moving from one zoom
       to another, and back we are bound to end up with incorrect metrics due
       to rounding errors.
*/
void GR_CairoGraphics::measureRenderedCharWidths(GR_RenderInfo & ri)
{
	UT_return_if_fail(ri.getType() == GRRI_CAIRO_PANGO);
	GR_PangoRenderInfo & RI = (GR_PangoRenderInfo &)ri;

	_scaleCharacterMetrics(RI);

	if(RI.m_pJustify)
	{
		_scaleJustification(RI);
	}
}

void GR_CairoGraphics::appendRenderedCharsToBuff(GR_RenderInfo & /*ri*/,
													 UT_GrowBuf & /*buf*/) const
{
	UT_return_if_fail( UT_NOT_IMPLEMENTED );
}

/*!
    returns true on success
 */
bool GR_CairoGraphics::_scriptBreak(GR_PangoRenderInfo &ri)
{
	UT_return_val_if_fail(ri.m_pText && ri.m_pGlyphs && ri.m_pItem, false);

	const GR_CairoPangoItem * pItem =
		static_cast<const GR_CairoPangoItem*>(ri.m_pItem);

	// fill the static buffer with UTF8 text
	UT_return_val_if_fail(ri.getUTF8Text(), false);

	// the buffer has to have at least one more slot than the number of glyphs
	if(!ri.s_pLogAttrs || ri.s_iStaticSize < ri.sUTF8->length() + 1)
	{
		UT_return_val_if_fail(ri.allocStaticBuffers(ri.sUTF8->length()+1),
							  false);
	}
	
	pango_break(ri.sUTF8->utf8_str(),
				ri.sUTF8->byteLength(),
				&(pItem->m_pi->analysis),
				ri.s_pLogAttrs, ri.s_iStaticSize);

	ri.s_pOwnerLogAttrs = &ri;
	return true;
}

bool GR_CairoGraphics::canBreak(GR_RenderInfo & ri, UT_sint32 &iNext,
									bool bAfter)
{
	UT_return_val_if_fail(ri.getType() == GRRI_CAIRO_PANGO &&
						  ri.m_iOffset < ri.m_iLength, false);
	
	GR_PangoRenderInfo & RI = (GR_PangoRenderInfo &)ri;
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
		// we have not found any breaks in this run -- signal this to the
		// caller
		iNext = -2;
	}
	
	return false;
}


bool GR_CairoGraphics::needsSpecialCaretPositioning(GR_RenderInfo &ri)
{
	// something smarter is needed here, so we do not go through this for
	// langugages that do not need it.
	// HACK HACK HACK
	// This simple code will return false for European Languages, 
	// otherwise it will return true
	// We should really some fancy pango function to determine if 
	// we have a complex script with combining chars
	//
	GR_PangoRenderInfo & RI = (GR_PangoRenderInfo &)ri;
	if(RI.m_pText == NULL)
		return false;

	UT_TextIterator & text = static_cast<UT_TextIterator &>(*RI.m_pText);
	UT_uint32 origPos = text.getPosition();

	for(UT_sint32 i = 0; i < RI.m_iLength && text.getStatus() == UTIter_OK;
		++i, ++text)
	{
		UT_UCS4Char c = text.getChar();
		if(c != ' ' && c<256)
		{
			// restore the iterator back to its original position
			text.setPosition(origPos);
			return false;
		}
	}

	// restore the iterator back to its original position
	text.setPosition(origPos);
	return true;
}

UT_uint32 GR_CairoGraphics::adjustCaretPosition(GR_RenderInfo & ri,
													bool bForward)
{
	UT_return_val_if_fail(ri.getType() == GRRI_CAIRO_PANGO, 0);
	GR_PangoRenderInfo & RI = (GR_PangoRenderInfo &)ri;
	
	if(!RI.s_pLogAttrs || RI.s_pOwnerLogAttrs != &ri)
		_scriptBreak(RI);

	UT_return_val_if_fail( RI.s_pLogAttrs, RI.m_iOffset );
	
	UT_sint32 iOffset = ri.m_iOffset;

	if(bForward)
		while(!RI.s_pLogAttrs[iOffset].is_cursor_position &&
			  iOffset < RI.m_iLength)
			iOffset++;
	else
		while(!RI.s_pLogAttrs[iOffset].is_cursor_position && iOffset > 0)
			iOffset--;
	
	return iOffset;
}

void GR_CairoGraphics::adjustDeletePosition(GR_RenderInfo & ri)
{
	UT_return_if_fail(ri.getType() == GRRI_CAIRO_PANGO);
	GR_PangoRenderInfo & RI = (GR_PangoRenderInfo &)ri;

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
	while(iOffset > 0 && iOffset > ri.m_iOffset &&
		  !RI.s_pLogAttrs[iOffset].is_cursor_position)
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

/*!
 * I believe this code clears all the justification points. MES June 2008
 * It returns the total space assigned to justify the text in layout units.
 */
UT_sint32 GR_CairoGraphics::resetJustification(GR_RenderInfo & ri,
												   bool bPermanent)
{
	UT_return_val_if_fail(ri.getType() == GRRI_CAIRO_PANGO, 0);
	GR_PangoRenderInfo & RI = (GR_PangoRenderInfo &)ri;

	if(!RI.m_pJustify)
		return 0;

	
	UT_sint32 iWidth2 = 0;
	for(UT_sint32 i = 0; i < RI.m_pGlyphs->num_glyphs; ++i)
	{
		iWidth2 += RI.m_pJustify[i];

		// TODO here we need to substract the amount from pango metrics
		RI.m_pGlyphs->glyphs[i].geometry.width -= RI.m_pJustify[i];
	}
	//
	// This sets the glyphs that will be displayed on screen.
	//
	_scaleCharacterMetrics(RI);

	if(bPermanent)
	{
		delete [] RI.m_pJustify;
		RI.m_pJustify = NULL;
	}
	else
	{
		memset(RI.m_pJustify, 0, RI.m_pGlyphs->num_glyphs * sizeof(int));
	}
	
	// Justification in pango units. Convert to layout units.

	return -ptlunz(iWidth2);
}


UT_sint32 GR_CairoGraphics::countJustificationPoints(const GR_RenderInfo & ri) const
{
	UT_return_val_if_fail(ri.getType() == GRRI_CAIRO_PANGO, 0);
	const GR_PangoRenderInfo & RI = static_cast<const GR_PangoRenderInfo &>(ri);

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

It requires as input RI.m_iJustificationAmount and RI.m_iJustificationPoints.
These are determined in fp_TextRun using calculations in layout units

 */
void GR_CairoGraphics::justify(GR_RenderInfo & ri)
{
	UT_return_if_fail(ri.getType() == GRRI_CAIRO_PANGO);
	GR_PangoRenderInfo & RI = (GR_PangoRenderInfo &) ri;
	if(!RI.m_iJustificationPoints || !RI.m_iJustificationAmount ||
	   !RI.m_pGlyphs)
		return;

	// make sure that we are not adding apples to oranges
	// We don't need this now.
	//	if(RI.m_iZoom != getZoomPercentage())
	//	_scaleCharacterMetrics(RI);
	
	if(!RI.m_pJustify)
		RI.m_pJustify = new int[RI.m_pGlyphs->num_glyphs];
	
	UT_return_if_fail(RI.m_pJustify);
	memset(RI.m_pJustify, 0, RI.m_pGlyphs->num_glyphs * sizeof(int));
	
	UT_uint32 iExtraSpace = RI.m_iJustificationAmount;
	UT_uint32 iPoints     = RI.m_iJustificationPoints;

	xxx_UT_DEBUGMSG(("::Justify Extra justification space %d \n",iExtraSpace));
	xxx_UT_DEBUGMSG(("::Justify Number of justification points %d \n",iPoints));

	UT_return_if_fail(RI.m_pText);
	
	UT_TextIterator & text = *RI.m_pText;
	UT_sint32 iGlyphCount  = RI.m_pGlyphs->num_glyphs;
	UT_BidiCharType iDir   =
		RI.m_iVisDir % 2 ? UT_BIDI_RTL : UT_BIDI_LTR;

	// Split into two big loops to avoid all the LTR/RTL logic inside -- if
	// you make changes to one branch, please make sure to do the same to the
	// other.
	
	UT_sint32 i; // glyph index
	UT_sint32 j; // text index

	UT_uint32 iSpace = iExtraSpace/iPoints;
	
	if (iDir == UT_BIDI_LTR)
	{
		i = 0;
		j = 0;
		
		while (text.getStatus() == UTIter_OK &&
			   i < iGlyphCount &&
			   j < RI.m_iLength)
		{
			UT_UCS4Char c = text.getChar();
		
			if(c == UCS_SPACE)
			{
			

				// iSpace is in layout units. Convert to pango units

				RI.m_pJustify[i] = ltpunz(iSpace);

				iPoints--;

				// add this amount the pango units
				xxx_UT_DEBUGMSG(("Justify-1 Prev geom width %d additional %d \n",RI.m_pGlyphs->glyphs[i].geometry.width,RI.m_pJustify[i]));

				RI.m_pGlyphs->glyphs[i].geometry.width += RI.m_pJustify[i];
			
				if(!iPoints)
					break;
			}


			// skip over any glyphs that belong to the current character
			// LTR run, the glyphs and the text are in the same order,
			// and logical offsets are increasing
			UT_sint32 iOffset = RI.m_pLogOffsets[i++];

			while (i < iGlyphCount && (RI.m_pLogOffsets[i] == iOffset)) {
				++i;
			}

			if (i >= iGlyphCount)
				break;
		
			// if the glyph cluster represents more characters than its
			// length, we have to advance the iterator accordingly
			UT_sint32 iDiff = RI.m_pLogOffsets[i] - iOffset;
		
			text += iDiff;
			j += iDiff;
		}
	}
	else
	{
		i = iGlyphCount - 1;
		j = 0;
		
		while (text.getStatus() == UTIter_OK &&
			   i >= 0 &&
			   j < RI.m_iLength)
		{
			UT_UCS4Char c = text.getChar();
		
			if(c == UCS_SPACE)
			{
			
				iPoints--;

				// iSpace is in layout units. Convert to pango units

				RI.m_pJustify[i] = ltpunz(iSpace);

				// add this amount the pango metrics

				xxx_UT_DEBUGMSG(("Justify-2 Prev geom width %d additional %d \n",RI.m_pGlyphs->glyphs[i].geometry.width,RI.m_pJustify[i]));
				RI.m_pGlyphs->glyphs[i].geometry.width += RI.m_pJustify[i];
			
				if(!iPoints)
					break;
			}


			// skip over any glyphs that belong to the current character
			// RTL run, so the glyphs are in reversed order of the text,
			// and logical offsets are decreesing
			UT_sint32 iOffset = RI.m_pLogOffsets[i--];
		
			while (RI.m_pLogOffsets[i] == iOffset && i >= 0)
				--i;

			if (i < 0)
				break;
		
			// if the glyph cluster represents more characters than its
			// length, we have to advance the iterator accordingly
			UT_sint32 iDiff = iOffset - RI.m_pLogOffsets[i];
		
			text += iDiff;
			j += iDiff;
		}
	}
	
	//
 	// Now scale the metrics for the drawing glyphs
 	//
 	_scaleCharacterMetrics(RI);
}

/*!
 * This function takes (x,y) in layout units and determines the location in the
 * pango string.
 */
UT_uint32 GR_CairoGraphics::XYToPosition(const GR_RenderInfo & ri, UT_sint32 x, 
											 UT_sint32 /*y*/) const
{
	UT_return_val_if_fail(ri.getType() == GRRI_CAIRO_PANGO, 0);
	const GR_PangoRenderInfo & RI = static_cast<const GR_PangoRenderInfo &>(ri);
	const GR_CairoPangoItem * pItem =
		static_cast<const GR_CairoPangoItem *>(RI.m_pItem);
	UT_return_val_if_fail(pItem, 0);

	// TODO: this is very inefficient: to cache or not to cache ?
	UT_UTF8String utf8;
	
	UT_sint32 i;
	for(i = 0; i < RI.m_iLength; ++i, ++(*(RI.m_pText)))
	{
		UT_return_val_if_fail(RI.m_pText->getStatus() == UTIter_OK, 0);
		if(isSymbol())
		{
			utf8 += adobeToUnicode(RI.m_pText->getChar());
		}
		else if(isDingbat())
		{
			utf8 += adobeDingbatsToUnicode(RI.m_pText->getChar());
		}
		utf8 += RI.m_pText->getChar();
	}
	
	// Since the glyphs are measured in pango units
	// we need to convert from layout units

	int x_pos = ltpunz(x);
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

/*!
 * Return a location in layout units (x,y) of a pango glyph.
 */
void GR_CairoGraphics::positionToXY(const GR_RenderInfo & ri,
										UT_sint32& x, UT_sint32& /*y*/,
										UT_sint32& x2, UT_sint32& /*y2*/,
										UT_sint32& /*height*/, bool& /*bDirection*/) const
{
	UT_return_if_fail(ri.getType() == GRRI_CAIRO_PANGO);
	GR_PangoRenderInfo & RI = (GR_PangoRenderInfo &) ri;
	GR_CairoPangoItem * pItem = (GR_CairoPangoItem *)RI.m_pItem;
  
	if(!pItem)
		return;

	// TODO: this is very inefficient: to cache or not to cache ?
	UT_UTF8String utf8;

	UT_sint32 i;
	for(i = 0; i < RI.m_iLength; ++i, ++(*(RI.m_pText)))
	{
		UT_return_if_fail(RI.m_pText->getStatus() == UTIter_OK);
		if(isSymbol())
		{
			utf8 += adobeToUnicode(RI.m_pText->getChar());
		}
		else if(isDingbat())
		{
			utf8 += adobeDingbatsToUnicode(RI.m_pText->getChar());
		}
		utf8 += RI.m_pText->getChar();
	}

	UT_sint32 iByteOffset = 0;
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
	else if(i >= 1)
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

	//
	// Since the glyphs are measured in pango units we need to convert to layout
	//
	x = ptlunz(x);
	x2 = x;
}

void GR_CairoGraphics::drawChars(const UT_UCSChar* pChars,
									int iCharOffset, int iLength,
									UT_sint32 xoff, UT_sint32 yoff,
									 int * pCharWidth)
{
	if (m_cr == NULL)
		return;
	_setProps();
	UT_UTF8String utf8;
	xxx_UT_DEBUGMSG(("isDingBat %d \n",isDingbat()));
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

	double xoffD = _tdudX(xoff);
	double yoffD = _tdudY(yoff+getFontAscent());

	PangoFont * pf = m_pPFont->getPangoFont();
	PangoRectangle LR;
	PangoFontset *pfs = NULL;
	bool bDoFontSubstitution = false;
	bool bClear_pf = false;
	
	for(int i = 0; i < iItemCount; ++i)
	{
		PangoItem *pItem = (PangoItem *)g_list_nth(pItems, i)->data;

		if(!pItem)
		{
			UT_ASSERT(pItem);
			if(pGstring)
				pango_glyph_string_free(pGstring);
			_pango_item_list_free(pItems);
			return;
		}

		if (bDoFontSubstitution)
		{
			if (bClear_pf)
			{
				g_object_unref(pf);
			}
			UT_ASSERT(pfs);
			UT_sint32 fontSize = pango_font_description_get_size(pango_font_describe(m_pPFont->getPangoFont()));
			pf = pango_fontset_get_font (pfs, g_utf8_get_char (utf8.utf8_str()+pItem->offset));
			PangoFontDescription * pfd = pango_font_describe (pf);
			pango_font_description_set_size (pfd, fontSize*m_iDeviceResolution/getResolution());
			UT_ASSERT(pfd);
			pf = pango_context_load_font(getLayoutContext(), pfd);
			pango_font_description_free(pfd);
			bClear_pf = true;
		}
		g_object_unref(pItem->analysis.font);
		pItem->analysis.font = (PangoFont*)g_object_ref((GObject*)pf);

		pango_shape(utf8.utf8_str()+ pItem->offset,
					pItem->length,
					&(pItem->analysis),
					pGstring);

		if (!bDoFontSubstitution)
		{
			// The following code only checks the first character of utf8
			// to see if a font substitution is needed
			// TODO: modify code so that it can handle a string where multiple
			//       fonts are needed.
			if (pGstring->glyphs[0].glyph & PANGO_GLYPH_UNKNOWN_FLAG)
			{
				bDoFontSubstitution = true;
				pfs = pango_font_map_load_fontset (getFontMap(),getContext(),
												   m_pPFont->getPangoDescription(),
												   pItem->analysis.language);
				i--;
				continue;
			}
		}

		if(pCharWidth)
		{
			for(int j=0; j<pGstring->num_glyphs; j++)
			{

				pGstring->glyphs[j].geometry.width = _tduX(pCharWidth[j]*PANGO_SCALE);
			}
		}

		cairo_save(m_cr);
		cairo_translate(m_cr, xoffD, yoffD);
		pango_cairo_show_glyph_string(m_cr, pf, pGstring);
		cairo_restore(m_cr);

		// now advance xoff
		pango_glyph_string_extents(pGstring, pf, NULL, &LR);
		xoffD += PANGO_PIXELS(LR.width);
	}

	if(pGstring)
		pango_glyph_string_free(pGstring);
	_pango_item_list_free(pItems);
	if(pfs)
	{
		g_object_unref((GObject*)pfs);
		pfs = NULL;
	}
	if (bClear_pf)
	{
		g_object_unref(pf);
	}
}

UT_uint32 GR_CairoGraphics::measureString(const UT_UCSChar * pChars,
											  int iCharOffset,
											  int iLength,
											  UT_GrowBufElement* pWidths,
											  UT_uint32 * height)
{
	UT_UTF8String utf8;
	UT_uint32 iWidth = 0;

	if (!iLength || iLength <= iCharOffset)
		return 0;
	
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
	GList * pItems = pango_itemize(getLayoutContext(),
								   utf8.utf8_str(),
								   0, utf8.byteLength(),
								   NULL, NULL);
	
	PangoGlyphString * pGstring = pango_glyph_string_new();

	PangoFont * pf = m_pPFont->getPangoLayoutFont();
	PangoRectangle LR;
	UT_uint32 iOffset = 0;
	GList * l = pItems;

	if (height)
		*height = 0;
	
	PangoFontset *pfs = NULL;
	bool bDoFontSubstitution = false;
	bool bClear_pf = false;

	while (l)
	{
		PangoItem *pItem = (PangoItem*)l->data;

		if(!pItem)
		{
			UT_ASSERT(pItem);
			iWidth = 0;
			goto cleanup;
		}

		if (bDoFontSubstitution)
		{
			if (bClear_pf)
			{
				g_object_unref(pf);
			}
			UT_ASSERT(pfs);
			UT_sint32 fontSize = pango_font_description_get_size(pango_font_describe(m_pPFont->getPangoFont()));
			pf = pango_fontset_get_font (pfs, g_utf8_get_char (utf8.utf8_str()+pItem->offset));
			PangoFontDescription * pfd = pango_font_describe (pf);
			pango_font_description_set_size (pfd, fontSize);
			UT_ASSERT(pfd);
			pf = pango_context_load_font(getLayoutContext(), pfd);
			pango_font_description_free(pfd);
			bClear_pf = true;
		}

		// the PangoItem has to take ownership of that.
		g_object_unref(pItem->analysis.font);
		pItem->analysis.font = (PangoFont*)g_object_ref((GObject*)pf);

		pango_shape(utf8.utf8_str()+ pItem->offset,
					pItem->length,
					&(pItem->analysis),
					pGstring);

		if (!bDoFontSubstitution)
		{
			// The following code only checks the first character of utf8
			// to see if a font substitution is needed
			// TODO: modify code so that it can handle a string where multiple
			//       fonts are needed.
			if (pGstring->glyphs[0].glyph & PANGO_GLYPH_UNKNOWN_FLAG)
			{
				UT_DEBUGMSG(("How MANY TIMES?\n"));
				bDoFontSubstitution = true;
				pfs = pango_font_map_load_fontset (getFontMap(),getContext(),
												   m_pPFont->getPangoDescription(),
												   pItem->analysis.language);
				continue;
			}
		}

		pango_glyph_string_extents(pGstring, pf, NULL, &LR);
		iWidth += (UT_uint32)(((double) LR.width + (double)LR.x)/PANGO_SCALE);
		UT_uint32 h = LR.height/PANGO_SCALE;
		xxx_UT_DEBUGMSG(("measure string iWidth %d height %d \n",iWidth,h));
		if (height && *height < h)



			*height = h;

		int * pLogOffsets = NULL;

		/* this is rather involved, fortunately the width array is not
		 * needed most of the time we use this function in abi
		 */
		if (pWidths)
		{
			int charLength =
				UT_MIN(g_utf8_strlen(utf8.utf8_str() + pItem->offset, -1),
					   pItem->num_chars);

			xxx_UT_DEBUGMSG(("*** strlen %d, num-chars %d ***\n",
						 g_utf8_strlen(utf8.utf8_str() + pItem->offset, -1),
						 pItem->num_chars));
			
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
						pWidths[k] = iMyWidth / (iEnd - (j + 1) + 1);
					}
					iOffset += iEnd - (j + 1) + 1;
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

		l = l->next;
	}

	if (pWidths)
	{
		/* This is a bit weird, possibly a Pango bug, but it is better
		 * to set any dangling widths to 0 than leave them at randomn values
		 */
		while (iOffset < (UT_uint32)iLength)
		{
			pWidths[iOffset++] = 0;
		}
	}
	
	xxx_UT_DEBUGMSG(("Length %d, Offset %d\n", iLength, iOffset));
	
cleanup:
	if(pGstring)
		pango_glyph_string_free(pGstring);

	_pango_item_list_free(pItems);
	if(pfs)
	{
		g_object_unref((GObject*)pfs);
		pfs = NULL;
	}
	if (bClear_pf)
	{
		g_object_unref(pf);
	}


	return iWidth;
}




/*!
 * Draw the specified image at the location specified in local units 
 * (xDest,yDest). xDest and yDest are in logical units.
 */
void GR_CairoGraphics::drawImage(GR_Image* pImg,
									 UT_sint32 xDest, UT_sint32 yDest)
{
	if (m_cr == NULL)
		return;
	_setProps();
	UT_ASSERT(pImg);

	if (!pImg) {
		return;
	}

	double idx = _tdudX(xDest);
	double idy = _tdudY(yDest);

	cairo_save(m_cr);
	_resetClip();


	if(!getAntiAliasAlways() && queryProperties(GR_Graphics::DGP_PAPER ))
		cairo_set_antialias(m_cr,CAIRO_ANTIALIAS_NONE);
	cairo_translate(m_cr, idx, idy);

	if (pImg->getType() == GR_Image::GRT_Raster) {
		static_cast<GR_CairoRasterImage*>(pImg)->cairoSetSource(m_cr);

		cairo_pattern_t *pattern = cairo_get_source(m_cr);
		cairo_pattern_set_extend(pattern, CAIRO_EXTEND_NONE);
		cairo_paint(m_cr);
	} else if (pImg->getType() == GR_Image::GRT_Vector) {
		/* for some obscure reason, using cairoSetSource() with an svg image
			sometimes fails when printing, see 13533 */
		static_cast<GR_CairoVectorImage*>(pImg)->renderToCairo(m_cr);
	}

	cairo_restore(m_cr);
}

void GR_CairoGraphics::setFont(const GR_Font * pFont)
{
	UT_return_if_fail( pFont && pFont->getType() == GR_FONT_UNIX_PANGO);

	//PangoFont * pf = (PangoFont*) pFont;
	m_pPFont = const_cast<GR_PangoFont*>(static_cast<const GR_PangoFont*>(pFont));

	_setIsSymbol(false);
	_setIsDingbat(false);

	const char* familyName = m_pPFont->getFamily();

	char * szLCFontName = familyName ? g_utf8_strdown (familyName, -1) : NULL;

	if (szLCFontName)
	{
		xxx_UT_DEBUGMSG(("GR_CairoGraphics::setFont: %s\n", szLCFontName));
		if(strstr(szLCFontName,"symbol") != NULL)
		{
			/*
			 * I am not too happy about this, and do not want to see the exception
			 * list to grow much more, but cannot think of another simple solution.
			 */
			if(!strstr(szLCFontName,"starsymbol") &&
			   !strstr(szLCFontName,"opensymbol") &&
			   !strstr(szLCFontName,"symbolnerve"))
				_setIsSymbol(true);
		}
		
		if(strstr(szLCFontName,"dingbat"))
			_setIsDingbat(true);
		FREEP(szLCFontName);
	}
		
	if(!m_pPFont->isGuiFont() && m_pPFont->getZoom() != getZoomPercentage())
	{
		m_pPFont->reloadFont(this);
	}
}

void GR_CairoGraphics::setZoomPercentage(UT_uint32 iZoom)
{
	// not sure if we should not call GR_UnixGraphics::setZoomPercentage() here
	// instead
	GR_Graphics::setZoomPercentage (iZoom); // chain up

	if(m_pPFont && !m_pPFont->isGuiFont() && m_pPFont->getZoom() != iZoom)
	{
		m_pPFont->reloadFont(this);
	}
}

GR_Font* GR_CairoGraphics::getDefaultFont(UT_String& /*fontFamily*/, 
											  const char * /*pLang*/)
{
	UT_return_val_if_fail( UT_NOT_IMPLEMENTED, NULL );
}

UT_uint32 GR_CairoGraphics::getFontAscent()
{
	return getFontAscent(m_pPFont);
}

UT_uint32 GR_CairoGraphics::getFontDescent()
{
	return getFontDescent(m_pPFont);
}

UT_uint32 GR_CairoGraphics::getFontHeight()
{
	return getFontHeight(m_pPFont);
}

UT_uint32 GR_CairoGraphics::getFontAscent(const GR_Font * pFont)
{
	UT_return_val_if_fail( pFont, 0 );

	const GR_PangoFont * pFP = static_cast<const GR_PangoFont*>(pFont);
	return pFP->getAscent();
}

UT_uint32 GR_CairoGraphics::getFontDescent(const GR_Font *pFont)
{
	UT_return_val_if_fail( pFont, 0 );

	const GR_PangoFont * pFP = static_cast<const GR_PangoFont*>(pFont);
	return pFP->getDescent();
}

UT_uint32 GR_CairoGraphics::getFontHeight(const GR_Font *pFont)
{
	UT_return_val_if_fail( pFont, 0 );

	const GR_PangoFont * pFP = static_cast<const GR_PangoFont*>(pFont);
	return pFP->getAscent() + pFP->getDescent();
}

typedef struct
{
  int value;
  const char str[16];
} FieldMap;

static const FieldMap style_map[] = {
  { PANGO_STYLE_NORMAL, "" },
  { PANGO_STYLE_OBLIQUE, "Oblique" },
  { PANGO_STYLE_ITALIC, "Italic" }
};

static const FieldMap variant_map[] = {
  { PANGO_VARIANT_NORMAL, "" },
  { PANGO_VARIANT_SMALL_CAPS, "Small-Caps" }
};

static const FieldMap weight_map[] = {
  { PANGO_WEIGHT_ULTRALIGHT, "Ultra-Light" },
  { PANGO_WEIGHT_ULTRALIGHT, "Ultralight" },
  { PANGO_WEIGHT_LIGHT, "Light" },
  { PANGO_WEIGHT_NORMAL, "" },
  { 500, "Medium" },
  { PANGO_WEIGHT_SEMIBOLD, "Semi-Bold" },
  { PANGO_WEIGHT_SEMIBOLD, "Semibold" },
  { PANGO_WEIGHT_BOLD, "Bold" },
  { PANGO_WEIGHT_ULTRABOLD, "Ultra-Bold" },
  { PANGO_WEIGHT_HEAVY, "Heavy" }
};

static const FieldMap stretch_map[] = {
  { PANGO_STRETCH_ULTRA_CONDENSED, "Ultra-Condensed" },
  { PANGO_STRETCH_EXTRA_CONDENSED, "Extra-Condensed" },
  { PANGO_STRETCH_CONDENSED,       "Condensed" },
  { PANGO_STRETCH_SEMI_CONDENSED,  "Semi-Condensed" },
  { PANGO_STRETCH_NORMAL,          "" },
  { PANGO_STRETCH_SEMI_EXPANDED,   "Semi-Expanded" },
  { PANGO_STRETCH_EXPANDED,        "Expanded" },
  { PANGO_STRETCH_EXTRA_EXPANDED,  "Extra-Expanded" },
  { PANGO_STRETCH_ULTRA_EXPANDED,  "Ultra-Expanded" }
};

static const FieldMap *find_field(const FieldMap *fma, size_t n, const char *elem)
{
	for (size_t i = 0; i < n; i++)
		{
			if (0 == g_ascii_strcasecmp(fma[i].str, elem))
				{
					return &(fma[i]);
				}
		}

	return NULL;
}

/* Static 'virtual' function declared in gr_Graphics.h */
const char* GR_Graphics::findNearestFont(const char* pszFontFamily,
										 const char* pszFontStyle,
										 const char* pszFontVariant,
										 const char* pszFontWeight,
										 const char* pszFontStretch,
										 const char* pszFontSize,
										 const char* /*pszFontLang*/)
{
	static UT_UTF8String s = pszFontFamily;

	PangoFontDescription *d = pango_font_description_new();
	
	if (d)
	{
		const FieldMap *fm;

		pango_font_description_set_family(d, pszFontFamily);
		pango_font_description_set_size(d, (int)((double)PANGO_SCALE * UT_convertToPoints(pszFontSize)));

		if ((fm = find_field(style_map, G_N_ELEMENTS(style_map), pszFontStyle)) != 0)
			{
				pango_font_description_set_style(d, (PangoStyle)fm->value);				
			}

		if ((fm = find_field(variant_map, G_N_ELEMENTS(variant_map), pszFontVariant)) != 0)
			{
				pango_font_description_set_variant(d, (PangoVariant)fm->value);				
			}

		if ((fm = find_field(weight_map, G_N_ELEMENTS(weight_map), pszFontWeight)) != 0)
			{
				pango_font_description_set_weight(d, (PangoWeight)fm->value);				
			}

		if ((fm = find_field(stretch_map, G_N_ELEMENTS(stretch_map), pszFontStretch)) != 0)
			{
				pango_font_description_set_stretch(d, (PangoStretch)fm->value);				
			}

		PangoFontMap *fontmap = pango_cairo_font_map_get_default();
		PangoContext *context = pango_font_map_create_context(fontmap);
		if (fontmap && context)
			{
				PangoFont *font = pango_font_map_load_font(fontmap, context, d);
				if (font)
					{
						PangoFontDescription *new_desc = pango_font_describe(font);
						s = pango_font_description_get_family(new_desc);
						pango_font_description_free(new_desc);
						g_object_unref(font);
					}
			}
		if (context) {
			g_object_unref(G_OBJECT (context));
			context = NULL;
		}

		pango_font_description_free(d);
	}

	xxx_UT_DEBUGMSG(("@@@@ ===== Requested [%s], found [%s]\n",
				 pszFontFamily, s.utf8_str()));

	return s.utf8_str();
}


GR_Font* GR_CairoGraphics::_findFont(const char* pszFontFamily,
										 const char* pszFontStyle,
										 const char* pszFontVariant,
										 const char* pszFontWeight,
										 const char* pszFontStretch,
										 const char* pszFontSize,
										 const char* pszLang)
{
	double dPointSize = UT_convertToPoints(pszFontSize);
	std::string s;

	// Pango is picky about the string we pass to it -- it cannot handle any
	// 'normal' values, and it will stop parsing when it encounters one.
	const char * pStyle = pszFontStyle;
	const char * pVariant = pszFontVariant;
	const char * pWeight = pszFontWeight;
	const char * pStretch = pszFontStretch;

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
	
	s = UT_std_string_sprintf("%s, %s %s %s %s",
					  pszFontFamily,
					  pStyle,
					  pVariant,
					  pWeight,
					  pStretch);
	
	return new GR_PangoFont(s.c_str(), dPointSize, this, pszLang);
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

void GR_CairoGraphics::getCoverage(UT_NumberVector& coverage)
{
	coverage.clear();

	UT_return_if_fail(m_pPFont);

	PangoCoverage * pc = m_pPFont->getPangoCoverage();
	
	if(!pc)
		return;

	MyPangoCoverage * mpc = (MyPangoCoverage*) pc;
	UT_uint32 iMaxChar = mpc->n_blocks * 256;

	xxx_UT_DEBUGMSG(("GR_CairoGraphics::getCoverage: iMaxChar %d\n", iMaxChar));
	
	bool bInRange = false;
	UT_uint32 iRangeStart = 0;
	
	// Skip the coverage for character 0 as pango doesn't seem to be able to
	// handle it properly.
	// Note that for almost all fonts pango reports that it has no coverage for
	// character 0, so this is a non-issue there. However, for some (broken?) fonts 
	// like 'Fixedsys Excelsior 2.00' pango reports it *has* coverage for character 0. 
	// This will lead to crashes when attempting to shape and/or draw it, like 
	// the crash in bug 11731 - MARCM
	for(UT_uint32 i = 1; i < iMaxChar; ++i)
	{
		PangoCoverageLevel pl = pango_coverage_get(pc, i);
		
		if(PANGO_COVERAGE_NONE == pl || PANGO_COVERAGE_FALLBACK == pl)
		{
			if(bInRange)
			{
				// according to the code in XAP_UnixFont::getCoverage(), the
				// range is of type <x,y)
				coverage.push_back(i - iRangeStart);
				bInRange = false;
			}
		}
		else
		{
			if(!bInRange)
			{
				coverage.push_back(i);
				iRangeStart = i;
				bInRange = true;
			}
		}
	}
}

const std::vector<std::string> & GR_CairoGraphics::getAllFontNames(void)
{
	XAP_Prefs * pPrefs = XAP_App::getApp()->getPrefs();
	bool bExclude = false;
	bool bInclude = false;

	/*
	 * Do this only once
	 */
	static std::vector<std::string> Vec;

	if (!Vec.empty())
		return Vec;
	
	if (pPrefs)
	{
		XAP_FontSettings & Fonts = pPrefs->getFontSettings();
		bExclude = Fonts.haveFontsToExclude();
		bInclude = Fonts.haveFontsToInclude();
		
		if (bInclude)
		{
			for (UT_uint32 k = 0; k < Fonts.getFonts().size(); ++k)
				Vec.push_back (Fonts.getFonts()[k].utf8_str());

			return Vec;
		}
	}


	UT_DEBUGMSG(("@@@@ ===== Loading system fonts =====\n"));
	PangoFontMap *fontmap = pango_cairo_font_map_get_default();
	PangoContext *context = pango_font_map_create_context(PANGO_FONT_MAP(fontmap));
	if (fontmap && context)
	{
		PangoFontFamily **font_families;
		int n_families;
		pango_font_map_list_families(fontmap, &font_families, &n_families);

		UT_DEBUGMSG(("@@@@ ===== Loading system fonts n_families: %d =====\n", n_families));
		for(UT_sint32 i = 0; i < n_families; ++i)
		{
			const char *family = pango_font_family_get_name(font_families[i]);

			if (bExclude)
			{
				XAP_FontSettings & Fonts = pPrefs->getFontSettings();
				if (Fonts.isOnExcludeList(family))
				{
					UT_DEBUGMSG(("@@@@ ===== Excluding font [%s] =====\n",
								 family));
					continue;
				}
			}

			PangoFontFace ** faces;
			int n_faces;
			bool is_scalable = true;
			pango_font_family_list_faces(font_families[i], &faces, &n_faces);
			for(int j = 0; j < n_faces; j++) 
			{
				int * sizes = NULL;
				int n_sizes = 0;
				pango_font_face_list_sizes(faces[j], &sizes, &n_sizes);
				if(sizes) 
				{
					g_free(sizes);
					is_scalable = false;
					UT_DEBUGMSG(("@@@@ ===== Excluding NON scalable font [%s] =====\n",
								 family));
					break;
				}
			}
			g_free(faces);
			if(is_scalable) 
			{
				Vec.push_back(family);
			}
		}
		g_free(font_families);
	}
	if(context)
	{
		g_object_unref (G_OBJECT (context));
		context = NULL;
	}
    std::sort(Vec.begin(), Vec.end());

	return Vec;
}

UT_uint32 GR_CairoGraphics::getAllFontCount()
{
	return getAllFontNames().size();
}

GR_Font * GR_CairoGraphics::getDefaultFont(GR_Font::FontFamilyEnum f,
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

void GR_CairoGraphics::getColor(UT_RGBColor& clr)
{
	clr = m_curColor;
}

void GR_CairoGraphics::setColor(const UT_RGBColor& clr)
{
	m_curColor = clr;
	m_curColorDirty = true;
}

void GR_CairoGraphics::drawLine(UT_sint32 x1, UT_sint32 y1,
							   UT_sint32 x2, UT_sint32 y2)
{
	if (m_cr == NULL)
		return;
	_setProps();

	UT_sint32 idx1 = _tduX(x1);
	UT_sint32 idx2 = _tduX(x2);

	UT_sint32 idy1 = _tduY(y1);
	UT_sint32 idy2 = _tduY(y2);

	cairo_save(m_cr);
	if(!getAntiAliasAlways())
		cairo_set_antialias(m_cr,CAIRO_ANTIALIAS_NONE);
	cairo_move_to (m_cr,idx1, idy1);
	cairo_line_to (m_cr,idx2, idy2);
	cairo_stroke (m_cr);
	cairo_restore(m_cr);
}

void GR_CairoGraphics::setLineWidth(UT_sint32 iLineWidth)
{
	m_lineWidth = iLineWidth;
	m_linePropsDirty = true;
}


void GR_CairoGraphics::setLineProperties ( double inWidth, 
										  GR_Graphics::JoinStyle inJoinStyle,
										  GR_Graphics::CapStyle inCapStyle,
										  GR_Graphics::LineStyle inLineStyle )
{
	m_lineWidth = inWidth;
	m_joinStyle = inJoinStyle;
	m_capStyle = inCapStyle;
	m_lineStyle = inLineStyle;
	m_linePropsDirty = true;
}

/*
 * This method looks the way it does because the Cairo XOR does not behave like
 * the bitwise xor used in the other graphics classes. We use this method
 * to draw temperary lines  across the page when dragging ruler controls.
 * The hack below preserves this behaviour.
 */
void GR_CairoGraphics::xorLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2,
							  UT_sint32 y2)
{
	if (m_cr == NULL)
		return;
	_setProps();

	UT_sint32 idx1 = _tduX(x1);
	UT_sint32 idx2 = _tduX(x2);

	UT_sint32 idy1 = _tduY(y1);
	UT_sint32 idy2 = _tduY(y2);
	if((idx1 == m_iPrevX1) && (idx2 == m_iPrevX2) && (idy1 == m_iPrevY1) && (idy2 == m_iPrevY2) && (m_iXORCount == 1))
	{
		//
		// We've XOR'd a previously written line, restore the content from 
		// m_iPrevRect
		//
		m_iXORCount = 0;
		restoreRectangle(m_iPrevRect);
		
	}
	else
	{
		//
		// Save the contents of the screen before drawing a line across it.
		//
		m_iPrevX1 = idx1;
		m_iPrevX2 = idx2;
		m_iPrevY1 = idy1;
		m_iPrevY2 = idy2;
		m_iXORCount = 1;
		UT_Rect r;
		UT_sint32 swap = 0;
		if(idx1 > idx2)
		{
			swap = idx1;
			idx1 = idx2;
			idx2 = swap;
		}
		if(idy1 > idy2)
		{
			swap = idy1;
			idy1 = idy2;
			idy2 = swap;
		}
		r.left = tlu(idx1);
		r.top = tlu(idy1);
		r.width = tlu(idx2 - idx1 +2);
		r.height = tlu(idy2 - idy1+2);
		saveRectangle(r,m_iPrevRect);

		cairo_save(m_cr);
		if(!getAntiAliasAlways())
			cairo_set_antialias(m_cr,CAIRO_ANTIALIAS_NONE);
		cairo_set_source_rgb(m_cr, 0.0 , 0.0 , 0.0);

		cairo_move_to(m_cr, idx1, idy1);
		cairo_line_to(m_cr, idx2, idy2);
		cairo_stroke(m_cr);
		cairo_restore(m_cr);
	}
}

void GR_CairoGraphics::polyLine(UT_Point * pts, UT_uint32 nPoints)
{
	if (m_cr == NULL)
		return;
	_setProps();

	UT_uint32 i;

	UT_return_if_fail(nPoints > 1);

	cairo_save(m_cr);
	if(!getAntiAliasAlways())
		cairo_set_antialias(m_cr,CAIRO_ANTIALIAS_NONE);

	i = 0;
	cairo_move_to(m_cr, _tdudX(pts[i].x), _tdudY(pts[i].y));
	i++;
	for (; i < nPoints; i++)
	{
		cairo_line_to(m_cr, _tdudX(pts[i].x), _tdudY(pts[i].y));
	}
	cairo_stroke(m_cr);
	cairo_restore(m_cr);
}

void GR_CairoGraphics::invertRect(const UT_Rect* /* pRect */)
{
/* TODO Rob
	UT_ASSERT(pRect);

	UT_sint32 idy = _tduY(pRect->top);
	UT_sint32 idx = _tduX(pRect->left);
	UT_sint32 idw = _tduR(pRect->width);
	UT_sint32 idh = _tduR(pRect->height);
*/
	UT_ASSERT_NOT_REACHED ();
}
/**
 * This appears to fix off-by-1 bugs in setting rectangles and drawing text
 */
double GR_CairoGraphics::_tdudX(UT_sint32 layoutUnits) const
{
	return _tduX(layoutUnits) -0.5;
}

/**
 * This appears to fix off-by-1 bugs in setting rectangles and drawing text
 */
double GR_CairoGraphics::_tdudY(UT_sint32 layoutUnits) const
{
	return _tduY(layoutUnits) -0.5;
}

void GR_CairoGraphics::setClipRect(const UT_Rect* pRect)
{
	m_pRect = pRect;
	m_clipRectDirty = true;
}

void GR_CairoGraphics::fillRect(const UT_RGBColor& c, UT_sint32 x, UT_sint32 y,
							   UT_sint32 w, UT_sint32 h)
{
	if (m_cr == NULL)
		return;
	_setProps();

	cairo_save(m_cr);
	if(!getAntiAliasAlways())
		cairo_set_antialias(m_cr,CAIRO_ANTIALIAS_NONE);

	_setSource(m_cr, c);
	cairo_rectangle(m_cr, _tdudX(x), _tdudY(y), _tduR(w), _tduR(h));
	cairo_fill(m_cr);

	cairo_restore(m_cr);
}

/*!
    Convert device units to pango units
*/
inline int GR_CairoGraphics::dtpu(int d) const
{
	return d * PANGO_SCALE;
}

/*!
    Convert pango units to device units
*/
inline int GR_CairoGraphics::ptdu(int p) const
{
	return PANGO_PIXELS(p);
}

/*!
    Convert pango units to layout units
*/
inline int GR_CairoGraphics::ptlu(int p) const
{
	double d = (double)p * (double) getResolution() * 100.0 /
		((double)getDeviceResolution()*(double)getZoomPercentage()*(double) PANGO_SCALE) + .5;

	return (int) d;
}


/*!
    Convert pango units to layout units without zoom
*/
inline int GR_CairoGraphics::ptlunz(int p) const
{
	double d = ((double)p / ((double) PANGO_SCALE)) + .5; //getDeviceResolution

	return (int) d;
}

/*!
    Convert layout units to pango units
*/
inline int GR_CairoGraphics::ltpu(int l) const
{
	double d = (double)l *
		(double)getDeviceResolution() * (double)PANGO_SCALE * (double)getZoomPercentage()/
		(100.0 * (double) getResolution()) + .5; 
	
	return (int) d;
}


/*!
    Convert layout units to pango units without zoom
*/
inline int GR_CairoGraphics::ltpunz(int l) const
{
	double d = (double)l * PANGO_SCALE  + .5; //getDeviceResolution()
	
	return (int) d;
}
	

/*!
    Convert pango font units to layout units

    (Pango font units == point size * PANGO_SCALE, hence at zoom of 100% there
    are 20/PANGO_SCALE layout units to each pango font unit.)
*/
inline int GR_CairoGraphics::pftlu(int pf) const
{
	double d = (double)pf * 2000.0 / ((double)getZoomPercentage() * (double)PANGO_SCALE);
	return (int) d;
}

void GR_CairoGraphics::fillRect(GR_Color3D c, UT_Rect &r)
{
	UT_ASSERT(m_bHave3DColors && c < COUNT_3D_COLORS);

	fillRect(c,r.left,r.top,r.width,r.height);
}

/*!
 * Rob sez: the original (before cairo) implementation did not restore colours after drawing.
 * We're trying to do the right thing here instead.
 */
void GR_CairoGraphics::fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
	if (m_cr == NULL)
		return;
	_setProps();
//	UT_ASSERT(m_bHave3DColors && c < COUNT_3D_COLORS);

	cairo_save (m_cr);

	if(!getAntiAliasAlways())
		cairo_set_antialias(m_cr,CAIRO_ANTIALIAS_NONE);

	_setSource(m_cr, m_3dColors[c]);
	cairo_rectangle(m_cr, tdu(x), tdu(y), tdu(w), tdu(h));
	cairo_fill(m_cr);

	cairo_restore (m_cr);
}

/*!
 * \todo Rob find out how to have this function used, and test.
 */
void GR_CairoGraphics::polygon(UT_RGBColor& c, UT_Point *pts,
								   UT_uint32 nPoints)
{
	if (m_cr == NULL)
		return;
	_setProps();
	UT_uint32 i;

	UT_return_if_fail(nPoints > 1);
	cairo_save(m_cr);
	if(!getAntiAliasAlways())
		cairo_set_antialias(m_cr,CAIRO_ANTIALIAS_NONE);

	i = 0;
	cairo_move_to(m_cr, _tdudX(pts[i].x), _tdudY(pts[i].y));
	i++;
	for (; i < nPoints; i++) {
		cairo_line_to(m_cr, _tdudX(pts[i].x), _tdudY(pts[i].y));
	}
	_setSource(m_cr, c);
	cairo_fill(m_cr);	

	cairo_restore(m_cr);
}

void GR_CairoGraphics::saveRectangle(UT_Rect &r, UT_uint32 iIndex)
{
	if(iIndex >= m_vSaveRect.size())
		m_vSaveRect.resize(iIndex + 1, NULL);
	if(iIndex >= m_vSaveRectBuf.size())
		m_vSaveRectBuf.resize(iIndex + 1, NULL);

	delete m_vSaveRect[iIndex];
	m_vSaveRect[iIndex] = new UT_Rect(r);

	cairo_save(m_cr);
	cairo_reset_clip(m_cr);

	cairo_rectangle_t cacheRect;
	cacheRect.x = -static_cast<double>(_tduX(r.left));
	cacheRect.y = -static_cast<double>(_tduY(r.top ));
	cacheRect.width  = static_cast<double>(_tduR(r.width ));
	cacheRect.height = static_cast<double>(_tduR(r.height));

	cairo_surface_flush(cairo_get_target(m_cr));
	cairo_surface_t* newC = _getCairoSurfaceFromContext(m_cr, cacheRect);

	cairo_surface_destroy(m_vSaveRectBuf[iIndex]);
	m_vSaveRectBuf[iIndex] = newC;

	cairo_restore(m_cr);
}

void GR_CairoGraphics::restoreRectangle(UT_uint32 iIndex)
{
	cairo_save(m_cr);
	cairo_reset_clip(m_cr);
	UT_Rect *r = m_vSaveRect[iIndex];
	cairo_surface_t *s = m_vSaveRectBuf[iIndex];
	double idx = static_cast<double>(_tduX(r->left)) - 0.5;
	double idy = static_cast<double>(_tduY(r->top)) - 0.5;
	cairo_surface_flush(cairo_get_target(m_cr));
	if(s && r)
	{
		cairo_set_source_surface(m_cr, s, idx, idy);
		cairo_paint(m_cr);
	}
	cairo_restore(m_cr);
}

void GR_CairoGraphics::clearArea(UT_sint32 x, UT_sint32 y,
									 UT_sint32 width, UT_sint32 height)
{
	if (width > 0)
	{
		static const UT_RGBColor clrWhite(255,255,255);
		fillRect(clrWhite, x, y, width, height);
	}
}

cairo_t *GR_CairoGraphics::getCairo()
{
	if (m_paintCount <= 0)
	{
		UT_DEBUGMSG(("GR_CairoGraphics::getCairo() called outside beginPaint/endPaint!\n"));
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		UT_DEBUGMSG(("GR_CairoGraphics::getCairo: calling beginPaint() for you, expect strange side-effects.\n"));
		beginPaint();
	}
	UT_ASSERT(m_cr);
	return m_cr;
}

void GR_CairoGraphics::setCairo(cairo_t *cr)
{
	m_cr = cr;
}

void GR_CairoGraphics::_DeviceContext_SwitchToBuffer()
{
	cairo_push_group(m_cr);
}

void GR_CairoGraphics::_DeviceContext_SwitchToScreen()
{
	cairo_pop_group_to_source(m_cr);
	cairo_paint(m_cr);
}

void GR_CairoGraphics::_DeviceContext_SuspendDrawing()
{
	cairo_push_group(m_cr);
}

void GR_CairoGraphics::_DeviceContext_ResumeDrawing()
{
	cairo_pattern_destroy (cairo_pop_group(m_cr));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GR_UnixPangFont implementation
//
GR_PangoFont::GR_PangoFont(const char * pDesc, double dSize,
								   GR_CairoGraphics * pG,
								   const char * pLang,
								   bool bGuiFont):
	m_dPointSize(dSize),
	m_iZoom(0), // forces creation of font by reloadFont()
	m_pf(NULL),
	m_bGuiFont(bGuiFont),
	m_pCover(NULL),
	m_pfdDev(NULL),
	m_pfdLay(NULL),
	m_pPLang(NULL),
	m_iAscent(0),
	m_iDescent(0),
	m_pLayoutF(NULL)
{
	m_eType = GR_FONT_UNIX_PANGO;
	UT_return_if_fail( pDesc && pG && pLang);

	m_sLayoutDesc = pDesc;
	m_sDesc = pDesc;
	setLanguage(pLang);
	reloadFont(pG);
	UT_DEBUGMSG(("Created UnixPangOFont %p \n",this));
}

GR_PangoFont::~GR_PangoFont()
{
	if(m_pCover)
		pango_coverage_unref(m_pCover);
	if (m_pf) 
	{
		g_object_unref(m_pf);
	}
	if (m_pLayoutF) 
	{
		g_object_unref(m_pLayoutF);
	}
	pango_font_description_free(m_pfdDev);
	pango_font_description_free(m_pfdLay);
}

void GR_PangoFont::setLanguage(const char * pLang)
{
	UT_return_if_fail( pLang );

	m_pPLang = pango_language_from_string(pLang); 
}

/*!
    Reloads the Pango font associated with this font, taking into account the
    current level of zoom
*/
void GR_PangoFont::reloadFont(GR_CairoGraphics * pG)
{
	UT_return_if_fail( pG );

	UT_uint32 iZoom = pG->getZoomPercentage();
	if(m_pf && (m_bGuiFont || m_iZoom == iZoom))
		return;
	
	m_iZoom = iZoom;

	UT_DEBUGMSG(("GR_PangoFont::reloadFont() zoom %% %d\n", iZoom));
	
	UT_LocaleTransactor t(LC_NUMERIC, "C");
 	std::string sLay;
 	std::string sDev;
	if(!m_bGuiFont && pG->queryProperties(GR_Graphics::DGP_SCREEN))
 	{
 		sDev = UT_std_string_sprintf("%s %f", m_sDesc.c_str(), m_dPointSize * (double)m_iZoom / 100.0);
 		sLay = UT_std_string_sprintf("%s %f", m_sLayoutDesc.c_str(), m_dPointSize);
 	}
	else
 	{
 		sDev = UT_std_string_sprintf("%s %f", m_sDesc.c_str(), m_dPointSize);
 		sLay = UT_std_string_sprintf("%s %f", m_sLayoutDesc.c_str(), m_dPointSize);
 	}		
  
 	if(m_pfdLay)
  	{
 		pango_font_description_free(m_pfdLay);
 		m_pfdLay = NULL;
  	}
 
 
 	if(m_pfdDev)
 	{
 		pango_font_description_free(m_pfdDev);
 		m_pfdDev = NULL;
 	}

 	m_pfdLay = pango_font_description_from_string(sLay.c_str());
 	UT_return_if_fail(m_pfdLay);
  
 	m_pfdDev = pango_font_description_from_string(sDev.c_str());
 	UT_return_if_fail(m_pfdDev);

	if (m_pf) {
		g_object_unref(m_pf);
	}
	m_pf = pango_context_load_font(pG->getContext(), m_pfdDev);
	if(m_pLayoutF) {
		g_object_unref(m_pLayoutF);
	}
	m_pLayoutF = pango_context_load_font(pG->getLayoutContext(), m_pfdLay);

	UT_return_if_fail( m_pf );
 	UT_return_if_fail( m_pLayoutF );
	// FIXME: we probably want the real language from somewhere
 	PangoFontMetrics * pfm = pango_font_get_metrics(m_pLayoutF, m_pPLang);
	UT_return_if_fail( pfm);

	// pango_metrics_ functions return in points * PANGO_SCALE (points * 1024)
 	m_iAscent = (UT_uint32) pango_font_metrics_get_ascent(pfm)/PANGO_SCALE;
 	m_iDescent = (UT_uint32) pango_font_metrics_get_descent(pfm)/PANGO_SCALE;
	UT_DEBUGMSG(("metrics asc %d desc %d\n", m_iAscent, m_iDescent));

 	xxx_UT_DEBUGMSG(("Layout Font Ascent %d point size %f zoom %d \n",m_iAscent, m_dPointSize, m_iZoom));
	pango_font_metrics_unref(pfm);

	UT_return_if_fail( pfm);
}


/*!
	Measure the unremapped char to be put into the cache.
	That means measuring it for a font size of 120
*/
UT_sint32 GR_PangoFont::measureUnremappedCharForCache(UT_UCS4Char /*cChar*/) const
{
	// this is not implemented because we do not use the width cache (when
	// shaping, it is not possible to measure characters, only glyphs)
	UT_ASSERT_HARMLESS( UT_NOT_IMPLEMENTED );
	return 0;
}

PangoCoverage * GR_PangoFont::getPangoCoverage() const
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
bool GR_PangoFont::doesGlyphExist(UT_UCS4Char g) const
{
	UT_return_val_if_fail( m_pf, false );

	PangoCoverage * pc = getPangoCoverage();
	UT_return_val_if_fail(pc, false);
	
	PangoCoverageLevel eLevel = pango_coverage_get(pc, g);

	if(PANGO_COVERAGE_NONE == eLevel || PANGO_COVERAGE_FALLBACK == eLevel)
		return false;

	return true;
}

static double fontPoints2float(double dSize, UT_sint32 iFontPoints)
{
	return dSize * ((double)iFontPoints / PANGO_SCALE) * 1.44/20.; // Last 20 is points to inches
}

static PangoGlyph getGlyphForChar(UT_UCS4Char g,
								  PangoFont *pf,
								  PangoContext *context)
{
	UT_UTF8String utf8(&g, 1);

	// this function expect indexes in bytes !!! (stupid)
	GList * pItems = pango_itemize(context,
								   utf8.utf8_str(),
								   0, utf8.byteLength(),
								   NULL, NULL);
	
	int iItemCount = g_list_length(pItems);
	PangoGlyphString * pGstring = pango_glyph_string_new();
	
	for(int i = 0; i < iItemCount; ++i)
	{
		PangoItem *pItem = (PangoItem *)g_list_nth(pItems, i)->data;

		if(!pItem)
		{
			UT_ASSERT(pItem);
			if(pGstring)
				pango_glyph_string_free(pGstring);
			_pango_item_list_free(pItems);
			return PANGO_GLYPH_EMPTY;
		}

		g_object_unref(pItem->analysis.font);
		pItem->analysis.font = (PangoFont*)g_object_ref((GObject*)pf);

		pango_shape(utf8.utf8_str()+ pItem->offset,
					pItem->length,
					&(pItem->analysis),
					pGstring);
	}

	PangoGlyph glyph = pGstring->glyphs[0].glyph;
	if(pGstring)
		pango_glyph_string_free(pGstring);
	_pango_item_list_free(pItems);
	return glyph;
}

bool GR_PangoFont::glyphBox(UT_UCS4Char g, UT_Rect & rec, GR_Graphics * pG)
{
	UT_return_val_if_fail( m_pf, false );
	
	double resRatio = pG->getResolutionRatio();

	guint iGlyphIndx = getGlyphForChar(g, m_pLayoutF, (static_cast<GR_CairoGraphics *>(pG))->getContext());

	PangoRectangle ink_rect;
	pango_font_get_glyph_extents(m_pLayoutF, iGlyphIndx, &ink_rect, NULL);

	double dSize = resRatio *(double)pG->getResolution() /
								  (double)pG->getDeviceResolution();

	rec.left   = static_cast<UT_sint32>(0.5 + fontPoints2float(dSize, ink_rect.x));
	
	rec.width  = static_cast<UT_sint32>(0.5 + fontPoints2float(dSize, ink_rect.width));
	
	rec.top    = static_cast<UT_sint32>(0.5 + fontPoints2float(dSize, -ink_rect.y));
	
	rec.height = static_cast<UT_sint32>(0.5 + fontPoints2float(dSize, ink_rect.height));

	UT_DEBUGMSG(("GlyphBox: %c [l:%d, w:%d, t:%d, h:%d\n",
				 (char)g, rec.left,rec.width,rec.top,rec.height));

	return true;
}

const char* GR_PangoFont::getFamily() const
{
	UT_return_val_if_fail( m_pfdLay, NULL );
	
	return pango_font_description_get_family(m_pfdLay);
}


//////////////////////////////////////////////////////////////////////////////
//
// GR_PangoRenderInfo Implementation
//

bool GR_PangoRenderInfo::canAppend(GR_RenderInfo &ri) const
{
	GR_PangoRenderInfo & RI = (GR_PangoRenderInfo &)ri;
	GR_CairoPangoItem * pItem1 = (GR_CairoPangoItem *)m_pItem;
	GR_CairoPangoItem * pItem2 = (GR_CairoPangoItem *)RI.m_pItem;

	/* Do not merger runs that have not been shapped yet */
	if (!pItem1 || !pItem2)
		return false;

	/* If the shapping resulted in font substitution we cannot merge */
	if (pItem1->m_pi->analysis.font == pItem2->m_pi->analysis.font)
		return true;

	return false;
}


bool GR_PangoRenderInfo::append(GR_RenderInfo &/*ri*/, bool /*bReverse*/)
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

bool GR_PangoRenderInfo::split (GR_RenderInfo *&pri, bool /*bReverse*/)
{
	UT_return_val_if_fail(m_pGraphics && m_pFont, false);

	UT_ASSERT_HARMLESS(!pri);

	// create a new RI and make a copy of item into
	if(!pri)
	{
		pri = new GR_PangoRenderInfo(m_eScriptType);
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

bool GR_PangoRenderInfo::cut(UT_uint32 /*offset*/, UT_uint32 /*iLen*/, bool /*bReverse*/)
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


bool GR_PangoRenderInfo::isJustified() const
{
    return (m_pJustify != NULL);
}


UT_uint32 adobeToUnicode(UT_uint32 c)
{
	/*
	 * generated from
	 * http://www.unicode.org/Public/MAPPINGS/VENDORS/ADOBE/symbol.txt
	 * maps Adobe Symbol Encoding to Unicode
	 */
	static const UT_uint32 map[256] = {
		/* 0x00 */      0,     0,     0,     0,     0,     0,     0,     0,
				0,     0,     0,     0,     0,     0,     0,     0,
		/* 0x10 */      0,     0,     0,     0,     0,     0,     0,     0,
				0,     0,     0,     0,     0,     0,     0,     0,
		/* 0x20 */ 0x0020,0x0021,0x2200,0x0023,0x2203,0x0025,0x0026,0x220B,
			   0x0028,0x0029,0x2217,0x002B,0x002C,0x2212,0x002E,0x002F,
		/* 0x30 */ 0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
			   0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
		/* 0x40 */ 0x2245,0x0391,0x0392,0x03A7,0x0394,0x0395,0x03A6,0x0393,
			   0x0397,0x0399,0x03D1,0x039A,0x039B,0x039C,0x039D,0x039F,
		/* 0x50 */ 0x03A0,0x0398,0x03A1,0x03A3,0x03A4,0x03A5,0x03C2,0x03A9,
			   0x039E,0x03A8,0x0396,0x005B,0x2234,0x005D,0x22A5,0x005F,
		/* 0x60 */ 0xF8E5,0x03B1,0x03B2,0x03C7,0x03B4,0x03B5,0x03C6,0x03B3,
			   0x03B7,0x03B9,0x03D5,0x03BA,0x03BB,0x00B5,0x03BD,0x03BF,
		/* 0x70 */ 0x03C0,0x03B8,0x03C1,0x03C3,0x03C4,0x03C5,0x03D6,0x03C9,
			   0x03BE,0x03C8,0x03B6,0x007B,0x007C,0x007D,0x223C,     0,
		/* 0x80 */      0,     0,     0,     0,     0,     0,     0,     0,
				0,     0,     0,     0,     0,     0,     0,     0,
		/* 0x90 */      0,     0,     0,     0,     0,     0,     0,     0,
				0,     0,     0,     0,     0,     0,     0,     0,
		/* 0xA0 */ 0x20AC,0x03D2,0x2032,0x2264,0x2044,0x221E,0x0192,0x2663,
			   0x2666,0x2665,0x2660,0x2194,0x2190,0x2191,0x2192,0x2193,
		/* 0xB0 */ 0x00B0,0x00B1,0x2033,0x2265,0x00D7,0x221D,0x2202,0x2022,
			   0x00F7,0x2260,0x2261,0x2248,0x2026,0xF8E6,0xF8E7,0x21B5,
		/* 0xC0 */ 0x2135,0x2111,0x211C,0x2118,0x2297,0x2295,0x2205,0x2229,
			   0x222A,0x2283,0x2287,0x2284,0x2282,0x2286,0x2208,0x2209,
		/* 0xD0 */ 0x2220,0x2207,0xF6DA,0xF6D9,0xF6DB,0x220F,0x221A,0x22C5,
			   0x00AC,0x2227,0x2228,0x21D4,0x21D0,0x21D1,0x21D2,0x21D3,
		/* 0xE0 */ 0x25CA,0x2329,0xF8E8,0xF8E9,0xF8EA,0x2211,0xF8EB,0xF8EC,
			   0xF8ED,0xF8EE,0xF8EF,0xF8F0,0xF8F1,0xF8F2,0xF8F3,0xF8F4,
		/* 0xF0 */      0,0x232A,0x222B,0x2320,0xF8F5,0x2321,0xF8F6,0xF8F7,
			   0xF8F8,0xF8F9,0xF8FA,0xF8FB,0xF8FC,0xF8FD,0xF8FE,     0,
		};
	if (c <= 0xFF && map[c] != 0)
		return map[c];
	else
		return c;
}

UT_uint32 adobeDingbatsToUnicode(UT_uint32 c)
{
	/*
	 * generated from
	 * http://www.unicode.org/Public/MAPPINGS/VENDORS/ADOBE/zdingbat.txt
	 * maps Adobe Zapf Dingbats Encoding to Unicode
	 */
	static const UT_uint32 map[256] = {
		/* 0x00 */      0,     0,     0,     0,     0,     0,     0,     0,
		                0,     0,     0,     0,     0,     0,     0,     0,
		/* 0x10 */      0,     0,     0,     0,     0,     0,     0,     0,
		                0,     0,     0,     0,     0,     0,     0,     0,
		/* 0x20 */ 0x0020,0x2701,0x2702,0x2703,0x2704,0x260E,0x2706,0x2707,
		           0x2708,0x2709,0x261B,0x261E,0x270C,0x270D,0x270E,0x270F,
		/* 0x30 */ 0x2710,0x2711,0x2712,0x2713,0x2714,0x2715,0x2716,0x2717,
		           0x2718,0x2719,0x271A,0x271B,0x271C,0x271D,0x271E,0x271F,
		/* 0x40 */ 0x2720,0x2721,0x2722,0x2723,0x2724,0x2725,0x2726,0x2727,
		           0x2605,0x2729,0x272A,0x272B,0x272C,0x272D,0x272E,0x272F,
		/* 0x50 */ 0x2730,0x2731,0x2732,0x2733,0x2734,0x2735,0x2736,0x2737,
		           0x2738,0x2739,0x273A,0x273B,0x273C,0x273D,0x273E,0x273F,
		/* 0x60 */ 0x2740,0x2741,0x2742,0x2743,0x2744,0x2745,0x2746,0x2747,
		           0x2748,0x2749,0x274A,0x274B,0x25CF,0x274D,0x25A0,0x274F,
		/* 0x70 */ 0x2750,0x2751,0x2752,0x25B2,0x25BC,0x25C6,0x2756,0x25D7,
		           0x2758,0x2759,0x275A,0x275B,0x275C,0x275D,0x275E,     0,
		/* 0x80 */ 0xF8D7,0xF8D8,0xF8D9,0xF8DA,0xF8DB,0xF8DC,0xF8DD,0xF8DE,
		           0xF8DF,0xF8E0,0xF8E1,0xF8E2,0xF8E3,0xF8E4,     0,     0,
		/* 0x90 */      0,     0,     0,     0,     0,     0,     0,     0,
		                0,     0,     0,     0,     0,     0,     0,     0,
		/* 0xA0 */      0,0x2761,0x2762,0x2763,0x2764,0x2765,0x2766,0x2767,
		           0x2663,0x2666,0x2665,0x2660,0x2460,0x2461,0x2462,0x2463,
		/* 0xB0 */ 0x2464,0x2465,0x2466,0x2467,0x2468,0x2469,0x2776,0x2777,
		           0x2778,0x2779,0x277A,0x277B,0x277C,0x277D,0x277E,0x277F,
		/* 0xC0 */ 0x2780,0x2781,0x2782,0x2783,0x2784,0x2785,0x2786,0x2787,
		           0x2788,0x2789,0x278A,0x278B,0x278C,0x278D,0x278E,0x278F,
		/* 0xD0 */ 0x2790,0x2791,0x2792,0x2793,0x2794,0x2192,0x2194,0x2195,
		           0x2798,0x2799,0x279A,0x279B,0x279C,0x279D,0x279E,0x279F,
		/* 0xE0 */ 0x27A0,0x27A1,0x27A2,0x27A3,0x27A4,0x27A5,0x27A6,0x27A7,
		           0x27A8,0x27A9,0x27AA,0x27AB,0x27AC,0x27AD,0x27AE,0x27AF,
		/* 0xF0 */      0,0x27B1,0x27B2,0x27B3,0x27B4,0x27B5,0x27B6,0x27B7,
		           0x27B8,0x27B9,0x27BA,0x27BB,0x27BC,0x27BD,0x27BE,     0,
		};
	if (c <= 0xFF && map[c] != 0)
		return map[c];
	else
		return c;
}

void GR_Font::s_getGenericFontProperties(const char * /*szFontName*/,
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
