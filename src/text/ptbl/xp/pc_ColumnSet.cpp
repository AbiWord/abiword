
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "pc_Column.h"
#include "pc_Column_Box.h"
#include "pc_Column_Circle.h"
#include "pc_ColumnSet.h"

PC_ColumnSet::PC_ColumnSet()
{
}

PC_ColumnSet::~PC_ColumnSet()
{
	UT_VECTOR_PURGEALL(PC_Column, m_vecColumns);
}

UT_Bool PC_ColumnSet::setAttributes(const XML_Char ** attrs)
{
	// a ColumnSet does not have any attributes at this time.
	return UT_TRUE;
}

UT_Bool PC_ColumnSet::addNewModel(const XML_Char ** attrs)
{
	const char * pszTypeName = UT_getAttribute("type", attrs);

	PC_Column * p = NULL;
	if (UT_stricmp(pszTypeName,PC_Column_Box::myName()) == 0)
		p = new PC_Column_Box();
	else if (UT_stricmp(pszTypeName,PC_Column_Circle::myName()) == 0)
		p = new PC_Column_Circle();
	if (!p)
		return UT_FALSE;

	if (!p->setAttributes(attrs))
	{
		delete p;
		return UT_FALSE;
	}
	m_vecColumns.addItem(p);
	return UT_TRUE;
}

PC_Column * PC_ColumnSet::getNthModel(UT_uint32 ndx) const
{
	if (ndx < m_vecColumns.getItemCount())
		return (PC_Column *)m_vecColumns.getNthItem(ndx);
	else
		return NULL;
}

void PC_ColumnSet::dump(FILE * fp)
{
	fprintf(fp,"\t\t\tColumnSet: 0x%08lx\n",(UT_uint32)this);

	UT_uint32 k;
	UT_uint32 kLimit = m_vecColumns.getItemCount();
	
	for (k=0; k<kLimit; k++)
	{
		PC_Column * p = (PC_Column *)m_vecColumns.getNthItem(k);
		UT_ASSERT(p);
		p->dump(fp);
	}
}

