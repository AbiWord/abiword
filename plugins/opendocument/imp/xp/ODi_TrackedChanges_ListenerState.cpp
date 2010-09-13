/* AbiSource
 * 
 * Worked on 2010 Ben Martin <monkeyiq@abisource.com>
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
#include "ODi_TrackedChanges_ListenerState.h"

// Internal includes
#include "ODi_Office_Styles.h"
#include "ODi_Style_List.h"
#include "ODi_Style_Style.h"
#include "ODi_Style_MasterPage.h"
#include "ODi_ListenerStateAction.h"
#include "ODi_ListLevelStyle.h"
#include "ODi_NotesConfiguration.h"
#include "ODi_StartTag.h"
#include "ODi_ElementStack.h"
#include "ODi_TableOfContent_ListenerState.h"
#include "ODi_Abi_Data.h"
#include "ut_growbuf.h"
#include "pf_Frag.h"
#include "ie_exp_RTF.h"
#include "ut_units.h"

// AbiWord includes
#include <ut_misc.h>
#include <pd_Document.h>
#include <pf_Frag_Strux.h>
#include "xad_Document.h"
#include "pd_Document.h"
#include "../../exp/xp/ODe_RevisionsTable.h"

#include <sstream>

static std::string UT_getAttributeString( const char* name, const gchar** pAttrs, const char* defaultValue = "" )
{
    const gchar* p = UT_getAttribute ( name, pAttrs );
    std::string ret = defaultValue;
    if( p )
        ret = p;
    return ret;
}

template < typename ClassName >
static ClassName toType( const char* s )
{
    UT_uint32 ret = 0;
    std::stringstream ss;
    ss << s;
    ss >> ret;
    return ret;
}
template < typename ClassName >
static ClassName toType( std::string s )
{
    UT_uint32 ret = 0;
    std::stringstream ss;
    ss << s;
    ss >> ret;
    return ret;
}

/**
 * Constructor
 */
ODi_TrackedChanges_ListenerState::ODi_TrackedChanges_ListenerState (
    PD_Document* pDocument,
    ODi_Office_Styles* pStyles,
    ODi_ElementStack& rElementStack,
    ODi_Abi_Data& rAbiData)
    : ODi_ListenerState("TrackedChanges", rElementStack)
    , m_pAbiDocument ( pDocument )
    , m_pStyles(pStyles)
    , m_elementParsingLevel(0)
    , m_bAcceptingText(false)
    , m_ctCurrentRevision(-1)
    , m_ctCurrentTransactionID(-1)
    , m_bPendingTransaction(false)
    , m_bPendingTransactionAuthor(false)
    , m_bPendingTransactionDate(false)
    , m_bPendingTransactionChangeLog(false)
{
    UT_ASSERT_HARMLESS(m_pAbiDocument);
    UT_ASSERT_HARMLESS(m_pStyles);
}


/**
 * Destructor
 */
ODi_TrackedChanges_ListenerState::~ODi_TrackedChanges_ListenerState() 
{
}


/**
 * Called when the XML parser finds a start element tag.
 * 
 * @param pName The name of the element.
 * @param ppAtts The attributes of the parsed start tag.
 */
void ODi_TrackedChanges_ListenerState::startElement ( const gchar* pName,
                                                      const gchar** ppAtts,
                                                      ODi_ListenerStateAction& rAction)
{
    if (!strcmp(pName, "delta:tracked-changes" )) {

        int showRevisions   = toType<int>(UT_getAttributeString("delta:showing-revisions", ppAtts, "1" ));
        int markRevisions   = toType<int>(UT_getAttributeString("delta:mark-revisions",    ppAtts, "1" ));
        int autoRevisioning = toType<int>(UT_getAttributeString("delta:auto-revisioning",  ppAtts, "0" ));
        UT_uint32 currRevision = toType<UT_uint32>(UT_getAttributeString("delta:current-revision",  ppAtts, "-1" ));

        UT_DEBUGMSG(("tc, delta:tracked-changes show-rev:%d mark:%d curr:%d\n",
                     showRevisions, markRevisions, currRevision ));
        
        m_pAbiDocument->setShowRevisions( showRevisions );			
        m_pAbiDocument->setMarkRevisions( markRevisions );
        m_ctCurrentRevision = currRevision;
        if( m_ctCurrentRevision == -1 )
            m_ctCurrentRevision = PD_MAX_REVISION;
        // if( m_ctCurrentRevision == 0 )
        //     m_ctCurrentRevision = PD_MAX_REVISION;
        
    } else if (!strcmp(pName, "delta:change-transaction" )) {

        m_ctCurrentTransactionID = toType<UT_uint32>(UT_getAttributeString("delta:change-id", ppAtts, "-1" ));
        if( m_ctCurrentTransactionID == -1 )
        {
            // error
        }
        
    } else if (!strcmp(pName, "delta:change-info" )) {
        
        m_bPendingTransaction = true;
        m_bAcceptingText = false;
        m_sTransactionChangeLog = "";
        m_sTransactionAuthor = "";
        m_sTransactionDate = "";
        
    } else if (!strcmp(pName, "delta:xvers" )) {
    } else if (!strcmp(pName, "delta:change-log" )) {
        
        m_bPendingTransactionChangeLog = true;
        m_bAcceptingText = false;

    } else if (!strcmp(pName, "dc:creator")) {

        if (m_bPendingTransaction) {
            m_bPendingTransactionAuthor = true;
            m_bAcceptingText = false;
        }
        
    } else if (!strcmp(pName, "dc:date")) {

        if (m_bPendingTransaction) {
            m_bPendingTransactionDate = true;
            m_bAcceptingText = false;
        }
        
    }
    
    m_elementParsingLevel++;
}


