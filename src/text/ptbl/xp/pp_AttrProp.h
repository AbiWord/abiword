/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
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
#include "ut_pair.h"
#include "ut_xml.h"
#include "pp_Property.h"
#include "pt_Types.h"

#include <map>

#include <glib.h>

using std::map;

class PD_Document;

// the following class holds the state of the view when we set
// m_iRevisionIndex
class ABI_EXPORT PP_RevisionState
{
  public:
	PP_RevisionState(){m_iId = 0; m_bShow = false; m_bMark = false;};

	bool operator == (const PP_RevisionState & rs) const
	     {
			 if(rs.m_iId == m_iId && rs.m_bShow == m_bShow && rs.m_bMark == m_bMark)
				 return true;
			 return false;
		 }

	bool isEqual(UT_uint32 iId, bool bShow, bool bMark) const
	     {
			 if(m_iId == iId && m_bShow == bShow && m_bMark == bMark)
				 return true;
			 return false;
		 }
	
	UT_uint32 m_iId;
	bool      m_bShow;
	bool      m_bMark;
};


// PP_AttrProp captures the complete set of XML and CSS
// Attributes/Properties for a piece of the document.
// These are generally created by the file-reader.  Attributes
// represent all of the attribute/value pairs in the XML with
// the exception of the PT_PROPS_ATTRIBUTE_NAME attribute.
// The value of the this attribute is parsed into CSS properties.
// PP_AttrProp just provides a pair of association lists
// (one for attributes and one for properties), it does not
// know the meaning of any of them.

class ABI_EXPORT PP_AttrProp
{
public:
	class PropertyPair
	{
	  public:
		
		GQuark            first;
		PP_PropertyType * second;

		PropertyPair ():first(0), second(NULL){}
		PropertyPair (GQuark value): first(value), second(NULL){}
		~PropertyPair () { delete second;}
	};
	

	typedef map <PT_Property,  PropertyPair> AP_Props;
	typedef map <PT_Attribute, GQuark>      AP_Attrs;

	PP_AttrProp();
	virtual ~PP_AttrProp();

	bool	setAttributes(const PT_AttributePair   * attributes);
	bool    setAttributes(const PT_AttributeVector & vAttrs);
	bool    setAttributes(const AP_Attrs * pAttrs);
	
	bool	setProperties(const PT_PropertyPair    * properties);
	bool	setProperties(const PT_PropertyVector  & vProps);
	bool    setProperties(const AP_Props * pProps);
	
	bool	setAttribute(PT_Attribute a, GQuark value );
	bool	setProperty (PT_Property  p, GQuark value);

	// return true if the loop should continue, false otherwise
	typedef bool (*PP_ForEachAttrFunc) (const PT_Attribute & a,
										const GQuark & value,
										UT_uint32 index,
										void * data);
	
	typedef bool (*PP_ForEachPropFunc) (const PT_Property  & p,
										const GQuark & value,
										UT_uint32 index,
										void * data);
	
	void    forEachAttribute (PP_ForEachAttrFunc f, void * data) const;
	void    forEachProperty  (PP_ForEachPropFunc f, void * data) const;

	bool    getAttribute(PT_Attribute a,
						 GQuark & value) const;
	const   AP_Attrs * getAttributes() const {return m_pAttributes;}
	
	bool    getProperty(PT_Property p,
						GQuark & value) const;
	const   AP_Props * getProperties() const {return m_pProperties;}
	
	const PP_PropertyType *getPropertyType(PT_Property p,
										   tProperty_type Type) const;

	bool    hasProperties(void) const;
	bool    hasAttributes(void) const;
	size_t  getPropertyCount (void) const;
	size_t  getAttributeCount (void) const;
	
	bool    areAlreadyPresent(const PT_AttributePair * attributes,
							  const PT_PropertyPair  * properties) const;
	
	bool    areAnyOfTheseNamesPresent(const PT_AttributePair * attributes,
									  const PT_PropertyPair * properties) const;
	
	bool    isExactMatch(const PP_AttrProp * pMatch) const;
	bool    isEquivalent(const PP_AttrProp * pAP2) const;
	bool    isEquivalent(const PT_AttributePair * attrs,
						 const PT_PropertyPair  * props) const;
	
	PP_AttrProp * createExactly(const PT_AttributePair * attrs,
								const PT_PropertyPair  * props) const;

	PP_AttrProp * cloneWithReplacements(const PT_AttributePair * attrs,
										const PT_PropertyPair  * props,
										bool bClearProps) const;
	
	PP_AttrProp * cloneWithElimination(const PT_AttributePair * attrs,
									   const PT_PropertyPair  * props) const;
	
	PP_AttrProp * cloneWithEliminationIfEqual(const PT_AttributePair * attrs,
											  const PT_PropertyPair  * props) const;

	void    markReadOnly(void);
	UT_uint32 getCheckSum(void) const;

	void operator = (const PP_AttrProp &Other);
	UT_uint32 getIndex(void);	//$HACK
	void setIndex(UT_uint32 i);	//$HACK

	/* m_iRevisionIndex points to a AP that has our revision attribute
	   inflated into actual attributes and properties; since the
	   inflation is view-dependent, we need to keep track of the view
	   state */
	PT_AttrPropIndex   getRevisedIndex() const {return m_iRevisedIndex;}
	PP_RevisionState & getRevisionState() const {return m_RevisionState;}
	
	void               setRevisedIndex (PT_AttrPropIndex i,
										UT_uint32 iId,
										bool bShow,
										bool bMark,
										bool bHidden) const
	                       {
							   m_iRevisedIndex = i; m_RevisionState.m_iId = iId;
							   m_RevisionState.m_bShow = bShow; m_RevisionState.m_bMark = bMark;
							   m_bRevisionHidden = bHidden;
						   }

	bool               getRevisionHidden() const {return m_bRevisionHidden;}

	void prune();
	bool explodeStyle(const PD_Document * pDoc, bool bOverwrite = false);

	void miniDump(const PD_Document * pDoc) const;
	
protected:
	void _computeCheckSum(void);
	void _clearEmptyProperties();
	void _clearEmptyAttributes();

	AP_Attrs * m_pAttributes;
	AP_Props * m_pProperties;
	
	bool				        m_bIsReadOnly;
	UT_uint32			        m_checkSum;
	UT_uint32			        m_index;	//$HACK
	
	mutable PT_AttrPropIndex    m_iRevisedIndex;
	mutable PP_RevisionState    m_RevisionState;
	mutable bool                m_bRevisionHidden;
};

#endif /* PP_ATTRPROP_H */
