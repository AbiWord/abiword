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


#ifndef PD_STYLE_H
#define PD_STYLE_H

#include "ut_types.h"
#include "pt_Types.h"
#include "ut_xml.h"
#include "pp_Property.h"

class pt_PieceTable;

//////////////////////////////////////////////////////////////////
// PD_Style is the representation for an individual style.

class ABI_EXPORT PD_Style
{
public:
	PD_Style(pt_PieceTable * pPT, PT_AttrPropIndex indexAP, const char * szName = NULL);
	virtual ~PD_Style();

	inline PT_AttrPropIndex		getIndexAP(void) const	{ return m_indexAP; };
	bool						setIndexAP(PT_AttrPropIndex indexAP);

	bool					getProperty(const XML_Char * szName, const XML_Char *& szValue) const;
	const PP_PropertyType *	getPropertyType(const XML_Char * szName, tProperty_type Type) const;
	bool					getAttribute(const XML_Char * szName, const XML_Char *& szValue) const;
	bool					getPropertyExpand(const XML_Char * szName, const XML_Char *& szValue);
	bool					getAttributeExpand(const XML_Char * szName, const XML_Char *& szValue);
	
	PD_Style *				getBasedOn(void);
	PD_Style *				getFollowedBy(void);

	virtual bool			isUserDefined(void) const { return true; };
	void					used(UT_sint32 count);
	bool					isUsed(void) const;
	bool					isCharStyle(void) const;
	bool					isList(void);
	
	bool					addProperty(const XML_Char * szName, const XML_Char * szValue);
	bool					addProperties(const XML_Char ** pProperties);
	bool					setAllAttributes(const XML_Char ** pAtts);
	bool					addAttributes(const XML_Char ** pAtts);
	bool                    getAllProperties( UT_Vector * vProps, UT_sint32 depth);
	bool                    getAllAttributes( UT_Vector * vAttribs, UT_sint32 depth);
	size_t getPropertyCount(void) const;
	size_t getAttributeCount(void) const;
	bool getNthProperty (int ndx, const XML_Char *&szName,
			     const XML_Char *&szValue) const;
	bool getNthAttribute (int ndx, const XML_Char *&szName,
			     const XML_Char *&szValue) const;

	inline const char * getName (void) const {return m_szName;}

protected:
	bool					_getPropertyExpand(const XML_Char * szName, const XML_Char *& szValue, UT_sint32 iDepth);
	bool					_getAttributeExpand(const XML_Char * szName, const XML_Char *& szValue, UT_sint32 iDepth);

	pt_PieceTable *			m_pPT;
	PT_AttrPropIndex		m_indexAP;

	char * 					m_szName;
	UT_sint32				m_iUsed;

	// lazily-bound attribute caches to speed lookups
	PD_Style *				m_pBasedOn;
	PD_Style *				m_pFollowedBy;

private:
	UT_sint32				m_iIsList;
};


//////////////////////////////////////////////////////////////////
// PD_BuiltinStyle 

class ABI_EXPORT PD_BuiltinStyle : public PD_Style
{
public:
	PD_BuiltinStyle(pt_PieceTable * pPT, PT_AttrPropIndex indexAP, const char * szName);
	virtual ~PD_BuiltinStyle();

	virtual bool			isUserDefined(void) const { return (m_indexAP != m_indexAPOrig); };

protected:
	PT_AttrPropIndex		m_indexAPOrig;	// the builtin one
};

#endif /* PD_STYLE_H */
