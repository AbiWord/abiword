
#include "ut_assert.h"
#include "ut_stack.h"

UT_Bool UT_Stack::push(void * pVoid)
{
	UT_ASSERT(pVoid);
	UT_uint32 error = m_vecStack.addItem(pVoid);
	return (error == 0);
}

UT_Bool UT_Stack::pop(void ** ppVoid)
{
	UT_ASSERT(ppVoid);
	UT_uint32 indexEnd = m_vecStack.getItemCount();
	if (!indexEnd)
		return UT_FALSE;
	*ppVoid = m_vecStack.getLastItem();
	m_vecStack.deleteNthItem(indexEnd-1);
	return UT_TRUE;
}
