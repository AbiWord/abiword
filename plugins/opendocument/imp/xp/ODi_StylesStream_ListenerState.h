/* AbiSource
 * 
 * Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
 * Copyright (C) 2004 Robert Staudinger <robsta@stereolyzer.net>
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

#ifndef _ODI_STYLESSTREAM_LISTENERSTATE_H_
#define _ODI_STYLESSTREAM_LISTENERSTATE_H_

// Internal includes
#include "ODi_ListenerState.h"
#include "ODi_ElementStack.h"

// External includes
#include <gsf/gsf.h>


// Internal classes
class ODi_Office_Styles;
class ODi_Abi_Data;

// AbiWord classes
class PD_Document;

/**
 * 
 */
class ODi_StylesStream_ListenerState : public ODi_ListenerState {

public:
    
    ODi_StylesStream_ListenerState (PD_Document* pAbiDocument,
                                   GsfInfile* pGsfInfile,
                                   ODi_Office_Styles* pStyles,
                                   ODi_ElementStack& rElementStack,
                                   ODi_Abi_Data& rAbiData);

    virtual ~ODi_StylesStream_ListenerState ();
    
    void startElement (const gchar* pName, const gchar** ppAtts,
                               ODi_ListenerStateAction& rAction);
                               
    void endElement (const gchar* pName, ODi_ListenerStateAction& rAction);
                             
    void charData (const gchar* pBuffer, int length);

private:

    PD_Document* m_pAbiDocument;
    GsfInfile* m_pGsfInfile;
    ODi_Office_Styles* m_pStyles;
    ODi_Abi_Data& m_rAbiData;
    bool          m_bOutlineStyle;
};

#endif //_ODI_STYLESSTREAM_LISTENERSTATE_H_
