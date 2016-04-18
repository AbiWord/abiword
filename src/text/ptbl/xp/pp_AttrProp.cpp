/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (c) 2001, 2002 Tomas Frydrych
 * Copyright (c) 2016 Hubert Figuiere
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
#include "ut_std_string.h"
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
	: m_bIsReadOnly(false)
	, m_checkSum(0)
	, m_index(0)
	, m_iRevisedIndex(0xffffffff)
	, m_bRevisionHidden(false)
{
}

/// This is the sole explicit destructor of class PP_AttrProp.
PP_AttrProp::~PP_AttrProp()
{
}

/*!
 * Returns the number of properties in this PP_AttrProp.
 */
size_t PP_AttrProp::getPropertyCount (void) const
{
	return m_properties.size();
}

/*!
 * Returns the number of attributes in this PP_AttrProp.
 */
size_t PP_AttrProp::getAttributeCount (void) const
{
	return m_attributes.size();
}


/*!
 * Sets attributes as given in the PP_PropertyVector, read as
 * (attribute, value) pairs.
 *
 * \param attributes A PP_PropertyVector, read in (attribute, value) form.
 */
bool PP_AttrProp::setAttributes(const PP_PropertyVector & attributes)
{
	std::size_t i = 0;
	for (auto iter = attributes.cbegin(); iter != attributes.cend(); ++iter, ++i) {
		if (!(i % 2)) {
			const std::string & key = *iter;
			++iter;
			++i;
			if (iter == attributes.cend()) {
				break;
			}
			if (!setAttribute(key.c_str(), iter->c_str())) {
				return false;
			}
		}
	}
	return true;
}

/*!
 * Sets properties as given in the PP_PropertyVector, read as
 * (property, value) pairs.
 *
 * \param pVector A PP_PropertyVector, read in (properties, value) form.
 */
