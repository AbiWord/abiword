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
  : IE_Exp (pDocument), 
	root(NULL),
	relsDir(NULL),
	wordDir(NULL),
	contentTypesFile(NULL),
	relFile(NULL),
	documentFile(NULL)
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
	OXML_Document* doc_ptr = OXML_Document::getNewInstance();
	return doc_ptr->serialize(this);
}

/**
 * Starts exporting the OXML_Document object
 */
UT_Error IE_Exp_OpenXML::startDocument()
{
	GError *err = NULL;	
	UT_Error error = UT_OK;

	GsfOutput* sink = getFp();

	if(!sink)
		return UT_SAVE_EXPORTERROR;

	root = gsf_outfile_zip_new(sink, &err);

	if(err != NULL || root == NULL)
	{
		UT_DEBUGMSG(("FRT: ERROR, Zip root file couldn't be created\n"));	
		g_object_unref (G_OBJECT (sink));
		return UT_IE_COULDNOTWRITE;		
	}

	g_object_unref (G_OBJECT (sink));

	error = writeContentTypes();
	if(error != UT_OK)
	{
		_cleanup();
		return error;
	}	

	error = writeRelations();
	if(error != UT_OK)
	{
		_cleanup();
		return error;
	}
	
	error = writeMainPart();
	if(error != UT_OK)
	{
		_cleanup();
		return error;
	}

	if(!gsf_output_close(GSF_OUTPUT(root)))
	{
		UT_DEBUGMSG(("FRT: ERROR, zip root file couldn't be closed\n"));	
		_cleanup();
		return UT_SAVE_EXPORTERROR;		
	}

	return UT_OK;	
}

/**
 * Finishes exporting OXML_Document object
 */
UT_Error IE_Exp_OpenXML::finishDocument()
{
	return UT_OK;
}

/**
 * Cleans up everything. Called by the destructor.
 */
void IE_Exp_OpenXML::_cleanup ()
{
	if(contentTypesFile && !gsf_output_is_closed(contentTypesFile))
		gsf_output_close(contentTypesFile);

	if(relFile && !gsf_output_is_closed(relFile))
		gsf_output_close(relFile);

	if(relsDir)
	{
		GsfOutput* rels_out = GSF_OUTPUT(relsDir);
		if(!gsf_output_is_closed(rels_out))
			gsf_output_close(rels_out);
	}

	if(documentFile && !gsf_output_is_closed(documentFile))
		gsf_output_close(documentFile);

	if(wordDir)
	{
		GsfOutput* word_out = GSF_OUTPUT(wordDir);
		if(!gsf_output_is_closed(word_out))
			gsf_output_close(word_out);
	}

	if(root)
	{
		GsfOutput* root_out = GSF_OUTPUT(root);
		if(!gsf_output_is_closed(root_out))
			gsf_output_close(root_out);
	}
}

/**
 * Writes the [Content_Types].xml file which describes the contents of the package
 */
UT_Error IE_Exp_OpenXML::writeContentTypes()
{
	UT_Error err = UT_OK;

	contentTypesFile = gsf_outfile_new_child(root, "[Content_Types].xml", FALSE); 

	if(contentTypesFile == NULL)
	{
		UT_DEBUGMSG(("FRT: ERROR, [Content_Types].xml file couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}

	//we only have .rels and .xml file types in the simple basis file
	//TODO: extend this for other file types as needed
	err = writeXmlHeader(contentTypesFile);
	if(err != UT_OK)
	{
		return err;
	}	
	
	if(!gsf_output_puts(contentTypesFile, 
"<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">\
<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>\
<Default Extension=\"xml\" ContentType=\"application/xml\"/>\
<Override PartName=\"/word/document.xml\" \
ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/></Types>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write to [Content_Types].xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}

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
UT_Error IE_Exp_OpenXML::writeRelations()
{
	UT_Error err = UT_OK;

	relsDir = GSF_OUTFILE(gsf_outfile_new_child(root, "_rels", TRUE)); 
	if(relsDir == NULL)
	{
		UT_DEBUGMSG(("FRT: ERROR, _rels directory couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}

	relFile = gsf_outfile_new_child(relsDir, ".rels", FALSE); 
	if(relFile == NULL)
	{
		UT_DEBUGMSG(("FRT: ERROR, .rels file couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}

	err = writeXmlHeader(relFile);
	if(err != UT_OK)
	{
		return err;
	}	
	
	if(!gsf_output_puts(relFile, 
"<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">\
<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" \
Target=\"word/document.xml\"/>\
</Relationships>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write to .rels file\n"));	
		return UT_IE_COULDNOTWRITE;
	}

	if(!gsf_output_close(relFile))
	{
		UT_DEBUGMSG(("FRT: ERROR, .rels file couldn't be closed\n"));	
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
UT_Error IE_Exp_OpenXML::writeMainPart()
{
	UT_Error err = UT_OK;

	wordDir = GSF_OUTFILE(gsf_outfile_new_child(root, "word", TRUE)); 
	if(wordDir == NULL)
	{
		UT_DEBUGMSG(("FRT: ERROR, word directory couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}

	documentFile = gsf_outfile_new_child(wordDir, "document.xml", FALSE); 
	if(documentFile == NULL)
	{
		UT_DEBUGMSG(("FRT: ERROR, document.xml file couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}	

	err = writeXmlHeader(documentFile);
	if(err != UT_OK)
	{
		return err;
	}	
	
	if(!gsf_output_puts(documentFile, 
"<w:wordDocument xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" \
xmlns:v=\"urn:schemas-microsoft-com:vml\" \
xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">\
<w:body>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write to document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}

	//TODO: Write the document body here

	if(!gsf_output_puts(documentFile, "</w:body></w:wordDocument>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write to document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}

	if(!gsf_output_close(documentFile))
	{
		UT_DEBUGMSG(("FRT: ERROR, document.xml file couldn't be closed\n"));	
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
