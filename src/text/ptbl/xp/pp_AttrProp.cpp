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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
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

#include "pt_Types.h"

#include "pp_AttrProp.h"
#include "pp_Revision.h"
#include "pd_Style.h"
#include "pd_Document.h"


/****************************************************************/

/// This is the sole explicit constructor of class PP_AttrProp.
PP_AttrProp::PP_AttrProp()
{
	m_pAttributes = NULL;
	m_pProperties = NULL;
	m_szProperties = NULL;
	m_bIsReadOnly = false;
	m_checkSum = 0;
	xxx_UT_DEBUGMSG(("creating pp_AttrProp %x \n",this));

	m_iRevisedIndex = 0xffffffff;
	m_bRevisionHidden = false;
}

/// This is the sole explicit destructor of class PP_AttrProp.
PP_AttrProp::~PP_AttrProp()
{
	xxx_UT_DEBUGMSG(("deleting pp_AttrProp %x \n",this));
	if (m_pAttributes)
	{
		UT_GenericStringMap<gchar*>::UT_Cursor c1(m_pAttributes);

		const gchar * s = c1.first();

		while (true) {
			FREEP(s);

			if (!c1.is_valid())
				break;

			s = c1.next();
		}

		delete m_pAttributes;
		m_pAttributes = NULL;
	}

	// delete any PP_Property_types;

	if(m_pProperties)
	{
		UT_GenericStringMap<PropertyPair*>::UT_Cursor c(m_pProperties);
		const PropertyPair * entry = NULL;

		for (entry = c.first(); c.is_valid(); entry = c.next())
		{
			if(entry)
			{
				// hack. don't do it.
				gchar* tmp = (gchar*)entry->first;
				FREEP(tmp);
				if (entry->second) 
				{
					delete entry->second;
				}
				delete entry;
			}
		}

		delete m_pProperties;
		m_pProperties = NULL;
	}
	if(m_szProperties)
	{
		delete [] m_szProperties;
	}
	m_szProperties = NULL;
}

/*!
 * Returns the number of properties in this PP_AttrProp.
 *
 * BE WEARY, BE VERY WEARY, the count can be greater than the number of valid items
 * stored in the hash -- always check the return value of getNthProperty()
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
 *
 * BE WEARY, BE VERY WEARY, the count can be greater than the number of valid items
 * stored in the hash -- always check the return value of getNthAttribute()
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
bool	PP_AttrProp::setAttributes(const gchar ** attributes)
{
	if (!attributes)
		return true;

	const gchar ** pp = attributes;
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
bool PP_AttrProp::setAttributes(const UT_GenericVector<const gchar*> * pVector)
{
	UT_uint32 kLimit = pVector->getItemCount();
	for (UT_uint32 k=0; k+1<kLimit; k+=2)
	{
		const gchar * pName = pVector->getNthItem(k);
		const gchar * pValue = pVector->getNthItem(k+1);
		if (!setAttribute(pName,pValue))
			return false;
	}
	return true;
}

/*!
 * Sets attributes as given in the NULL-terminated input array
 * of (attribute, value) pairs.
 */
