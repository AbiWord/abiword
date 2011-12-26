/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2009 Hubert Figuiere
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

#include "ut_assert.h"
#include "ut_std_string.h"
#include "ut_string.h"

#if 0
std::string & UT_escapeXML(std::string &s)
{
    gsize incr = 0;

	const char * ptr = s.c_str();
	while (*ptr) {
        if ((*ptr == '<') || (*ptr == '>')) {
            incr += 3;
        }
        else if (*ptr == '&') {
            incr += 4;
        }
        else if (*ptr == '"') {
            incr += 5;
        }
        ptr++;
    }

    gsize slice_size = s.size() + incr + 1;
    char * dest = (char *)g_slice_alloc(slice_size);
    char * current = dest;

	ptr = s.c_str();
	while (*ptr)
    {
        if (*ptr == '<')
        {
            memcpy(dest, "&lt;", 4);
            current += 4;
        }
        else if (*ptr == '>')
        {
            memcpy(dest, "&gt;", 4);
            current += 4;
        }
        else if (*ptr == '&')
        {
            memcpy(dest, "&amp;", 5);
            current += 5;
        }
        else if (*ptr == '"')
        {
            memcpy(dest, "&quot;", 6);
            current += 6;
        }
        ptr++;
    }
    *dest = 0;
    s = dest;
    g_slice_free1(slice_size, dest);
    return s;
}
#endif

 
std::string& UT_std_string_vprintf (std::string & inStr, const char *format,
                                    va_list      args1)
{
    char *buffer = g_strdup_vprintf(format, args1);
    inStr = buffer;
    g_free(buffer);

    return inStr;
}


std::string UT_std_string_sprintf(const char * inFormat, ...)
{
    std::string outStr;

    va_list args;
    va_start (args, inFormat);
    UT_std_string_vprintf (outStr, inFormat, args);
    va_end (args);

    return outStr;
}


bool ends_with( const std::string& s, const std::string& ending )
{
    if( ending.length() > s.length() )
        return false;
    
    return s.rfind(ending) == (s.length() - ending.length());
}

bool starts_with( const std::string& s, const std::string& starting )
{
    int starting_len = starting.length();
    int s_len = s.length();

    if( s_len < starting_len )
        return false;
    
    return !s.compare( 0, starting_len, starting );
}

std::string replace_all( const std::string& s, char oldc, char newc )
{
    std::string ret;
    for( std::string::const_iterator iter = s.begin(); iter != s.end(); ++iter )
    {
        if( *iter == oldc ) ret += newc;
        else                ret += *iter;
    }
    return ret;
}

std::string replace_all( const std::string& s,
                         const std::string& olds,
                         const std::string& news )
{
    std::string ret = s;
    int olds_length = olds.length();
    int news_length = news.length();
            
    std::string::size_type start = ret.find( olds );
    while( start != std::string::npos )
    {
        ret.replace( start, olds_length, news );
        start = ret.find( olds, start + news_length );
    }
    return ret;
}


std::string UT_XML_cloneNoAmpersands( const std::string& src )
{
    gchar* rszDest = 0;
    
    bool rc = UT_XML_cloneNoAmpersands( rszDest, src.c_str() );
    if( !rc )
        return src;

    std::string ret = rszDest;
    FREEP(rszDest);
    return ret;
}


/*!
 * Assuming a string of standard abiword properties eg. "fred:nerk; table-width:1.0in; table-height:10.in"
 * Return the value of the property sProp or NULL if it is not present.
 */
