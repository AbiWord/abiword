
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

protected:
	UT_HashTable * m_pAttributes;
	UT_HashTable * m_pProperties;
};

#endif /* PP_ATTRPROP_H */
