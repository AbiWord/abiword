
#include <stdlib.h>

// TODO change the 'int' types to 'UT_[su]int32' whichever is appropriate.

#include "ut_types.h"
#include "ut_vector.h"
#include "ut_assert.h"

UT_Vector::UT_Vector()
{
	m_iCutoffDouble = 128;
	m_iPostCutoffIncrement = 32;
	m_iCount = 0;
	m_iSpace = 0;
	m_pEntries = NULL;
}

void UT_Vector::clear()
{
	m_iCount = 0;
	m_iSpace = 0;
	free(m_pEntries);
	m_pEntries = NULL;
}

UT_Vector::~UT_Vector()
{
	if (m_pEntries)
	{
		free(m_pEntries);
	}
}

int UT_Vector::calcNewSpace()
{
	if (m_iSpace < m_iCutoffDouble)
	{
		if (m_iSpace > 0)
		{
			return m_iSpace * 2;
		}
		else
		{
			return m_iPostCutoffIncrement;
		}
	}
	else
	{
		return m_iSpace + m_iPostCutoffIncrement;
	}
}

int UT_Vector::getItemCount() const
{
	return m_iCount;
}

int UT_Vector::grow()
{
	int new_iSpace = calcNewSpace();

	void ** new_pEntries = (void**) calloc(new_iSpace, sizeof(void*));
	if (!new_pEntries)
	{
		return -1;
	}

	if (m_pEntries && (m_iCount > 0))
	{
		for (int i=0; i<m_iCount; i++)
		{
			new_pEntries[i] = m_pEntries[i];
		}

		free(m_pEntries);
	}

	m_iSpace = new_iSpace;
	m_pEntries = new_pEntries;

	return 0;
}

int UT_Vector::addItem(void* p, UT_uint32 * pIndex)
{
	int err = addItem(p);
	if (!err)
		*pIndex = m_iCount-1;
	return err;
}

int UT_Vector::addItem(void* p)
{
	if ((m_iCount+1) > m_iSpace)
	{
		int err = grow();
		if (err)
		{
			return err;
		}
	}

	m_pEntries[m_iCount++] = p;

	return 0;
}

void* UT_Vector::getNthItem(int n) const
{
	UT_ASSERT(m_pEntries);

	// TODO assert n in range
	return m_pEntries[n];
}

void* UT_Vector::getLastItem() const
{
	UT_ASSERT(m_iCount);

	return m_pEntries[m_iCount-1];
}

void* UT_Vector::getFirstItem() const
{
	UT_ASSERT(m_iCount > 0);
	UT_ASSERT(m_pEntries);

	return m_pEntries[0];
}

void UT_Vector::deleteNthItem(int n)
{
	if ((n < 0) || (n >= m_iCount))
		return;

	for (int k=0; k<m_iCount-1; k++)
		m_pEntries[k] = m_pEntries[k+1];
	m_pEntries[m_iCount-1] = 0;
	m_iCount--;

	return;
}


