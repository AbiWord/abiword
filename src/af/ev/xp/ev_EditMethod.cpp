/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */
 
#include <string.h>
#include <stdlib.h>

#include "ev_EditMethod.h"
#include "ut_assert.h"
#include "ut_vector.h"
#include "ut_hash.h"
#include "ut_string.h"

/*****************************************************************/
/*****************************************************************/

EV_EditMethodCallData::EV_EditMethodCallData()
	: m_xPos(0),
	  m_yPos(0),
	  m_dataLength(0),
	  m_bAllocatedData(false)
{
	  m_pData = NULL;
}


/* TF NOTE:
 I hate the fact that we need two almost identical constructors.  It really licks,
 not to mention the fact that nothing actually checks to see if the m_bAlocatedData
 flag is actually set, so covering this "failure" case may be pointless if other
 things don't play along.  Comment out this code just to see how well it would work.
*/
EV_EditMethodCallData::EV_EditMethodCallData(UT_UCSChar * pData, UT_uint32 dataLength)
	: m_xPos(0),
	  m_yPos(0)
{
	m_pData = new UT_UCSChar[dataLength];
	if (m_pData)
	{
		for (UT_uint32 k = 0; k < dataLength; k++)
			m_pData[k] = pData[k];
		m_dataLength = dataLength;
		m_bAllocatedData = true;
	}
	else								// since constructors can't fail, we create a zombie.
	{
		m_dataLength = 0;
		m_bAllocatedData = false;
	}
}

EV_EditMethodCallData::EV_EditMethodCallData(const char * pChar, UT_uint32 dataLength)
	: m_xPos(0),
	  m_yPos(0)
{
	m_pData = new UT_UCSChar[dataLength];
	if (m_pData)
	{
		for (UT_uint32 k = 0; k < dataLength; k++)
			m_pData[k] = pChar[k];
		m_dataLength = dataLength;
		m_bAllocatedData = true;
	}
	else								// since constructors can't fail, we create a zombie.
	{
		m_dataLength = 0;
		m_bAllocatedData = false;
	}
}

EV_EditMethodCallData::~EV_EditMethodCallData()
{
	delete [] m_pData;
}

/*****************************************************************/
/*****************************************************************/

EV_EditMethod::EV_EditMethod(const char * szName, EV_EditMethod_pFn fn, EV_EditMethodType emt, const char * szDescription)
	: m_szName(szName),
	  m_fn(fn),
	  m_emt(emt),
	  m_szDescription(szDescription)
{
}

EV_EditMethod_pFn EV_EditMethod::getFn() const
{
	return m_fn;
}

EV_EditMethodType EV_EditMethod::getType() const
{
	return m_emt;
}

inline const char * EV_EditMethod::getName() const
{
	return m_szName;
}

const char * EV_EditMethod::getDescription() const
{
	return m_szDescription;
}

/*****************************************************************/
/*****************************************************************/

EV_EditMethodContainer::EV_EditMethodContainer(UT_uint32 cStatic,EV_EditMethod arrayStaticEditMethods[])
{
	m_countStatic = cStatic;
	m_arrayStaticEditMethods = arrayStaticEditMethods;
}

EV_EditMethodContainer::~EV_EditMethodContainer()
{
	UT_VECTOR_PURGEALL(EV_EditMethod *, m_vecDynamicEditMethods);
}

bool EV_EditMethodContainer::addEditMethod(EV_EditMethod * pem)
{
	UT_ASSERT(pem);

	int error = m_vecDynamicEditMethods.addItem(pem);
	return (error == 0);
}

UT_uint32 EV_EditMethodContainer::countEditMethods()
{
	return m_countStatic + m_vecDynamicEditMethods.getItemCount();
}

EV_EditMethod * EV_EditMethodContainer::getNthEditMethod(UT_uint32 ndx)
{
	if (ndx < m_countStatic)
		return &m_arrayStaticEditMethods[ndx];
	else
		return (EV_EditMethod *)m_vecDynamicEditMethods.getNthItem(ndx-m_countStatic);
}

// for use in a binary search of an EV_EditMethod array
#ifdef __MRC__
extern "C"
#endif
static int ev_compar (const void * a, const void * b)
{
  const char * str = (const char *)a;
  EV_EditMethod * ev = (EV_EditMethod *)(b);

  return (strcmp (str, ev->getName()));
}

EV_EditMethod * EV_EditMethodContainer::findEditMethodByName(const char * szName) const
{
	if (!szName)
		return 0;

	EV_EditMethod *mthd = NULL;

	// TODO: make this also use a hashtable + bsearch

	// first, see if it's in our hashtable
	// TODO: should this be class-wide instead of static here?
	static UT_HashTable emHash (m_countStatic);

	HashValType entry = emHash.pick ((HashKeyType)szName);
	if (entry)
	  {
	    return (EV_EditMethod *)entry;
	  }

	// nope, bsearch for it in our private array
	mthd = (EV_EditMethod *)bsearch(szName, 
									m_arrayStaticEditMethods, 
									m_countStatic, 
									sizeof (EV_EditMethod),
									ev_compar);

	if (mthd)
	  {
	    // found it, insert it into our hash table for quicker lookup
	    // in the future and return
	    emHash.insert((HashKeyType)szName, (HashValType)mthd);
	    return mthd;
	  }

	// else do a linear search through our dynamic method vector

	UT_uint32 k, kLast;

	kLast = m_vecDynamicEditMethods.getItemCount();
	for (k=0; k<kLast; k++)
	{
		EV_EditMethod * pem = (EV_EditMethod *)m_vecDynamicEditMethods.getNthItem(k);
		if (strcmp(szName,pem->getName()) == 0)
			return pem;
	}

	return 0;
}

