/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 * 
 * Copyright (C) 2009 Firat Kiyak <firatkiyak@gmail.com>
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
#include <OXMLi_ListenerState_Numbering.h>

// Internal includes
#include <OXML_Document.h>
#include <OXML_FontManager.h>
#include <OXML_Types.h>
#include <OXML_List.h>

// AbiWord includes
#include <ut_assert.h>
#include <ut_misc.h>

// External includes
#include <string>

OXMLi_ListenerState_Numbering::OXMLi_ListenerState_Numbering():
	OXMLi_ListenerState(),
	m_currentList(NULL),
	m_currentNumId(""),
	m_parentListId("")
{

}

void OXMLi_ListenerState_Numbering::startElement (OXMLi_StartElementRequest * rqst)
{
	if (
		nameMatches(rqst->pName, NS_W_KEY, "numbering") ||
		nameMatches(rqst->pName, NS_W_KEY, "multiLevelType") ||
		nameMatches(rqst->pName, NS_W_KEY, "name") ||
		nameMatches(rqst->pName, NS_W_KEY, "nsid") ||
		nameMatches(rqst->pName, NS_W_KEY, "numStyleLink") ||
		nameMatches(rqst->pName, NS_W_KEY, "styleLink") ||
		nameMatches(rqst->pName, NS_W_KEY, "tmpl") ||
		nameMatches(rqst->pName, NS_W_KEY, "isLgl") ||
		nameMatches(rqst->pName, NS_W_KEY, "legacy") ||
		nameMatches(rqst->pName, NS_W_KEY, "lvlJc") ||
		nameMatches(rqst->pName, NS_W_KEY, "lvlPicBulletId") ||
		nameMatches(rqst->pName, NS_W_KEY, "lvlRestart") ||
		nameMatches(rqst->pName, NS_W_KEY, "suff")
		)
	{
		//TODO: add functionality here
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "abstractNum"))
	{
		const gchar* abstractNumId = attrMatches(NS_W_KEY, "abstractNumId", rqst->ppAtts);
		if(abstractNumId)
		{
			m_parentListId = std::string("1");
			m_parentListId += abstractNumId;
		}
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "lvl"))
	{
		const gchar* ilvl = attrMatches(NS_W_KEY, "ilvl", rqst->ppAtts);
		if(ilvl)
		{
			handleLevel(ilvl);
		}
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "start"))
	{
		const gchar* val = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
		if(val && m_currentList)
		{
			m_currentList->setStartValue(atoi(val));
		}
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "numFmt"))
	{
		const gchar* val = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
		if(val)
		{
			handleFormattingType(val);
		}
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "lvlText"))
	{
		const gchar* val = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
		if(val && m_currentList)
		{
			std::string delim(val);
			m_currentList->setDelim(delim);
		}
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "num")) 
	{	
		const gchar* numId = attrMatches(NS_W_KEY, "numId", rqst->ppAtts);
		if(numId)
		{
			m_currentNumId = std::string(numId);
		}
		rqst->handled = true;	
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "abstractNumId")) 
	{	
		const gchar* val = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
		if(val && !m_currentNumId.empty())
		{
			std::string abstractNumId("1"); //starts at 10 instead of zero
			abstractNumId += val;
			OXML_Document* doc = OXML_Document::getInstance();
			if(doc)
				doc->setMappedNumberingId(m_currentNumId, abstractNumId);
		}
		rqst->handled = true;	
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "pPr"))
	{
		//insert a dummy paragraph element to stack 
		//so that we can collect the properties from common listener
		OXML_SharedElement dummy(new OXML_Element_Paragraph(""));
		rqst->stck->push(dummy);
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "rPr"))
	{
		OXML_SharedElement dummy(new OXML_Element_Run(""));
		rqst->stck->push(dummy);
		rqst->handled = true;
	}
}

