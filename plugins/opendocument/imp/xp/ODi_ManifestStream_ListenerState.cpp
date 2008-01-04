/* AbiSource
 * 
 * Copyright (C) 2006 INdT
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
#include "ODi_ManifestStream_ListenerState.h"

// Internal includes
#include "ODi_ListenerStateAction.h"

// AbiWord includes
#include "pd_Document.h"


/**
 * Constructor
 */
ODi_ManifestStream_ListenerState::ODi_ManifestStream_ListenerState(
                                                PD_Document* pDocument,
                                                ODi_ElementStack& rElementStack)
        : ODi_ListenerState("ManifestStream", rElementStack),
          m_pDocument(pDocument),
          m_isDocumentEncripted(false)
{
}


/**
 * Called to signal that the start tag of an element has been reached.
 */
void ODi_ManifestStream_ListenerState::startElement (const gchar* pName,
                                                const gchar** ppAtts,
                                                ODi_ListenerStateAction& rAction) 
{
    if (!strcmp(pName, "manifest:encryption-data")) {
        m_isDocumentEncripted = true;
    }
}


/**
 * Called to signal that the end tag of an element has been reached.
 */
void ODi_ManifestStream_ListenerState::endElement (const gchar* pName,
                                              ODi_ListenerStateAction& rAction)
{
    if (!strcmp(pName, "manifest:manifest")) {
        rAction.popState();
    }
}
