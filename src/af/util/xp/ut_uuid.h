/* AbiSource Program Utilities
 * Copyright (C) 2003 Tomas Frydrych <tomas@frydrych.uklinux.net>
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

#include "ut_misc.h"

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

typedef unsigned char uuid_t[16];

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
	bool            makeUUID(UT_String & s) const;
	bool            makeUUID(uuid_t & u) const;

	// these set m_uuid to given UUID
	bool            setUUID(const UT_String &s);
	bool            setUUID(const char *s);
	bool            setUUID(const uuid_t &uu);
	
	// these retrieve various information from UUID
	time_t          getTime() const;
	time_t          getTime(const uuid_t & u) const;
	
	UT_sint32       getType() const;
	UT_sint32       getType(const uuid_t &uu) const;

	UT_UUIDVariant  getVariant() const;
	UT_UUIDVariant  getVariant(const uuid_t &uu) const;

	/* convert strings into uuid_t and vice versa */
	bool            strToUUID(const char * in, uuid_t &u) const;
	bool            strToUUID(const UT_String &s, uuid_t &u) const;
	bool            UUIDtoStr(const uuid_t &uu, UT_String & s) const;

	// NB: these are spatial operators, not temporal ...
	bool            operator ==(const UT_UUID &u) const;
	bool            operator !=(const UT_UUID &u) const;
	bool            operator < (const UT_UUID &u) const;
	bool            operator > (const UT_UUID &u) const;

	// temporal comparisons
	bool            isOlder(const UT_UUID &u) const;
	bool            isYounger(const UT_UUID &u) const;
	bool            isOfSameAge(const UT_UUID &u) const;
	
   protected:
	friend class UT_UUIDGenerator;
	/* various protected constructors */
	UT_UUID();
	UT_UUID(const UT_String &s);
	UT_UUID(const char *s);
	UT_UUID(const uuid_t &u);
	UT_UUID(const UT_UUID &u);

	
	// can be ovewritten when a better source of randomness than
	// UT_rand() is available on given platform
	virtual bool    _getRandomBytes(void *buf, int nbytes) const;
	
  private:
	bool            _pack(const struct uuid & unpacked, uuid_t &uuid) const;
	bool            _unpack(const uuid_t &in, struct uuid &uu) const;
	bool            _parse(const char * in, struct uuid &u) const;
	
	bool            _makeUUID(struct uuid & u) const;
	bool            _UUIDtoStr(const struct uuid &uu, UT_String & s) const;

	time_t          _getTime(const struct uuid & uu) const;
	UT_sint32       _getType(const struct uuid &uu) const;
	UT_UUIDVariant  _getVariant(const struct uuid &uu) const;
	
	bool            _getClock(UT_uint32 &iHigh, UT_uint32 &iLow, UT_uint16 &iSeq) const;

  private:	
	uuid                 m_uuid;
	bool                 m_bIsValid;
	static bool          s_bInitDone;
	static unsigned char s_node[6];
};

/*
    We will create an instance of this (or derived) class in XAP_App()
    and have XAP_App::getUUIDGenerator(). This will allow us to create
    platform specific instances of UT_UUID from xp code
*/
class ABI_EXPORT UT_UUIDGenerator
{
  public:
	UT_UUIDGenerator(){};
	virtual ~UT_UUIDGenerator(){};

	virtual UT_UUID * createUUID(){return new UT_UUID();}
	virtual UT_UUID * createUUID(const UT_String &s){return new UT_UUID(s);}
	virtual UT_UUID * createUUID(const char *s){return new UT_UUID(s);}
	virtual UT_UUID * createUUID(const uuid_t &u){return new UT_UUID(u);}
};

#endif /* UT_UUID_H */
