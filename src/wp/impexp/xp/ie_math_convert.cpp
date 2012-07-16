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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
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
				
		cur = xsltParseStylesheetFile((const xmlChar *)(path.utf8_str()));
		if (!cur)
		{
			UT_DEBUGMSG(("convertMathMLtoLaTeX: Parsing stylesheet failed\n"));
			return false;
		}
	}
	// bad bad bad, apparently on MacOS X, the system libxml2 take a non-const here.
	doc = xmlParseDoc((xmlChar*)(sMathML.utf8_str()));
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
		UT_sint32 i=3;
		UT_sint32 j=sLaTeX.size()-3;

		while((strcmp((sLaTeX.substr(i,1)).utf8_str(),"\n")==0 || strcmp((sLaTeX.substr(i,1)).utf8_str(),"\t")==0) && i< sLaTeX.size()-2)
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

// Function to convert OMML (from docx) to MathML

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
        
        // TO DO : add the post build event to the msvc project to copy the omml_xslt folder from openxml plugin to debug/release, after the gsoc2012math branch is merged with trunk

        //path = "../../plugins/openxml/common/omml_xslt/omml2mml.xsl";

        cur2 = xsltParseStylesheetFile((const xmlChar *)(path.c_str()));
        if(!cur2)
        {
            UT_DEBUGMSG(("convertOMMLtoMathML : Parsing stylesheet failed\n"));
            return false;
        }

    }

    doc = xmlParseDoc((xmlChar*)(pOMML.c_str()));
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

    g_free(qMathML);
    xmlFreeDoc(res);
    xmlFreeDoc(doc);
    return true;

}
