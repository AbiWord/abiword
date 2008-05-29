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

}


/**
 * Destructor
 */
IE_Exp_OpenXML::~IE_Exp_OpenXML ()
{

	_cleanup();
}

/**
 * Export the OOXML document here
 */
UT_Error IE_Exp_OpenXML::_writeDocument ()
{
	//error references to check 
	GError *err = NULL;	
	UT_Error error = UT_OK;

	//get the file pointer (target file)
	GsfOutput* sink = getFp();

	if(!sink)
		return UT_SAVE_EXPORTERROR;


	/** STEP 1. OPEN NEW FILES FOR WRITING **/
		
	//create a zip file root based on the target
	GsfOutfile* root = gsf_outfile_zip_new(sink, &err);

	if(err != NULL || root == NULL)
	{
		UT_DEBUGMSG(("FRT: ERROR, Zip root file couldn't be created\n"));	
		UT_DEBUGMSG(("FRT: Stopping the OpenXML exporting process\n"));
		g_object_unref (G_OBJECT (sink));
		return UT_IE_COULDNOTWRITE;		
	}

	//unreference sink since we don't need it directly anymore
	//instead we will be using "root"
	g_object_unref (G_OBJECT (sink));


	
	/** STEP 2. WRITE THE FILES **/

	//write the content types
	error = writeContentTypes(root);
	if(error != UT_OK)
	{
		_cleanup();
		return error;
	}	

	//write the relations
	error = writeRelations(root);
	if(error != UT_OK)
	{
		_cleanup();
		return error;
	}
	
	//write the main part
	error = writeMainPart(root);
	if(error != UT_OK)
	{
		_cleanup();
		return error;
	}


	/** STEP 3. CLOSE THE FILES **/

	//try to close the zip root file and make sure it is closed
	if(!gsf_output_close(GSF_OUTPUT(root)))
	{
		UT_DEBUGMSG(("FRT: ERROR, zip root file couldn't be closed\n"));	
		_cleanup();
		return UT_SAVE_EXPORTERROR;		
	}

	return UT_OK;	
}

/**
 * Cleans up everything. Called by the destructor.
 */
void IE_Exp_OpenXML::_cleanup ()
{

}

/**
 * Writes the [Content_Types].xml file which describes the contents of the package
 */
