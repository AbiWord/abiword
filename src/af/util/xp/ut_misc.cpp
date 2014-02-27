/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <glib.h>

#include "ut_string_class.h"
#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_std_string.h"
#include "ut_go_file.h"
#include "ut_debugmsg.h"

//#include "fl_AutoLists.h" // need definition of AUTO_LIST_RESERVED
#define AUTO_LIST_RESERVED 1000
#include <limits.h>

/*****************************************************************/
/*****************************************************************/

UT_Rect::UT_Rect()
{
	left = top = height = width = 0;
}

UT_Rect::UT_Rect(UT_sint32 iLeft, UT_sint32 iTop, UT_sint32 iWidth, UT_sint32 iHeight)
{
	left = iLeft;
	top = iTop;
	width = iWidth;
	height = iHeight;
}

UT_Rect::UT_Rect(const UT_Rect & r)
{
	left = r.left;
	top = r.top;
	width = r.width;
	height = r.height;
}


UT_Rect::UT_Rect(const UT_Rect * r)
{
	left = r->left;
	top = r->top;
	width = r->width;
	height = r->height;
}

bool UT_Rect::containsPoint(UT_sint32 x, UT_sint32 y) const
{
	// return true iff the given (x,y) is inside the rectangle.

	if ((x < left) || (x >= left+width))
		return false;
	if ((y < top) || (y >= top+height))
		return false;

	return true;
}

void UT_Rect::set(int iLeft, int iTop, int iWidth, int iHeight)
{
	left = iLeft;
	top = iTop;
	width = iWidth;
	height = iHeight;
}

/*!
 * This method makes a union of the current rectangle with the one in the 
 * parameter list. This rectangle is the smallest one that covers both
 * rectangles.
 */
void UT_Rect::unionRect(const UT_Rect * pRect)
{
	UT_sint32 fx1,fx2,fy1,fy2;
	fx1 = UT_MIN(left,pRect->left);
	fx2 = UT_MAX(left+width,pRect->left + pRect->width);
	fy1 = UT_MIN(top,pRect->top);
	fy2 = UT_MAX(top+height,pRect->top + pRect->height);
	left = fx1;
	width = fx2 - fx1;
	top = fy1;
	height = fy2 - fy1;
}

bool UT_Rect::intersectsRect(const UT_Rect * pRect) const
{
	// return true if this rectangle and pRect intersect.

#define R_RIGHT(pr)		(((pr)->left)+((pr)->width))
#define R_BOTTOM(pr)	(((pr)->top)+((pr)->height))
	
	if (R_RIGHT(pRect) < left)
		return false;

	if (R_RIGHT(this) < pRect->left)
		return false;

	if (R_BOTTOM(pRect) < top)
		return false;

	if (R_BOTTOM(this) < pRect->top)
		return false;

	return true;

#undef R_RIGHT
#undef R_BOTTOM
}

// Returns the suffix (including the dot) of the filename designated by `path`. 
// Path can be a local filename or an URI. Note that you can't pass this 
// function unix-style local filenames on Windows, nor the other way around.
// Returns an empty string if no suffix is found or an error occurred.
std::string UT_pathSuffix(std::string path)
{
	if (path.size() == 0)
		return "";

	bool isUri = UT_go_path_is_uri(path.c_str());
	bool isFilename = isUri ? false : path.find_last_of(G_DIR_SEPARATOR) == std::string::npos;
	
	// If 'path' is no URI but also not a filename, then it must be a
	// local path. If so, then we can convert it into a proper URI
	if (!isUri && !isFilename)
	{
		char* uri = g_filename_to_uri(path.c_str(), NULL, NULL);
		if (!uri)
			return "";
		path = uri;
		FREEP(uri);
	}

	// This algorithm is pretty simple: we search for a dot, and if the
	// dot happens AFTER the last slash (in the case of an URI), we consider
	// the stuff beyond the dot (in the forward direction) the extension.

	size_t slashpos = 0;
	if (!isFilename)
	{
		slashpos = path.find_last_of('/');
		slashpos++; // strip the leading / as well
	}

	size_t dotpos = path.find_last_of('.');
	if (dotpos == std::string::npos)
		return "";
	if (dotpos <= slashpos) return "";

	return std::string(path, dotpos, path.size() - dotpos);
}

/*!
 * Take a path name and search for the suffix. replace it the supplied 
 * suffix.
 */
