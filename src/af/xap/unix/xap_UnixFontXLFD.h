/* AbiSource Application Framework
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

#ifndef XAP_UNIXFONTXLFD_H
#define XAP_UNIXFONTXLFD_H

#include "ut_types.h"
#include "ut_string.h"

/*
  This XLFD class provides an object representation of an X
  Logical Font Descriptor (XLFD).  It exists for convenient access
  to individual fields of the string.  It can be fed a string to
  store, or it can be created empty and each field individually
  set.  It can be queried for an XLFD of its contents or its
  individual settings.  Feel free to add more functionality
  to suit.
*/

/*****************************************************************/

class XAP_UnixFontXLFD
{
public:

	XAP_UnixFontXLFD(void);
	XAP_UnixFontXLFD(const char * xlfd);
	XAP_UnixFontXLFD(XAP_UnixFontXLFD & copy);
	
	~XAP_UnixFontXLFD(void);

	// pass and receive full XLFDs to set/get contents
	void				setXLFD(const char * xlfd);
	char * 				getXLFD(void);	// caller must free result
	
	// pass individual fields to set contents
	void				setFoundry(const char * foundry) { UT_replaceString(m_foundry, foundry); };
	const char *		getFoundry(void) { return (m_foundry) ? m_foundry : "*"; };
	
	void				setFamily(const char * family) { UT_replaceString(m_family, family); };
	const char *		getFamily(void) { return (m_family) ? m_family : "*"; };

	void				setWeight(const char * weight) { UT_replaceString(m_weight, weight); };
	const char *		getWeight(void) { return (m_weight) ? m_weight : "*"; };

	void				setSlant(const char * slant) { UT_replaceString(m_slant, slant); };
	const char *		getSlant(void) { return (m_slant) ? m_slant : "*"; };

	void				setWidth(const char * width) { UT_replaceString(m_width, width); };
	const char *		getWidth(void) { return (m_width) ? m_width : "*"; };

	void				setAdStyle(const char * adStyle) { UT_replaceString(m_adStyle, adStyle); };
	const char *		getAdStyle(void) { return (m_adStyle) ? m_adStyle : "*"; };

	void				setPixelSize(UT_uint32 pixelSize) { m_pixelSize = pixelSize; };
	UT_uint32			getPixelSize(void) { return m_pixelSize; };

	void				setPointSize(UT_uint32 pointSize) { m_deciPointSize = pointSize * 10; };
	UT_uint32			getPointSize(void) { return m_deciPointSize / 10; };

	void				setDeciPointSize(UT_uint32 deciPointSize) { m_deciPointSize = deciPointSize; };
	UT_uint32			getDeciPointSize(void) { return m_deciPointSize; };
	
	void				setResX(UT_uint32 resX) { m_resX = resX; };
	UT_uint32			getResX(void) { return m_resX; };
	
	void				setResY(UT_uint32 resY) { m_resY = resY; };
	UT_uint32			getResY(void) { return m_resY; };
	
	void				setSpacing(const char * spacing) { UT_replaceString(m_spacing, spacing); };
	const char *		getSpacing(void) { return (m_spacing) ? m_spacing : "*"; };

	void				setAverageWidth(UT_uint32 averageWidth) { m_averageWidth = averageWidth; };
	UT_uint32			getAverageWidth(void) { return m_averageWidth; };
	
	void				setRegistry(const char * registry) { UT_replaceString(m_registry, registry); };
	const char *		getRegistry(void) { return (m_registry) ? m_registry : "*"; }

	void				setEncoding(const char * encoding) { UT_replaceString(m_encoding, encoding); };
	const char *		getEncoding(void) { return (m_encoding) ? m_encoding : "*"; };

protected:

	void			_blankMembers(void);
	void 			_wipeMembers(void);
	
	char * 			m_foundry;
	char *			m_family;
	char *			m_weight;
	char *			m_slant;
	char *			m_width;
	char *			m_adStyle;
	UT_uint32		m_pixelSize;
	UT_uint32		m_deciPointSize;	// we store decipoints (points * 10) to feed to X server
	UT_uint32		m_resX;
	UT_uint32		m_resY;
	char *			m_spacing;
	UT_uint32		m_averageWidth;
	char *			m_registry;
	char *			m_encoding;
		
};

#endif /* XAP_UNIXFONTXLFD_H */
