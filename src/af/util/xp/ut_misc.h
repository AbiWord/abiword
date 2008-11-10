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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */
 


#ifndef UTMISC_H
#define UTMISC_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

#ifndef UTVECTOR_H
#include "ut_vector.h"
#endif

#include <string>

class UT_RGBColor;
class UT_Rect;
class UT_String;
class UT_UTF8String;

// ----------------------------------------------------------------
#define UT_RGBCOLOR_PROXIMITY 45

class ABI_EXPORT UT_RGBColor
{
public:
	UT_RGBColor();
	UT_RGBColor(unsigned char, unsigned char, unsigned char, bool bTransparent = false);
	UT_RGBColor(const UT_RGBColor&);
	bool operator != (const UT_RGBColor &op1)
	{
		return (op1.m_red != m_red || op1.m_grn != m_grn || op1.m_blu != m_blu);
	}

	bool operator == (const UT_RGBColor &op1)
	{
		return (op1.m_red == m_red && op1.m_grn == m_grn && op1.m_blu == m_blu);
	}
	
	// returns true if the two colors are near each other in the RGB space
	bool operator %= (const UT_RGBColor &op1)
	{
		UT_uint32 iDiff = abs(m_red - op1.m_red) + abs(m_grn - op1.m_grn) + abs(m_blu - op1.m_blu);
		return (iDiff < UT_RGBCOLOR_PROXIMITY);
	}

	UT_RGBColor & operator ^= (const UT_RGBColor &op1)
	{
		m_red ^= op1.m_red;
		m_grn ^= op1.m_grn;
		m_blu ^= op1.m_blu;
		return *this;
	}

	UT_RGBColor & operator += (const unsigned char inc)
	{
		m_red += inc;
		m_grn += inc;
		m_blu += inc;
		return *this;
	}

	UT_RGBColor & operator += (const  UT_RGBColor &inc)
	{
		m_red += inc.m_red;
		m_grn += inc.m_grn;
		m_blu += inc.m_blu;
		return *this;
	}

	UT_RGBColor & operator -= (const  UT_RGBColor &inc)
	{
		m_red -= inc.m_red;
		m_grn -= inc.m_grn;
		m_blu -= inc.m_blu;
		return *this;
	}

	inline bool isTransparent() const {return m_bIsTransparent;}
	bool setColor(const char * pszColor);

	unsigned char m_red;
	unsigned char m_grn;
	unsigned char m_blu;
	bool m_bIsTransparent;
	
};

void UT_setColor(UT_RGBColor & col, unsigned char r, unsigned char g, unsigned char b, bool bTransparent = false);
ABI_EXPORT void UT_parseColor(const char*, UT_RGBColor&);
ABI_EXPORT std::string UT_colorToHex(const char*, bool bPrefix = false);

class ABI_EXPORT UT_HashColor
{
private:
	char m_colorBuffer[8]; // format: "" or "#abc123" (i.e., '#' + 6 lower-case hex digits)

public:
	UT_HashColor ();
	~UT_HashColor ();

	/* The following 5 functions return a pointer to m_colorBuffer on success,
	 * or 0 on failure (invalid or unknown color).
	 */
	const char * setColor (unsigned char r, unsigned char g, unsigned char b);
	const char * setColor (const UT_RGBColor & color) { return setColor (color.m_red, color.m_grn, color.m_blu); }
	const char * setColor (const char * color); // try match hash (e.g., "#C01bB7") or name (e.g., "turquoise")
	const char * lookupNamedColor (const char * color_name); // "Orange" or "blue" or "LightGoldenRodYellow" or...
	const char * setHashIfValid (const char * color_hash);   // "ff0013" or "AD5FE6" or... (NOTE: no '#')

	const UT_RGBColor rgb (); // Call this *if* setColor () succeeds; otherwise defaults to black.
};

// ----------------------------------------------------------------
class ABI_EXPORT UT_Rect
{
public:
	UT_Rect();
	UT_Rect(UT_sint32 iLeft, UT_sint32 iTop, UT_sint32 iWidth, UT_sint32 iHeight);
	UT_Rect(const UT_Rect &);
	UT_Rect(const UT_Rect * r);

	bool containsPoint(UT_sint32 x, UT_sint32 y) const;
	void set(UT_sint32 iLeft, UT_sint32 iTop, UT_sint32 iWidth, UT_sint32 iHeight);
	bool intersectsRect(const UT_Rect * pRect) const;
	void unionRect( const UT_Rect *pRect);
	UT_sint32	left;
	UT_sint32	top;
	UT_sint32	width;
	UT_sint32	height;
};

// ----------------------------------------------------------------
struct ABI_EXPORT UT_Point
{
	UT_sint32	x;
	UT_sint32	y;
};

#define UT_MAX(A,B)	(((A) > (B)) ? (A) : (B))
#define UT_MIN(A,B)	(((A) < (B)) ? (A) : (B))
#define UT_ABS(A)	( ((A) < 0) ? (-(A)) : (A) )

