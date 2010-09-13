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
 
// Class definition include
#include "ODe_ChangeTrackingParagraph_Listener.h"

// Internal includes
#include "ODe_AuxiliaryData.h"
#include "ODe_Styles.h"

// AbiWord includes
#include <pp_AttrProp.h>
#include <fl_TOCLayout.h>
#include <pp_Revision.h>


/**
 * Constructor
 */
ODe_ChangeTrackingParagraph_Listener::ODe_ChangeTrackingParagraph_Listener(
                                    ODe_Styles& rStyles,
                                    ODe_AuxiliaryData& rAuxiliaryData )
                                    : m_rStyles(rStyles)
                                    , m_rAuxiliaryData(rAuxiliaryData)
                                    , m_current(0)
{
}


void
ODe_ChangeTrackingParagraph_Listener::openBlock( const PP_AttrProp* pAP,
                                                 ODe_ListenerAction& /* rAction */ )
{
    UT_DEBUGMSG(("ODe_CTPara_Listener::openBlock() pos:%d AP:%p\n",getCurrentDocumentPosition(),pAP));
    m_current = m_rAuxiliaryData.ensureChangeTrackingParagraphData( getCurrentDocumentPosition() );

    const gchar* pValue;
    if( pAP->getAttribute("revision", pValue))
    {
        PP_RevisionAttr ra( pValue );
        if( const PP_Revision* last = ra.getLastRevision() )
            UT_DEBUGMSG(("ODe_CTPara_Listener::openBlock() last-revision-number:%d\n", last->getId() ));

        m_current->getData().update( &ra );
    }
}

void
ODe_ChangeTrackingParagraph_Listener::closeBlock()
{
    UT_DEBUGMSG(("ODe_CTPara_Listener::closeBlock() pos:%d\n",getCurrentDocumentPosition()));
    m_current->setEndPosition(getCurrentDocumentPosition());
    m_current = 0;
}

void
ODe_ChangeTrackingParagraph_Listener::openSpan( const PP_AttrProp* pAP )
{
    UT_DEBUGMSG(("ODe_CTPara_Listener::openSpan() pos:%d AP:%p\n",getCurrentDocumentPosition(),pAP));

    if( !m_current )
        return;
    
    const gchar* pValue;
    if( pAP->getAttribute("revision", pValue))
    {
        UT_DEBUGMSG(("ODe_CTPara_Listener::openSpan() revision-raw:%s\n", pValue ));
        PP_RevisionAttr ra( pValue );
        if( const PP_Revision* last = ra.getLastRevision() )
            UT_DEBUGMSG(("ODe_CTPara_Listener::openSpan() last-revision-number:%d\n", last->getId() ));

        m_current->getData().update( &ra );
    }
    
    
}

void ODe_ChangeTrackingParagraph_Listener::closeSpan()
{
    UT_DEBUGMSG(("ODe_CTPara_Listener::closeSpan() pos:%d\n",getCurrentDocumentPosition()));

    if( !m_current )
        return;
}

void ODe_ChangeTrackingParagraph_Listener::insertText( const UT_UTF8String& rText )
{
    UT_DEBUGMSG(("ODe_CTPara_Listener::insertText() %s\n",rText.utf8_str()));
}

