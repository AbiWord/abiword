/* AbiSource Program Utilities
 * Copyright (C) 2004 Hubert Figuiere
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

#ifndef _UT_PROP_VECTOR_H_
#define _UT_PROP_VECTOR_H_
#include "ut_vector.h"


class ABI_EXPORT UT_PropVector
	: public UT_GenericVector<gchar*>
{
public:
	UT_PropVector()
		: UT_GenericVector<gchar*>() {};
	void addOrReplaceProp(const gchar * pszProp, const gchar * pszVal);
	void getProp(const gchar * pszProp, const gchar * &pszVal) const;
	void removeProp(const gchar * pszProp);
};

#endif

