/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef UT_HASH_H
#define UT_HASH_H

#include <string.h>

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

#ifndef UTVECTOR_H
#include "ut_vector.h"
#endif

#include "ut_debugmsg.h"
#include "ut_string_class.h"

#if _MSC_VER >= 1310
// MSVC++ 7.1 warns about debug output limitations.
#pragma warning(disable: 4292)
#endif

// fwd. decl.
template <class T> class hash_slot;

template <class T> class UT_GenericStringMap;

template <class T> class UT_GenericStringMap
{
public:
	UT_GenericStringMap(size_t expected_cardinality = 11);
	virtual ~UT_GenericStringMap();

	// insertion/addition
	bool insert(const char* key, T value);
	bool insert(const UT_String & key, T value);

	void set (const char* key, T val);
	void set (const UT_String & key, T val);

	// "find"
	T pick(const char* key) const;
	T pick(const UT_String & key) const;

	// contains - if contains(key) val will be the result of the lookup
	bool contains(const char* key, T val) const;
	bool contains(const UT_String & key, T val) const;

	// these are for removal
	void remove(const char* key, T /* ignored */);
	void remove(const UT_String & key, T /* ignored */);
	void clear();

	/* IMPORTANT: list() is for use only with <XML_C/char*> maps
	 */
	const gchar ** list ();

	UT_GenericVector<T>* enumerate(bool strip_null_values = true) const;
	UT_GenericVector<const UT_String*>* keys(bool strip_null_values = true) const;

	// getting the # keys
	inline size_t size() const { return n_keys; }

	class UT_Cursor
	{
		friend class UT_GenericStringMap<T>;

	public:
		UT_Cursor(const UT_GenericStringMap<T> * owner)
			:	m_d(owner), m_index(-1)
			{
				//m_d._first(this);
			}

		~UT_Cursor() { }

		// these can't be const since we're passing a non-const this ptr
		inline const UT_String  &key()
			{return m_d->_key(*this); }
		inline void make_deleted()
			{m_d->_make_deleted(*this); }
		inline const T	first()
			{ return m_d->_first(*this); }
		inline const T	next()
			{ return m_d->_next(*this); }
		inline const T  prev()
			{ return m_d->_prev(*this); }
		inline bool	is_valid()
			{ return (m_index != -1); }

	private:

		inline void	_set_index(UT_sint32 i)
			{ m_index = i; }
		inline UT_sint32 _get_index()
			{ return m_index; }

		const UT_GenericStringMap<T>*	m_d;
		UT_sint32				m_index;
	};

	friend class UT_Cursor;

	/* purge objects by deleting them */
	void purgeData(void)
		{
			UT_Cursor hc1(this);
			for ( T hval1 = hc1.first(); hc1.is_valid(); hval1 = hc1.next() ) {
				if (hval1) {
					hc1.make_deleted();
					delete hval1;
				}
			}
		};

    /* purge objects by freeing them */
	void freeData(void)
		{
			UT_Cursor hc1(this);
			for ( T hval1 = hc1.first(); hc1.is_valid(); hval1 = hc1.next() ) {
				if (hval1) {
					hc1.make_deleted();
					g_free((gpointer)(hval1));
				}
			}
		}

private:
	UT_GenericStringMap(const UT_GenericStringMap<T>&);	// no impl
	void operator=(const UT_GenericStringMap<T>&);		// no impl


	enum SM_search_type
	{
		SM_INSERT,
		SM_LOOKUP,
		SM_REORG
	};

	void reorg(size_t slots_to_allocate);
	void grow();

	void assign_slots(hash_slot<T>* p, size_t old_num_slots);

	static size_t compute_reorg_threshold(size_t nslots);

	bool too_full() const
		{ return (n_keys + n_deleted) >= reorg_threshold; }

	bool too_many_deleted() const
		{ return n_deleted > (reorg_threshold / 4); }

	bool exceeds_n_delete_threshold() const
		{ return n_deleted > (reorg_threshold / 2); }

	hash_slot<T>* find_slot(const UT_String&		k,
					     SM_search_type		search_type,
					     size_t&			slot,
					     bool&				key_found,
					     size_t&			hashval,
					     const void*		v,
					     bool*				v_found,
					     void*				vi,
					     size_t				hashval_in) const;

	hash_slot<T>* find_slot(const char *		k,
					     SM_search_type		search_type,
					     size_t&			slot,
					     bool&				key_found,
					     size_t&			hashval,
					     const void*		v,
					     bool*				v_found,
					     void*				vi,
					     size_t				hashval_in) const;

	// enumeration of the elements
	const T _first(UT_Cursor& c) const;
	const T _next(UT_Cursor& c) const;
	const T _prev(UT_Cursor& c) const;
	const UT_String& _key(UT_Cursor& c) const;
	void _make_deleted(UT_Cursor& c) const;

	// data
	hash_slot<T>* m_pMapping;

	size_t n_keys;
	size_t n_deleted;
	size_t m_nSlots;
	size_t reorg_threshold;
	size_t flags;

	gchar ** m_list;
};

