/* AbiSource Program Utilities
 *
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
#include "ut_string_class.h"

// fwd. decl.
class hash_slot;

class UT_StringPtrMap
{
public:
	UT_StringPtrMap(size_t expected_cardinality = 11);
	~UT_StringPtrMap();

	// insertion/addition
	void insert(const char* key, const void* value);
	void insert(const UT_String & key, const void* value);

	void set (const char* key, const void* val);
	void set (const UT_String & key, const void* val);

	// "find"
	const void* pick(const char* key) const;
	const void* pick(const UT_String & key) const;
	
	// contains - if contains(key) val will be the result of the lookup
	bool contains(const char* key, const void* val) const;
	bool contains(const UT_String & key, const void* val) const;

	// these are for removal
	void remove(const char* key, const void* /* ignored */);
	void remove(const UT_String & key, const void* /* ignored */);
	void clear();
	
	UT_Vector* enumerate() const;
	UT_Vector* keys() const;
	
	// these are synonyms - for getting the # keys
	inline size_t cardinality() const { return n_keys; }
	inline size_t size() const { return n_keys; }

	// Like a std cursor
	class UT_Cursor
	{
		friend class UT_StringPtrMap;
		
	public:
		UT_Cursor(const UT_StringPtrMap * owner)
			:	m_d(owner), m_index(-1)
			{
				//m_d._first(this);
			}
		
		~UT_Cursor() { }
		
		// these can't be const since we're passing a non-const this ptr
		inline const UT_String  &key()
			{return m_d->_key(*this); }
		inline const void*	first()
			{ return m_d->_first(*this); }
		inline const void*	next()
			{ return m_d->_next(*this); }
		inline const void*  prev()
			{ return m_d->_prev(*this); }
		inline bool	is_valid()
			{ return (m_index != -1); }
		
	private:
		
		inline void	_set_index(int i)	
			{ m_index = i; }
		inline int	_get_index()		
			{ return m_index; }
		
		const UT_StringPtrMap*	m_d;
		UT_sint32				m_index;
	};
	friend class UT_Cursor;

private:
	UT_StringPtrMap(const UT_StringPtrMap&);	// no impl
	void operator=(const UT_StringPtrMap&);		// no impl

	enum SM_search_type
	{
		SM_INSERT,
		SM_LOOKUP,
		SM_REORG
	};

	void reorg(size_t slots_to_allocate);
	void grow();
	
	void assign_slots(hash_slot* p, size_t old_num_slots);
	
	static size_t compute_reorg_threshold(size_t nslots);
	
	bool too_full() const 
		{ return (n_keys + n_deleted) >= reorg_threshold; }
	
	bool too_many_deleted() const
		{ return n_deleted > (reorg_threshold / 4); }
	
	bool exceeds_n_delete_threshold() const
		{ return n_deleted > (reorg_threshold / 2); }
	
	hash_slot* find_slot(const UT_String&		k,
					     SM_search_type		search_type,
					     size_t&			slot,
					     bool&				key_found,
					     size_t&			hashval,
					     const void*		v,
					     bool*				v_found,
					     void*				vi,
					     size_t				hashval_in) const;
	
	// enumeration of the elements
	const void* _first(UT_Cursor& c) const;
	const void* _next(UT_Cursor& c) const;
	const void* _prev(UT_Cursor& c) const;
	const UT_String& _key(UT_Cursor& c) const;
	
	// data
	hash_slot* m_pMapping;
	
	size_t n_keys;
	size_t n_deleted;
	size_t m_nSlots;
	size_t reorg_threshold;
	size_t flags;
};


#define UT_HASH_PURGEDATA(type, hash, reaper)		\
	do { UT_StringPtrMap::UT_Cursor _hc1(hash);		\
        for ( type _hval1 = (type) _hc1.first(); _hc1.is_valid(); _hval1 = (type) _hc1.next() ) { \
	   if (_hval1)									\
		 reaper (_hval1);							\
	} } while (0);

#endif /* UT_HASH_H */
