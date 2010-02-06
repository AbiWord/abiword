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

#ifndef _ODI_LISTLEVELSTYLE_H_
#define _ODI_LISTLEVELSTYLE_H_

// Internal includes
#include "ODi_ListenerState.h"

// Internal classes
class ODi_Style_Style;

// AbiWord classes
class PD_Document;

/**
 * Abstract class
 */
class ODi_ListLevelStyle : public ODi_ListenerState {
    
public:
    
    ODi_ListLevelStyle(const char* pStateName, ODi_ElementStack& rElementStack);
    virtual ~ODi_ListLevelStyle() {}
    
    virtual void startElement (const gchar* pName, const gchar** ppAtts,
                               ODi_ListenerStateAction& rAction);
                               
    virtual void endElement (const gchar* pName,
                             ODi_ListenerStateAction& rAction);
                             
    virtual void charData (const gchar* /*pBuffer*/, int /*length*/) {}
    
    UT_uint32 getLevelNumber() const {return m_levelNumber;}
    
    void setAbiListID(UT_uint32 abiListID);
    const UT_UTF8String* getAbiListID() const {return &m_abiListID;}
    
    void setAbiListParentID(const UT_UTF8String& rAbiListParentID) {
        m_abiListParentID = rAbiListParentID;
    }
    
    bool isVisible(void) const;

    void setAbiListParentID(const gchar* pParentID) {
        m_abiListParentID.assign(pParentID);
    }
    
    const UT_UTF8String* getTextStyleName() const {return &m_textStyleName;}
    void setTextStyle(const ODi_Style_Style* pTextStyle) {m_pTextStyle = pTextStyle;}
    
    const UT_UTF8String* getAbiListParentID() const {return &m_abiListParentID;}

    /**
     * The AbiWord properties of the list depends on some properties already
     * defined by the AbiWord paragraph style.
     * 
     * @param rProps Will have the properties string appended.
     * @param pStyle Pointer to the paragraph style used on this list paragraph.
     */    
    void getAbiProperties(UT_UTF8String& rProps, const ODi_Style_Style* pStyle = NULL) const;
    
    void defineAbiList(PD_Document* pDocument);
    
    virtual void buildAbiPropsString();
    
    const UT_UTF8String* getMinLabelDistance() const {return &m_minLabelDistance;}
    
protected:

    UT_UTF8String m_level;
    UT_uint32 m_levelNumber;
    
    // The AbiWord list (<l> tag) ID.
    UT_UTF8String m_abiListID;
    
    // The AbiWord list (<l> tag) parent id.
    UT_UTF8String m_abiListParentID;
    
    // The AbiWord list (<l> tag) type.
    UT_UTF8String m_abiListType;
    
    // The AbiWord list (<l> tag) start value.
    UT_UTF8String m_abiListStartValue;
    
    // The AbiWord list (<l> tag) list delim.
    // It's a printf like string with the list maker format.
    UT_UTF8String m_abiListListDelim;
    
    // The AbiWord list (<l> tag) list decimal. 
    // It's the level delimiter, usually a ".", "," or "-"
    // Looks like the property name and its real name (the one used on the GUI)
    // don't match.
    UT_UTF8String m_abiListListDecimal;
    
    // The properties of the list, to be used on the "props" attribute of
    // abi paragraphs (<p>) that uses this list level style.
    UT_UTF8String m_abiProperties;


    
    // text:space-before attribute of <style:list-level-properties>
    UT_UTF8String m_spaceBefore;

    // text:min-label-width attribute of <style:list-level-properties>
    UT_UTF8String m_minLabelWidth;
    
    // text:min-label-distance attribute of <style:list-level-properties>
    // The minumum distance between the list label and the list text.
    // Can't be translated to AbiWord easily.
    UT_UTF8String m_minLabelDistance;
    
    // fo:text-indent attribute of <style:list-level-properties>
	UT_UTF8String m_textIndent;

	// fo:margin-left attribute of <style:list-level-properties>
	UT_UTF8String m_marginLeft;
    
    // text:style-name attribute of <text:list-level-style-*>
    // Maps, indirectly, to the AbiWord "field-font" property.
    UT_UTF8String m_textStyleName;
    const ODi_Style_Style* m_pTextStyle;
};


/**
 * Represents a <text:list-level-style-bullet> element.
 */
class ODi_Bullet_ListLevelStyle : public ODi_ListLevelStyle {
    
public:

    ODi_Bullet_ListLevelStyle(ODi_ElementStack& m_rElementStack);
    
    void startElement (const gchar* pName, const gchar** ppAtts,
                               ODi_ListenerStateAction& rAction);
                               
    /*void endElement (const gchar* pName,
                             ODi_ListenerStateAction& rAction);
                             
    void charData (const gchar* pBuffer, int length);*/
    
    void buildAbiPropsString();
};


/**
 * Represents a <text:list-level-style-number> element.
 */
class ODi_Numbered_ListLevelStyle : public ODi_ListLevelStyle {
    
public:

    ODi_Numbered_ListLevelStyle(ODi_ElementStack& m_rElementStack);
    
    void startElement (const gchar* pName, const gchar** ppAtts,
                               ODi_ListenerStateAction& rAction);
                               
    /*void endElement (const gchar* pName,
                             ODi_ListenerStateAction& rAction);
                             
    void charData (const gchar* pBuffer, int length);*/
    
    void buildAbiPropsString();
    
private:

    /**
     * Maps the value of the OpenDocument attribute style:num-format to the
     * correspondent AbiWord "type" attribute of the list (<l>) element tag.
     * 
     * @param pStyleNumFormat The value of the style:num-format attribute.
     */
    void _setAbiListType(const gchar* pStyleNumFormat);
};

#endif //_ODI_LISTLEVELSTYLE_H_
