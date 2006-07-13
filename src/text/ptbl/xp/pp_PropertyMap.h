/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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


#ifndef PP_PROPERTYMAP_H
#define PP_PROPERTYMAP_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

#include "ut_misc.h"
#include "ut_IntStrMap.h"
#include "pp_Property.h"

class ABI_EXPORT PP_PropertyMap
{
public:
	enum TypeColor
	{
		color__unset = 0,
		color_inherit,
		color_transparent,
		color_color
	};
	enum TypeLineStyle
	{
		linestyle__unset = 0,
		linestyle_none,   // "0" or "none"
		linestyle_solid,  // "1" or "solid"
		linestyle_dotted, // "2" or "dotted"
		linestyle_dashed, // "3" or "dashed"

		// insert new styles here; do not change order of existing styles

		linestyle_inherit
	};
	enum TypeThickness
	{
		thickness__unset = 0,
		thickness_inherit,
		thickness_length
	};

	/* currently support only for solid fill styles, but should be able to
	 * come up with something to support images etc. - what else?
	 */
	enum TypeBackground
	{
		background__unset = 0,
		background_none,  // background-color: transparent
		background_solid, // background-color: <color>

		// insert new styles here; do not change order of existing styles

		background_inherit
	};

	static TypeColor color_type (const char * property);
	static TypeLineStyle linestyle_type (const char * property);
	static TypeThickness thickness_type (const char * property);
	static TypeBackground background_type (const char * property);

	static const char * linestyle_for_CSS (const char * property);

	class ABI_EXPORT Line
	{
	public:
		Line ();

		void reset (); // restore defaults

		TypeColor		m_t_color;
		TypeLineStyle	m_t_linestyle;
		TypeThickness	m_t_thickness;

		UT_RGBColor		m_color;		// in case of color_color
		UT_uint32		m_thickness;	// in case of thickness_length
	};
	class ABI_EXPORT Background
	{
	public:
		Background ();

		void reset (); // restore defaults

		TypeBackground	m_t_background;

		UT_RGBColor		m_color;		// in case of background_solid
	};

private:
	UT_IntStrMap	m_map;
public:
	const UT_IntStrMap & map () const { return m_map; }

	inline void clear ()
	{
		m_map.clear ();
	}

	bool ins (PT_Property key, UT_UTF8String * value) // responsibility for value passes here
	{
		if ((value == 0) || (!PP_Property::isIndexValid(key))) return false;
		return m_map.ins (static_cast<UT_sint32>(key), value);
	}
	bool ins (PT_Property key, const char * value)
	{
		if ((value == 0) || (!PP_Property::isIndexValid(key))) return false;
		return m_map.ins (static_cast<UT_sint32>(key), value);
	}

	/* returns false if no such key-value
	 */
	inline bool del (PT_Property key) // value is deleted
	{
		if (!PP_Property::isIndexValid(key)) return false;
		return m_map.del (static_cast<UT_sint32>(key));
	}
	inline bool del (PT_Property key, UT_UTF8String *& value) // value is passed back
	{
		if (!PP_Property::isIndexValid(key)) return false;
		return m_map.del (static_cast<UT_sint32>(key), value);
	}

	inline const UT_UTF8String * operator[] (PT_Property key)
	{
		if (!PP_Property::isIndexValid(key)) return 0;
		return m_map[static_cast<UT_sint32>(key)];
	}
};

#endif /* ! PP_PROPERTYMAP_H */
