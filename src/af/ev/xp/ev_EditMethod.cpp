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

#include "ev_EditMethod.h"
#include "ut_assert.h"
#include "ut_vector.h"
#include "ut_string.h"

/*****************************************************************/
/*****************************************************************/

EV_EditMethodCallData::EV_EditMethodCallData(void)
{
	m_xPos = 0;
	m_yPos = 0;
	m_pData = 0;
	m_dataLength = 0;
	m_bAllocatedData = UT_FALSE;
}

EV_EditMethodCallData::EV_EditMethodCallData(UT_UCSChar * pData, UT_uint32 dataLength)
{
	m_xPos = 0;
	m_yPos = 0;
	m_pData = pData;
	m_dataLength = dataLength;
	m_bAllocatedData = UT_FALSE;
}

EV_EditMethodCallData::EV_EditMethodCallData(const char * pChar, UT_uint32 dataLength)
{
	m_xPos = 0;
	m_yPos = 0;
	m_pData = new UT_UCSChar[dataLength];
	if (m_pData)
	{
		for (UT_uint32 k=0; k<dataLength; k++)
			m_pData[k] = pChar[k];
		m_dataLength = dataLength;
		m_bAllocatedData = UT_TRUE;
	}
	else								// since constructors can't fail, we create a zombie.
	{
		m_dataLength = 0;
		m_bAllocatedData = UT_FALSE;
	}
}

EV_EditMethodCallData::~EV_EditMethodCallData()
{
	if (m_bAllocatedData)
		delete m_pData;
}

/*****************************************************************/
/*****************************************************************/

EV_EditMethod::EV_EditMethod(const char * szName,EV_EditMethod_pFn fn,EV_EditMethodType emt,const char * szDescription)
{
	m_szName = szName;					// we assume that the caller passed a static literal
	m_fn = fn;
	m_emt = emt;
	m_szDescription = szDescription;	// we assume that the caller passed a static literal
}

EV_EditMethod_pFn EV_EditMethod::getFn(void) const
{
	return m_fn;
}

EV_EditMethodType EV_EditMethod::getType(void) const
{
	return m_emt;
}

inline const char * EV_EditMethod::getName(void) const
{
	return m_szName;
}

const char * EV_EditMethod::getDescription(void) const
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

UT_Bool EV_EditMethodContainer::addEditMethod(EV_EditMethod * pem)
{
	UT_ASSERT(pem);

	int error = m_vecDynamicEditMethods.addItem(pem);
	return (error == 0);
}

UT_uint32 EV_EditMethodContainer::countEditMethods(void)
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

EV_EditMethod * EV_EditMethodContainer::findEditMethodByName(const char * szName) const
{
	if (!szName)
		return 0;

	UT_uint32 k, kLast;
	for (k=0; k<m_countStatic; k++)
		if (strcmp(szName,m_arrayStaticEditMethods[k].getName()) == 0)
			return (EV_EditMethod *)&m_arrayStaticEditMethods[k];
	
	kLast = m_vecDynamicEditMethods.getItemCount();
	for (k=0; k<kLast; k++)
	{
		EV_EditMethod * pem = (EV_EditMethod *)m_vecDynamicEditMethods.getNthItem(k);
		if (strcmp(szName,pem->getName()) == 0)
			return pem;
	}

	return 0;
}

