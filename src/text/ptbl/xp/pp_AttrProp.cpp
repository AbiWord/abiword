/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (c) 2001,2002 Tomas Frydrych
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
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_vector.h"
#include "ut_pair.h"

#include "pt_Types.h"

#include "pp_AttrProp.h"

/****************************************************************/
PP_AttrProp::PP_AttrProp()
{
	m_pAttributes = NULL;
	m_pProperties = NULL;
	m_bIsReadOnly = false;
	m_checkSum = 0;
	xxx_UT_DEBUGMSG(("creating pp_AttrProp %x \n",this));
}

PP_AttrProp::~PP_AttrProp()
{
	xxx_UT_DEBUGMSG(("deleting pp_AttrProp %x \n",this));
	if (m_pAttributes)
	{
		UT_StringPtrMap::UT_Cursor c1(m_pAttributes);

		const XML_Char * s = static_cast<const XML_Char *>(c1.first());

		while (true) {
			FREEP(s);

			if (!c1.is_valid())
				break;

			s = static_cast<const XML_Char *>(c1.next());
		}

		delete m_pAttributes;
		m_pAttributes = NULL;
	}

	// delete any PP_Property_types;

	if(m_pProperties)
	{
		UT_StringPtrMap::UT_Cursor c(m_pProperties);
		const UT_Pair * entry = NULL;
		for (entry = static_cast<const UT_Pair*>(c.first()); c.is_valid();
		     entry = static_cast<const UT_Pair*>(c.next()))
		{
			if(entry)
			{
				// hack. don't do it.
				void* tmp = const_cast<void*> (entry->first());
				FREEP(tmp);
				if (entry->second())
					delete static_cast<PP_PropertyType *const>(entry->second());
				delete entry;
			}
		}

		delete m_pProperties;
		m_pProperties = NULL;
	}

}

/*!
 * Returns the number of properties in this PP_AttrProp.
 */
size_t PP_AttrProp::getPropertyCount (void) const
{
	if(!m_pProperties)
		return 0;
	else
		return m_pProperties->size();
}

/*!
 * Returns the number of attributes in this PP_AttrProp.
 */
size_t PP_AttrProp::getAttributeCount (void) const
{
	if(!m_pAttributes)
		return 0;
	else
		return m_pAttributes->size();
}


/*!
 * Sets attributes as given in the NULL-terminated input array
 * of (attribute, value) pairs.
 *
 * \param attributes An array of strings, read in (attribute, value) form.
 */
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

/*!
 * Sets attributes as given in the UT_Vector of strings, read as
 * (attribute, value) pairs.
 *
 * \param attributes A UT_Vector of strings, read in (attribute, value) form.
 */
bool PP_AttrProp::setAttributes(const UT_Vector * pVector)
{
	UT_uint32 kLimit = pVector->getItemCount();
	for (UT_uint32 k=0; k+1<kLimit; k+=2)
	{
		const XML_Char * pName = static_cast<XML_Char *>(pVector->getNthItem(k));
		const XML_Char * pValue = static_cast<XML_Char *>(pVector->getNthItem(k+1));
		if (!setAttribute(pName,pValue))
			return false;
	}
	return true;
}

/*!
 * Sets attributes as given in the NULL-terminated input array
 * of (attribute, value) pairs.
 */
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

/*!
 * Sets properties as given in the UT_Vector of strings, read as
 * (property, value) pairs.
 *
 * \param pVector A UT_Vector of strings, read in (properties, value) form.
 */
bool PP_AttrProp::setProperties(const UT_Vector * pVector)
{
	UT_uint32 kLimit = pVector->getItemCount();
	for (UT_uint32 k=0; k+1<kLimit; k+=2)
	{
		const XML_Char * pName = static_cast<XML_Char *>(pVector->getNthItem(k));
		const XML_Char * pValue = static_cast<XML_Char *>(pVector->getNthItem(k+1));
		if (!setProperty(pName,pValue))
			return false;
	}
	return true;
}


/*!
 * Sets given attribute in this PP_AttrProp bundle.
 * Deals correctly with setting the PT_PROPS_ATTRIBUTE_NAME property:
 * intercepts this call and appends properties instead.
 *
 * Because all mutations of attributes go through here, it is always the
 * case that the props attribute is correctly handled.
 */
