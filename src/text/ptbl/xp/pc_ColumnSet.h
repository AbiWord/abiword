
#ifndef PC_COLUMNSET_H
#define PC_COLUMNSET_H

#include <stdio.h>						/* for dump only */
#include "ut_types.h"
#include "ut_vector.h"

class PC_Column;

/*****************************************************************/

class PC_ColumnSet
{
public:
	PC_ColumnSet();
	~PC_ColumnSet();

	UT_Bool				setAttributes(const XML_Char ** attrs);
	
	UT_Bool				addNewModel(const XML_Char ** attrs);
	PC_Column *			getNthModel(UT_uint32 ndx) const;

	void				dump(FILE * fp);

protected:
	UT_Vector			m_vecColumns;
};

#endif /* PC_COLUMNSET_H */
