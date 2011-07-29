#include "ie_exp_HTML_TagWriter.h"
#include "ie_exp_HTML.h"
#include "ut_Encoding.h"


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
    m_buffer += data;
    m_bDataWritten = true;
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

