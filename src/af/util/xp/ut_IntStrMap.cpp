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
#include <string.h>
#include <ctype.h>

#include "ut_exception.h"
#include "ut_IntStrMap.h"

static bool key_lt (const char * key, UT_uint32 key_length, const UT_UTF8String & key2);
static bool key_gt (const char * key, UT_uint32 key_length, const UT_UTF8String & key2);
static bool key_eq (const char * key, UT_uint32 key_length, const UT_UTF8String & key2);

static void         s_pass_whitespace (const char *& csstr);
static const char * s_pass_name (const char *& csstr, char end);
static const char * s_pass_value (const char *& csstr);
static const char * s_pass_string (const char *& csstr);


UT_GenericUTF8Hash::KeyValue::KeyValue (const UT_UTF8String & key) :
	m_key(key),
	m_value(0)
{
	// 
}

UT_GenericUTF8Hash::KeyValue::~KeyValue ()
{
	if (m_value) delete m_value;
}

/* responsibility for value passes here
 */
void UT_GenericUTF8Hash::KeyValue::setValue (UT_GenericBase * value)
{
	if (m_value) delete m_value;

	m_value = value;
}

UT_GenericBase * UT_GenericUTF8Hash::KeyValue::getValue ()
{
	UT_GenericBase * value = m_value;
	m_value = 0;
	return value;
}

UT_GenericUTF8Hash::UT_GenericUTF8Hash (UT_uint32 increment) :
	m_pair(0),
	m_pair_count(0),
	m_pair_max(0),
	m_index(0),
	m_increment(increment)
{
	// 
}

UT_GenericUTF8Hash::~UT_GenericUTF8Hash ()
{
	clear (false);
	if (m_pair) g_free (m_pair);
}

/* deletes all key/value pairs, but doesn't g_free() array of pointers
 */
void UT_GenericUTF8Hash::clear (bool delete_values)
{
	for (UT_uint32 i = 0; i < m_pair_count; i++)
		{
			if (!delete_values)
				m_pair[i]->getValue (); // detach value, if any, so not deleted
			delete m_pair[i];
		}
	m_pair_count = 0;
}

/* for easy sequential access of map members:
 */
bool UT_GenericUTF8Hash::pair (UT_uint32 index, const UT_UTF8String *& key, const UT_GenericBase *& value) const
{
	if (index >= m_pair_count) return false;
	key   = &m_pair[index]->key ();
	value =  m_pair[index]->value ();
	return true;
}

/* responsibility for value passes here
 */
bool UT_GenericUTF8Hash::ins (const UT_UTF8String & key, UT_GenericBase * value)
{
	UT_uint32 index;
	if (lookup (key, index))
		{
			m_pair[index]->setValue (value);
			return true;
		}
	if (!grow ())
		{
			if (value) delete value;
			return false;
		}

	KeyValue * KV = 0;
	UT_TRY
		{
			KV = new KeyValue(key);
		}
	UT_CATCH(...)
		{
			KV = 0;
		}
	if (KV == 0)
		{
			if (value) delete value;
			return false;
		}

	if (index < m_pair_count)
		{
			memmove (m_pair + index + 1, m_pair + index, (m_pair_count - index) * sizeof (KeyValue *));
		}
	++m_pair_count;

	m_pair[index] = KV;

	KV->setValue (value);
	return true;
}

/* returns false if no such key-value
 */
bool UT_GenericUTF8Hash::del (const char * key)
{
	UT_GenericBase * value = 0;

	bool removed = del (key, value);

	if (removed && value)
		delete value;

	return removed;
}

/* returns false if no such key-value
 */
bool UT_GenericUTF8Hash::del (const UT_UTF8String & key)
{
	UT_GenericBase * value = 0;

	bool removed = del (key, value);

	if (removed && value)
		delete value;

	return removed;
}

bool UT_GenericUTF8Hash::del (const char * key, UT_GenericBase *& value) // return value rather than deleting
{
	UT_uint32 index;
	if (!lookup (key, index)) return false;

	value = m_pair[index]->getValue ();

	delete m_pair[index];

	if (index < --m_pair_count)
		{
			memmove (m_pair + index, m_pair + index + 1, (m_pair_count - index) * sizeof (KeyValue *));
		}
	return true;
}

bool UT_GenericUTF8Hash::del (const UT_UTF8String & key, UT_GenericBase *& value) // return value rather than deleting
{
	UT_uint32 index;
	if (!lookup (key, index)) return false;

	value = m_pair[index]->getValue ();

	delete m_pair[index];

	if (index < --m_pair_count)
		{
			memmove (m_pair + index, m_pair + index + 1, (m_pair_count - index) * sizeof (KeyValue *));
		}
	return true;
}

