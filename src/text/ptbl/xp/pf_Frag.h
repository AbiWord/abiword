
#ifndef PF_FRAG_H
#define PF_FRAG_H

#include "ut_types.h"

// pf_Frag represents a fragment of the document.  This may
// be text, an inline object (such as an image), or structure
// information (such as a paragraph or section).
//
// pf_Frag is an abstract base class.
// We use an enum to remember type, rather than use any of the
// run-time stuff.

class pf_Frag
{
public:
	typedef enum _PFType { PFT_Text, PFT_Object, PFT_Strux } PFType;

	pf_Frag(PFType type);
	virtual ~pf_Frag();

	PFType					getType(void) const;
	pf_Frag *				getNext(void) const;
	pf_Frag *				getPrev(void) const;
	pf_Frag *				setNext(pf_Frag * pNext);
	pf_Frag *				setPrev(pf_Frag * pPrev);
	
protected:
	PFType					m_type;

	pf_Frag *				m_next;
	pf_Frag *				m_prev;
};

#endif /* PF_FRAG_H */
