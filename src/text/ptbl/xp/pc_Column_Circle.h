
#ifndef PC_COLUMN_CIRCLE_H
#define PC_COLUMN_CIRCLE_H

#include <stdio.h>						/* for dump only */
#include "ut_types.h"
#include "pc_Column.h"

/*****************************************************************/

class PC_Column_Circle : public PC_Column
{
public:
	PC_Column_Circle();
	virtual ~PC_Column_Circle();

	virtual UT_Bool setAttributes(const XML_Char ** attrs);
	virtual UT_Bool getNthAttr(int ndx, const char *& szName, const char *& szValue);
	virtual UT_Bool getAttr(const char * szName, const char *& szValue);

	virtual void dump(FILE * fp) const;
	
protected:
	char *			m_szLeft;
	char *			m_szTop;
	char *			m_szRadius;

public:
	static const char * myName(void);
};

#endif /* PC_COLUMN_CIRCLE_H */
