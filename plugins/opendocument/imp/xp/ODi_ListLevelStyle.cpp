/* AbiSource Program Utilities
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
#include "ODi_ListLevelStyle.h"

// Internal includes
#include "ODi_ListenerStateAction.h"
#include "ODi_Style_Style.h"
#include "ODi_ListLevelStyleFormats.h"

// AbiWord includes
#include <pd_Document.h>
#include <fp_types.h>
#include <ut_string_class.h>
#include <ut_units.h>
#include <ut_locale.h>

//External includes
#include <stdlib.h>


/**
 * Constructor
 */
ODi_ListLevelStyle::ODi_ListLevelStyle(const char* pStateName,
									 ODi_ElementStack& rElementStack) :
                        ODi_ListenerState(pStateName, rElementStack),
                        m_pTextStyle(NULL)
{
}


/**
 * 
 */
void ODi_ListLevelStyle::startElement (const gchar* pName,
                                      const gchar** ppAtts,
									   ODi_ListenerStateAction& /*rAction*/) 
{
    const gchar* pVal = NULL;

    if (!strcmp("text:list-level-style-bullet", pName) ||
        !strcmp("text:list-level-style-number", pName) ||
	!strcmp("text:outline-level-style",pName) ||
        !strcmp("text:list-level-style-image", pName)) {

        UT_uint32 result = 0;
            
        pVal = UT_getAttribute ("text:level", ppAtts);
        if (pVal) {
            result = sscanf(pVal, "%u", &m_levelNumber);
            // TODO: check result?
            m_level = pVal;
        } else {
            UT_DEBUGMSG(("ODi_ListLevelStyle::startElement: missing text:level attribute\n"));
        }
	bool bHeading = false;
	if(!strcmp("text:outline-level-style",pName))
	{
	  bHeading = true;
	}
        pVal = UT_getAttribute ("text:style-name", ppAtts);
        if (pVal) 
	{
            m_textStyleName = pVal;
        }
        else if(bHeading)
	{
	    UT_UTF8String sStyleName = "BaseHeading ";
	    sStyleName += m_level;
	    m_textStyleName =  sStyleName;
	    UT_DEBUGMSG(("Outline List level Style name %s \n",sStyleName.utf8_str()));
	     pVal = UT_getAttribute ("style:num-format", ppAtts);
	     if(pVal && *pVal)
	     {
		 if (pVal && !strcmp(pVal, "")) 
		 {
		     // We have an empty number format.
            
		     // Empty list label or "invisible" list.
		     m_abiListListDelim = "";
		 }
	       
	     }
	}
    } else if (!strcmp("style:list-level-properties", pName) ||
               !strcmp("style:list-level-label-alignment", pName)) {

        pVal = UT_getAttribute ("text:space-before", ppAtts);
        if (pVal) {
            m_spaceBefore = pVal;
        } else {
            m_spaceBefore = "0cm";
        }
        
        pVal = UT_getAttribute ("text:min-label-width", ppAtts);
        if (pVal) {
            m_minLabelWidth = pVal;
        } else {
            m_minLabelWidth = "0cm";
        }

        pVal = UT_getAttribute ("text:min-label-distance", ppAtts);
        if (pVal) {
            m_minLabelDistance = pVal;
        }

        pVal = UT_getAttribute ("fo:text-indent", ppAtts);
        if (pVal) {
            m_textIndent = pVal;
        }

        pVal = UT_getAttribute ("fo:margin-left", ppAtts);
        if (pVal) {
            m_marginLeft = pVal;
        }
    }
}


/**
 * 
 */
void ODi_ListLevelStyle::endElement (const gchar* pName,
                                    ODi_ListenerStateAction& rAction) {
                                        
    if (!strcmp("text:list-level-style-bullet", pName) ||
        !strcmp("text:list-level-style-number", pName) ||
	!strcmp("text:outline-level-style",pName) ||
        !strcmp("text:list-level-style-image", pName)) {
            
        // We're done.
        UT_DEBUGMSG(("Finished Level %s \n",m_textStyleName.utf8_str()));
        rAction.popState();
    }
}

