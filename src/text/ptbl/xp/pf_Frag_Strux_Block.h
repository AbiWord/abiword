 
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

#ifndef PF_FRAG_STRUX_BLOCK_H
#define PF_FRAG_STRUX_BLOCK_H

#include "ut_types.h"
#include "pf_Frag.h"
#include "pf_Frag_Strux.h"

// pf_Frag_Strux_Block represents structure information for a 
// block (paragraph) in the document.

class pf_Frag_Strux_Block : public pf_Frag_Strux
{
public:
	pf_Frag_Strux_Block(pt_PieceTable * pPT,
						PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux_Block();

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif
};

#endif /* PF_FRAG_STRUX_BLOCK_H */
