/* AbiWord
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


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_alphahash.h"
#include "ut_string.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "pp_AttrProp.h"

/****************************************************************/
PP_AttrProp::PP_AttrProp()
{
	m_pAttributes = NULL;
	m_pProperties = NULL;
	m_bIsReadOnly = false;
	m_checkSum = 0;
}

PP_AttrProp::~PP_AttrProp()
{
	DELETEP(m_pAttributes);
	DELETEP(m_pProperties);
}

size_t PP_AttrProp::getPropertyCount (void) const
{
  // return m_pProperties->getEntryCount();
        return m_pAttributes->getEntryCount();
}

bool	PP_AttrProp::setAttributes(const XML_Char ** attributes)
{
	if (!attributes)
		return true;

	const XML_Char ** pp = attributes;
	while (*pp)
	{
		if (!setAttribute(pp[0],pp[1]))
			return false;
		pp += 2;
	}
	return true;
}

bool PP_AttrProp::setAttributes(const UT_Vector * pVector)
{
	UT_uint32 kLimit = pVector->getItemCount();
	for (UT_uint32 k=0; k+1<kLimit; k+=2)
	{
		const XML_Char * pName = (const XML_Char *)pVector->getNthItem(k);
		const XML_Char * pValue = (const XML_Char *)pVector->getNthItem(k+1);
		if (!setAttribute(pName,pValue))
			return false;
	}
	return true;
}


bool	PP_AttrProp::setProperties(const XML_Char ** properties)
{
	if (!properties)
		return true;

	const XML_Char ** pp = properties;
	while (*pp)
	{
		if (!setProperty(pp[0],pp[1]))
			return false;
		pp += 2;
	}
	return true;
}

bool	PP_AttrProp::setAttribute(const XML_Char * szName, const XML_Char * szValue)
{
	// TODO when this assert fails, switch this file to use UT_XML_ version of str*() functions.
	UT_ASSERT(sizeof(char)==sizeof(XML_Char));
	
	if (0 == UT_strcmp(szName, PT_PROPS_ATTRIBUTE_NAME))	// PROPS -- cut value up into properties
	{
		char * pOrig = NULL;
		if (!UT_cloneString(pOrig,szValue))
		{
			UT_DEBUGMSG(("setAttribute: UT_cloneString failed on [%s]\n",szValue));
			return false;
		}

		// This function parses out CSS properties, separated by semicolons.

		char *z = pOrig;
		int bDone = 0;
		while (!bDone)
		{
			// p will point to the property name.  q will be the property value.
			char *p = z;
			char *q = p;
			// skip the whitespace before the property name
			while (isspace(*p))
				p++;

			// skip to the colon to find the value
			while (*q && (*q != ':'))
				q++;

			// if there was no colon, this is invalid
			if (!*q)
			{
				free(pOrig);
				return false;
			}

			// zero-out the colon, thus separating into two strings.
			*q = 0;
			q++;
			
			// now, search ahead for the next semicolon, and separate this property from the next
			z = q;
			while (*z && (*z != ';'))
				z++;

			if (*z == ';')
			{
				*z = 0;
				z++;
			}
			else
			{
				bDone = 1;
			}

			// skip the whitespace before the property value
			while (isspace(*q))
				q++;

			setProperty((const XML_Char*)p, (const XML_Char*)q);
		}

		free(pOrig);
		return true;
	}
	else								// not "PROPS" -- add to attribute list
	{
		if (!m_pAttributes)
		{
			m_pAttributes = new UT_AlphaHashTable(5);
			if (!m_pAttributes)
			{
				UT_DEBUGMSG(("setAttribute: could not allocate hash table.\n"));
				return false;
			}
		}

		// make sure we store attribute names in lowercase
		UT_ASSERT(sizeof(char) == sizeof(XML_Char));
		char * copy;
		if (!UT_cloneString(copy, szName))
		{
			UT_DEBUGMSG(("setAttribute: could not allocate lowercase copy.\n"));
			return false;
		}

		UT_lowerString(copy);
			
		bool bRet = (m_pAttributes->addEntry(copy, (char*)szValue, NULL) == 0);

		FREEP(copy);

		return bRet;
	}
}

