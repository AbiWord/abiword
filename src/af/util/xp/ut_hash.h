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

class UT_HashTable
{
public:
	UT_HashTable(size_t expected_cardinality = 11);
	~UT_HashTable();

	// our key/value pair types
	typedef char *HashKeyType;
	typedef void *HashValType;

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
	
	UT_Vector * enumerate (void) const;
	
	// these are synonyms - for getting the # keys
	inline size_t cardinality() const { return n_keys; }
	inline size_t size() const { return n_keys; }

	// Like a std cursor
	class UT_HashCursor
	{
		friend class UT_HashTable;
		
	public:
		UT_HashCursor(const UT_HashTable * owner)
			:	m_d(owner), m_index(-1)
			{
				//m_d._first(this);
			}
		
		~UT_HashCursor() { }
		
		// these can't be const since we're passing a non-const this ptr
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
		
	private:
		
		inline void	_set_index(int i)	
			{ m_index = i; }
		inline int	_get_index()		
			{ return m_index; }
		
		const UT_HashTable	* m_d;
		int		m_index;
	};
	
private:

	enum _CM_search_type
	{
		_CM_INSERT = 0,
		_CM_LOOKUP = 1,
		_CM_REORG = 2
	};

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

	// enumeration of the elements
	const HashValType _first(UT_HashCursor* c) const;
	const HashValType _next(UT_HashCursor* c) const;
	const HashValType _prev(UT_HashCursor* c) const;
	const HashKeyType _key(UT_HashCursor* c) const;
	
	// data
	hash_slot* m_pMapping;
	
	size_t n_keys;
	size_t n_deleted;
	size_t m_nSlots;
	size_t reorg_threshold;
	size_t flags;
};


#define UT_HASH_PURGEDATA(type, hash, reaper) \
do { UT_HashTable::UT_HashCursor _hc1 (hash); \
type _hval1 = (type) _hc1.first(); \
while (true) { \
   if (_hval1) \
     reaper (_hval1);\
   if (!_hc1.more()) \
     break; \
   _hval1 = (type) _hc1.next (); \
} } while (0);

#endif /* UT_HASH_H */
