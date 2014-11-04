/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiSource Program Utilities
 * Copyright (C) 2011 Hubert Figuiere
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



#ifndef __UT_STD_VECTOR_H__
#define __UT_STD_VECTOR_H__

#include "ut_assert.h"

// TODO I'm sure we can template the sparse out of it.
template <class V>
void UT_std_vector_sparsepurgeall(V & v)
{
	for(typename V::iterator iter = v.begin();iter != v.end(); ++iter) {
		if(*iter) {
			delete *iter;
        }
	}
}

template <class V>
void UT_std_vector_purgeall(V & v)
{
	for(typename V::iterator iter = v.begin();iter != v.end(); ++iter) {
		UT_ASSERT_HARMLESS(*iter);
		if(*iter) {
			delete *iter;
        }
	}
}

template <class V, typename F>
void UT_std_vector_freeall(V & v, F free_func = g_free)
{
	for(typename V::iterator iter = v.begin();iter != v.end(); ++iter) {
		if(*iter) {
			free_func(*iter);
        }
	}
}

#endif
