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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <ctype.h>

#ifdef WIN32
#ifdef _MSC_VER
#  include <winsock.h> // this is where timeval should be ...
#else
#  include <mswsock.h> // this is where timeval etc is defined ...
#endif
#else
#  include <sys/time.h> // this is where timeval should be ...
#endif

#include "ut_uuid.h"
#include "ut_assert.h"
#include "ut_string_class.h"
#include "ut_rand.h"
#include "ut_misc.h"

#include "xap_App.h"
#include "xap_Prefs.h"


//static UT_UUID _null();

bool            UT_UUID::s_bInitDone = false;
unsigned char   UT_UUID::s_node[6] = {0,0,0,0,0,0};
UT_UUID         UT_UUID::s_Null;


/*!
    This constructor is used if the object is only used to generate new UUIDs
    or if the uuid is to be set subsequently by setUUID(); it creates
    a NULL uuid
*/
UT_UUID::UT_UUID ()
	:m_bIsValid(false)
{
	memset(&m_uuid,0,sizeof(m_uuid));
}

/*!
    The following two constructors instantiate the class from
    existing UUIDs for further processing
*/
UT_UUID::UT_UUID(const UT_UTF8String &s)
{
	m_bIsValid = _parse(s.utf8_str(), m_uuid);

	// if the UUID was not valid, we will generate a new one
	if(!m_bIsValid)
		makeUUID();
}

UT_UUID::UT_UUID(const char * in)
{
	m_bIsValid = _parse(in, m_uuid);

	// if the UUID was not valid, we will generate a new one
	if(!m_bIsValid)
		makeUUID();
}

UT_UUID::UT_UUID(const struct uuid &u)
{
    memcpy(&m_uuid, &u, sizeof(u));
	m_bIsValid = !isNull();
}

/*! copy constructor */
UT_UUID::UT_UUID(const UT_UUID &u)
{
	m_uuid = u.m_uuid;
	m_bIsValid = u.m_bIsValid;
}


/*!
    parse UUID string into the internal uuid struct
*/
bool  UT_UUID::_parse(const char * in, struct uuid &uuid) const
{
    UT_sint32   i;
    const char  *cp;
    char        buf[3];

    UT_return_val_if_fail(in, false);

	// verify this is valid uuid string
    if(strlen(in) != 36)
        return false;
	
    for(i=0, cp = in; i <= 36; i++,cp++)
	{
        if ((i == 8) || (i == 13) || (i == 18) || (i == 23))
		{
            if (*cp == '-')
                continue;
            else
                return false;
        }
		
        if(i== 36)
            if(*cp == 0)
                continue;
		
        if(!isxdigit(*cp))
            return false;
    }

	// parse it
    uuid.time_low = strtoul(in, NULL, 16);
    uuid.time_mid = (UT_uint16)strtoul(in+9, NULL, 16);
    uuid.time_high_and_version = (UT_uint16)strtoul(in+14, NULL, 16);
    uuid.clock_seq = (UT_uint16)strtoul(in+19, NULL, 16);

	cp = in+24;
    buf[2] = 0;
    for (i=0; i < 6; i++)
	{
        buf[0] = *cp++;
        buf[1] = *cp++;
        uuid.node[i] = (unsigned char)strtoul(buf, NULL, 16);
    }

	return true;
}

/*!
    convert internal UUID struct to a string
*/
bool UT_UUID::_toString(const uuid &uu, UT_UTF8String & s) const
{
    UT_UTF8String_sprintf(s,"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        uu.time_low, uu.time_mid, uu.time_high_and_version,
        uu.clock_seq >> 8, uu.clock_seq & 0xFF,
        uu.node[0], uu.node[1], uu.node[2],
        uu.node[3], uu.node[4], uu.node[5]);

	return true;
}

/*!
    convert internal state to string
*/
bool UT_UUID::toString(UT_UTF8String & s) const
{
	UT_return_val_if_fail(m_bIsValid, false);
	return _toString(m_uuid, s);
}

std::string&
UT_UUID::toString( std::string& to ) const
{
    UT_UTF8String x;
    toString( x );
    to = x.utf8_str();
    return to;
}


bool UT_UUID::toStringFromBinary(char * s, UT_uint32 len, const struct uuid &uu)
{
	if(len < 37)
		return false;

    sprintf(s,"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        uu.time_low, uu.time_mid, uu.time_high_and_version,
        uu.clock_seq >> 8, uu.clock_seq & 0xFF,
        uu.node[0], uu.node[1], uu.node[2],
        uu.node[3], uu.node[4], uu.node[5]);

	s[36] = 0;

	return true;
}


