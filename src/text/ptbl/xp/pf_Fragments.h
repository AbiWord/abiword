 
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

#ifndef PF_FRAGMENTS_H
#define PF_FRAGMENTS_H

// pf_Fragments is a container for all of the pf_Frag
// derrived objects.  pf_Fragments provides a searchable,
// efficient ordering on the document fragments.
//
// Currently this consists of a simple doubly-linked list.
// We may need to add a tree structure on top of it, if we
// need to do various types of searches.

#include <stdio.h>
#include "pf_Frag.h"

class pf_Fragments
{
public:
	pf_Fragments();
	~pf_Fragments();

	void					appendFrag(pf_Frag * pf);
	void					insertFrag(pf_Frag * pfPlace, pf_Frag * pfNew);
	void					unlinkFrag(pf_Frag * pf);
	
	pf_Frag *				getFirst(void) const;

#ifdef PT_TEST
	void					__dump(FILE * fp) const;
#endif
	
protected:
	pf_Frag *				m_pFirst;
	pf_Frag *				m_pLast;
};

#endif /* PF_FRAGMENTS_H */
