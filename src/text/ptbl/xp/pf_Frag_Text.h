 
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

#ifndef PF_FRAG_TEXT_H
#define PF_FRAG_TEXT_H

#include "ut_types.h"
#include "pt_Types.h"
#include "pf_Frag.h"

// pf_Frag_Text represents a fragment of text in the document.
// note that it does not contain a PT_DocPosition -- the fragment
// does not know where it is in the document; it only knows its
// buffer position.

class pf_Frag_Text : public pf_Frag
{
public:
	pf_Frag_Text(pt_PieceTable * pPT,
				 PT_BufIndex bufIndex,
				 UT_uint32 length,
				 PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Text();
	
	virtual UT_Bool			createSpecialChangeRecord(PX_ChangeRecord ** ppcr) const;
	PT_BufIndex				getBufIndex(void) const;
	UT_uint32				getLength(void) const;
	PT_AttrPropIndex		getIndexAP(void) const;
	void					setIndexAP(PT_AttrPropIndex indexNewAP);
	void					changeLength(UT_uint32 newLength);
	void					adjustOffsetLength(PT_BufIndex bi, UT_uint32 newLength);
	
	virtual void			dump(FILE * fp) const;

protected:
	PT_BufIndex				m_bufIndex;
	UT_uint32				m_length;
	PT_AttrPropIndex		m_indexAP;
};

#endif /* PF_FRAG_TEXT_H */
