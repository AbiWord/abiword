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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#ifndef ODE_TOC_LISTENER_H_
#define ODE_TOC_LISTENER_H_

#include <map>

// Internal includes
#include "ODe_AbiDocListenerImpl.h"

// Internal classes
class ODe_AuxiliaryData;

// AbiWord classes
class PP_AttrProp;

/**
 * Builds up the TOC body, as OpenOffice.org requires the <text:index-body> tag to 
 * be present and filled when initially opening the document. Without it, it will show 
 * an empty TOC until the user regenerates it, which is not that pretty.
 *
 * Unfortunately, we can't generate an 100% accurate preview. We can't for example
 * output page numbers, as they only exist in AbiWord's layout classes and not in its 
 * PieceTable. Exporting the header outline is the best we can do as long as we don't
 * store all generated TOC data in the PieceTable.
 */
class ODe_TOC_Listener: public ODe_AbiDocListenerImpl {
public:
    ODe_TOC_Listener(ODe_AuxiliaryData& rAuxiliaryData);

    virtual void insertText(const UT_UTF8String& rText);

    virtual void insertTabChar();

    virtual void openBlock(const PP_AttrProp* pAP, ODe_ListenerAction& rAction);
    virtual void closeBlock();

private:
    bool m_bInTOCBlock;
    ODe_AuxiliaryData& m_rAuxiliaryData;
};

#endif /*ODE_TOC_LISTENER_H_*/
