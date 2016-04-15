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

// Class definition include
#include <OXML_List.h>

// Internal includes
#include <OXML_Types.h>
#include <OXML_Document.h>

// AbiWord includes
#include "ut_types.h"
#include "ut_misc.h"
#include "ut_std_string.h"
#include "pd_Document.h"

OXML_List::OXML_List() : 
	OXML_ObjectWithAttrProp(),
	id(0),
	parentId(0),
	level(0),
	startValue(0),
	delim(""),
	decimal(""),
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

void OXML_List::setDelim(const std::string & dlm)
{
	UT_Error err = UT_OK;

	delim = dlm;

	if(type == BULLETED_LIST)
	{
	    UT_UCS4String ucs4Str = delim;

		if (!ucs4Str.empty())
		{
			switch (ucs4Str[0])
			{
				case 8226: // U+2022 BULLET
					type = BULLETED_LIST;
					err = setProperty("field-font", "NULL");
					break;
                        
				case 8211: // U+2013 EN DASH
					type = DASHED_LIST;
					err = setProperty("field-font", "NULL");
					break;

				case 9632:  // U+25A0 BLACK SQUARE                        
				case 61607: // MS WORD 2007 BLACK SQUARE
					type = SQUARE_LIST;
					err = setProperty("field-font", "NULL");
					break;
                        
				case 9650: // U+25B2 BLACK UP-POINTING TRIANGLE
				case 9654: // TRIANGLE
				case 61656: // MS WORD 2007 TRIANGLE ARROW
					type = TRIANGLE_LIST;
					err = setProperty("field-font", "NULL");
					break;
                    
				case 9670: //U+25C6 DIAMOND
				case 9830: // U+2666 BLACK DIAMOND SUIT
				case 61558: // MS WORD 2007 DIAMOND SUIT
					type = DIAMOND_LIST;
					err = setProperty("field-font", "NULL");
					break;
                        
				case 10035: // U+2733 EIGHT SPOKED ASTERISK
				case 42: // MS WORD 2007 EIGHT SPOKED ASTERISK
					type = STAR_LIST;
					err = setProperty("field-font", "NULL");
					break;
                        
				case 10003: // U+2713 CHECK MARK
				case 61692: // MS WORD 2007 CHECK MARK
					type = TICK_LIST;
					err = setProperty("field-font", "NULL");
					break;
                        
				case 9633: //BOX
				case 10066: // U+2752 UPPER RIGHT SHADOWED WHITE SQUARE
					type = BOX_LIST;
					err = setProperty("field-font", "NULL");
					break;
                        
				case 9758: // U+261E WHITE RIGHT POINTING INDEX
					type = HAND_LIST;
					err = setProperty("field-font", "NULL");
					break;                        

				case 9829: // U+2665 BLACK HEART SUIT
				case 61609: //MS WORD 2007 HEART SUIT
					type = HEART_LIST;
					err = setProperty("field-font", "NULL");
					break;
                        
				case 8658: // U+21D2 RIGHTWARDS DOUBLE ARROW
					type = IMPLIES_LIST;
					err = setProperty("field-font", "NULL");
					break;

				default:
					type = BULLETED_LIST;
					err = setProperty("field-font", "NULL");
					break;
			};
		}
	}

	if(err != UT_OK)
	{
		UT_DEBUGMSG(("FRT:OpenXML importer setting field-font property failed\n"));
		type = BULLETED_LIST;
	}	
}

void OXML_List::setDecimal(const std::string & dcml)
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
	return delim.c_str();
}

const gchar* OXML_List::getDecimal()
{
	return decimal.c_str();
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
	
		std::string fontFamily("Times New Roman");
		const gchar* listType = "bullet";
		switch(type)
		{
			case NUMBERED_LIST:
				if((i % 3) == 1){
					listType = "lowerRoman";
				}
				else if((i % 3) == 2){
					listType = "lowerLetter";
				}
				else{
					listType = "decimal";
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
				fontFamily = "Wingdings";
				break;
			case DIAMOND_LIST:
				txt = DIAMOND;
				fontFamily = "Wingdings";
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
			case TICK_LIST:
				txt = TICK;
				fontFamily = "Wingdings";
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

		err = exporter->startRunProperties(TARGET_NUMBERING);
		if(err != UT_OK)
		{
			UT_DEBUGMSG(("FRT: Can't start List Level Run Properties\n"));
			return err;
		}

		err = exporter->setFontFamily(TARGET_NUMBERING, fontFamily.c_str());
		if(err != UT_OK)
		{
			UT_DEBUGMSG(("FRT: Can't set font family\n"));
			return err;
		}

		err = exporter->finishRunProperties(TARGET_NUMBERING);
		if(err != UT_OK)
		{
			UT_DEBUGMSG(("FRT: Can't finish List Level Run Properties\n"));
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

	std::string listId = UT_std_string_sprintf("%u", id);
	std::string parentListId = UT_std_string_sprintf("%u", parentId);
	std::string listType = UT_std_string_sprintf("%u", type);
	std::string listStartVal = UT_std_string_sprintf("%u", startValue);
	std::string listDelim("%L.");
	std::string listDecimal(".");
	if(decimal.compare(""))
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

