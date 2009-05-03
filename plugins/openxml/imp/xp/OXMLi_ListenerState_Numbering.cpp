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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

// Class definition include
#include <OXMLi_ListenerState_Numbering.h>

// Internal includes
#include <OXML_Document.h>
#include <OXML_FontManager.h>
#include <OXML_Types.h>

// AbiWord includes
#include <ut_assert.h>
#include <ut_misc.h>

// External includes
#include <string>

void OXMLi_ListenerState_Numbering::startElement (OXMLi_StartElementRequest * rqst)
{
	if (
		nameMatches(rqst->pName, NS_W_KEY, "numbering") ||
		nameMatches(rqst->pName, NS_W_KEY, "abstractNum") ||
		nameMatches(rqst->pName, NS_W_KEY, "lvl") ||
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
		nameMatches(rqst->pName, NS_W_KEY, "pStyle") ||
		nameMatches(rqst->pName, NS_W_KEY, "start") ||
		nameMatches(rqst->pName, NS_W_KEY, "suff")
		)
	{
		//TODO: add functionality here
		rqst->handled = true;
	}
}

void OXMLi_ListenerState_Numbering::endElement (OXMLi_EndElementRequest * rqst)
{
	if (
		nameMatches(rqst->pName, NS_W_KEY, "numbering") ||
		nameMatches(rqst->pName, NS_W_KEY, "abstractNum") ||
		nameMatches(rqst->pName, NS_W_KEY, "lvl") ||
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
		nameMatches(rqst->pName, NS_W_KEY, "pStyle") ||
		nameMatches(rqst->pName, NS_W_KEY, "start") ||
		nameMatches(rqst->pName, NS_W_KEY, "suff")
		)
	{
		//TODO: add functionality here
		rqst->handled = true;
	}
}

void OXMLi_ListenerState_Numbering::charData (OXMLi_CharDataRequest * /*rqst*/)
{
	UT_ASSERT ( UT_SHOULD_NOT_HAPPEN );
}
