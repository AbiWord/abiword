 
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


#ifndef PP_PROPERTY_H
#define PP_PROPERTY_h

#include "ut_types.h"
#include "xmlparse.h"
class PP_AttrProp;

// PP_Property captures knowledge of the various CSS properties,
// such as their initial/default values and whether they are
// inherited.

class PP_Property
{
public:
	XML_Char *	m_pszName;
	XML_Char *	m_pszInitial;
	UT_Bool		m_bInherit;

	const XML_Char *	getName() const;
	const XML_Char *	getInitial() const;
	UT_Bool				canInherit() const;
};

const PP_Property * PP_lookupProperty(const XML_Char * pszName);
const XML_Char * PP_evalProperty(const XML_Char * pszName,
								 const PP_AttrProp * pSpanAttrProp,
								 const PP_AttrProp * pBlockAttrProp,
								 const PP_AttrProp * pSectionAttrProp);

#endif /* PP_PROPERTY_H */
