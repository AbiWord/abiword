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

#ifndef XAP_RESOURCE_MANAGER_H
#define XAP_RESOURCE_MANAGER_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

#include "xap_Resource.h"

class UT_UTF8String;

class ABI_EXPORT XAP_ResourceManager
{
public:
	XAP_ResourceManager ();

	~XAP_ResourceManager ();

	const UT_UTF8String new_id (bool bInternal = true);

	/* returns resource corresponding to href; sets m_current to the result
	 * returns 0 if none is found
	 */
	XAP_Resource * resource (const char * href, bool bInternal, UT_uint32 * index = 0);
	XAP_Resource * current () const
	{
		return m_current;
	}
	void clear_current ()
	{
		m_current = 0;
	}

	/* resource objects are created/destroyed via ref/unref
	 * ref() returns false if href is not, and cannot be, created
	 */
	bool ref (const char * href);
	void unref (const char * href);

	/* number of internal resources
	 */
	UT_uint32 count () const { return m_resource_count; }

	XAP_Resource * operator[] (UT_uint32 i) const
	{
		return (i < m_resource_count) ? m_resource[i] : 0;
	}

	class ABI_EXPORT Writer : public XAP_InternalResource::Writer
	{
	public:
		virtual ~Writer () { }

		/* start element
		 */
		virtual UT_Error write_xml (void * context, const char * name, const char * const * atts) = 0;

		/* end element
		 */
		virtual UT_Error write_xml (void * context, const char * name) = 0;
	};

	UT_Error write_xml (void * context, Writer & writer); // call's writer's write_xml() & write_base64() callbacks

private:
	XAP_Resource * m_current; // current resource

	XAP_Resource ** m_resource;

	UT_uint32 m_resource_count;
	UT_uint32 m_resource_max;

	UT_uint32 m_id_number;

	bool grow ();
};

#endif /* ! XAP_RESOURCE_MANAGER_H */
