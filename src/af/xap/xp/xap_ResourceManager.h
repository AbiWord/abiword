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
 
#ifndef XAP_RESOURCE_MANAGER_H
#define XAP_RESOURCE_MANAGER_H

#include "ut_types.h"

class XAP_Resource;
class ABI_EXPORT XAP_ResourceManager
{
public:
	XAP_ResourceManager ();

	~XAP_ResourceManager ();

	/* returns resource corresponding to href
	 * returns 0 if none is found
	 * 
	 * (assume_number_sign => prefix href with '#')
	 */
	XAP_Resource * resource (const char * href, bool assume_number_sign = false, UT_uint32 * index = 0);

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

private:
	XAP_Resource ** m_resource;

	UT_uint32 m_resource_count;
	UT_uint32 m_resource_max;

	bool grow ();
};

#endif /* ! XAP_RESOURCE_MANAGER_H */
