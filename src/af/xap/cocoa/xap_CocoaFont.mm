/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2002 Hubert Figuiere
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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#import <Cocoa/Cocoa.h>

#include "ut_string.h"
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_CocoaFont.h"
#include "xap_EncodingManager.h"
#include "xap_App.h"
//#include "ut_AdobeEncoding.h"

#if 0
//this one is for use with qsort
static int s_compareUniWidths(const void * w1, const void * w2)
{
	const uniWidth   * W1 = (const uniWidth   * ) w1;
	const uniWidth   * W2 = (const uniWidth   * ) w2;

	if(W1->ucs < W2->ucs)
		return -1;
	if(W1->ucs > W2->ucs)
		return 1;
	return 0;
}

//this one is for use with bsearch
static int s_compareUniWidthsChar(const void * c, const void * w)
{
	const UT_UCSChar * C = (const UT_UCSChar * ) c;
	const uniWidth   * W = (const uniWidth   * ) w;
	if(*C < W->ucs)
		return -1;
	if(*C > W->ucs)
		return 1;
	return 0;
}
#endif

#ifdef DEBUG
#define ASSERT_MEMBERS	do { UT_ASSERT(m_name); UT_ASSERT(m_nsName); } while (0)
#else
#define ASSERT_MEMBERS
#endif

/*******************************************************************/

XAP_CocoaFont::XAP_CocoaFont()
  : m_size(0), m_font(NULL)
{
}

XAP_CocoaFont::XAP_CocoaFont(NSFont* font)
{
	m_font = font;
	[m_font retain];
}

XAP_CocoaFont::XAP_CocoaFont(const XAP_CocoaFont & copy)
  : GR_Font(copy)
{
	m_font = [copy.getNSFont() copy];
}

XAP_CocoaFont::~XAP_CocoaFont()
{
	[m_font release];
}


UT_uint32 XAP_CocoaFont::getSize(void)
{
	if (m_size == 0) {
		m_size = (UT_uint32)[m_font pointSize];
	}
	return m_size;
}

const char * XAP_CocoaFont::getName(void)
{
	return [[m_font fontName] cString];
}