std::string UT_std_string_getPropVal(const std::string & sPropertyString, const std::string & sProp)
{
	std::string sWork(sProp);
	sWork += ":";

	const char * szWork = sWork.c_str();
	const char * szProps = sPropertyString.c_str();
	const char * szLoc = strstr(szProps,szWork);
	if(szLoc == NULL)
	{
		return std::string();
	}
//
// Look if this is the last property in the string.
//
	const char * szDelim = strchr(szLoc,';');
	if(szDelim == NULL)
	{
//
// Remove trailing spaces
//
		UT_sint32 iSLen = strlen(szProps);
		while(iSLen > 0 && szProps[iSLen-1] == ' ')
		{
			iSLen--;
		}
//
// Calculate the location of the substring
//
		UT_sint32 offset = static_cast<UT_sint32>(reinterpret_cast<size_t>(szLoc) - reinterpret_cast<size_t>(szProps));
		offset += strlen(szWork);
		return sPropertyString.substr(offset,(iSLen - offset));
	}
	else
	{
		szDelim = strchr(szLoc,';');
		if(szDelim == NULL)
		{
//
// bad property string
//
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return std::string();
		}
//
// Remove trailing spaces.
//
		while(*szDelim == ';' || *szDelim == ' ')
		{
			szDelim--;
		}
//
// Calculate the location of the substring
//
		UT_sint32 offset = static_cast<UT_sint32>(reinterpret_cast<size_t>(szLoc) - reinterpret_cast<size_t>(szProps));
		offset += strlen(szWork);
		UT_sint32 iLen = static_cast<UT_sint32>(reinterpret_cast<size_t>(szDelim) - reinterpret_cast<size_t>(szProps)) + 1;
		return sPropertyString.substr(offset,(iLen - offset));
	}
}

/*!
 * Assuming a string of standard abiword properties eg. "fred:nerk; table-width:1.0in; table-height:10.in"
 * Add the property sProp with value sVal to the string of properties. If the property is already present, replace the 
 * old value with the new value.
 */
void UT_std_string_setProperty(std::string & sPropertyString, const std::string &sProp, const std::string & sVal)
{
//
// Remove the old value if it exists and tack the new property on the end.
//
	UT_std_string_removeProperty(sPropertyString, sProp);
	if(!sPropertyString.empty())
	{
		sPropertyString += "; ";
	}
	sPropertyString += sProp;
	sPropertyString += ":";
	sPropertyString += sVal;
}


/*!
 * Assuming a string of standard abiword properties eg. "fred:nerk; table-width:1.0in; table-height:10.in"
 * Remove the property sProp and it's value from the string of properties. 
 */
void UT_std_string_removeProperty(std::string & sPropertyString, const std::string & sProp)
{
	std::string sWork ( sProp );
	sWork += ":";
	const char * szWork = sWork.c_str();
	const char * szProps = sPropertyString.c_str();
	const char * szLoc = strstr(szProps,szWork);
	if(szLoc == NULL)
	{
//
// Not here, do nothing
		return ;
	}
//
// Found it, Get left part.
//
	UT_sint32 locLeft = static_cast<UT_sint32>(reinterpret_cast<size_t>(szLoc) - reinterpret_cast<size_t>(szProps));
	std::string sLeft;
	if(locLeft == 0)
	{
		sLeft.clear();
	}
	else
	{
		sLeft = sPropertyString.substr(0,locLeft);
	}
	locLeft = static_cast<UT_sint32>(sLeft.size());
	if(locLeft > 0)
	{
//
// If this element is the last item in the properties there is no "; ".
//
// Remove trailing ';' and ' '
//
		locLeft--;
		while(locLeft >= 0 && (sLeft[locLeft] == ';' || sLeft[locLeft] == ' '))
		{
			locLeft--;
		}
	}
	std::string sNew;
	if(locLeft > 0)
	{
		sNew = sLeft.substr(0,locLeft+1);
	}
	else
	{
		sNew.clear();
	}
//
// Look for ";" to get right part
//
	const char * szDelim = strchr(szLoc,';');
	if(szDelim == NULL)
	{
//
// No properties after this, just assign and return
//
		sPropertyString = sNew;
	}
	else
	{
//
// Just slice off the properties and tack them onto the pre-existing sNew
//
		while(*szDelim == ';' || *szDelim == ' ')
		{
			szDelim++;
		}
		UT_sint32 offset = static_cast<UT_sint32>(reinterpret_cast<size_t>(szDelim) - reinterpret_cast<size_t>(szProps));
		UT_sint32 iLen = sPropertyString.size() - offset;
		if(sNew.size() > 0)
		{
			sNew += "; ";
		}
		sNew += sPropertyString.substr(offset,iLen);
		sPropertyString = sNew;
	}
}
