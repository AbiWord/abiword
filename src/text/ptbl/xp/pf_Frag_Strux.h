
#ifndef PF_FRAG_STRUX_H
#define PF_FRAG_STRUX_H

#include "ut_types.h"
#include "pf_Frag.h"
#include "pt_Types.h"
#include "pd_Document.h"

// pf_Frag_Strux represents structure information (such as a
// paragraph or section) in the document.
//
// pf_Frag_Strux is descended from pf_Frag, but is a base
// class for _Section, etc.
// We use an enum to remember type, rather than use any of the
// run-time stuff.

class pf_Frag_Strux : public pf_Frag
{
public:
	pf_Frag_Strux(pt_PieceTable * pPT,
				  PTStruxType struxType,
				  UT_uint32 vsIndex,
				  pt_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux();

	PTStruxType				getStruxType(void) const;
	PL_StruxFmtHandle		getFmtHandle(PL_ListenerId lid) const;
	UT_Bool					setFmtHandle(PL_ListenerId lid, PL_StruxFmtHandle sfh);
	
	virtual UT_Bool			createSpecialChangeRecord(PX_ChangeRecord ** ppcr) const = 0;

	virtual void			dump(FILE * fp) const = 0;
	
protected:
	PTStruxType				m_struxType;
	UT_uint32				m_vsIndex;	/* which VS[] we are in */
	pt_AttrPropIndex		m_indexAP;	/* index in VS[].m_tableAttrProp to our A/P */
	UT_Vector				m_vecFmtHandle;
};

#endif /* PF_FRAG_STRUX_H */