const char * UT_pathSuffix(const char * path);
bool UT_isWordDelimiter(UT_UCSChar currentChar, UT_UCSChar followChar, UT_UCSChar prevChar);
ABI_EXPORT const gchar* UT_getAttribute(const gchar* name,
					   const gchar** atts);

ABI_EXPORT gchar ** UT_cloneAndDecodeAttributes (const gchar ** attrs);

UT_sint32 signedHiWord(UT_uint32 dw);
UT_sint32 signedLoWord(UT_uint32 dw);

UT_GenericVector<UT_String*> * simpleSplit (const UT_String & str, char separator = ' ',
						 size_t max = 0 /* 0 == full split */);

void warpString(UT_String& str, size_t col_max = 75);
UT_uint32 UT_HeadingDepth(const char * szHeadName);

/*
   The purpose of this class is to generate document-unique
   numerical identifiers; uniqueness is type-specific, i.e., id's of
   the different types generated by the class can overlap. Most of the
   time you probably will not want to use this class directly, but use
   the methods provided in PD_Document class.
*/

#define UT_UID_INVALID 0xffffffff
class ABI_EXPORT UT_UniqueId
{
  public:
	UT_UniqueId();
	~UT_UniqueId(){};
	
	enum idType
	{
		List = 0,
		Footnote,
		Endnote,
		HeaderFtr,
		Image,
		Math,
		Embed,

		/* must be last; for internal use only !*/
		_Last
	};
	
	UT_uint32 getUID(idType t);
	bool      setMinId(idType t, UT_uint32 iMin);
	bool      isIdUnique(idType t, UT_uint32 iId);

  private:
	UT_uint32 m_iID[(UT_uint32)_Last];
};

ABI_EXPORT bool UT_parseBool (const char * param, bool dfl);


#if 0
/*
   if your platform does not define timeval, turn on the following definitions
*/
#include <time.h>

typedef signed long suseconds_t;

struct timeval
{
	time_t           tv_sec;
	suseconds_t      tv_usec;
};
#endif

/*!
    UT_gettimeofday() fills in the timeval structure with current
    time; the platform implementation needs to be as accurate as
    possible since this function is used in the UT_UUID class.
 */
void UT_gettimeofday(struct timeval *tv);

typedef unsigned char UT_EthernetAddress[6];
/*!
    retrieve the 6-byte address of the network card; returns true on success
*/
bool UT_getEthernetAddress(UT_EthernetAddress &a);

class ABI_EXPORT UT_VersionInfo
{
  public:
	UT_VersionInfo(UT_uint32 maj, UT_uint32 min, UT_uint32 mic, UT_uint32 nan)
		: m_iMajor(maj), m_iMinor(min), m_iMicro(mic), m_iNano(nan) {};

	UT_VersionInfo()
		: m_iMajor(0), m_iMinor(0), m_iMicro(0), m_iNano(0) {};

	UT_uint32 getMajor() const {return m_iMajor;}
	UT_uint32 getMinor() const {return m_iMinor;}
	UT_uint32 getMicro() const {return m_iMicro;}
	UT_uint32 getNano()  const {return m_iNano;}

	void set(UT_uint32 maj, UT_uint32 min, UT_uint32 mic, UT_uint32 nan)
    	{m_iMajor = maj; m_iMinor = min; m_iMicro = mic; m_iNano = nan;}

	bool operator > (const UT_VersionInfo &v) const
	    {
			if(m_iMajor > v.m_iMajor)
				return true;
			if(m_iMajor < v.m_iMajor)
				return false;

			if(m_iMinor > v.m_iMinor)
				return true;
			if(m_iMinor < v.m_iMinor)
				return false;
			
			if(m_iMicro > v.m_iMicro)
				return true;
			if(m_iMicro < v.m_iMicro)
				return false;

			if(m_iNano > v.m_iNano)
				return true;
			if(m_iNano < v.m_iNano)
				return false;

			return false;
	    }

	const UT_UTF8String & getString() const;

	
  private:
	UT_uint32 m_iMajor;
	UT_uint32 m_iMinor;
	UT_uint32 m_iMicro;
	UT_uint32 m_iNano;
};

/*
   returns a copy of props which has all values (not names!) set to NULL; the caller must delete[] the returned pointer
   but not the contents

   this function reuses the property names from the original string, so if those are freed, the copy
   too becomes invalid !!!
*/
const gchar ** UT_setPropsToNothing(const gchar ** props);

const gchar ** UT_setPropsToValue(const gchar ** props, const gchar * value);

const gchar ** UT_splitPropsToArray(gchar * props);

UT_uint64 UT_hash64(const char * p, UT_uint32 bytelen = 0);
UT_uint32 UT_hash32(const char * p, UT_uint32 bytelen = 0);

// Hack so we get AbiNativeWidget with an xp include
#ifdef XP_UNIX_TARGET_GTK
#include "ut_unixMisc.h"
#else
// TODO maintainers please fix their platform
typedef void AbiNativeWidget;
#endif

#endif /* UTMISC_H */
