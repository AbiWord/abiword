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

#ifndef UTCOLOR_H
#define UTCOLOR_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif


#include <string>

// ----------------------------------------------------------------
#define UT_RGBCOLOR_PROXIMITY 45

class ABI_EXPORT UT_ColorPatImpl
{
public:
    virtual ~UT_ColorPatImpl();
    virtual UT_ColorPatImpl * clone() const= 0;
};

class ABI_EXPORT UT_RGBColor
{
public:
	UT_RGBColor();
	UT_RGBColor(unsigned char, unsigned char, unsigned char, bool bTransparent = false);
	UT_RGBColor(const UT_RGBColor&);
    // take ownership
    UT_RGBColor(const UT_ColorPatImpl * pattern);
    ~UT_RGBColor();
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

    UT_RGBColor & operator=(const  UT_RGBColor &inc);

	bool isTransparent() const
	{
		return m_bIsTransparent;
	}
	bool setColor(const char * pszColor);

    bool isPattern() const
    {
        return m_patImpl != NULL;
    }
    const UT_ColorPatImpl *pattern() const
    {
        return m_patImpl;
    }
	// take ownership
	void setPattern(const UT_ColorPatImpl *p)
	{
		if(m_patImpl) {
			delete m_patImpl;
		}
		m_patImpl = p;
	}

	unsigned char m_red;
	unsigned char m_grn;
	unsigned char m_blu;
	bool m_bIsTransparent;
private:
    const UT_ColorPatImpl * m_patImpl;
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

	const char * c_str() const
	{ return m_colorBuffer; }
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


// Hack so we get AbiNativeWidget with an xp include
#ifdef TOOLKIT_GTK_ALL
#include "ut_unixColor.h"
#else
// TODO maintainers please fix their platform
typedef void AbiNativeWidget;
#endif

#endif /* UTCOLOR_H */
