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
	contentTypesStream(NULL),
	relStream(NULL),
	documentStream(NULL)
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
	UT_Error err = UT_SAVE_EXPORTERROR;
	
	IE_Exp_OpenXML_Listener* listener = new IE_Exp_OpenXML_Listener(getDoc());

	OXML_Document* doc_ptr = listener->getDocument();
	
	if(doc_ptr)
		err = doc_ptr->serialize(this);
		
	DELETEP(listener);

	return err;	
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

	if(err || !root)
	{
		UT_DEBUGMSG(("FRT: ERROR, Zip root file couldn't be created\n"));	
		g_object_unref (G_OBJECT (sink));
		return UT_IE_COULDNOTWRITE;		
	}

	g_object_unref (G_OBJECT (sink));

	error = startContentTypes();
	if(error != UT_OK)
		return error;

	error = startRelations();
	if(error != UT_OK)
		return error;
	
	error = startMainPart();
	if(error != UT_OK)
		return error;

	return UT_OK;	
}

/**
 * Finishes exporting OXML_Document object
 */
UT_Error IE_Exp_OpenXML::finishDocument()
{
	UT_Error error = UT_OK;

	error = finishMainPart();
	if(error != UT_OK)
		return error;

	error = finishRelations();
	if(error != UT_OK)
		return error;

	error = finishContentTypes();
	if(error != UT_OK)
		return error;

	if(!gsf_output_close(GSF_OUTPUT(root)))
	{
		UT_DEBUGMSG(("FRT: ERROR, zip root file couldn't be closed\n"));	
		return UT_SAVE_EXPORTERROR;		
	}

	return UT_OK;
}

/**
 * Starts exporting the OXML_Section object
 */
