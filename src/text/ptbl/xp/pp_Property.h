/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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



#ifndef PP_PROPERTY_H
#define PP_PROPERTY_H

#include <string>
#include <vector>

// make sure we don't get caught in a BASEDON loop
#define pp_BASEDON_DEPTH_LIMIT	10

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_units.h"

#include "pp_PropertyMap.h"

class PP_AttrProp;
class PD_Document;

/// A basic container of properties vector.
/// This replaces the gchar**
typedef std::vector<std::string> PP_PropertyVector;

/// An empty immutable poperty vector
extern const PP_PropertyVector PP_NOPROPS;

// PP_Property captures knowledge of the various CSS properties,
// such as their initial/default values and whether they are
// inherited.

typedef enum {
	Property_type_bool,
	Property_type_int,
	Property_type_size,
	Property_type_color
	} tProperty_type;


class ABI_EXPORT PP_PropertyType
{
public:
	PP_PropertyType() {};
	virtual ~PP_PropertyType() {};

public:
	virtual tProperty_type getType() const = 0;

	static PP_PropertyType *createPropertyType(tProperty_type Type, const gchar *p_init);
};

class ABI_EXPORT PP_PropertyTypeBool : public PP_PropertyType
{

public:
	PP_PropertyTypeBool(const gchar *p_init);

	tProperty_type getType() const {return Property_type_bool;}

	bool getState() const {return State;}


private:

	bool State;
};

class ABI_EXPORT PP_PropertyTypeInt : public PP_PropertyType
{

public:
	PP_PropertyTypeInt(const gchar *p_init);

	tProperty_type getType() const {return Property_type_int;}

	int getValue() const {return Value;}


private:

	int Value;
};

class ABI_EXPORT PP_PropertyTypeSize : public PP_PropertyType
{

public:
	PP_PropertyTypeSize(const gchar *p_init);

	tProperty_type getType() const {return Property_type_size;}

	double getValue() const {return Value;}
	UT_Dimension getDim() const {return Dim;}


private:

	double Value;
	UT_Dimension Dim;
};

class ABI_EXPORT PP_PropertyTypeColor : public PP_PropertyType
{

public:
	PP_PropertyTypeColor(const gchar *p_init);

	tProperty_type getType() const {return Property_type_color;}

	const UT_RGBColor &getColor() const {return Color;}


private:

	UT_RGBColor Color;
};



typedef unsigned int tPropLevel;

// the m_iLevel member of PP_Property should be set by or-ing the
// following constants
#define PP_LEVEL_CHAR  0x00000001
#define PP_LEVEL_BLOCK 0x00000002
#define PP_LEVEL_SECT  0x00000004
#define PP_LEVEL_DOC   0x00000008
#define PP_LEVEL_TABLE 0x00000010
#define PP_LEVEL_OBJ   0x00000020
#define PP_LEVEL_IMG   0x00000040
#define PP_LEVEL_FIELD 0x00000080
#define PP_LEVEL_FRAME 0x00000100

class ABI_EXPORT PP_Property
{
public:
	const gchar *			m_pszName;
	const gchar *			m_pszInitial;
	bool				m_bInherit;
	PP_PropertyType *	m_pProperty;
	tPropLevel          m_iLevel;
	~PP_Property();

	inline const gchar *	getName() const {return m_pszName;}
	inline const gchar *	getInitial() const {return m_pszInitial;}
	const PP_PropertyType *	getInitialType (tProperty_type Type) const;
	inline bool				canInherit() const {return m_bInherit;}
	inline tPropLevel       getLevel() const {return m_iLevel;}
};

ABI_EXPORT const PP_Property * PP_lookupProperty(const gchar * pszName);

ABI_EXPORT void PP_resetInitialBiDiValues(const gchar * pszValue);

ABI_EXPORT void PP_setDefaultFontFamily(const char* pszFamily);

ABI_EXPORT const gchar * PP_evalProperty(const gchar * pszName,
								 const PP_AttrProp * pSpanAttrProp,
								 const PP_AttrProp * pBlockAttrProp,
								 const PP_AttrProp * pSectionAttrProp,
								 const PD_Document * pDoc,
								 bool bExpandStyles=false);


ABI_EXPORT const PP_PropertyType * PP_evalPropertyType(const gchar * pszName,
								 const PP_AttrProp * pSpanAttrProp,
								 const PP_AttrProp * pBlockAttrProp,
								 const PP_AttrProp * pSectionAttrProp,
								 tProperty_type Type,
								 const PD_Document * pDoc=NULL,
								 bool bExpandStyles=false);

ABI_EXPORT UT_uint32        PP_getPropertyCount();
ABI_EXPORT const gchar * PP_getNthPropertyName(UT_uint32 n);
ABI_EXPORT tPropLevel       PP_getNthPropertyLevel(UT_uint32 n);
#endif /* PP_PROPERTY_H */
