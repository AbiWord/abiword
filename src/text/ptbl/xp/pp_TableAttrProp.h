
#ifndef PP_TABLEATTRPROP_H
#define PP_TABLEATTRPROP_H

#include "ut_types.h"
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

	UT_Bool					createAP(void);
	UT_Bool					createAP(const XML_Char ** attributes,
									 const XML_Char ** properties,
									 UT_uint32 * pIndex);
	UT_Bool					createAP(UT_Vector * pVector,
									 UT_uint32 * pIndex);
	
	const PP_AttrProp *		getAP(UT_uint32 index) const;
	
protected:
	UT_Vector				m_vecTable;
};

#endif /* PP_TABLEATTRPROP_H */
