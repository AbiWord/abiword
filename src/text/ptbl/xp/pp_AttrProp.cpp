
#include <memory.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_hash.h"
#include "ut_string.h"
#include "xmlparse.h"
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
	UT_ASSERT(sizeof(char)==sizeof(XML_Char)); // TODO when this fails, switch this file to use UT_XML_ version of str*() functions.
	
	if (0 == UT_stricmp(szName, "STYLE"))			// STYLE -- cut value up into properties
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
	else								// not "STYLE" -- add to attribute list
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