const UT_GenericBase * UT_GenericUTF8Hash::lookup (const char * key)
{
	UT_uint32 index;
	return lookup (key, index) ? m_pair[index]->value () : 0;
}

const UT_GenericBase * UT_GenericUTF8Hash::lookup (const UT_UTF8String & key)
{
	UT_uint32 index;
	return lookup (key, index) ? m_pair[index]->value () : 0;
}

bool UT_GenericUTF8Hash::lookup (const char * key, UT_uint32 & index)
{
	return key ? lookup (key, strlen (key), index) : false;
}

bool UT_GenericUTF8Hash::lookup (const UT_UTF8String & key, UT_uint32 & index)
{
	return lookup (key.utf8_str (), key.byteLength (), index);
}

bool UT_GenericUTF8Hash::lookup (const char * key, UT_uint32 key_length, UT_uint32 & index)
{
	if (m_index < m_pair_count)
		if (key_eq (key, key_length, m_pair[m_index]->key ()))
			{
				index = m_index;
				return true;
			}
	if (m_pair_count == 0)
		{
			index = 0;
			return false;
		}
	if (!key_gt (key, key_length, m_pair[0]->key ()))
		{
			index = 0;
			return key_eq (key, key_length, m_pair[0]->key ());
		}
	if (key_gt (key, key_length, m_pair[m_pair_count-1]->key ()))
		{
			index = m_pair_count;
			return false;
		}
	if (m_index < m_pair_count)
		{
			if (key_lt (key, key_length, m_pair[m_index]->key ()) && m_index)
				if (key_gt (key, key_length, m_pair[m_index-1]->key ()))
					{
						index = m_index;
						return false;
					}
		}
	else
		{
			m_index = m_pair_count / 2;
			if (key_eq (key, key_length, m_pair[m_index]->key ()))
				{
					index = m_index;
					return true;
				}
		}

	bool bFound = false;

	UT_uint32 min_index = 0;
	UT_uint32 max_index = m_pair_count;

	if (key_gt (key, key_length, m_pair[m_index]->key ()))
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

			if (key_eq (key, key_length, m_pair[m_index]->key ()))
				{
					bFound = true;
					break;
				}
			if (key_gt (key, key_length, m_pair[m_index]->key ()))
				min_index = m_index;
			else
				max_index = m_index;
		}
	index = m_index;

	return bFound;
}

bool UT_GenericUTF8Hash::grow ()
{
	if (m_pair_count < m_pair_max) return true;

	if (m_pair == 0)
		{
			m_pair = (KeyValue **) g_try_malloc (m_increment * sizeof (KeyValue *));
			if (m_pair == 0) return false;

			m_pair_max = m_increment;
		}
	else
		{
			KeyValue ** more = (KeyValue **) g_try_realloc (m_pair, (m_pair_max + m_increment) * sizeof (KeyValue *));
			if (more == 0) return false;

			m_pair = more;
			m_pair_max += m_increment;
		}
	return true;
}

UT_UTF8Hash::UT_UTF8Hash (bool bStripEmptyValues) :
	UT_GenericUTF8Hash(32),
	m_bStripEmptyValues(bStripEmptyValues)
{
}

UT_UTF8Hash::~UT_UTF8Hash ()
{
	clear ();
}

bool UT_UTF8Hash::pair (UT_uint32 index, const UT_UTF8String *& key, const UT_UTF8String *& value) const
{
	const UT_GenericBase * generic_value = 0;

	bool found = UT_GenericUTF8Hash::pair (index, key, generic_value);

	value = static_cast<const UT_UTF8String *>(generic_value);
	return found;
}

bool UT_UTF8Hash::ins (const UT_UTF8String & key, const UT_UTF8String & value)
{
	if (m_bStripEmptyValues)
		if (value.byteLength () == 0)
			{
				del (key);
				return true;
			}

	UT_UTF8String * utf8_value = 0;
	UT_TRY
		{
			utf8_value = new UT_UTF8String(value);
		}
	UT_CATCH(...)
		{
			utf8_value = 0;
		}
	if (utf8_value == 0) return false;

	return ins (key, utf8_value);
}

