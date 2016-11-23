/* AbiSource
 * 
 * Copyright (C) 2012 Prashant Bafna <appu.bafna@gmail.com>
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
#include <OXML_Element_Math.h>

// AbiWord includes
#include "ut_std_string.h"
#include "ie_math_convert.h"

OXML_Element_Math::OXML_Element_Math(const std::string & id) : 
    OXML_Element(id, MATH_TAG, MATH)
{
}

OXML_Element_Math::~OXML_Element_Math()
{

}

void OXML_Element_Math::setMathML(const std::string & sMathML)
{
    m_MathML = sMathML;
}

const char * OXML_Element_Math::getMathML()
{
    UT_return_val_if_fail(!m_MathML.empty(), NULL);
    return m_MathML.c_str();
}

UT_Error OXML_Element_Math::serialize(IE_Exp_OpenXML* exporter)
{
    UT_Error err = UT_OK;
	
    err = exporter->startMath();
    if(err != UT_OK)
        return err;
    
    std::string sMathML;
    sMathML.assign(getMathML());
    std::string sOMML;

    if(convertMathMLtoOMML(sMathML, sOMML))
    {
        err = exporter->writeMath(sOMML.c_str());
        if(err != UT_OK)
            return err;
    }
    
    return exporter->finishMath();
}

UT_Error OXML_Element_Math::addToPT(PD_Document * pDocument)
{
    UT_uint32 id;
    id = pDocument->getUID(UT_UniqueId::Math);
    std::string mID = UT_std_string_sprintf("MathLatex%d", id);
    std::string lID = UT_std_string_sprintf("LatexMath%d", id);

    UT_ByteBufPtr mathBuf(new UT_ByteBuf);
    UT_ByteBufPtr latexBuf(new UT_ByteBuf);
    mathBuf->ins(0, reinterpret_cast<const UT_Byte *>(m_MathML.c_str()), static_cast<UT_uint32>(m_MathML.length()));

    UT_UTF8String sMathml; // TO DO : use std::string after enabling it in ie_math_convert
    UT_UTF8String sLatex,sitex;
    sMathml.assign(m_MathML.c_str());

    pDocument->createDataItem(mID.c_str(), false, mathBuf, "", NULL);

    if(convertMathMLtoLaTeX(sMathml, sLatex) && convertLaTeXtoEqn(sLatex,sitex))
    {
        // Conversion of MathML to LaTeX and the Equation Form suceeds
        latexBuf->ins(0, reinterpret_cast<const UT_Byte *>(sitex.utf8_str()), static_cast<UT_uint32>(sitex.size()));
        pDocument->createDataItem(lID.c_str(), false, latexBuf, "", NULL);
    }

    const PP_PropertyVector atts = {
      PT_IMAGE_DATAID, mID,
      "latexid", lID
    };
    if(!pDocument->appendObject(PTO_Math, atts))
        return UT_ERROR;

    return UT_OK;
}
