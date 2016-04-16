/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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


#ifndef PP_TABLEATTRPROP_H
#define PP_TABLEATTRPROP_H

#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "pp_AttrProp.h"


// pp_TableAttrProp implements an unbounded table of PP_AttrProp
// objects.  Each PP_AttrProp represents the complete Attribute/
// Property state of one or more pieces (subsequences) of the
// document.

class ABI_EXPORT pp_TableAttrProp
{
public:
	pp_TableAttrProp();
	~pp_TableAttrProp();

	bool					addAP(PP_AttrProp * pAP,
								  UT_sint32 * pSubscript);
	bool					createAP(UT_sint32 * pSubscript);

	bool					createAP(const PP_PropertyVector & attributes,
									 const PP_PropertyVector & properties,
									 UT_sint32 * pSubscript);

	bool					createAP(const PP_PropertyVector & pVector,
									 UT_sint32 * pSubscript);

	bool					findMatch(const PP_AttrProp * pMatch,
									  UT_sint32 * pSubscript) const;

	const PP_AttrProp *		getAP(UT_sint32 subscript) const;

protected:
	UT_GenericVector<PP_AttrProp *> m_vecTable;
	UT_GenericVector<PP_AttrProp *>	m_vecTableSorted;
};

#endif /* PP_TABLEATTRPROP_H */