#if 0 //def _MSC_VER // have to intialise the templates in order to have class exported

struct _dataItemPair;
template class ABI_EXPORT UT_GenericStringMap<struct _dataItemPair*>;

class UT_UTF8String;
template class ABI_EXPORT UT_GenericStringMap<UT_UTF8String *>;

class PD_Style;
template class ABI_EXPORT UT_GenericStringMap<PD_Style *>;

class GR_Font;
template class ABI_EXPORT UT_GenericStringMap<GR_Font *>;

template class ABI_EXPORT UT_GenericStringMap<char *>;

//template class ABI_EXPORT UT_GenericStringMap<void const*>;
#endif

// TODO Rob: try to export like this once plugin loading is fixed:
// template class ABI_EXPORT UT_GenericStringMap<void const *>;
class ABI_EXPORT UT_StringPtrMap : public UT_GenericStringMap<void const *> {
public:
	UT_StringPtrMap(size_t expected_cardinality = 11)
	: UT_GenericStringMap<void const *>(expected_cardinality)
	{}
};

// Template implementation

// fwd. decls.
ABI_EXPORT UT_uint32 _Recommended_hash_size(UT_uint32	size);


// wrapper class for keys
class ABI_EXPORT key_wrapper
{
public:
	key_wrapper()
		: m_hashval(0) { }

	void die()
		{ m_val.clear(); }

	bool eq(const UT_String &key) const
	{
		return (m_val == key);
	}

	bool eq(const char *key) const
	{
		return (!strcmp(m_val.c_str(),key));
	}

	void operator=(const UT_String &k)
		{ m_val = k; }

	UT_uint32 hashval() const
		{ return m_hashval; }
	void set_hashval(UT_uint32 h)
		{ m_hashval = h; }

	UT_String &value(void)
		{return m_val;}

	void operator=(const key_wrapper& rhs)
		{ m_val = rhs.m_val; m_hashval = rhs.m_hashval; }

	static UT_uint32 compute_hash(const UT_String &key)
		{
			return hashcode(key); // UT_String::hashcode
		}

	static UT_uint32 compute_hash(const char *key)
		{
			return hashcode(key);
		}

private:
	UT_String m_val;
	UT_uint32 m_hashval;
};




// bucket for data
template <class T> class hash_slot
{
public:
	hash_slot()
		: m_value(0) { }

	void make_deleted()
		{
			// this is a HACK: if the value of the slot is this, then slot is empty.
			m_value = reinterpret_cast<T>(this);
			m_key.die();
		}
	void make_empty()
		{ m_value = 0; }

	const T value() const
		{ return m_value; }

	void insert(const T v, const UT_String &k, UT_uint32 h)
		{
			m_value = v;
			m_key = k;
			m_key.set_hashval(h);
		}

	void assign(hash_slot<T>* s)
		{
			m_value = s->value();
			m_key = s->m_key;
		}

	bool empty() const
		{ return (m_value == 0); }

	bool deleted() const
		{
			return static_cast<const void*>(this) == m_value;
		}

	bool key_eq(const UT_String &test, size_t h) const
		{
#if 1
			UT_UNUSED(h);
			return m_key.eq(test);
#else
			return m_key.hashval() == h;
#endif
		}

	bool key_eq(const char  *test, size_t h) const
		{
#if 1
			UT_UNUSED(h);
			return m_key.eq(test);
#else
			return m_key.hashval() == h;
#endif
		}

	T	m_value;
	key_wrapper	m_key;
};

/*!
 * This class represents a mapping between key/value pairs where the keys are
 * represented by UT_String (a wrapper around char*) and the values may be of
 * any pointer type (void*)
 */
template <class T>
UT_GenericStringMap<T>::UT_GenericStringMap(size_t expected_cardinality)
:	n_keys(0),
	n_deleted(0),
	m_nSlots(_Recommended_hash_size(expected_cardinality)),
	reorg_threshold(compute_reorg_threshold(m_nSlots)),
	flags(0),
	m_list(0)
{
	m_pMapping = new hash_slot<T>[m_nSlots];
}


template <class T>
UT_GenericStringMap<T>::~UT_GenericStringMap()
{
	DELETEPV(m_pMapping);
	FREEP(m_list);
}

