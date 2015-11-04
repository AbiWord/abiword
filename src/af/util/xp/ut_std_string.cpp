/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2009-2015 Hubert Figuiere
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

#include <string.h>

#include "ut_assert.h"
#include "ut_std_string.h"
#include "ut_string.h"
#include "ut_debugmsg.h"

#include <iostream>
#include <sstream>
#include <list>

std::string UT_escapeXML(const std::string &s)
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
  while (*ptr) {
    if (*ptr == '<') {
      memcpy(current, "&lt;", 4);
      current += 4;
    }
    else if (*ptr == '>') {
      memcpy(current, "&gt;", 4);
      current += 4;
    }
    else if (*ptr == '&') {
      memcpy(current, "&amp;", 5);
      current += 5;
    }
    else if (*ptr == '"') {
      memcpy(current, "&quot;", 6);
      current += 6;
    }
    else {
      *current = *ptr;
      current++;
    }
    ptr++;
  }
  *current = 0;
  std::string result = dest;
  g_slice_free1(slice_size, dest);
  return result;
}

 
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

std::string UT_std_string_unicode(const UT_UCS4Char * unicode,
                                  UT_uint32 len)
{
    if (unicode == NULL || len == 0) {
        return std::string();
    }

    GError *error = NULL;
    gchar * utf8 = g_ucs4_to_utf8(unicode, len, NULL, NULL, &error);
    if (!utf8) {
        UT_DEBUGMSG(("Error converting UCS4 to UTF8: %s\n", error->message));
        g_error_free(error);
        return std::string();
    }
    std::string s = utf8;
    g_free(utf8);
    return s;
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
 * simplesplit splits the referring string along the character 'separator',
 * removing the separator character, and placing the resulting strings in a
 * vector.
 */
std::vector<std::string> UT_simpleSplit (const std::string & str, char separator)
{
	std::vector<std::string> v;
	UT_uint32 start = 0;

	for(size_t j = 0; start < str.size(); j++)
	{
		std::string utsEntry;

		for (; (str[start] != separator) && start < str.size(); start++) {
			utsEntry += str[start];
		}
		start++;						// skipping over the separator character
										// itself
		if (!utsEntry.empty()) {
			v.push_back(utsEntry);
		}
	}

	return v;
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
	    //Not here, do nothing
	    return ;
	}
	// Check if this is a real match
	if (szLoc != szProps)
	{
		// This is not the first property. It could be a false match
		// for example, 'frame-col-xpos' and 'xpos'
		std::string sWorkCheck("; ");
		sWorkCheck += sWork;
		const char * szLocCheck = strstr(szProps,sWorkCheck.c_str());
		if (!szLocCheck)
		{
			// False match
			return;
		}
		szLoc = szLocCheck;
	}	    

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

	std::string sNew;
	if(locLeft > 0)
	{
		sNew = sLeft.substr(0,locLeft+1);
	}
	else
	{
		sNew.clear();
	}

	// Look for ";" to get right part

	const char * szDelim = strchr(szLoc,';');
	if(szDelim == NULL)
	{
		// No properties after this, just assign and return
		sPropertyString = sNew;
	}
	else
	{
		// Just slice off the properties and tack them onto the pre-existing sNew
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



const std::string StreamToString( std::istream& iss )
{
    std::stringstream ss;
    iss.clear();
    std::copy( std::istreambuf_iterator<char>(iss),
               std::istreambuf_iterator<char>(),
               std::ostreambuf_iterator<char>(ss));
    return ss.str();
}

std::string toTimeString( time_t TT )
{
    const int bufmaxlen = 1025;
    char buf[bufmaxlen];
    struct tm* TM = 0;
    std::string format = "%y %b %e %H:%M";

//    TM = gmtime( &TT );
    TM = localtime( &TT );
            
    if( TM && strftime( buf, bufmaxlen, format.c_str(), TM) )
    {
        std::string s = buf;
        return s;
    }
    // FIXME
    return "";
}

time_t toTime( struct tm *tm )
{
    return mktime( tm );
}
time_t parseTimeString( const std::string& stddatestr )
{
    const char* datestr = stddatestr.c_str();
    const char* eos     = datestr + strlen( datestr );

    typedef std::list<std::string> formats_t;
    formats_t formats;
    
    formats.push_back( "%Y-%m-%dT%H:%M:%S" );
    formats.push_back( "%y %b %d %H:%M:%S" );
    formats.push_back( "%y %b %d %H:%M" );

    for( formats_t::iterator iter = formats.begin(); iter != formats.end(); ++iter )
    {
        std::string format = *iter;
        struct tm tm;
        memset( &tm, 0, sizeof(struct tm));
        const char* rc = UT_strptime( datestr, format.c_str(), &tm );
        if( rc == eos )
        {
//            UT_DEBUGMSG(("parseTimeString(OK) input:%s format:%s ret:%ld\n",
//                         datestr, format.c_str(), toTime(&tm) ));
            return toTime(&tm);
        }
    }

//    UT_DEBUGMSG(("parseTimeString(f) input:%s\n", datestr ));
    return 0;
}
