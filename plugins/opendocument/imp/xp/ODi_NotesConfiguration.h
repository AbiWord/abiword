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

#ifndef _ODI_NOTESCONFIGURATION_H_
#define _ODI_NOTESCONFIGURATION_H_

// Internal includes
#include "ODi_ListenerState.h"


/**
 * Represents a <text:notes-configuration> element.
 */
class ODi_NotesConfiguration : public ODi_ListenerState {
public:

    ODi_NotesConfiguration(ODi_ElementStack& rElementStack) :
        ODi_ListenerState("NotesConfiguration", rElementStack) {}
        
    virtual ~ODi_NotesConfiguration() {}
    
    void startElement(const gchar* pName, const gchar** ppAtts,
                      ODi_ListenerStateAction& rAction);
                      
    void endElement(const gchar* pName, ODi_ListenerStateAction& rAction);

    void charData (const gchar* /*pBuffer*/, int /*length*/) {}
    
    const UT_UTF8String* getCitationStyleName() const {return &m_citationStyleName;}

private:

    UT_UTF8String m_noteClass; // text:note-class
    UT_UTF8String m_citationStyleName; // text:citation-style-name
};

#endif //_ODI_NOTESCONFIGURATION_H_
