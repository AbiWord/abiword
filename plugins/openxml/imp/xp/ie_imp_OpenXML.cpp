/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 * 
 * Copyright (C) 2007 Philippe Milot <PhilMilot@gmail.com>
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
#include <ie_imp_OpenXML.h>

// Internal includes
#include <OXML_Types.h>
#include <OXML_Element.h>
#include <OXML_Section.h>
#include <OXML_Document.h>
#include <OXML_Style.h>
#include <OXML_Theme.h>
#include <OXMLi_PackageManager.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_assert.h>

// External includes
#include <iostream>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-infile-zip.h>


/**
 * Constructor
 */
IE_Imp_OpenXML::IE_Imp_OpenXML (PD_Document * pDocument)
  : IE_Imp (pDocument)
{
}


/*
 * Destructor
 */
IE_Imp_OpenXML::~IE_Imp_OpenXML ()
{
	_cleanup();
}

/**
 * Import the given file
 */
UT_Error IE_Imp_OpenXML::_loadFile (GsfInput * oo_src)
{
	UT_DEBUGMSG(("\n\n\nLoading an OpenXML file\n"));

	UT_Error ret = UT_OK;

	GsfInfile * pGsfInfile = GSF_INFILE (gsf_infile_zip_new (oo_src, NULL));
    
	if (pGsfInfile == NULL) {
		return UT_ERROR;
	}

	OXMLi_PackageManager * mgr = OXMLi_PackageManager::getNewInstance();
	if (mgr == NULL) {
		g_object_unref (G_OBJECT(pGsfInfile));
		_cleanup();
		return UT_ERROR;
	}

	mgr->setContainer(pGsfInfile);

	UT_DEBUGMSG(("Building the data model...\n"));
	//These calls build the data model
	if (UT_OK != (ret = mgr->parseDocumentFootnotes()))
	{
		UT_DEBUGMSG(("OpenXML import: failed to parse the document footnotes\n"));
	}

	if (UT_OK != (ret = mgr->parseDocumentEndnotes()))
	{
		UT_DEBUGMSG(("OpenXML import: failed to parse the document endnotes\n"));
	}

	if (UT_OK != (ret = mgr->parseDocumentTheme()))
	{
		UT_DEBUGMSG(("OpenXML import: failed to parse the document theme\n"));
	}

	if (UT_OK != (ret = mgr->parseDocumentSettings()))
	{
		UT_DEBUGMSG(("OpenXML import: failed to parse the document settings\n"));
	}

	if (UT_OK != (ret = mgr->parseDocumentStyles()))
	{
		UT_DEBUGMSG(("OpenXML import: failed to parse the document styles\n"));
	}

	if (UT_OK != (ret = mgr->parseDocumentNumbering()))
	{
		UT_DEBUGMSG(("OpenXML import: failed to parse the document numbering\n"));
	}

	if (UT_OK != (ret = mgr->parseDocumentStream()))
	{
		_cleanup();
		return ret;
	}

	UT_DEBUGMSG(("Data model built.  Building piecetable...\n"));

	OXML_Document * doc = OXML_Document::getInstance();
	if (doc == NULL) {
		_cleanup();
		return UT_ERROR;
	}

	//This call builds the piecetable from the data model
	if (UT_OK != (ret = doc->addToPT( getDoc() ))) 
	{
		_cleanup();
		return ret;
	}

	_cleanup();

	UT_DEBUGMSG(("Finished loading OpenXML file\n\n\n\n"));

	return ret;

}

void IE_Imp_OpenXML::_cleanup ()
{
	OXMLi_PackageManager::destroyInstance();
	OXML_Document::destroyInstance();
}

