
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
	
	pf_Frag *				getFirst(void) const;

	void					dump(FILE * fp) const;
	
protected:
	pf_Frag *				m_pFirst;
	pf_Frag *				m_pLast;
};

#endif /* PF_FRAGMENTS_H */
