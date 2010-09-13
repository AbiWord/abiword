/* AbiSource
 * 
 * Author: Ben Martin <monkeyiq@abisource.com>
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

#ifndef ODE_CHANGETRACKINGPARAGRAPH_LISTENER_H_
#define ODE_CHANGETRACKINGPARAGRAPH_LISTENER_H_

// Internal includes
#include "ODe_AbiDocListenerImpl.h"
#include "ODe_AuxiliaryData.h"

// AbiWord classes
class PP_AttrProp;
class ODe_Styles;


/**
 * Searches all TOCs for its heading styles, i.e.: the paragraph styles that are
 * used to build the document structure (chapters, sections, etc).
 */
class ODe_ChangeTrackingParagraph_Listener: public ODe_AbiDocListenerImpl {
public:
    ODe_ChangeTrackingParagraph_Listener(ODe_Styles& m_rStyles, ODe_AuxiliaryData& rAuxiliaryData);
    
    void openBlock(const PP_AttrProp* pAP, ODe_ListenerAction& rAction);
    void closeBlock();
    void openSpan(const PP_AttrProp* pAP);
    void closeSpan();
    virtual void insertText(const UT_UTF8String& rText);


private:
    ODe_Styles& m_rStyles;
    ODe_AuxiliaryData& m_rAuxiliaryData;
    pChangeTrackingParagraphData_t m_current;
};

#endif /*ODE_CHANGETRACKINGPARAGRAPH_LISTENER_H_*/