UT_Error IE_Exp_OpenXML::startSection()
{
	if(!gsf_output_puts(documentStream, "<wx:sect>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot start a new section to document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Finishes exporting the OXML_Section object
 */
UT_Error IE_Exp_OpenXML::finishSection()
{
	if(!gsf_output_puts(documentStream, "</wx:sect>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot finish section in document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Starts exporting the OXML_Element_Paragraph object
 */
UT_Error IE_Exp_OpenXML::startParagraph()
{
	if(!gsf_output_puts(documentStream, "<w:p>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot start a new paragraph to document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Finishes exporting the OXML_Element_Paragraph object
 */
UT_Error IE_Exp_OpenXML::finishParagraph()
{
	if(!gsf_output_puts(documentStream, "</w:p>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot finish paragraph in document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Starts exporting the OXML_Element_Text object
 */
UT_Error IE_Exp_OpenXML::startText()
{
	if(!gsf_output_puts(documentStream, "<w:t>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot start a new text to document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Writes the actual content of OXML_Element_Text object
 */
UT_Error IE_Exp_OpenXML::writeText(const char* text)
{
	if(!gsf_output_puts(documentStream, text))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write text to document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Finishes exporting the OXML_Element_Text object
 */
UT_Error IE_Exp_OpenXML::finishText()
{
	if(!gsf_output_puts(documentStream, "</w:t>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot finish text in document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Starts exporting the OXML_Element_Run object
 */
UT_Error IE_Exp_OpenXML::startRun()
{
	if(!gsf_output_puts(documentStream, "<w:r>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot start a new run to document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Finishes exporting the OXML_Element_Run object
 */
UT_Error IE_Exp_OpenXML::finishRun()
{
	if(!gsf_output_puts(documentStream, "</w:r>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot finish run in document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Starts exporting the OXML_Element_Run object's properties
 */
UT_Error IE_Exp_OpenXML::startRunProperties()
{
	if(!gsf_output_puts(documentStream, "<w:rPr>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot start the run properties in document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Finishes exporting the OXML_Element_Run object's properties
 */
UT_Error IE_Exp_OpenXML::finishRunProperties()
{
	if(!gsf_output_puts(documentStream, "</w:rPr>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot finish the run properties in document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Starts exporting the OXML_Element_Paragraph object's properties
 */
UT_Error IE_Exp_OpenXML::startParagraphProperties()
{
	if(!gsf_output_puts(documentStream, "<w:pPr>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot start the paragraph properties in document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Finishes exporting the OXML_Element_Paragraph object's properties
 */
UT_Error IE_Exp_OpenXML::finishParagraphProperties()
{
	if(!gsf_output_puts(documentStream, "</w:pPr>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot finish the paragraph properties in document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Sets bold style
 */
UT_Error IE_Exp_OpenXML::setBold()
{
	if(!gsf_output_puts(documentStream, "<w:b/>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot set bold style in document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Sets italic style
 */
UT_Error IE_Exp_OpenXML::setItalic()
{
	if(!gsf_output_puts(documentStream, "<w:i/>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot set italic style in document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Sets underline style
 */
UT_Error IE_Exp_OpenXML::setUnderline()
{
	if(!gsf_output_puts(documentStream, "<w:u w:val=\"single\"/>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot set underline style in document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Sets overline style
 */
UT_Error IE_Exp_OpenXML::setOverline()
{
	//TODO: Is there an overline option in Word 2007?
	return UT_OK;
}

/**
 * Sets line-through style
 */
UT_Error IE_Exp_OpenXML::setLineThrough()
{
	if(!gsf_output_puts(documentStream, "<w:strike/>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot set line-through style in document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Sets superscript style
 */
UT_Error IE_Exp_OpenXML::setSuperscript()
{
	if(!gsf_output_puts(documentStream, "<w:vertAlign w:val=\"superscript\"/>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot set superscript style in document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Sets subscript style
 */
UT_Error IE_Exp_OpenXML::setSubscript()
{
	if(!gsf_output_puts(documentStream, "<w:vertAlign w:val=\"subscript\"/>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot set subscript style in document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Sets text color style
 */
UT_Error IE_Exp_OpenXML::setTextColor(const gchar* color)
{
	if(!gsf_output_printf(documentStream, "<w:color w:val=\"%s\"/>", UT_colorToHex(color).c_str()))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot set color style in document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Sets text background color style
 */
UT_Error IE_Exp_OpenXML::setTextBackgroundColor(const gchar* color)
{
	if(!gsf_output_printf(documentStream, "<w:shd w:fill=\"%s\"/>", UT_colorToHex(color).c_str()))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot set background color style in document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Sets text alignment
 */
UT_Error IE_Exp_OpenXML::setTextAlignment(const gchar* alignment)
{
	if(!gsf_output_printf(documentStream, "<w:jc w:val=\"%s\"/>", alignment))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot set text alignment in document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Sets text indentation
 */
UT_Error IE_Exp_OpenXML::setTextIndentation(const gchar* indentation)
{
	const gchar* twips = convertToPositiveTwips(indentation);
	if(!twips)
		return UT_OK;

	gboolean printed = false;

	if(isNegativeQuantity(indentation))
		printed = gsf_output_printf(documentStream, "<w:ind w:hanging=\"%s\"/>", twips);
	else
		printed = gsf_output_printf(documentStream, "<w:ind w:firstLine=\"%s\"/>", twips);

	if(!printed)
	{	
		UT_DEBUGMSG(("FRT: ERROR, cannot set text indentation in document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}

	return UT_OK;
}

/**
 * Sets paragraph left margin
 */
UT_Error IE_Exp_OpenXML::setParagraphLeftMargin(const gchar* margin)
{
	const gchar* twips = convertToTwips(margin);
	if(!twips)
		return UT_OK;

	if(!gsf_output_printf(documentStream, "<w:ind w:left=\"%s\"/>", twips))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot set paragraph left margin in document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Sets paragraph right margin
 */
UT_Error IE_Exp_OpenXML::setParagraphRightMargin(const gchar* margin)
{
	const gchar* twips = convertToTwips(margin);
	if(!twips)
		return UT_OK;

	if(!gsf_output_printf(documentStream, "<w:ind w:right=\"%s\"/>", twips))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot set paragraph right margin in document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Sets paragraph top margin
 */
UT_Error IE_Exp_OpenXML::setParagraphTopMargin(const gchar* margin)
{
	const gchar* twips = convertToPositiveTwips(margin);
	if(!twips)
		return UT_OK;

	if(!gsf_output_printf(documentStream, "<w:spacing w:before=\"%s\"/>", twips))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot set paragraph top margin in document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Sets paragraph bottom margin
 */
UT_Error IE_Exp_OpenXML::setParagraphBottomMargin(const gchar* margin)
{
	const gchar* twips = convertToPositiveTwips(margin);
	if(!twips)
		return UT_OK;

	if(!gsf_output_printf(documentStream, "<w:spacing w:after=\"%s\"/>", twips))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot set paragraph bottom margin in document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Checks whether the quantity string is a negative quantity
 */
bool IE_Exp_OpenXML::isNegativeQuantity(const gchar* quantity)
{
	return *quantity == '-';
}

/**
 * Converts the string str to twips, returns positive whole number or NULL if twips=0
 */
const gchar * IE_Exp_OpenXML::convertToPositiveTwips(const gchar* str)
{
	double pt = UT_convertToPoints(str) * 20;
	if(pt < 0) 
		pt = -pt;
	if(pt < 1.0)
		return NULL;
	return UT_convertToDimensionlessString(pt, ".0");
}

/**
 * Converts the string str to twips, returns NULL if twips=0
 */
const gchar * IE_Exp_OpenXML::convertToTwips(const gchar* str)
{
	double pt = UT_convertToPoints(str) * 20;
	if(pt < 1.0 && pt > -1.0)
		return NULL;
	return UT_convertToDimensionlessString(pt, ".0");
}

/**
 * Cleans up everything. Called by the destructor.
 */
void IE_Exp_OpenXML::_cleanup ()
{
	if(contentTypesStream && !gsf_output_is_closed(contentTypesStream))
		gsf_output_close(contentTypesStream);

	if(relStream && !gsf_output_is_closed(relStream))
		gsf_output_close(relStream);

	if(relsDir)
	{
		GsfOutput* rels_out = GSF_OUTPUT(relsDir);
		if(!gsf_output_is_closed(rels_out))
			gsf_output_close(rels_out);
	}

	if(documentStream && !gsf_output_is_closed(documentStream))
		gsf_output_close(documentStream);

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
 * Starts the [Content_Types].xml file which describes the contents of the package
 */
UT_Error IE_Exp_OpenXML::startContentTypes()
{
	UT_Error err = UT_OK;

	contentTypesStream = gsf_output_memory_new();

	if(!contentTypesStream)
	{
		UT_DEBUGMSG(("FRT: ERROR, [Content_Types].xml file couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}

	//we only have .rels and .xml file types in the simple basis file
	//TODO: extend this for other file types as needed
	err = writeXmlHeader(contentTypesStream);
	if(err != UT_OK)
	{
		return err;
	}	
	
	if(!gsf_output_puts(contentTypesStream, 
"<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">\
<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>\
<Default Extension=\"xml\" ContentType=\"application/xml\"/>\
<Override PartName=\"/word/document.xml\" \
ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write to [Content_Types].xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}

	return UT_OK;
}

/**
 * Finishes the [Content_Types].xml file which describes the contents of the package
 */
UT_Error IE_Exp_OpenXML::finishContentTypes()
{
	if(!gsf_output_puts(contentTypesStream, "</Types>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write to [Content_Types].xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}

	GsfOutput* contentTypesFile = gsf_outfile_new_child(root, "[Content_Types].xml", FALSE);

	if(!contentTypesFile)
		return UT_SAVE_EXPORTERROR;		

 	if(!gsf_output_write(contentTypesFile, gsf_output_size(contentTypesStream), 
					 gsf_output_memory_get_bytes(GSF_OUTPUT_MEMORY(contentTypesStream))))
	{
		gsf_output_close(contentTypesFile);
		return UT_SAVE_EXPORTERROR;		
	}

	if(!gsf_output_close(contentTypesStream))
	{
		gsf_output_close(contentTypesFile);
		return UT_SAVE_EXPORTERROR;		
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
UT_Error IE_Exp_OpenXML::startRelations()
{
	UT_Error err = UT_OK;

	relStream = gsf_output_memory_new();
	if(!relStream)
	{
		UT_DEBUGMSG(("FRT: ERROR, .rels file couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}

	err = writeXmlHeader(relStream);
	if(err != UT_OK)
	{
		return err;
	}	
	
	if(!gsf_output_puts(relStream, 
"<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">\
<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" \
Target=\"word/document.xml\"/>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write to .rels file\n"));	
		return UT_IE_COULDNOTWRITE;
	}

	return UT_OK;
}

/**
 * Finishes the relationships
 */
UT_Error IE_Exp_OpenXML::finishRelations()
{
	if(!gsf_output_puts(relStream, "</Relationships>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write to .rels file\n"));	
		return UT_IE_COULDNOTWRITE;
	}

	relsDir = GSF_OUTFILE(gsf_outfile_new_child(root, "_rels", TRUE)); 
	if(!relsDir)
	{
		UT_DEBUGMSG(("FRT: ERROR, _rels directory couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}

	GsfOutput* relFile = gsf_outfile_new_child(relsDir, ".rels", FALSE);

	if(!relFile)
		return UT_SAVE_EXPORTERROR;

 	if(!gsf_output_write(relFile, gsf_output_size(relStream), 
					 gsf_output_memory_get_bytes(GSF_OUTPUT_MEMORY(relStream))))
	{
		gsf_output_close(relFile);
		return UT_SAVE_EXPORTERROR;
	}

	if(!gsf_output_close(relStream))
	{
		gsf_output_close(relFile);
		return UT_SAVE_EXPORTERROR;		
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
 * Starts the main part of the document to word/document.xml file.
 */
UT_Error IE_Exp_OpenXML::startMainPart()
{
	UT_Error err = UT_OK;

	documentStream = gsf_output_memory_new();
	if(!documentStream)
	{
		UT_DEBUGMSG(("FRT: ERROR, document.xml file couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}	

	err = writeXmlHeader(documentStream);
	if(err != UT_OK)
	{
		return err;
	}	
	
	if(!gsf_output_puts(documentStream, 
"<w:wordDocument xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" \
xmlns:v=\"urn:schemas-microsoft-com:vml\" \
xmlns:wx=\"http://schemas.microsoft.com/office/word/2003/auxHint\" \
xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"> \
<w:body>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write to document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}

	return UT_OK;
}

/**
 * Finishes the main part of the document to word/document.xml file.
 */
UT_Error IE_Exp_OpenXML::finishMainPart()
{	
	if(!gsf_output_puts(documentStream, "</w:body></w:wordDocument>"))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write to document.xml file\n"));	
		return UT_IE_COULDNOTWRITE;
	}

	wordDir = GSF_OUTFILE(gsf_outfile_new_child(root, "word", TRUE)); 
	if(!wordDir)
	{
		UT_DEBUGMSG(("FRT: ERROR, word directory couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}
	
	GsfOutput* documentFile = gsf_outfile_new_child(wordDir, "document.xml", FALSE);

	if(!documentFile)
		return UT_SAVE_EXPORTERROR;

 	if(!gsf_output_write(documentFile, gsf_output_size(documentStream), 
					 gsf_output_memory_get_bytes(GSF_OUTPUT_MEMORY(documentStream))))
	{
		gsf_output_close(documentFile);
		return UT_SAVE_EXPORTERROR;
	}
	
	if(!gsf_output_close(documentStream))
	{
		gsf_output_close(documentFile);
		return UT_SAVE_EXPORTERROR;		
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