bool PP_AttrProp::setProperties(const PP_PropertyVector & properties)
{
	std::size_t i = 0;
	for (auto iter = properties.cbegin(); iter != properties.cend(); ++iter, ++i) {
		if (!(i % 2)) {
			const std::string & key = *iter;
			++iter;
			++i;
			if (iter == properties.cend()) {
				break;
			}
			if (!setProperty(key.c_str(), iter->c_str())) {
				return false;
			}
		}
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
	// XXX fix this.
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

		// make sure we store attribute names in lowercase
		UT_ASSERT_HARMLESS(sizeof(char) == sizeof(gchar));

		char * copy = g_ascii_strdown(szName, -1);
		char * szDupValue = szValue ? g_strdup(szValue) : NULL;

		// get rid of any illegal chars we might have been given
		if(!UT_isValidXML(copy))
			UT_validXML(copy);
		if(!UT_isValidXML(szDupValue))
			UT_validXML(szDupValue);

		m_attributes[copy] = szDupValue;

		FREEP(szDupValue);
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

	UT_return_val_if_fail (!m_bIsReadOnly, false);
	m_properties[szName] = szValue2;

	// g_free the name duplicate if necessary
	FREEP(szValue2);
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
	if (m_attributes.empty())
		return false;
	if (static_cast<size_t>(ndx) >= m_attributes.size())
	{
		// UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN ); -- do not assert, some code in
		// while loops relies on this
		return false;
	}

	int i = 0;
	attributes_t::const_iterator iter;
	for (iter = m_attributes.cbegin(); iter != m_attributes.cend() && i < ndx;
		 ++iter, ++i) {
		// XXX this loop should be eliminated
	}

	if ((i == ndx) && iter != m_attributes.cend())
	{
	    szName = iter->first.c_str();
	    szValue = iter->second.c_str();
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
	if (m_properties.empty()) {
		return false;
	}

	if (static_cast<size_t>(ndx) >= m_properties.size()) {
		// UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN ); -- do not assert, some code in
		// while loops relies on this
  		return false;
	}

	int i = 0;
	properties_t::const_iterator iter;
	for (iter = m_properties.cbegin(); iter != m_properties.cend() && i < ndx;
	     ++iter, ++i) {
		// noop
	}

	if ((i == ndx) && iter != m_properties.cend()) {
		szName = iter->first.c_str();
		szValue = iter->second.c_str();
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
	if (m_properties.empty()) {
		return false;
	}
	auto iter = m_properties.find(szName);
	if (iter == m_properties.end()) {
		return false;
	}

	szValue = iter->second.c_str();
	return true;
}


PP_PropertyVector PP_AttrProp::getAttributes () const
{
	PP_PropertyVector attributes;
	for (auto iter = m_attributes.cbegin(); iter != m_attributes.cend();
		 ++iter) {
		attributes.push_back(iter->first);
		attributes.push_back(iter->second);
	}

	return attributes;
}

/*! This method retrieves the entirety of [this] AP's "props", and
 *  returns it as a PP_PropertyVector
 */
PP_PropertyVector PP_AttrProp::getProperties () const
{
	PP_PropertyVector properties;

	if(m_properties.empty()) {
		return properties;
	}

	for (auto iter = m_properties.cbegin(); iter != m_properties.cend();
		 ++iter) {
		properties.push_back(iter->first);
		properties.push_back(iter->second);
	}
	return properties;
}

/*! (?)TODO: PLEASE DOCUMENT ME!
*/
std::unique_ptr<PP_PropertyType> PP_AttrProp::getPropertyType(const gchar * szName, tProperty_type Type) const
{
	if (m_properties.empty()) {
		return NULL;
	}

	properties_t::const_iterator iter = m_properties.find(szName);
	if (iter == m_properties.end()) {
		return NULL;
	}

	return std::unique_ptr<PP_PropertyType>(PP_PropertyType::createPropertyType(Type, iter->second.c_str()));
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
	if (m_attributes.empty())
		return false;

	auto iter = m_attributes.find(szName);
	if (iter == m_attributes.end()) {
		return false;
	}

	szValue = iter->second.c_str();

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
	return !m_properties.empty();
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
	return !m_attributes.empty();
}

/// Returns whether or not the given attributes and properties are identically present in [this] AP.
/*! This method compares the given attributes and properties with those already present.
	 It compares the given items as inseparable pairs - if the attribute or property is
	 present in name but contains a different value, that does not count and false is
	 returned.
	 \return A bool indicating (directly) both presence and equality.
*/
bool PP_AttrProp::areAlreadyPresent(const PP_PropertyVector & attributes,
									const PP_PropertyVector & properties) const
{
	ASSERT_PV_SIZE(attributes);
	for (auto iter = attributes.cbegin(); iter != attributes.cend(); iter += 2) {
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

		if((iter + 1)->empty() && getAttribute(*iter, szValue)
		   && szValue && *szValue) {
			return false;
		}
		// the 'props' attribute has to be handled separatedly,
		// since it is not returned using getAttribute() (it is
		// not stored as attribute)
		else if((iter + 1)->empty() && (*iter != "props")
				&& hasProperties()) {
			return false;
		} else if(!(iter + 1)->empty()) {
			if (!getAttribute(*iter, szValue)) {
				return false;		// item not present
			}
			if (*(iter + 1) != szValue) {
				return false;		// item has different value
			}
		}
	}

	ASSERT_PV_SIZE(properties);
	for (auto iter = properties.cbegin(); iter != properties.cend(); iter += 2) {
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

		if(!(iter + 1)->empty() && getProperty(*iter, szValue)
		   && szValue && *szValue) {
			return false;
		} else if(!(iter + 1)->empty())	{
			if (!getProperty(*iter, szValue)) {
				return false;		// item not present
			}
			if (*(iter + 1) != szValue) {
				return false;		// item has different value
			}
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
bool PP_AttrProp::areAnyOfTheseNamesPresent(const PP_PropertyVector & attributes,
											const PP_PropertyVector & properties) const
{
	ASSERT_PV_SIZE(attributes);
	for (auto iter = attributes.cbegin(); iter != attributes.cend(); iter += 2) {

		const gchar * szValue = NULL;
		if (getAttribute(*iter, szValue)) {
			return true;
		}
	}
	ASSERT_PV_SIZE(properties);
	for (auto iter = properties.cbegin(); iter != properties.cend(); iter += 2) {

		const gchar * szValue = NULL;
		if (getProperty(*iter, szValue)) {
			return true;
		}
	}

	return false;					// didn't find any
}

/*! Checks to see if the given AP is identical to itself ([this]).  It
 *  also contains some useful points of instrumentation for
 *  benchmarking table and usage characteristics.
 *  \return TRUE, if and only if we match the AP given, false otherwise.
 */
bool PP_AttrProp::isExactMatch(const PP_AttrProp & pMatch) const
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

	//
	// Why is this here? Nothing is being changed?
	//	UT_return_val_if_fail (m_bIsReadOnly && pMatch->m_bIsReadOnly, false);
	if (m_checkSum != pMatch.m_checkSum) {
		return false;
	}

#ifdef PT_TEST
	s_PassedCheckSum++;
#endif

	UT_uint32 countMyAttrs = m_attributes.size();
	UT_uint32 countMatchAttrs = pMatch.m_attributes.size();
	if (countMyAttrs != countMatchAttrs) {
		return false;
	}

	UT_uint32 countMyProps = m_properties.size();
	UT_uint32 countMatchProps = pMatch.m_properties.size();
	if (countMyProps != countMatchProps) {
		return false;
	}

	if (countMyAttrs != 0) {
		auto ca1 = m_attributes.cbegin();
		auto ca2 = pMatch.m_attributes.cbegin();

		do {
			if (*ca1 != *ca2) {
				return false;
			}
			++ca1;
			++ca2;
		} while(ca1 != m_attributes.cend() && ca2 != pMatch.m_attributes.cend());
	}

	if (countMyProps > 0) {

		auto cp1 = m_properties.cbegin();
		auto cp2 = pMatch.m_properties.cbegin();

		do {
			if (*cp1 != *cp2) {
				return false;
			}
			++cp1;
			++cp2;
		} while(cp1 != m_properties.cend() && cp2 != pMatch.m_properties.cend());

	#ifdef PT_TEST
		s_Matches++;
	#endif
	}

	return true;
}


/*! Create a new AttrProp with exactly the attributes/properties given.
  \return NULL on failure, the newly-created PP_AttrProp.
*/
PP_AttrProp * PP_AttrProp::createExactly(const PP_PropertyVector & attributes,
					 const PP_PropertyVector & properties)
{
	// first, create a new AttrProp using just the values given.

	std::unique_ptr<PP_AttrProp> papNew(new PP_AttrProp());
	if (!papNew->setAttributes(attributes) || !papNew->setProperties(properties)) {
		return nullptr;
	}
	return papNew.release();
}

/*! Create a new AttrProp based upon the given one, adding or replacing the items given.
	 \return NULL on failure, the newly-created PP_AttrProp clone otherwise.
*/
PP_AttrProp * PP_AttrProp::cloneWithReplacements(const PP_PropertyVector & attributes,
												 const PP_PropertyVector & properties,
												 bool bClearProps) const
{
	bool bIgnoreProps = false; // see below

	// first, create a new AttrProp using just the values given.

	std::unique_ptr<PP_AttrProp> papNew(new PP_AttrProp());

	if (!papNew->setAttributes(attributes) || !papNew->setProperties(properties)) {
		return nullptr;
	}

	// next, add any items that we have that are not present
	// (have not been overridden) in the new one.

	UT_uint32 k;
	const gchar * n;
	const gchar * v;
	const gchar * vNew;

	k = 0;
	while (getNthAttribute(k++,n,v)) {
		// TODO decide if/whether to allow PT_PROPS_ATTRIBUTE_NAME here.
		// TODO The issue is: we use it to store the CSS properties and
		// TODO when we see it, we expand the value into one or more
		// TODO properties.  if we allow it to be given here, should
		// TODO we blowaway all of the existing properties and create
		// TODO them from this?  or should we expand it and override
		// TODO individual properties?
		// TODO for now, we just ignore it.
		if (strcmp(n, PT_PROPS_ATTRIBUTE_NAME) == 0) {
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			continue;
	    }
	    if (!papNew->getAttribute(n,vNew)) {
			bool bres = papNew->setAttribute(n,v);
			if (!bres) {
				return nullptr;
			}
	    }
	}

	// we want to be able to remove all properties by setting the
	// props attribute to ""; in order for that to work, we need to
	// skip the following loop if props is set to ""
	const gchar * szValue;

	if(papNew->getAttribute("props", szValue) && !*szValue) {
		bIgnoreProps = true;
	}

	if (!bClearProps && !bIgnoreProps) {
		k = 0;
		while (getNthProperty(k++,n,v))	{
			if (!papNew->getProperty(n,vNew)) {
				if (!papNew->setProperty(n,v)) {
					return nullptr;
				}
			}
		}
	}

	// the following will remove all properties set to ""; this allows us
	// to remove properties by setting them to ""
	papNew->_clearEmptyProperties();
	papNew->_clearEmptyAttributes();

	return papNew.release();
}

/// This function will remove all properties that are set to ""
void PP_AttrProp::_clearEmptyProperties()
{
	if(m_properties.empty()) {
		return;
	}
	UT_return_if_fail (!m_bIsReadOnly);

	for(auto iter = m_properties.begin(); iter != m_properties.end(); ) {
		if (iter->second.empty()) {
			iter = m_properties.erase(iter);
		} else {
			++iter;
		}
	}
}

/// This method clears both empty attributes and empty properties from
/// [this] AP.
void PP_AttrProp::prune()
{
	_clearEmptyAttributes();
	_clearEmptyProperties();
}

/// This function will remove all attributes that are equal to ""
void PP_AttrProp::_clearEmptyAttributes()
{
	if(m_attributes.empty()) {
		return;
	}
	UT_return_if_fail (!m_bIsReadOnly);

	for (auto iter = m_attributes.begin(); iter != m_attributes.end(); ) {
		if (iter->second.empty()) {
			iter = m_attributes.erase(iter);
		} else {
			++iter;
		}
	}
}

/*! Create a new AttrProp based upon the given one, removing the items given
	 regardless of their value.  See also PP_AttrProp::cloneWithEliminationIfEqual,
	 which does similarly but only removes the items given if their value is equal
	 to the value given.
	 \return NULL on failure, the newly-created PP_AttrProp clone otherwise.
*/
PP_AttrProp * PP_AttrProp::cloneWithElimination(const PP_PropertyVector & attributes,
												const PP_PropertyVector & properties) const
{

	// first, create an empty AttrProp.
	std::unique_ptr<PP_AttrProp> papNew(new PP_AttrProp);

	ASSERT_PV_SIZE(attributes);
	ASSERT_PV_SIZE(properties);

	UT_uint32 k;
	const gchar * n;
	const gchar * v;

	k = 0;
	while (getNthAttribute(k++,n,v))
	{
		// for each attribute in the old set, add it to the
		// new set only if it is not present in the given array.
		for (PP_PropertyVector::const_iterator iter = attributes.begin();
			 iter != attributes.end(); iter += 2) {

			UT_return_val_if_fail (*iter != PT_PROPS_ATTRIBUTE_NAME, NULL); // cannot handle PROPS here
			if (*iter == n) {		// found it, so we don't put it in the result.
				goto DoNotIncludeAttribute;
			}
		}

		// we didn't find it in the given array, add it to the new set.

		if (!papNew->setAttribute(n,v)) {
			return nullptr;
		}

	DoNotIncludeAttribute:
		;
	}

	k = 0;
	while (getNthProperty(k++,n,v))
	{
		// for each property in the old set, add it to the
		// new set only if it is not present in the given array.

		for (auto iter = properties.begin(); iter != properties.end();
			 iter += 2) {
			if (*iter == n) {		// found it, so we don't put it in the result.
				goto DoNotIncludeProperty;
			}
		}

		// we didn't find it in the given array, add it to the new set.

		if (!papNew->setProperty(n,v)) {
			return nullptr;
		}

	DoNotIncludeProperty:
		;
	}

	return papNew.release();
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
PP_AttrProp * PP_AttrProp::cloneWithEliminationIfEqual(const PP_PropertyVector & attributes,
												const PP_PropertyVector & properties) const
{
	// first, create an empty AttrProp.
	std::unique_ptr<PP_AttrProp> papNew(new PP_AttrProp);

	ASSERT_PV_SIZE(attributes);
	ASSERT_PV_SIZE(properties);

	UT_uint32 k;
	const gchar * n;
	const gchar * v;

	k = 0;
	while (getNthAttribute(k++,n,v))
	{
		// for each attribute in the old set, add it to the
		// new set only if it is not present in the given array.

		for (auto iter = attributes.begin(); iter != attributes.end();
			 iter += 2) {
			if (*iter != PT_PROPS_ATTRIBUTE_NAME)
				goto DoNotIncludeAttribute; // cannot handle PROPS here
			// XXX should it be n and v ?
			if (*iter == n && *(iter + 1) == n)		// found it, so we don't put it in the result.
				goto DoNotIncludeAttribute;
		}

		// we didn't find it in the given array, add it to the new set.

		if (!papNew->setAttribute(n,v)) {
			return nullptr;
		}

	DoNotIncludeAttribute:
		;
	}

	k = 0;
	while (getNthProperty(k++,n,v))
	{
		// for each property in the old set, add it to the
		// new set only if it is not present in the given array.

		for (auto iter = properties.begin(); iter != properties.end();
			 iter += 2) {
			// XXX should it be n and v ?
			if (*iter == n && *(iter + 1) == n) {		// found it, so we don't put it in the result.
				goto DoNotIncludeProperty;
			}
		}

		// we didn't find it in the given array, add it to the new set.

		if (!papNew->setProperty(n,v)) {
			return nullptr;
		}

	DoNotIncludeProperty:
		;
	}

	return papNew.release();
}

/*! (?)TODO: PLEASE DOCUMENT ME!
*/
static UT_uint32 hashcodeBytesAP(UT_uint32 init, const char * pv, UT_uint32 cb)
{
 	// modified from ut_string_class.cpp's hashcode() which got it from glib
 	UT_uint32 h = init;
	const unsigned char * pb = reinterpret_cast<const unsigned char *>(pv);

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

	if (m_attributes.empty() && m_properties.empty()) {
		return;
	}

	const gchar *s1, *s2;
	UT_uint32	cch = 0;
	gchar	*rgch;

	rgch = NULL;
	if (!m_attributes.empty()) {
		auto iter = m_attributes.begin();

		while (iter != m_attributes.end()) {
			s1 = iter->first.c_str();
			s2 = iter->second.c_str();

			cch = iter->first.size();

			m_checkSum = hashcodeBytesAP(m_checkSum, s1, cch);

			cch = iter->second.size();

			rgch = g_ascii_strdown(s2, 9);
			rgch[8] = '\0';
			m_checkSum = hashcodeBytesAP(m_checkSum, rgch, cch);
			g_free (rgch); rgch = NULL;

			++iter;
		}
	}

	if (!m_properties.empty()) {
		auto iter = m_properties.begin();

		while (iter != m_properties.end()) {

			s1 = iter->first.c_str();
			cch = iter->first.size();
			rgch = g_ascii_strdown(s1, 9);
			rgch[8] = '\0';
			m_checkSum = hashcodeBytesAP(m_checkSum, rgch, cch);
			g_free (rgch); rgch = NULL;

			s2 = iter->second.c_str();
			cch = iter->second.size();
			rgch = g_ascii_strdown(s2, 9);
			rgch[8] = '\0';
			m_checkSum = hashcodeBytesAP(m_checkSum, rgch, cch);
			g_free (rgch); rgch = NULL;

			++iter;
		}
	}
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
	return m_checkSum;
}

void PP_AttrProp::operator=(const PP_AttrProp &other)
{
	size_t countMyAttrs = other.m_attributes.size();

	UT_uint32 index;
	for(index = 0; index < countMyAttrs; index++)
	{
		const gchar * szName;
		const gchar * szValue;
		if(other.getNthAttribute(index, szName, szValue))
		{
			setAttribute(szName, szValue);
		}
	}

	UT_uint32 countMyProps = other.m_properties.size();

	for(index = 0; index < countMyProps; index++)
	{
		const gchar * szName;
		const gchar * szValue;
		if(other.getNthProperty(index, szName, szValue))
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

	if(getAttributeCount() != pAP2->getAttributeCount()
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
bool PP_AttrProp::isEquivalent(const PP_PropertyVector & attrs,
							   const PP_PropertyVector & props) const
{
	size_t iAttrsCount = attrs.size() / 2;
	size_t iPropsCount = props.size() / 2;

	if(getAttributeCount() != iAttrsCount
	   || getPropertyCount()  != iPropsCount) {
		return false;
	}

	UT_uint32 i;
	std::string name, value;
	const gchar * pValue2;

	for(i =  0; i < getAttributeCount(); ++i)
	{
		name = attrs[2*i];
		value = attrs[2*i + 1];

		if(!getAttribute(name.c_str(), pValue2)) {
			return false;
		}

		// ignore property attribute
		if(value == PT_PROPS_ATTRIBUTE_NAME) {
			continue;
		}

		// handle revision attribute correctly
		if(value == PT_REVISION_ATTRIBUTE_NAME) {
			// requires special treatment
			PP_RevisionAttr r1(value.c_str());
			PP_RevisionAttr r2(pValue2);

			if(!(r1 == r2))	{
				return false;
			}
		} else if(value != pValue2) {
			return false;
		}
	}

	for(i =  0; i < getPropertyCount(); ++i)
	{
		name = props[2 * i];
		value = props[2 * i + 1];

		if(!getProperty(name.c_str(), pValue2)) {
			return false;
		}

		if(value != pValue2) {
			return false;
		}
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

PP_PropertyVector PP_std_setPropsToNothing(const PP_PropertyVector & props)
{
	PP_PropertyVector props2;

	if(props.empty())
		return props2;

	std::size_t i = 0;
	for(auto iter = props.cbegin(); iter != props.cend(); ++iter, ++i)
	{
		if (!(i % 2)) {
			props2.push_back(*iter);
			props2.push_back("");
		}
	}

	return props2;
}

PP_PropertyVector PP_std_copyProps(const gchar ** props)
{
	PP_PropertyVector props2;

	if(!props)
		return props2;

	UT_uint32 i;
	for(i = 0; props[i]; i += 2) {
		props2.push_back(props[i]);
		const char *value = props[i + 1];
		props2.push_back(value ? value : "");
	}

	return props2;
}

PP_PropertyVector PP_cloneAndDecodeAttributes(const gchar ** attrs)
{
	PP_PropertyVector props;
	if (!attrs) {
		return props;
	}

	const gchar** p = attrs;
	while (*p) {
		props.push_back(UT_decodeXML(*p));
		++p;
	}

	if (props.size() % 2) {
		props.push_back("");
	}
	return props;
}

PP_PropertyVector PP_std_setPropsToValue(const PP_PropertyVector & props,
										 const gchar * value)
{
	PP_PropertyVector out;

	std::string svalue = value ? value : "";

	for(auto iter = props.cbegin(); iter != props.cend(); ++iter) {
		out.push_back(*iter);
		++iter;
		if (iter != props.cend()) {
			out.push_back(svalue);
		}
	}

	return out;
}

bool PP_hasAttribute(const char* name, const PP_PropertyVector & atts)
{
	std::size_t i = 0;
	for (auto iter = atts.cbegin(); iter != atts.cend(); ++iter, ++i) {
		if (!(i % 2) && *iter == name) {
			return true;
		}
	}
	return false;
}

/// _EMPTY_STRING. a const we return a reference to.
static const std::string _EMPTY_STRING;

const std::string &
PP_getAttribute(const char* name, const PP_PropertyVector & atts)
{
	std::size_t i = 0;
	for (auto iter = atts.cbegin(); iter != atts.cend(); ++iter, ++i) {
		if (!(i % 2) && *iter == name) {
			++iter;
			++i;
			if (iter != atts.cend()) {
				return *iter;
			}
		}
	}
	return _EMPTY_STRING;
}

bool PP_setAttribute(const char* name, const std::string & value, PP_PropertyVector & atts)
{
	bool changed = false;

	std::size_t i = 0;
	for (auto iter = atts.begin(); iter != atts.end(); ++iter, ++i) {
		if (!(i % 2) && *iter == name) {
			++iter;
			++i;
			if (iter != atts.end()) {
				*iter = value;
				changed = true;
			}
		}
	}
	return changed;
}

void PP_addOrSetAttribute(const char* name, const std::string & value, PP_PropertyVector & atts)
{
	if (!PP_setAttribute(name, value, atts)) {
		std::size_t size = atts.size();
		// make sure the atts size is even.
		if (size && (size % 2)) {
			atts.resize(size - 1);
		}
		atts.push_back(name);
		atts.push_back(value);
	}
}

bool PP_removeAttribute(const char* name, PP_PropertyVector & atts)
{
	std::size_t i = 0;
	for (auto iter = atts.cbegin(); iter != atts.cend(); ++iter, ++i) {
		if (!(i % 2) && *iter == name) {
			iter = atts.erase(iter);
			if (iter != atts.cend()) {
				atts.erase(iter);
			}
			return true;
		}
	}
	return false;
}
