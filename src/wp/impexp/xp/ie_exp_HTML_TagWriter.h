#ifndef IE_EXP_HTML_TAGWRITER_H
#define	IE_EXP_HTML_TAGWRITER_H


// External includes
#include <vector>
#include <string>
#include <gsf/gsf-output.h>

// Abiword includes
#include <ut_debugmsg.h>

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


#endif	/* IE_EXP_HTML_TAGWRITER_H */

