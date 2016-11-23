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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */


// Class definition include
#include <ie_exp_OpenXML.h>

// Abiword includes
#include <ut_std_string.h>

/**
 * Constructor
 */
IE_Exp_OpenXML::IE_Exp_OpenXML (PD_Document * pDocument)
  : IE_Exp (pDocument), 
	m_pDoc(pDocument),
	root(NULL),
	relsDir(NULL),
	wordDir(NULL),
	wordRelsDir(NULL),
	wordMediaDir(NULL),
	contentTypesStream(NULL),
	relStream(NULL),
	wordRelStream(NULL),
	documentStream(NULL),
	settingsStream(NULL),
	stylesStream(NULL),
	numberingStream(NULL),
	headerStream(NULL),
	footerStream(NULL),
	footnoteStream(NULL),
	endnoteStream(NULL),
	isOverline(false)
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

	error = startEndnotes();
	if(error != UT_OK)
		return error;

	error = startFootnotes();
	if(error != UT_OK)
		return error;

	error = startHeaders();
	if(error != UT_OK)
		return error;

	error = startFooters();
	if(error != UT_OK)
		return error;

	error = startContentTypes();
	if(error != UT_OK)
		return error;

	error = startRelations();
	if(error != UT_OK)
		return error;

	error = startWordRelations();
	if(error != UT_OK)
		return error;

	error = startWordMedia();
	if(error != UT_OK)
		return error;
	
	error = startMainPart();
	if(error != UT_OK)
		return error;

	error = startSettings();
	if(error != UT_OK)
		return error;

	error = startStyles();
	if(error != UT_OK)
		return error;

	error = startNumbering();
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

	error = finishSettings();
	if(error != UT_OK)
		return error;

	error = finishNumbering();
	if(error != UT_OK)
		return error;

	error = finishStyles();
	if(error != UT_OK)
		return error;

	error = finishWordMedia();
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

	error = finishHeaders();
	if(error != UT_OK)
		return error;

	error = finishFooters();
	if(error != UT_OK)
		return error;

	error = finishFootnotes();
	if(error != UT_OK)
		return error;

	error = finishEndnotes();
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
 * Starts exporting the OXML_Section object's properties
 */
UT_Error IE_Exp_OpenXML::startSectionProperties()
{
	return writeTargetStream(TARGET_DOCUMENT, "<w:sectPr>");
}

/**
 * Finishes exporting the OXML_Section object's properties
 */
UT_Error IE_Exp_OpenXML::finishSectionProperties()
{
	return writeTargetStream(TARGET_DOCUMENT, "</w:sectPr>");
}

/**
 * Starts exporting the OXML_Element_Paragraph object
 */
UT_Error IE_Exp_OpenXML::startParagraph(int target)
{
	return writeTargetStream(target, "<w:p>");
}

/**
 * Finishes exporting the OXML_Element_Paragraph object
 */
UT_Error IE_Exp_OpenXML::finishParagraph(int target)
{
	return writeTargetStream(target, "</w:p>");
}

/**
 * Starts exporting the OXML_Element_Text object
 */
UT_Error IE_Exp_OpenXML::startText(int target)
{
	if(isOverline)
	{
		return writeTargetStream(target, "<w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> EQ \\x \\to(");
	}
	else
	{
		return writeTargetStream(target, "<w:t xml:space=\"preserve\">");
	}
}

/**
 * Writes the actual content of OXML_Element_Text object
 */
UT_Error IE_Exp_OpenXML::writeText(int target, const UT_UCS4Char* text, bool list)
{
	// This shouldn't happen, but if it does just return UT_OK
	// to prevent export errors
	UT_return_val_if_fail(text, UT_OK);

	UT_uint32 len = UT_UCS4_strlen(text);

	UT_UTF8String sEscText;
	sEscText.reserve(len);

	const UT_UCS4Char* pText;
	for(pText = text; pText < text + len; pText++)
	{
		// Skipping first tab character of list element
		if(list && pText == text && *pText == '\t')
		{
			continue;
		}

		switch(*pText)
		{
			// any other special handling needed?

			default:

				if((*pText >= 0x20 && *pText != 0x7f) ||
					(*pText == '\n' || *pText == '\r' || *pText == '\t'))
				{
					sEscText.appendUCS4(pText, 1);
				}
				else
				{
					xxx_UT_DEBUGMSG(("OOXML export: dropping character (%d)\n", *pText));
				}
		}
	}

	sEscText.escapeXML();

	return writeTargetStream(target, sEscText.utf8_str());
}

/**
 * Finishes exporting the OXML_Element_Text object
 */
UT_Error IE_Exp_OpenXML::finishText(int target)
{
	if(isOverline)
	{
		return writeTargetStream(target, ") </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"end\"/>");
	}
	else
	{
		return writeTargetStream(target, "</w:t>");
	}
}

/**
 * Starts exporting the OXML_Element_Math object
 */
UT_Error IE_Exp_OpenXML::startMath()
{
    return writeTargetStream(TARGET_DOCUMENT, "<m:oMathPara>");
}

/**
 * Writes the actual content of OXML_Element_Math object
 */
UT_Error IE_Exp_OpenXML::writeMath(const char* omml)
{
    std::string str;
    str.assign(omml);
    return writeTargetStream(TARGET_DOCUMENT, str.c_str());
}

/**
 * Finishes exporting the OXML_Element_Math object
 */
UT_Error IE_Exp_OpenXML::finishMath()
{
    return writeTargetStream(TARGET_DOCUMENT, "</m:oMathPara>");
}

/**
 * Starts exporting the OXML_Element_Run object
 */
UT_Error IE_Exp_OpenXML::startRun(int target)
{
	return writeTargetStream(target, "<w:r>");
}

/**
 * Finishes exporting the OXML_Element_Run object
 */
