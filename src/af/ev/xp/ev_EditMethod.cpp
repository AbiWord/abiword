/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */
 
#include <string.h>
#include <stdlib.h>

#include "ev_EditMethod.h"
#include "ut_assert.h"
#include "ut_vector.h"
#include "ut_string.h"
#include "ut_string_class.h"

#include "xap_App.h"
#include "xap_Frame.h"

/*****************************************************************/
/*****************************************************************/

EV_EditMethodCallData::EV_EditMethodCallData()
	:  m_pData(0),
	   m_dataLength(0),
	   m_bAllocatedData(false),
	   m_xPos(0),
	   m_yPos(0)
{
}

EV_EditMethodCallData::EV_EditMethodCallData(const UT_String& stScriptName)
	: m_pData(0),
	  m_dataLength(0),
	  m_bAllocatedData(false),
	  m_xPos(0),
	  m_yPos(0),
	  m_stScriptName(stScriptName)
{
}

/* TF NOTE:
 I hate the fact that we need two almost identical constructors.  It really licks,
 not to mention the fact that nothing actually checks to see if the m_bAlocatedData
 flag is actually set, so covering this "failure" case may be pointless if other
 things don't play along.  Comment out this code just to see how well it would work.
*/
EV_EditMethodCallData::EV_EditMethodCallData(const UT_UCSChar * pData, UT_uint32 dataLength)
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

EV_EditMethod::EV_EditMethod(const char * szName, EV_EditMethod_pFn fn, EV_EditMethodType emt,
							 const char * szDescription)
	: m_szName(szName),
	  m_fn(fn),
	  m_CtxtFn(0),
	  m_emt(emt),
	  m_szDescription(szDescription),
	  m_context(0)
{
}

EV_EditMethod::EV_EditMethod(const char * szName, EV_EditMethod_pCtxtFn fn, EV_EditMethodType emt,
							 const char * szDescription, void * context)
	: m_szName(szName),
	  m_fn(0),
	  m_CtxtFn(fn),
	  m_emt(emt),
	  m_szDescription(szDescription),
	  m_context(context)
{
}

bool EV_EditMethod::Fn(AV_View * pView, EV_EditMethodCallData * pCallData) const
{
	if (m_fn)
		return (*m_fn) (pView, pCallData);
	else if (m_CtxtFn)
		return (*m_CtxtFn) (pView, pCallData, m_context);

	UT_ASSERT(m_fn || m_CtxtFn);
	return false;
}

EV_EditMethodType EV_EditMethod::getType() const
{
	return m_emt;
}

const char * EV_EditMethod::getName() const
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

bool EV_EditMethodContainer::removeEditMethod(EV_EditMethod * pem)
{
	UT_ASSERT(pem);
	xxx_UT_DEBUGMSG(("Bsearch for name \n"));

	UT_sint32 pos = m_vecDynamicEditMethods.findItem ( pem ) ;

	if ( pos >= 0 )
	    m_vecDynamicEditMethods.deleteNthItem(pos);
	return (pos >= 0) ;
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
		return m_vecDynamicEditMethods.getNthItem(ndx-m_countStatic);
}

// for use in a binary search of an EV_EditMethod array
static int ev_compar (const void * a, const void * b)
{
	const char * str = static_cast<const char *>(a);
	const EV_EditMethod * ev = static_cast<const EV_EditMethod *>(b);

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
	static std::map<std::string,EV_EditMethod *> emHash;
	std::map<std::string,EV_EditMethod *>::const_iterator iter;
	iter = emHash.find(szName);
	if (iter != emHash.end())
		return iter->second;

	// nope, bsearch for it in our private array
	mthd = static_cast<EV_EditMethod *>(bsearch(szName, 
					m_arrayStaticEditMethods, 
					m_countStatic, 
					sizeof (EV_EditMethod),
					ev_compar));

	if (mthd)
	{
	    // found it, insert it into our hash table for quicker lookup
	    // in the future and return
	    emHash.insert(std::make_pair(szName, mthd));
	    return mthd;
	}

	// else do a linear search through our dynamic method vector

	UT_uint32 k, kLast;
	xxx_UT_DEBUGMSG(("Linear search for it \n"));
	kLast = m_vecDynamicEditMethods.getItemCount();
	for (k=0; k<kLast; k++)
	{
		xxx_UT_DEBUGMSG(("Looking at method %d \n",k));
		EV_EditMethod * pem = m_vecDynamicEditMethods.getNthItem(k);
		if(pem == NULL)
			continue;
		if(pem->getName() == NULL)
			continue;
		if (strcmp(szName,pem->getName()) == 0)
			return pem;
	}

	return 0;
}

