/* AbiWord
 * Copyright (C) 2008 Xun Sun (xun.sun.cn@gmail.com)
 * Copyright (C) 2012 Prashant Bafna (appu.bafna@gmail.com)
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

#include "ie_math_convert.h"
#include "ut_debugmsg.h"
#include "xap_App.h"

#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

static xsltStylesheet * cur = NULL;

bool convertMathMLtoLaTeX(const UT_UTF8String & sMathML, UT_UTF8String & sLaTeX)
{
    //static xsltStylesheet *cur = NULL;
    xmlDocPtr doc, res;
    xmlChar * pLatex = NULL;
    int len;

    if (sMathML.empty())
        // Nothing has failed, but we have nothing to do anyway
	return false;
    if (!cur)
    {
	UT_UTF8String path(XAP_App::getApp()->getAbiSuiteLibDir());
	path += "/xsltml/mmltex.xsl";

	cur = xsltParseStylesheetFile((const xmlChar *)path.utf8_str());
	if (!cur)
	{
            UT_DEBUGMSG(("convertMathMLtoLaTeX: Parsing stylesheet failed\n"));
	    return false;
	}
    }

    doc = xmlParseDoc((const xmlChar*)sMathML.utf8_str());
    if (!doc)
    {
        xxx_UT_DEBUGMSG(("convertMathMLtoLaTeX: Parsing MathML document failed\n"));
	return false;
    }

    res = xsltApplyStylesheet(cur, doc, NULL);
    if (!res)
    {
        xxx_UT_DEBUGMSG(("convertMathMLtoLaTeX: Applying stylesheet failed\n"));
	xmlFreeDoc(doc);
	return false;
    }
	
    if (xsltSaveResultToString(&pLatex, &len, res, cur) != 0)
    {
	xmlFreeDoc(res);
	xmlFreeDoc(doc);
	return false;
    }
    
    sLaTeX.assign((const char*)pLatex, len);
	
    g_free(pLatex);
    xmlFreeDoc(res);
    xmlFreeDoc(doc);
    return true;
}

// Function to convert the generated LaTeX to the equation form acceptable in the editor

bool convertLaTeXtoEqn(const UT_UTF8String & sLaTeX,UT_UTF8String & eqnLaTeX)
{

    if (sLaTeX.empty())
	return false;

    if(strcmp((sLaTeX.substr(1,2)).utf8_str(),"\\[")==0 && strcmp((sLaTeX.substr(sLaTeX.size()-2,2)).utf8_str(),"\\]")==0)
    {
        UT_uint32 i=3;
	UT_uint32 j=sLaTeX.size()-3;

	while((strcmp((sLaTeX.substr(i,1)).utf8_str(),"\n")==0 || strcmp((sLaTeX.substr(i,1)).utf8_str(),"\t")==0) && i+2< sLaTeX.size())
	{
	    i++;
	}
	while((strcmp((sLaTeX.substr(j,1)).utf8_str(),"\n")==0 || strcmp((sLaTeX.substr(j,1)).utf8_str(),"\t")==0) && j> 2)
	{
	    j--;
	}

	eqnLaTeX = sLaTeX.substr(i,(j-i)+1);
	return true;
    }
    else if(strcmp((sLaTeX.substr(0,1)).utf8_str(),"$")==0 && strcmp((sLaTeX.substr(sLaTeX.size()-1,1)).utf8_str(),"$")==0)
    {
	eqnLaTeX = sLaTeX.substr(1,sLaTeX.size()-2);
	return true;
    }
    else
    {
	// No Conversion Required
	eqnLaTeX = sLaTeX;
	return true;
    }

}

// Function to convert OMML to MathML (for import of Math from docx)

static xsltStylesheet * cur2 = NULL;

bool convertOMMLtoMathML(const std::string & pOMML, std::string & pMathML)
{
    xmlDocPtr doc,res;
    xmlChar * qMathML = NULL;
    int len;

    if(pOMML.empty())
    {
        return false;
    }

    if(!cur2)
    {
        std::string path(XAP_App::getApp()->getAbiSuiteLibDir());
        path+= "/omml_xslt/omml2mml.xsl";

        // TODO : add the post build event to the msvc project to copy
        // the omml_xslt folder from openxml plugin to debug/release,
        // after the gsoc2012math branch is merged with trunk

        cur2 = xsltParseStylesheetFile((const xmlChar *)path.c_str());
        if(!cur2)
        {
            UT_DEBUGMSG(("convertOMMLtoMathML : Parsing stylesheet failed\n"));
            return false;
        }

    }

    doc = xmlParseDoc((const xmlChar*)pOMML.c_str());
    if(!doc)
    {
        xxx_UT_DEBUGMSG(("convertOMMLtoMathML : Parsing OMML document failed\n"));
        return false;
    }

    res = xsltApplyStylesheet(cur2, doc, NULL);
    if(!res)
    {
        xxx_UT_DEBUGMSG(("convertOMMLtoMathML: Applying stylesheet failed\n"));
        xmlFreeDoc(doc);
        return false;
    }

    if(xsltSaveResultToString(&qMathML, &len, res, cur2) != 0)
    {
        xmlFreeDoc(res);
        xmlFreeDoc(doc);
        return false;
    }

    pMathML.assign((const char*)qMathML, len);
    
    if(strncmp(pMathML.c_str(),"<?xml version=\"1.0\"?>\n",22) == 0)
    {
        // remove <?xml version=\"1.0\"?>\n from the MathML
        pMathML = pMathML.substr(22);
    }

    g_free(qMathML);
    xmlFreeDoc(res);
    xmlFreeDoc(doc);
    return true;

}

// Function to convert MathML to OMML (for export of Math to docx)

static xsltStylesheet * cur3 = NULL;

bool convertMathMLtoOMML(const std::string & rMathML, std::string & rOMML)
{
    xmlDocPtr doc,res;
    xmlChar * sOMML = NULL;
    int len;
    
    if(rMathML.empty())
    {
        return false;
    }

    if(!cur3)
    {
        std::string path(XAP_App::getApp()->getAbiSuiteLibDir());
        path+= "/omml_xslt/mml2omml.xsl";

        // TODO : add a post build event to the msvc project to copy
        // the omml_xslt folder from openxml plugin to debug/release,
        // after the gsoc2012math branch is merged with trunk

        cur3 = xsltParseStylesheetFile((const xmlChar *)path.c_str());
        if(!cur3)
        {
            UT_DEBUGMSG(("convertMathMLtoOMML : Parsing stylesheet failed\n"));
            return false;
        }
    }

    doc = xmlParseDoc((const xmlChar*)rMathML.c_str());
    if(!doc)
    {
        xxx_UT_DEBUGMSG(("convertMathMLtoOMML : Parsing MathML document failed\n"));
        return false;
    }

    res = xsltApplyStylesheet(cur3, doc, NULL);
    if(!res)
    {
        xxx_UT_DEBUGMSG(("convertMathMLtoOMML : Applying stylesheet failed\n"));
        xmlFreeDoc(doc);
        return false;
    }

    if(xsltSaveResultToString(&sOMML, &len, res, cur3) != 0)
    {
        xmlFreeDoc(res);
        xmlFreeDoc(doc);
        return false;
    }

    rOMML.assign((const char*)sOMML, len);

    if(strncmp(rOMML.c_str(),"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n",39) == 0)
    {
        rOMML = rOMML.substr(39);
    }

    if(strncmp(rOMML.c_str(),"<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\" xmlns:mml=\"http://www.w3.org/1998/Math/MathML\">",125) == 0)
    {
        rOMML = rOMML.substr(125);
        std::string temp;
        temp.assign("<m:oMath>");
        temp.append(rOMML.c_str());
        rOMML.assign(temp.c_str());
    }

    if(strncmp((rOMML.substr(rOMML.length()-1)).c_str(),"\n",1)==0)
    {
        rOMML = rOMML.substr(0,rOMML.length()-1);
    }

    g_free(sOMML);
    xmlFreeDoc(res);
    xmlFreeDoc(doc);
    return true;

}