bool  ODi_ListLevelStyle::isVisible(void) const
{
  return  (m_abiListListDelim.size() > 0);
}

/**
 * 
 */
void ODi_ListLevelStyle::setAbiListID(UT_uint32 abiListID) {
    gchar buffer[100];
    
    sprintf(buffer, "%u", abiListID);
    m_abiListID.assign(buffer);
}


/**
 * Defines a <l> tag on the AbiWord document corresponding to this
 * list level style.
 */
void ODi_ListLevelStyle::defineAbiList(PD_Document* pDocument) {
    const gchar* ppAttr[13];
    
    ppAttr[0] = "id";
    ppAttr[1] = m_abiListID.utf8_str();
    ppAttr[2] = "parentid";
    ppAttr[3] = m_abiListParentID.utf8_str();
    ppAttr[4] = "type";
    ppAttr[5] = m_abiListType.utf8_str();
    ppAttr[6] = "start-value";
    ppAttr[7] = m_abiListStartValue.utf8_str();
    ppAttr[8] = "list-delim";
    ppAttr[9] = m_abiListListDelim.utf8_str();
    ppAttr[10] = "list-decimal";
    ppAttr[11] = m_abiListListDecimal.utf8_str();
    ppAttr[12] = 0;
    
    pDocument->appendList(ppAttr);
}


/**
 * 
 */
void ODi_ListLevelStyle::buildAbiPropsString() {
    m_abiProperties.clear();
}


/**
 * The AbiWord properties of the list depends on some properties already
 * defined by the AbiWord paragraph style.
 * 
 * @param rProps Will have the properties string appended.
 * @param pStyle Pointer to the paragraph style used on this list paragraph.
 */    
void ODi_ListLevelStyle::getAbiProperties(UT_UTF8String& rProps,
                                         const ODi_Style_Style* pStyle) const {

    // Adds the fixed portion of the properties.
    if (!m_abiProperties.empty()) {
        if (!rProps.empty()) {
            rProps += "; ";
        }
        rProps += m_abiProperties;
    }
    
    // Precedence of list styles properties:
    //
    // 1. The properties of the style denoted by the paragraph's style:list-style-name, overridden by
    // 2. The properties of the paragraph's parent style, overridden by
    // 3. The properties of the paragraph style

    UT_UTF8String odMarginLeft;
    UT_UTF8String odTextIndent;

    // 1. The properties of the style denoted by the paragraph's style:list-style-name
    if (!pStyle->getListStyleName().empty())
    {
        if (!m_marginLeft.empty())
            odMarginLeft = m_marginLeft;
        if (!m_textIndent.empty())
            odTextIndent = m_textIndent;
    }

    // 2. The properties of the paragraph's parent style
    if (pStyle != NULL && !strcmp(pStyle->getFamily()->utf8_str(), "paragraph")) {
        const ODi_Style_Style* pParentStyle = pStyle->getParent();
        if (pParentStyle != NULL && !strcmp(pParentStyle->getFamily()->utf8_str(), "paragraph")) {
            if (pStyle->getMarginLeft() && !pStyle->getMarginLeft()->empty())
                odMarginLeft = *(pStyle->getMarginLeft());
            if (pStyle->getTextIndent() && !pStyle->getTextIndent()->empty())
                odTextIndent = *(pStyle->getTextIndent());
        }
    }

    // 3. The properties of the paragraph style
    if (pStyle != NULL && !strcmp(pStyle->getFamily()->utf8_str(), "paragraph")) {
        if (pStyle->getMarginLeft() && !pStyle->getMarginLeft()->empty())
            odMarginLeft = *(pStyle->getMarginLeft());
        if (pStyle->getTextIndent() && !pStyle->getTextIndent()->empty())
            odTextIndent = *(pStyle->getTextIndent());
    }

    // default to some 'sane' values if not set
    if (odMarginLeft.empty())
        odMarginLeft = "0.0cm";
    if (odTextIndent.empty())
        odTextIndent = "0.0cm";

    // From the OpenDocument OASIS standard, v1.0:
    //
    // "The text:space-before attribute specifies the space to include before
    // the number for all paragraphs at this level. If a paragraph has a left
    // margin that is greater than 0, the actual position of the list label box
    // is the left margin width plus the start indent value."
    //
    // AbiWord's margin-left = OpenDocument paragraph property fo:margin-left +
    //                         OpenDocument text:space-before +
    //                         OpenDocument text:min-label-witdh
    //
    // OpenDocument fo:margin-left + fo:text-indent + text:space-before = AbiWord's margin-left + text-indent.
    //

    double spaceBefore_cm;
    double minLabelWidth_cm;
    double marginLeft_cm;
    double textIndent_cm;

    gchar buffer[100];
    UT_LocaleTransactor lt(LC_NUMERIC, "C");
    
    spaceBefore_cm = UT_convertToDimension(m_spaceBefore.utf8_str(), DIM_CM);
    minLabelWidth_cm = UT_convertToDimension(m_minLabelWidth.utf8_str(), DIM_CM);
    marginLeft_cm = UT_convertToDimension(odMarginLeft.utf8_str(), DIM_CM);
    textIndent_cm = UT_convertToDimension(odTextIndent.utf8_str(), DIM_CM);
   
    double abiMarginLeft = marginLeft_cm + spaceBefore_cm + minLabelWidth_cm;
    sprintf(buffer, "%fcm", abiMarginLeft);
                            
    if (!rProps.empty()) {
        rProps += "; ";
    }
    rProps += "margin-left:";
    rProps.append(buffer);
    
    sprintf(buffer, "%fcm", marginLeft_cm + textIndent_cm + spaceBefore_cm - abiMarginLeft);
    rProps += "; text-indent:";
    rProps.append(buffer);
}


