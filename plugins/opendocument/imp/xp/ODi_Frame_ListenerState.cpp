/* AbiSource
 * 
 * Copyright (C) 2005 Daniel d'Andrada T. de Carvalho
 * <daniel.carvalho@indt.org.br>
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
#include "ODi_Frame_ListenerState.h"

// Internal includes
#include "ODi_ListenerStateAction.h"
#include "ODi_ElementStack.h"
#include "ODi_Style_Style.h"
#include "ODi_StartTag.h"
#include "ODi_Office_Styles.h"
#include "ODi_Abi_Data.h"

// AbiWord includes
#include <pt_Types.h>
#include <pd_Document.h>


/**
 * Constructor
 */
ODi_Frame_ListenerState::ODi_Frame_ListenerState(PD_Document* pDocument,
    ODi_Office_Styles* pStyles,
    ODi_Abi_Data& rAbiData,
    ODi_ElementStack& rElementStack) :
        ODi_ListenerState("Frame", rElementStack),
        m_pAbiDocument(pDocument),
        m_rAbiData(rAbiData),
	    m_pStyles(pStyles),
        m_parsedFrameStartTag(false),
        m_inlinedImage(false),
        m_iFrameDepth(0),
        m_pMathBB(NULL),
        m_bInMath(false)
{
    if (m_rElementStack.hasElement("office:document-content")) {
        m_bOnContentStream = true;
    } else {
        m_bOnContentStream = false;
    }
}


/**
 * 
 */
void ODi_Frame_ListenerState::startElement (const gchar* pName,
                                           const gchar** ppAtts,
                                           ODi_ListenerStateAction& rAction) {

    UT_return_if_fail(pName);

    if(m_bInMath && m_pMathBB && (strcmp(pName, "math:math") != 0))
    {
        if (strncmp(pName, "math:", 5) != 0) {
            return;
        }

        m_pMathBB->append(reinterpret_cast<const UT_Byte *>("<"), 1);
        m_pMathBB->append(reinterpret_cast<const UT_Byte *>(pName + 5), strlen(pName) - 5); //build the mathml
        m_pMathBB->append(reinterpret_cast<const UT_Byte *>(">"), 1);
        return;
    }

    if (!strcmp(pName, "draw:frame")) {
        if (m_parsedFrameStartTag) {
            // It's a nested frame.
            rAction.pushState("Frame");
        } else {
            m_parsedFrameStartTag = true;
        }
        
    } else if (!strcmp(pName, "draw:image")) {
        _drawImage(ppAtts, rAction);
        
    } else if (!strcmp(pName, "draw:text-box")) {
        if (m_rElementStack.hasElement("draw:text-box")) {
            // AbiWord doesn't support nested text boxes.
            // Let's ignore that one
            rAction.ignoreElement();
        } else {
            _drawTextBox(ppAtts, rAction);
        }
    } else if (!strcmp(pName, "draw:object")) {
      _drawObject(ppAtts, rAction);

    } else if (!strcmp(pName, "math:math")) {
        
        DELETEP(m_pMathBB);
        m_pMathBB = new UT_ByteBuf;
        m_pMathBB->append(reinterpret_cast<const UT_Byte *>("<math xmlns='http://www.w3.org/1998/Math/MathML' display='block'>"), 65);

        m_bInMath = true;
    }
}


/**
 * 
 */                
