/* AbiSource Application Framework
 * Copyright (C) 2002 Tomas Frydrych <tomas@frydrych.uklinux.net>
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

#include "xap_PangoFontManager.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_string_class.h"

static int s_cmp_qsort(const void * a, const void *b)
{
	return UT_strcmp(static_cast<const char *>(a), static_cast<const char *>(b));
}

XAP_PangoFontManager::XAP_PangoFontManager(PangoContext * pContext)
{
	m_pPangoContext = pContext;
	
	PangoFontFamily ** ppFontFamilies;
	UT_sint32 iFontFamiliesCount;

	pango_context_list_families(m_pPangoContext, &ppFontFamilies, &iFontFamiliesCount);
	UT_ASSERT(iFontFamiliesCount > 0 && ppFontFamilies != NULL);

	for (UT_sint32 i = 0; i < iFontFamiliesCount; i++)
	{
		m_vFontFamilies.addItem(static_cast<void*>(UT_strdup(pango_font_family_get_name(ppFontFamilies[i]))));
	}

	m_vFontFamilies.qsort(s_cmp_qsort);
	
	g_free(ppFontFamilies);
}

	

XAP_PangoFontManager::~XAP_PangoFontManager()
{
	UT_uint32 i;
	
	for(i = 0; i < m_vAllocatedFonts.getItemCount(); i++)
	{
		//not sure if simple g_free is what we need to use here ...
		g_free(m_vAllocatedFonts.getNthItem(i));
	}
	
	for(i = 0; i < m_vFontFamilies.getItemCount(); i++)
	{
		free(m_vFontFamilies.getNthItem(i));
	}
}
	
		
PangoFont * XAP_PangoFontManager::findFont(PangoFontDescription * pDescription)
{
	// first see if we already have such a font cached
	PangoFont * pFont;
	
	for(UT_uint32 i = 0; i < m_vAllocatedFonts.getItemCount(); i++)
	{
		pFont = static_cast<PangoFont*>(m_vAllocatedFonts.getNthItem(i));
		PangoFontDescription * pDescription2 = pango_font_describe(pFont);
		if(pango_font_description_equal(pDescription, pDescription2))
		{
			pango_font_description_free(pDescription2);
			return pFont;
		}

		pango_font_description_free(pDescription2);
		
	}
	
	// if not, load it ...
	pFont = pango_context_load_font(m_pPangoContext, pDescription);
	m_vAllocatedFonts.addItem(static_cast<void *>(pFont));
	return pFont;
}


PangoFont * XAP_PangoFontManager::findFont(const char* pszFontFamily, 
										   const char* pszFontStyle, 
										   const char* pszFontVariant, 
										   const char* pszFontWeight, 
										   const char* pszFontStretch, 
										   const char* pszFontSize)
{
	// turn the face name and size into description ...
	UT_String str("[");
	str += pszFontFamily;
	str += "] [";
	str += pszFontStyle;
	str += ' ';
	str += pszFontVariant;
	str += ' ';
	str += pszFontWeight;
	str += ' ';
	str += pszFontStretch;
	str += "] [";
	str += pszFontSize;
	str += ']';
	
	PangoFontDescription * pDescription = pango_font_description_from_string(str.c_str());

	// now call the other findFont
	PangoFont * pFont = findFont(pDescription);
	pango_font_description_free(pDescription);
	return pFont;
}


UT_uint32 XAP_PangoFontManager::getAvailableFontFamiliesCount() const
{
	return m_vFontFamilies.getItemCount();
}

const char * XAP_PangoFontManager::getNthAvailableFontFamily(UT_uint32 n) const
{
	return static_cast<const char *>(m_vFontFamilies.getNthItem(n));
}

