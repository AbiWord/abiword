/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#ifndef IE_EXP_HTML_UTIL_H
#define IE_EXP_HTML_UTIL_H

// External includes
#include <vector>
#include <string>
#include <gsf/gsf-output.h>

// Abiword includes
#include <ut_string_class.h>
#include <ut_types.h>
#include <ut_debugmsg.h>

#define MYEOL "\n"

extern const char * s_prop_list[];
extern const UT_uint32 s_PropListLen;
extern const char * s_DTD_XHTML_AWML;
extern const char * s_DTD_XHTML;
extern const char * s_DTD_HTML4;
extern const char * s_Delimiter;
extern const char * s_HeaderCompact;
extern bool m_bSecondPass;
extern bool m_bInAFENote;
extern bool m_bInAnnotation;
extern UT_UTF8String sMathSVGScript;

extern const char * s_Header[2];

UT_UTF8String s_string_to_url (const UT_String & str);
UT_UTF8String s_string_to_url (const UT_UTF8String & str);
bool is_CSS (const char * prop_name, const char ** prop_default = 0);
char * s_removeWhiteSpace (const char * text, UT_UTF8String & utf8str,
								  bool bLowerCase = true);


/**
 * Utility class that allows write character data or UTF8 strings
 */
class IE_Exp_HTML_OutputWriter
{
public:
    IE_Exp_HTML_OutputWriter(GsfOutput *output);
    void write(const gchar * data, size_t size);
    void write(const std::string &str);
    
private:
    GsfOutput *m_output;
};

/**
 * Utility class that gives simple interface to create HTML and XML documents
 */
class IE_Exp_HTML_TagWriter
{
public:
    IE_Exp_HTML_TagWriter(IE_Exp_HTML_OutputWriter *pOutputWriter);
    void openTag(const std::string &tagName, bool isInline = false, bool isSingle = false);
    void addAttribute(const std::string &name, const std::string &value);
    void writeData(const std::string &data);
    void closeTag();
    void flush();
    
    void openComment();
    void closeComment();
    
    void enableXmlMode(bool enable = true);
private:
    
    inline void _closeAttributes();
    std::vector<std::string> m_tagStack;
    std::vector<bool> m_inlineFlagStack;
    IE_Exp_HTML_OutputWriter *m_pOutputWriter;
    bool m_bXmlModeEnabled;
    bool m_bCurrentTagIsSingle;
    bool m_bAttributesWritten;
    bool m_bDataWritten;
    bool m_bInComment;
    std::string m_buffer;
};



#endif