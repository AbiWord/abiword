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

bool UT_UUID::s_bInitDone = false;

/*!
    This constructor is used if the object is only used to generate new UUIDs
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

/*!
    Converts string represenation of UUID to uuid_t
*/
bool UT_UUID::strToUUID(const UT_String &s, uuid_t &u) const
{
	return strToUUID(s.c_str(), u);
}


bool UT_UUID::strToUUID(const char * in, uuid_t &u) const
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
    uuid.time_hi_and_version = (UT_uint16)strtoul(in+14, NULL, 16);
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
void UT_UUID::_pack(const uuid &uu, uuid_t &u) const
{
    UT_uint32   tmp;
    unsigned char   *out = &u[0];

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

    tmp = uu.time_hi_and_version;
    out[7] = (unsigned char) tmp;
    tmp >>= 8;
    out[6] = (unsigned char) tmp;

    tmp = uu.clock_seq;
    out[9] = (unsigned char) tmp;
    tmp >>= 8;
    out[8] = (unsigned char) tmp;

    memcpy(out+10, uu.node, 6);
}

/*!
    convert internal UUID struct to a string
*/
void UT_UUID::_UUIDtoStr(const uuid &uu, UT_String & s) const
{
    UT_String_sprintf(s,"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        uu.time_low, uu.time_mid, uu.time_hi_and_version,
        uu.clock_seq >> 8, uu.clock_seq & 0xFF,
        uu.node[0], uu.node[1], uu.node[2],
        uu.node[3], uu.node[4], uu.node[5]);
}

/*!
    convert uuid_t to a string
 */
void UT_UUID::UUIDtoStr(const uuid_t &uu, UT_String & s) const
{
    struct uuid uuid;
    _unpack(uu, uuid);
	_UUIDtoStr(uuid, s);
}

/*!
    Unpack uuid_t into the internal uuid struct
*/
void UT_UUID::_unpack(const uuid_t &in, uuid &uu) const
{
    const unsigned char  *ptr = &in[0];
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
    uu.time_hi_and_version = tmp;

    tmp = *ptr++;
    tmp = (tmp << 8) | *ptr++;
    uu.clock_seq = tmp;

    memcpy(uu.node, ptr, 6);
}

/*!
    generate new UUID
*/
void UT_UUID::makeUUID(UT_String & s) const
{
	struct uuid uuid;
	_makeUUID(uuid);
	_UUIDtoStr(uuid, s);
}

void UT_UUID::makeUUID(uuid_t & u) const
{
	struct uuid uuid;
	_makeUUID(uuid);
    _pack(uuid, u);
}

time_t UT_UUID::getTime(const uuid_t & uu) const
{
	UT_return_val_if_fail(m_bIsValid, 0);
	
    struct uuid         uuid;
    UT_uint32           high;
    UT_uint64           clock_reg;
	time_t tRet;
	

    _unpack(uu, uuid);
    
    high = uuid.time_mid | ((uuid.time_hi_and_version & 0xFFF) << 16);
    clock_reg = uuid.time_low | ((UT_uint64) high << 32);

    clock_reg -= (((UT_uint64) 0x01B21DD2) << 32) + 0x13814000;
    tRet = (time_t)(clock_reg / 10000000);
	
    return tRet;
}

UT_sint32 UT_UUID::getType(const uuid_t &uu) const
{
	UT_return_val_if_fail(m_bIsValid, -1);

    struct uuid     uuid;

    _unpack(uu, uuid); 
    return ((uuid.time_hi_and_version >> 12) & 0xF);
}

UT_UUIDVariant UT_UUID::getVariant(const uuid_t &uu) const
{
	UT_return_val_if_fail(m_bIsValid, UUID_VARIANT_ERROR);

    struct uuid     uuid;
    UT_sint32       var;

    _unpack(uu, uuid); 
    var = uuid.clock_seq;

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
void UT_UUID::_getRandomBytes(void *buf, UT_sint32 nbytes) const
{
    UT_sint32 i;
    unsigned char *cp = (unsigned char *) buf;

    for (i = 0; i < nbytes; i++)
        *cp++ ^= (UT_rand() >> 7) & 0xFF;
    return;
}


/* Assume that the gettimeofday() has microsecond granularity */
#define MAX_ADJUSTMENT 10

UT_sint32 UT_UUID::_getClock(UT_uint32 &clock_high, UT_uint32 &clock_low, UT_uint16 &ret_clock_seq) const
{
    static UT_sint32          adjustment = 0;
    static struct timeval     last = {0, 0};
    static UT_uint16          clock_seq;
    struct timeval            tv;
    UT_uint64        clock_reg;
    
try_again:
    UT_gettimeofday(&tv);
    if ((last.tv_sec == 0) && (last.tv_usec == 0))
	{
        _getRandomBytes(&clock_seq, sizeof(clock_seq));
        clock_seq &= 0x1FFF;
        last = tv;
        last.tv_sec--;
    }

	if ((tv.tv_sec < last.tv_sec)
		|| ((tv.tv_sec == last.tv_sec) && (tv.tv_usec < last.tv_usec)))
	{
        clock_seq = (clock_seq+1) & 0x1FFF;
        adjustment = 0;
        last = tv;
    }
	else if ((tv.tv_sec == last.tv_sec) && (tv.tv_usec == last.tv_usec))
	{
        if (adjustment >= MAX_ADJUSTMENT)
            goto try_again;
        adjustment++;
    }
	else
	{
        adjustment = 0;
        last = tv;
    }
        
    clock_reg = tv.tv_usec*10 + adjustment;
    clock_reg += ((UT_uint64) tv.tv_sec)*10000000;
    clock_reg += (((UT_uint64) 0x01B21DD2) << 32) + 0x13814000;

    clock_high = (UT_uint32)(clock_reg >> 32);
    clock_low = (UT_uint32)clock_reg;
    ret_clock_seq = clock_seq;
    return 0;
}

void UT_UUID::_makeUUID(uuid &uu) const
{
    static UT_sint32 has_init = 0;
    UT_uint32  clock_mid;

    if(!s_bInitDone)
	{
        if(!UT_getEthernetAddress(s_node))
		{
            _getRandomBytes(s_node, 6);
            /*
             * Set multicast bit, to prevent conflicts
             * with IEEE 802 addresses obtained from
             * network cards
             */
            s_node[0] |= 0x80;
        }
        s_bInitDone = true;
    }
	
    _getClock(clock_mid, uu.time_low, uu.clock_seq);
	
    uu.clock_seq |= 0x8000;
    uu.time_mid = (UT_uint16) clock_mid;
    uu.time_hi_and_version = (clock_mid >> 16) | 0x1000;
    memcpy(uu.node, s_node, 6);
}
