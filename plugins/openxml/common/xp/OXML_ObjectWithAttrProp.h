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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef _OXML_OBJECTWITHATTRPROP_H_
#define _OXML_OBJECTWITHATTRPROP_H_

// Internal includes
#include "OXML_Types.h"

// AbiWord includes
#include <pp_AttrProp.h>

#include <string>

class OXML_ObjectWithAttrProp {
public:
	OXML_ObjectWithAttrProp();
	virtual ~OXML_ObjectWithAttrProp();

	UT_Error setAttribute(const gchar * szName, const gchar * szValue);
	UT_Error setProperty(const gchar * szName, const gchar * szValue);
	UT_Error setProperty(const std::string & szName, const std::string & szValue);
	UT_Error getAttribute(const gchar * szName, const gchar *& szValue) const;
	UT_Error getProperty(const gchar * szName, const gchar *& szValue) const;
	UT_Error setAttributes(const gchar ** attributes);
	UT_Error setProperties(const gchar ** properties);
	UT_Error appendAttributes(const gchar ** attributes);
	UT_Error appendProperties(const gchar ** properties);
	const gchar ** getAttributes() const;
	const gchar ** getProperties() const;

	UT_Error inheritProperties(OXML_ObjectWithAttrProp* parent);

	//! Provides the list of attributes including all the properties in one attribute.
	/*! This method takes all the properties of the object and combines them into one string in CSS
 	 *  format.  This string is used as the value of a new attribute whose key is defined by PP_PROPS_ATTRIBUTE_NAME.
	 	\return A list of all the object's attributes and with a new attribute containing all the properties.
	*/
	const gchar ** getAttributesWithProps();

	bool getNthProperty(int i, const gchar* & szName, const gchar* & szValue);
	size_t getPropertyCount();

private:
	PP_AttrProp* m_pAttributes;

	std::string _generatePropsString() const;
};

#endif //_OXML_OBJECTWITHATTRPROP_H_

