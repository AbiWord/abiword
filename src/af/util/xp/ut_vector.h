 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiSource Utilities.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

#ifndef UTVECTOR_H
#define UTVECTOR_H

#include "ut_types.h"
#include "ut_assert.h"

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

	UT_sint32	insertItemAt(void*, UT_uint32 ndx);
	void		deleteNthItem(UT_uint32 n);
	void		clear();

protected:
	UT_uint32		calcNewSpace();
	UT_sint32		grow();
	
	void**			m_pEntries;
	UT_uint32		m_iCount;
	UT_uint32		m_iSpace;
	UT_uint32		m_iCutoffDouble;
	UT_uint32		m_iPostCutoffIncrement;
};

// NB: this macro is useful only in destructors
#define UT_VECTOR_PURGEALL(d, v)						\
	do	{	int utmax = v.getItemCount();				\
			for (int uti=utmax-1; uti>=0; uti--)		\
			{											\
				d* p = (d*) v.getNthItem(uti);			\
				UT_ASSERT(p);							\
				if (p)									\
					delete p;							\
			}											\
	} while (0)

#endif /* UTVECTOR_H */
