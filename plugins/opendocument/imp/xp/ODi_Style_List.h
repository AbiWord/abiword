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

#ifndef _ODI_STYLE_LIST_H_
#define _ODI_STYLE_LIST_H_

// Internal includes
#include "ODi_ListenerState.h"

// AbiWord includes
#include <ut_string_class.h>
#include <ut_vector.h>

// Internal classes
class ODi_ListLevelStyle;

// AbiWord classes
class PD_Document;


/**
 * Represents a <text:list-style> element.
 */
class ODi_Style_List : public ODi_ListenerState {
    
public:

    ODi_Style_List(ODi_ElementStack& rElementStack) :
		ODi_ListenerState("StyleList", rElementStack) {}
                                     
    virtual ~ODi_Style_List();
    
    void startElement (const gchar* pName, const gchar** ppAtts,
                               ODi_ListenerStateAction& rAction);
                               
    void endElement (const gchar* pName,
                             ODi_ListenerStateAction& rAction);
                             
    void charData (const gchar* pBuffer, int length);
    
    void defineAbiList(PD_Document* pDocument);

    ODi_ListLevelStyle* getLevelStyle(UT_uint32 level) {
        // Levels starts from 1, but our vector starts from 0 (zero).
        return m_levelStyles[level-1];
    }
    
    UT_uint32 getLevelCount() const {return m_levelStyles.getItemCount();}
    
    void buildAbiPropertiesString();

private:
    // style:name attribute
    UT_UTF8String m_name;
    
    // style:display-name attribute
    UT_UTF8String m_displayName;
    
    // text:consecutive-numbering attribute
    bool m_bConsecutiveNumbering;
    
    UT_GenericVector<ODi_ListLevelStyle*> m_levelStyles;
};

#endif //_ODI_STYLE_LIST_H_
