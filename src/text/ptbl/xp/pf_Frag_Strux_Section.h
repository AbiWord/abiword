
#ifndef PF_FRAG_STRUX_SECTION_H
#define PF_FRAG_STRUX_SECTION_H

#include "ut_types.h"
#include "pf_Frag.h"
#include "pf_Frag_Strux.h"

// pf_Frag_Strux_Section represents structure information for
// a section in the document.

class pf_Frag_Strux_Section : public pf_Frag_Strux
{
public:
	pf_Frag_Strux_Section();
	virtual ~pf_Frag_Strux_Section();
	
protected:
	pb_CallbackList			m_cbList;	/* TODO */
};

#endif /* PF_FRAG_STRUX_SECTION_H */
