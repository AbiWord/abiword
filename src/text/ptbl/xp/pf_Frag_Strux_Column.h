
#ifndef PF_FRAG_STRUX_COLUMN_H
#define PF_FRAG_STRUX_COLUMN_H

#include "ut_types.h"
#include "pf_Frag.h"
#include "pf_Frag_Strux.h"

// pf_Frag_Strux_Column represents structure information for a 
// Column.  This is part of the column information for a section;
// that is, the geometry and type or model for a column on a page.
// It is not an instantiated column with content.

class pf_Frag_Strux_Column : public pf_Frag_Strux
{
public:
	pf_Frag_Strux_Column();
	virtual ~pf_Frag_Strux_Column();
	
protected:
	// m_pColumn contains an internalized representation of the
	// column (attributes/properties have been parsed and stored
	// for easy/quick access).  we defer this internalization
	// until the first time the column is access (asked to create
	// an actual column for the layout).
	
	PC_Column *			m_pColumn;
};

#endif /* PF_FRAG_STRUX_COLUMN_H */
