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
#include <stdio.h>
#include <string.h>
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "xap_FontPreview.h"

// standard font preview string
#define PREVIEW_ENTRY_DEFAULT_STRING	"AaBbYyZz"

XAP_FontPreview::XAP_FontPreview()
{
	m_pColorBackground      = NULL;
	m_pFontPreview          = NULL;
	m_width					= PREVIEW_WIDTH;
	m_height				= PREVIEW_HEIGHT;
	
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

	m_pFontPreview = new XAP_Preview_FontPreview(gc,m_pColorBackground);
	UT_ASSERT(m_pFontPreview);
	
	m_pFontPreview->setDrawString(m_drawString);
	m_pFontPreview->setVecProperties(&m_vecProps);
	m_pFontPreview->setWindowSize(width, height);
	
	addOrReplaceVecProp("font-size","36pt");
}

void XAP_FontPreview::addOrReplaceVecProp(const XML_Char * pszProp,
										  const XML_Char * pszVal)
{
	UT_sint32 iCount = m_vecProps.getItemCount();
	const char * pszV = NULL;
	if(iCount <= 0)
	{
		m_vecProps.addItem((void *) pszProp);
		m_vecProps.addItem((void *) pszVal);
		return;
	}
	UT_sint32 i = 0;
	for(i=0; i < iCount ; i += 2)
	{
		pszV = (const XML_Char *) m_vecProps.getNthItem(i);
		if( (pszV != NULL) && (strcmp( pszV,pszProp) == 0))
			break;
	}
	if(i < iCount)
		m_vecProps.setNthItem(i+1, (void *) pszVal, NULL);
	else
	{
		m_vecProps.addItem((void *) pszProp);
		m_vecProps.addItem((void *) pszVal);
	}
	return;
}

void XAP_FontPreview::setFontFamily(const XML_Char * pFontFamily)
{
	addOrReplaceVecProp("font-family",pFontFamily);
}

void XAP_FontPreview::setText(const XML_Char * pFontFamily)
{
	UT_UCS4_cloneString_char (&m_drawString, pFontFamily);
	m_pFontPreview->setDrawString(m_drawString);
}

void XAP_FontPreview::draw()
{
  if (m_pFontPreview)
    m_pFontPreview->draw();
}