bool UT_UTF8Hash::ins (const char * key, const char * value)
{
	if (value == 0)
		return false;

	if (m_bStripEmptyValues)
		if (*value == 0)
			{
				del (key);
				return true;
			}

	UT_UTF8String utf8_key(key);

	UT_UTF8String * utf8_value = 0;
	UT_TRY
		{
			utf8_value = new UT_UTF8String(value);
		}
	UT_CATCH(...)
		{
			utf8_value = 0;
		}
	if (utf8_value == 0) return false;

	return ins (utf8_key, utf8_value);
}

bool UT_UTF8Hash::ins (const char ** attrs)
{
	bool okay = true;

	UT_UTF8String utf8_key;

	while (*attrs)
		{
			utf8_key = *attrs++;

			const char * value = *attrs++;

			if (value == 0) continue;

			if (m_bStripEmptyValues)
				if (*value == 0)
					{
						del (utf8_key);
						continue;
					}

			UT_UTF8String * utf8_value = 0;
			UT_TRY
				{
					utf8_value = new UT_UTF8String(value);
				}
			UT_CATCH(...)
				{
					utf8_value = 0;
				}
			if (utf8_value == 0)
				{
					okay = false;
					break;
				}
			if (!ins (utf8_key, utf8_value))
				{
					okay = false;
					break;
				}
		}
	return okay;
}

/* return value rather than deleting
 */
bool UT_UTF8Hash::del (const char * key, UT_UTF8String *& value)
{
	UT_GenericBase * generic_value = 0;

	bool found = UT_GenericUTF8Hash::del (key, generic_value);

	value = static_cast<UT_UTF8String *>(generic_value);
	return found;
}

/* return value rather than deleting
 */
bool UT_UTF8Hash::del (const UT_UTF8String & key, UT_UTF8String *& value)
{
	UT_GenericBase * generic_value = 0;

	bool found = UT_GenericUTF8Hash::del (key, generic_value);

	value = static_cast<UT_UTF8String *>(generic_value);
	return found;
}

static bool key_lt (const char * key, UT_uint32 key_length, const UT_UTF8String & key2)
{
	UT_uint32 length = key2.byteLength ();
	
	if (length > key_length) {
		length = 1 + length;
	} else {
		length = 1 + key_length;
	}

	return (memcmp (key, key2.utf8_str (), length) < 0);
}

static bool key_gt (const char * key, UT_uint32 key_length, const UT_UTF8String & key2)
{
	UT_uint32 length = key2.byteLength ();
	
	if (length > key_length) {
		length = 1 + length;
	} else {
		length = 1 + key_length;
	}

	return (memcmp (key, key2.utf8_str (), length) > 0);
}

static bool key_eq (const char * key, UT_uint32 key_length, const UT_UTF8String & key2)
{
	if (key_length != key2.byteLength ())
		return false;
	if (key_length == 0)
		return true;

	return (memcmp (key, key2.utf8_str (), key_length) == 0);
}

static void s_pass_whitespace (const char *& csstr)
{
	while (*csstr)
		{
			unsigned char u = static_cast<unsigned char>(*csstr);
			if (u & 0x80)
				{
					UT_UTF8Stringbuf::UCS4Char ucs4 = UT_UTF8Stringbuf::charCode (csstr);

					if (UT_UCS4_isspace (ucs4))
						{
							while (static_cast<unsigned char>(*++csstr) & 0x80) { }
							continue;
						}
				}
			else if (isspace (static_cast<int>(u)))
				{
					csstr++;
					continue;
				}
			break;
		}
}

static const char * s_pass_name (const char *& csstr, char end)
{
	const char * name_end = csstr;

	while (*csstr)
		{
			unsigned char u = static_cast<unsigned char>(*csstr);
			if (u & 0x80)
				{
					UT_UTF8Stringbuf::UCS4Char ucs4 = UT_UTF8Stringbuf::charCode (csstr);
					if (UT_UCS4_isspace (ucs4))
						{
							name_end = csstr;
							break;
						}
					while (static_cast<unsigned char>(*++csstr) & 0x80) { }
					continue;
				}
			else if ((isspace (static_cast<int>(u))) || (*csstr == end))
				{
					name_end = csstr;
					break;
				}
			csstr++;
		}
	return name_end;
}

static const char * s_pass_value (const char *& csstr)
{
	const char * value_end = csstr;

	bool bQuoted = false;
	while (*csstr)
		{
			bool bSpace = false;
			unsigned char u = static_cast<unsigned char>(*csstr);
			if (u & 0x80)
				{
					UT_UTF8Stringbuf::UCS4Char ucs4 = UT_UTF8Stringbuf::charCode (csstr);

					if (!bQuoted)
						if (UT_UCS4_isspace (ucs4))
							{
								bSpace = true;
								break;
							}
					while (static_cast<unsigned char>(*++csstr) & 0x80) { }
					if (!bSpace) value_end = csstr;
					continue;
				}
			else if ((*csstr == '\'') || (*csstr == '"'))
				{
					bQuoted = (bQuoted ? false : true);
				}
			else if (*csstr == ';')
				{
					if (!bQuoted)
						{
							csstr++;
							break;
						}
				}
			else if (!bQuoted && isspace (static_cast<int>(u))) bSpace = true;

			csstr++;
			if (!bSpace) value_end = csstr;
		}
	return value_end;
}

