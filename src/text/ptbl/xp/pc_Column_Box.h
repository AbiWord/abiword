
#ifndef PC_COLUMN_BOX_H
#define PC_COLUMN_BOX_H

#include <stdio.h>						/* for dump only */
#include "ut_types.h"
#include "pc_Column.h"

/*****************************************************************/

class PC_Column_Box : public PC_Column
{
public:
	PC_Column_Box();
	virtual ~PC_Column_Box();

	virtual UT_Bool setAttributes(const XML_Char ** attrs);
	virtual UT_Bool getNthAttr(int ndx, const char *& szName, const char *& szValue);
	virtual UT_Bool getAttr(const char * szName, const char *& szValue);

	virtual void dump(FILE * fp) const;
	
protected:
	char *			m_szLeft;
	char *			m_szTop;
	char *			m_szWidth;
	char *			m_szHeight;

public:
	static const char * myName(void);
};

#endif /* PC_COLUMN_BOX_H */
