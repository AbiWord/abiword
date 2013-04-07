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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */
 
#include <stdlib.h>
#include <string.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string_class.h"

#include "xap_Resource.h"
#include "xap_ResourceManager.h"

XAP_ResourceManager::XAP_ResourceManager () :
	m_current(0),
	m_resource(0),
	m_resource_count(0),
	m_resource_max(0),
	m_id_number(0)
{
	// 
}

XAP_ResourceManager::~XAP_ResourceManager ()
{
	for (UT_uint32 i = 0; i < m_resource_count; i++) delete m_resource[i];
	if (m_resource) g_free (m_resource);
}

const UT_UTF8String XAP_ResourceManager::new_id (bool bInternal)
{
	static const char utf8_hex[16] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };

	char buf[11];

	/* external references (hyperlinks) are remapped from, say, http://this.com/that
	 * to, say, /re_abc123
	 */
	if (!bInternal)
		{
			buf[0] = '/';
			buf[1] = 'r';
			buf[2] = 'e';
		}
	else
		{
			buf[0] = '#';
			buf[1] = 'r';
			buf[2] = 'i';
		}
	buf[3] = '_';

	if ((m_id_number & 0xffffff) != m_id_number)
		{
			UT_DEBUGMSG(("argh!! too many resources! who do you think I am?!\n"));
			UT_ASSERT((m_id_number & 0xffffff) == m_id_number);

			buf[4] = 0;
		}
	else
		{
			UT_uint32 number = m_id_number++;

			buf[9] = utf8_hex[number & 0x0f]; number >>= 4;
			buf[8] = utf8_hex[number & 0x0f]; number >>= 4;
			buf[7] = utf8_hex[number & 0x0f]; number >>= 4;
			buf[6] = utf8_hex[number & 0x0f]; number >>= 4;
			buf[5] = utf8_hex[number & 0x0f]; number >>= 4;
			buf[4] = utf8_hex[number & 0x0f];

			buf[10] = 0;
		}
	return UT_UTF8String(buf);
}

/* returns resource corresponding to href
 * returns 0 if none is found
 */
XAP_Resource * XAP_ResourceManager::resource (const char * href, bool bInternal, UT_uint32 * index)
{
	m_current = 0;

	if ( href == 0) return 0;
	if (*href == 0) return 0;

	if (bInternal)
		{
			if (*href == '/') return 0;
			if (*href == '#') href++;
		}
	else
		{
			if (*href == '#') return 0;
			if (*href == '/') href++;
		}
	if (*href != 'r') return 0;

	for (UT_uint32 i = 0; i < m_resource_count; i++)
		if (m_resource[i]->bInternal == bInternal)
			if (strcmp (href, m_resource[i]->name().utf8_str()) == 0)
				{
					m_current = m_resource[i];
					if (index) *index = i;
					break;
				}

	return m_current;
}

/* resource objects are created/destroyed via ref/unref
 * ref() returns false if href is not, and cannot be, created
 */
bool XAP_ResourceManager::ref (const char * href)
{
	if ( href == 0) return false;
	if (*href == 0) return false;

	bool bInternal = false;
	if (*href == '#') bInternal = true;
	else if (*href != '/') return false;

	XAP_Resource * match = resource (href, bInternal);
	if (match)
		{
			match->ref ();
			return true;
		}
	if (!grow ()) return false;

	XAP_Resource * r = 0;
	try
		{
			if (bInternal)
				r = new XAP_InternalResource (href);
			else
				r = new XAP_ExternalResource (href);
		}
	catch (...)
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
	if ( href == 0) return;
	if (*href == 0) return;

	bool bInternal = false;
	if (*href == '#') bInternal = true;
	else if (*href != '/') return;

	UT_uint32 index;
	XAP_Resource * match = resource (href, bInternal, &index);
	if (match == 0) return;

	if (match->unref () > 0) return;

	delete m_resource[index];
	m_resource_count--;
	if (index < m_resource_count) m_resource[index] = m_resource[m_resource_count];
}

/* call's writer's write_xml() & write_base64() callbacks
 */
UT_Error XAP_ResourceManager::write_xml (void * context, Writer & writer)
{
	UT_Error err = UT_OK;

	static const char * psz_id = "id";
	static const char * psz_content_type = "type";
	static const char * psz_description = "desc";

	const char * atts[8];

	atts[4] = NULL;
	atts[5] = NULL;

	for (UT_uint32 i = 0; i < m_resource_count; i++)
		if (m_resource[i]->bInternal)
			{
				XAP_InternalResource * ri = dynamic_cast<XAP_InternalResource *>(m_resource[i]);

				UT_uint32 n = 0;

				atts[n++] = psz_id;
				atts[n++] = ri->name().utf8_str ();

				if (!(ri->type().empty ()))
					{
						atts[n++] = psz_content_type;
						atts[n++] = ri->type().utf8_str ();
					}
				if (!(ri->Description.empty ()))
					{
						atts[n++] = psz_description;
						atts[n++] = ri->Description.utf8_str ();
					}
				atts[n++] = NULL;
				atts[n++] = NULL;

				err = writer.write_xml (context, "resource", atts);
				if (err != UT_OK) break;

				err = ri->write_base64 (context, writer);
				if (err != UT_OK) break;

				err = writer.write_xml (context, "resource");
				if (err != UT_OK) break;
			}
	return err;
}

bool XAP_ResourceManager::grow ()
{
	if (m_resource_count < m_resource_max) return true;

	if (m_resource == 0)
		{
			m_resource = static_cast<XAP_Resource **>(g_try_malloc (8 * sizeof (XAP_Resource *)));
			if (m_resource == 0) return false;
			m_resource_max = 8;
			return true;
		}
	XAP_Resource ** more = static_cast<XAP_Resource **>(g_try_realloc (m_resource, (m_resource_max + 8) * sizeof (XAP_Resource *)));
	if (more == 0) return false;
	m_resource = more;
	m_resource_max += 8;
	return true;
}
