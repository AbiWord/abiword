/* AbiSource Application Framework
 * Copyright (C) 2003 Hubert Figuiere
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

#include "gr_CharWidthsCache.h"

#include "ut_types.h"
#include "ut_hash.h"
#include "ut_debugmsg.h"

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
	: m_pFontHash (new UT_GenericStringMap<GR_CharWidths*>())
{
}

GR_CharWidthsCache::~GR_CharWidthsCache()
{
	m_pFontHash->purgeData();
	DELETEP(m_pFontHash);
}

bool GR_CharWidthsCache::addFont (const GR_Font* pFont)
{
	GR_CharWidths* content = pFont->newFontWidths();
	bool added = m_pFontHash->insert(pFont->hashKey(), content);
	if (!added) {
		UT_DEBUGMSG(("cache font add failed.\n"));		
		DELETEP(content);
	}
	return added;
}

GR_CharWidths*	GR_CharWidthsCache::getWidthsForFont(const GR_Font* pFont)
{
	GR_CharWidths* pCharWidths;

	UT_ASSERT(m_pFontHash);
	pCharWidths = m_pFontHash->pick(pFont->hashKey());
	if (!pCharWidths) {
		addFont(pFont);
		UT_DEBUGMSG(("added font widths to cache for font with hashkey '%s'\n", pFont->hashKey().c_str()));
		pCharWidths = m_pFontHash->pick(pFont->hashKey());
		UT_ASSERT(pCharWidths);
	}
	return pCharWidths;
}