bool UT_addOrReplacePathSuffix(std::string & sPath, const char * sSuffix)
{
  UT_sint32 i = sPath.length() - 1;
  const char *sSlash = "/";
  const char *sBSlash = "\\";
  const char *sDot = ".";
  std::string s = sPath.substr(i,1);
  while(i>0 &&  (s != sDot) && (s!= sBSlash) && (s!=sSlash))
  {
      i -=1;
      s = sPath.substr(i,1);
  }
  if((s == sSlash) || (s == sBSlash) || (i<=0))
  {
      sPath += sSuffix;
  }
  else
  {
      std::string sLeader = sPath.substr(0,i);
      sPath = sLeader;
      sPath += sSuffix;
  }
  return true;
}
 

bool UT_isWordDelimiter(UT_UCSChar currentChar, UT_UCSChar followChar, UT_UCSChar prevChar)
{
	// fast track Ascii letters
	if('a' <= currentChar && currentChar <= 'z')
		return false;

	if('A' <= currentChar && currentChar <= 'Z')
		return false;

	switch (g_unichar_type(currentChar))
	{
		case G_UNICODE_MODIFIER_LETTER:
		case G_UNICODE_LOWERCASE_LETTER:
		case G_UNICODE_TITLECASE_LETTER:
		case G_UNICODE_UPPERCASE_LETTER:
		case G_UNICODE_OTHER_LETTER:
		case G_UNICODE_COMBINING_MARK:
		case G_UNICODE_ENCLOSING_MARK:
		case G_UNICODE_NON_SPACING_MARK:
		case G_UNICODE_DECIMAL_NUMBER:
		case G_UNICODE_LETTER_NUMBER:
		case G_UNICODE_OTHER_NUMBER:
			return false;
		case G_UNICODE_CONNECT_PUNCTUATION:
			return (currentChar == '_'); // _ is a word separator!

		case G_UNICODE_OTHER_PUNCTUATION:
		case G_UNICODE_INITIAL_PUNCTUATION:
		case G_UNICODE_FINAL_PUNCTUATION:
			switch (currentChar)
			{
				// some punctuation can be internal in word
				case 0x0022:           // QUOTATION MARK
				case 0x0027:           // APOSTROPHE
				case UCS_LDBLQUOTE:    // smart quote, open double /* wjc */
				case UCS_RDBLQUOTE:    // smart quote, close double /* wjc */
				case UCS_LQUOTE:       // smart quote, open single  /* wjc */
				case UCS_RQUOTE:	   // smart quote, close single
				case 0x055F:           // ARMENIAN ABBREVIATION MARK
				case 0x070A:           // SYRIAC CONTRACTION
				case 0x070F:           // SYRIAC ABBREVIATION MARK
				case 0x0970:           // DEVANAGARI ABBREVIATION SIGN
					if (UT_UCS4_isalpha(followChar) &&
						UT_UCS4_isalpha(prevChar))
						return false;
					else
						return true;
					
				default:
					return true;
			}
			
		default:
			return true;
	} // switch
}

gchar ** UT_cloneAndDecodeAttributes (const gchar ** attrs)
{
    UT_UTF8String s;
    UT_uint32 count = 0;
    const gchar ** p = attrs;

    while (*p)
    {
	count++;
	p++;
    }

    UT_return_val_if_fail(count % 2 == 0, NULL);

    gchar ** attrs2 =
	(gchar **) UT_calloc (count + 1, sizeof (gchar*));

    UT_uint32 i;
    for (i = 0; i < count; i++)
    {
	s = attrs[i];
	s.decodeXML();
	attrs2[i] = g_strdup(s.utf8_str());
    }

    attrs2[i] = NULL;

    return attrs2;
}

const gchar* UT_getAttribute( const gchar* name,
                              const gchar** atts, const gchar* def )
{
    const gchar* p = UT_getAttribute ( name, atts );
    return p ? p : def;
}

const gchar* UT_getAttribute(const gchar* name, const gchar** atts)
{
	UT_return_val_if_fail( atts, NULL );

	const gchar** p = atts;

	while (*p)
	{
		if (0 == strcmp(static_cast<const char*>(p[0]), static_cast<const char*>(name)))
			break;
		p += 2;
	}

	if (*p)
		return p[1];
	else
		return NULL;
}

bool isTrue( const char* s )
{
    if( !s )
        return false;
    if( !strcmp(s,"0"))
        return false;
    if( !strcmp(s,"false"))
        return false;
    return true;
}

//////////////////////////////////////////////////////////////////

