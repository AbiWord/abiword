
#ifndef PF_FRAG_OBJECT_H
#define PF_FRAG_OBJECT_H

#include "ut_types.h"
#include "pf_Frag.h"

// pf_Frag_Object represents an in-line object (such as
// an image) in the document.

class pf_Frag_Object : public pf_Frag
{
public:
	pf_Frag_Object();
	virtual ~pf_Frag_Object();
	
	virtual UT_Bool			createSpecialChangeRecord(PX_ChangeRecord ** ppcr) const;

	virtual void			dump(FILE * fp) const;

protected:
	
};

#endif /* PF_FRAG_OBJECT_H */
