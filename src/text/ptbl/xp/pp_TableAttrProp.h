
#ifndef PP_TABLEATTRPROP_H
#define PP_TABLEATTRPROP_H

#include "ut_type.h"
#include "ut_vector.h"
#include "pp_AttrProp.h"

// pp_TableAttrProp implements an unbounded table of PP_AttrProp
// objects.  Each PP_AttrProp represents the complete Attribute/
// Property state of one or more pieces (subsequences) of the
// document.

class pp_TableAttrProp
{
public:
	pp_TableAttrProp();
	~pp_TableAttrProp();
	
protected:
	UT_vector				m_vecTable;
};

#endif /* PP_TABLEATTRPROP_H */