/* get the binary representation of the uuid */
bool UT_UUID::toBinary(struct uuid &u) const
{
	memset(&u, 0, sizeof(u));
	if(m_bIsValid)
	    memcpy(&u, &m_uuid, sizeof(u));

	return m_bIsValid;
}


/*!
    Set internal state to the given value represented by string
*/
bool UT_UUID::setUUID(const UT_UTF8String &s)
{
	if(_parse(s.utf8_str(), m_uuid))
	{
		m_bIsValid = true;
		return true;
	}
	
	return false;
}

bool UT_UUID::setUUID(const char *s)
{
	if(_parse(s, m_uuid))
	{
		m_bIsValid = true;
		return true;
	}
	
	return false;
}

bool UT_UUID::setUUID(const struct uuid &u)
{
    memcpy(&m_uuid, &u, sizeof(u));
	m_bIsValid = !isNull();

	return m_bIsValid;
}

/*!
    generate new UUID and set the internal value to it
*/
bool UT_UUID::makeUUID()
{
	m_bIsValid = _makeUUID(m_uuid);
	return m_bIsValid;
}

/*!
    generate new UUID into provided string
*/
bool UT_UUID::makeUUID(UT_UTF8String & s)
{
	struct uuid uuid;
	bool bRet = _makeUUID(uuid);
	bRet &= _toString(uuid, s);
	return bRet;
}

/*!
    retrive the time at which the UUID was created
*/
time_t UT_UUID::getTime() const
{
	UT_return_val_if_fail(m_bIsValid, 0xffffffff);
	return _getTime(m_uuid);
}

time_t UT_UUID::_getTime(const struct uuid & uuid)
{
	UT_uint32 iHigh;
    UT_uint64 iClockReg;
	time_t    tRet;
    
    iHigh = uuid.time_mid | ((uuid.time_high_and_version & 0xFFF) << 16);
    iClockReg = uuid.time_low | ((UT_uint64) iHigh << 32);

    iClockReg -= (((UT_uint64) 0x01B21DD2) << 32) + 0x13814000;
    tRet = (time_t)(iClockReg / 10000000);
	
    return tRet;
}

/*!
    get the type of the UUID; internal and external variant
*/
UT_sint32 UT_UUID::getType() const
{
	UT_return_val_if_fail(m_bIsValid, -1);

	return _getType(m_uuid);
}

UT_sint32 UT_UUID::_getType(const struct uuid &uuid)
{
    return ((uuid.time_high_and_version >> 12) & 0xF);
}

/*!
    get the variant of the UUID
*/
UT_UUIDVariant UT_UUID::getVariant() const
{
	UT_return_val_if_fail(m_bIsValid, UT_UUID_VARIANT_ERROR);

	return _getVariant(m_uuid);
}

UT_UUIDVariant UT_UUID::_getVariant(const struct uuid &uuid)
{
	
    UT_sint32 var = uuid.clock_seq;

    if ((var & 0x8000) == 0)
        return UT_UUID_VARIANT_NCS;
    if ((var & 0x4000) == 0)
        return UT_UUID_VARIANT_DCE;
    if ((var & 0x2000) == 0)
        return UT_UUID_VARIANT_MICROSOFT;
    return UT_UUID_VARIANT_OTHER;
}

/*!
    Generate a series of random bytes. 
 */
bool UT_UUID::_getRandomBytes(void *buf, UT_sint32 nbytes) const
{
    UT_sint32 i;
    unsigned char *cp = (unsigned char *) buf;

    for (i = 0; i < nbytes; i++)
        *cp++ ^= (UT_rand() >> 7) & 0xFF;
	
    return true;
}


/*!
    get the three parts of the 60-bit time stamp
*/
/* Assume that the gettimeofday() has microsecond granularity */
#define MAX_ADJUSTMENT 10

