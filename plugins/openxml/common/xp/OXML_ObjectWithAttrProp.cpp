/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 * 
 * Copyright (C) 2007 Philippe Milot <PhilMilot@gmail.com>
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

// Class definition include
#include "OXML_ObjectWithAttrProp.h"

OXML_ObjectWithAttrProp::OXML_ObjectWithAttrProp() : 
	m_pAttributes(new PP_AttrProp())
{
}

OXML_ObjectWithAttrProp::~OXML_ObjectWithAttrProp()
{
	DELETEP(m_pAttributes);
}


UT_Error OXML_ObjectWithAttrProp::setAttribute(const gchar * szName, const gchar * szValue)
{
	return m_pAttributes->setAttribute(szName, szValue) ? UT_OK : UT_ERROR;
}

UT_Error OXML_ObjectWithAttrProp::setProperty(const gchar * szName, const gchar * szValue)
{
	return m_pAttributes->setProperty(szName, szValue) ? UT_OK : UT_ERROR;
}

UT_Error OXML_ObjectWithAttrProp::setProperty(const std::string & szName, const std::string & szValue)
{
	return setProperty(szName.c_str(), szValue.c_str());
}

UT_Error OXML_ObjectWithAttrProp::getAttribute(const gchar * szName, const gchar *& szValue) const
{
    szValue = NULL;
	UT_return_val_if_fail(szName && *szName, UT_ERROR);
	if(!m_pAttributes)
		return UT_ERROR;

	UT_Error ret;
	ret = m_pAttributes->getAttribute(szName, szValue) ? UT_OK : UT_ERROR;
	if(ret != UT_OK)
		return ret;

	return (szValue && *szValue) ? UT_OK : UT_ERROR;
}

UT_Error OXML_ObjectWithAttrProp::getProperty(const gchar * szName, const gchar *& szValue) const
{
    szValue = NULL;
	UT_return_val_if_fail(szName && *szName, UT_ERROR);
	if(!m_pAttributes)
		return UT_ERROR;

	UT_Error ret;
	ret = m_pAttributes->getProperty(szName, szValue) ? UT_OK : UT_ERROR;
	if(ret != UT_OK)
		return ret;

	return (szValue && *szValue) ? UT_OK : UT_ERROR;
}

UT_Error OXML_ObjectWithAttrProp::setAttributes(const gchar ** attributes)
{
	return m_pAttributes->setAttributes(attributes) ? UT_OK : UT_ERROR;
}

UT_Error OXML_ObjectWithAttrProp::setProperties(const gchar ** properties)
{
	return m_pAttributes->setProperties(properties) ? UT_OK : UT_ERROR;
}

UT_Error OXML_ObjectWithAttrProp::appendAttributes(const gchar ** attributes)
{
	UT_return_val_if_fail(attributes != NULL, UT_ERROR);
	UT_Error ret;
	for (UT_uint32 i = 0; attributes[i] != NULL; i += 2) {
		ret = setAttribute(attributes[i], attributes[i+1]);
		if (ret != UT_OK) return ret;
	}
	return UT_OK;
}

UT_Error OXML_ObjectWithAttrProp::appendProperties(const gchar ** properties)
{
	UT_return_val_if_fail(properties != NULL, UT_ERROR);
	UT_Error ret;
	for (UT_uint32 i = 0; properties[i] != NULL; i += 2) {
		ret = setProperty(properties[i], properties[i+1]);
		if (ret != UT_OK) return ret;
	}
	return UT_OK;
}

const gchar ** OXML_ObjectWithAttrProp::getAttributes() const
{
	return m_pAttributes->getAttributes();
}

const gchar ** OXML_ObjectWithAttrProp::getProperties() const
{
	return m_pAttributes->getProperties();
}

const gchar ** OXML_ObjectWithAttrProp::getAttributesWithProps()
{
	std::string propstring = _generatePropsString();
	if (propstring.empty())
        return getAttributes();

	// Use fakeprops here to avoid overwriting props attribute if already exists
	UT_return_val_if_fail(UT_OK == setAttribute("fakeprops", propstring.c_str()), NULL);
	const gchar ** atts = getAttributes();
	for (UT_uint32 i = 0; atts && (atts[i] != NULL); i += 2) {
		if (!strcmp(atts[i], "fakeprops"))
			atts[i] = PT_PROPS_ATTRIBUTE_NAME;
	}
	return atts;
}

std::string OXML_ObjectWithAttrProp::_generatePropsString() const
{
	const gchar ** props = getProperties();
	if (props == NULL) 
        return "";
	std::string fmt_props;

	for (UT_uint32 i = 0; props[i] != NULL; i += 2) {
		fmt_props += props[i];
		fmt_props += ":";
		fmt_props += props[i+1];
		fmt_props += ";";
	}
	fmt_props.resize(fmt_props.length() - 1); //Shave off the last semicolon, appendFmt doesn't like it
	return fmt_props;
}

UT_Error OXML_ObjectWithAttrProp::inheritProperties(OXML_ObjectWithAttrProp* parent)
{
	if(!parent)
		return UT_ERROR;

	UT_Error ret = UT_OK;

	size_t numProps = parent->getPropertyCount();

	const gchar* szName;
	const gchar* szValue;

	for (size_t i = 0; i<numProps; i++) {
		
		if(!parent->getNthProperty(i, szName, szValue))
			break;

		const gchar * prop = NULL;
		if((getProperty(szName, prop) != UT_OK) || !prop)
		{
			ret = setProperty(szName, szValue);		
			if(ret != UT_OK)
				return ret;
		}
	}

	return ret;
}

size_t OXML_ObjectWithAttrProp::getPropertyCount()
{
	return m_pAttributes->getPropertyCount();
}

bool OXML_ObjectWithAttrProp::getNthProperty(int i, const gchar* & szName, const gchar* & szValue)
{
	return m_pAttributes->getNthProperty(i, szName, szValue);
}
