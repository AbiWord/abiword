/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 


#include <memory.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_hash.h"
#include "ut_string.h"
#include "ut_vector.h"
#include "xmlparse.h"
#include "pt_Types.h"
#include "pp_AttrProp.h"

PP_AttrProp::PP_AttrProp()
{
	m_pAttributes = NULL;
	m_pProperties = NULL;
}

PP_AttrProp::~PP_AttrProp()
{
	if (m_pAttributes)
		delete m_pAttributes;

	if (m_pProperties)
		delete m_pProperties;
}

UT_Bool	PP_AttrProp::setAttributes(const XML_Char ** attributes)
{
	if (!attributes)
		return UT_TRUE;

	const XML_Char ** pp = attributes;
	while (*pp)
	{
		if (!setAttribute(pp[0],pp[1]))
			return UT_FALSE;
		pp += 2;
	}
	return UT_TRUE;
}

UT_Bool PP_AttrProp::setAttributes(const UT_Vector * pVector)
{
	UT_uint32 kLimit = pVector->getItemCount();
	for (UT_uint32 k=0; k+1<kLimit; k+=2)
	{
		const XML_Char * pName = (const XML_Char *)pVector->getNthItem(k);
		const XML_Char * pValue = (const XML_Char *)pVector->getNthItem(k+1);
		if (!setAttribute(pName,pValue))
			return UT_FALSE;
	}
	return UT_TRUE;
}


UT_Bool	PP_AttrProp::setProperties(const XML_Char ** properties)
{
	if (!properties)
		return UT_TRUE;

	const XML_Char ** pp = properties;
	while (*pp)
	{
		if (!setProperty(pp[0],pp[1]))
			return UT_FALSE;
		pp += 2;
	}
	return UT_TRUE;
}

UT_Bool	PP_AttrProp::setAttribute(const XML_Char * szName, const XML_Char * szValue)
{
	// TODO when this assert fails, switch this file to use UT_XML_ version of str*() functions.
	UT_ASSERT(sizeof(char)==sizeof(XML_Char));
	
	if (0 == UT_stricmp(szName, PT_PROPS_ATTRIBUTE_NAME))	// PROPS -- cut value up into properties
	{
		char *pOrig = strdup(szValue);
		if (!pOrig)
		{
			UT_DEBUGMSG(("setAttribute: strdup failed on [%s]\n",szValue));
			return UT_FALSE;
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
				return UT_FALSE;

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

			setProperty(p, q);
		}

		free(pOrig);
		return UT_TRUE;
	}
	else								// not "PRPOS" -- add to attribute list
	{
		if (!m_pAttributes)
		{
			m_pAttributes = new UT_HashTable();
			if (!m_pAttributes)
			{
				UT_DEBUGMSG(("setAttribute: could not allocate hash table.\n"));
				return UT_FALSE;
			}
		}

		return (m_pAttributes->addEntry(szName, szValue, NULL) == 0);
	}
}

UT_Bool	PP_AttrProp::setProperty(const XML_Char * szName, const XML_Char * szValue)
{
	if (!m_pProperties)
	{
		m_pProperties = new UT_HashTable();
		if (!m_pProperties)
		{
			UT_DEBUGMSG(("setProperty: could not allocate hash table.\n"));
			return UT_FALSE;
		}
	}

	UT_HashTable::UT_HashEntry* pEntry = m_pProperties->findEntry(szName);
	if (pEntry)
		return (m_pProperties->setEntry(pEntry, szValue, NULL) == 0);
	else
		return (m_pProperties->addEntry(szName, szValue, NULL) == 0);
}

UT_Bool	PP_AttrProp::getNthAttribute(int ndx, const XML_Char *& szName, const XML_Char *& szValue) const
{
	if (!m_pAttributes)
		return UT_FALSE;
	if (ndx >= m_pAttributes->getEntryCount())
		return UT_FALSE;
	UT_HashTable::UT_HashEntry * pEntry = m_pAttributes->getNthEntry(ndx);
	if (!pEntry)
		return UT_FALSE;
	szName = pEntry->pszLeft;
	szValue = pEntry->pszRight;
	return UT_TRUE;
}

UT_Bool	PP_AttrProp::getNthProperty(int ndx, const XML_Char *& szName, const XML_Char *& szValue) const
{
	if (!m_pProperties)
		return UT_FALSE;
	if (ndx >= m_pProperties->getEntryCount())
		return UT_FALSE;
	UT_HashTable::UT_HashEntry * pEntry = m_pProperties->getNthEntry(ndx);
	if (!pEntry)
		return UT_FALSE;
	szName = pEntry->pszLeft;
	szValue = pEntry->pszRight;
	return UT_TRUE;
}

UT_Bool PP_AttrProp::getProperty(const XML_Char * szName, const XML_Char *& szValue) const
{
	if (!m_pProperties)
		return UT_FALSE;
	
	UT_HashTable::UT_HashEntry* pEntry = m_pProperties->findEntry(szName);
	if (!pEntry)
		return UT_FALSE;

	szValue = pEntry->pszRight;

	return UT_TRUE;
}

UT_Bool PP_AttrProp::getAttribute(const XML_Char * szName, const XML_Char *& szValue) const
{
	if (!m_pAttributes)
		return UT_FALSE;
	
	UT_HashTable::UT_HashEntry* pEntry = m_pAttributes->findEntry(szName);
	if (!pEntry)
		return UT_FALSE;

	szValue = pEntry->pszRight;

	return UT_TRUE;
}

UT_Bool PP_AttrProp::areAlreadyPresent(const XML_Char ** attributes, const XML_Char ** properties) const
{
	// return TRUE if each attribute and property is already present
	// and has the same value as what we have.

	if (attributes && *attributes)
	{
		const XML_Char ** p = attributes;
		while (*p)
		{
			UT_ASSERT(p[1] && *p[1]);	// require value for each name
			const XML_Char * szValue = NULL;
			if (!getAttribute(p[0],szValue))
				return UT_FALSE;		// item not present
			if (UT_XML_stricmp(p[1],szValue)!=0)
				return UT_FALSE;		// item has different value
			p += 2;
		}
	}

	if (properties && *properties)
	{
		const XML_Char ** p = properties;
		while (*p)
		{
			UT_ASSERT(p[1] && *p[1]);	// require value for each name
			const XML_Char * szValue = NULL;
			if (!getProperty(p[0],szValue))
				return UT_FALSE;		// item not present
			if (UT_XML_stricmp(p[1],szValue)!=0)
				return UT_FALSE;		// item has different value
			p += 2;
		}
	}

	return UT_TRUE;						// everything matched
}

