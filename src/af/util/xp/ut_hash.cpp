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

#include <string.h>
#include "ut_hash.h"

#include "ut_debugmsg.h"


// fwd. decls.
static  UT_uint32 _Recommended_hash_size(UT_uint32	size);
static  UT_uint32 _Recommended_hash_size(UT_uint32	numslots,
					 UT_uint32	slotsize,
					 UT_uint32	max_tablesize);

// Here we declare a couple of classes internal to the hashtable's impl

// wrapper class for keys
class key_wrapper
{
public:
	key_wrapper() 
		: m_hashval(0) { }
	~key_wrapper() { }
	
	inline void die() 
		{ m_val.clear(); }
	
	inline bool eq(const UT_String &key) const
	{
		return (m_val == key);
	}
	
	inline void operator=(const UT_String &k)	
		{ m_val = k; }
	
	inline UT_uint32 hashval() const		
		{ return m_hashval; }
	inline void set_hashval(UT_uint32 h)	
		{ m_hashval = h; }
	
	inline UT_String &value(void) 
		{return m_val;}

	inline void operator=(const key_wrapper& rhs)
		{ m_val = rhs.m_val; m_hashval = rhs.m_hashval; }

	static inline UT_uint32 compute_hash(const UT_String &key) 
		{
#if 0
			// 9987001 is a reasnonably large prime, for void *
			return reinterpret_cast<UT_uint32>(key) * 0x9863b9; 
#else
			return hashcode (key); // UT_String::hashcode
#endif
		}

private:
	UT_String m_val;
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
			m_value = static_cast<void *>(this);
			m_key.die();
		}
	inline void make_empty() 
		{ m_value = 0; }

	inline const void * value() const 
		{ return m_value; }

	inline void insert(const void * v, const UT_String &k, UT_uint32 h)
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

	inline bool deleted(const void * delval) const
		{
			return ((delval ? false : static_cast<const void *>(this)) ==  (m_value));
		}

	inline bool key_eq(const UT_String &test, size_t h) const
		{
#if 1
			return m_key.eq(test);
#else
			return m_key.hashval() == h;
#endif
		}
	
	const void *	m_value;
	key_wrapper	m_key;	
};


/*!
 * This class represents a mapping between key/value pairs where the keys are
 * represented by UT_String (a wrapper around char*) and the values may be of
 * any pointer type (void *)
 */
UT_StringPtrMap::UT_StringPtrMap(size_t expected_cardinality)
	:	n_keys(0),
		n_deleted(0),
		m_nSlots(_Recommended_hash_size(expected_cardinality)),
		reorg_threshold(compute_reorg_threshold(m_nSlots)),
		flags(0)
{
	m_pMapping = new hash_slot[m_nSlots];
}


UT_StringPtrMap::~UT_StringPtrMap()
{
	delete[] m_pMapping;
}


/*!
 * Find the value associated with the key \k
 * \return 0 if key not found, object if found
 */
const void * UT_StringPtrMap::pick(const char * k) const
{
  UT_String aKey = k;
  return pick (aKey);
}

const void * UT_StringPtrMap::pick(const UT_String & k) const
{
	hash_slot*		sl = 0;
	bool			key_found = false;
	size_t			slot;
	size_t			hashval;
	
	sl = find_slot(k, _CM_LOOKUP, slot, key_found, hashval, 0, 0, 0, 0);
	return key_found ? sl->value() : 0;
}

/*!
 * See if the map contains the (key, value) pair represented by (\k, \v)
 * If \v is null, just see if the key \k exists
 * \return truth
 */
bool UT_StringPtrMap::contains(const char * k, const void * v) const
{
  UT_String aKey = k;
  return contains (aKey, v);
}

bool UT_StringPtrMap::contains(const UT_String & k, const void * v) const
{
	hash_slot * sl = 0;
	bool key_found = false;
	bool v_found = false;
	size_t slot = 0;
	size_t hashval = 0;

#if 0
	sl = find_slot (k, _CM_LOOKUP, slot, key_found,
			hashval, v, &v_found, 0, 0);
	return v_found;
#else
	sl = find_slot(k, _CM_LOOKUP, slot, key_found, hashval, 0, 0, 0, 0);

	// this is obviously not correct
	return key_found ? true : false;
#endif
}


/*!
 * Insert this key/value pair into the map
 */
void UT_StringPtrMap::insert(const char * key, const void * value)
{
  UT_String aKey = key;
  insert (aKey, value);
}

