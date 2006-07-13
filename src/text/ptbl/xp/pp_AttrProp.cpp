/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
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
#include "pp_Revision.h"
#include "pd_Style.h"
#include "pd_Document.h"
#include "pp_Attribute.h"

/****************************************************************/

/// This is the sole explicit constructor of class PP_AttrProp.
PP_AttrProp::PP_AttrProp()
    : m_pAttributes (NULL),
      m_pProperties (NULL),
      m_bIsReadOnly (false),
      m_checkSum (0),
      m_iRevisedIndex (0xffffffff),
      m_bRevisionHidden (false)
{
}

/// This is the sole explicit destructor of class PP_AttrProp.
PP_AttrProp::~PP_AttrProp()
{
	xxx_UT_DEBUGMSG(("deleting pp_AttrProp %x \n",this));
	if (m_pAttributes)
	{
		delete m_pAttributes;
	}

	// delete any PP_Property_types;

	if(m_pProperties)
	{
		delete m_pProperties;
	}
}

/*!
 * Returns the number of properties in this PP_AttrProp.
 *
 * BE WEARY, BE VERY WEARY, the count can be greater than the number of valid
 * items stored in the hash -- always check the return value of getNthProperty()
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
 * BE WEARY, BE VERY WEARY, the count can be greater than the number of valid
 * items stored in the hash -- always check the return value of
 * getNthAttribute()
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
bool PP_AttrProp::setAttributes(const PT_AttributePair * attributes)
{
	if (!attributes)
		return true;

	const PT_AttributePair * pp = attributes;
	while (pp)
	{
		if (!setAttribute(pp->a,pp->v))
			return false;
		pp++;
	}
	return true;
}

/*!
 * Sets attributes as given in the UT_Vector of strings, read as
 * (attribute, value) pairs.
 *
 * \param attributes A UT_Vector of strings, read in (attribute, value) form.
 */
bool PP_AttrProp::setAttributes(const PT_AttributeVector & vAttrs)
{
	UT_uint32 kLimit = vAttrs.getItemCount();
	for (UT_uint32 k=0; k < kLimit; k++)
	{
		const PT_AttributePair & p = vAttrs.getNthItem(k);
		if (!setAttribute(p.a, p.v))
			return false;
	}
	return true;
}


bool PP_AttrProp::setAttributes(const AP_Attrs * pAttrs)
{
	if(!pAttrs)
		return false;

	AP_Attrs::const_iterator c = pAttrs->begin();
	while (c != pAttrs->end())
	{
		if(!setAttribute(c->first, c->second))
			return false;

		++c;
	}
	
	return true;
}


/*!
 * Sets attributes as given in the NULL-terminated input array
 * of (attribute, value) pairs.
 */
bool	PP_AttrProp::setProperties(const PT_PropertyPair * properties)
{
	if (!properties)
		return true;

	const PT_PropertyPair * pp = properties;
	while (pp->p)
	{
		if (!setProperty(pp->p,pp->v))
			return false;
		pp++;
	}
	return true;
}

/*!
 * Sets properties as given in the UT_Vector of strings, read as
 * (property, value) pairs.
 *
 * \param pVector A UT_Vector of strings, read in (properties, value) form.
 */
bool PP_AttrProp::setProperties(const PT_PropertyVector & vProps)
{
	UT_uint32 kLimit = vProps.getItemCount();
	for (UT_uint32 k=0; k < kLimit; k++)
	{
		const PT_PropertyPair & p = vProps.getNthItem(k);
		if (!setProperty(p.p, p.v))
			return false;
	}
	return true;
}

