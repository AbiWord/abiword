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

#ifndef XAP_RESOURCE_H
#define XAP_RESOURCE_H

#include "ut_string_class.h"

class ABI_EXPORT XAP_Resource
{
protected:
	XAP_Resource (const char * resource_name, bool resource_internal);

public:
	virtual ~XAP_Resource () { }

	const UT_UTF8String & name () const { return m_resource_name; }

	UT_uint32 ref ()   { return ++m_ref_count; }
	UT_uint32 unref () { return m_ref_count ? --m_ref_count : 0; }

	UT_uint32 count () const { return m_ref_count; }

	const bool bInternal;

	UT_UTF8String Description; // Doesn't seem to be any point in hiding this...
private:
	UT_UTF8String m_resource_name;

	UT_uint32 m_ref_count;
};

class ABI_EXPORT XAP_InternalResource : public XAP_Resource
{
public:
	class ABI_EXPORT Writer
	{
	public:
		virtual ~Writer () { }

		virtual UT_Error write_base64 (void * context, const char * base64, UT_uint32 length, bool final) = 0;
	};

	XAP_InternalResource (const char * resource_id);

	~XAP_InternalResource ();

	/* buffer containing binary representation
	 *
	 * returns 0 on mem-err or illegal base64 chars encountered; if base64_encoded then length will change
	 */
	const char * buffer (const char * new_buffer, UT_uint32 new_buffer_length, bool base64_encoded = false);

	const char * buffer () const { return m_buffer; }
	UT_uint32    length () const { return m_buffer_length; }

	UT_Error write_base64 (void * context, Writer & writer); // call's writer's write_base64() callback

	const UT_UTF8String & type (const UT_UTF8String & new_content_type);
	const UT_UTF8String & type (const char * new_content_type);

	const UT_UTF8String & type () const { return m_content_type; }

private:
	void clear ();

	char *    m_buffer;
	UT_uint32 m_buffer_length;

	UT_UTF8String m_content_type;
};

class ABI_EXPORT XAP_ExternalResource : public XAP_Resource
{
public:
	XAP_ExternalResource (const char * resource_id);

	~XAP_ExternalResource ();

	const UT_UTF8String & URL () const
	{
		return m_url;
	}
	const UT_UTF8String & URL (const UT_UTF8String & url);

private:
	UT_UTF8String m_url;
};

#endif /* ! XAP_RESOURCE_H */
