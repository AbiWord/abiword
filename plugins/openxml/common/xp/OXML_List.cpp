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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

// Class definition include
#include <OXML_List.h>

// Internal includes
#include <OXML_Types.h>
#include <OXML_Document.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_misc.h>
#include <pd_Document.h>

// External includes
#include <string>

OXML_List::OXML_List() : 
	OXML_ObjectWithAttrProp(),
	id(0),
	parentId(0),
	level(0),
	startValue(0),
	delim(NULL),
	decimal(NULL)
{

}

OXML_List::~OXML_List()
{
}

void OXML_List::setId(UT_uint32 listId)
{
	id = listId;
}

void OXML_List::setParentId(UT_uint32 parentListId)
{
	parentId = parentListId;
}

void OXML_List::setLevel(UT_uint32 lvl)
{
	level = lvl;
}

void OXML_List::setStartValue(UT_uint32 val)
{
	startValue = val;
}

void OXML_List::setDelim(const gchar* dlm)
{
	delim = dlm;
}

void OXML_List::setDecimal(const gchar* dcml)
{
	decimal = dcml;
}

UT_uint32 OXML_List::getId()
{
	return id;
}

UT_uint32 OXML_List::getParentId()
{
	return parentId;
}

UT_uint32 OXML_List::getLevel()
{
	return level;
}

UT_uint32 OXML_List::getStartValue()
{
	return startValue;
}

const gchar* OXML_List::getDelim()
{
	return delim;
}

const gchar* OXML_List::getDecimal()
{
	return decimal;
}

UT_Error OXML_List::serialize(IE_Exp_OpenXML* /*exporter*/)
{
	UT_DEBUGMSG(("Serializing List Started\n"));
	UT_DEBUGMSG(("Id=%d\n", id));
	UT_DEBUGMSG(("ParentId=%d\n", parentId));
	UT_DEBUGMSG(("Level=%d\n", level));
	UT_DEBUGMSG(("StartValue=%d\n", startValue));
	UT_DEBUGMSG(("Delim=%s\n", delim));
	UT_DEBUGMSG(("Decimal=%s\n", decimal));
	UT_DEBUGMSG(("Serializing List Finished\n"));
	return UT_OK;
}

UT_Error OXML_List::addToPT(PD_Document * /*pDocument*/)
{
	//TODO
	return UT_OK;
}

