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
    UT_uint16   time_hi_and_version;
    UT_uint16   clock_seq;
    UT_Byte     node[6];
};

class ABI_EXPORT UT_UUID
{

 public:
	/* various constructors */
	UT_UUID();
	UT_UUID(const UT_String &s);
	UT_UUID(const char *);
	UT_UUID(const uuid_t &s);
	
	virtual ~UT_UUID (){};

	void makeUUID(UT_String & s) const;
	void makeUUID(uuid_t & u) const;
	
	time_t          getTime(const uuid_t & u) const;
	UT_sint32       getType(const uuid_t &uu) const;
	UT_UUIDVariant  getVariant(const uuid_t &uu) const;

	bool strToUUID(const char * in, uuid_t &u) const;
	bool strToUUID(const UT_String &s, uuid_t &u) const;

	void UUIDtoStr(const uuid_t &uu, UT_String & s) const;
	
 protected:
	void _pack(const uuid & unpacked, uuid_t &uuid) const;
	void _unpack(const uuid_t &in, uuid &uu) const;
	
	void _makeUUID(uuid & u) const;
	void _UUIDtoStr(const uuid &uu, UT_String & s) const;

	virtual void      _getRandomBytes(void *buf, int nbytes) const;
	
	UT_sint32         _getClock(UT_uint32 &clock_high, UT_uint32 &clock_low,
								UT_uint16 &ret_clock_seq) const;
	

	bool _parse(const char * in, struct uuid &u) const;
	
private:

	uuid                 m_uuid;
	bool                 m_bIsValid;
	static bool          s_bInitDone;
	static unsigned char s_node[6];
};

#endif /* UT_UUID_H */
