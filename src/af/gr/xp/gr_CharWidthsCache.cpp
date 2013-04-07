/* AbiSource Application Framework
 * Copyright (C) 2003, 2013 Hubert Figuiere
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

#include "gr_CharWidthsCache.h"

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_std_map.h"

#include "gr_Graphics.h"
#include "gr_CharWidths.h"

GR_CharWidthsCache* GR_CharWidthsCache::s_pInstance = NULL;
 
void GR_CharWidthsCache::_instantiate(void)
{
	if (!s_pInstance) {
		s_pInstance = new GR_CharWidthsCache;
	}
}

GR_CharWidthsCache::GR_CharWidthsCache()
{
}

GR_CharWidthsCache::~GR_CharWidthsCache()
{
	UT_map_delete_all_second(m_fontHash);
}

GR_CharWidthsCache::FontCache::iterator
GR_CharWidthsCache::addFont (const GR_Font* pFont)
{
	GR_CharWidths* content = pFont->newFontWidths();
	std::pair<FontCache::iterator, bool> insertion =
		m_fontHash.insert(make_pair(pFont->hashKey(), content));

	return insertion.first;
}

GR_CharWidths*	GR_CharWidthsCache::getWidthsForFont(const GR_Font* pFont)
{
	FontCache::const_iterator iter = m_fontHash.find(pFont->hashKey());
	if (iter == m_fontHash.end()) {
		iter = addFont(pFont);
		UT_DEBUGMSG(("added font widths to cache for font with hashkey '%s'\n", pFont->hashKey().c_str()));
	}
	return iter->second;
}