bool UT_UUID::_getClock(UT_uint32 &iHigh, UT_uint32 &iLow, UT_uint16 &iSeq) const
{
    static UT_sint32          iAdjustment = 0;
    static struct timeval     last = {0, 0};
    static UT_uint16          iClockSeq;
    struct timeval            tv;
    UT_uint64                 iClockReg;
    
try_again:
    UT_gettimeofday(&tv);
    if ((last.tv_sec == 0) && (last.tv_usec == 0))
	{
        _getRandomBytes(&iClockSeq, sizeof(iClockSeq));
        iClockSeq &= 0x1FFF;
        last = tv;
        last.tv_sec--;
    }

	if ((tv.tv_sec < last.tv_sec)
		|| ((tv.tv_sec == last.tv_sec) && (tv.tv_usec < last.tv_usec)))
	{
        iClockSeq = (iClockSeq+1) & 0x1FFF;
        iAdjustment = 0;
        last = tv;
    }
	else if ((tv.tv_sec == last.tv_sec) && (tv.tv_usec == last.tv_usec))
	{
        if (iAdjustment >= MAX_ADJUSTMENT)
            goto try_again;
        iAdjustment++;
    }
	else
	{
        iAdjustment = 0;
        last = tv;
    }
        
    iClockReg = tv.tv_usec*10 + iAdjustment;
    iClockReg += ((UT_uint64) tv.tv_sec)*10000000;
    iClockReg += (((UT_uint64) 0x01B21DD2) << 32) + 0x13814000;

    iHigh = (UT_uint32)(iClockReg >> 32);
    iLow  = (UT_uint32)iClockReg;
    iSeq  = iClockSeq;
    return true;
}

bool UT_UUID::resetTime()
{
    UT_uint32  clock_mid;
    bool bRet = _getClock(clock_mid, m_uuid.time_low, m_uuid.clock_seq);
	
    m_uuid.clock_seq |= 0x8000;
    m_uuid.time_mid = (UT_uint16) clock_mid;
    m_uuid.time_high_and_version = (clock_mid >> 16) | 0x1000;

	return bRet;
}

bool UT_UUID::_makeUUID(uuid &uu)
{
    UT_uint32  clock_mid;

	bool bRet = true;
	
    if(!s_bInitDone)
	{
#if 0
		bool bNoMAC;
		XAP_App::getApp()->getPrefsValueBool((gchar*)XAP_PREF_KEY_NoMACinUUID,
											 &bNoMAC);
		
        if(bNoMAC || !UT_getEthernetAddress(s_node))
#endif
		{
            bRet &= _getRandomBytes(s_node, 6);
            /*
             * Set multicast bit, to prevent conflicts
             * with IEEE 802 addresses obtained from
             * network cards
             */
            s_node[0] |= 0x80;
        }
        s_bInitDone = bRet;
    }
	
    bRet &= _getClock(clock_mid, uu.time_low, uu.clock_seq);
	
    uu.clock_seq |= 0x8000;
    uu.time_mid = (UT_uint16) clock_mid;
    uu.time_high_and_version = (clock_mid >> 16) | 0x1000;
    memcpy(uu.node, s_node, 6);

	return bRet;
}

/*!
    comparison operators working over the UUID space (not temporal !!!)
*/
bool UT_UUID::operator ==(const UT_UUID &u) const
{
	if(m_uuid.time_low != u.m_uuid.time_low)
		return false;

	if(m_uuid.time_mid != u.m_uuid.time_mid)
		return false;
	
	if(m_uuid.time_high_and_version != u.m_uuid.time_high_and_version)
		return false;

	if(m_uuid.clock_seq != u.m_uuid.clock_seq)
		return false;

	if(memcmp(m_uuid.node, u.m_uuid.node, 6) != 0)
		return false;

	return true;
}

bool UT_UUID::operator !=(const UT_UUID &u) const
{
	if(m_uuid.time_low != u.m_uuid.time_low)
		return true;

	if(m_uuid.time_mid != u.m_uuid.time_mid)
		return true;
	
	if(m_uuid.time_high_and_version != u.m_uuid.time_high_and_version)
		return true;

	if(m_uuid.clock_seq != u.m_uuid.clock_seq)
		return true;

	if(memcmp(m_uuid.node, u.m_uuid.node, 6) != 0)
		return true;

	return false;
}

bool UT_UUID::operator <(const UT_UUID &u) const
{
	if(m_uuid.time_low < u.m_uuid.time_low)
		return true;

	if(m_uuid.time_mid < u.m_uuid.time_mid)
		return true;
	
	if(m_uuid.time_high_and_version < u.m_uuid.time_high_and_version)
		return true;

	if(m_uuid.clock_seq < u.m_uuid.clock_seq)
		return true;

	if(memcmp(m_uuid.node, u.m_uuid.node, 6) < 0)
		return true;

	return false;
}