bool	PP_AttrProp::setAttribute(const XML_Char * szName, const XML_Char * szValue)
{
	// TODO when this assert fails, switch this file to use UT_XML_ version of str*() functions.
	UT_ASSERT(sizeof(char)==sizeof(XML_Char));

	if (0 == UT_strcmp(szName, PT_PROPS_ATTRIBUTE_NAME) && *szValue)	// PROPS -- cut value up into properties
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

			setProperty(static_cast<const XML_Char*>(p), static_cast<const XML_Char*>(q));
		}

		free(pOrig);
		return true;
	}
	else								// not "PROPS" -- add to attribute list
	{
		if (!m_pAttributes)
		{
			m_pAttributes = new UT_StringPtrMap(5);
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
		char * szDupValue = UT_strdup(szValue);

		if(!m_pAttributes->insert(copy, static_cast<void *>(szDupValue)))
			FREEP(szDupValue);

		FREEP(copy);

		return true;
	}
}

bool	PP_AttrProp::setProperty(const XML_Char * szName, const XML_Char * szValue)
{
	if (!m_pProperties)
	{
		m_pProperties = new UT_StringPtrMap(5);
		if (!m_pProperties)
		{
			UT_DEBUGMSG(("setProperty: could not allocate hash table.\n"));
			return false;
		}
	}

#if 0
	// no. we have to set it empty, otherwise the code that changes
	// properties has no way of knowing that this property is not to
	// be present
	// 
	// if szValue == NULL or *szValue == 0, we want this property
	// removed
	bool bRemove = (!szValue || !*szValue);
#endif
	const void * pEntry = m_pProperties->pick(szName);
	if (pEntry)
	{
		const UT_Pair* p = static_cast<const UT_Pair*>(pEntry);

		// hack. don't do it.
		void* tmp = const_cast<void*> (p->first());
		UT_ASSERT(!m_bIsReadOnly);
		if(UT_strcmp(szName,"line-height") == 0)
		{
			UT_DEBUGMSG(("Found line-height, Old value %s new value is %s \n",tmp,szValue));
		}

		FREEP(tmp);
		if (p->second())
			delete static_cast<PP_PropertyType *const>(p->second());
		delete p;
#if 0
		if(bRemove)
		{
			m_pProperties->remove(szName,NULL);
		}
		else
#endif
		{
			m_pProperties->set(szName,
							   static_cast<void *>(new UT_Pair(UT_strdup(szValue), static_cast<void *>(NULL))));
		}
		
	}
	else
	{
#if 0
		if(!bRemove)
#endif
		{
			m_pProperties->insert(szName,
								  static_cast<void *>(new UT_Pair(UT_strdup(szValue), static_cast<void *>(NULL))));
		}
	}
	return true;
}

bool	PP_AttrProp::getNthAttribute(int ndx, const XML_Char *& szName, const XML_Char *& szValue) const
{
	if (!m_pAttributes)
		return false;
	if (static_cast<UT_uint32>(ndx) >= m_pAttributes->size())
		return false;

	int i = 0;
	UT_StringPtrMap::UT_Cursor c(m_pAttributes);
	const void * val = NULL;

	for (val = c.first(); (c.is_valid() && (i < ndx)); val = c.next(), i++)
	{
	  // noop
	}

	if ((i == ndx) && c.is_valid())
	  {
	    szName = static_cast<const XML_Char*>(c.key().c_str());
	    szValue = static_cast<const XML_Char*>(val);
	    return true;
	  }
	return false;
}

bool	PP_AttrProp::getNthProperty(int ndx, const XML_Char *& szName, const XML_Char *& szValue) const
{
	if (!m_pProperties)
		return false;

 	if (static_cast<UT_uint32>(ndx) >= m_pProperties->size())
  		return false;

 	int i = 0;
 	UT_StringPtrMap::UT_Cursor c(m_pProperties);
 	const void * val = NULL;

	for (val = c.first(); (c.is_valid() && (i < ndx)); val = c.next(), i++)
 	{
	  // noop
 	}

	if ( (i == ndx) && c.is_valid())
 		{
		  szName = static_cast<const XML_Char*>(c.key().c_str());
		  szValue = static_cast<XML_Char* const>(static_cast<const UT_Pair*>(val)->first());
		  return true;
 		}
	return false;
}

bool PP_AttrProp::getProperty(const XML_Char * szName, const XML_Char *& szValue) const
{
	if (!m_pProperties)
		return false;

	const void * pEntry = m_pProperties->pick(szName);
	if (!pEntry)
		return false;

	szValue = static_cast<XML_Char *const>(static_cast<const UT_Pair*>(pEntry)->first());

	return true;
}