UT_sint32 signedLoWord(UT_uint32 dw)
{
	// return low word as a signed quantity

	unsigned short u16 = static_cast<unsigned short>(dw & 0xffff);
	signed short   s16 = *reinterpret_cast<signed short *>(&u16);
	UT_sint32      s32 = s16;

	return s32;
}

UT_sint32 signedHiWord(UT_uint32 dw)
{
	// return high word as a signed quantity

	unsigned short u16 = static_cast<unsigned short>((dw >> 16) & 0xffff);
	signed short   s16 = *reinterpret_cast<signed short *>(&u16);
	UT_sint32      s32 = s16;

	return s32;
}

//////////////////////////////////////////////////////////////////

/*!
 * simplesplit splits the referring string along the character 'separator',
 * removing the separator character, and placing the resulting strings in a
 * vector.
 */
std::vector<std::string> * simpleSplit (const std::string & str, char separator)
{
	std::vector<std::string> * utvResult = new std::vector<std::string>();
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
			utvResult->push_back(utsEntry);
		}
	}

	return utvResult;
}

/*!
 * Strips off the first numeric part of string and returns it as a uint32.
 * ie. "Numbered Heading 5" would return 5.
 */
UT_uint32 UT_HeadingDepth(const char * szHeadingName)
{
	std::string sz;
	UT_uint32 i = 0;
	bool bFound = false;
	for(i=0; i < strlen(szHeadingName) ; i++)
	{
		if(szHeadingName[i] >= '0' && szHeadingName[i] <= '9')
		{
			sz += szHeadingName[i];
			bFound = true;
		}
		else if(bFound)
		{
			break;
		}
	}
	i = static_cast<UT_uint32>(atoi(sz.c_str()));
	return i;
}

void * UT_calloc ( UT_uint32 nmemb, UT_uint32 size )
{
  return g_try_malloc0 ( nmemb * size ) ;
}


//////////////////////////////////////////////////
UT_UniqueId::UT_UniqueId()
{
	memset(m_iID,0,sizeof(m_iID));

	UT_uint32 i = (UT_uint32) List;
	m_iID[i] = AUTO_LIST_RESERVED;
}


/*!
    returns unique id (0 <= id < UT_UID_INVALID) of given type or
    UT_UID_INVALID on failure
*/
UT_uint32 UT_UniqueId::getUID(idType t)
{
	UT_return_val_if_fail(t < _Last, UT_UID_INVALID);
	UT_uint32 i = (UT_uint32)t;
	UT_uint32 r = m_iID[i]++;
	return r;
}

/*!
    sets the minimum id to be returned by subsequent calls to getUID()
    to iMin and returns true on success; returns false if it fails
    (either because unknown type is specified or because the value of
    iMin is too high)
*/
bool UT_UniqueId::setMinId(idType t, UT_uint32 iMin)
{
	UT_return_val_if_fail(t < _Last, false);

	// we really want some space left to generate future id's
	UT_return_val_if_fail(iMin < UINT_MAX - 1000, false);
	
	UT_uint32 i = (UT_uint32) t;

	if(m_iID[i] > iMin)
		return false;
	
	m_iID[i] = iMin;
	return true;
}

/*!
    returns true of iId can be used as a unique identifier of a type
    t; before using iId, THE CALLER MUST CALL setMinId(t, iId+1) !!!
*/
bool UT_UniqueId::isIdUnique(idType t, UT_uint32 iId) const
{
	UT_return_val_if_fail(t < _Last, false);

	// we really want some space left to generate future id's
	UT_return_val_if_fail(iId < UINT_MAX - 1000, false);
	
	UT_uint32 i = (UT_uint32) t;

	if(m_iID[i] > iId)
		return false;
	
	return true;
}

/**
 * UT_parseBool 
 *
 * Returns true if param is [true, 1, yes, allow, enable, on]
 * Returns false if param is [false, 0, no, disallow, disable, off]
 * Returns dfl otherwise, including if param is null
 */
bool UT_parseBool (const char * param, bool dfl)
{
	UT_return_val_if_fail (param && strlen(param), dfl);

	if (!g_ascii_strncasecmp(param, "true", 4) || !g_ascii_strncasecmp(param, "1", 1) ||
		!g_ascii_strncasecmp(param, "yes", 3) || !g_ascii_strncasecmp(param, "allow", 5) ||
		!g_ascii_strncasecmp(param, "enable", 6) || !g_ascii_strncasecmp(param, "on", 2))
		return true;
	else if (!g_ascii_strncasecmp(param, "false", 5) || !g_ascii_strncasecmp(param, "0", 1) ||
			 !g_ascii_strncasecmp(param, "no", 2) || !g_ascii_strncasecmp(param, "disallow", 8) ||
			 !g_ascii_strncasecmp(param, "disable", 7) || !g_ascii_strncasecmp(param, "off", 3))
		return false;
	
	return dfl;
}

