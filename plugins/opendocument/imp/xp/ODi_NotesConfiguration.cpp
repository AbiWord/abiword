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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */
 
// Class definition include.
#include "ODi_NotesConfiguration.h"

// Internal includes
#include "ODi_ListenerStateAction.h"

// AbiWord includes
#include <ut_misc.h>


/**
 * 
 */
void ODi_NotesConfiguration::startElement(const gchar* pName,
										  const gchar** ppAtts,
										  ODi_ListenerStateAction& /*rAction*/) 
{
                                                
    const gchar* pVal;
                                        
    if (!strcmp("text:notes-configuration", pName)) {
        
        pVal = UT_getAttribute ("text:note-class", ppAtts);
        UT_ASSERT(pVal);
        m_noteClass = pVal;
        
        pVal = UT_getAttribute ("text:citation-style-name", ppAtts);
        if (pVal) {
            m_citationStyleName = pVal;
        }
    }
}


/**
 * 
 */
void ODi_NotesConfiguration::endElement(const gchar* pName,
                                           ODi_ListenerStateAction& rAction) {
                                            
    if (!strcmp("text:notes-configuration", pName)) {
        rAction.popState();
    }
}
