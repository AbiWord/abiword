/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
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
#include <ut_locale.h>
#include <ut_units.h>
#include <ie_math_convert.h>

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
        m_bInMath(false),
		m_bInlineImagePending(false),
		m_bPositionedImagePending(false),
		m_bInAltTitle(false),
		m_bInAltDesc(false)
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
	} else if (!strcmp(pName, "svg:title")) {
		m_bInAltTitle = true;
	} else if (!strcmp(pName, "svg:desc")) {
		m_bInAltDesc = true;
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

        m_pMathBB.reset(new UT_ByteBuf);
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
		
		if (m_bInlineImagePending || m_bPositionedImagePending)
		{
			if (!m_sAltTitle.empty())
				m_mPendingImgProps["title"] = m_sAltTitle;
			if (!m_sAltDesc.empty())
				m_mPendingImgProps["alt"] = m_sAltDesc;

			// write out the pending image
			PP_PropertyVector attribs;
			for (auto cit = m_mPendingImgProps.cbegin(); cit != m_mPendingImgProps.cend(); cit++)
			{
				attribs.push_back(cit->first);
				attribs.push_back(cit->second);
			}

			if (m_bInlineImagePending)
			{
				if (!m_pAbiDocument->appendObject (PTO_Image, attribs)) {
					UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
				}
				
				m_bInlineImagePending = false;
			}
			else if (m_bPositionedImagePending)
			{
				if(!m_pAbiDocument->appendStrux(PTX_SectionFrame, attribs)) {
					UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
				} else {
					m_iFrameDepth++;
				}
				m_bPositionedImagePending = false;
			}

			m_sAltTitle = "";
			m_sAltDesc = "";
			m_mPendingImgProps.clear();
		}

        if (!m_inlinedImage && (m_iFrameDepth > 0)) {
            if(!m_pAbiDocument->appendStrux(PTX_EndFrame, PP_NOPROPS)) {
                UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
            } else {
                m_iFrameDepth--;
            }
        }

        // We're done.
        rAction.popState();
	} else if (!strcmp(pName, "svg:title")) {
		m_bInAltTitle = false;
	} else if (!strcmp(pName, "svg:desc")) {
		m_bInAltDesc = false;
    } else if (!strcmp(pName, "math:math")) {
        
        if (m_pMathBB) {

            m_pMathBB->append(reinterpret_cast<const UT_Byte *>("</math>"), 7);

            // Create the data item
            UT_uint32 id = m_pAbiDocument->getUID(UT_UniqueId::Math);
			std::string sID = UT_std_string_sprintf("MathLatex%d", id);

            std::string lID;
			lID.assign("LatexMath");
     	    lID.append((sID.substr(9,sID.size()-8)).c_str());
			
      	    UT_ByteBufPtr latexBuf(new UT_ByteBuf);
   	    UT_UTF8String PMathml = (const char*)(m_pMathBB->getPointer(0));
	    UT_UTF8String PLatex,Pitex;

	    m_pAbiDocument->createDataItem(sID.c_str(), false, m_pMathBB, "", NULL);

	    if(convertMathMLtoLaTeX(PMathml, PLatex) && convertLaTeXtoEqn(PLatex,Pitex))
 	    {
		// Conversion of MathML to LaTeX and the Equation Form suceeds
		latexBuf->ins(0, reinterpret_cast<const UT_Byte *>(Pitex.utf8_str()), static_cast<UT_uint32>(Pitex.size()));
		m_pAbiDocument->createDataItem(lID.c_str(), false, latexBuf, "", NULL);
    	    }

            const PP_PropertyVector atts = {
				PT_IMAGE_DATAID, sID,
				"latexid", lID
			};
            m_pAbiDocument->appendObject(PTO_Math, atts);

            m_pMathBB.reset();
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
	} else if (m_bInAltTitle) {
		m_sAltTitle += std::string(reinterpret_cast<const char*>(pBuffer), length);
	} else if (m_bInAltDesc) {
		m_sAltDesc += std::string(reinterpret_cast<const char*>(pBuffer), length);
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
    UT_String dataId; // id of the data item that contains the image.
 
	UT_return_if_fail(!m_bInlineImagePending && !m_bPositionedImagePending);
    
    //
    // Adds a reference to the added data item according to anchor mode, etc.
    //
   
    pChar = m_rElementStack.getStartTag(0)->getAttributeValue("draw:style-name");
    UT_ASSERT(pChar);
    
    UT_DebugOnly<const ODi_Style_Style*> pGraphicStyle = m_pStyles->getGraphicStyle(pChar, m_bOnContentStream);
    UT_ASSERT(pGraphicStyle);
    
    pChar = m_rElementStack.getStartTag(0)->getAttributeValue("text:anchor-type");
    UT_ASSERT_HARMLESS(pChar);
    
	// as-char anchoring maps to abiword's inline images
	// Note: AbiWord does not support positioned images in Headers and Footers,
	// so convert those to inlined images.
    if ( pChar && 
		 (!strcmp(pChar, "as-char" ) || 
		   m_rElementStack.hasElement("style:header") || 
		   m_rElementStack.hasElement("style:footer") )) {
        // No frames are used on AbiWord for in-line wrapping: it
        // uses a <image> tag right in the paragraph text.
        _drawInlineImage(ppAtts);
    } else {
        // This is a positiioned image. In AbiWord we define a frame 
		// and place the image in it.

        if (m_rElementStack.hasElement("draw:text-box")) {
            // AbiWord can't have nested frames (a framed image inside a textbox),
            // so convert it to an inline image for now

            _drawInlineImage(ppAtts);
            return;
        }
        
        std::string props = "frame-type:image";
            
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
        
		m_mPendingImgProps["strux-image-dataid"] = dataId.c_str();
        m_mPendingImgProps["props"] = props.c_str();
        
		// don't write the image out yet as we might get more properties, for
		// example alt descriptions from the <svg:desc> tag
		m_bPositionedImagePending = true;
    }

}

void ODi_Frame_ListenerState::_drawInlineImage (const gchar** ppAtts)
{
    const gchar* pWidth = NULL;
    const gchar* pHeight = NULL;
    UT_String dataId;

    m_inlinedImage = true;

    if(!m_rAbiData.addImageDataItem(dataId, ppAtts)) {
        UT_DEBUGMSG(("ODT import: no suitable image importer found\n"));
        return;
    }

    UT_String propsBuffer;
        
    pWidth = m_rElementStack.getStartTag(0)->getAttributeValue("svg:width");
    UT_ASSERT(pWidth);
        
    pHeight = m_rElementStack.getStartTag(0)->getAttributeValue("svg:height");
    UT_ASSERT(pHeight);  
        
    UT_String_sprintf(propsBuffer, "width:%s; height:%s", pWidth, pHeight);
        
	m_mPendingImgProps["props"] = propsBuffer.c_str();
	m_mPendingImgProps["dataid"] = dataId.c_str();

	// don't write the image out yet as we might get more properties, for
	// example alt descriptions from the <svg:desc> tag
	m_bInlineImagePending = true;
}

/**
 * 
 * @param The attributes of a <draw:image> element.
 */
void ODi_Frame_ListenerState::_drawObject (const gchar** ppAtts,
					   ODi_ListenerStateAction& rAction)
{
    const gchar* pChar = NULL;
    UT_String dataId; // id of the data item that contains the object.
    
    
    //
    // Adds a reference to the added data item according to anchor mode, etc.
    //
   
    pChar = m_rElementStack.getStartTag(0)->getAttributeValue("draw:style-name");
    UT_ASSERT(pChar);
    
    UT_DebugOnly<const ODi_Style_Style*> pGraphicStyle;
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
		 

	std::string extraID;
	std::string objectID;
	objectID = (dataId.substr(9,dataId.length()-8)).c_str();
	extraID.assign("LatexMath");
	extraID.append(objectID.c_str());

	   
        
        pWidth = m_rElementStack.getStartTag(0)->getAttributeValue("svg:width");
        UT_ASSERT(pWidth);
        
        pHeight = m_rElementStack.getStartTag(0)->getAttributeValue("svg:height");
        UT_ASSERT(pHeight);  
        
        std::string propsBuffer =
            UT_std_string_sprintf("width:%s; height:%s", pWidth, pHeight);
        
        PP_PropertyVector attribs = {
            "props", propsBuffer,
            "dataid", dataId.c_str(),
            "latexid", extraID
        };
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
        
        std::string props = "frame-type:image";
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
        
		m_mPendingImgProps["strux-image-dataid"] = dataId.c_str();
		m_mPendingImgProps["props"] = props.c_str();
        
		// don't write the image out yet as we might get more properties, for
		// example alt descriptions from the <svg:desc> tag
		m_bPositionedImagePending = true;
    }

}


