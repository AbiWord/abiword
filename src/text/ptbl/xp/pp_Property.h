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

#include "pt_Types.h"

// convenience macro
#define PP_Q(x) \
PP_Property::commonToQuark( PP_COMMON_ ## x )

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


class ABI_EXPORT PP_PropertyType
{
public:
	PP_PropertyType() {};
	virtual ~PP_PropertyType() {};

public:
	virtual tProperty_type getType() const = 0;

	static PP_PropertyType *createPropertyType(tProperty_type Type,
											   GQuark init);
};

class ABI_EXPORT PP_PropertyTypeBool : public PP_PropertyType
{

public:
	PP_PropertyTypeBool(const XML_Char *p_init);

	tProperty_type getType() const {return Property_type_bool;}

	bool getState() const {return State;}


private:

	bool State;
};

class ABI_EXPORT PP_PropertyTypeInt : public PP_PropertyType
{

public:
	PP_PropertyTypeInt(const XML_Char *p_init);

	tProperty_type getType() const {return Property_type_int;}

	int getValue() const {return Value;}


private:

	int Value;
};

class ABI_EXPORT PP_PropertyTypeSize : public PP_PropertyType
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

class ABI_EXPORT PP_PropertyTypeColor : public PP_PropertyType
{

public:
	PP_PropertyTypeColor(const XML_Char *p_init);

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

/* This enum represents strings that are commonly used in property and
 * attribute values.
 *
 * MUST keep in sync with the array defined in pp_Property.cpp
 */
typedef enum {
	PP_COMMON_Times_New_Roman = 0,
	PP_COMMON_Arial,
	PP_COMMON_None,
	PP_COMMON_Normal,
	PP_COMMON_Heading_1,
	PP_COMMON_Heading_2,
	PP_COMMON_Heading_3,
	PP_COMMON_Heading_4,
	PP_COMMON_Heading_5,
	PP_COMMON_Heading_6,
	PP_COMMON_Heading_7,
	PP_COMMON_Heading_8,
	PP_COMMON_Heading_9,
	PP_COMMON_c,
	PP_COMMON_C,
	PP_COMMON_emptystr,
	PP_COMMON_inherit,
	PP_COMMON_logical_ltr,
	PP_COMMON_logical_rtl,
	PP_COMMON_ltr,
	PP_COMMON_rtl,
	PP_COMMON_override_ltr,
	PP_COMMON_override_rtl,
	PP_COMMON_override_clear,
	PP_COMMON_locked,
	PP_COMMON_unlocked,
	PP_COMMON_Current_Settings,
	PP_COMMON_numeric,
	PP_COMMON_A4,
	PP_COMMON_Custom,
	PP_COMMON_mm,
	PP_COMMON_cm,
	PP_COMMON_in,
	PP_COMMON_landscape,
	PP_COMMON_portrait,
	PP_COMMON_app_ver,
	PP_COMMON_app_id,
	PP_COMMON_app_options,
	PP_COMMON_app_target,
	PP_COMMON_app_compiledate,
	PP_COMMON_app_compiletime,
	PP_COMMON_char_count,
	PP_COMMON_date,
	PP_COMMON_date_mmddyy,
	PP_COMMON_date_ddmmyy,
	PP_COMMON_date_mdy,
	PP_COMMON_date_mthdy,
	PP_COMMON_date_dfl,
	PP_COMMON_date_ntdfl,
	PP_COMMON_date_wkday,
	PP_COMMON_date_doy,
	PP_COMMON_datetime_custom,
	PP_COMMON_endnote_ref,
	PP_COMMON_endnote_anchor,
	PP_COMMON_file_name,
	PP_COMMON_footnote_ref,
	PP_COMMON_footnote_anchor,
	PP_COMMON_list_label,
	PP_COMMON_line_count,
	PP_COMMON_mail_merge,
	PP_COMMON_meta_title,
	PP_COMMON_meta_creator,
	PP_COMMON_meta_subject,
	PP_COMMON_meta_publisher,
	PP_COMMON_meta_date,
	PP_COMMON_meta_type,
	PP_COMMON_meta_language,
	PP_COMMON_meta_rights,
	PP_COMMON_meta_keywords,
	PP_COMMON_meta_contributor,
	PP_COMMON_meta_coverage,
	PP_COMMON_meta_description,
	PP_COMMON_martin_test,
	PP_COMMON_nbsp_count,
	PP_COMMON_page_number,
	PP_COMMON_page_count,
	PP_COMMON_para_count,
	PP_COMMON_page_ref,
	PP_COMMON_sum_rows,
	PP_COMMON_sum_cols,
	PP_COMMON_test,
	PP_COMMON_time,
	PP_COMMON_time_miltime,
	PP_COMMON_time_ampm,
	PP_COMMON_time_zone,
	PP_COMMON_time_epoch,
	PP_COMMON_word_count,
	PP_COMMON_end
	
} PP_CommonValue;

class ABI_EXPORT PP_Property
{
public:
	
	XML_Char *			m_pszName;
	XML_Char *			m_pszInitial;
	bool				m_bInherit;
	PP_PropertyType *	m_pProperty;
	tPropLevel          m_iLevel;
	PT_Property         m_iIndex;
	
	~PP_Property();

	inline const XML_Char   * getName() const {return m_pszName;}
	inline const XML_Char   * getInitial() const {return m_pszInitial;}
	const PP_PropertyType   * getInitialType (tProperty_type Type) const;
	inline bool				  canInherit() const {return m_bInherit;}
	inline tPropLevel         getLevel() const {return m_iLevel;}
	inline PT_Property        getIndex() const {return m_iIndex;}

	/* static methods */
	static
	inline const XML_Char   * getName (PT_Property i);

	static
	inline PT_Property        lookupIndex (const XML_Char * name);

	static
	inline const PP_Property* getProperty (PT_Property i);

	static
	const PP_Property *       lookupProperty(const XML_Char * pszName);

	static
	GQuark                    evalProperty(PT_Property i,
										   const PP_AttrProp * pSpanAP,
										   const PP_AttrProp * pBlockAP,
										   const PP_AttrProp * pSectionAP,
										   PD_Document * pDoc,
										   bool bExpandStyles=false);

	static
	const PP_PropertyType   * evalPropertyType(PT_Property i,
											   const PP_AttrProp * pSpanAP,
											   const PP_AttrProp * pBlockAP,
											   const PP_AttrProp * pSectionAP,
											   tProperty_type Type,
											   PD_Document * pDoc=NULL,
											   bool bExpandStyles=false);

	static
	inline bool               isIndexValid (PT_Property i);

	static
	void                      initCommonValues ();
	
	static
	GQuark                    commonToQuark (PP_CommonValue i);
	
	static
	void                      resetInitialBiDiValues(const XML_Char* pszValue);
	
	static
	void                      setDefaultFontFamily  (const char * pszFamily);

private:
	static
	inline PP_Property      * _getProperty (PT_Property i);
};

ABI_EXPORT GQuark PP_getProperty (PT_Property p,
								  const PT_PropertyPair *props);

#endif /* PP_PROPERTY_H */
