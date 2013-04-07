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
#include <OXML_Theme.h>

OXML_Theme::OXML_Theme()
{
	for (int i = 0; i < 12; i++) {
		m_colorScheme[i] = "";
	}
}

std::string OXML_Theme::getMajorFont(std::string script)
{
	OXML_FontScheme::iterator it;
	it = m_majorFontScheme.find(script);
	return it != m_majorFontScheme.end() ? it->second : "" ;
}

std::string OXML_Theme::getMinorFont(std::string script)
{
	OXML_FontScheme::iterator it;
	it = m_minorFontScheme.find(script);
	return it != m_minorFontScheme.end() ? it->second : "" ;
}

