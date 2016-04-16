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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


#ifndef PD_STYLE_H
#define PD_STYLE_H

#include <memory>

#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "ut_xml.h"
#include "pp_Property.h"

class pt_PieceTable;

//////////////////////////////////////////////////////////////////
// PD_Style is the representation for an individual style.

class ABI_EXPORT PD_Style
{
public:
	PD_Style(pt_PieceTable * pPT, PT_AttrPropIndex indexAP, const char * szName = NULL, bool bDisplayed = true);
	virtual ~PD_Style();

	inline PT_AttrPropIndex		getIndexAP(void) const	{ return m_indexAP; };
	bool						setIndexAP(PT_AttrPropIndex indexAP);

	bool					getProperty(const gchar * szName, const gchar *& szValue) const;
	std::unique_ptr<PP_PropertyType>	getPropertyType(const gchar * szName, tProperty_type Type) const;
	bool					getAttribute(const gchar * szName, const gchar *& szValue) const;
	bool					getPropertyExpand(const gchar * szName, const gchar *& szValue) const;
	bool					getAttributeExpand(const gchar * szName, const gchar *& szValue) const;

	PD_Style *				getBasedOn(void) const;
	PD_Style *				getFollowedBy(void) const;

	virtual bool			isUserDefined(void) const { return true; };
	void					used(UT_sint32 count);
	bool					isUsed(void) const;
	bool					isCharStyle(void) const;
	bool					isList(void) const;
	bool					isDisplayed(void) const { return m_bDisplayed; }

	bool					addProperty(const gchar * szName, const gchar * szValue);
	bool					addProperties(const gchar ** pProperties);
	bool					setAllAttributes(const gchar ** pAtts);
	bool					addAttributes(const gchar ** pAtts);
	bool                    getAllProperties( UT_Vector * vProps, UT_sint32 depth) const;
	bool                    getAllAttributes( UT_Vector * vAttribs, UT_sint32 depth) const;
	size_t getPropertyCount(void) const;
	size_t getAttributeCount(void) const;
	bool getNthProperty (int ndx, const gchar *&szName,
			     const gchar *&szValue) const;
	bool getNthAttribute (int ndx, const gchar *&szName,
			     const gchar *&szValue) const;

	inline const char * getName (void) const {return m_szName;}

protected:
	bool					_getPropertyExpand(const gchar * szName, const gchar *& szValue, UT_sint32 iDepth) const;
	bool					_getAttributeExpand(const gchar * szName, const gchar *& szValue, UT_sint32 iDepth) const;

	pt_PieceTable *			m_pPT;
	PT_AttrPropIndex		m_indexAP;

	char * 					m_szName;
	bool					m_bDisplayed;
	UT_sint32				m_iUsed;

	// lazily-bound attribute caches to speed lookups
	// hence their mutability
	mutable PD_Style *			m_pBasedOn;
	mutable PD_Style *			m_pFollowedBy;
};


//////////////////////////////////////////////////////////////////
// PD_BuiltinStyle

class ABI_EXPORT PD_BuiltinStyle : public PD_Style
{
public:
	PD_BuiltinStyle(pt_PieceTable * pPT, PT_AttrPropIndex indexAP, const char * szName, bool bDisplayed);
	virtual ~PD_BuiltinStyle();

	virtual bool			isUserDefined(void) const { return (m_indexAP != m_indexAPOrig); };

protected:
	PT_AttrPropIndex		m_indexAPOrig;	// the builtin one
};

#endif /* PD_STYLE_H */
