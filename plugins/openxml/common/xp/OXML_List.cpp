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

//External includes
#include <boost/lexical_cast.hpp>

// Class definition include
#include <OXML_List.h>

// Internal includes
#include <OXML_Types.h>
#include <OXML_Document.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_misc.h>
#include <pd_Document.h>

OXML_List::OXML_List() : 
	OXML_ObjectWithAttrProp(),
	id(0),
	parentId(0),
	level(0),
	startValue(0),
	delim(NULL),
	decimal(NULL),
	type(NUMBERED_LIST)
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

void OXML_List::setType(FL_ListType typ)
{
	type = typ;
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

FL_ListType OXML_List::getType()
{
	return type;
}

/**
 * Serializes the abstract numbering definitions. Numbering definitions are serialized
 * in serializeNumbering function.
 */
UT_Error OXML_List::serialize(IE_Exp_OpenXML* exporter)
{
	UT_Error err = UT_OK;

	err = exporter->startAbstractNumbering(TARGET_NUMBERING, id);
	if(err != UT_OK)
	{
		UT_DEBUGMSG(("FRT: Can't start Abstract Numbering\n"));
		return err;
	}

	err = exporter->setMultilevelType(TARGET_NUMBERING, "hybridMultilevel");
	if(err != UT_OK)
	{
		UT_DEBUGMSG(("FRT: Can't set Multilevel Type\n"));
		return err;
	}

	int i;
	for(i=0; i<=8; i++)
	{
		err = exporter->startNumberingLevel(TARGET_NUMBERING, i); //level
		if(err != UT_OK)
		{
			UT_DEBUGMSG(("FRT: Can't start Numbering Level\n"));
			return err;
		}
	
		err = exporter->setListStartValue(TARGET_NUMBERING, startValue);
		if(err != UT_OK)
		{
			UT_DEBUGMSG(("FRT: Can't start List Start Value\n"));
			return err;
		}
	
		std::string txt(delim);
		const char* search = "%L";
		size_t index = txt.find(search, 0, 2);
		
		if(index != std::string::npos)
		{
			txt = txt.replace(index+1, 1, 1, '1'+i);
		}
	
		const gchar* listType = "bullet";
		switch(type)
		{
			case NUMBERED_LIST:
				if((i % 3) == 1){
					listType = "decimal";
				}
				else if((i % 3) == 2){
					listType = "lowerLetter";
				}
				else{
					listType = "lowerRoman";
				}
				break;
			
			case UPPERCASE_LIST:
				listType = "upperLetter";
				break;
	
			case LOWERCASE_LIST:
				listType = "lowerLetter";
				break;
	
			case UPPERROMAN_LIST:
				listType = "upperRoman";
				break;
	
			case LOWERROMAN_LIST:
				listType = "lowerRoman";
				break;
	
			case ARABICNUMBERED_LIST:
				listType = "arabicAbjad";
				break;
	
			case HEBREW_LIST:
				listType = "hebrew1";
				break;
	
			//the rest is bullet
			case DASHED_LIST:
				txt = DASH;
				break;
			case SQUARE_LIST:
				txt = SQUARE;
				break;
			case TRIANGLE_LIST:
				txt = TRIANGLE;
				break;
			case DIAMOND_LIST:
				txt = DIAMOND;
				break;
			case STAR_LIST:
				txt = STAR;
				break;
			case IMPLIES_LIST:
				txt = IMPLIES;
				break;
			case BOX_LIST:
				txt = BOX;
				break;
			case HAND_LIST:
				txt = HAND;
				break;
			case HEART_LIST:
				txt = HEART;
				break;
			case BULLETED_LIST:
				txt = BULLET;
				break;
			default:
				txt = BULLET;
				break;
		}
	
		err = exporter->setListType(TARGET_NUMBERING, listType);
		if(err != UT_OK)
		{
			UT_DEBUGMSG(("FRT: Can't set List Type\n"));
			return err;
		}
	
		err = exporter->setListLevelText(TARGET_NUMBERING, txt.c_str());
		if(err != UT_OK)
		{
			UT_DEBUGMSG(("FRT: Can't set List Level Text\n"));
			return err;
		}
	
		err = exporter->finishNumberingLevel(TARGET_NUMBERING);
		if(err != UT_OK)
		{
			UT_DEBUGMSG(("FRT: Can't finish Numbering Level\n"));
			return err;
		}	
	}	
	
	err = exporter->finishAbstractNumbering(TARGET_NUMBERING);
	if(err != UT_OK)
	{
		UT_DEBUGMSG(("FRT: Can't finish Abstract Numbering\n"));
		return err;
	}

	return UT_OK;
}

/**
 * Serializes the numbering definitions. Numbering definitions have to 
 * come after all the abstract numbering definitions. 
 */
UT_Error OXML_List::serializeNumbering(IE_Exp_OpenXML* exporter)
{
	UT_Error err = UT_OK;

	err = exporter->startNumbering(TARGET_NUMBERING, id);
	if(err != UT_OK)
	{
		UT_DEBUGMSG(("FRT: Can't start Numbering\n"));
		return err;
	}

	err = exporter->setAbstractNumberingId(TARGET_NUMBERING, id);
	if(err != UT_OK)
	{
		UT_DEBUGMSG(("FRT: Can't set Abstract Numbering Id\n"));
		return err;
	}

	return exporter->finishNumbering(TARGET_NUMBERING);
}

UT_Error OXML_List::addToPT(PD_Document * pDocument)
{
	UT_Error err = UT_ERROR;

	const gchar* ppAttr[13];

	std::string listId = boost::lexical_cast<std::string>(id);
	std::string parentListId = boost::lexical_cast<std::string>(parentId);
	std::string listType = boost::lexical_cast<std::string>(type);
	std::string listStartVal = boost::lexical_cast<std::string>(startValue);
	std::string listDelim("%L."); //TODO: need to fix for openxml delim
	std::string listDecimal(".");
	if(decimal)
		listDecimal = decimal;

	ppAttr[0] = "id";
	ppAttr[1] = listId.c_str();
	ppAttr[2] = "parentid";
	ppAttr[3] = parentListId.c_str();
	ppAttr[4] = "type";
	ppAttr[5] = listType.c_str();
	ppAttr[6] = "start-value";
	ppAttr[7] = listStartVal.c_str();
	ppAttr[8] = "list-delim";
	ppAttr[9] = listDelim.c_str();
	ppAttr[10] = "list-decimal";
	ppAttr[11] = listDecimal.c_str();
	ppAttr[12] = 0;
    
	if (pDocument->appendList(ppAttr))
		err = UT_OK;

	return err;
}

