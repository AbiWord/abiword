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

#include <stdlib.h>
#include <glib.h>

#include "ut_types.h"
#include "ut_assert.h"

#include "xap_UnixFontXLFD.h"

#define REPLACE_ELSE_BLANK(dest, src)	\
                                        if (src && src[0]) 					\
                                        {									\
							                UT_replaceString(dest, src);	\
                                        }									\
                                        else								\
                                        {									\
	                                        UT_replaceString(dest, "*");	\
                                        }

/********************************************************************/

enum XLFDField
{
	// these are kinda bogus, since they start at 1, but that's
	// how g_strsplit() works out the leading delimeter in an
	// XLFD
	XLFD_FOUNDRY = 1, XLFD_FAMILY, XLFD_WEIGHT, XLFD_SLANT,
	XLFD_WIDTH, XLFD_ADSTYLE, XLFD_PIXELSIZE, XLFD_POINTSIZE,
	XLFD_RESOLUTIONX, XLFD_RESOLUTIONY, XLFD_SPACING,
	XLFD_AVERAGEWIDTH, XLFD_REGISTRY, XLFD_ENCODING
};

/********************************************************************/

XAP_UnixFontXLFD::XAP_UnixFontXLFD(void)
{
	_blankMembers();
	_wipeMembers();
}

XAP_UnixFontXLFD::XAP_UnixFontXLFD(const char * xlfd)
{
	_blankMembers();
	_wipeMembers();
	setXLFD(xlfd);
}

XAP_UnixFontXLFD::XAP_UnixFontXLFD(XAP_UnixFontXLFD & copy)
{
	UT_cloneString(m_foundry, copy.m_foundry);
	UT_cloneString(m_family, copy.m_family);
	UT_cloneString(m_weight, copy.m_weight);
	UT_cloneString(m_slant, copy.m_slant);
	UT_cloneString(m_width, copy.m_width);
	UT_cloneString(m_adStyle, copy.m_adStyle);
	m_pixelSize = copy.m_pixelSize;
	m_deciPointSize = copy.m_deciPointSize;
	m_resX = copy.m_resX;
	m_resY = copy.m_resY;
	UT_cloneString(m_spacing, copy.m_spacing);
	m_averageWidth = copy.m_averageWidth;
	UT_cloneString(m_registry, copy.m_registry);
	UT_cloneString(m_encoding, copy.m_encoding);
}

	
XAP_UnixFontXLFD::~XAP_UnixFontXLFD(void)
{
	_wipeMembers();
}


void XAP_UnixFontXLFD::setXLFD(const char * xlfd)
{
	UT_ASSERT(xlfd);
	
	gchar ** fields = g_strsplit(xlfd, "-", 14);
	UT_ASSERT(fields);

	// perhaps a better way to check this at run-time?
	UT_ASSERT(fields[0] && fields[1] && fields[2] && fields[3] && fields[4] &&
			  fields[5] && fields[6] && fields[7] && fields[8] && fields[9] &&
			  fields[10] && fields[11] && fields[12] && fields[13] && fields[14]);

	REPLACE_ELSE_BLANK(m_foundry, fields[XLFD_FOUNDRY]);
	REPLACE_ELSE_BLANK(m_family, fields[XLFD_FAMILY]);
	REPLACE_ELSE_BLANK(m_weight, fields[XLFD_WEIGHT]);
	REPLACE_ELSE_BLANK(m_slant, fields[XLFD_SLANT]);
	REPLACE_ELSE_BLANK(m_width, fields[XLFD_WIDTH]);
	REPLACE_ELSE_BLANK(m_adStyle, fields[XLFD_ADSTYLE]);
	m_pixelSize = 			(UT_uint32) atol(fields[XLFD_PIXELSIZE]);
	m_deciPointSize = 		(UT_uint32) atol(fields[XLFD_POINTSIZE]);
	m_resX = 				(UT_uint32) atol(fields[XLFD_RESOLUTIONX]);
	m_resY = 				(UT_uint32) atol(fields[XLFD_RESOLUTIONY]);	
	REPLACE_ELSE_BLANK(m_spacing, fields[XLFD_SPACING]);
	m_averageWidth  = 		(UT_uint32) atol(fields[XLFD_AVERAGEWIDTH]);
	REPLACE_ELSE_BLANK(m_registry, fields[XLFD_REGISTRY]);
	REPLACE_ELSE_BLANK(m_encoding, fields[XLFD_ENCODING]);	

	g_strfreev(fields);
}

char * XAP_UnixFontXLFD::getXLFD(void)
{
	// could GString be Unicode in the future?  let's hope not
	GString * xlfd = g_string_new("-");	// start off with the first dash
	UT_ASSERT(xlfd);

	gchar numberBuffer[10];
	
	g_string_append(xlfd, m_foundry); g_string_append(xlfd, "-");
	g_string_append(xlfd, m_family); g_string_append(xlfd, "-");
	g_string_append(xlfd, m_weight); g_string_append(xlfd, "-");
	g_string_append(xlfd, m_slant); g_string_append(xlfd, "-");
	g_string_append(xlfd, m_width); g_string_append(xlfd, "-");
	g_string_append(xlfd, m_adStyle); g_string_append(xlfd, "-");

	g_snprintf(numberBuffer, 10, "%ld", m_pixelSize);
	g_string_append(xlfd, numberBuffer); g_string_append(xlfd, "-");

	g_snprintf(numberBuffer, 10, "%ld", m_deciPointSize);
	g_string_append(xlfd, numberBuffer); g_string_append(xlfd, "-");

	g_snprintf(numberBuffer, 10, "%ld", m_resX);
	g_string_append(xlfd, numberBuffer); g_string_append(xlfd, "-");

	g_snprintf(numberBuffer, 10, "%ld", m_resY);
	g_string_append(xlfd, numberBuffer); g_string_append(xlfd, "-");
					
	g_string_append(xlfd, m_spacing); g_string_append(xlfd, "-");

	g_snprintf(numberBuffer, 10, "%ld", m_averageWidth);
	g_string_append(xlfd, numberBuffer); g_string_append(xlfd, "-");
					
	g_string_append(xlfd, m_registry); g_string_append(xlfd, "-");
	g_string_append(xlfd, m_encoding); // no trailing -

	char * result;
	UT_cloneString(result, xlfd->str);

	// wipe up the old GString
	g_string_free(xlfd, 1);
	
	return result;
}

/********************************************************************/

void XAP_UnixFontXLFD::_blankMembers(void)
{
	// set all the pointers to NULL
	m_foundry = NULL;
	m_family = NULL;
	m_weight = NULL;
	m_slant = NULL;
	m_width = NULL;
	m_adStyle = NULL;
	m_spacing = NULL;
	m_registry = NULL;
	m_encoding = NULL;
}

void XAP_UnixFontXLFD::_wipeMembers(void)
{
	FREEP(m_foundry);
	FREEP(m_family);
	FREEP(m_weight);
	FREEP(m_slant);
	FREEP(m_width);
	FREEP(m_adStyle);
	m_pixelSize = 0;
	m_deciPointSize = 0;
	m_resX = 0;
	m_resY = 0;
	FREEP(m_spacing);
	m_averageWidth = 0;
	FREEP(m_registry);
	FREEP(m_encoding);
}