/* IMPORTANT: for use only with <char*> -> <char*> maps
   TODO: make this a specialized method.
 */
template <class T>
const gchar ** UT_GenericStringMap<T>::list()
{
	if (!m_list)
	{
		m_list = reinterpret_cast<gchar **>(g_try_malloc (2 * (n_keys + 1) * sizeof (gchar *)));
		if (m_list == 0)
			return 0;

		UT_uint32 index = 0;

		UT_Cursor c(this);

		for (const gchar * value = (gchar*)(c.first ());
			 c.is_valid ();
			 value = (gchar*)(c.next ()))
		{
			const char * key = c.key().c_str ();

			if (!key || !value)
				continue;

			m_list[index++] = static_cast<gchar *>(const_cast<char *>(key));
			m_list[index++] = static_cast<gchar *>(const_cast<char *>(value));
		}
		m_list[index++] = NULL;
		m_list[index  ] = NULL;
	}
	return const_cast<const gchar **>(m_list);
}

/*!
 * Find the value associated with the key \k
 * \return 0 if key not found, object if found
 */
template <class T>
T UT_GenericStringMap<T>::pick(const char* k) const
{
	hash_slot<T>*		sl = 0;
	bool			key_found = false;
	size_t			slot;
	size_t			hashval;

	sl = find_slot(k, SM_LOOKUP, slot, key_found, hashval, 0, 0, 0, 0);
	return key_found ? sl->value() : 0;
}

template <class T>
T UT_GenericStringMap<T>::pick(const UT_String & k) const
{
  return pick (k.c_str());
}

/*!
 * See if the map contains the (key, value) pair represented by (\k, \v)
 * If \v is null, just see if the key \k exists
 * \return truth
 */
template <class T>
bool UT_GenericStringMap<T>::contains(const char* k, T v) const
{
  UT_String aKey(k);
  return contains (aKey, v);
}

template <class T>
bool UT_GenericStringMap<T>::contains(const UT_String& k, T v) const
{
	//	hash_slot<T> * sl;
	bool key_found = false;
	bool v_found   = false;
	size_t slot    = 0;
	size_t hashval = 0;

	// DOM: TODO: make this call work
	/*sl =*/ find_slot (k, SM_LOOKUP, slot, key_found,
			hashval, v, &v_found, 0, 0);
	return v_found;
}


/*!
 * Insert this key/value pair into the map
 */
template <class T>
bool UT_GenericStringMap<T>::insert(const char* key, T value)
{
  UT_String aKey(key);
  return insert (aKey, value);
}

template <class T>
bool UT_GenericStringMap<T>::insert(const UT_String& key, T value)
{
	FREEP(m_list);

	size_t		slot = 0;
	bool		key_found = false;
	size_t		hashval = 0;

	hash_slot<T>* sl = find_slot(key, SM_INSERT, slot, key_found,
				  hashval, 0, 0, 0, 0);

	if(key_found)
		return false;

	sl->insert(value, key, hashval);
	++n_keys;

	if (too_full())
	{
		if (too_many_deleted())
		{
			reorg(m_nSlots);
		}
		else
		{
			grow();
		}
	}

	return true;
}

/*!
 * Set the item determined by \key to the value \value
 * If item(\key) does not exist, insert it into the map
 */
template <class T>
void UT_GenericStringMap<T>::set(const char* key, T value)
{
	UT_String aKey(key);
	set (aKey, value);
}

template <class T>
void UT_GenericStringMap<T>::set(const UT_String& key, T value)
{
	FREEP(m_list);

	size_t		slot = 0;
	bool		key_found = false;
	size_t		hashval = 0;

	hash_slot<T>* sl = find_slot(key, SM_LOOKUP, slot, key_found,
							  hashval, 0, 0, 0, 0);

	if (!sl || !key_found) // TODO: should we insert or just return?
	{
		insert(key, value);
		return;

	}

	sl->insert(value, key, hashval);
}

/*!
 * Return a UT_Vector of elements in the HashTable that you must
 * Later g_free with a call to delete
 */
template <class T>
UT_GenericVector<T>* UT_GenericStringMap<T>::enumerate (bool strip_null_values) const
{
	UT_GenericVector<T> * pVec = new UT_GenericVector<T>(size());

	UT_Cursor cursor(this);

	T val = NULL;

	for (val = cursor.first(); cursor.is_valid(); val = cursor.next ())
	{
		// we don't allow nulls since so much of our code depends on this
		// behavior
		if (!strip_null_values || val)
		{
			pVec->addItem (val);
		}
	}

	return pVec;
}