void ODi_Frame_ListenerState::endElement (const gchar* pName,
                                         ODi_ListenerStateAction& rAction) {

    UT_return_if_fail(pName);

    if(m_bInMath && m_pMathBB && (strcmp(pName, "math:math") != 0))
    {
        if (strncmp(pName, "math:", 5) != 0) {
            return;
        }

        m_pMathBB->append(reinterpret_cast<const UT_Byte *>("</"), 2);
        m_pMathBB->append(reinterpret_cast<const UT_Byte *>(pName + 5), strlen(pName) - 5); //build the mathml
        m_pMathBB->append(reinterpret_cast<const UT_Byte *>(">"), 1);
        return;
    }

    if (!strcmp(pName, "draw:frame")) {

        if (!m_inlinedImage && (m_iFrameDepth > 0)) {
            if(!m_pAbiDocument->appendStrux(PTX_EndFrame, NULL)) {
                UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
            } else {
                m_iFrameDepth--;
            }
        }

        // We're done.
        rAction.popState();
    } else if (!strcmp(pName, "math:math")) {
        
        if (m_pMathBB) {

            m_pMathBB->append(reinterpret_cast<const UT_Byte *>("</math>"), 7);

            // Create the data item
            UT_uint32 id = m_pAbiDocument->getUID(UT_UniqueId::Math);
            UT_UTF8String sID = UT_UTF8String_sprintf("MathLatex%d", id);
            m_pAbiDocument->createDataItem(sID.utf8_str(), false, m_pMathBB, NULL, NULL);

            const gchar *atts[3] = { NULL, NULL, NULL };
            atts[0] = PT_IMAGE_DATAID;
            atts[1] = sID.utf8_str();
            m_pAbiDocument->appendObject(PTO_Math, atts);

            DELETEP(m_pMathBB);
        }

        m_bInMath = false;
    }
}


/**
 * 
 */
void ODi_Frame_ListenerState::charData (const gchar* pBuffer, int length) 
{
    if (m_bInMath && m_pMathBB) {
        m_pMathBB->append(reinterpret_cast<const UT_Byte *>(pBuffer), length);
        return;
    }
}


/**
 * 
 * @param The attributes of a <draw:image> element.
 */
void ODi_Frame_ListenerState::_drawImage (const gchar** ppAtts,
                                          ODi_ListenerStateAction& rAction)
{
    const gchar* pChar;
    const ODi_Style_Style* pGraphicStyle;
    UT_String dataId; // id of the data item that contains the image.
    
    
    //
    // Adds a reference to the added data item according to anchor mode, etc.
    //
   
    pChar = m_rElementStack.getStartTag(0)->getAttributeValue("draw:style-name");
    UT_ASSERT(pChar);
    
    pGraphicStyle = m_pStyles->getGraphicStyle(pChar, m_bOnContentStream);
    UT_ASSERT(pGraphicStyle);
    
    pChar = m_rElementStack.getStartTag(0)->getAttributeValue("text:anchor-type");
    UT_ASSERT_HARMLESS(pChar);
    
    if ( pChar && (!strcmp(pChar, "as-char") ||
         !strcmp(pChar, "char"))) {
        // In-line wrapping.
        // No frames are used on AbiWord for in-line wrapping.
        // It uses a <image> tag right in the paragraph text.
        
        m_inlinedImage = true;
        
        const gchar* pWidth;
        const gchar* pHeight;
        
        if(!m_rAbiData.addImageDataItem(dataId, ppAtts)) {
            UT_DEBUGMSG(("ODT import: no suitable image importer found\n"));
            return;
        }
       
        const gchar* attribs[5];
        UT_String propsBuffer;
        
        pWidth = m_rElementStack.getStartTag(0)->getAttributeValue("svg:width");
        UT_ASSERT(pWidth);
        
        pHeight = m_rElementStack.getStartTag(0)->getAttributeValue("svg:height");
        UT_ASSERT(pHeight);  
        
        UT_String_sprintf(propsBuffer, "width:%s; height:%s", pWidth, pHeight);
        
        attribs[0] = "props";
        attribs[1] = propsBuffer.c_str();
        attribs[2] = "dataid";
        attribs[3] = dataId.c_str();
        attribs[4] = 0;
    
        if (!m_pAbiDocument->appendObject (PTO_Image, attribs)) {
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
        }
        
    } else {
        // We define a frame with the image in it.
        
        if (m_rElementStack.hasElement("draw:text-box")) {
            // AbiWord can't have nested frames (a framed image inside a textbox).
            // Abort mission!
            rAction.ignoreElement();
            return;
        }
        
        const gchar* attribs[5];
        UT_UTF8String props;
        
        props = "frame-type:image";
            
        if(!_getFrameProperties(props, ppAtts)) {
            // Abort mission!
            rAction.ignoreElement();
            return;
        }
        
        // Avoid having frame border lines.
        props += "; bot-style:none; left-style:none;"
                 " right-style:none; top-style:none";
        
        
        if(!m_rAbiData.addImageDataItem(dataId, ppAtts)) {
            UT_DEBUGMSG(("ODT import: no suitable image importer found\n"));
            return;
        }
        
        attribs[0] = "strux-image-dataid";
        attribs[1] = dataId.c_str();
        attribs[2] = "props";
        attribs[3] = props.utf8_str();
        attribs[4] = 0;
        
        if(!m_pAbiDocument->appendStrux(PTX_SectionFrame, attribs)) {
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
        } else {
            m_iFrameDepth++;
        }
    }

}


