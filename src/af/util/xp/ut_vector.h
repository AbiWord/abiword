
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

	int		addItem(void*);
	int		addItem(void* p, UT_uint32 * pIndex);
	void*	getNthItem(int n) const;
	void*	getFirstItem() const;
	void*	getLastItem() const;
	int		getItemCount() const;

	void	insertItemAt(void*, int ndx);
	void	deleteNthItem(int n);
	void	clear();

protected:
	int		calcNewSpace();
	int		grow();
	void**	m_pEntries;
	int		m_iCount;
	int		m_iSpace;
	int		m_iCutoffDouble;
	int		m_iPostCutoffIncrement;
};

#endif /* UTVECTOR_H */
