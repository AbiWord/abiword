 
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


#ifndef FL_LAYOUT_H
#define FL_LAYOUT_H

#include "ut_types.h"
#include "pt_Types.h"

class PP_AttrProp;
class PD_Document;

/*
	fl_Layout is the base class for all layout objects which correspond to 
	logical elements of the PD_Document.  

	We use an enum to remember type, rather than use any of the
	run-time stuff.
*/

class fl_Layout
{
public:
	fl_Layout(PTStruxType type, PL_StruxDocHandle sdh);
	~fl_Layout();

	PTStruxType			getType(void) const;
	void				setPTvars(PT_VarSetIndex vsIndex, PT_AttrPropIndex apIndex);
	UT_Bool				getAttrProp(const PP_AttrProp ** ppAP) const;
	UT_Bool				getSpanAttrProp(UT_uint32 offset, const PP_AttrProp ** ppAP) const;
	
protected:
	PTStruxType				m_type;
	PL_StruxDocHandle		m_sdh;
	PT_VarSetIndex			m_vsIndex;
	PT_AttrPropIndex		m_apIndex;

	PD_Document *			m_pDoc;		// set by child
};

#endif /* FL_LAYOUT_H */
