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

#include "ut_uuid.h"
#include "ut_assert.h"
#include "ut_string_class.h"
#include "ut_rand.h"

bool          UT_UUID::s_bInitDone = false;
unsigned char UT_UUID::s_node[6] = {0,0,0,0,0,0};


/*!
    This constructor is used if the object is only used to generate new UUIDs
    or if the uuid is to be set subsequently by setUUID()
*/
UT_UUID::UT_UUID ()
	:m_bIsValid(false)
{
}

/*!
    The following three constructors instantiate the class from
    existing UUIDs for further processing
*/
UT_UUID::UT_UUID(const UT_String &s)
{
	m_bIsValid = _parse(s.c_str(), m_uuid);
}

UT_UUID::UT_UUID(const char * in)
{
	m_bIsValid = _parse(in, m_uuid);
}

UT_UUID::UT_UUID(const uuid_t &u)
	:m_bIsValid(true)
{
    _unpack(u, m_uuid);
}

/* copy constructor */
UT_UUID::UT_UUID(const UT_UUID &u)
{
	m_uuid = u.m_uuid;
	m_bIsValid = u.m_bIsValid;
}



/*!
    Converts string represenation of UUID to uuid_t
*/
bool UT_UUID::fromString(const UT_String &s, uuid_t &u) const
{
	return fromString(s.c_str(), u);
}


bool UT_UUID::fromString(const char * in, uuid_t &u) const
{
	struct uuid uuid;
	
    if(!_parse(in, uuid))
		return false;
	
    _pack(uuid, u);
    return true;
}

/*!
    parse UUID string into the internal (unpacked) uuid struct
*/
bool  UT_UUID::_parse(const char * in, struct uuid &uuid) const
{
    UT_sint32   i;
    const char  *cp;
    char        buf[3];

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
    convert internal UUID struct to a string
*/
bool UT_UUID::_toString(const uuid &uu, UT_String & s) const
{
    UT_String_sprintf(s,"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        uu.time_low, uu.time_mid, uu.time_high_and_version,
        uu.clock_seq >> 8, uu.clock_seq & 0xFF,
        uu.node[0], uu.node[1], uu.node[2],
        uu.node[3], uu.node[4], uu.node[5]);

	return true;
}

/*!
    convert uuid_t to a string
 */
bool UT_UUID::toString(const uuid_t &uu, UT_String & s) const
{
    struct uuid uuid;
    bool bRet = _unpack(uu, uuid);
	bRet &= _toString(uuid, s);

	return bRet;
}

bool UT_UUID::toString(UT_String & s) const
{
	UT_return_val_if_fail(m_bIsValid, false);
	return _toString(m_uuid, s);
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

/*!
    Set internal UUID to the given value
*/
bool UT_UUID::setUUID(const UT_String &s)
{
	m_bIsValid = _parse(s.c_str(), m_uuid);
	return m_bIsValid;
}

bool UT_UUID::setUUID(const char *s)
{
	m_bIsValid = _parse(s, m_uuid);
	return m_bIsValid;
}

bool UT_UUID::setUUID(const uuid_t &uu)
{
	struct uuid uuid;
    m_bIsValid = _unpack(uu, uuid);
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
    generate new UUID into provided string or uuid_t
*/
bool UT_UUID::makeUUID(UT_String & s) const
{
	struct uuid uuid;
	bool bRet = _makeUUID(uuid);
	bRet &= _toString(uuid, s);
	return bRet;
}

bool UT_UUID::makeUUID(uuid_t & u) const
{
	struct uuid uuid;
	bool bRet = _makeUUID(uuid);
    bRet &= _pack(uuid, u);
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


time_t UT_UUID::getTime(const uuid_t & uu) const
{
    struct uuid uuid;
    bool bValid = _unpack(uu, uuid);
	UT_return_val_if_fail(bValid, 0xffffffff);

	return _getTime(uuid);
}

time_t UT_UUID::_getTime(const struct uuid & uuid) const
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
    get the type of the UUID
*/
UT_sint32 UT_UUID::getType() const
{
	UT_return_val_if_fail(m_bIsValid, -1);

	return _getType(m_uuid);
}

UT_sint32 UT_UUID::getType(const uuid_t &uu) const
{
    struct uuid uuid;
    bool bValid = _unpack(uu, uuid); 
	UT_return_val_if_fail(bValid, -1);

	return _getType(uuid);
}

UT_sint32 UT_UUID::_getType(const struct uuid &uuid) const
{
    return ((uuid.time_high_and_version >> 12) & 0xF);
}

/*!
    get the variant of the UUID
*/
UT_UUIDVariant UT_UUID::getVariant() const
{
	UT_return_val_if_fail(m_bIsValid, UUID_VARIANT_ERROR);

	return _getVariant(m_uuid);
}

UT_UUIDVariant UT_UUID::getVariant(const uuid_t &uu) const
{
    struct uuid uuid;
	bool bValid = _unpack(uu, uuid);
	UT_return_val_if_fail(bValid, UUID_VARIANT_ERROR);

	return _getVariant(uuid);
}

UT_UUIDVariant UT_UUID::_getVariant(const struct uuid &uuid) const
{
	
    UT_sint32 var = uuid.clock_seq;

    if ((var & 0x8000) == 0)
        return UUID_VARIANT_NCS;
    if ((var & 0x4000) == 0)
        return UUID_VARIANT_DCE;
    if ((var & 0x2000) == 0)
        return UUID_VARIANT_MICROSOFT;
    return UUID_VARIANT_OTHER;
}

/*
 * Generate a series of random bytes. 
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

bool UT_UUID::_makeUUID(uuid &uu) const
{
    static UT_sint32 has_init = 0;
    UT_uint32  clock_mid;

	bool bRet = true;
	
    if(!s_bInitDone)
	{
        if(!UT_getEthernetAddress(s_node))
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

void UT_UUID::clear()
{
	memset(&(this->m_uuid), 0, sizeof(m_uuid));
	m_bIsValid = false;
}

