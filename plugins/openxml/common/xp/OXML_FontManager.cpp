/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 * 
 * Copyright (C) 2007 Philippe Milot <PhilMilot@gmail.com>
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

// Class definition include
#include <OXML_FontManager.h>

// Internal includes
#include <OXML_Document.h>
#include <OXML_Theme.h>

// AbiWord includes
#include <ut_assert.h>

// External includes
#include <string>

OXML_FontManager::OXML_FontManager() : 
	m_defaultFont("Times New Roman")
{
	m_major_rts.clear();
	m_minor_rts.clear();
}

std::string OXML_FontManager::getValidFont(OXML_FontLevel level, OXML_CharRange range)
{
	UT_return_val_if_fail(	UNKNOWN_LEVEL != level && 
							UNKNOWN_RANGE != range, m_defaultFont);
	//Algorithm:
	// 1) Retrieve the lang code mapped with this level/range combination in the Doc Settings
	std::string script(""), font_name("");
	OXML_RangeToScriptMap::iterator it;
	if (level == MAJOR_FONT) {
		it = m_major_rts.find(range);
		if (it == m_major_rts.end()) {
			switch (range) {
			// 1a) If no mapping exists, then Ascii/HAnsi = "latin"; EastAsia = "ea"; Complex = "cs"
			case (ASCII_RANGE): //fallthrough to HANSI_RANGE
			case (HANSI_RANGE): script = "latin"; break;
			case (COMPLEX_RANGE): script = "cs"; break;
			case (EASTASIAN_RANGE): script = "ea"; break;
			default: UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
		} else {
			script = it->second;
		}
	} else {
		it = m_minor_rts.find(range);
		if (it == m_minor_rts.end()) {
			switch (range) {
			// 1a) If no mapping exists, then Ascii/HAnsi = "latin"; EastAsia = "ea"; Complex = "cs"
			case (ASCII_RANGE): //fallthrough to HANSI_RANGE
			case (HANSI_RANGE): script = "latin"; break;
			case (COMPLEX_RANGE): script = "cs"; break;
			case (EASTASIAN_RANGE): script = "ea"; break;
			default: UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
		} else {
			script = it->second;
		}
	}

	// 2) Retrieve the font name mapped with this country string in the Theme
	OXML_Document * doc = OXML_Document::getInstance();
	if (NULL == doc) return m_defaultFont;
	OXML_SharedTheme theme = doc->getTheme();
	if (theme.get() == NULL) return m_defaultFont;
	if (level == MAJOR_FONT)
		font_name = theme->getMajorFont(script);
	else 
		font_name = theme->getMinorFont(script);

	// 2a) If no mapping exists, return the return default document font
	if (!font_name.compare("")) return m_defaultFont;

	// 3) Return getValidFont(font name)
	return getValidFont(font_name);
}

std::string OXML_FontManager::getValidFont(std::string name)
{
	//TODO: write this function
	//Algorithm:
	// 1) If name is a valid font name, return it
	// 2) Look up font name in the FontTable (lookup using both font name and altname)
	// 2a) If there are no matching entries, return default document font
	// 3) If either font name or altname is a valid font name, return it.
	// 3a) If neither font name or altname is a valid font name, return a valid substitution based on FontTable data
	return name;
}

void OXML_FontManager::mapRangeToScript(OXML_CharRange range, std::string script)
{
	m_major_rts[range] = script;
	m_minor_rts[range] = script;
}

