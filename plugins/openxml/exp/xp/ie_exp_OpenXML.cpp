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
#include <ie_exp_OpenXML.h>

// Internal includes

// AbiWord includes
#include <ut_types.h>
#include <ut_assert.h>

// External includes


/**
 * Constructor
 */
IE_Exp_OpenXML::IE_Exp_OpenXML (PD_Document * pDocument)
  : IE_Exp (pDocument)
{
	UT_DEBUGMSG(("FRT: OOXML Exporter, Inside Constructor\n"));	
}


/**
 * Destructor
 */
IE_Exp_OpenXML::~IE_Exp_OpenXML ()
{
	UT_DEBUGMSG(("FRT: OOXML Exporter, Inside Destructor\n"));	
	_cleanup();
}

/**
 * Export the OOXML document here
 */
UT_Error IE_Exp_OpenXML::_writeDocument (){
	UT_DEBUGMSG(("FRT: Writing the OOXML file\n"));	
	return UT_OK;	
}

/**
 * Cleans up everything. Called by the destructor.
 */
void IE_Exp_OpenXML::_cleanup ()
{
	UT_DEBUGMSG(("FRT: Cleaning up the OOXML exporter\n"));	
}

