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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef IE_EXP_HTML_UTIL_H
#define IE_EXP_HTML_UTIL_H

// External includes
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <gsf/gsf-output.h>

// Abiword includes
#include <ie_TOC.h>
#include <ie_Table.h>
#include <pd_Document.h>
#include <ut_go_file.h>
#include <ut_string_class.h>
#include <ut_types.h>
#include <ut_debugmsg.h>
#include <ut_base64.h>

#define MYEOL "\n"
#define FILES_DIR_NAME "_files"
#define MULTIPART_BOUNDARY "AbiWord_multipart_boundary____________"
#define MULTIPART_FIELD(key, value) UT_UTF8String_sprintf("%s : %s\n", key, value)

extern const char * s_prop_list[];
extern const UT_uint32 s_PropListLen;
extern const char s_DTD_XHTML_AWML[];
extern const char s_DTD_XHTML[];
extern const char s_DTD_HTML4[];
extern const char s_Delimiter[];
extern const char s_HeaderCompact[];
extern bool m_bSecondPass;
extern bool m_bInAFENote;
extern bool m_bInAnnotation;
extern UT_UTF8String sMathSVGScript;
extern UT_UTF8String sStyleSheet;

extern const char * s_Header[2];

UT_UTF8String s_string_to_url (const UT_String & str);
UT_UTF8String s_string_to_url (const UT_UTF8String & str);
bool is_CSS (const char * prop_name, const char ** prop_default = 0);
char * s_removeWhiteSpace (const char * text, UT_UTF8String & utf8str,
                           bool bLowerCase = true);
// Returns alpha-numeric contents of string
UT_UTF8String ConvertToClean(const UT_UTF8String &str);

// Returns css representation for size params
UT_UTF8String getStyleSizeString(const gchar * szWidth, double widthPercentage,
	UT_Dimension widthDim, const gchar * szHeight,
	UT_Dimension heightDim, bool bUseScale);

bool getPropertySize(const PP_AttrProp * pAP, const gchar* szWidthProp,
	const gchar* szHeightProp, const gchar** szWidth, double& widthPercentage,
	const gchar** szHeight, double dPageWidthInches, double dSecLeftMarginInches,
	double dSecRightMarginInches, double dCellWidthInches,
	ie_Table &tableHelper);

class IE_Exp_HTML_OutputWriter;
/*
 * This class allows to control creation of files (like CSS, JS and images)
 * while exporting to {X,P}HTML
 */
class ABI_EXPORT IE_Exp_HTML_DataExporter{
public:
    IE_Exp_HTML_DataExporter(PD_Document* pDocument,
            const UT_UTF8String &filename);
    virtual ~IE_Exp_HTML_DataExporter(){}
    /*
     * Saves object with specified dataid to disk and returns relative to index
     * document path it
     */
    virtual UT_UTF8String saveData(
        const gchar *szDataId, const gchar* extension) = 0;
    virtual UT_UTF8String saveData(const UT_UTF8String &name,
    const UT_UTF8String &data) = 0;

    /*
     * Encodes data with specified dataid using Base64 and places string
     * containing result in buffer
     */
    void encodeDataBase64(const gchar *szDataId, UT_UTF8String &result,
        bool bAddInfo = true);

protected:
    PD_Document *m_pDocument;
    UT_UTF8String m_fileDirectory;
    UT_UTF8String m_baseDirectory;
};

class IE_Exp_HTML_FileExporter : public IE_Exp_HTML_DataExporter
{
public:
    IE_Exp_HTML_FileExporter(PD_Document* pDocument,
            const UT_UTF8String &baseName);

    UT_UTF8String saveData(const gchar *szDataId, const gchar* extension);
    UT_UTF8String saveData(const UT_UTF8String &name,
    const UT_UTF8String &data);
private:
    void _init();
    bool m_bInitialized;
    std::map<UT_UTF8String, UT_UTF8String> m_saved;
};



class ABI_EXPORT IE_Exp_HTML_MultipartExporter : public IE_Exp_HTML_DataExporter
{
public:
    IE_Exp_HTML_MultipartExporter(PD_Document* pDocument,
            const UT_UTF8String &baseName,
            UT_UTF8String &buffer,
            const UT_UTF8String &title);

    UT_UTF8String saveData(const gchar *szDataId, const gchar* extension);
    UT_UTF8String saveData(const UT_UTF8String &name,
        const UT_UTF8String &data);

    UT_UTF8String generateHeader(const UT_UTF8String &index,
        const UT_UTF8String &mimetype);
private:
    UT_UTF8String &m_buffer;
    UT_UTF8String m_title;
};

/**
 * Utility class that allows write character data or UTF8 strings
 */
class ABI_EXPORT IE_Exp_HTML_OutputWriter
{
public:
    virtual ~IE_Exp_HTML_OutputWriter() {}
    virtual void write(const UT_UTF8String &str) = 0;
 };

class ABI_EXPORT IE_Exp_HTML_FileWriter : public IE_Exp_HTML_OutputWriter
{
public:
    IE_Exp_HTML_FileWriter(GsfOutput *output);
    void write(const UT_UTF8String &str);
private:
    GsfOutput *m_output;
};

class ABI_EXPORT IE_Exp_HTML_StringWriter: public IE_Exp_HTML_OutputWriter
{
public:
    IE_Exp_HTML_StringWriter();
    void write(const UT_UTF8String &str);
    UT_UTF8String getString() const { return m_buffer; }
private:
    UT_UTF8String m_buffer;
};

/**
 * Utility class that gives simple interface to create HTML and XML documents
 */
class ABI_EXPORT IE_Exp_HTML_TagWriter
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
