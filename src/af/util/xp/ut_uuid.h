/* AbiSource Program Utilities
 * Copyright (C) 2003-2004 Tomas Frydrych <tomasfrydrych@yahoo.co.uk>
 *
 * Based on libuuid
 * Copyright (C) 1996, 1997, 1998 Theodore Ts'o.
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

#ifndef UT_UUID_H
#define UT_UUID_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

#include <time.h>

#ifdef WIN32
#  include <winsock.h> // this is where timeval etc is defined ...
#  include <rpc.h>   // this is where uuid_t is define
#else
#  include <sys/time.h> // this is where timeval should be ...
#  include <sys/uuid.h> // this is where uuid_t should be ...
#endif

class UT_String;

/* UUID Variant definitions */
typedef enum
{
		UUID_VARIANT_NCS = 0,
		UUID_VARIANT_DCE = 1,
		UUID_VARIANT_MICROSOFT = 2,
		UUID_VARIANT_OTHER = 3,
		UUID_VARIANT_ERROR = 0xffffffff
} UT_UUIDVariant;

#if 0
// this is what uuid_t is supposed to look; if it is not defined for
// your platform, enable this definition
typedef struct _uuid_t
{
    UT_uint32 d1;
    UT_uint16 d2;
    UT_uint16 d3;
    unsigned char d4[8];
} uuid_t;
#endif

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
	/*
	   all constructors are protected; instances of UT_UUID will be
	   created through UT_UUIDGenerator declared below
	*/
	
	/* virtual destructor*/
	virtual ~UT_UUID (){};

	// these generate new UUIDs
	bool            makeUUID();
	bool            makeUUID(UT_String & s);
	bool            makeUUID(uuid_t & u);

	// these set m_uuid to given UUID
	bool            setUUID(const UT_String &s);
	bool            setUUID(const char *s);
	bool            setUUID(const uuid_t &uu);

	// get it in standard uuid_t format
	bool            getUUID(uuid_t &u) const {return _pack(m_uuid,u);}
	
	/* convert uuid to and from strings */
	bool            fromString(const UT_String &from) {return setUUID(from);}
	bool            fromString(const UT_String &from, uuid_t &to) const;
	bool            fromString(const char * from, uuid_t &to) const;

	bool            toString(UT_String & to) const;
	bool            toString(const uuid_t &from, UT_String & to) const;

	UT_uint32       hash32() const;
	UT_uint64       hash64() const;
	
	static const UT_UUID & getNull() {return s_Null;}

	// these retrieve various information from UUID
	time_t          getTime() const;
	time_t          getTime(const uuid_t & u) const;
	
	UT_sint32       getType() const;
	UT_sint32       getType(const uuid_t &uu) const;

	UT_UUIDVariant  getVariant() const;
	UT_UUIDVariant  getVariant(const uuid_t &uu) const;

	// NB: these are spatial operators, not temporal ...
	bool            operator ==(const UT_UUID &u) const;
	bool            operator !=(const UT_UUID &u) const;
	bool            operator < (const UT_UUID &u) const;
	bool            operator > (const UT_UUID &u) const;

	UT_UUID &       operator = (const UT_UUID &u);
	
	// temporal comparisons
	bool            isOlder(const UT_UUID &u) const;
	bool            isYounger(const UT_UUID &u) const;
	bool            isOfSameAge(const UT_UUID &u) const;

	bool            isValid() const {return m_bIsValid;}
	bool            isNull() const;

	void            clear();
	
   protected:
	friend class UT_UUIDGenerator;
	/* various protected constructors */
	UT_UUID(); // constructs NULL uuid; subsequent call to makeUUID() needed
	UT_UUID(const UT_String &s);
	UT_UUID(const char *s);
	UT_UUID(const uuid_t &u);
	UT_UUID(const UT_UUID &u);

	
	// can be ovewritten when a better source of randomness than
	// UT_rand() is available on given platform
	virtual bool    _getRandomBytes(void *buf, int nbytes);
	
  private:
	bool            _pack(const struct uuid & unpacked, uuid_t &uuid) const;
	bool            _unpack(const uuid_t &in, struct uuid &uu) const;
	bool            _parse(const char * in, struct uuid &u) const;
	
	bool            _makeUUID(struct uuid & u);
	bool            _toString(const struct uuid &uu, UT_String & s) const;

	time_t          _getTime(const struct uuid & uu) const;
	UT_sint32       _getType(const struct uuid &uu) const;
	UT_UUIDVariant  _getVariant(const struct uuid &uu) const;
	
	bool            _getClock(UT_uint32 &iHigh, UT_uint32 &iLow, UT_uint16 &iSeq);

  private:	
	uuid                   m_uuid;
	bool                   m_bIsValid;
	static bool            s_bInitDone;
	static unsigned char   s_node[6];
	static UT_UUID         s_Null;
};

/*
    This class mediates creation of UT_UUID class.
    
    We create an instance of UT_UUIDGeneratr (or derived) class in
    XAP_App() and have XAP_App::getUUIDGenerator() to gain access to
    it.  This allows us to create platform specific instances of
    UT_UUID from xp code.
*/
class ABI_EXPORT UT_UUIDGenerator
{
  public:
	UT_UUIDGenerator(){};
	virtual ~UT_UUIDGenerator(){};

	// because the default constructor creates NULL uuid, we also need
	// to call makeUUID() with this one
	virtual UT_UUID * createUUID(){UT_UUID *p = new UT_UUID(); if(p)p->makeUUID(); return p;}

	virtual UT_UUID * createUUID(const UT_String &s){return new UT_UUID(s);}
	virtual UT_UUID * createUUID(const char *s){return new UT_UUID(s);}
	virtual UT_UUID * createUUID(const uuid_t &u){return new UT_UUID(u);}
	virtual UT_UUID * createUUID(const UT_UUID &u){return new UT_UUID(u);}
};

#endif /* UT_UUID_H */
