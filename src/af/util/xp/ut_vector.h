
#ifndef UTVECTOR_H
#define UTVECTOR_H

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
	void*	getNthItem(int n);
	void*	getFirstItem();
	void*	getLastItem();
	int		getItemCount();

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
