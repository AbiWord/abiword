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
	wordRelsDir(NULL),
	contentTypesStream(NULL),
	relStream(NULL),
	wordRelStream(NULL),
	documentStream(NULL),
	stylesStream(NULL)
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

	error = startWordRelations();
	if(error != UT_OK)
		return error;
	
	error = startMainPart();
	if(error != UT_OK)
		return error;

	error = startStyles();
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

	error = finishStyles();
	if(error != UT_OK)
		return error;

	error = finishWordRelations();
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
	return writeTargetStream(TARGET_DOCUMENT, "<wx:sect>");
}

/**
 * Finishes exporting the OXML_Section object
 */
UT_Error IE_Exp_OpenXML::finishSection()
{
	return writeTargetStream(TARGET_DOCUMENT, "</wx:sect>");
}

/**
 * Starts exporting the OXML_Element_Paragraph object
 */
UT_Error IE_Exp_OpenXML::startParagraph()
{
	return writeTargetStream(TARGET_DOCUMENT, "<w:p>");
}

/**
 * Finishes exporting the OXML_Element_Paragraph object
 */
UT_Error IE_Exp_OpenXML::finishParagraph()
{
	return writeTargetStream(TARGET_DOCUMENT, "</w:p>");
}

/**
 * Starts exporting the OXML_Element_Text object
 */
UT_Error IE_Exp_OpenXML::startText()
{
	return writeTargetStream(TARGET_DOCUMENT, "<w:t>");
}

/**
 * Writes the actual content of OXML_Element_Text object
 */
UT_Error IE_Exp_OpenXML::writeText(const char* text)
{
	return writeTargetStream(TARGET_DOCUMENT, text);
}

/**
 * Finishes exporting the OXML_Element_Text object
 */
UT_Error IE_Exp_OpenXML::finishText()
{
	return writeTargetStream(TARGET_DOCUMENT, "</w:t>");
}

/**
 * Starts exporting the OXML_Element_Run object
 */
UT_Error IE_Exp_OpenXML::startRun()
{
	return writeTargetStream(TARGET_DOCUMENT, "<w:r>");
}

/**
 * Finishes exporting the OXML_Element_Run object
 */
UT_Error IE_Exp_OpenXML::finishRun()
{
	return writeTargetStream(TARGET_DOCUMENT, "</w:r>");
}

/**
 * Starts exporting the OXML_Element_Run object's properties
 */
UT_Error IE_Exp_OpenXML::startRunProperties(int target)
{
	return writeTargetStream(target, "<w:rPr>");
}

/**
 * Finishes exporting the OXML_Element_Run object's properties
 */
UT_Error IE_Exp_OpenXML::finishRunProperties(int target)
{
	return writeTargetStream(target, "</w:rPr>");
}

/**
 * Starts exporting the OXML_Element_Paragraph object's properties
 */
UT_Error IE_Exp_OpenXML::startParagraphProperties(int target)
{
	return writeTargetStream(target, "<w:pPr>");
}

/**
 * Finishes exporting the OXML_Element_Paragraph object's properties
 */
UT_Error IE_Exp_OpenXML::finishParagraphProperties(int target)
{
	return writeTargetStream(target, "</w:pPr>");
}

/**
 * Starts exporting the OXML_Element_Table object
 */
UT_Error IE_Exp_OpenXML::startTable()
{
	return writeTargetStream(TARGET_DOCUMENT, "<w:tbl>");
}

/**
 * Finishes exporting the OXML_Element_Table object
 */
UT_Error IE_Exp_OpenXML::finishTable()
{
	return writeTargetStream(TARGET_DOCUMENT, "</w:tbl>");
}

/**
 * Starts exporting the OXML_Element_Table's properties
 */
UT_Error IE_Exp_OpenXML::startTableProperties(int target)
{
	return writeTargetStream(target, "<w:tblPr>");
}

/**
 * Finishes exporting the OXML_Element_Table's properties
 */
UT_Error IE_Exp_OpenXML::finishTableProperties(int target)
{
	return writeTargetStream(target, "</w:tblPr>");
}

/**
 * Starts exporting the OXML_Element_Table's border properties
 */