void UT_StringPtrMap::insert(const UT_String & key, const void * value)
{
	size_t		slot = 0;
	bool		key_found = false;
	size_t		hashval = 0;

	hash_slot* sl = find_slot(key, _CM_INSERT, slot, key_found, 
				  hashval, 0, 0, 0, 0);
	sl->insert(value, key, hashval);
	++n_keys;
	
	if (too_full()) {
		if (too_many_deleted()) {
			reorg(m_nSlots);
		} else {
			grow();
		}
	}

#if 1
	UT_DEBUGMSG(("DOM: inserted (%s, %p)\n", key.c_str(), value));

	const void * v = pick (key);
	bool b = contains (key, 0);

	UT_DEBUGMSG(("DOM: %d (%s, %p, %d)\n", b, key.c_str(), v, v==value));

#endif
}

/*!
 * Set the item determined by \key to the value \value
 * If item(\key) does not exist, insert it into the map
 */
void UT_StringPtrMap::set(const char * key, const void * value)
{
  UT_String aKey = key;
  set (aKey, value);
}

void UT_StringPtrMap::set(const UT_String & key, const void * value)
{
	size_t		slot = 0;
	bool		key_found = false;
	size_t		hashval = 0;
	
	hash_slot* sl = find_slot(key, _CM_LOOKUP, slot, key_found, 
				  hashval, 0, 0, 0, 0);
	
	if (!sl || !key_found) // TODO: should we insert or just return?
	{
#if 1
		insert(key, value);
#endif
		return;
	}
	
	sl->insert(value, key, hashval);
}

/*!
 * Return a UT_Vector of elements in the HashTable that you must
 * Later free with a call to delete
 */
UT_Vector * UT_StringPtrMap::enumerate (void) const
{
	UT_Vector * pVec = new UT_Vector (size());

	UT_Cursor cursor(this);

	const void * val = cursor.first ();

	while (true) {
		// we don't allow nulls since so much of our code depends on this
		// behavior
#if 1
		if (val)
			pVec->addItem ((void *)val);
#endif
		if (!cursor.more())
			break;
		val = cursor.next();
	}

	return pVec;
}

/*!
 * Remove the item referenced by \key in the map
 */
void UT_StringPtrMap::remove(const char * key, const void *)
{
  UT_String aKey = key;
  remove (aKey, 0);
}

void UT_StringPtrMap::remove(const UT_String & key, const void *)
{
	size_t slot = 0, hashval;
	bool found = false;
	hash_slot* sl = find_slot(key, _CM_LOOKUP, slot, found,
							  hashval, 0, 0, 0, 0);
	
	if (found) {
		sl->make_deleted();
		--n_keys;
		++n_deleted;
		if (m_nSlots > 11 && m_nSlots / 4 >= n_keys)
			reorg(_Recommended_hash_size(m_nSlots/2));
	}
}

/*!
 * Remove all key/value pairs from the map
 */
void UT_StringPtrMap::clear()
{
	hash_slot* slots = m_pMapping;
	for (size_t x=0; x < m_nSlots; x++) {
		hash_slot& this_slot = slots[x];
		if (!this_slot.empty()) {
			if (!this_slot.deleted(0))
				this_slot.make_deleted();
			this_slot.make_empty();
		}
	}
	n_keys = 0;
	n_deleted = 0;
}


/*********************************************************************/
/*********************************************************************/