/******************************************************************************/


/**
 * 
 */
ODi_Bullet_ListLevelStyle::ODi_Bullet_ListLevelStyle(ODi_ElementStack& rElementStack)
	: ODi_ListLevelStyle("Bullet_ListLevelStyle", rElementStack)
{
    // Dummy values
    m_abiListStartValue.assign("0");
    m_abiListListDelim.assign("%L");
    m_abiListListDecimal.assign("NULL");
}


/**
 * 
 */
void ODi_Bullet_ListLevelStyle::startElement(const gchar* pName,
                                             const gchar** ppAtts,
                                             ODi_ListenerStateAction& rAction) {

    const gchar* pVal = NULL;
    UT_UCS4String ucs4Str;
    
    // Let the parent class do the processing common to all list types.
    ODi_ListLevelStyle::startElement (pName, ppAtts, rAction);

    if (!strcmp("text:list-level-style-bullet", pName)) {
        pVal = UT_getAttribute ("text:bullet-char", ppAtts);
        
        if (pVal != NULL) {
        
            ucs4Str = pVal;
            
            if (!ucs4Str.empty()) {
                switch (ucs4Str[0]) {
                    case 8226: // U+2022 BULLET
                        // Bullet List
                        UT_UTF8String_sprintf(m_abiListType, "%d", BULLETED_LIST);
                        break;
                        
                    case 8211: // U+2013 EN DASH
                    case 8722: // U+2212 MINUS SIGN
                        // Dashed List
                        UT_UTF8String_sprintf(m_abiListType, "%d", DASHED_LIST);
                        break;
                        
                    case 9632: // U+25A0 BLACK SQUARE
                        // Square List
                        UT_UTF8String_sprintf(m_abiListType, "%d", SQUARE_LIST);
                        break;
                        
                    case 9650: // U+25B2 BLACK UP-POINTING TRIANGLE
                        // Triangle List
                        UT_UTF8String_sprintf(m_abiListType, "%d", TRIANGLE_LIST);
                        break;
                    
                    case 9830: // U+2666 BLACK DIAMOND SUIT
                        // Diamond List
                        UT_UTF8String_sprintf(m_abiListType, "%d", DIAMOND_LIST);
                        break;
                        
                    case 10035: // U+2733 EIGHT SPOKED ASTERISK
                        // Star List
                        UT_UTF8String_sprintf(m_abiListType, "%d", STAR_LIST);
                        break;
                        
                    case 10003: // U+2713 CHECK MARK
                        // Tick List
                        UT_UTF8String_sprintf(m_abiListType, "%d", TICK_LIST);
                        break;
                        
                    case 10066: // U+2752 UPPER RIGHT SHADOWED WHITE SQUARE
                        // Box List
                        UT_UTF8String_sprintf(m_abiListType, "%d", BOX_LIST);
                        break;
                        
                    case 9758: // U+261E WHITE RIGHT POINTING INDEX
                        // Hand List
                        UT_UTF8String_sprintf(m_abiListType, "%d", HAND_LIST);
                        break;
                        
                    case 9829: // U+2665 BLACK HEART SUIT
                        // Heart List
                        UT_UTF8String_sprintf(m_abiListType, "%d", HEART_LIST);
                        break;
                        
                    case 8658: // U+21D2 RIGHTWARDS DOUBLE ARROW
                        // Implies List
                        UT_UTF8String_sprintf(m_abiListType, "%d", IMPLIES_LIST);
                        break;
                        
                    default:
                        // Bullet List
                        UT_UTF8String_sprintf(m_abiListType, "%d", BULLETED_LIST);
                };
                
            } // if (!ucs4Str.empty())
        } else /* from if (pVal != NULL) */ {
            // Bullet List
            UT_UTF8String_sprintf(m_abiListType, "%d", BULLETED_LIST);
        }
        
    } else if (!strcmp("text:list-level-style-image", pName)) {
        // Force it into a default Bullet List
        UT_UTF8String_sprintf(m_abiListType, "%d", BULLETED_LIST);
    }
}


