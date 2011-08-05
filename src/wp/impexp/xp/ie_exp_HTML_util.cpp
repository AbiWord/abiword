/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiWord
* Copyright (C) 2007, 2009 Hubert Figuiere
* Copyright (C) 2003-2005 Mark Gilbert <mg_abimail@yahoo.com>
* Copyright (C) 2002, 2004 Francis James Franklin <fjf@alinameridon.com>
* Copyright (C) 2001-2002 AbiSource, Inc.
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

#include "ie_exp_HTML_util.h"

const char * s_DTD_XHTML_AWML = "!DOCTYPE html PUBLIC \"-//ABISOURCE//DTD XHTML plus AWML 2.2//EN\" \"http://www.abisource.com/2004/xhtml-awml/xhtml-awml.mod\"";

const char * s_DTD_XHTML = "!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\"";

const char * s_DTD_HTML4 = "!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\"";

const char * s_Delimiter =
    "=======================================================";

const char * s_Header[2] = {
"Created by AbiWord, a free, Open Source wordprocessor. ",
"For more information visit http://www.abisource.com.   "
};

const char * s_HeaderCompact = "Created by Abiword, www.abisource.com";

bool m_bSecondPass = false;
bool m_bInAFENote = false;
bool m_bInAnnotation = false;
#include "MathSVGScript.h"

UT_UTF8String sStyleSheet = "#toc,\n"
".toc,\n"
".mw-warning {\n"
"	border: 1px solid #aaa;\n"
"	background-color: #f9f9f9;\n"
"	padding: 5px;\n"
"	font-size: 95%;\n"
"}\n"
"#toc h2,\n"
".toc h2 {\n"
"	display: inline;\n"
"	border: none;\n"
"	padding: 0;\n"
"	font-size: 100%;\n"
"	font-weight: bold;\n"
"}\n"
"#toc #toctitle,\n"
".toc #toctitle,\n"
"#toc .toctitle,\n"
".toc .toctitle {\n"
"	text-align: center;\n"
"}\n"
"#toc ul,\n"
".toc ul {\n"
"	list-style-type: none;\n"
"	list-style-image: none;\n"
"	margin-left: 0;\n"
"	padding-left: 0;\n"
"	text-align: left;\n"
"}\n"
"#toc ul ul,\n"
".toc ul ul {\n"
"	margin: 0 0 0 2em;\n"
"}\n"
"#toc .toctoggle,\n"
".toc .toctoggle {\n"
"	font-size: 94%;\n"
"}";

UT_UTF8String s_string_to_url (const UT_String & str)
{
	UT_UTF8String url;

	static const char hex[16] = {
		'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
	};
	char buf[4];
	buf[0] = '%';
	buf[3] = 0;

	const char * ptr = str.c_str ();
	while (*ptr)
	{
		bool isValidPunctuation = false;
		switch (*ptr)
		{
			case '-': // TODO: any others?
			case '_':
			case '.':
				isValidPunctuation = true;
				break;
			default:
				break;
		}
		unsigned char u = (unsigned char) *ptr;
		if (!isalnum (static_cast<int>(u)) && !isValidPunctuation)
		{
			buf[1] = hex[(u >> 4) & 0x0f];
			buf[2] = hex[ u       & 0x0f];
			url += buf;
		}
		else
		{
			buf[2] = (char) *ptr;
			url += (buf + 2);
		}
		ptr++;
	}
	return url;
}

UT_UTF8String s_string_to_url (const UT_UTF8String & str)
{
	UT_String s(str.utf8_str());
	return s_string_to_url(s);
}

extern const char * s_prop_list[] = {
	"background-color",	"transparent",
	"color",			"",
	"font-family",		"",
	"font-size",		"medium",
	"font-style",		"normal",
	"font-variant",		"normal",
	"font-weight",		"normal",
	"height",			"auto",
	"margin-bottom",	"0pt",
	"margin-left",		"0pt",
	"margin-right",		"0pt",
	"margin-top",		"0pt",
	"orphans",			"2",
	"text-align",		"",
	"text-decoration",	"none",
	"text-transform",	"none",
	"text-indent",		"0in",
	"vertical-align",	"baseline",
	"widows",			"2",
	"width",			"auto",
	0, 0
};
extern const UT_uint32 s_PropListLen = G_N_ELEMENTS(s_prop_list) - 2; /* don't include the zeros */