bool	PP_AttrProp::setProperties(const gchar ** properties)
{
	if (!properties)
		return true;

	const gchar ** pp = properties;
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
bool PP_AttrProp::setProperties(const UT_GenericVector<gchar*> * pVector)
{
	UT_uint32 kLimit = pVector->getItemCount();
	for (UT_uint32 k=0; k+1<kLimit; k+=2)
	{
		const gchar * pName = pVector->getNthItem(k);
		const gchar * pValue = pVector->getNthItem(k+1);
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
bool	PP_AttrProp::setAttribute(const gchar * szName, const gchar * szValue)
{
	// TODO when this assert fails, switch this file to use UT_XML_ version of str*() functions.
	UT_return_val_if_fail (sizeof(char)==sizeof(gchar), false);

	if (0 == strcmp(szName, PT_PROPS_ATTRIBUTE_NAME) && *szValue)	// PROPS -- cut value up into properties
	{
		char * pOrig = NULL;
		
		if (!(pOrig = g_strdup(szValue)))
		{
			UT_DEBUGMSG(("setAttribute: g_strdup() failed on [%s]\n",szValue));
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
				g_free(pOrig);
				UT_DEBUGMSG(("props: %s\n", szValue));
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
			while ((*q > 0) && isspace(*q))
				q++;

			setProperty(p, q);
		}

		g_free(pOrig);
		return true;
	}
	else if (0 == strcmp(szName, PT_XID_ATTRIBUTE_NAME) && *szValue)
	{
		// XID is a unique id for the xml element / PT frag. Its function is to facilitate
		// comparing/merging documents and we do not want it in the AP
		return true;
	}
	else // not "PROPS" -- add to attribute list
	{
		UT_UTF8String url;
		if (szValue && *szValue && (0 == strcmp(szName, "xlink:href") || 0 == strcmp(szName, "href")))
		{
			url = szValue;
			url.decodeURL();
			szValue = url.utf8_str();
		}
		
		if (!m_pAttributes)
		{
			m_pAttributes = new UT_GenericStringMap<gchar*>(5);
			if (!m_pAttributes)
			{
				UT_DEBUGMSG(("setAttribute: could not allocate hash table.\n"));
				return false;
			}
		}

		// make sure we store attribute names in lowercase
		UT_ASSERT_HARMLESS(sizeof(char) == sizeof(gchar));

		char * copy = g_ascii_strdown(szName, -1);
		char * szDupValue = szValue ? g_strdup(szValue) : NULL;

		// get rid of any illegal chars we might have been given
		if(!UT_isValidXML(copy))
			UT_validXML(copy);
		if(!UT_isValidXML(szDupValue))
			UT_validXML(szDupValue);

		const char * pEntry = m_pAttributes->pick(copy);

		if(pEntry)
		{
			// attribute exists, replace it
			FREEP(pEntry);
			m_pAttributes->set(copy, szDupValue);
		}
		else
		{
			bool bRet = m_pAttributes->insert(copy, szDupValue);
			UT_ASSERT_HARMLESS( bRet );
			if(!bRet)
			{
				FREEP(szDupValue);
			}
		}
		
		FREEP(copy);

		return true;
	}
}

/*! This method inserts a new pair of property name and value into [this] APs set of
	 properties, creating "props" if it does not already exist and overwriting the value
	 of any property of the same name with the newly passed value.
	 (?)It appears as though we replace the entire pair, rather than only the value.
		 (?)Is there a reason for this?
	 \return Whether or not the operation succeeded.
*/
bool	PP_AttrProp::setProperty(const gchar * szName, const gchar * szValue)
{
	UT_return_val_if_fail( szName, false );
	
	if (!m_pProperties)
	{
		m_pProperties = new UT_GenericStringMap<PropertyPair*>(5);
		if (!m_pProperties)
		{
			UT_DEBUGMSG(("setProperty: could not allocate hash table.\n"));
			return false;
		}
	}

	// if szValue == NULL or *szValue == 0, indicates absent property.
	// We have to set it empty, otherwise the code that changes
	// properties has no way of knowing that this property is not to
	// be present
	// 
	//bool bRemove = (!szValue || !*szValue);

	// get rid of any chars invalid in xml
	char * szName2 = NULL;
	if(!UT_isValidXML(szName))
	{
		szName2 = g_strdup(szName);

		// get rid of any illegal chars we were passed
		UT_validXML(szName2);

		szName = szName2;
	}
	
	char * szValue2 = szValue ? g_strdup(szValue) : NULL;
	UT_return_val_if_fail( szName && (szValue2 || !szValue), false);

	// get rid of any illegal chars we might have been given in the value
	if(!UT_isValidXML(szValue2))
		UT_validXML(szValue2);
	
	const PropertyPair * pEntry = m_pProperties->pick(szName);
	if (pEntry)
	{
		const PropertyPair* p = pEntry;

		// hack. don't do it.
		gchar* tmp = (gchar*)p->first;
		UT_return_val_if_fail (!m_bIsReadOnly, false);
		if(strcmp(szName,"line-height") == 0)
		{
			UT_DEBUGMSG(("Found line-height, Old value %s new value is %s \n",tmp,szValue));
		}

		FREEP(tmp);
		if (p->second) 
		{
			delete p->second;
		}
		delete p;
		m_pProperties->set(szName, 
				   new PropertyPair(szValue2, 
						    (const PP_PropertyType*)NULL));
	}
	else
	{
		m_pProperties->insert(szName, 
				      new PropertyPair(szValue2, 
						       (const PP_PropertyType*)NULL));
	}

	// g_free the name duplicate if necessary
	FREEP(szName2);
	
	return true;
}

/*! This method finds the Nth attribute where N is
	 \param ndx The number in order of the attribute to be found
	 and assigns that attribute's name and value to
	 \param szName The name of the attribute found
	 \param szValue The value of the attribute found
	 respectively.  The method returns
	 \return whether or not the operation succeeded.

    WARNING: Always check the return value before trying to work with szValue!
*/
bool	PP_AttrProp::getNthAttribute(int ndx, const gchar *& szName, const gchar *& szValue) const
{
	if (!m_pAttributes)
		return false;
	if (static_cast<UT_uint32>(ndx) >= m_pAttributes->size())
	{
		// UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN ); -- do not assert, some code in
		// while loops relies on this
		return false;
	}
	
	int i = 0;
	UT_GenericStringMap<gchar*>::UT_Cursor c(m_pAttributes);
	const gchar * val = NULL;

	for (val = c.first(); (c.is_valid() && (i < ndx)); val = c.next(), i++)
	{
	  // noop
	}

	if ((i == ndx) && c.is_valid())
	  {
	    szName = c.key().c_str();
	    szValue = val;
	    return true;
	  }
	return false;
}

/*! This method finds the Nth property where N is
	 \param ndx The number in order of the property to be found
	 and assigns that property's name and value to
	 \param szName The name of the property found
	 \param szValue The value of the property found
	 respectively.  The method returns
	 \return whether or not the operation succeeded.

    WARNING: Always check the return value before trying to work with szValue!
*/
bool	PP_AttrProp::getNthProperty(int ndx, const gchar *& szName, const gchar *& szValue) const
{
	if (!m_pProperties)
		return false;

 	if (static_cast<UT_uint32>(ndx) >= m_pProperties->size())
	{
		// UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN ); -- do not assert, some code in
		// while loops relies on this
  		return false;
	}
	
 	int i = 0;
	UT_GenericStringMap<PropertyPair*>::UT_Cursor c(m_pProperties);
 	const PropertyPair * val = NULL;

	for (val = c.first(); (c.is_valid() && (i < ndx)); val = c.next(), i++)
 	{
	  // noop
 	}

	if ( (i == ndx) && c.is_valid())
 		{
		  szName = c.key().c_str();
		  szValue = val->first;
		  return true;
 		}
	return false;
}

/*! This method finds the property indicated by name
	 \param szName (the name of the property the value of which to find)
	 and assigns its value to
	 \param szValue (the value found of the property requested)
	 or returns false if the properties as a whole or the property requested are not found.
	 It returns
	 \return whether or not the operation succeeded.

	 WARNING: Be sure to check the return value before trying to work with szValue.
*/
bool PP_AttrProp::getProperty(const gchar * szName, const gchar *& szValue) const
{
	if (!m_pProperties)
		return false;
	const PropertyPair * pEntry = m_pProperties->pick(szName);

	if (!pEntry)
		return false;

	szValue = pEntry->first;
	return true;
}
/*! This method retrieves the entirety of [this] AP's "props", and returns it as an array of
	 gchar * pairs (name and value).

    WARNING: Do not g_free this memory. It's cached here.
*/
const gchar ** PP_AttrProp::getProperties () const
{
	if(!m_pProperties)
		return NULL;
	if(m_szProperties != NULL)
	{
		return m_szProperties;
	}
	UT_uint32 iPropsCount = m_pProperties->size();
	m_szProperties = new const gchar * [iPropsCount*2+2];

	const gchar ** pList = m_pProperties->list();
	UT_uint32 i = 0;

	
	// where the values should be, we actually have pointers to PropertyPair;
	for(i = 1; i < iPropsCount * 2; i += 2)
	{
		PropertyPair * pP = (PropertyPair *) pList[i];
		m_szProperties[i-1] = pList[i-1];
		m_szProperties[i] = pP->first;
	}
	m_szProperties[i-1] = NULL;
	m_szProperties[i] = NULL;
	return m_szProperties;
}

/*! (?)TODO: PLEASE DOCUMENT ME!
*/
const PP_PropertyType *PP_AttrProp::getPropertyType(const gchar * szName, tProperty_type Type) const
{
	if (!m_pProperties)
		return NULL;

	const PropertyPair * pEntry = m_pProperties->pick(szName);
	if (!pEntry)
		return NULL;

	if(!pEntry->second)
	{
		m_pProperties->set(szName, new PropertyPair(pEntry->first,
				    PP_PropertyType::createPropertyType(Type,pEntry->first)));
		delete pEntry;
		pEntry = m_pProperties->pick(szName);
	}

	return pEntry->second;
}

/*! This method finds the attribute indicated by name
	 \param szName (the name of the attribute the value of which to find)
	 and assigns its value to
	 \param szValue (the value found of the attribute requested)
	 or returns false if the attributes as a whole or the attribute requested are not found.
	 It returns
	 \return whether or not the operation succeeded.

	 WARNING: Be sure to check the return value before trying to work with szValue.
*/
bool PP_AttrProp::getAttribute(const gchar * szName, const gchar *& szValue) const
{
	if (!m_pAttributes)
		return false;

	const gchar * pEntry = m_pAttributes->pick(szName);
	if (!pEntry)
		return false;

	szValue = pEntry;


	xxx_UT_DEBUGMSG(("SEVIOR: getAttribute Found value %s \n",szValue));

	return true;
}

/// Returns whether or not the AP has any properties.
/*! This method checks [this] AP for the "props" attribute.
	 The "props" attribute is a special attribute that contains
	 properties.  If the "props" attribute is absent, the
	 method returns false.  If the "props" attribute is present
	 but empty (m_pProperties->size() == 0), it returns false.
	 If the "props" attribute is present and has something in it,
	 (m_pProperties->size() > 0), it returns true, but beware that
	 no sanity checking is done to make sure that whatever is in
	 there is anything more legible than uninitialized memory.
*/
bool PP_AttrProp::hasProperties(void) const
{
	if (!m_pProperties)
		return false;

	return (m_pProperties->size() > 0);
}

/// Returns whether or not the AP has any attributes.
/*! This method checks [this] AP for the presence of any attribute.
	 If there is any attribute at all in the AP, m_pAttributes->size()
	 returns positive and so does this method.  Otherwise, this method
	 returns false.
	 Beware that no sanity checking is done to make sure that whatever
	 is in there (being counted by size()) is anything more legible
	 than uninitialized memory.
*/
bool PP_AttrProp::hasAttributes(void) const
{
	if (!m_pAttributes)
		return false;

	return (m_pAttributes->size() > 0);
}

/// Returns whether or not the given attributes and properties are identically present in [this] AP.
/*! This method compares the given attributes and properties with those already present.
	 It compares the given items as inseparable pairs - if the attribute or property is
	 present in name but contains a different value, that does not count and false is
	 returned.
	 \return A bool indicating (directly) both presence and equality.
*/
bool PP_AttrProp::areAlreadyPresent(const gchar ** attributes, const gchar ** properties) const
{
	if (attributes && *attributes)
	{
		const gchar ** p = attributes;
		while (*p)
		{
			/*
			    It seems that we also want empty strings for attributes,
				at least for the 'param' attribute which goes with fields.
				Is this bogus too? -PL

				Yes. 
				We use empty strings and NULL in values to indicate that the
				attribute/property should be absent. Sometimes these values filter down
				here, so we need to handle this. TF
			*/
			// UT_return_val_if_fail (p[1] /* && *p[1]*/, false);	// require value for each name

			// first deal with the case where the value is set to NULL or "" -- we want
			// that attribute to be absent, not present
			const gchar * szValue = NULL;

			if((!p[1] || !*p[1]) && getAttribute(p[0],szValue) && szValue && *szValue)
				return false;
			// the 'props' attribute has to be handled separatedly, since it is not
			// returned using getAttribute() (it is not stored as attribute)
			else if((!p[1] || !*p[1]) && !strcmp(p[0],"props") && hasProperties())
				return false;
			else if(p[1] && *p[1])
			{
				if (!getAttribute(p[0],szValue))
					return false;		// item not present
				if (strcmp(p[1],szValue)!=0)
					return false;		// item has different value
			}
					
			p += 2;
		}
	}

	if (properties && *properties)
	{
		const gchar ** p = properties;
		while (*p)
		{
			/*
				Jeff, I weakened the following assert because we
				*want* to represent no tabstops as an empty string.
				If this isn't safe, let me know.   -- PCR

				We use empty strings and NULL in values to indicate that the
				attribute/property should be absent. Sometimes these values filter down
				here, so we need to handle this. TF
			*/
			// UT_return_val_if_fail (p[1] /* && *p[1]*/, false);	// require value for each name

			// first deal with the case where the value is set to NULL or "" -- we want
			// that attribute to be absent, not present
			const gchar * szValue = NULL;

			if((!p[1] || !*p[1]) && getProperty(p[0],szValue) && szValue && *szValue)
				return false;
			else if(p[1] && p[1])
			{
				if (!getProperty(p[0],szValue))
					return false;		// item not present
				if (strcmp(p[1],szValue)!=0)
					return false;		// item has different value
			}
			
			p += 2;
		}
	}

	return true;						// everything matched
}

/*! Find out if any attribute- or property-name is present.
	 This is like areAlreadyPresent(), but we don't care about
	 the values, and it returns true after the first discovery
	 regardless of whether or not any other of the given names are present.
	 \return A bool that's TRUE if any of the given attr. or prop. names is present, false otherwise.
*/
bool PP_AttrProp::areAnyOfTheseNamesPresent(const gchar ** attributes, const gchar ** properties) const
{
	// TODO consider using the fact that we are now (Dec 12 1998) using
	// TODO alpha-hash-table rather than just a hash-table to optimize
	// TODO these loops somewhat.

	if (attributes && *attributes)
	{
		const gchar ** p = attributes;
		while (*p)
		{
			const gchar * szValue = NULL;
			if (getAttribute(p[0],szValue))
				return true;
			p += 2;						// skip over value
		}
	}

	if (properties && *properties)
	{
		const gchar ** p = properties;
		while (*p)
		{
			const gchar * szValue = NULL;
			if (getProperty(p[0],szValue))
				return true;
			p += 2;						// skip over value
		}
	}

	return false;					// didn't find any
}

/*! Checks to see if the given AP is identical to itself ([this]).  It also contains
	 some useful points of instrumentation for benchmarking table and usage characteristics.
	 \return TRUE, if and only if we match the AP given, false otherwise.
*/
bool PP_AttrProp::isExactMatch(const PP_AttrProp * pMatch) const
{
	// The counters below are used in testing to profile call and chksum characteristics,
	// including collision rates.
	// NB: I'm not sure this initialization block is in the correct place.
#ifdef PT_TEST
	static UT_uint32 s_Calls = 0;
	static UT_uint32 s_PassedCheckSum = 0;
	static UT_uint32 s_Matches = 0;
#endif
#ifdef PT_TEST
	s_Calls++;
#endif

	UT_return_val_if_fail (pMatch, false);
	//
	// Why is this here? Nothing is being changed?
	//	UT_return_val_if_fail (m_bIsReadOnly && pMatch->m_bIsReadOnly, false);
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
		UT_GenericStringMap<gchar*>::UT_Cursor ca1(m_pAttributes);
		UT_GenericStringMap<gchar*>::UT_Cursor ca2(pMatch->m_pAttributes);

		const gchar * v1 = ca1.first();
		const gchar * v2 = ca2.first();

		do
		{
			const gchar *l1 = ca1.key().c_str();
			const gchar *l2 = ca2.key().c_str();

			if (strcmp(l1, l2) != 0)
				return false;

			l1 = v1;
			l2 = v2;

			if (strcmp(l1,l2) != 0)
				return false;

			v1 = ca1.next();
			v2 = ca2.next();
		} while (ca1.is_valid() && ca2.is_valid());
	}

	if (countMyProps > 0)
	{
		UT_GenericStringMap<PropertyPair*>::UT_Cursor cp1(m_pProperties);
		UT_GenericStringMap<PropertyPair*>::UT_Cursor cp2(pMatch->m_pProperties);

		const PropertyPair* v1 = cp1.first();
		const PropertyPair* v2 = cp2.first();

		do
		{
			const gchar *l1 = cp1.key().c_str();
			const gchar *l2 = cp2.key().c_str();

			if (strcmp(l1, l2) != 0)
				return false;

			l1 = v1->first;
			l2 = v2->first;

			if (strcmp(l1,l2) != 0)
				return false;

			v1 = cp1.next();
			v2 = cp2.next();
		} while (cp1.is_valid() && cp2.is_valid());

	#ifdef PT_TEST
		s_Matches++;
	#endif
	}

	return true;
}


/*! Create a new AttrProp with exactly the attributes/properties given.
  \return NULL on failure, the newly-created PP_AttrProp.
*/
PP_AttrProp * PP_AttrProp::createExactly(const gchar ** attributes,
					 const gchar ** properties) const
{
	// first, create a new AttrProp using just the values given.

	PP_AttrProp * papNew = new PP_AttrProp();
	if (!papNew)
		goto Failed;
	if (!papNew->setAttributes(attributes) || !papNew->setProperties(properties))
		goto Failed;
	return papNew;

Failed:
	DELETEP(papNew);
	return NULL;
}

/*! Create a new AttrProp based upon the given one, adding or replacing the items given.
	 \return NULL on failure, the newly-created PP_AttrProp clone otherwise.
*/
PP_AttrProp * PP_AttrProp::cloneWithReplacements(const gchar ** attributes,
												 const gchar ** properties,
												 bool bClearProps) const
{
	bool bIgnoreProps = false; // see below

	// first, create a new AttrProp using just the values given.

	PP_AttrProp * papNew = new PP_AttrProp();
	if (!papNew)
		goto Failed;
	if (!papNew->setAttributes(attributes) || !papNew->setProperties(properties))
		goto Failed;

	// next, add any items that we have that are not present
	// (have not been overridden) in the new one.

	UT_uint32 k;
	const gchar * n;
	const gchar * v;
	const gchar * vNew;

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
		// TODO for now, we just ignore it.
	    if (strcmp(n,PT_PROPS_ATTRIBUTE_NAME) == 0)
	    {
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		continue;
	    }
	    if (!papNew->getAttribute(n,vNew))
	    {
		bool bres = papNew->setAttribute(n,v);
		if (!bres)
		    goto Failed;
	    }
	}

	// we want to be able to remove all properties by setting the
	// props attribute to ""; in order for that to work, we need to
	// skip the following loop if props is set to ""
	const gchar * szValue;

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

/// This function will remove all properties that are set to ""
void PP_AttrProp::_clearEmptyProperties()
{
	if(!m_pProperties)
		return;

	UT_GenericStringMap<PropertyPair*>::UT_Cursor _hc1(m_pProperties);
	PropertyPair * pEntry;

	for ( pEntry  = _hc1.first(); _hc1.is_valid(); pEntry = _hc1.next())
	{
		if (pEntry)
		{
			const PropertyPair* p = pEntry;

			const char *s = p->first;
			if(s == NULL || *s == 0)
			{

				gchar* tmp = const_cast<gchar*>(p->first);
				UT_return_if_fail (!m_bIsReadOnly);
				FREEP(tmp);
				m_pProperties->remove(_hc1.key(),pEntry);
				if (p->second) 
				{
					delete p->second;
				}
				delete p;

			}
		}
	}
}

/// This method clears both empty attributes and empty properties from [this] AP.
void PP_AttrProp::prune()
{
	_clearEmptyAttributes();
	_clearEmptyProperties();
}

/// This function will remove all attributes that are equal to "" (*<gchar *> == NULL)
void PP_AttrProp::_clearEmptyAttributes()
{
	if(!m_pAttributes)
		return;

	UT_GenericStringMap<gchar*>::UT_Cursor _hc1(m_pAttributes);
	gchar * pEntry;

	for ( pEntry  = _hc1.first(); _hc1.is_valid(); pEntry = _hc1.next())
	{
		if (pEntry && !*pEntry)
		{
			UT_return_if_fail (!m_bIsReadOnly);
			m_pAttributes->remove(_hc1.key(),pEntry);
			FREEP(pEntry);
		}
	}
}

/*! Create a new AttrProp based upon the given one, removing the items given
	 regardless of their value.  See also PP_AttrProp::cloneWithEliminationIfEqual,
	 which does similarly but only removes the items given if their value is equal
	 to the value given.
	 \return NULL on failure, the newly-created PP_AttrProp clone otherwise.
*/
PP_AttrProp * PP_AttrProp::cloneWithElimination(const gchar ** attributes,
												const gchar ** properties) const
{

	// first, create an empty AttrProp.
	PP_AttrProp * papNew = new PP_AttrProp();
	if (!papNew)
		goto Failed;

	UT_uint32 k;
	const gchar * n;
	const gchar * v;

	k = 0;
	while (getNthAttribute(k++,n,v))
	{
		// for each attribute in the old set, add it to the
		// new set only if it is not present in the given array.

		if (attributes && *attributes)
		{
			const gchar ** p = attributes;
			while (*p)
			{
				UT_return_val_if_fail (strcmp(p[0],PT_PROPS_ATTRIBUTE_NAME)!=0, NULL); // cannot handle PROPS here
				if (strcmp(n,p[0])==0)		// found it, so we don't put it in the result.
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
			const gchar ** p = properties;
			while (*p)
			{
				if (strcmp(n,p[0])==0)		// found it, so we don't put it in the result.
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

/*! After the construction of a new AP, use this method to mark it read-only.
	 It returns if the AP is already read-only; otherwise it marks [this] AP
	 as read-only and then computes its checksum in order to speed subsequent
	 equivalence testing.  There is no return value.
*/
void PP_AttrProp::markReadOnly(void)
{
	UT_return_if_fail (!m_bIsReadOnly);
	m_bIsReadOnly = true;
	_computeCheckSum();
}

/*! Create a new AttrProp based upon the given one, removing the items given
	 if their value is equal to that given.  See also PP_AttrProp::cloneWithElimination,
	 which does similarly but removes the items given regardless of whether or not
	 their value is equal to the value given.
	 \return NULL on failure, the newly-created PP_AttrProp clone otherwise.
*/
PP_AttrProp * PP_AttrProp::cloneWithEliminationIfEqual(const gchar ** attributes,
												const gchar ** properties) const
{
	// first, create an empty AttrProp.
	PP_AttrProp * papNew = new PP_AttrProp();
	if (!papNew)
		goto Failed;

	UT_uint32 k;
	const gchar * n;
	const gchar * v;

	k = 0;
	while (getNthAttribute(k++,n,v))
	{
		// for each attribute in the old set, add it to the
		// new set only if it is not present in the given array.

		if (attributes && *attributes)
		{
			const gchar ** p = attributes;
			while (*p)
			{
				if(strcmp(p[0],PT_PROPS_ATTRIBUTE_NAME)!=0)
					goto DoNotIncludeAttribute; // cannot handle PROPS here
				if (strcmp(n,p[0])==0 && strcmp(n,p[1])==0)		// found it, so we don't put it in the result.
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
			const gchar ** p = properties;
			while (*p)
			{
				if (strcmp(n,p[0])==0 && strcmp(n,p[1])==0)		// found it, so we don't put it in the result.
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

/*! (?)TODO: PLEASE DOCUMENT ME!
*/
static UT_uint32 hashcodeBytesAP(UT_uint32 init, const void * pv, UT_uint32 cb)
{
 	// modified from ut_string_class.cpp's hashcode() which got it from glib
 	UT_uint32 h = init;
 	const unsigned char * pb = static_cast<const unsigned char *>(pv);
 
 	if (cb)
 	{
 		// for AP data, limit hash to consume at most 8 bytes
 		if (cb > 8) { cb = 8; }
 
 		for (; cb != 0; pb += 1, cb -= 1)
 		{
 			h = (h << 5) - h + *pb;
 		}
 	}
 	
 	return h;
}

/*! Compute the checksum by which we speed AP equivalence testing.  (?)To the best of my knowledge,
	 collision is still possible.  (?)TODO: Document the algorithm/process here, and
	 answer remaining questions about this chunk of code.
*/
void PP_AttrProp::_computeCheckSum(void)
{
	m_checkSum = 0;
 
 	if (!m_pAttributes && !m_pProperties)
  		return;
 
	const gchar *s1, *s2;
 	UT_uint32	cch = 0;
 	gchar	*rgch;
    
	rgch = NULL;
 	if (m_pAttributes)
  	{
 		UT_GenericStringMap<gchar*>::UT_Cursor c1(m_pAttributes);
 		const gchar *val = c1.first();
 
 		while (val != NULL)
 		{
 			s1 = c1.key().c_str();
 			s2 = val;
  
 			cch = strlen(s1);
  
 			m_checkSum = hashcodeBytesAP(m_checkSum, s1, cch);
 
 			cch = strlen(s2);

			rgch = g_ascii_strdown(s2, 9);
			rgch[8] = '\0';
 			m_checkSum = hashcodeBytesAP(m_checkSum, rgch, cch);
			g_free (rgch); rgch = NULL;

 			if (!c1.is_valid())
 				break;
 			val = c1.next();
 		}
 	}
 
 	if (m_pProperties)
  	{
 		UT_GenericStringMap<PropertyPair*>::UT_Cursor c2(m_pProperties);
 		const PropertyPair *val;
 
 		val = c2.first();
 		while (val != NULL)
 		{
 			s1 = c2.key().c_str(); 
 			cch = strlen(s1);
			rgch = g_ascii_strdown(s1, 9);
			rgch[8] = '\0';
 			m_checkSum = hashcodeBytesAP(m_checkSum, rgch, cch);
			g_free (rgch); rgch = NULL;
  
 			s2 = val->first;
 			cch = strlen(s2);
			rgch = g_ascii_strdown(s2, 9);
			rgch[8] = '\0';
 			m_checkSum = hashcodeBytesAP(m_checkSum, rgch, cch);
			g_free (rgch); rgch = NULL;
 
 			if (!c2.is_valid())
 				break;
 			val = c2.next();
 		}
	}
	return;
}

/*! This is an accessor method that gets the checksum of [this] AP, by which we speed
	 equivalence testing.  The speedup occurs when we bypass full-testing
	 in the case of checksum mismatch when we know for sure that the APs are not
	 equivalent.  Because collisions still occur, we do the full-testing if the
	 checksums do match.
	 \retval m_checkSum The unsigned 32-bit integer which serves as the AP's checksum.
*/
UT_uint32 PP_AttrProp::getCheckSum(void) const
{
  //
  // Why is this assert present? This is harmless.
  //	UT_ASSERT_HARMLESS(m_bIsReadOnly);
	return m_checkSum;
}

void PP_AttrProp::operator = (const PP_AttrProp &Other)
{
	UT_uint32 countMyAttrs = ((Other.m_pAttributes) ? Other.m_pAttributes->size() : 0);

	UT_uint32 Index;
	for(Index = 0; Index < countMyAttrs; Index++)
	{
		const gchar * szName;
		const gchar * szValue;
		if(Other.getNthAttribute(Index, szName, szValue))
		{
			setAttribute(szName, szValue);
		}
	}

	UT_uint32 countMyProps = ((Other.m_pProperties) ? Other.m_pProperties->size() : 0);

	for(Index = 0; Index < countMyProps; Index++)
	{
		const gchar * szName;
		const gchar * szValue;
		if(Other.getNthProperty(Index, szName, szValue))
		{
			setProperty(szName, szValue);
		}
	}

}

UT_uint32 PP_AttrProp::getIndex(void) const
{
	return m_index;
}

void PP_AttrProp::setIndex(UT_uint32 i)
{
	m_index = i;
	if(m_szProperties)
	{
		delete [] m_szProperties;
	}
	m_szProperties = NULL;
}

/*! This method checks if [this] AP and the given AP are functionally equivalent.
    In contrast to is isExactMatch this function will return true if
    the attrs and props are same even if they are in different order;
    it is computationally much more involved than isExactMatch().
	 On that note, it may be worth looking into putting some more explicit collision
	 guarantees into the checksum computation algorithm, such to take advantage of
	 those checksums here.  IOW, make it such to be trusted that if checksums match,
	 APs are equivalent (but not necessarily exact matches).
	 \retval TRUE if equivalent to given AP (regardless of order), FALSE otherwise.
*/
bool PP_AttrProp::isEquivalent(const PP_AttrProp * pAP2) const
{
	if(!pAP2)
		return false;
	
	if(   getAttributeCount() != pAP2->getAttributeCount()
	   || getPropertyCount()  != pAP2->getPropertyCount())
		return false;
	
	UT_uint32 i;
	const gchar * pName, * pValue, * pValue2;
	
	for(i =  0; i < getAttributeCount(); ++i)
	{
		UT_return_val_if_fail(getNthAttribute(i,pName,pValue),false);

		if(!pAP2->getAttribute(pName,pValue2))
			return false;

		// ignore property attribute
		if(0 == strcmp(pValue, PT_PROPS_ATTRIBUTE_NAME))
			continue;

		// handle revision attribute correctly
		if(0 == strcmp(pValue, PT_REVISION_ATTRIBUTE_NAME))
		{
			// requires special treatment
			PP_RevisionAttr r1(pValue);
			PP_RevisionAttr r2 (pValue2);

			if(!(r1 == r2))
			{
				return false;
			}
		}
		else if(0 != strcmp(pValue,pValue2))
			return false;
	}

	for(i =  0; i < getPropertyCount(); ++i)
	{
		UT_return_val_if_fail(getNthProperty(i,pName,pValue),false);

		if(!pAP2->getProperty(pName,pValue2))
			return false;

		if(0 != strcmp(pValue,pValue2))
			return false;
	}

	return true;
}

/*! This method does the same as PP_AttrProp::isEquivalent(const PP_AttrProp *) except
	 that instead of being passed another AP to be compared to, it is passed sets of
	 attributes and properties which only virtually comprise another AP.
	 \retval TRUE if equivalent (regardless of order) to given Attrs and Props, FALSE otherwise.
*/
bool PP_AttrProp::isEquivalent(const gchar ** attrs, const gchar ** props) const
{
	UT_uint32 iAttrsCount  = 0;
	UT_uint32 iPropsCount = 0;

	const gchar ** p = attrs;

	while(p && *p)
	{
		iAttrsCount++;
		p += 2;
	}
	
	p = props;

	while(p && *p)
	{
		iPropsCount++;
		p += 2;
	}

	
	if(   getAttributeCount() != iAttrsCount
	   || getPropertyCount()  != iPropsCount)
		return false;
	
	UT_uint32 i;
	const gchar * pName, * pValue, * pValue2;
	
	for(i =  0; i < getAttributeCount(); ++i)
	{
		pName = attrs[2*i];
		pValue = attrs[2*i + 1];

		if(!getAttribute(pName,pValue2))
			return false;

		// ignore property attribute
		if(0 == strcmp(pValue, PT_PROPS_ATTRIBUTE_NAME))
			continue;

		// handle revision attribute correctly
		if(0 == strcmp(pValue, PT_REVISION_ATTRIBUTE_NAME))
		{
			// requires special treatment
			PP_RevisionAttr r1(pValue);
			PP_RevisionAttr r2 (pValue2);

			if(!(r1 == r2))
			{
				return false;
			}
		}
		else if(0 != strcmp(pValue,pValue2))
			return false;
	}

	for(i =  0; i < getPropertyCount(); ++i)
	{
		pName = props[2*i];
		pValue = props[2*i + 1];

		if(!getProperty(pName,pValue2))
			return false;

		if(0 != strcmp(pValue,pValue2))
			return false;
	}

	return true;
}

/*! \fn bool PP_AttrProp::explodeStyle(const PD_Document * pDoc, bool bOverwrite)
	 \brief This function transfers attributes and properties defined in style into the AP.
    \param bOverwrite indicates what happens if the property/attribute is already present. If false \
     (default) the style definition is ignored; if true the style value overrides the present value.
*/
bool PP_AttrProp::explodeStyle(const PD_Document * pDoc, bool bOverwrite)
{
	UT_return_val_if_fail(pDoc,false);
	
	// expand style if present
	const gchar * pszStyle = NULL;
	if(getAttribute(PT_STYLE_ATTRIBUTE_NAME, pszStyle))
	{
		PD_Style * pStyle = NULL;

        if(pszStyle && (strcmp(pszStyle, "None") != 0) && pDoc->getStyle(pszStyle,&pStyle))
        {
			UT_Vector vAttrs;
			UT_Vector vProps;

			UT_sint32 i;

			pStyle->getAllAttributes(&vAttrs, 100);
			pStyle->getAllProperties(&vProps, 100);

			for(i = 0; i < vProps.getItemCount(); i += 2)
			{
				const gchar * pName =  (const gchar *)vProps.getNthItem(i);
				const gchar * pValue = (const gchar *)vProps.getNthItem(i+1);
				const gchar * p;

				bool bSet = bOverwrite || !getProperty(pName, p);

				if(bSet)
					setProperty(pName, pValue);
			}

			// attributes are more complicated, because there are some style attributes that must
			// not be transferred to the generic AP
			for(i = 0; i < vAttrs.getItemCount(); i += 2)
			{
				const gchar * pName = (const gchar *)vAttrs.getNthItem(i);
				if(!pName || !strcmp(pName, "type")
				          || !strcmp(pName, "name")
				          || !strcmp(pName, "basedon")
				          || !strcmp(pName, "followedby")
 				          || !strcmp(pName, "props"))
				{
					continue;
				}

				const gchar * pValue = (const gchar *)vAttrs.getNthItem(i+1);
				const gchar * p;

				bool bSet = bOverwrite || !getAttribute(pName, p);

				if(bSet)
					setAttribute(pName, pValue);
			}
		}
	}

	return true;
}

/*! This is a debugging tool which serves to dump in readable form (as UT_DEBUGMSGs)
	 the contents of [this] AP.
*/
void PP_AttrProp::miniDump(const PD_Document * pDoc) const
{
#ifdef DEBUG
	const gchar * pName, * pValue;
	UT_uint32 i = 0;
	
	UT_DEBUGMSG(("--------------------- PP_AttrProp mini dump --------------------------------\n"));
	UT_DEBUGMSG(("Attributes:\n"));
	while(getNthAttribute(i,pName,pValue))
	{
		UT_DEBUGMSG(("%s : %s\n", pName, pValue));
		++i;
	}
		  
	UT_DEBUGMSG(("Properties:\n"));
	i = 0;
	while(getNthProperty(i,pName,pValue))
	{
		UT_DEBUGMSG(("%s : %s\n", pName, pValue));
		++i;
	}

	if(m_iRevisedIndex != 0xffffffff && pDoc)
	{
		UT_DEBUGMSG(("Attached revised AP:\n"));
		const PP_AttrProp * pRevAP;
		pDoc->getAttrProp(m_iRevisedIndex, const_cast<const PP_AttrProp **>(&pRevAP));

		// avoid endless loop on circular reference
		if(pRevAP && pRevAP->getRevisedIndex() != m_iRevisedIndex)
			pRevAP->miniDump(pDoc);
	}
	
	UT_DEBUGMSG(("----------------------------------------------------------------------------\n"));
#else
	UT_UNUSED(pDoc);
#endif
}