bool UT_UUID::operator >(const UT_UUID &u) const
{
	if(m_uuid.time_low > u.m_uuid.time_low)
		return true;

	if(m_uuid.time_mid > u.m_uuid.time_mid)
		return true;
	
	if(m_uuid.time_high_and_version > u.m_uuid.time_high_and_version)
		return true;

	if(m_uuid.clock_seq > u.m_uuid.clock_seq)
		return true;

	if(memcmp(m_uuid.node, u.m_uuid.node, 6) > 0)
		return true;

	return false;
}

/*!
    Assignment operator.
*/
UT_UUID & UT_UUID::operator = (const UT_UUID &u)
{
	m_uuid = u.m_uuid;
	m_bIsValid = u.m_bIsValid;
	return *this;
}

/*!
    Operators for temporal comparisons.
*/
bool UT_UUID::isYounger(const UT_UUID &u) const
{
	if((m_uuid.time_high_and_version & 0xFFF) > (u.m_uuid.time_high_and_version & 0xFFF))
		return true;
	else if((m_uuid.time_high_and_version & 0xFFF) < (u.m_uuid.time_high_and_version & 0xFFF))
		return false;
	
	if(m_uuid.time_mid > u.m_uuid.time_mid)
		return true;
	else if(m_uuid.time_mid < u.m_uuid.time_mid)
		return false;
	   
	if(m_uuid.time_low > u.m_uuid.time_low)
		return true;
	if(m_uuid.time_low < u.m_uuid.time_low)
		return false;

	return false;
}

bool UT_UUID::isOlder(const UT_UUID &u) const
{
	if((m_uuid.time_high_and_version & 0xFFF) < (u.m_uuid.time_high_and_version & 0xFFF))
		return true;
	else if((m_uuid.time_high_and_version & 0xFFF) > (u.m_uuid.time_high_and_version & 0xFFF))
		return false;
	
	if(m_uuid.time_mid < u.m_uuid.time_mid)
		return true;
	else if(m_uuid.time_mid > u.m_uuid.time_mid)
		return false;
	   
	if(m_uuid.time_low < u.m_uuid.time_low)
		return true;
	if(m_uuid.time_low > u.m_uuid.time_low)
		return false;

	return false;
}

bool UT_UUID::isOfSameAge(const UT_UUID &u) const
{
	if((m_uuid.time_high_and_version & 0xFFF) != (u.m_uuid.time_high_and_version & 0xFFF))
		return false;
	
	if(m_uuid.time_mid != u.m_uuid.time_mid)
		return false;
	   
	if(m_uuid.time_low != u.m_uuid.time_low)
		return false;

	return true;
}

bool UT_UUID::isNull() const
{
	// will treat it as null if not valid ...
	UT_return_val_if_fail(isValid(),true);

	const unsigned char * c = (const unsigned char *) &(this->m_uuid);

	for(UT_uint32 i = 0; i < sizeof(m_uuid); ++i, ++c)
		if(*c != 0)
			return false;

	return true;
}

/*!
    Reset internal state to NULL uuid
*/
void UT_UUID::clear()
{
	memset(&(this->m_uuid), 0, sizeof(m_uuid));
	m_bIsValid = false;
}

/* 
    perform a 32 bit Fowler/Noll/Vo hash
*/
UT_uint32 UT_UUID::hash32() const
{
#if 0
    // 32 bit Fowler/Noll/Vo hash on a buffer I have run extensive
	// tests using the FNV and the other algorithm in the #else branch
	// (based on UT_String) and the latter turns out to be slightly
	// less collision prone on randomly generated uuid's but
	// significantly less collision prone on uuid's generated on the
	// same machine and close in time
	static UT_uint32 hval = 0x811c9dc5;
    unsigned char *bp = (unsigned char *)&m_uuid;

	for(UT_uint32 i = 0; i < sizeof(m_uuid); ++i)
	{
		/* multiply by the 32 bit FNV magic prime mod 2^32 */
		hval *= 0x01000193;

		/* xor the bottom with the current octet */
		hval ^= (UT_uint32)*bp++;
    }

    /* return our new hash value */
    return hval;
#else
	// base on UT_String
	const unsigned char * p = (const unsigned char *)& m_uuid;
	UT_uint32 h = (UT_uint32)*p;
	
	for (UT_uint32 i = 1; i < sizeof(m_uuid); ++i, ++p)
	{
		h = (h << 5) - h + *p;
	}

	return h;
#endif
}

