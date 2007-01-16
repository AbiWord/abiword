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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef _UT_PROP_VECTOR_H_
#define _UT_PROP_VECTOR_H_
#include "ut_vector.h"


class UT_PropVector 
	: public UT_GenericVector<XML_Char*>
{
public:
	UT_PropVector()
		: UT_GenericVector<XML_Char*>() {};
	void addOrReplaceProp(const XML_Char * pszProp, const XML_Char * pszVal);
	void getProp(const XML_Char * pszProp, const XML_Char * &pszVal);
	void removeProp(const XML_Char * pszProp);
};

#endif