/**
 * 
 * @param The attributes of a <draw:image> element.
 */
void ODi_Frame_ListenerState::_drawObject (const gchar** ppAtts,
					   ODi_ListenerStateAction& rAction)
{
    const gchar* pChar = NULL;
    const ODi_Style_Style* pGraphicStyle;
    UT_String dataId; // id of the data item that contains the object.
    
    
    //
    // Adds a reference to the added data item according to anchor mode, etc.
    //
   
    pChar = m_rElementStack.getStartTag(0)->getAttributeValue("draw:style-name");
    UT_ASSERT(pChar);
    
    pGraphicStyle = m_pStyles->getGraphicStyle(pChar, m_bOnContentStream);
    UT_ASSERT(pGraphicStyle);
    
    pChar = m_rElementStack.getStartTag(0)->getAttributeValue("text:anchor-type");
    UT_ASSERT_HARMLESS(pChar);
    
    if ( pChar && (!strcmp(pChar, "as-char") ||
         !strcmp(pChar, "char"))) {
        // In-line wrapping.
        // No frames are used on AbiWord for in-line wrapping.
        // It uses a <image> tag right in the paragraph text.
        
        m_inlinedImage = true;
        
        const gchar* pWidth;
        const gchar* pHeight;
        
	int pto_Type;

        if(!m_rAbiData.addObjectDataItem(dataId, ppAtts, pto_Type)) {
            UT_DEBUGMSG(("ODT import: no suitable object importer found\n"));
            return;
        }
       
        const gchar* attribs[5];
        UT_String propsBuffer;
        
        pWidth = m_rElementStack.getStartTag(0)->getAttributeValue("svg:width");
        UT_ASSERT(pWidth);
        
        pHeight = m_rElementStack.getStartTag(0)->getAttributeValue("svg:height");
        UT_ASSERT(pHeight);  
        
        UT_String_sprintf(propsBuffer, "width:%s; height:%s", pWidth, pHeight);
        
        attribs[0] = "props";
        attribs[1] = propsBuffer.c_str();
        attribs[2] = "dataid";
        attribs[3] = dataId.c_str();
        attribs[4] = 0;
    
        if (!m_pAbiDocument->appendObject ((PTObjectType)pto_Type, attribs)) {
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
        }
        
    } else {
        // We define a frame with the image in it.
        
        if (m_rElementStack.hasElement("draw:text-box")) {
            // AbiWord can't have nested frames (a framed image inside a textbox).
            // Abort mission!
            rAction.ignoreElement();
            return;
        }
        
        const gchar* attribs[5];
        UT_UTF8String props;
        
        props = "frame-type:image";
            
        if(!_getFrameProperties(props, ppAtts)) {
            return;
        }
        
        // Avoid having frame border lines.
        props += "; bot-style:none; left-style:none;"
                 " right-style:none; top-style:none";
        
        int pto_Type;
        if(!m_rAbiData.addObjectDataItem(dataId, ppAtts, pto_Type)) {
            UT_DEBUGMSG(("ODT import: no suitable object importer found\n"));
            return;
        }
        
        attribs[0] = "strux-image-dataid";
        attribs[1] = dataId.c_str();
        attribs[2] = "props";
        attribs[3] = props.utf8_str();
        attribs[4] = 0;
        
        if(!m_pAbiDocument->appendStrux(PTX_SectionFrame, attribs)) {
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
        } else {
            m_iFrameDepth++;
        }
    }

}


/**
 * @param ppAtts The attributes of a <draw:text-box> element.
 * @param rAction Any action to be taken, regarding state change.
 */
