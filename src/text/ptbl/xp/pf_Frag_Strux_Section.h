
#ifndef PF_FRAG_STRUX_SECTION_H
#define PF_FRAG_STRUX_SECTION_H

#include "ut_types.h"
#include "pf_Frag.h"
#include "pf_Frag_Strux.h"
class pt_PieceTable;

// pf_Frag_Strux_Section represents structure information for
// a section in the document.

class pf_Frag_Strux_Section : public pf_Frag_Strux
{
public:
	pf_Frag_Strux_Section(pt_PieceTable * pPT,
						  PT_VarSetIndex vsIndex,
						  PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux_Section();

	virtual void			dump(FILE * fp) const;
};

#endif /* PF_FRAG_STRUX_SECTION_H */