UT_Error IE_Exp_OpenXML::writeContentTypes(GsfOutfile* root)
{
	UT_Error err = UT_OK;

	/** STEP 1. OPEN NEW FILES FOR WRITING **/
		
	//[Content_Types].xml file 
	GsfOutput* contentTypesFile = gsf_outfile_new_child(root, "[Content_Types].xml", FALSE); 
	//check if we get a valid pointer
	if(contentTypesFile == NULL)
	{
		UT_DEBUGMSG(("FRT: ERROR, [Content_Types].xml file couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}


	/** STEP 2. WRITE THE FILES **/
		
	//write the basic file contents
	//we only have .rels and .xml file types in the simple basis file
	//TODO: extend this for other file types as needed
	err = writeXmlHeader(contentTypesFile);
	//check if an error occured while writing the xml header
	if(err != UT_OK)
	{
		//try to close the output files since there is an error
		gsf_output_close(contentTypesFile);
		return err;
	}	
	
	if(!gsf_output_puts(contentTypesFile, "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\"><Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/><Default Extension=\"xml\" ContentType=\"application/xml\"/><Override PartName=\"/word/document.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/></Types>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write to [Content_Types].xml file\n"));	
		//try to close the output files since there is an error
		gsf_output_close(contentTypesFile);
		return UT_IE_COULDNOTWRITE;
	}


	/** STEP 3. CLOSE THE FILES **/

	//close the [Content_Types].xml file and make sure it is closed
	if(!gsf_output_close(contentTypesFile))
	{
		UT_DEBUGMSG(("FRT: ERROR, [Content_Types].xml file couldn't be closed\n"));	
		return UT_SAVE_EXPORTERROR;		
	}

	return UT_OK;
}


/**
 * Writes the relationships for the files within the package 
 * Outputs the _rels folder and _rels/.rels file which defines the package relations.
 */
UT_Error IE_Exp_OpenXML::writeRelations(GsfOutfile* root)
{
	UT_Error err = UT_OK;

	/** STEP 1. OPEN NEW FILES FOR WRITING **/

	//_rels directory
	GsfOutfile* relsDir = GSF_OUTFILE(gsf_outfile_new_child(root, "_rels", TRUE)); 
	//check if we get a valid pointer
	if(relsDir == NULL)
	{
		UT_DEBUGMSG(("FRT: ERROR, _rels directory couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}

	//_rels/.rel file
	GsfOutput* relFile = gsf_outfile_new_child(relsDir, ".rels", FALSE); 
	//check if we get a valid pointer
	if(relFile == NULL)
	{
		UT_DEBUGMSG(("FRT: ERROR, .rels file couldn't be created\n"));	
		//try to close the _rels directory since there is an error
		gsf_output_close(GSF_OUTPUT(relsDir));
		return UT_SAVE_EXPORTERROR;
	}

	/** STEP 2. WRITE THE FILES **/
	
	//write the basic file content
	//TODO: extend this for other relations as needed
	err = writeXmlHeader(relFile);
	//check if an error occured while writing the xml header
	if(err != UT_OK)
	{
		//try to close the output files since there is an error
		gsf_output_close(relFile);
		gsf_output_close(GSF_OUTPUT(relsDir));
		return err;
	}	
	
	//write to .rels file 
	if(!gsf_output_puts(relFile, "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\"><Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"word/document.xml\"/></Relationships>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write to .rels file\n"));	
		//try to close the output files since there is an error
		gsf_output_close(relFile);
		gsf_output_close(GSF_OUTPUT(relsDir));
		return UT_IE_COULDNOTWRITE;
	}

	
	/** STEP 3. CLOSE THE FILES **/

	//close the .rels file and rels directory and make sure they are closed
	if(!gsf_output_close(relFile))
	{
		UT_DEBUGMSG(("FRT: ERROR, .rels file couldn't be closed\n"));	
		gsf_output_close(GSF_OUTPUT(relsDir));
		return UT_SAVE_EXPORTERROR;		
	}
	if(!gsf_output_close(GSF_OUTPUT(relsDir)))
	{
		UT_DEBUGMSG(("FRT: ERROR, _rels directory couldn't be closed\n"));	
		return UT_SAVE_EXPORTERROR;		
	}

	return UT_OK;
}

/**
 * Writes the main part of the document to word/document.xml file.
 */
UT_Error IE_Exp_OpenXML::writeMainPart(GsfOutfile* root)
{
	UT_Error err = UT_OK;


	/** STEP 1. OPEN NEW FILES FOR WRITING **/
		
	//word directory
	GsfOutfile* wordDir = GSF_OUTFILE(gsf_outfile_new_child(root, "word", TRUE)); 
	//make sure we get a valid pointer
	if(wordDir == NULL)
	{
		UT_DEBUGMSG(("FRT: ERROR, word directory couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}

	//word/document.xml file
	GsfOutput* documentFile = gsf_outfile_new_child(wordDir, "document.xml", FALSE); 
	//make sure we get a valid pointer
	if(documentFile == NULL)
	{
		UT_DEBUGMSG(("FRT: ERROR, document.xml file couldn't be created\n"));	
		//try to close the word directory since there is an error
		gsf_output_close(GSF_OUTPUT(wordDir));
		return UT_SAVE_EXPORTERROR;
	}	


	/** STEP 2. WRITE THE FILES **/

	//write the basic content to document.xml
	err = writeXmlHeader(documentFile);
	//check if an error occured while writing the xml header
	if(err != UT_OK)
	{
		//try to close the output files since there is an error
		gsf_output_close(documentFile);
		gsf_output_close(GSF_OUTPUT(wordDir));
		return err;
	}	
	
	//try to write to document.xml and handle if there is an error
	if(!gsf_output_puts(documentFile, "<w:wordDocument xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write to document.xml file\n"));	
		//try to close the output files since there is an error
		gsf_output_close(documentFile);
		gsf_output_close(GSF_OUTPUT(wordDir));
		return UT_IE_COULDNOTWRITE;
	}

	//TODO: Write the document body here

	if(!gsf_output_puts(documentFile, "</w:body></w:wordDocument>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write to document.xml file\n"));	
		//try to close the output files since there is an error
		gsf_output_close(documentFile);
		gsf_output_close(GSF_OUTPUT(wordDir));
		return UT_IE_COULDNOTWRITE;
	}


	/** STEP 3. CLOSE THE FILES **/
	
	//close the document.xml file and word directory and make sure they are actually closed
	if(!gsf_output_close(documentFile))
	{
		UT_DEBUGMSG(("FRT: ERROR, document.xml file couldn't be closed\n"));	
		gsf_output_close(GSF_OUTPUT(wordDir));
		return UT_SAVE_EXPORTERROR;		
	}
	if(!gsf_output_close(GSF_OUTPUT(wordDir)))
	{
		UT_DEBUGMSG(("FRT: ERROR, word directory couldn't be closed\n"));	
		return UT_SAVE_EXPORTERROR;		
	}

	return UT_OK;
}

/**
 * Write the simple xml header to the file
 * This function should be called before anything written to file
 */
UT_Error IE_Exp_OpenXML::writeXmlHeader(GsfOutput* file)
{
	gboolean successful = gsf_output_puts(file, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>");	

	if(!successful)
	{
		UT_DEBUGMSG(("FRT: ERROR, xml header couldn't be written\n"));	
		return UT_IE_COULDNOTWRITE;
	}

	return UT_OK;
}
