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
#include "ut_base64.h"

#include "xap_Resource.h"

XAP_Resource::XAP_Resource (const char * resource_name, bool resource_internal) : 
	bInternal(resource_internal),
	Description(""),
	m_resource_name(resource_name),
	m_ref_count(1)
{
	// 
}

XAP_InternalResource::XAP_InternalResource (const char * resource_id) :
	XAP_Resource(resource_id,true),
	m_buffer(0),
	m_buffer_length(0),
	m_content_type("")
{
	// 
}

XAP_InternalResource::~XAP_InternalResource ()
{
	clear ();
}

void XAP_InternalResource::clear ()
{
	DELETEPV(m_buffer);
	m_buffer_length = 0;
}

const char * XAP_InternalResource::buffer (const char * new_buffer, UT_uint32 new_buffer_length, bool base64_encoded)
{
	clear ();

	if ((new_buffer == 0) || (new_buffer_length == 0)) return 0;

	UT_uint32 buffer_length = new_buffer_length;
	if (base64_encoded) buffer_length -= buffer_length >> 2;

	try
		{
			m_buffer = new char[buffer_length];
		}
	catch (...)
		{
			m_buffer = 0;
		}
	if (m_buffer == 0) return m_buffer;

	if (!base64_encoded)
		{
			memcpy (m_buffer, new_buffer, buffer_length);
			m_buffer_length = buffer_length;
			return m_buffer;
		}

	const char * b64ptr = new_buffer;
	char * binptr = m_buffer;

	size_t b64len = new_buffer_length;
	size_t binlen = buffer_length;

	if (UT_UTF8_Base64Decode (binptr, binlen, b64ptr, b64len))
		{
			m_buffer_length = buffer_length - binlen;
		}
	else clear ();

	return m_buffer;
}

UT_Error XAP_InternalResource::write_base64 (void * context, Writer & writer) // call's writer's write_base64() callback
{
	UT_Error err = UT_OK;

	char b64buf[73];

	const char * binptr = m_buffer;
	UT_uint32 buffer_length = m_buffer_length;

	while (buffer_length >= 54)
		{
			char * b64ptr = b64buf;

			size_t binlen = 54;
			size_t b64len = 72;

			if (!UT_UTF8_Base64Encode (b64ptr, b64len, binptr, binlen))
				{
					err = UT_ERROR;
					break;
				}
			buffer_length -= 54;

			b64buf[72] = 0;

			err = writer.write_base64 (context, b64buf, 72, (buffer_length == 0));
			if (err != UT_OK) break;
		}
	if (err != UT_OK) return err;

	if (buffer_length)
		{
			char * b64ptr = b64buf;

			size_t binlen = buffer_length;
			size_t b64len = 72;

			if (!UT_UTF8_Base64Encode (b64ptr, b64len, binptr, binlen))
				{
					err = UT_ERROR;
				}
			else
				{
					b64buf[72-b64len] = 0;
					err = writer.write_base64 (context, b64buf, 72-b64len, true);
				}
		}
	return err;
}

const UT_UTF8String & XAP_InternalResource::type (const UT_UTF8String & new_content_type)
{
	m_content_type = new_content_type;
	return m_content_type;
}

const UT_UTF8String & XAP_InternalResource::type (const char * new_content_type)
{
	if (new_content_type)
		m_content_type = new_content_type;
	else
		m_content_type = "";

	return m_content_type;
}

XAP_ExternalResource::XAP_ExternalResource (const char * resource_id) :
	XAP_Resource(resource_id,false),
	m_url("")
{
	// 
}

XAP_ExternalResource::~XAP_ExternalResource ()
{
	// 
}

const UT_UTF8String & XAP_ExternalResource::URL (const UT_UTF8String & url)
{
	m_url = url;
	return m_url;
}
