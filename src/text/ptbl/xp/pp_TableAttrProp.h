 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
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