bool	PP_AttrProp::setProperty(const XML_Char * szName, const XML_Char * szValue)
{
	if (!m_pProperties)
	{
		m_pProperties = new UT_AlphaHashTable(5);
		if (!m_pProperties)
		{
			UT_DEBUGMSG(("setProperty: could not allocate hash table.\n"));
			return false;
		}
	}

	UT_HashEntry* pEntry = m_pProperties->findEntry((char*)szName);
	if (pEntry)
		return (m_pProperties->setEntry(pEntry, (char*)szValue, NULL) == 0);
	else
		return (m_pProperties->addEntry((char*)szName, (char*)szValue, NULL) == 0);
}

bool	PP_AttrProp::getNthAttribute(int ndx, const XML_Char *& szName, const XML_Char *& szValue) const
{
	if (!m_pAttributes)
		return false;
	if (ndx >= m_pAttributes->getEntryCount())
		return false;
	UT_HashEntry * pEntry = m_pAttributes->getNthEntryAlpha(ndx);
	if (!pEntry)
		return false;
	szName = pEntry->pszLeft;
	szValue = pEntry->pszRight;
	return true;
}

bool	PP_AttrProp::getNthProperty(int ndx, const XML_Char *& szName, const XML_Char *& szValue) const
{
	if (!m_pProperties)
		return false;
	if (ndx >= m_pProperties->getEntryCount())
		return false;
	UT_HashEntry * pEntry = m_pProperties->getNthEntryAlpha(ndx);
	if (!pEntry)
		return false;
	szName = pEntry->pszLeft;
	szValue = pEntry->pszRight;
	return true;
}

bool PP_AttrProp::getProperty(const XML_Char * szName, const XML_Char *& szValue) const
{
	if (!m_pProperties)
		return false;
	
	UT_HashEntry* pEntry = m_pProperties->findEntry((char*)szName);
	if (!pEntry)
		return false;

	szValue = pEntry->pszRight;

	return true;
}

bool PP_AttrProp::getAttribute(const XML_Char * szName, const XML_Char *& szValue) const
{
	if (!m_pAttributes)
		return false;
	
	UT_HashEntry* pEntry = m_pAttributes->findEntry((char*)szName);
	if (!pEntry)
		return false;

	szValue = pEntry->pszRight;

	return true;
}

bool PP_AttrProp::hasProperties(void) const
{
	if (!m_pProperties)
		return false;

	return (m_pProperties->getEntryCount() > 0);
}

bool PP_AttrProp::areAlreadyPresent(const XML_Char ** attributes, const XML_Char ** properties) const
{
	// return TRUE if each attribute and property is already present
	// and has the same value as what we have.

	// TODO consider using the fact that we are now (Dec 12 1998) using
	// TODO alpha-hash-table rather than just a hash-table to optimize
	// TODO these loops somewhat.
	
	if (attributes && *attributes)
	{
		const XML_Char ** p = attributes;
		while (*p)
		{
			UT_ASSERT(p[1] && *p[1]);	// require value for each name
			const XML_Char * szValue = NULL;
			if (!getAttribute(p[0],szValue))
				return false;		// item not present
			if (UT_XML_stricmp(p[1],szValue)!=0)
				return false;		// item has different value
			p += 2;
		}
	}

	if (properties && *properties)
	{
		const XML_Char ** p = properties;
		while (*p)
		{
			/*
				Jeff, I weakened the following assert because we 
				*want* to represent no tabstops as an empty string.
				If this isn't safe, let me know.   -- PCR
			*/
			UT_ASSERT(p[1] /* && *p[1] */);	// require value for each name
			const XML_Char * szValue = NULL;
			if (!getProperty(p[0],szValue))
				return false;		// item not present
			if (UT_XML_stricmp(p[1],szValue)!=0)
				return false;		// item has different value
			p += 2;
		}
	}

	return true;						// everything matched
}

bool PP_AttrProp::areAnyOfTheseNamesPresent(const XML_Char ** attributes, const XML_Char ** properties) const
{
	// return TRUE if any attribute- or property-name is present.
	// this is like areAlreadyPresent() but we don't care about
	// the values.

	// TODO consider using the fact that we are now (Dec 12 1998) using
	// TODO alpha-hash-table rather than just a hash-table to optimize
	// TODO these loops somewhat.

	if (attributes && *attributes)
	{
		const XML_Char ** p = attributes;
		while (*p)
		{
			const XML_Char * szValue = NULL;
			if (getAttribute(p[0],szValue))
				return true;
			p += 2;						// skip over value
		}
	}

	if (properties && *properties)
	{
		const XML_Char ** p = properties;
		while (*p)
		{
			const XML_Char * szValue = NULL;
			if (getProperty(p[0],szValue))
				return true;
			p += 2;						// skip over value
		}
	}

	return false;					// didn't find any
}

