
#ifndef UT_STACK_H
#define UT_STACK_H

#include "ut_types.h"
#include "ut_vector.h"

class UT_Stack
{
public:
	UT_Bool			push(void * pVoid);
	UT_Bool			pop(void ** ppVoid);
	UT_Bool			viewTop(void ** ppVoid) const;

protected:
	UT_Vector		m_vecStack;
};

#endif /* UT_STACK_H */