const PP_PropertyType *PP_AttrProp::getPropertyType(const XML_Char * szName, tProperty_type Type) const
{
	if (!m_pProperties)
		return NULL;

	const UT_Pair * pEntry = static_cast<const UT_Pair *>(m_pProperties->pick(szName));
	if (!pEntry)
		return NULL;

	if(!pEntry->second())
	{
		m_pProperties->set(szName, new UT_Pair
				   (pEntry->first(),
				    PP_PropertyType::createPropertyType(Type,
									static_cast<XML_Char *const>(pEntry->first()))));
		delete pEntry;
		pEntry = static_cast<const UT_Pair *>(m_pProperties->pick(szName));
	}

	return static_cast<PP_PropertyType *const>(pEntry->second());
}
bool PP_AttrProp::getAttribute(const XML_Char * szName, const XML_Char *& szValue) const
{
	if (!m_pAttributes)
		return false;

	const void * pEntry = m_pAttributes->pick(szName);
	if (!pEntry)
		return false;

	szValue = static_cast<const XML_Char *>(pEntry);


	xxx_UT_DEBUGMSG(("SEVIOR: getAttribute Found value %s \n",szValue));

	return true;
}

bool PP_AttrProp::hasProperties(void) const
{
	if (!m_pProperties)
		return false;

	return (m_pProperties->size() > 0);
}

bool PP_AttrProp::hasAttributes(void) const
{
	if (!m_pAttributes)
		return false;

	return (m_pAttributes->size() > 0);
}