bool PP_AttrProp::isExactMatch(const PP_AttrProp * pMatch) const
{
#ifdef PT_TEST
	static UT_uint32 s_Calls = 0;
	static UT_uint32 s_PassedCheckSum = 0;
	static UT_uint32 s_Matches = 0;
#endif

	// return TRUE iff we exactly match the AP given.

#ifdef PT_TEST
	s_Calls++;
#endif

	UT_ASSERT(pMatch);
	UT_ASSERT(m_bIsReadOnly && pMatch->m_bIsReadOnly);
	if (m_checkSum != pMatch->m_checkSum)
		return false;

#ifdef PT_TEST
	s_PassedCheckSum++;
#endif

	UT_uint32 countMyAttrs = ((m_pAttributes) ? m_pAttributes->getEntryCount() : 0);
	UT_uint32 countMatchAttrs = ((pMatch->m_pAttributes) ? pMatch->m_pAttributes->getEntryCount() : 0);
	if (countMyAttrs != countMatchAttrs)
		return false;

	UT_uint32 countMyProps = ((m_pProperties) ? m_pProperties->getEntryCount() : 0);
	UT_uint32 countMatchProps = ((pMatch->m_pProperties) ? pMatch->m_pProperties->getEntryCount() : 0);
	if (countMyProps != countMatchProps)
		return false;

	UT_uint32 k;

	for (k=0; (k < countMyAttrs); k++)
	{
		UT_HashEntry * pMyEntry = m_pAttributes->getNthEntryAlpha(k);
		UT_HashEntry * pMatchEntry = pMatch->m_pAttributes->getNthEntryAlpha(k);
		if (UT_XML_stricmp(pMyEntry->pszLeft,pMatchEntry->pszLeft) != 0)
			return false;
		if (UT_XML_stricmp(pMyEntry->pszRight,pMatchEntry->pszRight) != 0)
			return false;
	}

	for (k=0; (k < countMyProps); k++)
	{
		UT_HashEntry * pMyEntry = m_pProperties->getNthEntryAlpha(k);
		UT_HashEntry * pMatchEntry = pMatch->m_pProperties->getNthEntryAlpha(k);
		if (UT_XML_stricmp(pMyEntry->pszLeft,pMatchEntry->pszLeft) != 0)
			return false;
		if (UT_XML_stricmp(pMyEntry->pszRight,pMatchEntry->pszRight) != 0)
			return false;
	}

#ifdef PT_TEST
	s_Matches++;
#endif

	return true;
}

PP_AttrProp * PP_AttrProp::cloneWithReplacements(const XML_Char ** attributes,
												 const XML_Char ** properties,
												 bool bClearProps) const
{
	// create a new AttrProp based upon the given one
	// and adding or replacing the items given.
	// return NULL on failure.

	// first, create a new AttrProp using just the values given.

	PP_AttrProp * papNew = new PP_AttrProp();
	if (!papNew)
		goto Failed;
	if (!papNew->setAttributes(attributes) || !papNew->setProperties(properties))
		goto Failed;
	
	// next, add any items that we have that are not present
	// (have not been overridden) in the new one.

	UT_uint32 k;
	const XML_Char * n;
	const XML_Char * v;
	const XML_Char * vNew;
	
	k = 0;
	while (getNthAttribute(k++,n,v))
	{
		// TODO decide if/whether to allow PT_PROPS_ATTRIBUTE_NAME here.
		// TODO The issue is: we use it to store the CSS properties and
		// TODO when we see it, we expand the value into one or more
		// TODO properties.  if we allow it to be given here, should
		// TODO we blowaway all of the existing properties and create
		// TODO them from this?  or should we expand it and override
		// TODO individual properties?  
		// TODO for now, we just barf on it.
		UT_ASSERT(UT_XML_stricmp(n,PT_PROPS_ATTRIBUTE_NAME)!=0); // cannot handle PROPS here
		if (!papNew->getAttribute(n,vNew))
			if (!papNew->setAttribute(n,v))
				goto Failed;
	}

	if (!bClearProps)
	{
		k = 0;
		while (getNthProperty(k++,n,v))
		{
			if (!papNew->getProperty(n,vNew))
				if (!papNew->setProperty(n,v))
					goto Failed;
		}
	}

	return papNew;

Failed:
	DELETEP(papNew);
	return NULL;
}

