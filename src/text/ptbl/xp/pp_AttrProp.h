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
#include "ut_xml.h"
#include "pp_Property.h"


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
	
	bool	setAttributes(const XML_Char ** attributes);
	bool setAttributes(const UT_Vector * pVector);
	bool	setProperties(const XML_Char ** properties);

	bool	setAttribute(const XML_Char * szName, const XML_Char * szValue);
	bool	setProperty(const XML_Char * szName, const XML_Char * szValue);

	bool	getNthAttribute(int ndx, const XML_Char *& szName, const XML_Char *& szValue) const;
	bool	getNthProperty(int ndx, const XML_Char *& szName, const XML_Char *& szValue) const;

	bool getAttribute(const XML_Char * szName, const XML_Char *& szValue) const;
	bool getProperty(const XML_Char * szName, const XML_Char *& szValue) const;
	const PP_PropertyType *getPropertyType(const XML_Char * szName, tProperty_type Type) const;

	bool hasProperties(void) const;
	size_t getPropertyCount (void) const;
	bool areAlreadyPresent(const XML_Char ** attributes, const XML_Char ** properties) const;
	bool areAnyOfTheseNamesPresent(const XML_Char ** attributes, const XML_Char ** properties) const;
	bool isExactMatch(const PP_AttrProp * pMatch) const;

	PP_AttrProp * cloneWithReplacements(const XML_Char ** attributes,
										const XML_Char ** properties,
										bool bClearProps) const;
	PP_AttrProp * cloneWithElimination(const XML_Char ** attributes,
									   const XML_Char ** properties) const;
	void markReadOnly(void);
	UT_uint32 getCheckSum(void) const;

	void operator = (const PP_AttrProp &Other);

protected:
	void _computeCheckSum(void);
	
	UT_StringPtrMap * m_pAttributes;
	UT_StringPtrMap * m_pProperties;
	bool				m_bIsReadOnly;
	UT_uint32			m_checkSum;
};

#endif /* PP_ATTRPROP_H */