static bool _convertBorderThickness(const char* szIncoming, std::string& sConverted)
{
    UT_return_val_if_fail(szIncoming && *szIncoming, false);

    double d = 0.0f;
    UT_Dimension units = UT_determineDimension (szIncoming, DIM_none);

    if (units == DIM_none) {
        // no (valid) dimension specified, we'll assume inches
        d = UT_convertToInches(szIncoming);
        d = UT_convertInchesToDimension(d, DIM_PT);

    } else {
        d = UT_convertToPoints(szIncoming);
    }

    UT_LocaleTransactor t(LC_NUMERIC, "C");
    sConverted = UT_std_string_sprintf("%.2fpt", d);

    return true;
}


/**
 * @param ppAtts The attributes of a <draw:text-box> element.
 * @param rAction Any action to be taken, regarding state change.
 */
void ODi_Frame_ListenerState::_drawTextBox (const gchar** ppAtts,
                                           ODi_ListenerStateAction& rAction) {
    const gchar* pStyleName = NULL;
    const ODi_Style_Style* pGraphicStyle = NULL;
    std::string props;
    std::string sThickness;
    
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

        if(pGraphicStyle->getBorderBottom_thickness() && !pGraphicStyle->getBorderBottom_thickness()->empty()) {

            sThickness.clear();
            bool bRet = _convertBorderThickness(pGraphicStyle->getBorderBottom_thickness()->c_str(), sThickness);

            if(bRet) {
                props += "; bot-thickness:";
                props += sThickness.c_str();
            }
        }

        if(pGraphicStyle->getBorderLeft_thickness() && !pGraphicStyle->getBorderLeft_thickness()->empty()) {

            sThickness.clear();
            bool bRet = _convertBorderThickness(pGraphicStyle->getBorderLeft_thickness()->c_str(), sThickness);

            if(bRet) {
                props += "; left-thickness:";
                props += sThickness.c_str();
            }
        }

        if(pGraphicStyle->getBorderRight_thickness() && !pGraphicStyle->getBorderRight_thickness()->empty()) {

            sThickness.clear();
            bool bRet = _convertBorderThickness(pGraphicStyle->getBorderRight_thickness()->c_str(), sThickness);

            if(bRet) {
                props += "; right-thickness:";
                props += sThickness.c_str();
            }
        }

        if(pGraphicStyle->getBorderTop_thickness() && !pGraphicStyle->getBorderTop_thickness()->empty()) {

            sThickness.clear();
            bool bRet = _convertBorderThickness(pGraphicStyle->getBorderTop_thickness()->c_str(), sThickness);

            if(bRet) {
                props += "; top-thickness:";
                props += sThickness.c_str();
            }
        }


        if(pGraphicStyle->getHorizPos(true) && !pGraphicStyle->getHorizPos(true)->empty()) 
	{
	       props += "; frame-horiz-align:";
	       props += *(pGraphicStyle->getHorizPos(true));
            
        }


    } else {  //just hard-code some defaults
        props += "bot-style:1; left-style:1; right-style:1; top-style:1";
    }

    const PP_PropertyVector attribs = {
        "props", props
    };
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
bool ODi_Frame_ListenerState::_getFrameProperties(std::string& rProps,
                                                   const gchar** ppAtts) {

    const gchar* pStyleName;
    const ODi_Style_Style* pGraphicStyle;
    const std::string* pWrap;
    const std::string* pBackgroundColor;
    const gchar* pVal = NULL;
    
    pStyleName = m_rElementStack.getStartTag(0)->getAttributeValue("draw:style-name");
    UT_ASSERT(pStyleName);
    
    pGraphicStyle = m_pStyles->getGraphicStyle(pStyleName, m_bOnContentStream);
    UT_return_val_if_fail(pGraphicStyle, false);
    
    pWrap = pGraphicStyle->getWrap(false);
                                                    
    if ( !strcmp(pWrap->c_str(), "run-through")) {
        // Floating wrapping.
        rProps += "; wrap-mode:above-text";
        
    } else if ( !strcmp(pWrap->c_str(), "left")) {
        rProps += "; wrap-mode:wrapped-to-left";
        
    } else if ( !strcmp(pWrap->c_str(), "right")) {
        rProps += "; wrap-mode:wrapped-to-right";
        
    } else if ( !strcmp(pWrap->c_str(), "parallel")) {
        rProps += "; wrap-mode:wrapped-both";
        
    } else {
        // Unsupported.        
        // Let's put an arbitrary wrap mode to avoid an error.
        rProps += "; wrap-mode:wrapped-both";
    }


    pBackgroundColor = pGraphicStyle->getBackgroundColor();
    if(pBackgroundColor && pBackgroundColor->length()) {
        rProps += "; background-color:";
        rProps += pBackgroundColor->c_str();
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
        if (pVal && *pVal) {
            rProps += "; frame-page-xpos:";
            rProps += pVal;
        }
        
        pVal = m_rElementStack.getStartTag(0)->getAttributeValue("svg:y");
        if (pVal && *pVal) {
            rProps += "; frame-page-ypos:";
            rProps += pVal;
        }
        
    } else if (pVal && (!strcmp(pVal, "char") || !strcmp(pVal, "as-char"))) {
		// AbiWord does not support anchoring frames/texboxes to chars; 
		// let's just convert it to paragraph anchoring, so we don't lose the 
		// entire frame
		// FIXME: "char" means an inline thing in AbiWord terms, NOT a positioned thing
		rProps += "; position-to:block-above-text";

	    pVal = m_rElementStack.getStartTag(0)->getAttributeValue("svg:x");
        if (pVal && *pVal) {
            rProps += "; xpos:";
            rProps += pVal;
        }
        
        pVal = m_rElementStack.getStartTag(0)->getAttributeValue("svg:y");
        if (pVal && *pVal) {
            rProps += "; ypos:";
            rProps += pVal;
        }
	} else {	
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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
    if (pVal) 
    {
        rProps += "; frame-width:";
        rProps += pVal;
    }
    
    pVal = UT_getAttribute("style:rel-width", ppAtts);
    if(pVal)
    {
        rProps += "; frame-rel-width:";
        rProps += pVal;
    }
    else
    {
         pVal = m_rElementStack.getStartTag(0)->getAttributeValue("style:rel-width");
	 if(pVal)
	 {
	      rProps += "; frame-rel-width:";
	      rProps += pVal;
	 }
    }

    pVal = UT_getAttribute("fo:min-height", ppAtts);
    if (pVal == NULL) 
    {
        pVal = m_rElementStack.getStartTag(0)->getAttributeValue("svg:height");
        if (pVal == NULL) {
            pVal = m_rElementStack.getStartTag(0)->getAttributeValue("fo:min-height");
            if (UT_determineDimension(pVal, DIM_none) == DIM_PERCENT) {
                // TODO: Do the conversion from percentage to a real
                //       unit (ie: "cm" or "in").
                UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
            }
        }
    } 
    else 
    {
        if (UT_determineDimension(pVal, DIM_none) == DIM_PERCENT) 
	{
            // TODO: Do the conversion from percentage to a real
            //       unit (ie: "cm" or "in").
            UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
        }
	rProps += "; frame-min-height:";
	rProps += pVal;
    }
    if (pVal) {
        rProps += "; frame-height:";
        rProps += pVal;
    }
    
    return true;
}
