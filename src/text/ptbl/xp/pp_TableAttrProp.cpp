
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_vector.h"
#include "pp_AttrProp.h"
#include "pp_TableAttrProp.h"

pp_TableAttrProp::pp_TableAttrProp()
{
}

pp_TableAttrProp::~pp_TableAttrProp()
{
	UT_VECTOR_PURGEALL(PP_AttrProp, m_vecTable);
}

UT_Bool pp_TableAttrProp::createAP(UT_uint32 * pSubscript)
{
	PP_AttrProp * pNew = new PP_AttrProp();
	if (!pNew)
		return UT_FALSE;
	if (m_vecTable.addItem(pNew,pSubscript) != 0)
	{
		delete pNew;
		return UT_FALSE;
	}

	return UT_TRUE;
}

UT_Bool pp_TableAttrProp::createAP(const XML_Char ** attributes,
								   const XML_Char ** properties,
								   UT_uint32 * pSubscript)
{
	UT_uint32 subscript;
	if (!createAP(&subscript))
		return UT_FALSE;

	PP_AttrProp * pAP = (PP_AttrProp *)m_vecTable.getNthItem(subscript);
	UT_ASSERT(pAP);
	if (!pAP->setAttributes(attributes) || !pAP->setProperties(properties))
		return UT_FALSE;

	*pSubscript = subscript;
	return UT_TRUE;
}

UT_Bool pp_TableAttrProp::createAP(const UT_Vector * pVector,
								   UT_uint32 * pSubscript)
{
	UT_uint32 subscript;
	if (!createAP(&subscript))
		return UT_FALSE;

	PP_AttrProp * pAP = (PP_AttrProp *)m_vecTable.getNthItem(subscript);
	UT_ASSERT(pAP);
	if (!pAP->setAttributes(pVector))
		return UT_FALSE;
	
	*pSubscript = subscript;
	return UT_TRUE;
}

UT_Bool pp_TableAttrProp::findMatch(const XML_Char ** attributes,
									UT_uint32 * pSubscript) const
{
	// return true if we find an AP in our table which is
	// an exact match for the attributes.
	// set *pSubscript to the subscript of the matching item.
	
	if (!attributes || !*attributes)
	{
		// we preload the zeroth cell with empty attributes.
		*pSubscript = 0;
		return UT_TRUE;
	}

	// TODO compute a hash on the contents of the attributes and
	// TODO look it up in the table, etc., etc.
	// TODO for now just fail to find a match.

	return UT_FALSE;
}

UT_Bool pp_TableAttrProp::findMatch(const UT_Vector * pVector,
									UT_uint32 * pSubscript) const
{
	// return true if we find an AP in our table which is
	// an exact match for the attributes.
	// set *pSubscript to the subscript of the matching item.
	
	if (!pVector || pVector->getItemCount()==0)
	{
		// we preload the zeroth cell with empty attributes.
		*pSubscript = 0;
		return UT_TRUE;
	}

	// TODO compute a hash on the contents of the attributes and
	// TODO look it up in the table, etc., etc.
	// TODO for now just fail to find a match.

	return UT_FALSE;
}

	
const PP_AttrProp * pp_TableAttrProp::getAP(UT_uint32 subscript) const
{
	UT_uint32 count = m_vecTable.getItemCount();
	if (subscript < count)
		return (const PP_AttrProp *)m_vecTable.getNthItem(subscript);
	else
		return NULL;
}

