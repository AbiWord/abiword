/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2003 Francis James Franklin <fjf@alinameridon.com>
 * Copyright (C) 2003 AbiSource, Inc.
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

#ifndef UT_INTSTRMAP_H
#define UT_INTSTRMAP_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

class UT_UTF8String;

class ABI_EXPORT UT_IntStrMap
{
private:
	struct IntStr
	{
		UT_sint32		key;
		UT_UTF8String *	value;
	};
public:
	UT_IntStrMap ();
	UT_IntStrMap (UT_uint32 increment);

	~UT_IntStrMap ();

	void clear ();

	bool ins (UT_sint32 key, UT_UTF8String * value); // responsibility for value passes here
	bool ins (UT_sint32 key, const char * value);

	/* returns false if no such key-value
	 */
	bool del (UT_sint32 key);                         // value is deleted
	bool del (UT_sint32 key, UT_UTF8String *& value); // value is passed back

	inline const UT_UTF8String * operator[] (UT_sint32 key)
	{
		UT_uint32 index;
		return lookup (key, index) ? m_pair[index].value : 0;
	}

private:
	bool lookup (UT_sint32 key, UT_uint32 & index);

	bool grow ();

	IntStr *	m_pair;

	UT_uint32	m_pair_count;
	UT_uint32	m_pair_max;

	UT_uint32	m_index;
	UT_uint32	m_increment;

public:
	inline bool pair (UT_uint32 index, UT_sint32 & key, const UT_UTF8String *& value) const
	{
		if (index >= m_pair_count) return false;
		key   = m_pair[index].key;
		value = m_pair[index].value;
		return true;
	}
	UT_uint32 count () const { return m_pair_count; }
};

#endif /* ! UT_INTSTRMAP_H */