bool PP_AttrProp::setProperties(const AP_Props * pProps)
{
	if (!pProps)
		return false;

	AP_Props::const_iterator c = pProps->begin();

	while (c != pProps->end())
	{
		if(!setProperty(c->first, c->second.first))
			return false;
		++c;
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
bool PP_AttrProp::setAttribute(PT_Attribute name, GQuark value)
{
	if (name == PT_PROPS_ATTRIBUTE_NAME && value)	
	{
        // PROPS -- cut value up into properties
		char * pOrig = NULL;
		
		if (!UT_cloneString(pOrig,g_quark_to_string (value)))
		{
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

			// now, search ahead for the next semicolon, and separate this
			// property from the next
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

			setProperty(PP_Property::lookupIndex(p),
						g_quark_from_string(q));
		}

		free(pOrig);
		return true;
	}
	else if (name == PT_XID_ATTRIBUTE_NAME && value)
	{
		// XID is a unique id for the xml element / PT frag. Its function is to
		// facilitate comparing/merging documents and we do not want it in the
		// AP
		return true;
	}
	else // not "PROPS" -- add to attribute list
	{
		UT_UTF8String url;
		const char * szValue = NULL;
		
		if (value &&
			(name == PT_XLINKHREF_ATTRIBUTE_NAME ||
			 name == PT_HREF_ATTRIBUTE_NAME))
		{
			url = g_quark_to_string (value);
			url.decodeURL();
			szValue = url.utf8_str();
		}

		if (szValue)
			value = g_quark_from_string (szValue);
		
		if (!m_pAttributes)
		{
			m_pAttributes = new AP_Attrs;
			if (!m_pAttributes)
			{
				UT_DEBUGMSG(("setAttribute: could not allocate hash table.\n"));
				return false;
			}
		}

		(*m_pAttributes)[name] = value;
		
		return true;
	}
}

/*! This method inserts a new pair of property name and value into [this] APs
	set of properties, creating "props" if it does not already exist and
	overwriting the value of any property of the same name with the newly passed
	value.  (?)It appears as though we replace the entire pair, rather than only
	the value.  (?)Is there a reason for this?
	
	\return Whether or not the operation succeeded.
*/
bool PP_AttrProp::setProperty(PT_Property name, GQuark value)
{
	if (!m_pProperties)
	{
		m_pProperties = new AP_Props;
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
	char * szValue2 = NULL;
	const char * szValue = g_quark_to_string (value);
	
	if(!UT_isValidXML(szValue))
	{
		szValue2 = UT_strdup(szValue);

		// get rid of any illegal chars we were passed
		UT_validXML(szValue2);

		value = g_quark_from_string (szValue2);
		FREEP(szValue2);
	}
	
	(*m_pProperties)[name].first = value;

	return true;
}

void PP_AttrProp::forEachAttribute (PP_ForEachAttrFunc func,
									void * data) const
{
	if (!m_pAttributes)
		return;

	AP_Attrs::iterator c = m_pAttributes->begin();
	UT_uint32 i = 0;
	
	while (c != m_pAttributes->end())
	{
		if (! func (c->first, c->second, i, data))
			break;
		i++;
		c++;
	}
}

void PP_AttrProp::forEachProperty  (PP_ForEachPropFunc func,
									void * data) const
{
	if (!m_pProperties)
		return;

	AP_Props::iterator c = m_pProperties->begin();
	UT_uint32 i = 0;
	
	while (c != m_pProperties->end())
	{
		if (! func(c->first, c->second.first, i, data))
			break;
		i++;
		c++;
	}
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
bool PP_AttrProp::getProperty(PT_Property name, GQuark & value) const
{
	if (!m_pProperties)
		return false;

	AP_Props::iterator c = m_pProperties->find(name);

	if(c == m_pProperties->end())
		return false;

	value = c->second.first;
	
	return true;
}

/*! (?)TODO: PLEASE DOCUMENT ME!
*/
const PP_PropertyType *PP_AttrProp::getPropertyType(PT_Property name, tProperty_type Type) const
{
	if (!m_pProperties)
		return NULL;

	AP_Props::iterator c = m_pProperties->find(name);

	if (c == m_pProperties->end())
		return NULL;

	if(!c->second.second)
	{
		c->second.second =
			PP_PropertyType::createPropertyType(Type,c->second.first);
	}

	return c->second.second;
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
bool PP_AttrProp::getAttribute(PT_Attribute name, GQuark & value) const
{
	if (!m_pAttributes)
		return false;

	AP_Attrs::iterator c = m_pAttributes->find(name);
	if (c == m_pAttributes->end())
		return false;

	value = c->second;
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
/*! This method checks [this] AP for the presence of any attribute.  If there is
    any attribute at all in the AP, m_pAttributes->size() returns positive and
    so does this method.  Otherwise, this method returns false.  Beware that no
    sanity checking is done to make sure that whatever is in there (being
    counted by size()) is anything more legible than uninitialized memory.
*/
bool PP_AttrProp::hasAttributes(void) const
{
	if (!m_pAttributes)
		return false;

	return (m_pAttributes->size() > 0);
}

/// Returns whether or not the given attributes and properties are identically
/// present in [this] AP.
/*! This method compares the given attributes and properties with those already
    present.  It compares the given items as inseparable pairs - if the
    attribute or property is present in name but contains a different value,
    that does not count and false is returned.

    \return A bool indicating (directly) both presence and equality.
*/
bool PP_AttrProp::areAlreadyPresent(const PT_AttributePair * attributes,
									const PT_PropertyPair  * properties) const
{
	if (attributes)
	{
		const PT_AttributePair * p = attributes;
		while (p->a)
		{
			/*
			    It seems that we also want empty strings for attributes,
				at least for the 'param' attribute which goes with fields.
				Is this bogus too? -PL

				Yes.  We use empty strings and NULL in values to indicate that
				the attribute/property should be absent. Sometimes these values
				filter down here, so we need to handle this. TF
			*/
			
			// first deal with the case where the value is set to NULL or "" --
			// we want that attribute to be absent, not present
			GQuark value = 0;

			if((p->v == 0) && getAttribute(p->a, value) && value)
				return false;
			
			// the 'props' attribute has to be handled separatedly, since it is
			// not returned using getAttribute() (it is not stored as attribute)
			else if((p->v == 0)
					&& p->a == PT_PROPS_ATTRIBUTE_NAME
					&& hasProperties())
				return false;
			
			else if(p->v != 0)
			{
				if (!getAttribute(p->a, value))
					return false;		// item not present
				
				if (p->v != value)
					return false;		// item has different value
			}
					
			p++;
		}
	}

	if (properties)
	{
		const PT_PropertyPair * p = properties;
		while (p->p)
		{
			/*
				We use empty strings and NULL in values to indicate that the
				attribute/property should be absent. Sometimes these values
				filter down here, so we need to handle this. TF
			*/
			// first deal with the case where the value is set to NULL or "" --
			// we want that attribute to be absent, not present
			GQuark value = 0;

			if((p->v == 0) &&
			   getProperty(p->p, value) && value)
				return false;
			else if(p->v != 0)
			{
				if (!getProperty(p->p, value))
					return false;		// item not present
				if (p->v != value)
					return false;		// item has different value
			}
			
			p++;
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
bool PP_AttrProp::areAnyOfTheseNamesPresent(const PT_AttributePair *attributes,
											const PT_PropertyPair  *properties) const
{
	if (attributes)
	{
		const PT_AttributePair * p = attributes;
		while (p->a)
		{
			GQuark value;
			if (getAttribute(p->a, value))
				return true;
			p++;						// skip over value
		}
	}

	if (properties)
	{
		const PT_PropertyPair * p = properties;
		while (p->p)
		{
			GQuark value;
			if (getProperty(p->p, value))
				return true;
			p++;						// skip over value
		}
	}

	return false;					// didn't find any
}

/*! Checks to see if the given AP is identical to itself ([this]).  It also
	contains some useful points of instrumentation for benchmarking table and
	usage characteristics.  \return TRUE, if and only if we match the AP given,
	false otherwise.
*/
bool PP_AttrProp::isExactMatch(const PP_AttrProp * pMatch) const
{
	// The counters below are used in testing to profile call and chksum
	// characteristics, including collision rates.  NB: I'm not sure this
	// initialization block is in the correct place.
#ifdef PT_TEST
	static UT_uint32 s_Calls = 0;
	static UT_uint32 s_PassedCheckSum = 0;
	static UT_uint32 s_Matches = 0;
#endif
#ifdef PT_TEST
	s_Calls++;
#endif

	UT_return_val_if_fail (pMatch, false);
	UT_return_val_if_fail (m_bIsReadOnly && pMatch->m_bIsReadOnly, false);
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
		AP_Attrs::iterator ca1 = m_pAttributes->begin();
		AP_Attrs::iterator ca2 = pMatch->m_pAttributes->begin();

		do
		{
			// compare keys
			if (ca1->first != ca2->first)
				return false;

			//compare values
			if (ca1->second != ca2->second)
				return false;

			ca1++;
			ca2++;
			
		} while (ca1 != m_pAttributes->end());
	}

	if (countMyProps > 0)
	{
		AP_Props::iterator cp1 = m_pProperties->begin();
		AP_Props::iterator cp2 = pMatch->m_pProperties->begin();

		do
		{
			if (cp1->first != cp2->first)
				return false;

			if (cp1->second.first != cp2->second.first)
				return false;

			cp1++;
			cp2++;
			
		} while (cp1 != m_pProperties->end());

#ifdef PT_TEST
		s_Matches++;
#endif
	}

	return true;
}


/*! Create a new AttrProp with exactly the attributes/properties given.
  \return NULL on failure, the newly-created PP_AttrProp.
*/
PP_AttrProp * PP_AttrProp::createExactly(const PT_AttributePair * attrs,
										 const PT_PropertyPair   * props) const
{
	// first, create a new AttrProp using just the values given.

	PP_AttrProp * papNew = new PP_AttrProp();
	if (!papNew)
		goto Failed;
	
	if (!papNew->setAttributes(attrs) ||
		!papNew->setProperties(props))
		goto Failed;
	
	return papNew;

Failed:
	DELETEP(papNew);
	return NULL;
}

/*! Create a new AttrProp based upon the given one, adding or replacing the
	 items given.

	 \return NULL on failure, the newly-created PP_AttrProp clone
	 otherwise.
*/
PP_AttrProp * PP_AttrProp::cloneWithReplacements(const PT_AttributePair *attrs,
												 const PT_PropertyPair  *props,
												 bool bClearProps) const
{
	bool bIgnoreProps = false; // see below

	// first, create a new AttrProp using just the values given.

	PP_AttrProp * papNew = new PP_AttrProp();
	if (!papNew)
		goto Failed;
	
	if (!papNew->setAttributes(attrs) ||
		!papNew->setProperties(props))
		goto Failed;

	// next, add any items that we have that are not present
	// (have not been overridden) in the new one.

	GQuark v;
	GQuark vNew;

	if (m_pAttributes)
	{
		AP_Attrs::iterator ca = m_pAttributes->begin();
	
		while (ca != m_pAttributes->end())
		{
			if (ca->first == PT_PROPS_ATTRIBUTE_NAME)
			{
				ca++;
				continue;
			}
			
			if (!papNew->getAttribute(ca->first,vNew))
				if (!papNew->setAttribute(ca->first,ca->second))
					goto Failed;
			ca++;
		}
	}
	
	// we want to be able to remove all properties by setting the
	// props attribute to ""; in order for that to work, we need to
	// skip the following loop if props is set to ""

	if(papNew->getAttribute(PT_PROPS_ATTRIBUTE_NAME, v) &&
	   v == g_quark_from_string (""))
		bIgnoreProps = true;

	if (!bClearProps && !bIgnoreProps && m_pProperties)
	{
		AP_Props::iterator cp = m_pProperties->begin ();
		while (cp != m_pProperties->end())
		{
			if (!papNew->getProperty(cp->first,vNew))
				if (!papNew->setProperty(cp->first,cp->second.first))
					goto Failed;

			cp++;
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

	AP_Props::iterator c = m_pProperties->begin();

	while (c != m_pProperties->end())
	{
		if (c->second.first == 0 ||
			c->second.first == g_quark_from_string (""))
		{
			delete c->second.second;
			m_pProperties->erase (c);
		}
		
		c++;
	}
}

/// This function will remove all attributes that are equal to "" (*<XML_Char *> == NULL)
void PP_AttrProp::_clearEmptyAttributes()
{
	if(!m_pAttributes)
		return;

	AP_Attrs::iterator c = m_pAttributes->begin();

	
	while (c != m_pAttributes->end())
	{
		if (c->second == 0 ||
			c->second == g_quark_from_string (""))
		{
			m_pAttributes->erase (c);
		}
		
		c++;
	}
}

/// This method clears both empty attributes and empty properties from [this] AP.
void PP_AttrProp::prune()
{
	_clearEmptyAttributes();
	_clearEmptyProperties();
}


/*! Create a new AttrProp based upon the given one, removing the items given
	 regardless of their value.  See also
	 PP_AttrProp::cloneWithEliminationIfEqual, which does similarly but only
	 removes the items given if their value is equal to the value given.
	 \return NULL on failure, the newly-created PP_AttrProp clone otherwise.
*/
PP_AttrProp * PP_AttrProp::cloneWithElimination(const PT_AttributePair * attrs,
												const PT_PropertyPair *  props) const
{
	// first, create an empty AttrProp.
	PP_AttrProp * papNew = new PP_AttrProp();
	if (!papNew)
		goto Failed;

	if (m_pAttributes)
	{
		AP_Attrs::iterator ca = m_pAttributes->begin();
	
		while (ca != m_pAttributes->end())
		{
			// for each attribute in the old set, add it to the
			// new set only if it is not present in the given array.

			if(ca->first == PT_PROPS_ATTRIBUTE_NAME)
				goto DoNotIncludeAttribute;
			
			// TODO optimise this by taking the if () of the loop here
			if (attrs)
			{
				const PT_AttributePair * p = attrs;
				while (p->a)
				{
					if (p->a == PT_PROPS_ATTRIBUTE_NAME)
					{
						p++;
						continue;
					}
					
					if (ca->first == p->a)// found it - don't put it in
						goto DoNotIncludeAttribute;
					
					p++;								// skip over value
				}
			}

			// we didn't find it in the given array, add it to the new set.

			if (!papNew->setAttribute(ca->first, ca->second))
				goto Failed;

		  DoNotIncludeAttribute:
			ca++;
		}
	}

	if (m_pProperties)
	{
		AP_Props::iterator cp = m_pProperties->begin ();
		
		while (cp != m_pProperties->end ())
		{
			// for each property in the old set, add it to the
			// new set only if it is not present in the given array.

			if (props)
			{
				const PT_PropertyPair * p = props;
				while (p->p)
				{
					if (cp->first == p->p)// found it - don't put it in
						goto DoNotIncludeProperty;
					
					p++;
				}
			}

			// we didn't find it in the given array, add it to the new set.

			if (!papNew->setProperty(cp->first,cp->second.first))
				goto Failed;

		  DoNotIncludeProperty:
			cp++;
		}
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

/*! Create a new AttrProp based upon the given one, removing the items given if
    their value is equal to that given.  See also
    PP_AttrProp::cloneWithElimination, which does similarly but removes the
    items given regardless of whether or not their value is equal to the value
    given.

    \return NULL on failure, the newly-created PP_AttrProp clone
    otherwise.
*/
PP_AttrProp * PP_AttrProp::cloneWithEliminationIfEqual(const PT_AttributePair* attributes,
												const PT_PropertyPair * properties) const
{
	// first, create an empty AttrProp.
	PP_AttrProp * papNew = new PP_AttrProp();
	if (!papNew)
		goto Failed;

	if (m_pAttributes)
	{
		AP_Attrs::iterator ca = m_pAttributes->begin ();
		
		while (ca != m_pAttributes->end())
		{
			// for each attribute in the old set, add it to the
			// new set only if it is not present in the given array.
			if(ca->first == PT_PROPS_ATTRIBUTE_NAME)
				goto DoNotIncludeAttribute;

			if (attributes)
			{
				const PT_AttributePair * p = attributes;
				while (p->a)
				{
					if (p->a == PT_PROPS_ATTRIBUTE_NAME)
					{
						p++;
						continue;
					}
					
					if (ca->first == p->a &&
						ca->second == p->v)// found it, don't put it in
						goto DoNotIncludeAttribute;
					
					p++;								// skip over value
				}
			}

			// we didn't find it in the given array, add it to the new set.

			if (!papNew->setAttribute(ca->first,ca->second))
				goto Failed;

		  DoNotIncludeAttribute:
			ca++;
		}
	}
	
	if (m_pProperties)
	{
		AP_Props::iterator cp = m_pProperties->begin ();
		
		while (cp != m_pProperties->end ())
		{
			// for each property in the old set, add it to the
			// new set only if it is not present in the given array.

			if (properties)
			{
				const PT_PropertyPair * p = properties;
				while (p->p)
				{
					if (cp->first == p->p &&
						cp->second.first == p->v) // found it, don't put it in
						goto DoNotIncludeProperty;
					
					p++;
				}
			}

			// we didn't find it in the given array, add it to the new set.

			if (!papNew->setProperty(cp->first,cp->second.first))
				goto Failed;

		  DoNotIncludeProperty:
			cp++;
		}
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

/*! Compute the checksum by which we speed AP equivalence testing.
	
	(?)To the best of my knowledge, collision is still possible.

	(?)TODO: Document the algorithm/process here, and answer remaining
	questions about this chunk of code.
*/
void PP_AttrProp::_computeCheckSum(void)
{
	m_checkSum = 0;
 
 	if (!m_pAttributes && !m_pProperties)
  		return;
 
 	XML_Char	rgch[9];
  
 	rgch[8] = 0;
  
 	if (m_pAttributes)
  	{
 		AP_Attrs::iterator ca = m_pAttributes->begin();

		while (ca != m_pAttributes->end())
 		{
			GQuark v = ca->first;
 			m_checkSum = hashcodeBytesAP(m_checkSum, &v, sizeof(GQuark));

			v = ca->second;
 			m_checkSum = hashcodeBytesAP(m_checkSum, &v, sizeof(GQuark));

			ca++;
 		}
 	}
 
 	if (m_pProperties)
  	{
 		AP_Props::iterator cp = m_pProperties->begin();
 
 		while (cp != m_pProperties->end())
 		{
			GQuark v = cp->first;
 			m_checkSum = hashcodeBytesAP(m_checkSum, &v, sizeof(GQuark));

			v = cp->second.first;
 			m_checkSum = hashcodeBytesAP(m_checkSum, &v, sizeof(GQuark));

			cp++;
 		}
	}
	return;
}

/*! This is an accessor method that gets the checksum of [this] AP, by which we
	speed equivalence testing.  The speedup occurs when we bypass full-testing
	in the case of checksum mismatch when we know for sure that the APs are not
	equivalent.  Because collisions still occur, we do the full-testing if the
	checksums do match.

	\retval m_checkSum The unsigned 32-bit integer which serves as the AP's
	checksum.
*/
UT_uint32 PP_AttrProp::getCheckSum(void) const
{
	UT_ASSERT_HARMLESS(m_bIsReadOnly);
	return m_checkSum;
}

void PP_AttrProp::operator = (const PP_AttrProp &Other)
{
	UT_uint32 countMyAttrs = ((Other.m_pAttributes) ? Other.m_pAttributes->size() : 0);

	if (countMyAttrs)
	{
		AP_Attrs::iterator ca = Other.m_pAttributes->begin ();

		while (ca != Other.m_pAttributes->end ())
		{
			setAttribute (ca->first, ca->second);
			ca++;
		}
	}
	
	UT_uint32 countMyProps = ((Other.m_pProperties) ? Other.m_pProperties->size() : 0);

	if (countMyProps)
	{
		AP_Props::iterator cp = Other.m_pProperties->begin ();

		while (cp != Other.m_pProperties->end ())
		{
			setProperty (cp->first, cp->second.first);
			cp++;
		}
	}
}

UT_uint32 PP_AttrProp::getIndex(void)
{
	return m_index;
}

void PP_AttrProp::setIndex(UT_uint32 i)
{
	m_index = i;
}

/*! This method checks if [this] AP and the given AP are functionally
    equivalent.

    In contrast to is isExactMatch this function will return true
    if the attrs and props are same even if they are in different order; it is
    computationally much more involved than isExactMatch().

    On that note, it may be worth looking into putting some more explicit
    collision guarantees into the checksum computation algorithm, such to take
    advantage of those checksums here.

    IOW, make it such to be trusted that if checksums match, APs are equivalent
    (but not necessarily exact matches).

    \retval TRUE if equivalent to given AP (regardless of order), FALSE
    otherwise.
*/
bool PP_AttrProp::isEquivalent(const PP_AttrProp * pAP2) const
{
	if(!pAP2)
		return false;
	
	if(   getAttributeCount() != pAP2->getAttributeCount()
	   || getPropertyCount()  != pAP2->getPropertyCount())
		return false;
	
	if(m_pAttributes)
	{
		AP_Attrs::iterator ca = m_pAttributes->begin ();
		GQuark value2;
		
		while (ca != m_pAttributes->end())
		{
			// ignore property attribute
			if(ca->first == PT_PROPS_ATTRIBUTE_NAME)
				continue;
			
			if(!pAP2->getAttribute(ca->first, value2))
				return false;
 

			// handle revision attribute correctly
			if(ca->first == PT_REVISION_ATTRIBUTE_NAME)
			{
				// requires special treatment
				PP_RevisionAttr r1(ca->second);
				PP_RevisionAttr r2 (value2);

				if(!(r1 == r2))
				{
					return false;
				}
			}
			else if(ca->second != value2)
				return false;

			ca++;
		}
	}

	if (m_pProperties)
	{
		AP_Props::iterator cp = m_pProperties->begin ();
		GQuark value2;
		
		while (cp != m_pProperties->end())
		{
			if(!pAP2->getProperty(cp->first,value2))
				return false;

			if(cp->second.first != value2)
				return false;
		}
	}
	
	return true;
}

/*! This method does the same as PP_AttrProp::isEquivalent(const PP_AttrProp *)
	except that instead of being passed another AP to be compared to, it is
	passed sets of attributes and properties which only virtually comprise
	another AP.

	\retval TRUE if equivalent (regardless of order) to given Attrs and Props,
	FALSE otherwise.
*/
bool PP_AttrProp::isEquivalent(const PT_AttributePair * attrs,
							   const PT_PropertyPair  * props) const
{
	UT_uint32 iAttrsCount  = 0;
	UT_uint32 iPropsCount = 0;

	const PT_AttributePair * pa = attrs;

	while(pa->a)
	{
		iAttrsCount++;
		pa++;
	}
	
	const PT_PropertyPair * pp = props;

	while(pp->p)
	{
		iPropsCount++;
		pp++;
	}

	
	if(   getAttributeCount() != iAttrsCount
	   || getPropertyCount()  != iPropsCount)
		return false;

	if (m_pAttributes)
	{
		GQuark value2;
		for(UT_uint32 i =  0; i < iAttrsCount; ++i)
		{
			// ignore property attribute
			if(attrs[i].a == PT_PROPS_ATTRIBUTE_NAME)
				continue;

			if(!getAttribute(attrs[i].a,value2))
				return false;

			// handle revision attribute correctly
			if(attrs[i].a == PT_REVISION_ATTRIBUTE_NAME)
			{
				// requires special treatment
				PP_RevisionAttr r1(attrs[i].v);
				PP_RevisionAttr r2 (value2);

				if(!(r1 == r2))
				{
					return false;
				}
			}
			else if(attrs[i].v != value2)
				return false;
		}
	}

	if(m_pProperties)
	{
		GQuark value2;
		
		for(UT_uint32 i =  0; i < iPropsCount; ++i)
		{
			if(!getProperty(props[i].p,value2))
				return false;

			if(value2 != props[i].v)
				return false;
		}
	}
	
	return true;
}

/*! \fn bool PP_AttrProp::explodeStyle(const PD_Document * pDoc, bool
	 bOverwrite)

	\brief This function transfers attributes and properties
	defined in style into the AP.

	\param bOverwrite indicates what happens if the property/attribute is
	already present. If false (default) the style definition is ignored; if true
	the style value overrides the present value.
*/
bool PP_AttrProp::explodeStyle(const PD_Document * pDoc, bool bOverwrite)
{
	UT_return_val_if_fail(pDoc,false);
	
	// expand style if present
	GQuark style = 0;
	if(getAttribute(PT_STYLE_ATTRIBUTE_NAME, style))
	{
		PD_Style * pStyle = NULL;

        if(style &&
		   style != PP_Q(None) &&
		   pDoc->getStyle(style,&pStyle))
        {
			PT_AttributeVector vAttrs;
			PT_PropertyVector  vProps;

			UT_uint32 i;

			pStyle->getAllAttributes(vAttrs, 100);
			pStyle->getAllProperties(vProps, 100);

			for(i = 0; i < vProps.getItemCount(); i++)
			{
				const PT_PropertyPair & pp = vProps.getNthItem(i);
				
				GQuark p;

				bool bSet = bOverwrite || !getProperty(pp.p, p);

				if(bSet)
					setProperty(pp.p, pp.v);
			}

			// attributes are more complicated, because there are some style
			// attributes that must not be transferred to the generic AP
			for(i = 0; i < vAttrs.getItemCount(); i++)
			{
				const PT_AttributePair & pa = vAttrs.getNthItem(i);
				/* FIXME -- tied down g_quarks for common property values */				
				if(pa.a == 0                            ||
				   pa.a == PT_TYPE_ATTRIBUTE_NAME       ||
				   pa.a == PT_NAME_ATTRIBUTE_NAME       ||
				   pa.a == PT_BASEDON_ATTRIBUTE_NAME    ||
				   pa.a == PT_FOLLOWEDBY_ATTRIBUTE_NAME ||
 				   pa.a == PT_PROPS_ATTRIBUTE_NAME)
				{
					continue;
				}

				GQuark p;

				bool bSet = bOverwrite || !getAttribute(pa.a, p);

				if(bSet)
					setAttribute(pa.a, pa.v);
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
	UT_DEBUGMSG(("--------------------- PP_AttrProp mini dump --------------------------------\n"));
	UT_DEBUGMSG(("Attributes:\n"));

	if (m_pAttributes)
	{
		AP_Attrs::iterator ca = m_pAttributes->begin();
		while(ca != m_pAttributes->end())
		{
			UT_DEBUGMSG(("%s : %s\n",
						 PP_Attribute::getName(ca->first),
						 g_quark_to_string (ca->second)));
			ca++;
		}
	}
	
	UT_DEBUGMSG(("Properties:\n"));

	if (m_pProperties)
	{
		AP_Props::iterator cp = m_pProperties->begin();
		
		while(cp != m_pProperties->end())
		{
			UT_DEBUGMSG(("%s : %s\n",
						 PP_Property::getName(cp->first),
						 g_quark_to_string (cp->second.first)));
			cp++;
		}
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
#endif
}