UT_Error IE_Exp_OpenXML::startTableBorderProperties(int target)
{
	return writeTargetStream(target, "<w:tblBorders>");
}

/**
 * Finishes exporting the OXML_Element_Table's border properties
 */
UT_Error IE_Exp_OpenXML::finishTableBorderProperties(int target)
{
	return writeTargetStream(target, "</w:tblBorders>");
}

/**
 * Starts exporting the OXML_Element_Cell's border properties
 */
UT_Error IE_Exp_OpenXML::startCellBorderProperties(int target)
{
	return writeTargetStream(target, "<w:tcBorders>");
}

/**
 * Finishes exporting the OXML_Element_Cell's border properties
 */
UT_Error IE_Exp_OpenXML::finishCellBorderProperties(int target)
{
	return writeTargetStream(target, "</w:tcBorders>");
}

/**
 * Starts exporting the OXML_Element_Row object
 */
UT_Error IE_Exp_OpenXML::startRow()
{
	return writeTargetStream(TARGET_DOCUMENT, "<w:tr>");
}

/**
 * Finishes exporting the OXML_Element_Row object
 */
UT_Error IE_Exp_OpenXML::finishRow()
{
	return writeTargetStream(TARGET_DOCUMENT, "</w:tr>");
}

/**
 * Starts exporting the OXML_Element_Cell object
 */
UT_Error IE_Exp_OpenXML::startCell()
{
	return writeTargetStream(TARGET_DOCUMENT, "<w:tc>");
}

/**
 * Finishes exporting the OXML_Element_Cell object
 */
UT_Error IE_Exp_OpenXML::finishCell()
{
	return writeTargetStream(TARGET_DOCUMENT, "</w:tc>");
}

/**
 * Starts exporting the OXML_Element_Cell object's properties
 */
UT_Error IE_Exp_OpenXML::startCellProperties(int target)
{
	return writeTargetStream(target, "<w:tcPr>");
}

/**
 * Finishes exporting the OXML_Element_Cell object's properties
 */
UT_Error IE_Exp_OpenXML::finishCellProperties(int target)
{
	return writeTargetStream(target, "</w:tcPr>");
}

/**
 * Writes to the target stream
 */
UT_Error IE_Exp_OpenXML::writeTargetStream(int target, const char* str)
{
	if(!str) 
		return UT_IE_COULDNOTWRITE;

	if(!gsf_output_puts(getTargetStream(target), str))
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write string %s to target stream %d\n", str, target));	
		return UT_IE_COULDNOTWRITE;
	}
	return UT_OK;
}

/**
 * Retrieves the target stream
 */
GsfOutput* IE_Exp_OpenXML::getTargetStream(int target)
{
	switch(target)
	{
		case TARGET_STYLES:
			return stylesStream;
		case TARGET_DOCUMENT:
			return documentStream;
		case TARGET_DOCUMENT_RELATION:
			return wordRelStream;
		case TARGET_RELATION:
			return relStream;
		case TARGET_CONTENT:
			return contentTypesStream;
		default:
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			return documentStream;
	}
}
	
/**
 * Sets bold style
 */
UT_Error IE_Exp_OpenXML::setBold(int target)
{
	return writeTargetStream(target, "<w:b/>");
}

/**
 * Sets italic style
 */
UT_Error IE_Exp_OpenXML::setItalic(int target)
{
	return writeTargetStream(target, "<w:i/>");
}

/**
 * Sets underline style
 */
UT_Error IE_Exp_OpenXML::setUnderline(int target)
{
	return writeTargetStream(target, "<w:u w:val=\"single\"/>");
}

/**
 * Sets overline style
 */
UT_Error IE_Exp_OpenXML::setOverline(int /* target */)
{
	//TODO: Is there an overline option in Word 2007?
	return UT_OK;
}

/**
 * Sets line-through style
 */
UT_Error IE_Exp_OpenXML::setLineThrough(int target)
{
	return writeTargetStream(target, "<w:strike/>");
}

/**
 * Sets superscript style
 */
UT_Error IE_Exp_OpenXML::setSuperscript(int target)
{
	return writeTargetStream(target, "<w:vertAlign w:val=\"superscript\"/>");
}