/**
 * 
 */
void ODi_Bullet_ListLevelStyle::buildAbiPropsString() {
    
    ODi_ListLevelStyle::buildAbiPropsString();
    
    if (!m_abiProperties.empty()) {
        m_abiProperties += "; ";
    }
    
    m_abiProperties += "list-style:";
    switch (atoi(m_abiListType.utf8_str())) {
        case BULLETED_LIST:
            m_abiProperties += "Bullet List; field-font:Symbol";
            break;
            
        case DASHED_LIST:
            m_abiProperties += "Dashed List; field-font:Symbol";
            break;
            
        case SQUARE_LIST:
            m_abiProperties += "Square List; field-font:Dingbats";
            break;
            
        case TRIANGLE_LIST:
            m_abiProperties += "Triangle List; field-font:Dingbats";
            break;
            
        case DIAMOND_LIST:
            m_abiProperties += "Diamond List; field-font:Dingbats";
            break;
            
        case STAR_LIST:
            m_abiProperties += "Star List; field-font:Dingbats";
            break;
            
        case IMPLIES_LIST:
            m_abiProperties += "Implies List; field-font:Symbol";
            break;
            
        case TICK_LIST:
            m_abiProperties += "Tick List; field-font:Dingbats";
            break;
            
        case BOX_LIST:
            m_abiProperties += "Box List; field-font:Dingbats";
            break;
            
        case HAND_LIST:
            m_abiProperties += "Hand List; field-font:Dingbats";
            break;
            
        case HEART_LIST:
            m_abiProperties += "Heart List; field-font:Dingbats";
            break;
            
        default:
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
    }
}


/******************************************************************************/


/**
 * Constructor
 */
ODi_Numbered_ListLevelStyle::ODi_Numbered_ListLevelStyle(ODi_ElementStack& rElementStack)
    : ODi_ListLevelStyle("Numbered_ListLevelStyle", rElementStack) {
        
    // It seens that OpenDocument aways uses a dot "." as level delimiter.
    m_abiListListDecimal = ".";
}



/**
 * 
 */