/*****************************************************************/
/*****************************************************************/

bool ev_EditMethod_invoke (const EV_EditMethod * pEM, 
			   EV_EditMethodCallData * pData)
{
  // no method or no call data == bad joo joo - return false
  UT_ASSERT(pEM);
  UT_ASSERT(pData);
  if ( !pEM || !pData )
    return false ;

  // no controlling view == bad joo joo - return false
  // Actually allow this for plugins invoked from the command line
  //
  AV_View * pView = NULL;
  XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
  if(!pFrame)
  {
	  return pEM->Fn(pView, pData);
  }
  pView = pFrame->getCurrentView() ;
  UT_return_val_if_fail(pView, false);

  // return whatever the method says to based on the data at hand
  return pEM->Fn(pView, pData);
}

bool ev_EditMethod_invoke (const EV_EditMethod * pEM, const UT_String & data)
{
  EV_EditMethodCallData callData ( data.c_str(), static_cast<UT_uint32>(data.size()) ) ;
  return ev_EditMethod_invoke ( pEM, &callData ) ;
}

bool ev_EditMethod_invoke (const EV_EditMethod * pEM, const UT_UCS4String & data)
{
  EV_EditMethodCallData callData ( data.ucs4_str(), static_cast<UT_uint32>(data.size()) ) ;
  return ev_EditMethod_invoke ( pEM, &callData ) ;
}

bool ev_EditMethod_invoke (const char * methodName, const UT_String & data)
{
  return ev_EditMethod_invoke ( ev_EditMethod_lookup ( methodName ), data ) ;
}

bool ev_EditMethod_invoke (const char * methodName, const UT_UCS4String & data)
{
  return ev_EditMethod_invoke ( ev_EditMethod_lookup ( methodName ), data ) ;
}

bool ev_EditMethod_invoke (const char * methodName, const char * data)
{
  UT_return_val_if_fail(data, false);
  return ev_EditMethod_invoke ( methodName, UT_String(data) ) ;
}

bool ev_EditMethod_invoke (const char * methodName, const UT_UCSChar * data)
{
  UT_return_val_if_fail(data, false);
  return ev_EditMethod_invoke ( methodName, UT_UCS4String(data) ) ;
}

bool ev_EditMethod_invoke (const UT_String& methodName, const UT_String & data)
{
  return ev_EditMethod_invoke ( methodName.c_str(), data ) ;
}

bool ev_EditMethod_invoke (const UT_String& methodName, const UT_UCS4String & data)
{
  return ev_EditMethod_invoke ( methodName.c_str(), data ) ;
}

/*****************************************************************/
/*****************************************************************/

bool ev_EditMethod_exists (const char * methodName)
{
  return ( ev_EditMethod_lookup ( methodName ) != NULL ) ;
}

bool ev_EditMethod_exists (const UT_String & methodName)
{
  return ev_EditMethod_exists ( methodName.c_str() ) ;
}

/*****************************************************************/
/*****************************************************************/

EV_EditMethod* ev_EditMethod_lookup (const char * methodName)
{
  UT_ASSERT(methodName);
  EV_EditMethodContainer* pEMC = XAP_App::getApp()->getEditMethodContainer() ;
  return pEMC->findEditMethodByName ( methodName ) ;
}

EV_EditMethod* ev_EditMethod_lookup (const UT_String & methodName)
{
  return ev_EditMethod_lookup ( methodName.c_str() ) ;
}


