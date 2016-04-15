/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 *
 * Copyright (C) 2008 Firat Kiyak <firatkiyak@gmail.com>
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

#ifndef _OXML_LIST_H_
#define _OXML_LIST_H_

// Internal includes
#include <OXML_Types.h>
#include <OXML_ObjectWithAttrProp.h>

// AbiWord includes
#include <ut_types.h>
#include <pd_Document.h>
#include <fl_AutoNum.h>

// External includes
#include <memory>
#include <string>

//bullet characters
#define BULLET "&#8226;"
#define SQUARE "&#9632;"
#define TRIANGLE "&#61656;"
#define DIAMOND "&#61558;"
#define BOX "&#9633;"
#define HAND "&#9758;"
#define HEART "&#9829;"
#define STAR "*"
#define IMPLIES "&#8658;"
#define DASH "&#8211;"
#define TICK "&#61692;"

class IE_Exp_OpenXML;

class OXML_List : public OXML_ObjectWithAttrProp
{

public:
	OXML_List();
	virtual ~OXML_List();

	virtual void setId(UT_uint32 id);
	virtual void setParentId(UT_uint32 id);
	virtual void setLevel(UT_uint32 id);
	virtual void setDelim(const std::string & delim);
	virtual void setDecimal(const std::string & decimal);
	virtual void setStartValue(UT_uint32 id);
	virtual void setType(FL_ListType type);

	UT_uint32 getId();
	UT_uint32 getParentId();
	UT_uint32 getLevel();
	UT_uint32 getStartValue();
	const gchar* getDelim();
	const gchar* getDecimal();
	FL_ListType getType();

	UT_Error serialize(IE_Exp_OpenXML* exporter);
	UT_Error serializeNumbering(IE_Exp_OpenXML* exporter);
	UT_Error addToPT(PD_Document * pDocument);

private:
	UT_uint32 id;
	UT_uint32 parentId;
	UT_uint32 level;
	UT_uint32 startValue;
	std::string delim;
	std::string decimal;
	FL_ListType type;
};

typedef std::shared_ptr<OXML_List> OXML_SharedList;

#endif //_OXML_LIST_H_

