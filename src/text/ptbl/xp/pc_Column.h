
#ifndef PC_COLUMN_H
#define PC_COLUMN_H

#include <stdio.h>						/* for dump only */
#include "ut_types.h"
#include "ut_misc.h"
#include "xmlparse.h"

/*****************************************************************/

class PC_Column
{
public:
	virtual UT_Bool setAttributes(const XML_Char ** attrs) = 0;
	virtual UT_Bool getNthAttr(int ndx, const char *& szName, const char *& szValue) = 0;
	virtual UT_Bool getAttr(const char * szName, const char *& szValue) = 0;

	virtual void dump(FILE * fp) const = 0;
	
protected:
};

#endif /* PC_COLUMN_H */
