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


#ifndef PP_ATTRPROP_H
#define PP_ATTRPROP_H

#include "ut_types.h"
#include "ut_hash.h"
#include "ut_vector.h"
#include "xmlparse.h"

// PP_AttrProp captures the complete set of XML and CSS
// Attributes/Properties for a piece of the document.
// These are generally created by the file-reader.  Attributes
// represent all of the attribute/value pairs in the XML with
// the exception of the PT_PROPS_ATTRIBUTE_NAME attribute.
// The value of the this attribute is parsed into CSS properties.
// PP_AttrProp just provides a pair of association lists
// (one for attributes and one for properties), it does not
// know the meaning of any of them.

class PP_AttrProp
{
public:
	PP_AttrProp();
	~PP_AttrProp();

	// The "XML_Char **" is an argv-like thing containing
	// multiple sets of name/value pairs.  names are in the
	// even cells; values are in the odd.  the list is
	// terminated by a null name.
	
	UT_Bool	setAttributes(const XML_Char ** attributes);
	UT_Bool setAttributes(const UT_Vector * pVector);
	UT_Bool	setProperties(const XML_Char ** properties);

	UT_Bool	setAttribute(const XML_Char * szName, const XML_Char * szValue);
	UT_Bool	setProperty(const XML_Char * szName, const XML_Char * szValue);

	UT_Bool	getNthAttribute(int ndx, const XML_Char *& szName, const XML_Char *& szValue) const;
	UT_Bool	getNthProperty(int ndx, const XML_Char *& szName, const XML_Char *& szValue) const;

	UT_Bool getAttribute(const XML_Char * szName, const XML_Char *& szValue) const;
	UT_Bool getProperty(const XML_Char * szName, const XML_Char *& szValue) const;

	UT_Bool areAlreadyPresent(const XML_Char ** attributes, const XML_Char ** properties) const;
	UT_Bool areAnyOfTheseNamesPresent(const XML_Char ** attributes, const XML_Char ** properties) const;

protected:
	UT_HashTable * m_pAttributes;
	UT_HashTable * m_pProperties;
};

#endif /* PP_ATTRPROP_H */