void UT_VersionInfo::makeVersString()
{
	m_versString = UT_std_string_sprintf("%d.%d.%d.%d",
					     m_iMajor, m_iMinor,
					     m_iMicro, m_iNano);
}

const gchar ** UT_setPropsToNothing(const gchar ** props)
{
	if(!props)
		return NULL;
	
	const gchar ** props2;

	UT_uint32 iCount  = 0;
									
	while(props[iCount])
		iCount += 2;

									
	props2 = new const gchar * [iCount+1];

	UT_uint32 i;
	for(i = 0; i < iCount; i += 2)
	{
		props2[i] = props[i];
		props2[i+1] = NULL;
	}

	props2[i] = NULL;

	return props2;
}

const gchar ** UT_setPropsToValue(const gchar ** props, const gchar * value)
{
	if(!props)
		return NULL;
	
	const gchar ** props2;

	UT_uint32 iCount  = 0;
									
	while(props[iCount])
		iCount += 2;

									
	props2 = new const gchar * [iCount+1];

	UT_uint32 i;
	for(i = 0; i < iCount; i += 2)
	{
		props2[i] = props[i];
		props2[i+1] = value;
	}

	props2[i] = NULL;

	return props2;
}

/*!
   splits the xml property string (font-size:24pt;font-face:Arial') into names and values
   and stores them in an array

   the caller has to delete[] the array; the process is destructive to props
*/
const gchar ** UT_splitPropsToArray(gchar * pProps)
{
		UT_return_val_if_fail( pProps, NULL);
	
		UT_uint32 iLen = strlen(pProps);
	
		UT_uint32 i = 1; // *props != 0 => at least one
		if(pProps[iLen-1] == ';')
		{
			// trailing ;
			--i;
		}

		char * semi = NULL;
		const char * p = pProps;
 		while((semi = (char *) strchr(p, ';')))
		{
			*semi = 0;
			p = semi + 1;
			i++;
		}
	
	
		UT_uint32 iPropCount = i;
		UT_uint32 j = 0;
		const gchar ** pPropsArray = new const gchar *[2 * iPropCount + 1];
		UT_return_val_if_fail( pPropsArray, NULL );
	
		const char * pStart = pProps;

		// we want to include the 0-terminator
		for(i = 0; i <= iLen; i++)
		{
			if(pProps[i] == 0)
			{
				pPropsArray[j++] = pStart;
				char * colon = (char *)  strchr(pStart, ':');
				UT_return_val_if_fail( colon,NULL );
				*colon = 0;
				pPropsArray[j++] = colon + 1;

				if(i == iLen)
					break;
				
				pStart = pProps + i + 1;
				while(isspace(*pStart))
					pStart++;
			}
		}
	
		UT_return_val_if_fail( j == 2 * iPropCount, NULL );

		pPropsArray[j] = NULL;
		return pPropsArray;
}


#if defined(WIN32) && !defined(__GNUC__)	
#   define MYZERO 0
#else
#   define MYZERO 0LL
#endif

UT_uint64 UT_hash64(const char * p, UT_uint32 bytelen)
{
	UT_return_val_if_fail( p, MYZERO );
	
	if(!bytelen)
	{
		bytelen = strlen(p);
	}

	UT_return_val_if_fail( bytelen, MYZERO );
	
	UT_uint64 h = (UT_uint64)*p;
	
	for (UT_uint32 i = 1; i < bytelen; ++i, ++p)
	{
		h = (h << 5) - h + *p;
	}

	return h;
}

UT_uint32 UT_hash32(const char * p, UT_uint32 bytelen)
{
	UT_return_val_if_fail( p, 0 );
	
	if(!bytelen)
	{
		bytelen = strlen(p);
	}

	UT_return_val_if_fail( bytelen, 0 );

	UT_uint32 h = (UT_uint32)*p;
	
	for (UT_uint32 i = 1; i < bytelen; ++i, ++p)
	{
		h = (h << 5) - h + *p;
	}

	return h;
}

#undef MYZERO