/*!	This function returns true if the given property is a valid CSS
  property.  It is based on the list in pp_Property.cpp, and, as such,
  is quite brittle.

  prop_default may be zero on return, indicating that the default is not fixed
*/
bool is_CSS (const char * prop_name, const char ** prop_default)
{
	if (prop_name == 0)
		return false;
	if (*prop_name == 0)
		return false;

	bool bCSS = false;

	for (UT_uint32 i = 0; i < s_PropListLen; i += 2)
	{
		if (!strcmp (prop_name, s_prop_list[i]))
		{
			if (prop_default) *prop_default = s_prop_list[i+1];
			bCSS = true;
			break;
		}
	}
	return bCSS;
}

/*!	This function copies a string to a new string, removing all the white
  space in the process.
*/
char * s_removeWhiteSpace (const char * text, UT_UTF8String & utf8str,
								  bool bLowerCase)
{
	utf8str = "";

	if (text)
	{
		char buf[2]; // ick! [TODO ??]
		buf[1] = 0;
		const char * ptr = text;
		while (*ptr)
		{
			if (isspace ((int) ((unsigned char) *ptr)))
			{
				buf[0] = '_';
			}
			else
			{
				buf[0] = *ptr;
			}
			utf8str += buf;
			ptr++;
		}

		if(bLowerCase)
			utf8str.lowerCase();
	}
	return 0;
}

UT_UTF8String ConvertToClean(const UT_UTF8String & str)
{
    UT_UTF8String result = "";

    UT_UTF8Stringbuf::UTF8Iterator i = str.getIterator();
    i = i.start();


    if (i.current())
    {
        while (true)
        {
            const gchar *pCurrent = i.current();

            if (*pCurrent == 0)
            {
                break;
            }

            if (isalnum(*pCurrent) || (*pCurrent == '-') || (*pCurrent == '_'))
            {
                result += *pCurrent;
            }

            i.advance();
        }
    }
    return result;
}

bool getPropertySize(const PP_AttrProp * pAP, const gchar* szWidthProp, 
	const gchar* szHeightProp, const gchar** szWidth, double& widthPercentage, 
	const gchar** szHeight, double dPageWidthInches, double dSecLeftMarginInches, 
	double dSecRightMarginInches, double dCellWidthInches,
	ie_Table &tableHelper)
{
	UT_return_val_if_fail(pAP, false);
	UT_return_val_if_fail(szWidth, false)
	UT_return_val_if_fail(szHeight, false)

	// get the object width as displayed in AbiWord
	*szWidth = NULL;
	pAP->getProperty (szWidthProp, *szWidth);
	
	// get the object height as displayed in AbiWord
	*szHeight = NULL;
	pAP->getProperty (szHeightProp, *szHeight);
	
	// determine the total width of this object, so we can calculate the object's
	// width as a percentage of that
	widthPercentage = 100;
	if (*szWidth)
	{
		double total = 0;
		if(tableHelper.getNestDepth() > 0)
		{
			total = dCellWidthInches;
		}
		else
		{
			total =  dPageWidthInches - dSecLeftMarginInches - dSecRightMarginInches;
		}

		double dWidth = UT_convertToInches(*szWidth);
		widthPercentage = 100.0 * dWidth / total;
		if (widthPercentage > 100.)
			widthPercentage = 100.0;
	}
		
	return true;
}

UT_UTF8String getStyleSizeString(const gchar * szWidth, double widthPercentage, 
	UT_Dimension widthDim, const gchar * szHeight, 
	UT_Dimension heightDim, bool bUseScale)
{
	UT_UTF8String props;
	
	if (szWidth)
	{
		props += "width:";
		if (bUseScale)
		{
			UT_sint32 iPercent = (UT_sint32)(widthPercentage + 0.5);
			props += UT_UTF8String_sprintf("%d%%", iPercent);
		}
		else
		{
			double d = UT_convertToDimension(szWidth, widthDim);
			props += UT_formatDimensionString(widthDim, d);
		}
	}

	if (szHeight)
	{
		if (props.size() > 0)
			props += "; ";
		props += "height:";
		double d = UT_convertToDimension(szHeight, heightDim);
		props += UT_formatDimensionString(heightDim , d);
	}	

	if (props.size() > 0)
		return props;

	return "";
}


