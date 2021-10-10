/* AbiSource Application Framework
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "xap_FontPreview.h"

// standard font preview string
#define PREVIEW_ENTRY_DEFAULT_STRING	"AaBbYyZz"

XAP_FontPreview::XAP_FontPreview()
	: m_width(PREVIEW_WIDTH)
	, m_height(PREVIEW_HEIGHT)
	, m_pFontPreview(NULL)
{
	UT_UCS4_cloneString_char (&m_drawString, PREVIEW_ENTRY_DEFAULT_STRING);
}

XAP_FontPreview::~XAP_FontPreview(void)
{
	FREEP(m_drawString);
	DELETEP(m_pFontPreview);
}

void XAP_FontPreview::_createFontPreviewFromGC(GR_Graphics * gc,
											   UT_uint32 width,
											   UT_uint32 height)
{
	UT_ASSERT(gc);
	UT_DEBUGMSG(("SEVIOR!!!!!!!!!!! font priview created!!!!!\n"));
	m_pFontPreview = new XAP_Preview_FontPreview(gc,NULL);
	UT_return_if_fail(m_pFontPreview);
	
	m_pFontPreview->setDrawString(m_drawString);
	m_pFontPreview->setVecProperties(&m_mapProps);
	m_pFontPreview->setWindowSize(width, height);
	m_width = gc->tlu(width);
	m_height = gc->tlu(height);
	addOrReplaceVecProp("font-size","36pt");
}

void XAP_FontPreview::addOrReplaceVecProp(const std::string & pszProp,
										  const std::string & pszVal)
{
	m_mapProps[pszProp] = pszVal;
}

void XAP_FontPreview::setFontFamily(const gchar * pFontFamily)
{
	addOrReplaceVecProp("font-family",pFontFamily);
}

void XAP_FontPreview::setText(const gchar * pFontFamily)
{
	UT_return_if_fail(pFontFamily);
	FREEP(m_drawString);
	UT_UCS4_cloneString_char (&m_drawString, pFontFamily);
	m_pFontPreview->setDrawString(m_drawString);
}

void XAP_FontPreview::draw()
{
  if (m_pFontPreview)
    m_pFontPreview->queueDraw();
}