void ODi_Frame_ListenerState::_drawTextBox (const gchar** ppAtts,
                                           ODi_ListenerStateAction& rAction) {

    const gchar* attribs[3];
    const gchar* pStyleName = NULL;
    const ODi_Style_Style* pGraphicStyle = NULL;
    UT_UTF8String props;
    
    props = "frame-type:textbox";
            
    if (!_getFrameProperties(props, ppAtts)) {
        // Abort mission!

        // <draw:frame>     -  0
        //  <draw:text-box> - -1
        //
        // We want to ignore the whole frame.
        rAction.ignoreElement(0);
        return;
    }
    
    if (!props.empty()) {
        props += "; ";
    }

    // TODO: translate border thicknesses

    if (m_rElementStack.getStartTag(0)) {
        pStyleName = m_rElementStack.getStartTag(0)->getAttributeValue("draw:style-name");
    }

    if (pStyleName) {
        pGraphicStyle = m_pStyles->getGraphicStyle(pStyleName, m_bOnContentStream);
    }

    if (pGraphicStyle) {
        // For now, we'll assume HAVE_BORDER_UNSPECIFIED == HAVE_BORDER_YES
        if (pGraphicStyle->hasBottomBorder() != ODi_Style_Style::HAVE_BORDER_NO) {
            props += "bot-style:1";
            if(pGraphicStyle->getBorderBottom_color() && !pGraphicStyle->getBorderBottom_color()->empty()) {
                props += "; bot-color:";
                props += *(pGraphicStyle->getBorderBottom_color());
            }
        } else {
            props += "bot-style:0";
        }

        if (pGraphicStyle->hasLeftBorder() != ODi_Style_Style::HAVE_BORDER_NO) {
            props += "; left-style:1";
            if(pGraphicStyle->getBorderLeft_color() && !pGraphicStyle->getBorderLeft_color()->empty()) {
                props += "; left-color:";
                props += *(pGraphicStyle->getBorderLeft_color());
            }
        } else {
            props += "; left-style:0";
        }

        if (pGraphicStyle->hasRightBorder() != ODi_Style_Style::HAVE_BORDER_NO) {
            props += "; right-style:1";
            if(pGraphicStyle->getBorderRight_color() && !pGraphicStyle->getBorderRight_color()->empty()) {
                props += "; right-color:";
                props += *(pGraphicStyle->getBorderRight_color());
            }
        } else {
            props += "; right-style:0";
        }

        if (pGraphicStyle->hasTopBorder() != ODi_Style_Style::HAVE_BORDER_NO) {
            props += "; top-style:1";
            if(pGraphicStyle->getBorderTop_color() && !pGraphicStyle->getBorderTop_color()->empty()) {
                props += "; top-color:";
                props += *(pGraphicStyle->getBorderTop_color());
            }
        } else {
            props += "; top-style:0";
        }
    } else {  //just hard-code some defaults
        props += "bot-style:1; left-style:1; right-style:1; top-style:1";
    }

    attribs[0] = "props";
    attribs[1] = props.utf8_str();
    attribs[2] = 0;

    if(!m_pAbiDocument->appendStrux(PTX_SectionFrame, attribs)) {
        UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
    } else {
        m_iFrameDepth++;
    }

    // We are going to receive text content.
    rAction.pushState("TextContent");
}


/**
 * 
 */