/**
 * Sets subscript style
 */
UT_Error IE_Exp_OpenXML::setSubscript(int target)
{
	return writeTargetStream(target, "<w:vertAlign w:val=\"subscript\"/>");
}

/**
 * Sets text color style
 */
UT_Error IE_Exp_OpenXML::setTextColor(int target, const gchar* color)
{
	std::string str("<w:color w:val=\"");
	str += UT_colorToHex(color);
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets background color style
 */
UT_Error IE_Exp_OpenXML::setBackgroundColor(int target, const gchar* color)
{
	std::string str("<w:shd w:fill=\"");
	str += UT_colorToHex(color);
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets font size
 */
UT_Error IE_Exp_OpenXML::setFontSize(int target, const gchar* size)
{
	std::string str("<w:sz w:val=\"");
	str += computeFontSize(size);
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets font family
 */
UT_Error IE_Exp_OpenXML::setFontFamily(int target, const gchar* family)
{
	std::string str("<w:rFonts w:ascii=\"");
	str += family;
	str += "\" w:cs=\"";
	str += family;
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets text direction, eg. right-to-left
 */
UT_Error IE_Exp_OpenXML::setTextDirection(int target, const gchar* direction)
{
	std::string str(direction);
	if(str.compare("rtl") == 0)
		return writeTargetStream(target, "<w:rtl v:val=\"on\"/>");
	else if(str.compare("ltr") == 0)
		return writeTargetStream(target, "<w:rtl v:val=\"off\"/>");
	return UT_OK;
}

/**
 * Sets the widows 
 */
UT_Error IE_Exp_OpenXML::setWidows(int target, const gchar* widows)
{
	UT_sint32 wdws = atoi(widows);
	if(wdws > 0)
		return writeTargetStream(target, "<w:widowControl w:val=\"on\"/>");
	return UT_OK;
}

/**
 * Sets text alignment
 */
UT_Error IE_Exp_OpenXML::setTextAlignment(int target, const gchar* alignment)
{
	std::string str("<w:jc w:val=\"");
	str += alignment;
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets the paragraph style
 */
UT_Error IE_Exp_OpenXML::setParagraphStyle(int target, const gchar* style)
{
	std::string str("<w:pStyle w:val=\"");
	str += style;
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets text indentation
 */
UT_Error IE_Exp_OpenXML::setTextIndentation(int target, const gchar* indentation)
{
	const gchar* twips = convertToPositiveTwips(indentation);
	if(!twips)
		return UT_OK;

	std::string str("<w:ind ");

	if(isNegativeQuantity(indentation))
		str += "w:hanging=\"";
	else
		str += "w:firstLine=\"";

	str += twips;
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets paragraph left margin
 */
UT_Error IE_Exp_OpenXML::setParagraphLeftMargin(int target, const gchar* margin)
{
	const gchar* twips = convertToTwips(margin);
	if(!twips)
		return UT_OK;

	std::string str("<w:ind w:left=\"");
	str += twips;
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets paragraph right margin
 */
UT_Error IE_Exp_OpenXML::setParagraphRightMargin(int target, const gchar* margin)
{
	const gchar* twips = convertToTwips(margin);
	if(!twips)
		return UT_OK;

	std::string str("<w:ind w:right=\"");
	str += twips;
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets paragraph top margin
 */
UT_Error IE_Exp_OpenXML::setParagraphTopMargin(int target, const gchar* margin)
{
	const gchar* twips = convertToPositiveTwips(margin);
	if(!twips)
		return UT_OK;

	std::string str("<w:spacing w:before=\"");
	str += twips;
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets paragraph bottom margin
 */
UT_Error IE_Exp_OpenXML::setParagraphBottomMargin(int target, const gchar* margin)
{
	const gchar* twips = convertToPositiveTwips(margin);
	if(!twips)
		return UT_OK;

	std::string str("<w:spacing w:after=\"");
	str += twips;
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets line height
 */
UT_Error IE_Exp_OpenXML::setLineHeight(int target, const gchar* height)
{
	const gchar* twips = NULL;
	const gchar* lineRule = NULL;

	if(strstr(height, "pt+")) 
	{
		lineRule = "atLeast";
		std::string h(height);
		h.resize(h.length()-1); //get rid of '+' char
		twips = convertToTwips(h.c_str());
	}
	else if(strstr(height, "pt")) 
	{
		lineRule = "exact";
		twips = convertToTwips(height);
	}
	else 
	{
		lineRule = "auto";
		twips = convertToLines(height);
	}
	
	if(!twips)
		return UT_OK;
	
	std::string str("<w:spacing w:line=\"");
	str += twips;
	str += "\" w:lineRule=\"";
	str += lineRule;
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets grid span for horizontally merged cells
 */
UT_Error IE_Exp_OpenXML::setGridSpan(int target, UT_sint32 hspan)
{
	char buffer[12]; 
	int len = snprintf(buffer, 12, "%d", hspan);
	if(len <= 0)
		return UT_IE_COULDNOTWRITE;
	std::string str("<w:gridSpan w:val=\"");
	str += buffer;
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets vertical merge feature for vertically merged cells
 */
UT_Error IE_Exp_OpenXML::setVerticalMerge(int target, const char* vmerge)
{
	std::string str("<w:vmerge w:val=\"");
	str += vmerge;
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets table border style for the specified border in the table
 */
UT_Error IE_Exp_OpenXML::setTableBorder(int target, const char* border, const char* type, const char* color, const char* size)
{
	UT_return_val_if_fail(type, UT_OK);
	
	std::string str("<w:");
	str += border;
	str += " w:val=\"";
	str += type;
	str += "\"";

	if(color)
	{
		str += " w:color=\"";
		str += UT_colorToHex(color);
		str += "\"";
	}

	if(size)
	{
		str += " w:sz=\"";
		str += computeBorderWidth(size);
		str += "\"";
	}

	str += "/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Starts table grid
 */
UT_Error IE_Exp_OpenXML::startTableGrid(int target)
{
	return writeTargetStream(target, "<w:tblGrid>");
}

/**
 * Finishes table grid
 */
UT_Error IE_Exp_OpenXML::finishTableGrid(int target)
{
	return writeTargetStream(target, "</w:tblGrid>");
}

/**
 * Sets grid column
 */
UT_Error IE_Exp_OpenXML::setGridCol(int target, const char* column)
{
	const gchar* twips = convertToPositiveTwips(column);
	if(!twips || !*twips)
		return UT_OK;

	std::string str("");
	str += "<w:gridCol w:w=\"";
	str += twips;
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets column width
 */
UT_Error IE_Exp_OpenXML::setColumnWidth(int target, const char* width)
{
	const gchar* twips = convertToPositiveTwips(width);
	if(!twips || !*twips)
		return UT_OK;

	std::string str("");
	str += "<w:tcW w:w=\"";
	str += twips;
	str += "\" w:type=\"dxa\"/>";
	return writeTargetStream(target, str.c_str());
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
 * Converts the string str to lines, returns NULL if lines=0
 */
const gchar * IE_Exp_OpenXML::convertToLines(const gchar* str)
{
	//1 point == 20 twips; 1 line == 12pts --> 1 line == 20*12=240twips
	double pt = UT_convertDimensionless(str) * 240;
	if(pt < 1.0 && pt > -1.0)
		return NULL;
	return UT_convertToDimensionlessString(pt, ".0");
}

/**
 * Converts the string str to eighths of a point
 */
const gchar * IE_Exp_OpenXML::computeBorderWidth(const gchar* str)
{
	//in eighths of a point
	double pt = UT_convertDimensionless(str) * 160;
	if(pt < 1.0 && pt > -1.0)
		return "0";
	return UT_convertToDimensionlessString(pt, ".0");
}

/**
 * Computes the font-size 
 */
const gchar * IE_Exp_OpenXML::computeFontSize(const gchar* str)
{
	//font-size=X pt --> return 2*X
	double pt = UT_convertDimensionless(str) * 2;
	return UT_convertToDimensionlessString(pt, ".0");
}

/**
 * Cleans up everything. Called by the destructor.
 */
void IE_Exp_OpenXML::_cleanup ()
{
	if(stylesStream && !gsf_output_is_closed(stylesStream))
		gsf_output_close(stylesStream);

	if(contentTypesStream && !gsf_output_is_closed(contentTypesStream))
		gsf_output_close(contentTypesStream);

	if(relStream && !gsf_output_is_closed(relStream))
		gsf_output_close(relStream);

	if(wordRelStream && !gsf_output_is_closed(wordRelStream))
		gsf_output_close(wordRelStream);

	if(documentStream && !gsf_output_is_closed(documentStream))
		gsf_output_close(documentStream);

	if(relsDir)
	{
		GsfOutput* rels_out = GSF_OUTPUT(relsDir);
		if(!gsf_output_is_closed(rels_out))
			gsf_output_close(rels_out);
	}

	if(wordRelsDir)
	{
		GsfOutput* wordRels_out = GSF_OUTPUT(wordRelsDir);
		if(!gsf_output_is_closed(wordRels_out))
			gsf_output_close(wordRels_out);
	}

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
 * Starts the styles.xml file which describes the default styles
 */
UT_Error IE_Exp_OpenXML::startStyles()
{
	UT_Error err = UT_OK;

	stylesStream = gsf_output_memory_new();

	if(!stylesStream)
	{
		UT_DEBUGMSG(("FRT: ERROR, styles.xml file couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}

	err = writeXmlHeader(stylesStream);
	if(err != UT_OK)
	{
		return err;
	}	

	std::string str("<w:styles ");
	str += "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" ";
	str += "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">";

	return writeTargetStream(TARGET_STYLES, str.c_str());
}

/**
 * Finishes the [Content_Types].xml file which describes the contents of the package
 */
UT_Error IE_Exp_OpenXML::finishStyles()
{
	UT_Error err = UT_OK;

	err = writeTargetStream(TARGET_STYLES, "</w:styles>");
	if(err != UT_OK)
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write to styles.xml file\n"));	
		return err;
	}

	GsfOutput* stylesFile = gsf_outfile_new_child(wordDir, "styles.xml", FALSE);

	if(!stylesFile)
		return UT_SAVE_EXPORTERROR;		

 	if(!gsf_output_write(stylesFile, gsf_output_size(stylesStream), 
					 gsf_output_memory_get_bytes(GSF_OUTPUT_MEMORY(stylesStream))))
	{
		gsf_output_close(stylesFile);
		return UT_SAVE_EXPORTERROR;		
	}

	if(!gsf_output_close(stylesStream))
	{
		gsf_output_close(stylesFile);
		return UT_SAVE_EXPORTERROR;		
	}

	if(!gsf_output_close(stylesFile))
	{
		UT_DEBUGMSG(("FRT: ERROR, styles.xml file couldn't be closed\n"));	
		return UT_SAVE_EXPORTERROR;		
	}
	return UT_OK;
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

	std::string str("<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">");
	str += "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>";
	str += "<Default Extension=\"xml\" ContentType=\"application/xml\"/>";
	str += "<Override PartName=\"/word/document.xml\" ";
	str += "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/>";
	str += "<Override PartName=\"/word/styles.xml\" ";
	str += "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml\"/>";
	
	return writeTargetStream(TARGET_CONTENT, str.c_str());

}

/**
 * Finishes the [Content_Types].xml file which describes the contents of the package
 */
UT_Error IE_Exp_OpenXML::finishContentTypes()
{
	UT_Error err = UT_OK;

	err = writeTargetStream(TARGET_CONTENT, "</Types>");
	if(err != UT_OK)
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write to [Content_Types].xml file\n"));	
		return err;
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

	std::string str("<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">");
	str += "<Relationship Id=\"rId1\" ";
	str += "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" ";
	str += "Target=\"word/document.xml\"/>";
	
	return writeTargetStream(TARGET_RELATION, str.c_str());

}

/**
 * Finishes the relationships
 */
UT_Error IE_Exp_OpenXML::finishRelations()
{
	UT_Error err = UT_OK;

	err = writeTargetStream(TARGET_RELATION, "</Relationships>");
	if(err != UT_OK)
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write to .rels file\n"));	
		return err;
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

	return UT_OK;
}

/** 
 * Outputs the word/_rels folder and word/_rels/document.xml.rels file
 */
UT_Error IE_Exp_OpenXML::startWordRelations()
{
	UT_Error err = UT_OK;

	wordRelStream = gsf_output_memory_new();
	if(!wordRelStream)
	{
		UT_DEBUGMSG(("FRT: ERROR, document.xml.rels file couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}

	err = writeXmlHeader(wordRelStream);
	if(err != UT_OK)
	{
		return err;
	}	

	std::string str("<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">");
	str += "<Relationship Id=\"rId1\" ";
	str += "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\" ";
	str += "Target=\"styles.xml\"/>";
	
	return writeTargetStream(TARGET_DOCUMENT_RELATION, str.c_str());

}

/**
 * Finishes the relationships
 */
UT_Error IE_Exp_OpenXML::finishWordRelations()
{
	UT_Error err = UT_OK;
	
	err = writeTargetStream(TARGET_DOCUMENT_RELATION, "</Relationships>");
	if(err != UT_OK)
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write to document.xml.rels file\n"));	
		return err;
	}

	wordRelsDir = GSF_OUTFILE(gsf_outfile_new_child(wordDir, "_rels", TRUE)); 
	if(!wordRelsDir)
	{
		UT_DEBUGMSG(("FRT: ERROR, word/_rels directory couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}

	GsfOutput* wordRelFile = gsf_outfile_new_child(wordRelsDir, "document.xml.rels", FALSE);

	if(!wordRelFile)
		return UT_SAVE_EXPORTERROR;

 	if(!gsf_output_write(wordRelFile, gsf_output_size(wordRelStream), 
					 gsf_output_memory_get_bytes(GSF_OUTPUT_MEMORY(wordRelStream))))
	{
		gsf_output_close(wordRelFile);
		return UT_SAVE_EXPORTERROR;
	}

	if(!gsf_output_close(wordRelStream))
	{
		gsf_output_close(wordRelFile);
		return UT_SAVE_EXPORTERROR;		
	}

	if(!gsf_output_close(wordRelFile))
	{
		UT_DEBUGMSG(("FRT: ERROR, document.xml.rels file couldn't be closed\n"));	
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

	std::string str("<w:wordDocument xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" ");
	str += "xmlns:v=\"urn:schemas-microsoft-com:vml\" ";
	str += "xmlns:wx=\"http://schemas.microsoft.com/office/word/2003/auxHint\" ";
	str += "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body>";
	
	return writeTargetStream(TARGET_DOCUMENT, str.c_str());
}

/**
 * Finishes the main part of the document to word/document.xml file.
 */
UT_Error IE_Exp_OpenXML::finishMainPart()
{
	UT_Error err = UT_OK;

	err = writeTargetStream(TARGET_DOCUMENT, "</w:body></w:wordDocument>");
	if(err != UT_OK)
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write to document.xml file\n"));	
		return err;
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

UT_Error IE_Exp_OpenXML::startStyle(std::string style, std::string basedon, std::string followedby)
{
	std::string str("");
	str += "<w:style w:styleId=\"" + style + "\">";
	str += "<w:name w:val=\"" + style + "\"/>";
	
	if(!basedon.empty())
		str += "<w:basedOn w:val=\"" + basedon + "\"/>";		
	if(!followedby.empty())
		str += "<w:next w:val=\"" + followedby + "\"/>";		

	return writeTargetStream(TARGET_STYLES, str.c_str());
}

UT_Error IE_Exp_OpenXML::finishStyle()
{
	return writeTargetStream(TARGET_STYLES, "</w:style>");
}

UT_Error IE_Exp_OpenXML::writeDefaultStyle()
{
	//TODO: add more default settings here
	std::string str("<w:docDefaults>");
	str += "<w:pPrDefault><w:pPr><w:pStyle w:val=\"Normal\"/></w:pPr></w:pPrDefault>";
	str += "<w:rPrDefault><w:rPr><w:rStyle w:val=\"Normal\"/></w:rPr></w:rPrDefault>";
	str += "</w:docDefaults>";
	return writeTargetStream(TARGET_STYLES, str.c_str());

}
