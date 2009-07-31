/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

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
#include <OXMLi_ListenerState_Valid.h>

// Internal includes
#include <OXML_Document.h>
#include <OXML_FontManager.h>
#include <OXMLi_PackageManager.h>

// AbiWord includes
#include <ut_assert.h>
#include <ut_misc.h>

// External includes
#include <string>

OXMLi_ListenerState_Valid::OXMLi_ListenerState_Valid():
	OXMLi_ListenerState()
{

}

void OXMLi_ListenerState_Valid::startElement (OXMLi_StartElementRequest * rqst)
{
	//TODO
	rqst->valid = true;
}

void OXMLi_ListenerState_Valid::endElement (OXMLi_EndElementRequest * rqst)
{
	//TODO
	rqst->valid = true;
}

void OXMLi_ListenerState_Valid::charData (OXMLi_CharDataRequest * rqst)
{
	//TODO
	rqst->valid = true;
}
