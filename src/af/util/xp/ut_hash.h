/* AbiSource Program Utilities
 *
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2001 Mike Nordell <tamlin@alogonet.se>
 * Copyright (C) 2001 Dom Lachowicz <cinamod@hotmail.com>
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

#ifndef UT_HASH_H
#define UT_HASH_H

#include <string.h>
#include "ut_types.h"
#include "ut_vector.h"

// fwd. decl.
class hash_slot;
class _hash_cursor;

enum _CM_search_type
{
	_CM_INSERT = 0,
	_CM_LOOKUP = 1,
	_CM_REORG = 2
};

// this class is optimized for string keys and pointer values

typedef char *HashKeyType;
typedef void *HashValType;

// the actual hashtable
class UT_HashTable
{
public:
	UT_HashTable(size_t expected_cardinality = 10);
	~UT_HashTable();

	// insertion/addition
	void insert(const HashKeyType key, const HashValType value);

	void set (const HashKeyType key, const HashValType val);

	// "find"
	const HashValType pick(const HashKeyType key) const;
	
	// contains - if contains(key) val will be the result of the lookup
	bool contains(const HashKeyType key, const HashValType val) const;

	// these are for removal
	void remove(const HashKeyType key, const HashValType /* ignored */);
	void clear();
	
	// enumeration of the elements
	const HashValType _first(_hash_cursor* c) const;
	const HashValType _next(_hash_cursor* c) const;
	const HashValType _prev(_hash_cursor* c) const;
	const HashKeyType _key(_hash_cursor* c) const;
	
	UT_Vector * enumerate (void) const;
	
	// these are synonyms - for getting the # keys
	inline size_t cardinality() const { return n_keys; }
	inline size_t size() const { return n_keys; }
	
private:
	void reorg(size_t slots_to_allocate);
	void grow();
	
	void assign_slots(hash_slot* p, size_t old_num_slots);
	
	static size_t compute_reorg_threshold(size_t nslots);
	
	inline bool too_full() const 
		{ return (n_keys + n_deleted) >= reorg_threshold; }
	
	inline bool too_many_deleted() const
		{ return n_deleted > (reorg_threshold / 4); }
	
	inline bool exceeds_n_delete_threshold() const
		{ return n_deleted > (reorg_threshold / 2); }
	
	hash_slot* find_slot(const HashKeyType		k,
						 _CM_search_type search_type,
						 size_t&			slot,
						 bool&			key_found,
						 size_t&			hashval,
						 const HashValType		v,
						 bool*			v_found,
						 void*			vi,
						 size_t			hashval_in) const;
	
	// data
	hash_slot* m_pMapping;
	
	size_t n_keys;
	size_t n_deleted;
	size_t m_nSlots;
	size_t reorg_threshold;
	size_t flags;
};


class _hash_cursor
{
public:
	_hash_cursor(const UT_HashTable * owner)
		:	m_d(owner), m_index(-1)
		{
			//m_d._first(this);
		}
	
	~_hash_cursor() { }
	
	// these can't be const
	inline HashKeyType  key()
		{return m_d->_key(this); }
	inline HashValType	first()
		{ return m_d->_first(this);	}
	inline HashValType	next()
		{ return m_d->_next(this); }
	inline HashValType  prev()
		{ return m_d->_prev(this); }
	inline bool	more()
		{ return (m_index != -1); }
	
	inline void	_set_index(int i)	
		{ m_index = i; }
	inline int	_get_index()		
		{ return m_index; }
	
private:
	const UT_HashTable	* m_d;
	int		m_index;
};


// wrapper class for keys
class key_wrapper
{
public:
	key_wrapper() 
		: m_val(0), m_hashval(0) { }
	~key_wrapper() { }
	
	inline void die() 
		{ m_val = 0;}
	
	inline bool eq(const HashKeyType key) const
	{
			//return m_val == key;
		if (m_val && key)
		{
			return (strcmp(key, m_val) ? false : true);
		}
		return false;
	}
	
	inline void operator=(const HashKeyType k)	
		{ m_val = k; }
	
	inline UT_uint32 hashval() const		
		{ return m_hashval; }
	inline void set_hashval(UT_uint32 h)	
		{ m_hashval = h; }
	
	inline operator const HashKeyType() const	
		{ return m_val; }
	inline HashKeyType value(void) 
		{return m_val;}

	inline void operator=(const key_wrapper& rhs)
		{ m_val = rhs.m_val; m_hashval = rhs.m_hashval; }

	static UT_uint32 compute_hash(const HashKeyType key);

private:
	HashKeyType		m_val;
	UT_uint32 m_hashval;
};


// bucket for data
class hash_slot
{
public:
	hash_slot() 
		: m_value(0) { }
	~hash_slot() { }

	inline void make_deleted()
		{
			m_value = (char*)this;
			m_key.die();
		}
	inline void make_empty() 
		{ m_value = 0; }

	inline HashValType value() const 
		{ return m_value; }

	inline void insert(const HashValType v, const HashKeyType k, UT_uint32 h)
		{
			m_value = v;
			m_key = k;
			m_key.set_hashval(h);
		}

	inline void assign(hash_slot* s) 
		{
			m_value = s->value();
			m_key = s->m_key;
		}

	inline bool empty() const 
		{ return (m_value == 0); }

	inline bool deleted(const HashValType delval) const
		{
			return ((delval ? delval : (void *)this) == (HashValType)m_value);
		}

	inline bool key_eq(const HashKeyType test, size_t h) const
		{
#if 0
			return m_key.eq(test);
#else
			return m_key.hashval() == h;
#endif
		}
	
	HashValType		m_value;
	key_wrapper	m_key;	
};

#define UT_HASH_PURGEDATA(type, hash, reaper) \
do { _hash_cursor _hc1 (hash); \
type _hval1 = (type) _hc1.first(); \
while (true) { \
   if (_hval1) \
     reaper (_hval1);\
   if (!_hc1.more()) \
     break; \
   _hval1 = (type) _hc1.next (); \
} } while (0);

#endif /* UT_HASH_H */
