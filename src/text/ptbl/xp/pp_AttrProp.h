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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


#ifndef PP_ATTRPROP_H
#define PP_ATTRPROP_H

#include <utility>

#include "ut_types.h"
#include "ut_hash.h"
#include "ut_vector.h"
#include "ut_xml.h"
#include "pp_Property.h"
#include "pt_Types.h"

class PD_Document;

// the following class holds the state of the view when we set
// m_iRevisionIndex
class ABI_EXPORT PP_RevisionState
{
  public:
	PP_RevisionState()
		:m_iId(0),
		m_bShow(false),
		m_bMark(false)
	{
	}

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
	PP_AttrProp();
	virtual ~PP_AttrProp();

	// The "gchar **" is an argv-like thing containing
	// multiple sets of name/value pairs.  names are in the
	// even cells; values are in the odd.  the list is
	// terminated by a null name.

	bool	setAttributes(const gchar ** attributes);
	bool    setAttributes(const UT_GenericVector<const gchar*> * pVector);
	bool	setProperties(const gchar ** properties);
	bool	setProperties(const UT_GenericVector<gchar*> * pVector);

	const gchar ** getAttributes () const { return m_pAttributes ? m_pAttributes->list () : 0; }
	const gchar ** getProperties () const;

	bool	setAttribute(const gchar * szName, const gchar * szValue);
	bool	setProperty(const gchar * szName, const gchar * szValue);

	bool	getNthAttribute(int ndx, const gchar *& szName, const gchar *& szValue) const;
	bool	getNthProperty(int ndx, const gchar *& szName, const gchar *& szValue) const;

	bool getAttribute(const gchar * szName, const gchar *& szValue) const;
	bool getProperty(const gchar * szName, const gchar *& szValue) const;
	const PP_PropertyType *getPropertyType(const gchar * szName, tProperty_type Type) const;

	bool hasProperties(void) const;
	bool hasAttributes(void) const;
	size_t getPropertyCount (void) const;
	size_t getAttributeCount (void) const;
	bool areAlreadyPresent(const gchar ** attributes, const gchar ** properties) const;
	bool areAnyOfTheseNamesPresent(const gchar ** attributes, const gchar ** properties) const;
	bool isExactMatch(const PP_AttrProp * pMatch) const;
	bool isEquivalent(const PP_AttrProp * pAP2) const;
	bool isEquivalent(const gchar ** attrs, const gchar ** props) const;

	PP_AttrProp * createExactly(const gchar ** attributes,
				    const gchar ** properties) const;

	PP_AttrProp * cloneWithReplacements(const gchar ** attributes,
										const gchar ** properties,
										bool bClearProps) const;
	PP_AttrProp * cloneWithElimination(const gchar ** attributes,
									   const gchar ** properties) const;
	PP_AttrProp * cloneWithEliminationIfEqual(const gchar ** attributes,
									   const gchar ** properties) const;

	void markReadOnly(void);
	UT_uint32 getCheckSum(void) const;

	void operator = (const PP_AttrProp &Other);
	UT_uint32 getIndex(void) const;	//$HACK
	void setIndex(UT_uint32 i);	//$HACK

	/* m_iRevisionIndex points to a AP that has our revision attribute
	   inflated into actual attributes and properties; since the
	   inflation is view-dependent, we need to keep track of the view
	   state */
	PT_AttrPropIndex   getRevisedIndex() const {return m_iRevisedIndex;}
	PP_RevisionState & getRevisionState() const {return m_RevisionState;}

	void               setRevisedIndex (PT_AttrPropIndex i, UT_uint32 iId, bool bShow, bool bMark, bool bHidden) const
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


	typedef std::pair<const gchar*,const PP_PropertyType *> PropertyPair;

	UT_GenericStringMap<gchar*> * m_pAttributes; // of gchar*
	UT_GenericStringMap<PropertyPair*> * m_pProperties; // of PropertyPair

	bool				m_bIsReadOnly;
	UT_uint32			m_checkSum;
	UT_uint32			m_index;	//$HACK
	mutable const gchar **   m_szProperties;

	mutable PT_AttrPropIndex    m_iRevisedIndex;
	mutable PP_RevisionState    m_RevisionState;
	mutable bool                m_bRevisionHidden;
};

#endif /* PP_ATTRPROP_H */