bool PP_AttrProp::areAlreadyPresent(const XML_Char ** attributes, const XML_Char ** properties) const
{
	// return TRUE if each attribute and property is already present
	// and has the same value as what we have.

	if (attributes && *attributes)
	{
		const XML_Char ** p = attributes;
		while (*p)
		{
			/*
			    It seems that we also want empty strings for attributes,
				at least for the 'param' attribute which goes with fields.
				Is this bogus too? -PL
			*/
			UT_ASSERT(p[1] /* && *p[1]*/);	// require value for each name
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

	UT_uint32 countMyAttrs = ((m_pAttributes) ? m_pAttributes->size() : 0);
	UT_uint32 countMatchAttrs = ((pMatch->m_pAttributes) ? pMatch->m_pAttributes->size() : 0);
	if (countMyAttrs != countMatchAttrs)
		return false;

	UT_uint32 countMyProps = ((m_pProperties) ? m_pProperties->size() : 0);
	UT_uint32 countMatchProps = ((pMatch->m_pProperties) ? pMatch->m_pProperties->size() : 0);
	if (countMyProps != countMatchProps)
		return false;

	if (countMyAttrs != 0)
	{
		UT_StringPtrMap::UT_Cursor ca1(m_pAttributes);
		UT_StringPtrMap::UT_Cursor ca2(pMatch->m_pAttributes);

		const void * v1 = ca1.first();
		const void * v2 = ca2.first();

		do
		{
			const XML_Char *l1 = static_cast<const XML_Char *>(ca1.key().c_str());
			const XML_Char *l2 = static_cast<const XML_Char *>(ca2.key().c_str());

			if (UT_XML_stricmp(l1, l2) != 0)
				return false;

			l1 = static_cast<const XML_Char *>(v1);
			l2 = static_cast<const XML_Char *>(v2);

			if (UT_XML_stricmp(l1,l2) != 0)
				return false;

			v1 = ca1.next();
			v2 = ca2.next();
		} while (ca1.is_valid());
	}

	if (countMyProps > 0)
	{
		UT_StringPtrMap::UT_Cursor cp1(m_pProperties);
		UT_StringPtrMap::UT_Cursor cp2(pMatch->m_pProperties);

		const void * v1 = cp1.first();
		const void * v2 = cp2.first();

		do
		{
			const XML_Char *l1 = static_cast<const XML_Char *>(cp1.key().c_str());
			const XML_Char *l2 = static_cast<const XML_Char *>(cp2.key().c_str());

			if (UT_XML_stricmp(l1, l2) != 0)
				return false;

			l1 = (XML_Char *) (static_cast<const UT_Pair*>(v1))->first();;
			l2 = (XML_Char *) (static_cast<const UT_Pair*>(v2))->first();;

			if (UT_XML_stricmp(l1,l2) != 0)
				return false;

			v1 = cp1.next();
			v2 = cp2.next();
		} while (cp1.is_valid());

	#ifdef PT_TEST
		s_Matches++;
	#endif
	}

	return true;
}

PP_AttrProp * PP_AttrProp::cloneWithReplacements(const XML_Char ** attributes,
												 const XML_Char ** properties,
												 bool bClearProps) const
{
	bool bIgnoreProps = false; // see below
	
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

	// we want to be able to remove all properties by setting the
	// props attribute to ""; in order for that to work, we need to
	// skip the following loop if props is set to ""
	const XML_Char * szValue;

	if(papNew->getAttribute("props", szValue) && !*szValue)
		bIgnoreProps = true;

	if (!bClearProps && !bIgnoreProps)
	{
		k = 0;
		while (getNthProperty(k++,n,v))
		{
			if (!papNew->getProperty(n,vNew))
				if (!papNew->setProperty(n,v))
					goto Failed;
		}
	}

	// the following will remove all properties set to ""; this allows us
	// to remove properties by setting them to ""
	papNew->_clearEmptyProperties();
	papNew->_clearEmptyAttributes();

	return papNew;

Failed:
	DELETEP(papNew);
	return NULL;
}

/*
  This function will remove all properties that are set to ""
*/
void PP_AttrProp::_clearEmptyProperties()
{
	if(!m_pProperties)
		return;

	UT_StringPtrMap::UT_Cursor _hc1(m_pProperties);
	const void * pEntry;

	for ( pEntry  = static_cast<const UT_Pair*>(_hc1.first()); _hc1.is_valid(); pEntry = static_cast<const UT_Pair*>(_hc1.next()) )
	{
		if (pEntry)
		{
			const UT_Pair* p = static_cast<const UT_Pair*>(pEntry);

			if(*(static_cast<XML_Char *const>(p->first())) == 0)
			{

				void* tmp = const_cast<void*> (p->first());
				UT_ASSERT(!m_bIsReadOnly);
				FREEP(tmp);
				if (p->second())
					delete static_cast<PP_PropertyType *const>(p->second());
				delete p;

				m_pProperties->remove(_hc1.key(),pEntry);
			}
		}
	}
}

void PP_AttrProp::_clearEmptyAttributes()
{
	if(!m_pAttributes)
		return;

	UT_StringPtrMap::UT_Cursor _hc1(m_pAttributes);
	const XML_Char * pEntry;

	for ( pEntry  = static_cast<const XML_Char*>(_hc1.first()); _hc1.is_valid(); pEntry = static_cast<const XML_Char*>(_hc1.next()) )
	{
		if (pEntry && !*pEntry)
		{
			UT_ASSERT(!m_bIsReadOnly);
			FREEP(pEntry);
			m_pAttributes->remove(_hc1.key(),pEntry);
		}
	}
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
				if (UT_XML_stricmp(n,p[0])==0)		// found it, so we don't put it in the result.
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
				if (UT_XML_stricmp(n,p[0])==0)		// found it, so we don't put it in the result.
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

PP_AttrProp * PP_AttrProp::cloneWithEliminationIfEqual(const XML_Char ** attributes,
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
				if (UT_XML_stricmp(n,p[0])==0 && UT_XML_stricmp(n,p[1])==0)		// found it, so we don't put it in the result.
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
				if (UT_XML_stricmp(n,p[0])==0 && UT_XML_stricmp(n,p[1])==0)		// found it, so we don't put it in the result.
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

void PP_AttrProp::_computeCheckSum(void)
{
	m_checkSum = 0;

	// compute some easy-to-compute checksum for this AP. something
	// that is guaranteed to return the same result for the same input.
	// something that we can use like a hash for a quick check in
	// isExactMatch() to quickly dismiss different APs.  it should be
	// based strictly on the content of the name/value pairs and be
	// pointer- and order-independent.

	if (!m_pAttributes || !m_pProperties)
		return;

	const XML_Char * s1, *s2;

	UT_StringPtrMap::UT_Cursor c1(m_pAttributes);
	UT_StringPtrMap::UT_Cursor c2(m_pProperties);

	const void *val = c1.first();

	while (val != NULL)
	{
		s1 = static_cast<const XML_Char *>(c1.key().c_str());
		s2 = static_cast<const XML_Char *>(val);

		m_checkSum += UT_XML_strlen(s1);
		m_checkSum += UT_XML_strlen(s2);

		if (!c1.is_valid())
			break;
		val = c1.next();
	}


	val = c2.first();
	while (val != NULL)
	{
		s1 = static_cast<const XML_Char *>(c2.key().c_str());
		s2 = static_cast<XML_Char *const>(static_cast<const UT_Pair*>(val)->first());

		m_checkSum += UT_XML_strlen(s1);
		m_checkSum += UT_XML_strlen(s2);

		if (!c2.is_valid())
			break;
		val = c2.next();
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
	UT_uint32 countMyAttrs = ((Other.m_pAttributes) ? Other.m_pAttributes->size() : 0);

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

	UT_uint32 countMyProps = ((Other.m_pProperties) ? Other.m_pProperties->size() : 0);

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

