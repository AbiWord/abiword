
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
	pf_Frag();
	virtual ~pf_Frag();

	typedef enum _PFType { PFT_Text, PFT_Object, PFT_Strux } PFType;
	
protected:
	PFType					m_type;
	
};

#endif /* PF_FRAG_H */