IE_Exp_HTML_DataExporter::IE_Exp_HTML_DataExporter(PD_Document* pDocument, 
    const UT_UTF8String& baseName):
    m_pDocument(pDocument),
    m_fileDirectory(UT_go_basename_from_uri(baseName.utf8_str()) 
        + UT_UTF8String(FILES_DIR_NAME)),
    m_baseDirectory(UT_go_dirname_from_uri(baseName.utf8_str(), false))
{
    
}
void IE_Exp_HTML_DataExporter::_init()
{
    if (!m_bInitialized)
    {
        UT_go_directory_create((m_baseDirectory + G_DIR_SEPARATOR_S +  m_fileDirectory).utf8_str(), 
                               0644, NULL);
        
        m_bInitialized = true;
    }
}

UT_UTF8String IE_Exp_HTML_DataExporter::saveData(const gchar *szDataId, 
                                                 const gchar* extension)
{
    _init();
    UT_UTF8String filename = szDataId;
    
    if (extension != NULL)
    {
        filename += ".";
        filename += extension;
    }
    
    const UT_ByteBuf * pByteBuf = 0;
    if (!m_pDocument->getDataItemDataByName(szDataId, &pByteBuf, 
                                            NULL, NULL))
    {
        UT_ASSERT("No data item with specified dataid found\n");
        return "";
    }
    
    pByteBuf->writeToURI((m_baseDirectory + G_DIR_SEPARATOR_S + m_fileDirectory 
        + G_DIR_SEPARATOR_S + filename).utf8_str());
    
    return m_fileDirectory + G_DIR_SEPARATOR_S + filename;
}

GsfOutput* IE_Exp_HTML_DataExporter::createFile(const UT_UTF8String& name,
                                                UT_UTF8String &filename)
{
    _init();
    filename = m_fileDirectory 
        + G_DIR_SEPARATOR_S  + name;
    
    return UT_go_file_create((m_baseDirectory + G_DIR_SEPARATOR_S  + 
        m_fileDirectory + G_DIR_SEPARATOR_S + name).utf8_str(), 
                             NULL);
}

void IE_Exp_HTML_DataExporter::encodeDataBase64(const gchar* szDataId, 
                                                UT_UTF8String& result)
{
    std::string mimeType;
    const UT_ByteBuf * pByteBuf = 0;
    if (!m_pDocument->getDataItemDataByName(szDataId, &pByteBuf, 
                                            &mimeType, NULL))
    {
        UT_ASSERT("No data item with specified dataid found\n");
        return;
    }
    
    
    
    char buffer[75];
	char * bufptr = 0;
	size_t buflen;
	size_t imglen = pByteBuf->getLength ();
	const char * imgptr = reinterpret_cast<const char *>(pByteBuf->getPointer (0));

	buffer[0] = '\r';
	buffer[1] = '\n';
    result.clear();
    result += "data:";
    result += mimeType.c_str();
    result += ";base64,";
	while (imglen)
	{
		buflen = 72;
		bufptr = buffer + 2;

		UT_UTF8_Base64Encode (bufptr, buflen, imgptr, imglen);

		*bufptr = 0;

		result += buffer;
	}
}

/*
 * 
 */ 
IE_Exp_HTML_OutputWriter::IE_Exp_HTML_OutputWriter(GsfOutput* output):
m_output(output)
{
   
}

void IE_Exp_HTML_OutputWriter::write(const gchar* data, size_t size)
{
    gsf_output_write(m_output, size, (const guint8*)data);
}

void IE_Exp_HTML_OutputWriter::write(const std::string& str)
{
    write(str.c_str(), strlen(str.c_str()));
}

IE_Exp_HTML_TagWriter::IE_Exp_HTML_TagWriter(IE_Exp_HTML_OutputWriter* pOutputWriter):
m_pOutputWriter(pOutputWriter),
    m_bXmlModeEnabled(false),
    m_bCurrentTagIsSingle(false),
    m_bAttributesWritten(false),
        m_bDataWritten(false),
    m_bInComment(false),
    m_buffer("")
{
    
}