static const char * s_pass_string (const char *& csstr_ptr)
{
	if (*csstr_ptr == 0) return 0;

	const char * csstr = csstr_ptr;

	char quote = 0;

	if ((*csstr == '\'') || (*csstr == '"')) quote = *csstr;

	bool valid = true;
	bool skip = false;

	while (true)
		{
			unsigned char u = static_cast<unsigned char>(*++csstr);

			if ((u & 0xc0) == 0x80) continue; // trailing byte
			if (u == 0)
				{
					valid = false;
					break;
				}
			if (skip)
				{
					skip = false;
					continue;
				}
			if (*csstr == quote)
				{
					++csstr;
					break;
				}
			if (*csstr == '\\') skip = true;
		}
	if (valid)
		{
			csstr_ptr = csstr;
			csstr--;
		}
	else
		{
			csstr = csstr_ptr;
		}
	return csstr; // points to end quote on success, and to start quote on failure
}

void UT_UTF8Hash::parse_properties (const char * properties)
{
	if ( properties == 0) return;
	if (*properties == 0) return;

	const char * csstr = properties;

	UT_UTF8String name;
	UT_UTF8String value;

	bool bSkip = false;

	while (*csstr)
		{
			if (bSkip)
				{
					if (*csstr == ';')
						bSkip = false;
					++csstr;
					continue;
				}
			s_pass_whitespace (csstr);

			const char * name_start = csstr;
			const char * name_end   = s_pass_name (csstr, ':');

			if (*csstr == 0) break; // whatever we have, it's not a "name:value;" pair
			if (name_start == name_end) // ?? stray colon?
				{
					bSkip = true;
					continue;
				}
			name.assign (name_start, name_end - name_start);

			s_pass_whitespace (csstr);
			if (*csstr != ':') // whatever we have, it's not a "name:value;" pair
				{
					bSkip = true;
					continue;
				}

			csstr++;
			s_pass_whitespace (csstr);

			if (*csstr == 0) break; // whatever we have, it's not a "name:value;" pair

			const char * value_start = csstr;
			const char * value_end   = s_pass_value (csstr);

			if (value_start == value_end) // ?? no value...
				{
					bSkip = true;
					continue;
				}
			value.assign (value_start, value_end - value_start);

			ins (name, value);
		}
}

void UT_UTF8Hash::parse_attributes (const char * attributes)
{
	if ( attributes == 0) return;
	if (*attributes == 0) return;

	const char * atstr = attributes;

	UT_UTF8String name;
	UT_UTF8String value;

	while (*atstr)
		{
			s_pass_whitespace (atstr);

			const char * name_start = atstr;
			const char * name_end   = s_pass_name (atstr, '=');

			if (*atstr != '=') break; // whatever we have, it's not a name="value" pair
			if (name_start == name_end) break; // ?? stray equals?

			name.assign (name_start, name_end - name_start);

			atstr++;

			if ((*atstr != '\'') && (*atstr != '"')) break; // whatever we have, it's not a name="value" pair

			const char * value_start = atstr;
			const char * value_end   = s_pass_string (atstr);

			if (value_start == value_end) break; // ?? no value...

			value_start++;

			value.assign (value_start, value_end - value_start);

			ins (name, value);
		}
}

bool operator== (const UT_UTF8Hash & lhs, const UT_UTF8Hash & rhs)
{
	if (lhs.count () != rhs.count ())
		return false;

	const UT_UTF8String * lhs_key   = 0;
	const UT_UTF8String * lhs_value = 0;
	const UT_UTF8String * rhs_key   = 0;
	const UT_UTF8String * rhs_value = 0;

	bool equal = true;

	UT_uint32 count = lhs.count ();

	for (UT_uint32 i = 0; i < count; i++)
		{
			lhs.pair (i, lhs_key, lhs_value);
			rhs.pair (i, rhs_key, rhs_value);

			if ((*lhs_key != *rhs_key) || (*lhs_value != *rhs_value))
				{
					equal = false;
					break;
				}
		}
	return equal;
}