/*!
 * Return a UT_Vector of pointers to our UT_String keys in the Hashtable
 * You must FREEP the UT_Vector* but not the keys
 */
template <class T>
UT_GenericVector<const UT_String*>* UT_GenericStringMap<T>::keys (bool strip_null_values) const
{
	UT_GenericVector<const UT_String*>* pVec = new UT_GenericVector<const UT_String*>(size());

	UT_Cursor cursor(this);

	T val = NULL;

	for (val = cursor.first(); cursor.is_valid(); val = cursor.next ())
	{
		// we don't allow nulls since so much of our code depends on this
		// behavior
		if (!strip_null_values || val)
		{
			pVec->addItem (&cursor.key());
		}
	}

	return pVec;
}


/*!
 * Remove the item referenced by \key in the map
 */
template <class T>
void UT_GenericStringMap<T>::remove(const char* key, T)
{
	UT_String aKey(key);
	remove (aKey, 0);
}

template <class T>
void UT_GenericStringMap<T>::remove(const UT_String& key, T)
{
	FREEP(m_list);

	size_t slot = 0, hashval;
	bool bFound = false;
	hash_slot<T>* sl = find_slot(key, SM_LOOKUP, slot, bFound,
							  hashval, 0, 0, 0, 0);

	if (bFound)
	{
		sl->make_deleted();
		--n_keys;
		++n_deleted;
		if (m_nSlots > 11 && m_nSlots / 4 >= n_keys)
		{
			reorg(_Recommended_hash_size(m_nSlots/2));
		}
	}
}

/*!
 * Remove all key/value pairs from the map
 */
template <class T>
void UT_GenericStringMap<T>::clear()
{
	FREEP(m_list);

	hash_slot<T>* the_slots = m_pMapping;
	for (size_t x=0; x < m_nSlots; x++)
	{
		hash_slot<T>& this_slot = the_slots[x];
		if (!this_slot.empty())
		{
			if (!this_slot.deleted())
			{
				this_slot.make_deleted();
			}
			this_slot.make_empty();
		}
	}
	n_keys = 0;
	n_deleted = 0;
}


/*********************************************************************/
/*********************************************************************/

template <class T>
void UT_GenericStringMap<T>::assign_slots(hash_slot<T>* p, size_t old_num_slot)
{
	size_t target_slot = 0;

	for (size_t slot_num=0; slot_num < old_num_slot; ++slot_num, ++p)
	{
		if (!p->empty() && !p->deleted())
		{
			bool kf = false;

			size_t hv;
			hash_slot<T>* sl = find_slot(p->m_key.value(),
									  SM_REORG,
									  target_slot,
									  kf,
									  hv,
									  0,
									  0,
									  NULL,
									  p->m_key.hashval());
			sl->assign(p);
		}
	}
}

template <class T>
size_t UT_GenericStringMap<T>::compute_reorg_threshold(size_t nSlots)
{
	return nSlots * 7 / 10;	// reorg threshold = 70% of nSlots
}


template <class T> hash_slot<T>*
UT_GenericStringMap<T>::find_slot(const UT_String& k,
							SM_search_type	search_type,
							size_t&			slot,
							bool&			key_found,
							size_t&			hashval,
							const void*		v,
							bool*			v_found,
							void*			vi,
							size_t			hashval_in) const
{
 return find_slot( k.c_str(), search_type, slot, key_found, hashval, v, v_found, vi, hashval_in);
}

