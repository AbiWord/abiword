/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
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

class pp_TableAttrProp
{
public:
	pp_TableAttrProp();
	~pp_TableAttrProp();

	UT_Bool					createAP(UT_uint32 * pSubscript);

	UT_Bool					createAP(const XML_Char ** attributes,
									 const XML_Char ** properties,
									 UT_uint32 * pSubscript);

	UT_Bool					createAP(const UT_Vector * pVector,
									 UT_uint32 * pSubscript);

	UT_Bool					findMatch(const XML_Char ** attributes,
									  UT_uint32 * pSubscript) const;

	UT_Bool					findMatch(const UT_Vector * pVector,
									  UT_uint32 * pSubscript) const;
	
	const PP_AttrProp *		getAP(UT_uint32 subscript) const;

	UT_Bool					cloneWithReplacements(const PP_AttrProp * papOld,
												  const XML_Char ** attributes,
												  const XML_Char ** properties,
												  UT_uint32 * pSubscript);
	
protected:
	UT_Vector				m_vecTable;
};

#endif /* PP_TABLEATTRPROP_H */
