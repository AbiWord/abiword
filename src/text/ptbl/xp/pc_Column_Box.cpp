
#include <memory.h>
#include <malloc.h>
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_types.h"
#include "pc_Column.h"
#include "pc_Column_Box.h"

PC_Column_Box::PC_Column_Box()
{
}

PC_Column_Box::~PC_Column_Box()
{
	if (m_szLeft)
		free(m_szLeft);
	if (m_szTop)
		free(m_szTop);
	if (m_szWidth)
		free(m_szWidth);
	if (m_szHeight)
		free(m_szHeight);
}

UT_Bool PC_Column_Box::setAttributes(const XML_Char ** attrs)
{
	const char * pszLeft = UT_getAttribute("left", attrs);
	const char * pszTop = UT_getAttribute("top", attrs);
	const char * pszWidth = UT_getAttribute("width", attrs);
	const char * pszHeight = UT_getAttribute("height", attrs);

	if (!pszLeft || !pszTop || !pszWidth || !pszHeight)
		return UT_FALSE;
	
	// TODO validate these strings as dimensioned numbers.
	// TODO left,top should be numbers.
	// TODO width,height should be numbers or '*'.

	UT_cloneString(m_szLeft,pszLeft);
	UT_cloneString(m_szTop,pszTop);
	UT_cloneString(m_szWidth,pszWidth);
	UT_cloneString(m_szHeight,pszHeight);

	if (!m_szLeft || !m_szTop || !m_szWidth || !m_szHeight)
		return UT_FALSE;

	return UT_TRUE;
}

UT_Bool PC_Column_Box::getNthAttr(int ndx, const char *& szName, const char *& szValue)
{
	switch (ndx)
	{
	case 0:		szName = "type";		szValue = myName();		return UT_TRUE;
	case 1:		szName = "left";		szValue = m_szLeft;		return UT_TRUE;
	case 2:		szName = "top";			szValue = m_szTop;		return UT_TRUE;
	case 3:		szName = "width";		szValue = m_szWidth;	return UT_TRUE;
	case 4:		szName = "height";		szValue = m_szHeight;	return UT_TRUE;
	default:
		return UT_FALSE;
	}
}

UT_Bool PC_Column_Box::getAttr(const char * szName, const char *& szValue)
{
#define	IfMatch(n,v)	do { if (UT_stricmp(szName,(n))==0) { szValue = (v); return UT_TRUE; }} while (0)

	IfMatch("type",myName());
	IfMatch("left",m_szLeft);
	IfMatch("top",m_szTop);
	IfMatch("width",m_szWidth);
	IfMatch("height",m_szHeight);

	return UT_FALSE;
#undef IfMatch
}

const char * PC_Column_Box::myName(void)
{
	return "Box";
}

void PC_Column_Box::dump(FILE * fp) const
{
	fprintf(fp,"\t\t\t\tBoxColumnDump: 0x%08lx\n",(UT_uint32)this);
}
