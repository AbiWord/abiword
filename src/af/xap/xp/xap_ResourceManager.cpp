/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * 
 * Copyright (C) 2002 Francis James Franklin <fjf@alinameridon.com>
 * Copyright (C) 2002 AbiSource, Inc.
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
#include <string.h>

#include "ut_exception.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_Resource.h"
#include "xap_ResourceManager.h"

XAP_ResourceManager::XAP_ResourceManager () :
	m_resource(0),
	m_resource_count(0),
	m_resource_max(0)
{
	// 
}

XAP_ResourceManager::~XAP_ResourceManager ()
{
	for (UT_uint32 i = 0; i < m_resource_count; i++) delete m_resource[i];
	if (m_resource) free (m_resource);
}

/* returns resource corresponding to href
 * returns 0 if none is found
 * 
 * (assume_number_sign => prefix href with '#')
 */
XAP_Resource * XAP_ResourceManager::resource (const char * href, bool assume_number_sign, UT_uint32 * index)
{
	XAP_Resource * match = 0;

	if ( href == 0) return match;
	if (*href == 0) return match;

	bool bInternal;
	if (assume_number_sign)
		{
			if (*href == '#') return match; // huh?
			bInternal = true;
		}
	else
		{
			if (*href == '#')
				{
					href++;
					bInternal = true;
				}
			else bInternal = false;
		}

	for (UT_uint32 i = 0; i < m_resource_count; i++)
		if (m_resource[i]->bInternal == bInternal)
			if (strcmp (href, m_resource[i]->name().utf8_str()) == 0)
				{
					match = m_resource[i];
					if (index) *index = i;
					break;
				}

	return match;
}

/* resource objects are created/destroyed via ref/unref
 * ref() returns false if href is not, and cannot be, created
 */
bool XAP_ResourceManager::ref (const char * href)
{
	if (href == 0) return false;

	bool bInternal = (*href == '#');
	if (bInternal) href++;

	if (*href == 0) return false;

	XAP_Resource * match = resource (href, true);
	if (match)
		{
			match->ref ();
			return true;
		}
	if (!grow ()) return false;

	XAP_Resource * r = 0;
	UT_TRY
		{
			if (bInternal)
				r = new XAP_InternalResource (href);
			else
				r = new XAP_ExternalResource (href);
		}
	UT_CATCH (...)
		{
			r = 0;
		}
	if (r == 0) return false;

	m_resource[m_resource_count++] = r;
	return true;
}

/* resource objects are created/destroyed via ref/unref
 * ref() returns false if href is not and cannot be created
 */
void XAP_ResourceManager::unref (const char * href)
{
	if (href == 0) return;

	bool bInternal = (*href == '#');
	if (bInternal) href++;

	if (*href == 0) return;

	UT_uint32 index;
	XAP_Resource * match = resource (href, true, &index);
	if (match == 0) return;

	if (match->unref () > 0) return;

	delete m_resource[index];
	m_resource_count--;
	if (index < m_resource_count) m_resource[index] = m_resource[m_resource_count];
}

bool XAP_ResourceManager::grow ()
{
	if (m_resource_count < m_resource_max) return true;

	if (m_resource == 0)
		{
			m_resource = (XAP_Resource **) malloc (8 * sizeof (XAP_Resource *));
			if (m_resource == 0) return false;
			m_resource_max = 8;
			return true;
		}
	XAP_Resource ** more = (XAP_Resource **) realloc (m_resource, (m_resource_max + 8) * sizeof (XAP_Resource *));
	if (more == 0) return false;
	m_resource = more;
	m_resource_max += 8;
	return true;
}