template <class T> hash_slot<T>*
UT_GenericStringMap<T>::find_slot(const char *k,
							SM_search_type	search_type,
							size_t&			slot,
							bool&			key_found,
							size_t&			hashval,
							const void*		v,
							bool*			v_found,
							void*			/*vi*/,
							size_t			hashval_in) const
{
	if ( m_nSlots == 0 )
	  {
	    key_found = false ; return NULL ;
	  }

	hashval = (hashval_in ? hashval_in : key_wrapper::compute_hash(k));
	int nSlot = hashval % m_nSlots;

	xxx_UT_DEBUGMSG(("DOM: hashval for \"%s\" is %d (#%dth slot)\n", k, hashval, nSlot));

	hash_slot<T>* sl = &m_pMapping[nSlot];

	if (sl->empty())
	{

		xxx_UT_DEBUGMSG(("DOM: empty slot\n"));

		slot = nSlot;
		key_found = false;
		return sl;
	}
	else
	{
		if (search_type != SM_REORG &&
			!sl->deleted() &&
			sl->key_eq(k, hashval))
	    {
			slot = nSlot;
			key_found = true;

			if (v_found)
			{
				// so, if v_found is non-null, we should set it.
				// if v is also non-null, sl->value() must match v
				// otherwise we already have a key match, so we win!
				if (v)
				{
					*v_found = (sl->value() == v);
				} else {
					*v_found = true;
				}
			}

			xxx_UT_DEBUGMSG(("DOM: found something #1\n"));

			return sl;
	    }
	}

	int delta = (nSlot ? (m_nSlots - nSlot) : 1);
	hash_slot<T>* tmp_sl = sl;
	sl = 0;
	size_t s = 0;
	key_found = false;

	while (1)
	{
		nSlot -= delta;
		if (nSlot < 0)
		{
			nSlot += m_nSlots;
			tmp_sl += (m_nSlots - delta);
		}
		else
		{
			tmp_sl -= delta;
		}

		if (tmp_sl->empty())
		{
			if (!s)
			{
				s = nSlot;
				sl = tmp_sl;
			}
			break;

		}

		if (tmp_sl->deleted())
		{
			if (!s)
			{
				s = nSlot;
				sl = tmp_sl;
			}
		}
		else if (search_type != SM_REORG && tmp_sl->key_eq(k, hashval))
		{
			s = nSlot;
			sl = tmp_sl;
			key_found = true;

			if (v_found)
			{
				if (v)
				{
					*v_found = (sl->value() == v);
				} else {
					*v_found = true;
				}
			}
			break;
		}
	}

	slot = s;
	return sl;
}


template <class T>
void UT_GenericStringMap<T>::grow()
{
	size_t slots_to_allocate = ::_Recommended_hash_size(m_nSlots / 2 + m_nSlots);
	reorg(slots_to_allocate);
}


template <class T>
void UT_GenericStringMap<T>::reorg(size_t slots_to_allocate)
{
	hash_slot<T>* pOld = m_pMapping;

	if (slots_to_allocate < 11)
	{
		slots_to_allocate = 11;
	}

	m_pMapping = new hash_slot<T>[slots_to_allocate];

	const size_t old_num_slot = m_nSlots;

	m_nSlots = slots_to_allocate;
	reorg_threshold = compute_reorg_threshold(m_nSlots);

	assign_slots(pOld, old_num_slot);
	DELETEPV(pOld);

	n_deleted = 0;
}


template <class T> const T
UT_GenericStringMap<T>::_first(UT_Cursor& c) const
{
	const hash_slot<T>* map = m_pMapping;
	size_t x;
	for (x=0; x < m_nSlots; ++x)
	{
		if (!map[x].empty() && !map[x].deleted())
		{
			break;
		}
	}
	if (x < m_nSlots)
	{
		c._set_index(x);	// c = 'UT_Cursor etc'
		return map[x].value();
	}

	c._set_index(-1);
	return 0;
}

template <class T>
const UT_String& UT_GenericStringMap<T>::_key(UT_Cursor& c) const
{
	hash_slot<T> & slot = m_pMapping[c._get_index()];

	// we used to return a reference to a static variable here in case the
	// following conditions failed, but that breaks some compilers (the
	// variable has to be instantiated somewhere for each template instance
	// and this can lead to multiple instances of it in different abi
	// modules (for example with the cs2005q3.2 compiler for Maemo 2) --
	// the caller must ensure that the cursor is valid; that is not that
	// much to ask
	UT_ASSERT_HARMLESS(!slot.empty() && !slot.deleted());

	return slot.m_key.value();
}

template <class T>
void UT_GenericStringMap<T>::_make_deleted(UT_Cursor& c) const
{
	hash_slot<T> & slot = m_pMapping[c._get_index()];

	if (!slot.empty() && !slot.deleted())
	{
		slot.make_deleted();
	}
}

template <class T> const T
UT_GenericStringMap<T>::_next(UT_Cursor& c) const
{
	const hash_slot<T>* map = m_pMapping;
	size_t x;
	for (x = c._get_index() + 1; x < m_nSlots; ++x)
	{
		if (!map[x].empty() && !map[x].deleted())
		{
			break;
		}
	}
	if (x < m_nSlots)
	{
		c._set_index(x);
		return map[x].value();
	}

	c._set_index(-1);
	return 0;
}


template <class T> const T
UT_GenericStringMap<T>::_prev(UT_Cursor& c) const
{
	const hash_slot<T>* map = m_pMapping;
	UT_sint32 x;
	for (x = c._get_index() - 1; x >= 0; --x)
	{
		if (!map[x].empty() && !map[x].deleted())
		{
			break;
		}
	}
	if (x >= 0)
	{
		c._set_index(x);
		return map[x].value();
	}

	c._set_index(-1);
	return 0;
}


#endif /* UT_HASH_H */