UT_Error IE_Exp_OpenXML::finishRun(int target)
{
	isOverline = false;
	return writeTargetStream(target, "</w:r>");
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
 * Starts exporting the OXML_Element_List properties
 */
UT_Error IE_Exp_OpenXML::startListProperties(int target)
{
	return writeTargetStream(target, "<w:numPr>");
}

/**
 * Finishes exporting the OXML_Element_List properties
 */
UT_Error IE_Exp_OpenXML::finishListProperties(int target)
{
	return writeTargetStream(target, "</w:numPr>");
}

/**
 * Starts exporting the OXML_List abstract numbering
 */
UT_Error IE_Exp_OpenXML::startAbstractNumbering(int target, UT_uint32 id)
{
	char buffer[12]; 
	int len = snprintf(buffer, 12, "%d", id);
	if(len <= 0)
		return UT_IE_COULDNOTWRITE;

	std::string str("<w:abstractNum w:abstractNumId=\"");
	str += buffer;
	str += "\">";
	return writeTargetStream(target, str.c_str());
}

/**
 * Finishes exporting the OXML_List abstract numbering
 */
UT_Error IE_Exp_OpenXML::finishAbstractNumbering(int target)
{
	return writeTargetStream(target, "</w:abstractNum>");
}

/**
 * Starts exporting the OXML_List numbering
 */
UT_Error IE_Exp_OpenXML::startNumbering(int target, UT_uint32 id)
{
	char buffer[12]; 
	int len = snprintf(buffer, 12, "%d", id);
	if(len <= 0)
		return UT_IE_COULDNOTWRITE;

	std::string str("<w:num w:numId=\"");
	str += buffer;
	str += "\">";
	return writeTargetStream(target, str.c_str());
}

/**
 * Finishes exporting the OXML_List numbering definition
 */
UT_Error IE_Exp_OpenXML::finishNumbering(int target)
{
	return writeTargetStream(target, "</w:num>");
}

/**
 * Starts exporting the OXML_List abstract numbering level
 */
UT_Error IE_Exp_OpenXML::startNumberingLevel(int target, UT_uint32 id)
{
	char buffer[12]; 
	int len = snprintf(buffer, 12, "%d", id);
	if(len <= 0)
		return UT_IE_COULDNOTWRITE;

	std::string str("<w:lvl w:ilvl=\"");
	str += buffer;
	str += "\">";
	return writeTargetStream(target, str.c_str());
}

/**
 * Finishes exporting the OXML_List abstract numbering level
 */
UT_Error IE_Exp_OpenXML::finishNumberingLevel(int target)
{
	return writeTargetStream(target, "</w:lvl>");
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
 * Starts exporting the OXML_Element_Hyperlink object
 */
UT_Error IE_Exp_OpenXML::startExternalHyperlink(const gchar* id)
{
	std::string str("<w:hyperlink r:id=\"");
	str += id;
	str += "\">";
	return writeTargetStream(TARGET_DOCUMENT, str.c_str());
}

/**
 * Starts exporting the OXML_Element_Hyperlink object
 */
UT_Error IE_Exp_OpenXML::startInternalHyperlink(const gchar* anchor)
{
	UT_UTF8String sEscAnchor = anchor;
	sEscAnchor.escapeXML();

	std::string str("<w:hyperlink w:anchor=\"");
	str += sEscAnchor.utf8_str();
	str += "\">";
	return writeTargetStream(TARGET_DOCUMENT, str.c_str());
}

/**
 * Finishes exporting the OXML_Element_Hyperlink object
 */
UT_Error IE_Exp_OpenXML::finishHyperlink()
{
	return writeTargetStream(TARGET_DOCUMENT, "</w:hyperlink>");
}

/**
 * Exports the OXML_Element_BookmarkStart object
 */
UT_Error IE_Exp_OpenXML::startBookmark(const gchar* id, const gchar* name)
{
	UT_UTF8String sEscName = name;
	sEscName.escapeXML();

	std::string str("<w:bookmarkStart w:id=\"");
	str += id;
	str += "\" ";
	str += "w:name=\"";
	str += sEscName.utf8_str();
	str += "\"/>";
	return writeTargetStream(TARGET_DOCUMENT, str.c_str());
}

/**
 * Exports the OXML_Element_BookmarkFinish object
 */
UT_Error IE_Exp_OpenXML::finishBookmark(const gchar* id)
{
	std::string str("<w:bookmarkEnd w:id=\"");
	str += id;
	str += "\"/>";
	return writeTargetStream(TARGET_DOCUMENT, str.c_str());
}

/**
 * Starts exporting the OXML_Element_TextBox object
 */
UT_Error IE_Exp_OpenXML::startTextBox(int target, const gchar* id)
{
	std::string str("");
	str += "<w:pict>";
	str += "<v:shape w:id=\"";
	str += id;
	str += "\" ";
	return writeTargetStream(target, str.c_str());
}

/**
 * Finishes exporting the OXML_Element_TextBox object
 */
UT_Error IE_Exp_OpenXML::finishTextBox(int target)
{
	std::string str("");
	str += "</v:shape>";
	str += "</w:pict>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Starts exporting the OXML_Element_TextBox object's properties
 */
UT_Error IE_Exp_OpenXML::startTextBoxProperties(int target)
{
	std::string str("");
	str += "style=\"";
	return writeTargetStream(target, str.c_str());
}

/**
 * Finishes exporting the OXML_Element_TextBox object's properties
 */
UT_Error IE_Exp_OpenXML::finishTextBoxProperties(int target)
{
	std::string str("");
	str += "\">";
	return writeTargetStream(target, str.c_str());
}

/**
 * Starts exporting the OXML_Element_TextBox object's content
 */
UT_Error IE_Exp_OpenXML::startTextBoxContent(int target)
{
	std::string str("<v:textbox>");
	str += "<w:txbxContent>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Finishes exporting the OXML_Element_TextBox object's content
 */
UT_Error IE_Exp_OpenXML::finishTextBoxContent(int target)
{
	std::string str("</w:txbxContent>");
	str += "</v:textbox>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets textbox width
 */
UT_Error IE_Exp_OpenXML::setTextBoxWidth(int target, const gchar* width)
{
	std::string str("width:");
	str += convertToPoints(width);
	str += "pt;";	
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets textbox height
 */
UT_Error IE_Exp_OpenXML::setTextBoxHeight(int target, const gchar* height)
{
	std::string str("height:");
	str += convertToPoints(height);
	str += "pt;";	
	return writeTargetStream(target, str.c_str());
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
		case TARGET_NUMBERING:
			return numberingStream;
		case TARGET_HEADER:
			return headerStream;
		case TARGET_FOOTER:
			return footerStream;
		case TARGET_SETTINGS:
			return settingsStream;
		case TARGET_FOOTNOTE:
			return footnoteStream;
		case TARGET_ENDNOTE:
			return endnoteStream;
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
UT_Error IE_Exp_OpenXML::setOverline()
{
	isOverline = true;
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
	UT_UTF8String sEscFamily = family;
	sEscFamily.escapeXML();

	std::string str("<w:rFonts w:ascii=\"");
	str += sEscFamily.utf8_str();
	str += "\" w:cs=\"";
	str += sEscFamily.utf8_str();
	str += "\" w:hAnsi=\"";
	str += sEscFamily.utf8_str();
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets language
 */
UT_Error IE_Exp_OpenXML::setLanguage(int target, const gchar* lang)
{
	UT_UTF8String sEscLang = lang;
	sEscLang.escapeXML();

	std::string str("<w:lang w:val=\"");
	str += sEscLang.utf8_str();
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets no proof
 */
UT_Error IE_Exp_OpenXML::setNoProof(int target)
{
	return writeTargetStream(target, "<w:noProof/>");
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
	UT_UTF8String sEscStyle = style;
	sEscStyle.escapeXML();

	std::string str("<w:pStyle w:val=\"");
	str += sEscStyle.utf8_str();
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
 * Sets the necessary relationship for hyperlink target address
 */
UT_Error IE_Exp_OpenXML::setHyperlinkRelation(int target, const char* id, const char* addr, const char* mode)
{
	UT_UTF8String sEscAddr = addr;
	sEscAddr.escapeURL();

	std::string str("<Relationship Id=\"");
	str += id;
	str += "\" ";
	str += "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink\" ";
	str += "Target=\"";
	str += sEscAddr.utf8_str();
	str += "\" ";
	str += "TargetMode=\"";
	str += mode;
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets the necessary relationships for header
 */
UT_Error IE_Exp_OpenXML::setHeaderRelation(const char* relId, const char* headerId)
{
	UT_Error err = UT_OK;

	std::string str("<Relationship Id=\"");
	str += relId;
	str += "\" ";
	str += "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/header\" ";
	str += "Target=\"header";
	str += headerId;
	str += ".xml\"/>";

	err = writeTargetStream(TARGET_DOCUMENT_RELATION, str.c_str());
	if(err != UT_OK)
		return err;

	str = "";
	str += "<Override PartName=\"/word/header";
	str += headerId;
	str += ".xml\" ";
	str += "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml\"/>";

	return writeTargetStream(TARGET_CONTENT, str.c_str());
}

/**
 * Sets the titlePg tag for the first page headers/footers
 */
UT_Error IE_Exp_OpenXML::setTitlePage()
{
	return writeTargetStream(TARGET_DOCUMENT, "<w:titlePg/>");
}

/**
 * Sets the evenAndOddHeaders tag for the even/odd page headers/footers
 */
UT_Error IE_Exp_OpenXML::setEvenAndOddHeaders()
{
	return writeTargetStream(TARGET_SETTINGS, "<w:evenAndOddHeaders/>");
}

/**
 * Sets the necessary relationship for footer
 */
UT_Error IE_Exp_OpenXML::setFooterRelation(const char* relId, const char* footerId)
{
	UT_Error err = UT_OK;

	std::string str("<Relationship Id=\"");
	str += relId;
	str += "\" ";
	str += "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer\" ";
	str += "Target=\"footer";
	str += footerId;
	str += ".xml\"/>";

	err = writeTargetStream(TARGET_DOCUMENT_RELATION, str.c_str());
	if(err != UT_OK)
		return err;

	str = "";
	str += "<Override PartName=\"/word/footer";
	str += footerId;
	str += ".xml\" ";
	str += "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml\"/>";

	return writeTargetStream(TARGET_CONTENT, str.c_str());
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
 * Sets tab stops
 */
UT_Error IE_Exp_OpenXML::setTabstops(int target, const gchar* tabstops)
{
	std::string tabs("<w:tabs>");

	std::string str("");
	str += tabstops;
	str += ",";

	std::string::size_type prev = -1;
	std::string::size_type pos = str.find_first_of(",");
		
	while (pos != std::string::npos) 
	{
		std::string token("");
		token = str.substr(prev+1, pos-prev-1);

		std::string::size_type typePos = token.find_first_of("/");

		if(typePos != std::string::npos)
		{
			std::string tabStopType = token.substr(typePos+1, 1);
			std::string type = token.substr(typePos+2, token.length()-1);		
			token = token.substr(0, typePos);

			if(strstr(tabStopType.c_str(), "L"))
				tabs += "<w:tab w:val=\"left\" ";
			else if(strstr(tabStopType.c_str(), "R"))
				tabs += "<w:tab w:val=\"right\" ";
			else if(strstr(tabStopType.c_str(), "C"))
				tabs += "<w:tab w:val=\"center\" ";
			else if(strstr(tabStopType.c_str(), "D"))
				tabs += "<w:tab w:val=\"decimal\" ";
			else if(strstr(tabStopType.c_str(), "B"))
				tabs += "<w:tab w:val=\"bar\" ";
			else
				tabs += "<w:tab w:val=\"clear\" ";


			if(strstr(type.c_str(), "3"))
				tabs += "w:leader=\"underscore\" ";
			else if(strstr(type.c_str(), "1"))
				tabs += "w:leader=\"dot\" ";
			else if(strstr(type.c_str(), "2"))
				tabs += "w:leader=\"hyphen\" ";
		
			tabs += "w:pos=\"";
			tabs += convertToPositiveTwips(token.c_str());
			tabs += "\"/>";	
		}
	
		prev = pos;	
		pos = str.find_first_of(",", pos + 1);
	}

	tabs += "</w:tabs>";

	return writeTargetStream(target, tabs.c_str());
}

/**
 * Sets the columns for the section
 */
UT_Error IE_Exp_OpenXML::setColumns(int target, const gchar* num, const gchar* sep)
{
	if(UT_convertDimensionless(num) <= 0)
		return UT_OK;

	if((strcmp(sep, "on") != 0) && (strcmp(sep, "off") != 0))
	{
		// this code should never be reached due to the string checks in
		// OXML_Section::serializeProperties()
		UT_ASSERT_NOT_REACHED();
		return UT_OK;
	}

	std::string str("");
	str += "<w:cols ";
	str += "w:num=\"";
	str += num;
	str += "\" ";
	str += "w:sep=\"";
	str += sep;
	str += "\" ";
	str += "w:equalWidth=\"1\"/>";

	return writeTargetStream(target, str.c_str());
}

/**
 * Sets the section type continuous
 */
UT_Error IE_Exp_OpenXML::setContinuousSection(int target)
{
	std::string str("");
	str += "<w:type w:val=\"continuous\"/>";
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
	std::string str("<w:vMerge w:val=\"");
	str += vmerge;
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}


/**
 * Sets page break
 */
UT_Error IE_Exp_OpenXML::setPageBreak(int target)
{
	std::string str("<w:pageBreakBefore/>");
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets page size and orientation
 */
UT_Error IE_Exp_OpenXML::setPageSize(int target, const char* width, const char* height, const char* orientation)
{
	std::string str("<w:pgSz w:w=\"");
	str += width;
	str += "\"";

	str += " w:h=\"";
	str += height;
	str += "\"";

	str += " w:orient=\"";
	str += orientation;
	str += "\"/>";

	return writeTargetStream(target, str.c_str());	
}

/**
 * Sets page margins
 */
UT_Error IE_Exp_OpenXML::setPageMargins(int target, const char* top, const char* left, const char* right, const char* bottom)
{
	std::string str("<w:pgMar w:top=\"");
	str += convertToTwips(top);
	str += "\"";

	str += " w:left=\"";
	str += convertToTwips(left);
	str += "\"";

	str += " w:right=\"";
	str += convertToTwips(right);
	str += "\"";

	str += " w:bottom=\"";
	str += convertToTwips(bottom);
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
 * Sets table row height to some exact value
 */
UT_Error IE_Exp_OpenXML::setRowHeight(int target, const char* height)
{
	std::string str("<w:trHeight w:val=\"");
	str += convertToPositiveTwips(height);
	str += "\" w:hRule=\"exact\"/>";
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
 * Starts table row properties
 */
UT_Error IE_Exp_OpenXML::startRowProperties(int target)
{
	return writeTargetStream(target, "<w:trPr>");
}

/**
 * Finishes table row properties
 */
UT_Error IE_Exp_OpenXML::finishRowProperties(int target)
{
	return writeTargetStream(target, "</w:trPr>");
}

/**
 * Starts footnote
 */
UT_Error IE_Exp_OpenXML::startFootnote(const char* id)
{
	std::string str("<w:footnote w:id=\"");
	str += id;
	str += "\">";
	return writeTargetStream(TARGET_FOOTNOTE, str.c_str());
}

/**
 * Finishes footnote
 */
UT_Error IE_Exp_OpenXML::finishFootnote()
{
	return writeTargetStream(TARGET_FOOTNOTE, "</w:footnote>");
}

/**
 * Starts endnote
 */
UT_Error IE_Exp_OpenXML::startEndnote(const char* id)
{
	std::string str("<w:endnote w:id=\"");
	str += id;
	str += "\">";
	return writeTargetStream(TARGET_ENDNOTE, str.c_str());
}

/**
 * Finishes endnote
 */
UT_Error IE_Exp_OpenXML::finishEndnote()
{
	return writeTargetStream(TARGET_ENDNOTE, "</w:endnote>");
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
 * Sets list level
 */
UT_Error IE_Exp_OpenXML::setListLevel(int target, const char* level)
{
	std::string str("<w:ilvl w:val=\"");
	str += level;
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets list format
 */
UT_Error IE_Exp_OpenXML::setListFormat(int target, const char* format)
{
	std::string str("<w:numId w:val=\"");
	str += format;
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets list type
 */
UT_Error IE_Exp_OpenXML::setListType(int target, const char* type)
{
	std::string str("<w:numFmt w:val=\"");
	str += type;
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets the start value of the list
 */
UT_Error IE_Exp_OpenXML::setListStartValue(int target, UT_uint32 startValue)
{
	char buffer[12]; 
	int len = snprintf(buffer, 12, "%d", startValue);
	if(len <= 0)
		return UT_IE_COULDNOTWRITE;

	std::string str("<w:start w:val=\"");
	str += buffer;
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets list level text
 */
UT_Error IE_Exp_OpenXML::setListLevelText(int target, const char* text)
{
	UT_UTF8String sEscText = text;
	if(!isListBullet(text))
		sEscText.escapeXML();

	std::string str("<w:lvlText w:val=\"");
	str += sEscText.utf8_str();
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Checks whether given a string is a special list bullet symbol
 */
bool IE_Exp_OpenXML::isListBullet(const char* str)
{
	return !strcmp(str, BULLET) || !strcmp(str, SQUARE) || !strcmp(str, TRIANGLE) || !strcmp(str, TICK) || !strcmp(str, IMPLIES) || 
			!strcmp(str, DIAMOND) || !strcmp(str, BOX) || !strcmp(str, HAND) || !strcmp(str, HEART) || !strcmp(str, DASH);
}

/**
 * Sets abstract numbering id
 */
UT_Error IE_Exp_OpenXML::setAbstractNumberingId(int target, UT_uint32 id)
{
	char buffer[12]; 
	int len = snprintf(buffer, 12, "%d", id);
	if(len <= 0)
		return UT_IE_COULDNOTWRITE;

	std::string str("<w:abstractNumId w:val=\"");
	str += buffer;
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets the numbering format of the list
 */
UT_Error IE_Exp_OpenXML::setNumberingFormat(int target, const char* format)
{
	std::string str("<w:numFmt w:val=\"");
	str += format;
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets the multilevel type of the list
 */
UT_Error IE_Exp_OpenXML::setMultilevelType(int target, const char* type)
{
	std::string str("<w:multiLevelType w:val=\"");
	str += type;
	str += "\"/>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Sets the inline image
 */
UT_Error IE_Exp_OpenXML::setImage(const char* id, const char* relId, const char* filename, const char* width, const char* height)
{
	std::string str("");
	std::string h("");
	std::string w("");

	h += convertToPositiveEmus(height);
	w += convertToPositiveEmus(width);

	str += "<w:drawing>";
	str += "<wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\">";
	str += "<wp:extent cx=\"";
	str += w;
	str += "\" cy=\"";
	str += h;
	str += "\"/>";
	str += "<wp:docPr id=\"";
	str += id;
	str += "\" name=\"";
	str += filename;
	str += "\"/>";
	str += "<a:graphic>";
	str += "<a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">";
	str += "<pic:pic>";
	str += "<pic:nvPicPr>";
	str += "<pic:cNvPr id=\"";
	str += id;
	str += "\" name=\"";
	str += filename;
	str += "\"/>";
	str += "<pic:cNvPicPr/>";
	str += "</pic:nvPicPr>";
	str += "<pic:blipFill>";
	str += "<a:blip r:embed=\"";
	str += relId;
	str += "\"/>";
	str += "</pic:blipFill>";
	str += "<pic:spPr>";
	str += "<a:xfrm>";
	str += "<a:off x=\"0\" y=\"0\"/>";
	str += "<a:ext cx=\"";
	str += w;
	str += "\" cy=\"";
	str += h;
	str += "\"/>";
	str += "</a:xfrm>";
	str += "<a:prstGeom prst=\"rect\">";
	str += "<a:avLst/>";
	str += "</a:prstGeom>";
	str += "</pic:spPr>";
	str += "</pic:pic>";
	str += "</a:graphicData>";
	str += "</a:graphic>";
	str += "</wp:inline>";
	str += "</w:drawing>";

	return writeTargetStream(TARGET_DOCUMENT, str.c_str());
}

/**
 * Sets the positioned image
 */
UT_Error IE_Exp_OpenXML::setPositionedImage(const char* id, const char* relId, const char* filename, const char* width, const char* height, const char* xpos, const char* ypos, const char* wrapMode)
{
	std::string str("");
	std::string h("");
	std::string w("");
	std::string x("");
	std::string y("");
	std::string wm("bothSides"); // default wrap mode

	if(!strcmp(wrapMode, "wrapped-to-right"))
	{
		wm = "right";
	}
	else if(!strcmp(wrapMode, "wrapped-to-left"))
	{
		wm = "left";
	}

	h += convertToPositiveEmus(height);
	w += convertToPositiveEmus(width);
	x += convertToPositiveEmus(xpos);
	y += convertToPositiveEmus(ypos);

	str += "<w:drawing>";
	str += "<wp:anchor distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\" simplePos=\"0\" allowOverlap=\"0\" layoutInCell=\"1\" locked=\"0\" behindDoc=\"0\" relativeHeight=\"0\">";
	str += "<wp:simplePos x=\"0\" y=\"0\"/>";
	str += "<wp:positionH relativeFrom=\"column\">";
	str += "<wp:posOffset>";
	str += x;
	str += "</wp:posOffset>";
	str += "</wp:positionH>";
	str += "<wp:positionV relativeFrom=\"paragraph\">";
	str += "<wp:posOffset>";
	str += y;
	str += "</wp:posOffset>";
	str += "</wp:positionV>";
	str += "<wp:extent cx=\"";
	str += w;
	str += "\" cy=\"";
	str += h;
	str += "\"/>";
	str += "<wp:effectExtent l=\"0\" t=\"0\" r=\"0\" b=\"0\"/>";
	str += "<wp:wrapSquare wrapText=\"";
	str += wm;
	str += "\"/>";
	str += "<wp:docPr id=\"";
	str += id;
	str += "\" name=\"";
	str += filename;
	str += "\"/>";
	str += "<wp:cNvGraphicFramePr>";
	str += "<a:graphicFrameLocks noChangeAspect=\"1\"/>";
	str += "</wp:cNvGraphicFramePr>";
	str += "<a:graphic>";
	str += "<a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">";
	str += "<pic:pic>";
	str += "<pic:nvPicPr>";
	str += "<pic:cNvPr id=\"";
	str += id;
	str += "\" name=\"";
	str += filename;
	str += "\"/>";
	str += "<pic:cNvPicPr/>";
	str += "</pic:nvPicPr>";
	str += "<pic:blipFill>";
	str += "<a:blip r:embed=\"";
	str += relId;
	str += "\"/>";
	str += "</pic:blipFill>";
	str += "<pic:spPr>";
	str += "<a:xfrm>";
	str += "<a:off x=\"0\" y=\"0\"/>";
	str += "<a:ext cx=\"";
	str += w;
	str += "\" cy=\"";
	str += h;
	str += "\"/>";
	str += "</a:xfrm>";
	str += "<a:prstGeom prst=\"rect\">";
	str += "<a:avLst/>";
	str += "</a:prstGeom>";
	str += "</pic:spPr>";
	str += "</pic:pic>";
	str += "</a:graphicData>";
	str += "</a:graphic>";
	str += "</wp:anchor>";
	str += "</w:drawing>";

	return writeTargetStream(TARGET_DOCUMENT, str.c_str());
}

/**
 * Sets the relation of the image 
 */
UT_Error IE_Exp_OpenXML::setImageRelation(const char* filename, const char* id)
{
	std::string str("<Relationship Id=\"");
	str += id;
	str += "\" ";
	str += "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/image\" ";
	str += "Target=\"media/";
	str += filename;
	str += "\"/>";

	return writeTargetStream(TARGET_DOCUMENT_RELATION, str.c_str());
}

/**
 * Sets the simple field
 */
UT_Error IE_Exp_OpenXML::setSimpleField(int target, const char* instr, const char* value)
{
	UT_UTF8String sEscInstr = instr;
	sEscInstr.escapeXML();
	UT_UTF8String sEscValue = value;
	sEscValue.escapeXML();
	
	std::string str("");
	str += "<w:fldSimple w:instr=\"";
	str += sEscInstr.utf8_str();
	str += "\">";
	str += "<w:r>";
	str += "<w:t>";
	str += sEscValue.utf8_str();
	str += "</w:t>";
	str += "</w:r>";
	str += "</w:fldSimple>";
	return writeTargetStream(target, str.c_str());
}

/**
 * Set the header reference
 */
UT_Error IE_Exp_OpenXML::setHeaderReference(const char* id, const char* type)
{
	std::string str("");
	str += "<w:headerReference w:type=\"";
	str += type;
	str += "\" ";
	str += "r:id=\"";
	str += id;
	str += "\"/>";
	return writeTargetStream(TARGET_DOCUMENT, str.c_str());
}

/**
 * Set the footnote reference
 */
UT_Error IE_Exp_OpenXML::setFootnoteReference(const char* id)
{
	std::string str("");
	str += "<w:footnoteReference ";
	str += "w:id=\"";
	str += id;
	str += "\"/>";
	return writeTargetStream(TARGET_DOCUMENT, str.c_str());
}

/**
 * Set the footnoteRef tag
 */
UT_Error IE_Exp_OpenXML::setFootnoteRef()
{
	std::string str("");
	str += "<w:footnoteRef/>";
	return writeTargetStream(TARGET_FOOTNOTE, str.c_str());
}

/**
 * Set the endnote reference
 */
UT_Error IE_Exp_OpenXML::setEndnoteReference(const char* id)
{
	std::string str("");
	str += "<w:endnoteReference ";
	str += "w:id=\"";
	str += id;
	str += "\"/>";
	return writeTargetStream(TARGET_DOCUMENT, str.c_str());
}

/**
 * Set the endnoteRef tag
 */
UT_Error IE_Exp_OpenXML::setEndnoteRef()
{
	std::string str("");
	str += "<w:endnoteRef/>";
	return writeTargetStream(TARGET_ENDNOTE, str.c_str());
}

/**
 * Set the footer reference
 */
UT_Error IE_Exp_OpenXML::setFooterReference(const char* id, const char* type)
{
	std::string str("");
	str += "<w:footerReference w:type=\"";
	str += type;
	str += "\" ";
	str += "r:id=\"";
	str += id;
	str += "\"/>";
	return writeTargetStream(TARGET_DOCUMENT, str.c_str());
}

/**
 * Checks whether the quantity string is a negative quantity
 */
bool IE_Exp_OpenXML::isNegativeQuantity(const gchar* quantity)
{
	return *quantity == '-';
}

/**
 * Converts the string str to EMUs, returns non-negative whole number
 */
const gchar * IE_Exp_OpenXML::convertToPositiveEmus(const gchar* str)
{
	//1 inch = 914400 EMUs
	double emu = UT_convertToInches(str) * 914400;
	if(emu < 1.0) 
		return "0";
	return UT_convertToDimensionlessString(emu, ".0");
}

/**
 * Converts the string str to points
 */
const gchar * IE_Exp_OpenXML::convertToPoints(const gchar* str)
{
	double pt = UT_convertToPoints(str);
	return UT_convertToDimensionlessString(pt, ".0");
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
		pt = 0.0;
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
	double pt = UT_convertToPoints(str) * 8;
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
	m_pDoc = NULL;

	if(footnoteStream && !gsf_output_is_closed(footnoteStream))
		gsf_output_close(footnoteStream);

	if(endnoteStream && !gsf_output_is_closed(endnoteStream))
		gsf_output_close(endnoteStream);

	if(settingsStream && !gsf_output_is_closed(settingsStream))
		gsf_output_close(settingsStream);

	if(headerStream && !gsf_output_is_closed(headerStream))
		gsf_output_close(headerStream);
	
	if(footerStream && !gsf_output_is_closed(footerStream))
		gsf_output_close(footerStream);
	
	if(numberingStream && !gsf_output_is_closed(numberingStream))
		gsf_output_close(numberingStream);

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

	if(wordMediaDir)
	{
		GsfOutput* wordMedia_out = GSF_OUTPUT(wordMediaDir);
		if(!gsf_output_is_closed(wordMedia_out))
			gsf_output_close(wordMedia_out);
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
 * Starts the numbering.xml file which describes the default list styles
 */
UT_Error IE_Exp_OpenXML::startNumbering()
{
	UT_Error err = UT_OK;

	numberingStream = gsf_output_memory_new();

	if(!numberingStream)
	{
		UT_DEBUGMSG(("FRT: ERROR, numbering.xml file couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}

	err = writeXmlHeader(numberingStream);
	if(err != UT_OK)
	{
		return err;
	}	

	std::string str("<w:numbering ");
	str += "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"";
	str += ">";

	return writeTargetStream(TARGET_NUMBERING, str.c_str());
}

/**
 * Finishes the numbering.xml file which describes the contents of the package
 */
UT_Error IE_Exp_OpenXML::finishNumbering()
{
	UT_Error err = UT_OK;

	err = writeTargetStream(TARGET_NUMBERING, "</w:numbering>");
	if(err != UT_OK)
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write to numbering.xml file\n"));	
		return err;
	}

	GsfOutput* numberingFile = gsf_outfile_new_child(wordDir, "numbering.xml", FALSE);

	if(!numberingFile)
		return UT_SAVE_EXPORTERROR;		

 	if(!gsf_output_write(numberingFile, gsf_output_size(numberingStream), 
					 gsf_output_memory_get_bytes(GSF_OUTPUT_MEMORY(numberingStream))))
	{
		gsf_output_close(numberingFile);
		return UT_SAVE_EXPORTERROR;		
	}

	if(!gsf_output_close(numberingStream))
	{
		gsf_output_close(numberingFile);
		return UT_SAVE_EXPORTERROR;		
	}

	if(!gsf_output_close(numberingFile))
	{
		UT_DEBUGMSG(("FRT: ERROR, numbering.xml file couldn't be closed\n"));	
		return UT_SAVE_EXPORTERROR;		
	}
	return UT_OK;
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
 * Finishes the styles.xml file which describes the contents of the package
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
	str += "<Default Extension=\"png\" ContentType=\"image/png\"/>";
	str += "<Default Extension=\"jpg\" ContentType=\"image/jpeg\"/>";
	str += "<Default Extension=\"jpeg\" ContentType=\"image/jpeg\"/>";
	str += "<Default Extension=\"gif\" ContentType=\"image/gif\"/>";
	str += "<Default Extension=\"tiff\" ContentType=\"image/tiff\"/>";
	str += "<Default Extension=\"svg\" ContentType=\"image/svg+xml\"/>";
	str += "<Override PartName=\"/word/document.xml\" ";
	str += "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/>";
	str += "<Override PartName=\"/word/styles.xml\" ";
	str += "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml\"/>";
	str += "<Override PartName=\"/word/settings.xml\" ";
	str += "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml\"/>";
	str += "<Override PartName=\"/word/numbering.xml\" ";
	str += "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml\"/>";
	str += "<Override PartName=\"/word/footnotes.xml\" ";
	str += "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.footnotes+xml\"/>";
	str += "<Override PartName=\"/word/endnotes.xml\" ";
	str += "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.endnotes+xml\"/>";

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
	str += "<Relationship Id=\"rId2\" ";
	str += "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering\" ";
	str += "Target=\"numbering.xml\"/>";
	str += "<Relationship Id=\"rId3\" ";
	str += "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings\" ";
	str += "Target=\"settings.xml\"/>";
	str += "<Relationship Id=\"rId4\" ";
	str += "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/footnotes\" ";
	str += "Target=\"footnotes.xml\"/>";
	str += "<Relationship Id=\"rId5\" ";
	str += "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/endnotes\" ";
	str += "Target=\"endnotes.xml\"/>";

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
 * Does nothing for now. 
 * If we need a default file in word/media folder we should create the necessary stream here.
 */
UT_Error IE_Exp_OpenXML::startWordMedia()
{
	return UT_OK;
}

/**
 * Exports all the image streams to actual files in the word/media folder
 */
UT_Error IE_Exp_OpenXML::finishWordMedia()
{
	wordMediaDir = GSF_OUTFILE(gsf_outfile_new_child(wordDir, "media", TRUE)); 
	if(!wordMediaDir)
	{
		UT_DEBUGMSG(("FRT: ERROR, word/media directory couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}

	std::map<std::string, GsfOutput*>::iterator it;
	for (it = mediaStreams.begin(); it != mediaStreams.end(); it++) {

		GsfOutput* imageFile = gsf_outfile_new_child(wordMediaDir, it->first.c_str(), FALSE);

		if(!imageFile)
			return UT_SAVE_EXPORTERROR;

	 	if(!gsf_output_write(imageFile, gsf_output_size(it->second), 
						 gsf_output_memory_get_bytes(GSF_OUTPUT_MEMORY(it->second))))
		{
			gsf_output_close(imageFile);
			return UT_SAVE_EXPORTERROR;
		}
	
		if(!gsf_output_close(it->second))
		{
			gsf_output_close(imageFile);
			return UT_SAVE_EXPORTERROR;		
		}

		if(!gsf_output_close(imageFile))
		{
			UT_DEBUGMSG(("FRT: ERROR, image file couldn't be closed\n"));	
			return UT_SAVE_EXPORTERROR;		
		}
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

	std::string str("<w:document xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" ");
	str += "xmlns:v=\"urn:schemas-microsoft-com:vml\" ";
	str += "xmlns:wx=\"http://schemas.microsoft.com/office/word/2003/auxHint\" ";
	str += "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" ";
	str += "xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\" ";
	str += "xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" ";
	str += "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\" ";
	str += "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body>";
	
	return writeTargetStream(TARGET_DOCUMENT, str.c_str());
}

/**
 * Finishes the main part of the document to word/document.xml file.
 */
UT_Error IE_Exp_OpenXML::finishMainPart()
{
	UT_Error err = UT_OK;

	err = writeTargetStream(TARGET_DOCUMENT, "</w:body></w:document>");
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
 * Starts the settings of the document in word/settings.xml file.
 */
UT_Error IE_Exp_OpenXML::startSettings()
{
	UT_Error err = UT_OK;

	settingsStream = gsf_output_memory_new();
	if(!settingsStream)
	{
		UT_DEBUGMSG(("FRT: ERROR, settings.xml file couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}	

	err = writeXmlHeader(settingsStream);
	if(err != UT_OK)
	{
		return err;
	}	

	std::string str("<w:settings xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" ");
	str += "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">";
	
	return writeTargetStream(TARGET_SETTINGS, str.c_str());
}

/**
 * Finishes the settings of the document in word/setting.xml file.
 */
UT_Error IE_Exp_OpenXML::finishSettings()
{
	UT_Error err = UT_OK;

	err = writeTargetStream(TARGET_SETTINGS, "</w:settings>");
	if(err != UT_OK)
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write to settings.xml file\n"));	
		return err;
	}
	
	GsfOutput* settingsFile = gsf_outfile_new_child(wordDir, "settings.xml", FALSE);

	if(!settingsFile)
		return UT_SAVE_EXPORTERROR;

 	if(!gsf_output_write(settingsFile, gsf_output_size(settingsStream), 
					 gsf_output_memory_get_bytes(GSF_OUTPUT_MEMORY(settingsStream))))
	{
		gsf_output_close(settingsFile);
		return UT_SAVE_EXPORTERROR;
	}
	
	if(!gsf_output_close(settingsStream))
	{
		gsf_output_close(settingsFile);
		return UT_SAVE_EXPORTERROR;		
	}

	if(!gsf_output_close(settingsFile))
	{
		UT_DEBUGMSG(("FRT: ERROR, setting.xml file couldn't be closed\n"));	
		return UT_SAVE_EXPORTERROR;		
	}

	return UT_OK;
}

/**
 * Does nothing for now.
 */
UT_Error IE_Exp_OpenXML::startHeaders()
{
	return UT_OK;
}

/**
 * Finishes the headers in word/header.xml file.
 */
UT_Error IE_Exp_OpenXML::finishHeaders()
{
	std::map<std::string, GsfOutput*>::iterator it;
	for (it = headerStreams.begin(); it != headerStreams.end(); it++) {

		std::string filename("header");
		filename += it->first.c_str();
		filename += ".xml";

		GsfOutput* headerFile = gsf_outfile_new_child(wordDir, filename.c_str(), FALSE);

		if(!headerFile)
			return UT_SAVE_EXPORTERROR;

	 	if(!gsf_output_write(headerFile, gsf_output_size(it->second), 
						 gsf_output_memory_get_bytes(GSF_OUTPUT_MEMORY(it->second))))
		{
			gsf_output_close(headerFile);
			return UT_SAVE_EXPORTERROR;
		}
	
		if(!gsf_output_close(it->second))
		{
			gsf_output_close(headerFile);
			return UT_SAVE_EXPORTERROR;		
		}

		if(!gsf_output_close(headerFile))
		{
			UT_DEBUGMSG(("FRT: ERROR, header file couldn't be closed\n"));	
			return UT_SAVE_EXPORTERROR;		
		}
	}
	
	return UT_OK;
}

/**
 * Does nothing for now.
 */
UT_Error IE_Exp_OpenXML::startFooters()
{
	return UT_OK;
}

/**
 * Finishes the headers in word/footer.xml file.
 */
UT_Error IE_Exp_OpenXML::finishFooters()
{
	std::map<std::string, GsfOutput*>::iterator it;
	for (it = footerStreams.begin(); it != footerStreams.end(); it++) {

		std::string filename("footer");
		filename += it->first.c_str();
		filename += ".xml";

		GsfOutput* footerFile = gsf_outfile_new_child(wordDir, filename.c_str(), FALSE);

		if(!footerFile)
			return UT_SAVE_EXPORTERROR;

	 	if(!gsf_output_write(footerFile, gsf_output_size(it->second), 
						 gsf_output_memory_get_bytes(GSF_OUTPUT_MEMORY(it->second))))
		{
			gsf_output_close(footerFile);
			return UT_SAVE_EXPORTERROR;
		}
	
		if(!gsf_output_close(it->second))
		{
			gsf_output_close(footerFile);
			return UT_SAVE_EXPORTERROR;		
		}

		if(!gsf_output_close(footerFile))
		{
			UT_DEBUGMSG(("FRT: ERROR, footer file couldn't be closed\n"));	
			return UT_SAVE_EXPORTERROR;		
		}
	}
	
	return UT_OK;
}

/**
 * Starts the footnotes.xml file which describes the footnotes
 */
UT_Error IE_Exp_OpenXML::startFootnotes()
{
	UT_Error err = UT_OK;

	footnoteStream = gsf_output_memory_new();

	if(!footnoteStream)
	{
		UT_DEBUGMSG(("FRT: ERROR, footnotes.xml file couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}

	err = writeXmlHeader(footnoteStream);
	if(err != UT_OK)
	{
		return err;
	}	

	std::string str("<w:footnotes ");
	str += "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"";
	str += ">";

	return writeTargetStream(TARGET_FOOTNOTE, str.c_str());
}

/**
 * Finishes the footnotes.xml file 
 */
UT_Error IE_Exp_OpenXML::finishFootnotes()
{
	UT_Error err = UT_OK;

	err = writeTargetStream(TARGET_FOOTNOTE, "</w:footnotes>");
	if(err != UT_OK)
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write to footnotes.xml file\n"));	
		return err;
	}

	GsfOutput* footnoteFile = gsf_outfile_new_child(wordDir, "footnotes.xml", FALSE);

	if(!footnoteFile)
		return UT_SAVE_EXPORTERROR;		

 	if(!gsf_output_write(footnoteFile, gsf_output_size(footnoteStream), 
					 gsf_output_memory_get_bytes(GSF_OUTPUT_MEMORY(footnoteStream))))
	{
		gsf_output_close(footnoteFile);
		return UT_SAVE_EXPORTERROR;		
	}

	if(!gsf_output_close(footnoteStream))
	{
		gsf_output_close(footnoteFile);
		return UT_SAVE_EXPORTERROR;		
	}

	if(!gsf_output_close(footnoteFile))
	{
		UT_DEBUGMSG(("FRT: ERROR, footnotes.xml file couldn't be closed\n"));	
		return UT_SAVE_EXPORTERROR;		
	}
	return UT_OK;
}

/**
 * Starts the endnotes.xml file which describes the endnotes
 */
UT_Error IE_Exp_OpenXML::startEndnotes()
{
	UT_Error err = UT_OK;

	endnoteStream = gsf_output_memory_new();

	if(!endnoteStream)
	{
		UT_DEBUGMSG(("FRT: ERROR, endnotes.xml file couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}

	err = writeXmlHeader(endnoteStream);
	if(err != UT_OK)
	{
		return err;
	}	

	std::string str("<w:endnotes ");
	str += "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"";
	str += ">";

	return writeTargetStream(TARGET_ENDNOTE, str.c_str());
}

/**
 * Finishes the endnotes.xml file 
 */
UT_Error IE_Exp_OpenXML::finishEndnotes()
{
	UT_Error err = UT_OK;

	err = writeTargetStream(TARGET_ENDNOTE, "</w:endnotes>");
	if(err != UT_OK)
	{
		UT_DEBUGMSG(("FRT: ERROR, cannot write to endnotes.xml file\n"));	
		return err;
	}

	GsfOutput* endnoteFile = gsf_outfile_new_child(wordDir, "endnotes.xml", FALSE);

	if(!endnoteFile)
		return UT_SAVE_EXPORTERROR;		

 	if(!gsf_output_write(endnoteFile, gsf_output_size(endnoteStream), 
					 gsf_output_memory_get_bytes(GSF_OUTPUT_MEMORY(endnoteStream))))
	{
		gsf_output_close(endnoteFile);
		return UT_SAVE_EXPORTERROR;		
	}

	if(!gsf_output_close(endnoteStream))
	{
		gsf_output_close(endnoteFile);
		return UT_SAVE_EXPORTERROR;		
	}

	if(!gsf_output_close(endnoteFile))
	{
		UT_DEBUGMSG(("FRT: ERROR, endnotes.xml file couldn't be closed\n"));	
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

UT_Error IE_Exp_OpenXML::startStyle(const std::string& name, const std::string& basedon, const std::string& followedby, const std::string& type)
{
	std::string sEscName = UT_escapeXML(name);
	std::string sEscBasedOn = UT_escapeXML(basedon);
	std::string sEscFollowedBy = UT_escapeXML(followedby);
	std::string sEscType = UT_escapeXML(type);

	std::string str("");
	str += "<w:style";
	if(!type.empty())
	{
		str += " w:type=\"";
		str += sEscType;
		str += "\"";
	}
	str += " w:styleId=\"";
	str += sEscName;
	str += "\">";
	str += "<w:name w:val=\"";
	str += sEscName;
	str += "\"/>";
	
	if(!basedon.empty())
	{
		str += "<w:basedOn w:val=\"";
		str += sEscBasedOn;
		str += "\"/>";		
	}
	if(!followedby.empty())
	{
		str += "<w:next w:val=\"";
		str += sEscFollowedBy;
		str += "\"/>";
	}

	return writeTargetStream(TARGET_STYLES, str.c_str());
}

UT_Error IE_Exp_OpenXML::finishStyle()
{
	return writeTargetStream(TARGET_STYLES, "</w:style>");
}

UT_Error IE_Exp_OpenXML::startDocumentDefaultProperties()
{
	return writeTargetStream(TARGET_STYLES, "<w:docDefaults>");
}

UT_Error IE_Exp_OpenXML::finishDocumentDefaultProperties()
{
	return writeTargetStream(TARGET_STYLES, "</w:docDefaults>");
}

UT_Error IE_Exp_OpenXML::startRunDefaultProperties()
{
	return writeTargetStream(TARGET_STYLES, "<w:rPrDefault>");
}

UT_Error IE_Exp_OpenXML::finishRunDefaultProperties()
{
	return writeTargetStream(TARGET_STYLES, "</w:rPrDefault>");
}

UT_Error IE_Exp_OpenXML::startParagraphDefaultProperties()
{
	return writeTargetStream(TARGET_STYLES, "<w:pPrDefault>");
}

UT_Error IE_Exp_OpenXML::finishParagraphDefaultProperties()
{
	return writeTargetStream(TARGET_STYLES, "</w:pPrDefault>");
}

UT_Error IE_Exp_OpenXML::writeImage(const char* filename, const UT_ConstByteBufPtr & data)
{
	GsfOutput* imageStream = gsf_output_memory_new();

	if(!imageStream)
	{
		UT_DEBUGMSG(("FRT: ERROR, image file couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}	

 	if(!gsf_output_write(imageStream, data->getLength(), data->getPointer(0)))
	{
		gsf_output_close(imageStream);
		return UT_SAVE_EXPORTERROR;		
	}

	std::string str("");
	str += filename;
	mediaStreams[str] = imageStream;
		
	return UT_OK;
}

UT_Error IE_Exp_OpenXML::startHeaderStream(const char* id)
{
	UT_Error err = UT_OK;

	headerStream = gsf_output_memory_new();
	if(!headerStream)
	{
		UT_DEBUGMSG(("FRT: ERROR, header.xml file couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}

	err = writeXmlHeader(headerStream);
	if(err != UT_OK)
		return err;

	std::string str("<w:hdr xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" ");
	str += "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">";

	std::string strId("");
	strId += id;
	headerStreams[strId] = headerStream;
	
	return writeTargetStream(TARGET_HEADER, str.c_str());
}

UT_Error IE_Exp_OpenXML::finishHeaderStream()
{
	return writeTargetStream(TARGET_HEADER, "</w:hdr>");
}

UT_Error IE_Exp_OpenXML::startFooterStream(const char* id)
{
	UT_Error err = UT_OK;

	footerStream = gsf_output_memory_new();
	if(!footerStream)
	{
		UT_DEBUGMSG(("FRT: ERROR, footer.xml file couldn't be created\n"));	
		return UT_SAVE_EXPORTERROR;
	}

	err = writeXmlHeader(footerStream);
	if(err != UT_OK)
		return err;

	std::string str("<w:ftr xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" ");
	str += "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">";

	std::string strId("");
	strId += id;
	footerStreams[strId] = footerStream;
	
	return writeTargetStream(TARGET_FOOTER, str.c_str());
}

UT_Error IE_Exp_OpenXML::finishFooterStream()
{
	return writeTargetStream(TARGET_FOOTER, "</w:ftr>");
}