/**
 * Called when an "end of element" tag is parsed (like <myElementName/>)
 * 
 * @param pName The name of the element
 */
void ODi_TrackedChanges_ListenerState::endElement (const gchar* pName,
                                                   ODi_ListenerStateAction& rAction)
{
    UT_ASSERT(m_elementParsingLevel >= 0);

    if (!strcmp(pName, "delta:tracked-changes")) {

        m_pAbiDocument->setShowRevisionId(m_ctCurrentRevision);
        rAction.popState();

        // DEBUG
        {
            const UT_GenericVector<AD_Revision*> & vRevisions = m_pAbiDocument->getRevisions();
            for (UT_sint32 k=0; k < vRevisions.getItemCount(); k++)
            {
                UT_String s;
                const AD_Revision * pRev = vRevisions.getNthItem(k);
                UT_continue_if_fail(pRev);

                UT_uint32   changeID = pRev->getId();
                std::string author   = pRev->getAuthor();
                UT_uint64   mtimet   = (UT_uint64)pRev->getStartTime();
                std::string mtimestr = ODe_RevisionsTable::timetToODTChangeTrackingString(mtimet);
                UT_UTF8String descutf8;
                descutf8.appendUCS4( pRev->getDescription() );

                UT_DEBUGMSG(("TrackedChanges_ListenerState::endElement() id:%d author:%s mtimet:%ld mtimestr:%s desc:%s\n",
                             changeID, author.c_str(), mtimet, mtimestr.c_str(), descutf8.utf8_str() ));
            }
    
        }
        
    } else if (!strcmp(pName, "delta:change-log" )) {
        
        m_bPendingTransactionChangeLog = false;
        m_bAcceptingText = true;
        
    } else if (!strcmp(pName, "delta:change-info" )) {

        time_t m_currentRevisionTime = 0;
        UT_uint32 m_currentRevisionVersion = 0;
        bool bGenCR = true;

        if( !m_sTransactionDate.empty() )
        {
            struct tm tm;
            strptime( m_sTransactionDate.c_str(), "%Y-%m-%dT%H:%M:%S", &tm);
            m_currentRevisionTime = timegm( &tm );
        }
        
        UT_DEBUGMSG(("tc, change-info found, id:%d tt:%d author:%s desc:%s\n",
                     m_ctCurrentTransactionID, m_currentRevisionTime,
                     m_sTransactionAuthor.c_str(),
                     m_sTransactionChangeLog.c_str()
                        ));
        m_pAbiDocument->addRevision( m_ctCurrentTransactionID,
                                     m_sTransactionChangeLog,
                                     m_currentRevisionTime,
                                     m_currentRevisionVersion,
                                     m_sTransactionAuthor,
                                     bGenCR );

    } else if (!strcmp(pName, "dc:creator")) {

        if (m_bPendingTransactionAuthor) {
            m_bPendingTransactionAuthor = false;
            m_bAcceptingText = true;
        }
        
    } else if (!strcmp(pName, "dc:date")) {

        if (m_bPendingTransactionDate) {
            m_bPendingTransactionDate = false;
            m_bAcceptingText = true;
        }
    }
    
    m_elementParsingLevel--;
}


/**
 * 
 */
void
ODi_TrackedChanges_ListenerState::charData ( const gchar* pBuffer, int length )
{
    UT_DEBUGMSG(("ODi_TrackedChanges_ListenerState::charData() ta:%d td:%d tcl:%d buf:%s\n",
                 m_bPendingTransactionAuthor,
                 m_bPendingTransactionDate,
                 m_bPendingTransactionChangeLog,
                 UT_UCS4String (pBuffer, length, true).utf8_str()
                    ));
    
    if (pBuffer && length) 
    {
        if (m_bAcceptingText) 
        {
            m_charData += UT_UCS4String (pBuffer, length, true);
        }
        else if (m_bPendingTransactionAuthor) 
        {
            UT_UCS4String t(pBuffer, length, true);
            m_sTransactionAuthor = t.utf8_str();
            UT_DEBUGMSG(("ODi_TrackedChanges_ListenerState::charData() have-author:%s\n",
                         m_sTransactionAuthor.c_str() ));
        } 
        else if (m_bPendingTransactionDate) 
        {
            UT_UCS4String t(pBuffer, length, true);
            m_sTransactionDate = t.utf8_str();
        }
        else if (m_bPendingTransactionChangeLog) 
        {
            UT_UCS4String t(pBuffer, length, true);
            m_sTransactionChangeLog = t.utf8_str();
            UT_DEBUGMSG(("ODi_TrackedChanges_ListenerState::charData() have-desc:%s\n",
                         m_sTransactionChangeLog.c_str() ));
        }
    }
    
}




/**
 * 
 */
void ODi_TrackedChanges_ListenerState::_flush ()
{
}



    

