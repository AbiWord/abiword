
#include <memory.h>
#include <malloc.h>
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_types.h"
#include "pc_Column.h"
#include "pc_Column_Circle.h"


PC_Column_Circle::PC_Column_Circle()
{
}

PC_Column_Circle::~PC_Column_Circle()
{
	if (m_szLeft)
		free(m_szLeft);
	if (m_szTop)
		free(m_szTop);
	if (m_szRadius)
		free(m_szRadius);
}

UT_Bool PC_Column_Circle::setAttributes(const XML_Char ** attrs)
{
	const char * pszLeft = UT_getAttribute("left", attrs);
	const char * pszTop = UT_getAttribute("top", attrs);
	const char * pszRadius = UT_getAttribute("radius", attrs);

	if (!pszLeft || !pszTop || !pszRadius)
		return UT_FALSE;
	
	// TODO validate these strings as dimensioned numbers.
	// TODO left,top should be numbers.
	// TODO radius should be a number or '*'.

	UT_cloneString(m_szLeft,pszLeft);
	UT_cloneString(m_szTop,pszTop);
	UT_cloneString(m_szRadius,pszRadius);

	if (!m_szLeft || !m_szTop || !m_szRadius)
		return UT_FALSE;

	return UT_TRUE;
}

UT_Bool PC_Column_Circle::getNthAttr(int ndx, const char *& szName, const char *& szValue)
{
	switch (ndx)
	{
	case 0:		szName = "type";		szValue = myName();		return UT_TRUE;
	case 1:		szName = "left";		szValue = m_szLeft;		return UT_TRUE;
	case 2:		szName = "top";			szValue = m_szTop;		return UT_TRUE;
	case 3:		szName = "radius";		szValue = m_szRadius;	return UT_TRUE;
	default:
		return UT_FALSE;
	}
}

UT_Bool PC_Column_Circle::getAttr(const char * szName, const char *& szValue)
{
#define	IfMatch(n,v)	do { if (UT_stricmp(szName,(n))==0) { szValue = (v); return UT_TRUE; }} while (0)

	IfMatch("type",myName());
	IfMatch("left",m_szLeft);
	IfMatch("top",m_szTop);
	IfMatch("radius",m_szRadius);

	return UT_FALSE;
#undef IfMatch
}

const char * PC_Column_Circle::myName(void)
{
	return "Circle";
}

void PC_Column_Circle::dump(FILE * fp) const
{
	fprintf(fp,"\t\t\t\tCircleColumnDump: 0x%08lx\n",(UT_uint32)this);
}