UT_uint64 UT_UUID::hash64() const
{
#if 0
	// see comments in hash32()
#if defined(WIN32) && !defined(__GNUC__)	
	static UT_uint64 hval = 0xcbf29ce484222325; // value FNV1_64_INIT;
#else
	static UT_uint64 hval = 0xcbf29ce484222325LL; // value FNV1_64_INIT;
#endif
    unsigned char *bp = (unsigned char *) &m_uuid;

    /*
     * FNV-1 hash each octet of the buffer
     */
	for(UT_uint32 i = 0; i < sizeof(m_uuid); ++i)
	{
		/* multiply by the 64 bit FNV magic prime mod 2^64 */
#if defined(WIN32) && !defined(__GNUC__)	
		hval *= 0x100000001b3;
#else
		hval *= 0x100000001b3LL;
#endif
		/* xor the bottom with the current octet */
		hval ^= (UT_uint64)*bp++;
    }

    /* return our new hash value */
    return hval;
#else
	// base on UT_String
	const unsigned char * p = (const unsigned char *)& m_uuid;
	UT_uint64 h = (UT_uint64)*p;
	
	for (UT_uint32 i = 1; i < sizeof(m_uuid); ++i, ++p)
	{
		h = (h << 5) - h + *p;
	}

	return h;
#endif
}


UT_uint32 UT_UUIDGenerator::getNewUUID32()
{
	// We cannot initialise m_pUUID in the constructor, because we
	// want it to be an instance of a platform specific class where
	// such exists and we cannot call virtual createUUID() before the
	// constructor of the derived class was called.
	if(!m_pUUID)
		m_pUUID = createUUID();

	UT_return_val_if_fail(m_pUUID, 0);

	m_pUUID->makeUUID();
	UT_ASSERT(m_pUUID->isValid());

	return m_pUUID->hash32();
}

UT_uint64 UT_UUIDGenerator::getNewUUID64()
{
	// We cannot initialise m_pUUID in the constructor, because we
	// want it to be an instance of a platform specific class where
	// such exists and we cannot call virtual createUUID() before the
	// constructor of the derived class was called.
	if(!m_pUUID)
		m_pUUID = createUUID();

	UT_return_val_if_fail(m_pUUID, 0);

	m_pUUID->makeUUID();
	UT_ASSERT(m_pUUID->isValid());
	
	return m_pUUID->hash64();
}




#if 0
/*
    Due to portability problems I removed the various functions
    operating on uuid_t. However, I suspect a day will come someone
    will decide we need those after all, so I leave here the code for
    _pack() and _unpack()
*/

/*!
   pack UUID from the internal struct to uuid_t
*/
bool UT_UUID::_pack(const uuid &uu, uuid_t &u) const
{
    UT_uint32   tmp;
    unsigned char   *out = (unsigned char *)&u;

    tmp = uu.time_low;
    out[3] = (unsigned char) tmp;
    tmp >>= 8;
    out[2] = (unsigned char) tmp;
    tmp >>= 8;
    out[1] = (unsigned char) tmp;
    tmp >>= 8;
    out[0] = (unsigned char) tmp;
    
    tmp = uu.time_mid;
    out[5] = (unsigned char) tmp;
    tmp >>= 8;
    out[4] = (unsigned char) tmp;

    tmp = uu.time_high_and_version;
    out[7] = (unsigned char) tmp;
    tmp >>= 8;
    out[6] = (unsigned char) tmp;

    tmp = uu.clock_seq;
    out[9] = (unsigned char) tmp;
    tmp >>= 8;
    out[8] = (unsigned char) tmp;

    memcpy(out+10, uu.node, 6);

	return true;
}

/*!
    Unpack uuid_t into the internal uuid struct
*/
bool UT_UUID::_unpack(const uuid_t &in, uuid &uu) const
{
    const unsigned char  *ptr = (const unsigned char*)&in;
    UT_uint32       tmp;

    tmp = *ptr++;
    tmp = (tmp << 8) | *ptr++;
    tmp = (tmp << 8) | *ptr++;
    tmp = (tmp << 8) | *ptr++;
    uu.time_low = tmp;

    tmp = *ptr++;
    tmp = (tmp << 8) | *ptr++;
    uu.time_mid = tmp;
    
    tmp = *ptr++;
    tmp = (tmp << 8) | *ptr++;
    uu.time_high_and_version = tmp;

    tmp = *ptr++;
    tmp = (tmp << 8) | *ptr++;
    uu.clock_seq = tmp;

    memcpy(uu.node, ptr, 6);

	return true;
}
#endif
