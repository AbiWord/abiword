/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998,1999 AbiSource, Inc.
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

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_hash.h"
#include "ut_vector.h"

#include "xad_Document.h"

#ifdef ENABLE_RESOURCE_MANAGER
#include "xap_ResourceManager.h"
#endif

AD_Document::AD_Document() :
#ifdef ENABLE_RESOURCE_MANAGER
	m_pResourceManager(new XAP_ResourceManager),
#else
	m_pResourceManager(0),
#endif
	m_iRefCount(1),
	m_szFilename(NULL),
	m_szEncodingName(""), // Should this have a default? UTF-8, perhaps?
	m_lastSavedTime(time(NULL)),
	m_bPieceTableChanging(false)
{	// TODO: clear the ignore list
	

	// 
}

AD_Document::~AD_Document()
{
	UT_ASSERT(m_iRefCount == 0);

   	// NOTE: let subclass clean up m_szFilename, so it matches the alloc mechanism

	// & finally...
#ifdef ENABLE_RESOURCE_MANAGER
	DELETEP(m_pResourceManager);
#endif
}

void AD_Document::ref(void)
{
	UT_ASSERT(m_iRefCount > 0);

	m_iRefCount++;
}


void AD_Document::unref(void)
{
	UT_ASSERT(m_iRefCount > 0);

	if (--m_iRefCount == 0)
	{
		delete this;
	}
}


bool AD_Document::isPieceTableChanging(void)
{
        return m_bPieceTableChanging;
}

const char * AD_Document::getFilename(void) const
{
	return m_szFilename;
}

// Document-wide Encoding name used for some file formats (Text, RTF, HTML)

void AD_Document::setEncodingName(const char *szEncodingName)
{
	if (szEncodingName == NULL)
		szEncodingName = "";

	m_szEncodingName = szEncodingName;
}

const char * AD_Document::getEncodingName(void) const
{
	return m_szEncodingName.size() ? m_szEncodingName.c_str() : 0;
}