PP_AttrProp * PP_AttrProp::cloneWithElimination(const XML_Char ** attributes,
												const XML_Char ** properties) const
{
	// create a new AttrProp based upon the given one
	// and removing the items given.
	// return FALSE on failure.

	// first, create an empty AttrProp.

	PP_AttrProp * papNew = new PP_AttrProp();
	if (!papNew)
		goto Failed;
	
	UT_uint32 k;
	const XML_Char * n;
	const XML_Char * v;
	
	k = 0;
	while (getNthAttribute(k++,n,v))
	{
		// for each attribute in the old set, add it to the
		// new set only if it is not present in the given array.

		if (attributes && *attributes)
		{
			const XML_Char ** p = attributes;
			while (*p)
			{
				UT_ASSERT(UT_XML_stricmp(p[0],PT_PROPS_ATTRIBUTE_NAME)!=0); // cannot handle PROPS here
				if (UT_XML_stricmp(n,p[0])!=0)		// found it, so we don't put it in the result.
					goto DoNotIncludeAttribute;
				p += 2;								// skip over value
			}
		}

		// we didn't find it in the given array, add it to the new set.
		
		if (!papNew->setAttribute(n,v))
			goto Failed;

	DoNotIncludeAttribute:
		;
	}

	k = 0;
	while (getNthProperty(k++,n,v))
	{
		// for each property in the old set, add it to the
		// new set only if it is not present in the given array.

		if (properties && *properties)
		{
			const XML_Char ** p = properties;
			while (*p)
			{
				if (UT_XML_stricmp(n,p[0])!=0)		// found it, so we don't put it in the result.
					goto DoNotIncludeProperty;
				p += 2;
			}
		}

		// we didn't find it in the given array, add it to the new set.
		
		if (!papNew->setProperty(n,v))
			goto Failed;

	DoNotIncludeProperty:
		;
	}

	return papNew;

Failed:
	DELETEP(papNew);
	return NULL;
}

void PP_AttrProp::markReadOnly(void)
{
	UT_ASSERT(!m_bIsReadOnly);
	m_bIsReadOnly = true;
	_computeCheckSum();
}

void PP_AttrProp::_computeCheckSum(void)
{
	m_checkSum = 0;

	// compute some easy-to-compute checksum for this AP. something
	// that is guaranteed to return the same result for the same input.
	// something that we can use like a hash for a quick check in
	// isExactMatch() to quickly dismiss different APs.  it should be
	// based strictly on the content of the name/value pairs and be
	// pointer- and order-independent.

	UT_uint32 k;
	UT_uint32 countMyAttrs = ((m_pAttributes) ? m_pAttributes->getEntryCount() : 0);
	for (k=0; (k < countMyAttrs); k++)
	{
		UT_HashEntry * pMyEntry = m_pAttributes->getNthEntryAlpha(k);
		m_checkSum += UT_XML_strlen(pMyEntry->pszLeft);
		m_checkSum += UT_XML_strlen(pMyEntry->pszRight);
	}
	UT_uint32 countMyProps = ((m_pProperties) ? m_pProperties->getEntryCount() : 0);
	for (k=0; (k < countMyProps); k++)
	{
		UT_HashEntry * pMyEntry = m_pProperties->getNthEntryAlpha(k);
		m_checkSum += UT_XML_strlen(pMyEntry->pszLeft);
		m_checkSum += UT_XML_strlen(pMyEntry->pszRight);
	}

	return;
}

UT_uint32 PP_AttrProp::getCheckSum(void) const
{
	UT_ASSERT(m_bIsReadOnly);
	return m_checkSum;
}

void PP_AttrProp::operator = (const PP_AttrProp &Other)
{
	UT_uint32 countMyAttrs = ((Other.m_pAttributes) ? Other.m_pAttributes->getEntryCount() : 0);

	UT_uint32 Index;
	for(Index = 0; Index < countMyAttrs; Index++)
	{
		const XML_Char * szName;
		const XML_Char * szValue;
		if(Other.getNthAttribute(Index, szName, szValue))
		{
			setAttribute(szName, szValue);
		}
	}

	UT_uint32 countMyProps = ((Other.m_pProperties) ? Other.m_pProperties->getEntryCount() : 0);

	for(Index = 0; Index < countMyProps; Index++)
	{
		const XML_Char * szName;
		const XML_Char * szValue;
		if(Other.getNthProperty(Index, szName, szValue))
		{
			setProperty(szName, szValue);
		}
	}
		
}

