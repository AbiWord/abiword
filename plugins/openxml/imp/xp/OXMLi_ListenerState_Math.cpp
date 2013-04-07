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
#include <OXMLi_ListenerState_Math.h>

// Internal includes
#include <OXML_Types.h>

// AbiWord includes
#include "ie_math_convert.h"

OXMLi_ListenerState_Math::OXMLi_ListenerState_Math():
    OXMLi_ListenerState(), m_pMathBB(NULL), m_bInMath(false)
{

}

void OXMLi_ListenerState_Math::startElement (OXMLi_StartElementRequest * rqst)
{
    if (m_bInMath && m_pMathBB && !(nameMatches(rqst->pName, NS_M_KEY, "oMath")))
    {
        if(strncmp((rqst->pName).c_str(), "M:",2)!=0)
        {
            return;
        }

        m_pMathBB->append(reinterpret_cast<const UT_Byte *>("<m:"), 3);
        m_pMathBB->append(reinterpret_cast<const UT_Byte *>(((rqst->pName).substr(2)).c_str()),((rqst->pName).substr(2)).length());

        const gchar* val = attrMatches(NS_M_KEY, "val", rqst->ppAtts);
        if(val != NULL)
        {
            m_pMathBB->append(reinterpret_cast<const UT_Byte *>(" m:val=\""), 8);
            m_pMathBB->append(reinterpret_cast<const UT_Byte *>(val), strlen(val));
            m_pMathBB->append(reinterpret_cast<const UT_Byte *>("\""), 1);
        }

        m_pMathBB->append(reinterpret_cast<const UT_Byte *>(">"), 1);

        rqst->handled = true;
        return;
    }

    if(nameMatches(rqst->pName, NS_M_KEY, "oMath"))
    {
        DELETEP(m_pMathBB);
        m_pMathBB = new UT_ByteBuf;
        m_pMathBB->append(reinterpret_cast<const UT_Byte *>("<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"),78);
        m_bInMath = true;

        OXML_SharedElement mathElem(new OXML_Element_Math(""));
        rqst->stck->push(mathElem);
        rqst->handled = true;
    }

        
}

void OXMLi_ListenerState_Math::endElement (OXMLi_EndElementRequest * rqst)
{
    if (m_bInMath && m_pMathBB && (!nameMatches(rqst->pName, NS_M_KEY, "oMath")))
    {
        if(strncmp((rqst->pName).c_str(), "M:", 2) != 0)
        {
            return;
        }

        m_pMathBB->append(reinterpret_cast<const UT_Byte *>("</m:"),4);
        m_pMathBB->append(reinterpret_cast<const UT_Byte *>(((rqst->pName).substr(2)).c_str()),((rqst->pName).substr(2)).length());
        m_pMathBB->append(reinterpret_cast<const UT_Byte *>(">"),1);        

        rqst->handled = true;
        return;
    }

    if(nameMatches(rqst->pName, NS_M_KEY, "oMath"))
    {
        if(rqst->sect_stck->empty())
        {
            rqst->handled = false;
            rqst->valid = false;
        }

        if(m_pMathBB)
        {
            m_pMathBB->append(reinterpret_cast<const UT_Byte *>("</m:oMath>"),10);     

            std::string pomml;
            pomml.assign((const char*)(m_pMathBB->getPointer(0)));
            std::string pmathml;            
        
            if (!convertOMMLtoMathML(pomml,pmathml))
            {
                // if conversion from OMML to MathML fails
                return;                             
            }
                  
            OXML_SharedElement mathElem = rqst->stck->top();
            OXML_Element* elem = mathElem.get();

            if(!elem || (elem->getTag() != MATH_TAG))
                return;

            OXML_Element_Math* MathElement = static_cast<OXML_Element_Math*>(elem);
            MathElement->setMathML(pmathml);
            
            UT_return_if_fail( this->_error_if_fail( UT_OK == _flushTopLevel(rqst->stck, rqst->sect_stck) ) );
            rqst->handled = true;

        }
        m_bInMath = false;   
        DELETEP(m_pMathBB);
        
    }

}

void OXMLi_ListenerState_Math::charData (OXMLi_CharDataRequest * rqst)
{
    if(m_bInMath && m_pMathBB)
    {
        m_pMathBB->append(reinterpret_cast<const UT_Byte *>(rqst->buffer), rqst->length);
        return;
    }
}
