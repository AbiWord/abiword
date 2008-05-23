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
UT_Error IE_Exp_OpenXML::_writeDocument ()
{

	UT_DEBUGMSG(("FRT: Writing the OOXML file started\n"));	
	
	//get the file pointer (target file)
	GsfOutput* sink = getFp();

	if(sink != NULL)
	{
		UT_DEBUGMSG(("FRT: Zip root file created successfully\n"));		
		
		//create a zip file root based on the target
		GsfOutfile* root = gsf_outfile_zip_new(sink, NULL);

		//unreference sink since we don't need it directly anymore
		//instead we will be using "root"
		g_object_unref (G_OBJECT (sink));


		writeContentTypes(root);
		writeRelations(root);
		writeMainPart(root);

		//finally close the file 
		gsf_output_close((GsfOutput*)root);
	}
	else
	{
		UT_DEBUGMSG(("FRT: Error, Zip root file couldn't be created\n"));
		return UT_IE_COULDNOTWRITE;		
	}

	UT_DEBUGMSG(("FRT: Writing the OOXML file completed\n"));		

	return UT_OK;	
}

/**
 * Cleans up everything. Called by the destructor.
 */
void IE_Exp_OpenXML::_cleanup ()
{
	UT_DEBUGMSG(("FRT: Cleaning up the OOXML exporter\n"));	
}

/**
 * Writes the [Content_Types].xml file which describes the contents of the package
 */
UT_Error IE_Exp_OpenXML::writeContentTypes(GsfOutfile* root)
{

	UT_DEBUGMSG(("FRT: Writing the [Content_Types].xml file started\n"));	
		
	//[Content_Types].xml file 
	GsfOutput* contentTypesFile = gsf_outfile_new_child(root, "[Content_Types].xml", FALSE); 
		
	//write the basic file contents
	//we only have .rels and .xml file types in the simple basis file
	//TODO: extend this for other file types as needed
	writeXmlHeader(contentTypesFile);
	
	gsf_output_puts(contentTypesFile, "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">");
	gsf_output_puts(contentTypesFile, "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>");
	gsf_output_puts(contentTypesFile, "<Default Extension=\"xml\" ContentType=\"application/xml\"/>");
	gsf_output_puts(contentTypesFile, "<Override PartName=\"/word/document.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/>");
	gsf_output_puts(contentTypesFile, "</Types>");
		
	
	//close the file
	gsf_output_close(contentTypesFile);
	
	UT_DEBUGMSG(("FRT: Writing the [Content_Types].xml file completed\n"));	

	return UT_OK;
}


/**
 * Writes the relationships for the files within the package 
 * Outputs the _rels folder and _rels/.rels file which defines the package relations.
 */
UT_Error IE_Exp_OpenXML::writeRelations(GsfOutfile* root)
{

	UT_DEBUGMSG(("FRT: Writing the _rels/.rels file started\n"));	
	//_rels directory
	GsfOutfile* relsDir = GSF_OUTFILE(gsf_outfile_new_child(root, "_rels", TRUE)); 
	//_rels/.rel file
	GsfOutput* relFile = gsf_outfile_new_child(relsDir, ".rels", FALSE); 

	//write the basic file content
	//TODO: extend this for other relations as needed
	writeXmlHeader(relFile);
	
	gsf_output_puts(relFile, "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">");		
	gsf_output_puts(relFile, "<Relationship Id=\"rId1\" ");
	gsf_output_puts(relFile, 	"Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" ");
	gsf_output_puts(relFile, 	"Target=\"word/document.xml\"/>");
	gsf_output_puts(relFile, "</Relationships>");
	

	//close the .rels file and rels directory
	gsf_output_close(relFile);
	gsf_output_close((GsfOutput*)relsDir);

	UT_DEBUGMSG(("FRT: Writing the _rels/.rels file completed\n"));	

	return UT_OK;
}

/**
 * Writes the main part of the document to word/document.xml file.
 */
UT_Error IE_Exp_OpenXML::writeMainPart(GsfOutfile* root)
{
	
	UT_DEBUGMSG(("FRT: Writing the word/document.xml file started\n"));	

	//word directory
	GsfOutfile* wordDir = GSF_OUTFILE(gsf_outfile_new_child(root, "word", TRUE)); 
	//word/document.xml file
	GsfOutput* documentFile = gsf_outfile_new_child(wordDir, "document.xml", FALSE); 

	//write the basic content to document.xml
	writeXmlHeader(documentFile);
	
	gsf_output_puts(documentFile, "<w:wordDocument xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" ");
	gsf_output_puts(documentFile, 	"xmlns:v=\"urn:schemas-microsoft-com:vml\" ");
	gsf_output_puts(documentFile,	"xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">");
	gsf_output_puts(documentFile, "<w:body>");		

	//TODO: Write the document body here

	gsf_output_puts(documentFile, "</w:body>");		
	gsf_output_puts(documentFile, "</w:wordDocument>");		
	
	//close the document.xml file and word directory
	gsf_output_close(documentFile);
	gsf_output_close((GsfOutput*)wordDir);

	UT_DEBUGMSG(("FRT: Writing the word/document.xml file completed\n"));	

	return UT_OK;
}

/**
 * Write the simple xml header to the file
 * This function should be called before anything written to file
 */
UT_Error IE_Exp_OpenXML::writeXmlHeader(GsfOutput* file)
{

	gsf_output_puts(file, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>");

	UT_DEBUGMSG(("FRT: Writing xml header completed\n"));	

	return UT_OK;
}
