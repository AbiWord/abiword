
#ifndef PF_FRAG_STRUX_COLUMNSET_H
#define PF_FRAG_STRUX_COLUMNSET_H

#include "ut_types.h"
#include "pf_Frag.h"
#include "pf_Frag_Strux.h"
class PC_ColumnSet;
class pt_PieceTable;

// pf_Frag_Strux_ColumnSet represents structure information for a 
// ColumnSet.  This is part of the column information for a section;
// that is, the number of columns on a page and their shapes.

class pf_Frag_Strux_ColumnSet : public pf_Frag_Strux
{
public:
	pf_Frag_Strux_ColumnSet(pt_PieceTable * pPT,
							UT_uint32 vsIndex,
							pt_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux_ColumnSet();

	virtual UT_Bool			createSpecialChangeRecord(PX_ChangeRecord ** ppcr) const;
	
	virtual void			dump(FILE * fp) const;

protected:
	PC_ColumnSet *			m_pColumnSet;
};

#endif /* PF_FRAG_STRUX_COLUMNSET_H */
