
#ifndef UTVECTOR_H
#define UTVECTOR_H

#include "ut_types.h"

// TODO change the 'int' types to 'UT_[su]int32' whichever is appropriate.

// ----------------------------------------------------------------
/*
	The following class is a simple, portable implementation of a vector.
	Following in Mozilla's footsteps, we don't use STL because templates 
	are not yet portable enough for our needs.  (Same goes for exceptions
	and namespaces, BTW.)
*/
class UT_Vector
{
public:
	UT_Vector();
	~UT_Vector();

	UT_sint32	addItem(void*);
	UT_sint32	addItem(void* p, UT_uint32 * pIndex);
	void*		getNthItem(UT_uint32 n) const;
	void*		getFirstItem() const;
	void*		getLastItem() const;
	UT_uint32	getItemCount() const;
	UT_sint32	findItem(void*);

	UT_sint32	insertItemAt(void*, UT_sint32 ndx);
	void		deleteNthItem(UT_uint32 n);
	void		clear();

protected:
	int				calcNewSpace();
	int				grow();
	
	void**			m_pEntries;
	UT_uint32		m_iCount;
	UT_uint32		m_iSpace;
	UT_uint32		m_iCutoffDouble;
	UT_uint32		m_iPostCutoffIncrement;
};

#endif /* UTVECTOR_H */
