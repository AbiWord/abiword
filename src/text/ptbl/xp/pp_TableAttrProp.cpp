
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

UT_Bool pp_TableAttrProp::createAP(pt_AttrPropIndex * pIndex)
{
	PP_AttrProp * pNew = new PP_AttrProp();
	if (!pNew)
		return UT_FALSE;
	if (m_vecTable.addItem(pNew) != 0)
	{
		delete pNew;
		return UT_FALSE;
	}

	if (pIndex)
		*pIndex = m_vecTable.getItemCount()-1;
	return UT_TRUE;
}

UT_Bool pp_TableAttrProp::createAP(const XML_Char ** attributes,
								   const XML_Char ** properties,
								   pt_AttrPropIndex * pIndex)
{
	pt_AttrPropIndex index;
	if (!createAP(&index))
		return UT_FALSE;

	PP_AttrProp * pAP = (PP_AttrProp *)m_vecTable.getNthItem(index);
	UT_ASSERT(pAP);
	if (!pAP->setAttributes(attributes) || !pAP->setProperties(properties))
		return UT_FALSE;

	*pIndex = index;
	return UT_TRUE;
}

UT_Bool pp_TableAttrProp::createAP(const UT_Vector * pVector,
								   pt_AttrPropIndex * pIndex)
{
	pt_AttrPropIndex index;
	if (!createAP(&index))
		return UT_FALSE;

	PP_AttrProp * pAP = (PP_AttrProp *)m_vecTable.getNthItem(index);
	UT_ASSERT(pAP);

	pt_AttrPropIndex kLimit = pVector->getItemCount();
	for (pt_AttrPropIndex k=0; k+1<kLimit; k+=2)
	{
		const XML_Char * pName = (const XML_Char *)pVector->getNthItem(k);
		const XML_Char * pValue = (const XML_Char *)pVector->getNthItem(k+1);
		if (!pAP->setAttribute(pName,pValue))
			return UT_FALSE;
	}
	
	*pIndex = index;
	return UT_TRUE;
}
	
const PP_AttrProp * pp_TableAttrProp::getAP(pt_AttrPropIndex index) const
{
	pt_AttrPropIndex count = m_vecTable.getItemCount();
	if (index < count)
		return (const PP_AttrProp *)m_vecTable.getNthItem(index);
	else
		return NULL;
}