void OXMLi_ListenerState_Numbering::endElement (OXMLi_EndElementRequest * rqst)
{
	if (
		nameMatches(rqst->pName, NS_W_KEY, "numbering") ||
		nameMatches(rqst->pName, NS_W_KEY, "abstractNum") ||
		nameMatches(rqst->pName, NS_W_KEY, "multiLevelType") ||
		nameMatches(rqst->pName, NS_W_KEY, "name") ||
		nameMatches(rqst->pName, NS_W_KEY, "nsid") ||
		nameMatches(rqst->pName, NS_W_KEY, "numStyleLink") ||
		nameMatches(rqst->pName, NS_W_KEY, "styleLink") ||
		nameMatches(rqst->pName, NS_W_KEY, "tmpl") ||
		nameMatches(rqst->pName, NS_W_KEY, "isLgl") ||
		nameMatches(rqst->pName, NS_W_KEY, "legacy") ||
		nameMatches(rqst->pName, NS_W_KEY, "lvlJc") ||
		nameMatches(rqst->pName, NS_W_KEY, "lvlPicBulletId") ||
		nameMatches(rqst->pName, NS_W_KEY, "lvlRestart") ||
		nameMatches(rqst->pName, NS_W_KEY, "lvlText") ||
		nameMatches(rqst->pName, NS_W_KEY, "numFmt") ||
		nameMatches(rqst->pName, NS_W_KEY, "start") ||
		nameMatches(rqst->pName, NS_W_KEY, "suff") ||
		nameMatches(rqst->pName, NS_W_KEY, "abstractNumId")
		)
	{
		//TODO: add functionality here
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "lvl"))
	{
		OXML_Document * doc = OXML_Document::getInstance();		
		if(!doc)
		{
			doc = OXML_Document::getNewInstance();
		}			
		OXML_SharedList sharedList(m_currentList);
		doc->addList(sharedList);
		m_currentList = NULL;
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "num"))
	{
		m_currentNumId.clear(); //set it to empty string
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "pPr") || 
			nameMatches(rqst->pName, NS_W_KEY, "rPr"))
	{
		if(rqst->stck->empty())
		{
			rqst->handled = false;
			rqst->valid = false;
			return;
		}
		OXML_SharedElement dummy = rqst->stck->top();
		
		if(m_currentList)
		{
			m_currentList->setAttributes(dummy->getAttributes());
			m_currentList->setProperties(dummy->getProperties());
		}
		rqst->stck->pop(); //remove the dummy element
		rqst->handled = true;
	}
}

void OXMLi_ListenerState_Numbering::charData (OXMLi_CharDataRequest * /*rqst*/)
{
	//don't do anything here
}


//private helper functions


/**
 * Handles the new level tag in the form <lvl ilvl="3"> 
 */
void OXMLi_ListenerState_Numbering::handleLevel(const gchar* ilvl)
{
	m_currentList = new OXML_List();
	m_currentList->setLevel(atoi(ilvl)+1); //levels start with index 1
	//create a new list with the list id and parent id
	//all lists are encoded as id=parentId+ilvl
	std::string listId(m_parentListId);
	listId += ilvl;
	m_currentList->setId(atoi(listId.c_str()));
	if(!strcmp(ilvl, "0"))
	{
		//this is the first level
		m_currentList->setParentId(0); //no parent
	}
	else
	{
		std::string parentListId(m_parentListId);
		parentListId += ('0'+(atoi(ilvl)-1));				
		m_currentList->setParentId(atoi(parentListId.c_str())); //has parent (ilvl-1)
	}
}

void OXMLi_ListenerState_Numbering::handleFormattingType(const gchar* val)
{
	if(!m_currentList)
		return;

	if(!strcmp(val, "decimal"))
		m_currentList->setType(NUMBERED_LIST);
	else if(!strcmp(val, "lowerLetter"))
		m_currentList->setType(LOWERCASE_LIST);
	else if(!strcmp(val, "upperLetter"))
		m_currentList->setType(UPPERCASE_LIST);
	else if(!strcmp(val, "lowerRoman"))
		m_currentList->setType(LOWERROMAN_LIST);
	else if(!strcmp(val, "upperRoman"))
		m_currentList->setType(UPPERROMAN_LIST);
	else if(!strcmp(val, "aravicAbjad"))
		m_currentList->setType(ARABICNUMBERED_LIST);
	else if(!strcmp(val, "hebrew1"))
		m_currentList->setType(HEBREW_LIST);
	else //default
		m_currentList->setType(BULLETED_LIST);
	//TODO: add more types here
}