void IE_Exp_HTML_TagWriter::openTag(const std::string& tagName, bool isInline, bool isSingle)
{
    if (m_bInComment)
    {
        UT_ASSERT("Trying to open tag inside comment\n");
        return;
    }
        
    if ((m_tagStack.size() > 0) && m_bCurrentTagIsSingle)
    {
        closeTag();
    } else
    {
        _closeAttributes();
    }
    m_bCurrentTagIsSingle = isSingle;
    m_bAttributesWritten = false;
    m_bDataWritten = false;
    m_tagStack.push_back(tagName);
    m_inlineFlagStack.push_back(isInline);
   
    
    
    if (!isInline)
    {
        std::string indent = "";
        for (size_t i = 0; i < m_tagStack.size() - 1; i++)
        {
            indent += "    ";
        }
        
        m_buffer += indent;
    }
    m_buffer += "<" + tagName;
    
    UT_DEBUGMSG(("Opened tag: %s\n", tagName.c_str())); 
}

void IE_Exp_HTML_TagWriter::addAttribute(const std::string& name, const std::string& value)
{
    if (m_bInComment)
    {
        UT_ASSERT("Trying to add attribute inside comment\n");
        return;
    }
    m_buffer += " " + name + "=\"" + value + "\""; 
}

void IE_Exp_HTML_TagWriter::_closeAttributes()
{
    if (m_tagStack.size() ==  0)
    {
        return;
    }
    
    if (m_bInComment)
    {
        UT_ASSERT("Trying to close attribute list inside comment\n");
        return;
    }
        
    if (!m_bAttributesWritten)
    {
        if (m_bXmlModeEnabled)
        {
                m_buffer += " />";
        } else
        {
            m_buffer += ">";
        }
        
        if (!m_inlineFlagStack.back())
        {
            m_buffer += "\n";
        }
        
        m_bAttributesWritten = true;
    }
}

void IE_Exp_HTML_TagWriter::writeData(const std::string& data)
{
    if (!m_bInComment)
    {
        _closeAttributes();
    }
    m_bDataWritten = true;
    m_buffer += data;
}

void IE_Exp_HTML_TagWriter::closeTag()
{
    if (m_bInComment)
    {
        UT_ASSERT("Trying to close tag inside comment\n");
        return;
    }
    
    if (m_tagStack.size() == 0)
    {
        UT_ASSERT("Trying to close unopened tag\n");
        return;
    }
    _closeAttributes();
    
    if (!m_bCurrentTagIsSingle)
    {

        if (m_bDataWritten && !m_inlineFlagStack.back())
        {
            std::string indent = "";
            for (size_t i = 0; i < m_tagStack.size() - 1; i++)
            {
                indent += "    ";
            }
            m_buffer += "\n" + indent;
        }
        
        m_buffer += "</" + m_tagStack.back() + ">";
        if (!m_inlineFlagStack.back())
        {
            m_buffer += "\n";
        }
    } else
    {
        m_bCurrentTagIsSingle = false;
    }
    
    UT_DEBUGMSG(("Closed tag: %s\n", m_tagStack.back().c_str()));
    m_tagStack.pop_back();
    m_inlineFlagStack.pop_back();
    
    flush();
    
    
}

void IE_Exp_HTML_TagWriter::openComment()
{
    if (m_bInComment)
    {
        UT_ASSERT("Trying to create nested comment\n");
        return;
    }
    _closeAttributes();
    m_bInComment = true;
    m_buffer += "<!-- ";
}

void IE_Exp_HTML_TagWriter::closeComment()
{
    if (!m_bInComment)
    {
        UT_ASSERT("Trying to close unopened comment\n");
        return;
    }
    m_bInComment = false;
    m_buffer += " -->";
}

void IE_Exp_HTML_TagWriter::flush()
{
    if (m_buffer.length() > 0)
    {
        m_pOutputWriter->write(m_buffer);
        m_buffer = "";
    }
}

void IE_Exp_HTML_TagWriter::enableXmlMode(bool enable)
{
    m_bXmlModeEnabled = enable;
}
