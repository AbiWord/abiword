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

#include <stdlib.h>

#include "ut_exception.h"
#include "ut_string_class.h"
#include "ut_IntStrMap.h"

UT_IntStrMap::UT_IntStrMap () :
	m_pair(0),
	m_pair_count(0),
	m_pair_max(0),
	m_index(0),
	m_increment(8)
{
	// 
}

UT_IntStrMap::UT_IntStrMap (UT_uint32 increment) :
	m_pair(0),
	m_pair_count(0),
	m_pair_max(0),
	m_index(0),
	m_increment(increment)
{
	if (m_increment == 0) m_increment = 8;
}

UT_IntStrMap::~UT_IntStrMap ()
{
	clear ();
	if (m_pair) free (m_pair);
}

void UT_IntStrMap::clear ()
{
	for (UT_uint32 i = 0; i < m_pair_count; i++) delete m_pair[i].value;
	m_pair_count = 0;
}

/* responsibility for value passes here
 */
bool UT_IntStrMap::ins (UT_sint32 key, UT_UTF8String * value)
{
	if (value == 0) return false;

	UT_uint32 index;
	if (lookup (key, index))
		{
			delete m_pair[index].value;
			m_pair[index].value = value;
			return true;
		}

	if (!grow ()) return false;

	if (index < m_pair_count)
		{
			memmove (m_pair + index + 1, m_pair + index, (m_pair_count - index) * sizeof (IntStr));
		}
	++m_pair_count;

	m_pair[index].key   = key;
	m_pair[index].value = value;

	return true;
}

bool UT_IntStrMap::ins (UT_sint32 key, const char * value)
{
	if (value == 0) return false;

	UT_uint32 index;
	if (lookup (key, index))
		{
			*(m_pair[index].value) = value;
			return true;
		}

	if (!grow ()) return false;

	UT_UTF8String * utf8str = 0;
	UT_TRY
		{
			utf8str = new UT_UTF8String(value);
		}
	UT_CATCH(...)
		{
			utf8str = 0;
		}
	if (utf8str == 0) return false;

	return ins (key, utf8str);
}

/* value is deleted
 */
bool UT_IntStrMap::del (UT_sint32 key)
{
	UT_uint32 index;
	if (!lookup (key, index)) return false;

	delete m_pair[index].value;

	if (index < --m_pair_count)
		{
			memmove (m_pair + index, m_pair + index + 1, (m_pair_count - index) * sizeof (IntStr));
		}
	return true;
}

/* value is passed back
 */
bool UT_IntStrMap::del (UT_sint32 key, UT_UTF8String *& value)
{
	UT_uint32 index;
	if (!lookup (key, index)) return false;

	value = m_pair[index].value;

	if (index < --m_pair_count)
		{
			memmove (m_pair + index, m_pair + index + 1, (m_pair_count - index) * sizeof (IntStr));
		}
	return true;
}

bool UT_IntStrMap::lookup (UT_sint32 key, UT_uint32 & index)
{
	if (m_pair_count == 0)
		{
			index = 0;
			return false;
		}
	if (key < m_pair[0].key)
		{
			index = 0;
			return false;
		}
	if (key > m_pair[m_pair_count-1].key)
		{
			index = m_pair_count;
			return false;
		}
	if (m_index < m_pair_count)
		{
			if (key == m_pair[m_index].key)
				{
					index = m_index;
					return true;
				}
			if ((key < m_pair[m_index].key) && m_index)
				if (key > m_pair[m_index-1].key)
					{
						index = m_index;
						return false;
					}
		}
	else
		{
			m_index = m_pair_count / 2;
			if (key == m_pair[m_index].key)
				{
					index = m_index;
					return true;
				}
		}

	bool bFound = false;

	UT_uint32 min_index = 0;
	UT_uint32 max_index = m_pair_count;

	if (key > m_pair[m_index].key)
		min_index = m_index;
	else
		max_index = m_index;

	while (true)
		{
			if (max_index - min_index == 1)
				{
					m_index = max_index;
					break;
				}
			m_index = min_index + (max_index - min_index) / 2;

			if (key == m_pair[m_index].key)
				{
					bFound = true;
					break;
				}
			if (key > m_pair[m_index].key)
				min_index = m_index;
			else
				max_index = m_index;
		}
	index = m_index;

	return bFound;
}

bool UT_IntStrMap::grow ()
{
	if (m_pair_count < m_pair_max) return true;

	if (m_pair == 0)
		{
			m_pair = (IntStr *) malloc (m_increment * sizeof (IntStr));
			if (m_pair == 0) return false;

			m_pair_max = m_increment;
		}
	else
		{
			IntStr * more = (IntStr *) realloc (m_pair, (m_pair_max + m_increment) * sizeof (IntStr));
			if (more == 0) return false;

			m_pair = more;
			m_pair_max += m_increment;
		}
	return true;
}
