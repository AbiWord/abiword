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



#ifndef PP_PROPERTY_H
#define PP_PROPERTY_H

// make sure we don't get caught in a BASEDON loop
#define pp_BASEDON_DEPTH_LIMIT	10

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_units.h"
class PP_AttrProp;
class PD_Document;

// PP_Property captures knowledge of the various CSS properties,
// such as their initial/default values and whether they are
// inherited.

typedef enum {
	Property_type_bool,
	Property_type_int,
	Property_type_size,
	Property_type_color
	} tProperty_type;


class PP_PropertyType
{
public:
	PP_PropertyType() {};
	virtual ~PP_PropertyType() {};

public:
	virtual tProperty_type getType() const = 0;

	static PP_PropertyType *createPropertyType(tProperty_type Type, const XML_Char *p_init);
};

class PP_PropertyTypeBool : public PP_PropertyType
{

public:
	PP_PropertyTypeBool(const XML_Char *p_init);

	tProperty_type getType() const {return Property_type_bool;}

	bool getState() const {return State;}


private:

	bool State;
};

class PP_PropertyTypeInt : public PP_PropertyType
{

public:
	PP_PropertyTypeInt(const XML_Char *p_init);

	tProperty_type getType() const {return Property_type_int;}

	int getValue() const {return Value;}


private:

	int Value;
};

class PP_PropertyTypeSize : public PP_PropertyType
{

public:
	PP_PropertyTypeSize(const XML_Char *p_init);

	tProperty_type getType() const {return Property_type_size;}

	double getValue() const {return Value;}
	UT_Dimension getDim() const {return Dim;}


private:

	double Value;
	UT_Dimension Dim;
};

class PP_PropertyTypeColor : public PP_PropertyType
{

public:
	PP_PropertyTypeColor(const XML_Char *p_init);

	tProperty_type getType() const {return Property_type_color;}

	const UT_RGBColor &getColor() const {return Color;}


private:

	UT_RGBColor Color;
};

	




class PP_Property
{
public:
	XML_Char *			m_pszName;
	XML_Char *			m_pszInitial;
	bool				m_bInherit;
	PP_PropertyType *	m_pProperty;

	inline const XML_Char *	getName() const {return m_pszName;}
	inline const XML_Char *	getInitial() const {return m_pszInitial;}
	const PP_PropertyType *	getInitialType(tProperty_type Type) const;
	inline bool				canInherit() const {return m_bInherit;}
};

const PP_Property * PP_lookupProperty(const XML_Char * pszName);

#ifdef BIDI_ENABLED
void PP_resetInitialBiDiValues(const XML_Char * pszValue);
#endif

const XML_Char * PP_evalProperty(const XML_Char * pszName,
								 const PP_AttrProp * pSpanAttrProp,
								 const PP_AttrProp * pBlockAttrProp,
								 const PP_AttrProp * pSectionAttrProp,
								 PD_Document * pDoc=NULL,
								 bool bExpandStyles=false);


const PP_PropertyType * PP_evalPropertyType(const XML_Char * pszName,
								 const PP_AttrProp * pSpanAttrProp,
								 const PP_AttrProp * pBlockAttrProp,
								 const PP_AttrProp * pSectionAttrProp,
								 tProperty_type Type,
								 PD_Document * pDoc=NULL,
								 bool bExpandStyles=false);

#endif /* PP_PROPERTY_H */
