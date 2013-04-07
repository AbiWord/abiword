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

#ifndef _GR_CHAR_WIDTHS_CACHE_H__
#define _GR_CHAR_WIDTHS_CACHE_H__

#include <map>
#include <string>

#include "ut_types.h"

class GR_Font;
class GR_CharWidths;

class ABI_EXPORT GR_CharWidthsCache
{
public:

	/*!
		Get the singleton instance for the class. Instantiate it if needed.
	 */
	static GR_CharWidthsCache* getCharWidthCache()
	{
		_instantiate();
		return s_pInstance;
	}

	enum {
		CACHE_FONT_SIZE = 120
	};

	static void destroyCharWidthsCache()
	{
		if(s_pInstance)
		{
			delete s_pInstance;
			s_pInstance = NULL;
		}
	}

protected:
	typedef std::map<std::string, GR_CharWidths*> FontCache;

	FontCache::iterator addFont (const GR_Font* pFont);
	/*!
		Return the char Width for the font
	 */
	GR_CharWidths*	getWidthsForFont(const GR_Font* pFont);
	friend class GR_Font;
private:
	GR_CharWidthsCache();
	~GR_CharWidthsCache();

	/*!
		Instanciate the singleton instance.
	 */
	static void _instantiate(void);

	/*!
		The singleton instance.
	 */
	static GR_CharWidthsCache* s_pInstance;
	/*!
		The font hash that contains the GR_CharWidths
	 */
	FontCache m_fontHash;
};


#endif