void ODi_Numbered_ListLevelStyle::startElement (const gchar* pName,
                                               const gchar** ppAtts,
                                               ODi_ListenerStateAction& rAction) {

    const gchar* pVal;

    // Let the parent class do the processing common to all list types.
    ODi_ListLevelStyle::startElement (pName, ppAtts, rAction);
                                                
    if (!strcmp("text:list-level-style-number", pName) || 
	!strcmp("text:outline-level-style", pName)) {
        UT_UTF8String prefix, suffix;
        UT_DEBUGMSG(("Doing a numbered list type %s \n",pName));
        pVal = UT_getAttribute ("style:num-format", ppAtts);
        UT_ASSERT_HARMLESS(pVal);
        _setAbiListType(pVal);

        if (pVal && !strcmp(pVal, "")) {
            // We have an empty number format.
            
            // Empty list label or "invisible" list.
            m_abiListListDelim = "";
            
        }
	else {
            // We have a number format defined.
            
            pVal = UT_getAttribute ("style:num-prefix", ppAtts);
            if(pVal) {
                prefix = pVal;
            }
            
            pVal = UT_getAttribute ("style:num-suffix", ppAtts);
            if(pVal) {
                suffix = pVal;
            }
            
            m_abiListListDelim  = prefix;
            m_abiListListDelim += "%L";
            m_abiListListDelim += suffix;
        }
        
        pVal = UT_getAttribute ("text:start-value", ppAtts);
        if(pVal) {
            m_abiListStartValue = pVal;
        } else {
            // AbiWord's default value is 0, but on OpenDocument it's 1.
            m_abiListStartValue = "1";
        }
    }
}


/**
 * 
 */
void ODi_Numbered_ListLevelStyle::buildAbiPropsString() {
    
    ODi_ListLevelStyle::buildAbiPropsString();
    
    if (!m_abiProperties.empty()) {
        m_abiProperties += "; ";
    }
    
    m_abiProperties += "field-font: ";
    if (m_pTextStyle) {
        m_abiProperties += *(m_pTextStyle->getFontName());
    } else {
        m_abiProperties += "NULL";
    }
    
    m_abiProperties += "; list-style:";
    switch (atoi(m_abiListType.utf8_str())) {
        case NUMBERED_LIST:
            m_abiProperties += "Numbered List";
            break;
            
        case LOWERCASE_LIST:
            m_abiProperties += "Lower Case List";
            break;
            
        case UPPERCASE_LIST:
            m_abiProperties += "Upper Case List";
            break;
            
        case LOWERROMAN_LIST:
            m_abiProperties += "Lower Roman List";
            break;
            
        case UPPERROMAN_LIST:
            m_abiProperties += "Upper Roman List";
            break;

        case ARABICNUMBERED_LIST:
            m_abiProperties += "Arabic List";
            break;
            
        default:
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
    }
}


/**
 * Maps the value of the OpenDocument attribute style:num-format to the
 * correspondent AbiWord "type" attribute of the list (<l>) element tag.
 * 
 * @param pStyleNumFormat The value of the style:num-format attribute.
 */
void ODi_Numbered_ListLevelStyle::_setAbiListType(const gchar* pStyleNumFormat) {

    if (!pStyleNumFormat) {
        // Use an arbitrary list type.
        UT_UTF8String_sprintf(m_abiListType, "%d", NUMBERED_LIST);
        
    } else if (!strcmp(pStyleNumFormat, "1")) {
        UT_UTF8String_sprintf(m_abiListType, "%d", NUMBERED_LIST);
        
    } else if (!strcmp(pStyleNumFormat, "a")) {
        UT_UTF8String_sprintf(m_abiListType, "%d", LOWERCASE_LIST);
        
    } else if (!strcmp(pStyleNumFormat, "A")) {
        UT_UTF8String_sprintf(m_abiListType, "%d", UPPERCASE_LIST);
        
    } else if (!strcmp(pStyleNumFormat, "i")) {
        UT_UTF8String_sprintf(m_abiListType, "%d", LOWERROMAN_LIST);
        
    } else if (!strcmp(pStyleNumFormat, "I")) {
        UT_UTF8String_sprintf(m_abiListType, "%d", UPPERROMAN_LIST);
        
    } else if (!strcmp(pStyleNumFormat, ODI_LISTLEVELSTYLE_ARABIC)) {
        UT_UTF8String_sprintf(m_abiListType, "%d", ARABICNUMBERED_LIST);
        
    } else {
        // Use an arbitrary list type.
        UT_UTF8String_sprintf(m_abiListType, "%d", NUMBERED_LIST);
    }
}
