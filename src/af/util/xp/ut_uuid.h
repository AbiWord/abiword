/* AbiSource Program Utilities
 * Copyright (C) 2003-2004 Tomas Frydrych <tomasfrydrych@yahoo.co.uk>
 *
 * Based on libuuid
 * Copyright (C) 1996, 1997, 1998 Theodore Ts'o.
 *
 * The hash functions can be compile to use Fowler/Noll/Vo (FNV) public domain algorithm;
 * see http://www.isthe.com/chongo/tech/comp/fnv/index.html
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

#ifndef UT_UUID_H
#define UT_UUID_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_vector.h"
#include <time.h>

#include <string>

class UT_UTF8String;


/* UUID Variant definitions */
enum UT_UUIDVariant
{
	UT_UUID_VARIANT_NCS = 0,
	UT_UUID_VARIANT_DCE = 1,
	UT_UUID_VARIANT_MICROSOFT = 2,
	UT_UUID_VARIANT_OTHER = 3,
	UT_UUID_VARIANT_ERROR = 0xffffffff
};

struct uuid
{
    UT_uint32   time_low;
    UT_uint16   time_mid;
    UT_uint16   time_high_and_version;
    UT_uint16   clock_seq;
    UT_Byte     node[6];
};

// forward declaration
class UT_UUIDGenerator;


/*!
    Class for generating and managing UUIDs

    On platforms which provide means of generating random data that
    is superior to calling UT_rand() a platform specific derrived
    class should implement virtual _getRandomBytes().

    If a derived class is created, it will need to be accompanied by
    corresponding derived UT_UUIDGenerator class (described below) and
    the call in xap_*App constructor to _setUUIDGenerator() will need
    to be passed an instance of the platfrom-specific class.
*/
class ABI_EXPORT UT_UUID
{
  public:
        friend void UT_UUIDGenerator__test(UT_UUIDGenerator*);
  public:
	/*
	   All constructors are protected; instances of UT_UUID will be
	   created through UT_UUIDGenerator declared below.
	*/

	/* virtual destructor */
	virtual ~UT_UUID (){};

	/*
	   Various manipulation functions; in general functions that take
	   some kind of an output parameter (usually named 'out' or 'to'
	   below) DO NOT modifiy internal state of the class, and return
	   value of 'true' indicates success.
	*/

	/* These generate new UUIDs */
	bool            makeUUID();                // changes internal state
	bool            makeUUID(UT_UTF8String & out); // does not change internal state !!!

	/* these set m_uuid to given UUID, i.e., force internal state change */
	bool            setUUID(const UT_UTF8String &s);
	bool            setUUID(const char *s);
	bool            setUUID(const struct uuid &u);

	/* translate internal state into string representation; do not change
	   internal state */
	bool		toString(UT_UTF8String & to) const;
	std::string&	toString( std::string& to ) const;

	/* get the binary representation of the uuid */
	bool            toBinary(struct uuid &u) const;

	/* convert binary uuid representaiton to a string */
	static bool     toStringFromBinary(char * s, UT_uint32 len, const struct uuid &u);

	/* create hash of the uuid -- if all you need is the hash, use
	   UT_UUIDGenerator::getUUID*() instead of these; it provides collision correction */
	UT_uint32       hash32() const;
	UT_uint64       hash64() const;

	/* return a NULL uuid; useful in fuction that return reference to
	   UT_UUID to indicate failure */
	static const UT_UUID & getNull() {return s_Null;}

	/* these retrieve various information from UUID; internal and
	   external variants */
	time_t          getTime() const; // NB: time_t has only 1s granularity !!!
	UT_sint32       getType() const;
	UT_UUIDVariant  getVariant() const;

	bool            resetTime(); // sets the time of UUID to now


	/* NB: these are operators over the UUID space, not temporal
	   operators !!! */
	bool            operator ==(const UT_UUID &u) const;
	bool            operator !=(const UT_UUID &u) const;
	bool            operator < (const UT_UUID &u) const;
	bool            operator > (const UT_UUID &u) const;

	UT_UUID &       operator = (const UT_UUID &u);

	/* temporal comparisons */
	bool            isOlder(const UT_UUID &u) const;
	bool            isYounger(const UT_UUID &u) const;
	bool            isOfSameAge(const UT_UUID &u) const;

	bool            isValid() const {return m_bIsValid;}
	bool            isNull() const;

	/* reset internal state to NULL uuid */
	void            clear();

  protected:
	friend class UT_UUIDGenerator;

	/* various protected constructors */

	UT_UUID(); // constructs NULL uuid; subsequent call to makeUUID() needed to initialise
	UT_UUID(const UT_UTF8String &s); // initialises from string
	UT_UUID(const char *s);      // initialises from string
	UT_UUID(const struct uuid&u);   // initialise from binary representation
	UT_UUID(const UT_UUID &u);   // copy constructor

	/* the following function can be ovewritten when a better source
	   of randomness than UT_rand() is available on given platform
	   (see ut_Win32Uuid.h/cpp for an example) */
	virtual bool    _getRandomBytes(void *buf, int nbytes) const;

  private:
	bool            _parse(const char * in, struct uuid &u) const;

	bool            _makeUUID(struct uuid & u);
	bool            _toString(const struct uuid &uu, UT_UTF8String & s) const;

	// these three functions could be made public, but I think it better not to
	// encourage operations on the struct -- create an instance of UT_UUID if you need to
	// do any operations with it
	static time_t         _getTime(const struct uuid & uu);//NB: time_t has only 1s granularity !!!
	static UT_sint32      _getType(const struct uuid &uu);
	static UT_UUIDVariant _getVariant(const struct uuid &uu);

	bool           _getClock(UT_uint32 &iHigh, UT_uint32 &iLow, UT_uint16 &iSeq) const;

  private:
	uuid                   m_uuid;
	bool                   m_bIsValid;
	static bool            s_bInitDone;
	static unsigned char   s_node[6];
	static UT_UUID         s_Null;
};

/*
    This class mediates creation of UT_UUID instances.

    We create an instance of UT_UUIDGeneratr (or derived) class in
    XAP_App() and have XAP_App::getUUIDGenerator() to gain access to
    it.  This allows us to create platform specific instances in place
    for generic UT_UUID from xp code.
*/
//#define UT_UUID_HASH_TEST
class ABI_EXPORT UT_UUIDGenerator
{
  public:
	UT_UUIDGenerator()
		:m_pUUID(NULL)
	{
	};

	virtual ~UT_UUIDGenerator(){if(m_pUUID) delete m_pUUID;};

	// because the default constructor creates NULL uuid, we also need
	// to call makeUUID() with this one
	virtual UT_UUID * createUUID(){UT_UUID *p = new UT_UUID(); if(p)p->makeUUID(); return p;}

	virtual UT_UUID * createUUID(const UT_UTF8String &s){return new UT_UUID(s);}
	virtual UT_UUID * createUUID(const char *s){return new UT_UUID(s);}
	virtual UT_UUID * createUUID(const UT_UUID &u){return new UT_UUID(u);}
	virtual UT_UUID * createUUID(const struct uuid &u){return new UT_UUID(u);}

	UT_uint32 getNewUUID32();
	UT_uint64 getNewUUID64();

  public:
        friend void UT_UUIDGenerator__test(UT_UUIDGenerator*);

  private:

	UT_UUID * m_pUUID;
};

#endif /* UT_UUID_H */