void UT_StringPtrMap::assign_slots(hash_slot* p, size_t old_num_slot)
{
	size_t target_slot = 0;
	
	for (size_t slot_num=0; slot_num < old_num_slot; ++slot_num, ++p) {
		if (!p->empty() && !p->deleted(0)) {
			bool kf = false;
			
			size_t hv;
			hash_slot* sl = find_slot(p->m_key.value(),
									  _CM_REORG,
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

size_t UT_StringPtrMap::compute_reorg_threshold(size_t nSlots)
{
	return nSlots * 7 / 10;	// reorg threshold = 70% of nSlots
}


hash_slot*
UT_StringPtrMap::find_slot(const UT_String & k,
						_CM_search_type	search_type,
						size_t&			slot,
						bool&			key_found,
						size_t&			hashval,
						const void*	        v,
						bool*			v_found,
						void*			vi,
						size_t			hashval_in) const
{
	hashval = (hashval_in ? hashval_in : key_wrapper::compute_hash(k));

	int nSlot = hashval % m_nSlots;

	xxx_UT_DEBUGMSG(("DOM: hashval for \"%s\" is %d (#%dth slot)\n", k.c_str(), hashval, nSlot));

	hash_slot* sl = &m_pMapping[nSlot];
	
	if (sl->empty()) {
		
		xxx_UT_DEBUGMSG(("DOM: empty slot\n"));
		
		slot = nSlot;
		key_found = false;
		return sl;
	} 
	else {
		if (search_type != _CM_REORG &&
			!sl->deleted(0) &&
			sl->key_eq(k, hashval))
	    {
			slot = nSlot;
			key_found = true;
			
			if (v)
				*v_found = ((sl->value() == v) ? true : false);

			xxx_UT_DEBUGMSG(("DOM: found something #1\n"));

			return sl;
	    }
	}
	
	int delta = (nSlot ? (m_nSlots - nSlot) : 1);
	hash_slot* tmp_sl = sl;
	sl = 0;
	size_t s = 0;
	key_found = false;
	
	while (1) {
		nSlot -= delta;
		if (nSlot < 0) {
			nSlot += m_nSlots;
			tmp_sl += (m_nSlots - delta);
		}
		else
			tmp_sl -= delta;
		
		if (tmp_sl->empty()) {
			if (!s) {
				s = nSlot;
				sl = tmp_sl;
			}
			break;
			
		} else if (tmp_sl->deleted(0)) {
			if (!s) {
				s = nSlot;
				sl = tmp_sl;
			}
			
		} else if (search_type != _CM_REORG && tmp_sl->key_eq(k, hashval)) {
			s = nSlot;
			sl = tmp_sl;
			key_found = true;
			
			if (v) {
				*v_found = (sl->value() == (void *)v) ? true : false;
			}
			break;
		}
	}
	
	slot = s;
	return sl;
}


void UT_StringPtrMap::grow()
{
	size_t slots_to_allocate = ::_Recommended_hash_size(m_nSlots / 2 + m_nSlots);
	reorg(slots_to_allocate);
}


void UT_StringPtrMap::reorg(size_t slots_to_allocate)
{
	hash_slot* pOld = m_pMapping;
	
	//const size_t old_map_size = m_nSlots * sizeof(hash_slot);
	
	if (slots_to_allocate < 11) {
		slots_to_allocate = 11;
	}
	
	m_pMapping = new hash_slot[slots_to_allocate];
	
	const size_t old_num_slot = m_nSlots;
	
	m_nSlots = slots_to_allocate;
	reorg_threshold = compute_reorg_threshold(m_nSlots);
	
	assign_slots(pOld, old_num_slot);

	n_deleted = 0;

#if 0 	// todo: aren't these redundant?
	memset(pOld, 0, old_map_size);
#endif
	delete[] pOld;
}


const void * UT_StringPtrMap::_first(UT_Cursor* c) const
{
	const hash_slot* map = m_pMapping;
	size_t x;
	for (x=0; x < m_nSlots; ++x) {
		if (!map[x].empty() && !map[x].deleted(0))
			break;
	}
	if (x < m_nSlots) {
		c->_set_index(x);	// c = 'UT_Cursor etc'
		return map[x].value();
	}
	
	c->_set_index(-1);
	return 0;
}

const UT_String & UT_StringPtrMap::_key(UT_Cursor* c) const
{
	hash_slot slot = m_pMapping[c->_get_index()];

	if (!slot.empty() && !slot.deleted(0))
		return slot.m_key.value();
	// should never happen
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

const void * UT_StringPtrMap::_next(UT_Cursor* c) const
{
	const hash_slot* map = m_pMapping;
	size_t x;
	for (x = c->_get_index() + 1; x < m_nSlots; ++x) {
		if (!map[x].empty() && !map[x].deleted(0))
			break;
	}
	if (x < m_nSlots) {
		c->_set_index(x);
		return map[x].value();
	}

	c->_set_index(-1);
	return 0;
}


const void * UT_StringPtrMap::_prev(UT_Cursor *c) const
{
	const hash_slot* map = m_pMapping;
	size_t x;
	for (x = c->_get_index() - 1; x >= 0; --x) {
		if (!map[x].empty() && !map[x].deleted(0))
			break;
	}
	if (x >= 0) {
		c->_set_index(x);
		return map[x].value();
	}
	
	c->_set_index(-1);
	return 0;
}

/*********************************************************************/
/*********************************************************************/

const UT_uint32 _Hash_magic_numbers[] =
{
	2,		3,		5,		7,		11,		13,		17,		19,
	23,		29,		31,		37,		41,		43,		47,		53,
	59,		61,		67,		71,		73,		79,		83,		89,
	97,		101,	103,	107,	109,	113,	127,	131,
	137,	139,	149,	151,	157,	163,	167,	173,
	179,	181,	191,	193,	197,	199,	211,	223,
	227,	229,	233,	239,	241,	251,	257,	263,
	269,	271,	277,	281,	283,	293,	307,	311,
	313,	317,	331,	337,	347,	349,	353,	359,
	367,	373,	379,	383,	389,	397,	401,	409,
	419,	421,	431,	433,	439,	443,	449,	457,
	461,	463,	467,	479,	487,	491,	499,	503,
	509,	521,	523,	541,	547,	557,	563,	569,
	571,	577,	587,	593,	599,	601,	607,	613,
	619,	631,	641,	647,	653,	659,	661,	673,
	677,	683,	691,	701,	709,	719,	727,	733,
	739,	743,	751,	757,	761,	769,	773,	787,
	797,	809,	811,	821,	829,	839,	853,	859,
	863,	877,	883,	887,	907,	911,	919,	929,
	937,	941,	947,	953,	967,	971,	977,	983,
	991,	997,	1009,	1019,	1021,	1031,	1039,	1049,
	1051,	1061,	1069,	1087,	1097,	1103,	1109,	1117,
	1123,	1129,	1151,	1153,	1163,	1171,	1181,	1187,
	1193,	1201,	1213,	1223,	1231,	1237,	1249,	1259,
	1277,	1289,	1301,	1307,	1319,	1327,	1361,	1373,
	1381,	1399,	1409,	1423,	1433,	1447,	1459,	1471,
	1483,	1493,	1499,	1511,	1523,	1531,	1543,	1553,
	1567,	1579,	1583,	1597,	1609,	1621,	1637,	1657,
	1669,	1693,	1709,	1723,	1733,	1747,	1759,	1777,
	1789,	1801,	1811,	1823,	1831,	1847,	1861,	1879,
	1889,	1907,	1913,	1931,	1949,	1951,	1973,	1987,
	2003,	2017,	2029,	2039,	2053,	2069,	2089,	2099,
	2113,	2131,	2143,	2161,	2179,	2203,	2221,	2243,
	2251,	2273,	2293,	2311,	2333,	2351,	2371,	2393,
	2411,	2423,	2447,	2467,	2477,	2503,	2521,	2543,
	2557,	2579,	2593,	2617,	2633,	2659,	2683,	2707,
	2731,	2753,	2777,	2803,	2819,	2843,	2861,	2887,
	2909,	2927,	2953,	2971,	2999,	3023,	3049,	3079,
	3109,	3137,	3167,	3191,	3221,	3253,	3271,	3301,
	3331,	3361,	3391,	3413,	3433,	3467,	3499,	3533,
	3559,	3593,	3623,	3659,	3691,	3727,	3761,	3797,
	3833,	3863,	3889,	3923,	3947,	3967,	4003,	4027,
	4057,	4093,	4133,	4159,	4177,	4217,	4259,	4297,
	4339,	4373,	4409,	4451,	4493,	4523,	4567,	4603,
	4649,	4691,	4733,	4759,	4801,	4831,	4877,	4919,
	4967,	5011,	5059,	5107,	5153,	5197,	5237,	5281,
	5333,	5381,	5431,	5483,	5531,	5581,	5623,	5669,
	5717,	5749,	5801,	5857,	5903,	5953,	6011,	6067,
	6121,	6173,	6229,	6287,	6343,	6397,	6451,	6491,
	6553,	6607,	6673,	6737,	6803,	6871,	6917,	6983,
	7043,	7109,	7177,	7247,	7309,	7369,	7433,	7507,
	7577,	7649,	7723,	7793,	7867,	7937,	8011,	8089,
	8167,	8243,	8317,	8389,	8467,	8543,	8627,	8713,
	8783,	8867,	8951,	9029,	9109,	9199,	9283,	9371,
	9463,	9551,	9643,	9739,	9833,	9931,	10009,	10103,
	10193,	10289,	10391,	10487,	10589,	10691,	10789,	10891,
	10993,	11093,	11197,	11299,	11411,	11519,	11633,	11743,
	11839,	11953,	12071,	12163,	12281,	12401,	12517,	12641,
	12763,	12889,	13009,	13127,	13249,	13381,	13513,	13633,
	13763,	13883,	14011,	14149,	14281,	14423,	14563,	14699,
	14843,	14983,	15131,	15277,	15427,	15581,	15733,	15889,
	16033,	16193,	16349,	16493,	16657,	16823,	16987,	17137,
	17299,	17471,	17627,	17791,	17959,	18133,	18313,	18493,
	18671,	18839,	19013,	19183,	19373,	19559,	19753,	19949,
	20147,	20347,	20549,	20753,	20959,	21163,	21347,	21559,
	21773,	21977,	22193,	22409,	22621,	22817,	23041,	23269,
	23497,	23719,	23929,	24151,	24391,	24631,	24877,	25121,
	25367,	25609,	25849,	26107,	26357,	26597,	26861,	27127,
	27397,	27653,	27919,	28183,	28463,	28729,	29009,	29297,
	29587,	29881,	30169,	30469,	30773,	31079,	31387,	31699,
	32009,	32327,	32647,	32971,	33289,	33619,	33941,	34273,
	34613,	34949,	35291,	35617,	35969,	36319,	36677,	37039,
	37409,	37783,	38153,	38501,	38873,	39251,	39631,	40013,
	40387,	40787,	41189,	41597,	41999,	42409,	42829,	43237,
	43669,	44101,	44537,	44971,	45413,	45863,	46309,	46771,
	47237,	47701,	48163,	48623,	49109,	49597,	50087,	50587,
	51071,	51581,	52081,	52583,	53101,	53629,	54163,	54679,
	55219,	55763,	56311,	56873,	57427,	57991,	58567,	59149,
	59729,	60317,	60919,	61519,	62131,	62743,	63367,	63997,
	64633,	65269,	65921,	66571,	67231,	67901,	68567,	69247,
	69931,	70627,	71333,	72043,	72763,	73483,	74209,	74941,
	75689,	76441,	77201,	77969,	78737,	79493,	80287,	81083,
	81883,	82699,	83497,	84319,	85159,	85999,	86857,	87721,
	88591,	89459,	90353,	91253,	92153,	93059,	93983,	94907,
	95819,	96769,	97729,	98689,	99667,	100649,	101653,	102667,
	103687,	104723,	105769,	106823,	107881,	108959,	110039,	111127,
	112237,	113359,	114487,	115631,	116747,	117911,	119089,	120277,
	121469,	122663,	123887,	125119,	126359,	127609,	128879,	130147,
	131447,	132761,	134087,	135427,	136777,	138143,	139511,	140897,
	142297,	143719,	145139,	146581,	148021,	149497,	150991,	152461,
	153953,	155473,	157019,	158581,	160163,	161761,	163367,	164999,
	166643,	168293,	169957,	171653,	173359,	175081,	176819,	178571,
	180347,	182141,	183959,	185797,	187651,	189523,	191413,	193327,
	195259,	197207,	199153,	201139,	203141,	205171,	207199,	209269,
	211349,	213461,	215587,	217739,	219911,	222109,	224327,	226553,
	228799,	231079,	233371,	235699,	238039,	240379,	242779,	245183,
	247633,	250109,	252607,	255133,	257671,	260231,	262819,	265427,
	268069,	270749,	273433,	276151,	278911,	281683,	284489,	287333,
	290201,	293099,	296027,	298943,	301927,	304943,	307969,	311041,
	314137,	317269,	320431,	323623,	326831,	330097,	333397,	336727,
	340079,	343433,	346867,	350293,	353783,	357319,	360869,	364471,
	368111,	371779,	375481,	379207,	382999,	386809,	390673,	394579,
	398509,	402487,	406507,	410561,	414653,	418799,	422969,	427181,
	431449,	435763,	440101,	444487,	448927,	453379,	457903,	462481,
	467101,	471769,	476479,	481231,	486043,	490891,	495799,	500741,
	505727,	510773,	515873,	521023,	526231,	531481,	536791,	542153,
	547567,	553037,	558563,	564133,	569773,	575441,	581183,	586981,
	592849,	598777,	604759,	610801,	616909,	623071,	629281,	635567,
	641909,	648317,	654799,	661343,	667949,	674603,	681341,	688147,
	695021,	701969,	708979,	716063,	723221,	730451,	737753,	745117,
	752527,	760043,	767633,	775309,	783043,	790871,	798773,	806737,
	814799,	822907,	831109,	839413,	847789,	856249,	864811,	873437,
	882169,	890969,	899863,	908861,	917927,	927097,	936361,	945701,
	955153,	964703,	974329,	984059,	993893,	1003819,1013851,1023977,
	1034207,1044529,1054957,1065503,1076143,1086901,1097743,1108717,
	1119799,1130981,1142287,1153687,1165223,1176871,1188637,1200509,
	1212487,1224599,1236827,1249187,1261649,1274249,1286983,1299841,
	1312823,1325941,1339199,1352557,1366031,1379681,1393471,1407397,
	1421461,1435669,1450021,1464503,1479139,1493929,1508867,1523953,
	1539149,1554529,1570073,1585769,1601623,1617619,1633789,1650109,
	1666607,1683271,1700099,1717099,1734247,1751587,1769101,1786787,
	1804643,1822673,1840877,1859281,1877873,1896647,1915609,1934761,
	1954097,1973633,1993367,2013299,2033429,2053757,2074279,2095021,
	2115961,2137117,2158483,2180051,2201839,2223857,2246077,2268517,
	2291197,2314097,2337233,2360597,2384197,2408011,2432077,2456383,
	2480927,2505707,2530751,2556041,2581597,2607403,2633473,2659801,
	2686393,2713253,2740379,2767771,2795437,2823389,2851621,2880131,
	2908931,2938009,2967389,2997031,3027001,3057253,3087811,3118673,
	3149851,3181349,3213151,3245267,3277699,3310469,3343559,3376993,
	3410753,3444851,3479249,3514013,3549137,3584617,3620443,3656641,
	3693203,3730093,3767389,3805057,3843107,3881509,3920311,3959507,
	3999067,4039051,4079431,4120223,4161419,4203019,4245029,4287473,
	4330343,4373623,4417351,4461493,4506053,4551103,4596607,4642549,
	4688951,4735823,4783169,4830967,4879267,4928029,4977263,5027023,
	5077283,5128037,5179303,5231089,5283389,5336209,5389543,5443429,
	5497861,5552819,5608331,5664401,5721041,5778251,5836027,5894377,
	5953319,6012847,6072973,6133669,6195001,6256951,6319513,6382699,
	6446513,6510967,6576067,6641827,6708227,6775273,6843019,6911447,
	6980551,7050353,7120847,7192043,7263959,7336577,7409921,7484011,
	7558843,7634387,7710719,7787803,7865657,7944301,8023739,8103971,
	8184977,8266807,8349433,8432923,8517233,8602397,8688409,8775287,
	8863037,8951659,9041173,9131579,9222887,9315109,9408257,9502333,
	9597349,9693317,9790229,9888127,9987001
};

const UT_uint32  _Hash_n_magic_numbers = sizeof _Hash_magic_numbers / sizeof *_Hash_magic_numbers;

static UT_uint32 _Recommended_hash_size(UT_uint32 size)	// Verifies reasonably
{
	UT_uint32 lo = 0;
	UT_uint32 hi = _Hash_n_magic_numbers - 1;
	while (lo < hi)
	{
		UT_uint32 mid = (hi + lo) / 2;
		UT_uint32 s = _Hash_magic_numbers[mid];
		if (s < size)
			lo = mid + 1;
		else if (s > size)
			hi = mid - 1;
		else
			return s;
	}
	if (_Hash_magic_numbers[lo] < size)
		lo++;
	if (lo >= _Hash_n_magic_numbers) 
		return (UT_uint32)-1;
	
	return _Hash_magic_numbers[lo];
}

static UT_uint32 _Recommended_hash_size(UT_uint32 numslots, 
					UT_uint32 slotsize, 
					UT_uint32 max_tablesize)
{
	UT_uint32 lo = 0;
	UT_uint32 hi = _Hash_n_magic_numbers - 1;
	while (hi > lo)
	{
		UT_uint32 mid = (hi + lo) / 2;
		UT_uint32 s = _Hash_magic_numbers[mid];
		if (numslots > s) {
			lo = mid + 1;
		} else if (numslots <= s) {
			hi = mid - 1;
		} else
			return s;
	}
	while (_Hash_magic_numbers[lo] * slotsize > max_tablesize)
		--lo;
	if (lo >= _Hash_n_magic_numbers)
		return (UT_uint32)-1;
	return _Hash_magic_numbers[lo];
}
