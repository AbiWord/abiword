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
#include "ut_string_class.h"

/* Four hash maps are defined here:
 * 
 * 1. UT_IntStrMap
 *    Maps integers to UTF-8 strings, e.g. map[1] == "one"
 *    
 * 2. UT_NumberMap
 *    Maps UTF-8 strings to integers, e.g. map["one"] == 1
 *    
 * 3. UT_GenericUTF8Hash
 *    An alternative to UT_StringPtrMap (defined in ut_hash.h) but restricted to UTF-8 string keys;
 *    one important difference is that UT_GenericUTF8Hash automatically free()s/deletes its contents.
 *    Can't be instantiated, but isn't abstract. Designed for easy subclassing.
 *    
 *    NOTE: UT_GenericBase is declared in ut_string_class.h
 *    
 * 4. UT_UTF8Hash
 *    Useful subclass of UT_GenericUTF8Hash which maps UTF-8 strings to UTF-8 strings, e.g. map["one"] == "1"
 */

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

class ABI_EXPORT UT_NumberMap
{
private:
	struct NumberStr
	{
		UT_UTF8String *	key;
		UT_sint32		value;
	};
public:
	/* operator[] must return something, even if key not found, so what?
	 * set default return value in constructor
	 */
	UT_NumberMap (UT_sint32 default_value = -1, UT_uint32 increment = 32);

	~UT_NumberMap ();

	void clear ();

	bool ins (const UT_UTF8String & key, UT_sint32 value);

	/* returns false if no such key-value
	 */
	bool del (const char * key);

	inline UT_sint32 operator[] (const char * key)
	{
		UT_uint32 index;
		return lookup (key, index) ? m_pair[index].value : m_default_value;
	}
	inline UT_sint32 operator[] (const UT_UTF8String & key)
	{
		UT_uint32 index;
		return lookup (key, index) ? m_pair[index].value : m_default_value;
	}

private:
	bool lookup (const char *          key, UT_uint32 & index);
	bool lookup (const UT_UTF8String & key, UT_uint32 & index);

	bool lookup (const char * key, UT_uint32 key_length, UT_uint32 & index);

	bool grow ();

	NumberStr *	m_pair;

	UT_uint32	m_pair_count;
	UT_uint32	m_pair_max;

	UT_uint32	m_index;
	UT_uint32	m_increment;

	UT_sint32	m_default_value;

public:
	inline bool pair (UT_uint32 index, const UT_UTF8String *& key, UT_sint32 & value) const
	{
		if (index >= m_pair_count) return false;
		key   = m_pair[index].key;
		value = m_pair[index].value;
		return true;
	}
	UT_uint32 count () const { return m_pair_count; }
};

class ABI_EXPORT UT_GenericUTF8Hash
{
private:
	class ABI_EXPORT KeyValue
	{
	private:
		UT_UTF8String		m_key;
		UT_GenericBase *	m_value;

	public:
		KeyValue (const UT_UTF8String & key);

		~KeyValue ();

		/* responsibility for value passes here
		 */
		void setValue (UT_GenericBase * value);

		/* and passes back here
		 */
		UT_GenericBase * getValue ();

		const UT_UTF8String &  key ()   const { return m_key;   }
		const UT_GenericBase * value () const { return m_value; }
	};

	// disable these:
	UT_GenericUTF8Hash (const UT_GenericUTF8Hash & rhs);
	UT_GenericUTF8Hash & operator= (const UT_GenericUTF8Hash & rhs);
protected:
	UT_GenericUTF8Hash (UT_uint32 increment);
public:
	/* the destructor calls clear(false);
	 * subclasses' destructors should call clear(true) if values need to be deleted
	 */
	virtual ~UT_GenericUTF8Hash ();

	inline UT_uint32 count () const { return m_pair_count; }

protected:
	/* deletes all key/value pairs, but doesn't free() array of pointers
	 */
	void clear (bool delete_values);

	/* for easy sequential access of map members:
	 */
	virtual bool pair (UT_uint32 index, const UT_UTF8String *& key, const UT_GenericBase *& value) const;

	/* responsibility for value passes here
	 */
	virtual bool ins (const UT_UTF8String & key, UT_GenericBase * value);

	/* returns false if no such key-value
	 */
	virtual bool del (const char * key);
	virtual bool del (const UT_UTF8String & key);

	/* return value rather than deleting
	 */
	virtual bool del (const char * key, UT_GenericBase *& value);
	virtual bool del (const UT_UTF8String & key, UT_GenericBase *& value);

	virtual const UT_GenericBase * lookup (const char * key);
	virtual const UT_GenericBase * lookup (const UT_UTF8String & key);

private:
	bool lookup (const char *          key, UT_uint32 & index);
	bool lookup (const UT_UTF8String & key, UT_uint32 & index);

	bool lookup (const char * key, UT_uint32 key_length, UT_uint32 & index);

	bool grow ();

	KeyValue **	m_pair;

	UT_uint32	m_pair_count;
	UT_uint32	m_pair_max;

	UT_uint32	m_index;
	UT_uint32	m_increment;
};

class ABI_EXPORT UT_UTF8Hash : public UT_GenericUTF8Hash
{
public:
	UT_UTF8Hash ();

	~UT_UTF8Hash ();

	inline void clear ()
	{
		UT_GenericUTF8Hash::clear (true);
	}

	/* for easy sequential access of map members:
	 */
	bool pair (UT_uint32 index, const UT_UTF8String *& key, const UT_UTF8String *& value) const;

	/* responsibility for value passes here
	 */
	inline bool ins (const UT_UTF8String & key, UT_UTF8String * value)
	{
		return value ? UT_GenericUTF8Hash::ins (key, value) : false;
	}

	bool ins (const UT_UTF8String & key, const UT_UTF8String & value);
	bool ins (const char *  key, const char * value); // make sure you provide valid UTF-8!
	bool ins (const char ** attrs); // attribute pairs with a 0 value are ignored

	/* returns false if no such key-value
	 */
	inline bool del (const char * key)
	{
		return UT_GenericUTF8Hash::del (key);
	}
	inline bool del (const UT_UTF8String & key)
	{
		return UT_GenericUTF8Hash::del (key);
	}

	/* return value rather than deleting
	 */
	bool del (const char *          key, UT_UTF8String *& value);
	bool del (const UT_UTF8String & key, UT_UTF8String *& value);

	inline const UT_UTF8String * operator[] (const char * key)
	{
		return static_cast<const UT_UTF8String *>(UT_GenericUTF8Hash::lookup (key));
	}
	inline const UT_UTF8String * operator[] (const UT_UTF8String & key)
	{
		return static_cast<const UT_UTF8String *>(UT_GenericUTF8Hash::lookup (key));
	}
};

#endif /* ! UT_INTSTRMAP_H */