bool ODi_Frame_ListenerState::_getFrameProperties(UT_UTF8String& rProps,
                                                   const gchar** ppAtts) {

    const gchar* pStyleName;
    const ODi_Style_Style* pGraphicStyle;
    const UT_UTF8String* pWrap;
    const UT_UTF8String* pBackgroundColor;
    const gchar* pVal = NULL;
    
    pStyleName = m_rElementStack.getStartTag(0)->getAttributeValue("draw:style-name");
    UT_ASSERT(pStyleName);
    
    pGraphicStyle = m_pStyles->getGraphicStyle(pStyleName, m_bOnContentStream);
    UT_return_val_if_fail(pGraphicStyle, false);
    
    pWrap = pGraphicStyle->getWrap(false);
                                                    
    if ( !strcmp(pWrap->utf8_str(), "run-through")) {
        // Floating wrapping.
        rProps += "; wrap-mode:above-text";
        
    } else if ( !strcmp(pWrap->utf8_str(), "left")) {
        rProps += "; wrap-mode:wrapped-to-left";
        
    } else if ( !strcmp(pWrap->utf8_str(), "right")) {
        rProps += "; wrap-mode:wrapped-to-right";
        
    } else if ( !strcmp(pWrap->utf8_str(), "parallel")) {
        rProps += "; wrap-mode:wrapped-both";
        
    } else {
        // Unsupported.        
        // Let's put an arbitrary wrap mode to avoid an error.
        rProps += "; wrap-mode:wrapped-both";
    }


    pBackgroundColor = pGraphicStyle->getBackgroundColor();
    if(pBackgroundColor && pBackgroundColor->length()) {
        rProps += "; background-color:";
        rProps += pBackgroundColor->utf8_str();
    }

    
    pVal = m_rElementStack.getStartTag(0)->getAttributeValue("text:anchor-type");

    if (pVal && !strcmp(pVal, "paragraph")) {
        rProps += "; position-to:block-above-text";
        
        pVal = m_rElementStack.getStartTag(0)->getAttributeValue("svg:x");
        if (pVal) {
            rProps += "; xpos:";
            rProps += pVal;
        }
        
        pVal = m_rElementStack.getStartTag(0)->getAttributeValue("svg:y");
        if (pVal) {
            rProps += "; ypos:";
            rProps += pVal;
        }
        
    } else if (pVal && !strcmp(pVal, "page")) {
        rProps += "; position-to:page-above-text";
        
        pVal = m_rElementStack.getStartTag(0)->getAttributeValue("svg:x");
        UT_ASSERT(pVal);
        rProps += "; frame-page-xpos:";
        rProps += pVal;
        
        pVal = m_rElementStack.getStartTag(0)->getAttributeValue("svg:y");
        UT_ASSERT(pVal);
        rProps += "; frame-page-ypos:";
        rProps += pVal;
        
    } else {
        // "char" or "as-char"
        // AbiWord doesn't have this kind of inlined frame/textbox.

        return false;
    }
    
    // From the OpenDocument standard v1.0, about text boxes properties:
    //
    // "The fo:min-height and fo:min-width attributes specify a minimum height
    // or width for a text box. If they are existing, they overwrite the height
    // or width of a text box specified by the svg:height and svg:width
    // attributes of the surrounding <draw:frame> element."
    
    // TODO: make a robust support for the relation between fo:min-width/height
    //       and svg:width/height on both <draw:frame> and <draw:text-box>
    
    pVal = UT_getAttribute("fo:min-width", ppAtts);
    if (pVal == NULL) {
        pVal = m_rElementStack.getStartTag(0)->getAttributeValue("svg:width");
        if (pVal == NULL) {
            pVal = m_rElementStack.getStartTag(0)->getAttributeValue("fo:min-width");
            if (UT_determineDimension(pVal, DIM_none) == DIM_PERCENT) {
                // TODO: Do the conversion from percentage to a real
                //       unit (ie: "cm" or "in").
                UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
            }
        }
    } else {
        if (UT_determineDimension(pVal, DIM_none) == DIM_PERCENT) {
            // TODO: Do the conversion from percentage to a real
            //       unit (ie: "cm" or "in").
            UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
        }
    }
    UT_ASSERT_HARMLESS(pVal); // Is it really ok to not have a defined width?
    if (pVal) {
        rProps += "; frame-width:";
        rProps += pVal;
    }
    
    
    
    pVal = UT_getAttribute("fo:min-height", ppAtts);
    if (pVal == NULL) {
        pVal = m_rElementStack.getStartTag(0)->getAttributeValue("svg:height");
        if (pVal == NULL) {
            pVal = m_rElementStack.getStartTag(0)->getAttributeValue("fo:min-height");
            if (UT_determineDimension(pVal, DIM_none) == DIM_PERCENT) {
                // TODO: Do the conversion from percentage to a real
                //       unit (ie: "cm" or "in").
                UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
            }
        }
    } else {
        if (UT_determineDimension(pVal, DIM_none) == DIM_PERCENT) {
            // TODO: Do the conversion from percentage to a real
            //       unit (ie: "cm" or "in").
            UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
        }
    }
    if (pVal) {
        rProps += "; frame-height:";
        rProps += pVal;
    }
    
    return true;
}
