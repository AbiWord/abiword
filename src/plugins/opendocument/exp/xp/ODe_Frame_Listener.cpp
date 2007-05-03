/* AbiSource
 * 
 * Copyright (C) 2005 INdT
 * Author: Daniel d'Andrada T. de Carvalho <daniel.carvalho@indt.org.br>
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
 
// Class definition include
#include "ODe_Frame_Listener.h"

// Internal includes
#include "ODe_AutomaticStyles.h"
#include "ODe_AuxiliaryData.h"
#include "ODe_Common.h"
#include "ODe_Style_Style.h"
#include "ODe_Text_Listener.h"
#include "ODe_ListenerAction.h"

// AbiWord includes
#include <pp_AttrProp.h>


/**
 * Constructor
 */
ODe_Frame_Listener::ODe_Frame_Listener(ODe_AutomaticStyles& rAutomatiStyles,
                                       GsfOutput* pTextOutput,
                                       ODe_AuxiliaryData& rAuxiliaryData,
                                       UT_uint8 zIndex,
                                       UT_uint8 spacesOffset)
                                       :
                                       ODe_AbiDocListenerImpl(spacesOffset),
                                       m_rAutomatiStyles(rAutomatiStyles),
                                       m_pTextOutput(pTextOutput),
                                       m_rAuxiliaryData(rAuxiliaryData),
                                       m_zIndex(zIndex)
{
}


/**
 * 
 */
void ODe_Frame_Listener::openFrame(const PP_AttrProp* pAP,
                                   ODe_ListenerAction& rAction) {
    bool ok;
    const gchar* pValue;
    
    ok = pAP->getProperty("frame-type", pValue);
    UT_ASSERT_HARMLESS(ok && pValue != NULL);
    
    if (pValue && !strcmp(pValue, "textbox")) {
        _openODTextbox(*pAP, rAction);
    }
}


/**
 * 
 */
void ODe_Frame_Listener::closeFrame(ODe_ListenerAction& rAction) {
        UT_UTF8String output;
        
        m_spacesOffset--;
        _printSpacesOffset(output);
        output += "</draw:text-box>\n";
        
        m_spacesOffset--;
        _printSpacesOffset(output);
        output += "</draw:frame>";
    
        ODe_writeToFile(m_pTextOutput, output);
        
        rAction.popListenerImpl();
}


/**
 * 
 */
void ODe_Frame_Listener::openTable(const PP_AttrProp* pAP,
                                   ODe_ListenerAction& rAction) {
    ODe_Text_Listener* pTextListener;
    pTextListener = new ODe_Text_Listener(m_rAutomatiStyles,
                                          m_pTextOutput,
                                          m_rAuxiliaryData,
                                          m_zIndex+1,
                                          m_spacesOffset);
    rAction.pushListenerImpl(pTextListener, true);
}


/**
 * 
 */
void ODe_Frame_Listener::openBlock(const PP_AttrProp* pAP,
                                   ODe_ListenerAction& rAction) {
    ODe_Text_Listener* pTextListener;
    pTextListener = new ODe_Text_Listener(m_rAutomatiStyles,
                                          m_pTextOutput,
                                          m_rAuxiliaryData,
                                          m_zIndex+1,
                                          m_spacesOffset);
    rAction.pushListenerImpl(pTextListener, true);
}


/**
 * 
 */
void ODe_Frame_Listener::_openODTextbox(const PP_AttrProp& rAP,
                                        ODe_ListenerAction& rAction) {
    UT_UTF8String output;
    UT_UTF8String str;
    bool ok;
    const gchar* pValue;
    ODe_Style_Style* pStyle;
    
    pStyle = new ODe_Style_Style();
    pStyle->setFamily("graphic");
    pStyle->fetchAttributesFromAbiFrame(rAP);
    
    // Abi frames have no padding
    // (no margin between frame borders and its content)
    pStyle->setPadding("0cm");
    
    // Abi frames are aways positioned from its top-left corner.
    pStyle->setHorizontalPos("from-left");
    pStyle->setVerticalPos("from-top");
    
    m_rAutomatiStyles.storeGraphicStyle(pStyle);

    ////
    // Write <draw:frame>
    
    _printSpacesOffset(output);
    output += "<draw:frame";

    UT_UTF8String_sprintf(str, "Frame%u", m_rAuxiliaryData.m_frameCount+1);
    ODe_writeAttribute(output, "draw:name", str);
    m_rAuxiliaryData.m_frameCount++;
    
    ODe_writeAttribute(output, "draw:style-name", pStyle->getName());

    UT_UTF8String_sprintf(str, "%u", m_zIndex);
    ODe_writeAttribute(output, "draw:z-index", str);


    ok = rAP.getProperty("position-to", pValue);
    UT_ASSERT (ok && pValue != NULL);

    if (pValue && !strcmp(pValue, "block-above-text")) {

        ODe_writeAttribute(output, "text:anchor-type", "paragraph");

        ok = rAP.getProperty("xpos", pValue);
        UT_ASSERT(ok && pValue != NULL);
        ODe_writeAttribute(output, "svg:x", pValue);

        ok = rAP.getProperty("ypos", pValue);
        UT_ASSERT(ok && pValue != NULL);
        ODe_writeAttribute(output, "svg:y", pValue);
    } else {
        // Everything else (column and page) will be treated as page
        // anchored.
        
        ODe_writeAttribute(output, "text:anchor-type", "page");

        ok = rAP.getProperty("frame-page-xpos", pValue);
        UT_ASSERT(ok && pValue != NULL);
        ODe_writeAttribute(output, "svg:x", pValue);
        
        ok = rAP.getProperty("frame-page-ypos", pValue);
        UT_ASSERT(ok && pValue != NULL);
        ODe_writeAttribute(output, "svg:y", pValue);
    }
    
    
    ok = rAP.getProperty("frame-width", pValue);
    if (ok && pValue != NULL) {
        ODe_writeAttribute(output, "svg:width", pValue);
    }
    
    output += ">\n";
    
    ODe_writeToFile(m_pTextOutput, output);
    m_spacesOffset++;
    
    ////
    // Write <draw:text-box>
    
    output.clear();
    _printSpacesOffset(output);
    output += "<draw:text-box";
    
    ok = rAP.getProperty("frame-height", pValue);
    if (ok && pValue != NULL) {
        ODe_writeAttribute(output, "fo:min-height", pValue);
    }
    
    output += ">\n";
    
    ODe_writeToFile(m_pTextOutput, output);
    m_spacesOffset++;
}
