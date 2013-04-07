/* AbiWord
 * Copyright (C) 2011 Ben Martin
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

#include "pl_ListenerCoupleCloser.h"
#include "pp_AttrProp.h"
#include "pf_Frag_Strux.h"
#include "px_CR_FmtMark.h"
#include "px_CR_FmtMarkChange.h"
#include "px_CR_Object.h"
#include "px_CR_ObjectChange.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"
#include "px_CR_StruxChange.h"

#include <pd_DocumentRDF.h>


class PD_Bookmark
{
    const PP_AttrProp* m_pAP;
    bool m_isEnd;
    std::string m_id;
public:
    PD_Bookmark( PD_Document* pDoc, PT_AttrPropIndex api );
    bool isEnd();
    std::string getID();
};

PD_Bookmark::PD_Bookmark( PD_Document* pDoc, PT_AttrPropIndex api )
    : m_pAP( 0 )
    , m_isEnd( true )
{
    pDoc->getAttrProp(api,&m_pAP);
    
    const gchar* pValue = NULL;
    if(m_pAP
       && m_pAP->getAttribute("type",pValue)
       && pValue
       && (strcmp(pValue, "start") == 0))
    {
        m_isEnd = false;
    }

    if(m_pAP->getAttribute("name",pValue) && pValue)
    {
        m_id = pValue;
    }
}

bool PD_Bookmark::isEnd()
{
    return m_isEnd;
}

std::string PD_Bookmark::getID()
{
    return m_id;
}

bool
PL_ListenerCoupleCloser::shouldClose( const std::string& id,
                                      bool /*isEnd*/,
                                      stringlist_t& sl )
{
    stringlist_t::iterator iter = find( sl.begin(), sl.end(), id );
    if( iter != sl.end() )
    {
        sl.erase( iter );
        return true;
    }
    return false;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/


PL_ListenerCoupleCloser::PL_ListenerCoupleCloser()
    : m_pDocument(0)
    , m_delegate(0)
    , m_AfterContentListener(this)
    , m_BeforeContentListener(this)
    , m_NullContentListener(this)
{
}

PL_ListenerCoupleCloser::~PL_ListenerCoupleCloser()
{
}

PD_Document*
PL_ListenerCoupleCloser::getDocument(void)
{
    return m_pDocument;
}

void
PL_ListenerCoupleCloser::setDocument(PD_Document * pDoc)
{
    m_pDocument = pDoc;
}


void
PL_ListenerCoupleCloser::setDelegate( PL_Listener* delegate )
{
    m_delegate = delegate;
}

void
PL_ListenerCoupleCloser::reset()
{
    m_rdfUnclosedAnchorStack.clear();
    m_rdfUnopenedAnchorStack.clear();
    m_bookmarkUnclosedStack.clear();
    m_bookmarkUnopenedStack.clear();
}



/****************************************/
/****************************************/
/****************************************/

bool
PL_ListenerCoupleCloser::populateStrux( pf_Frag_Strux* /*sdh*/,
                                        const PX_ChangeRecord * /*pcr*/,
                                        fl_ContainerLayout* * /* psfh */ )
{
    return true;
}

void
PL_ListenerCoupleCloser::trackOpenClose( const std::string& id,
                                         bool isEnd,
                                         stringlist_t& unclosed,
                                         stringlist_t& unopened )
{
    if( isEnd )
    {
        stringlist_t::iterator iter = find( unclosed.begin(),
                                            unclosed.end(),
                                            id );
        if( iter == unclosed.end() )
        {
            // closing an object which was not opened in range.
            unopened.push_back( id );
        }
        else
        {
            unclosed.erase( iter );
        }
    }
    else
    {
        unclosed.push_back( id );
    }
}


bool
PL_ListenerCoupleCloser::populate(fl_ContainerLayout* /* sfh */,
                                  const PX_ChangeRecord * pcr)
{
	UT_DebugOnly<PT_AttrPropIndex> indexAP = pcr->getIndexAP();
	UT_DEBUGMSG(("MIQ: PL_ListenerCoupleCloser::Populate() indexAP %d pcr.type:%d \n",
		     (PT_AttrPropIndex)indexAP, pcr->getType() ));
	switch (pcr->getType())
	{
        case PX_ChangeRecord::PXT_InsertSpan:
        {
            return true;
        }
        case PX_ChangeRecord::PXT_InsertObject:
        {
            const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *>(pcr);
			PT_AttrPropIndex api = pcr->getIndexAP();
			switch (pcro->getObjectType())
            {
                case PTO_Bookmark:
                {
                    PD_Bookmark a( getDocument(), api );
                    trackOpenClose( a.getID(), a.isEnd(),
                                    m_bookmarkUnclosedStack,
                                    m_bookmarkUnopenedStack );
                    break;
                }
                
                case PTO_RDFAnchor:
                {
                    RDFAnchor a( getDocument(), api );
                    trackOpenClose( a.getID(), a.isEnd(),
                                    m_rdfUnclosedAnchorStack,
                                    m_rdfUnopenedAnchorStack );
                    
                    // std::string xmlid = a.getID();
                    
                    // if( a.isEnd() )
                    // {
                    //     stringlist_t::iterator iter = find( m_rdfUnclosedAnchorStack.begin(),
                    //                                         m_rdfUnclosedAnchorStack.end(),
                    //                                         xmlid );
                    //     if( iter == m_rdfUnclosedAnchorStack.end() )
                    //     {
                    //         // closing an rdf anchor which was not opened in range.
                    //         m_rdfUnopenedAnchorStack.push_back( xmlid );
                    //     }
                    //     else
                    //     {
                    //         m_rdfUnclosedAnchorStack.erase( iter );
                    //     }
                    // }
                    // else
                    // {
                    //     m_rdfUnclosedAnchorStack.push_back( xmlid );
                    // }
                    
                    break;
                }
                default:
                    break;
            }
            
            return true;
        }
        default:
            return true;
    }
	return true;
}


/****************************************/
/****************************************/
/****************************************/
/****************************************/
/****************************************/
/****************************************/

bool
PL_ListenerCoupleCloser::populateStruxAfter( pf_Frag_Strux* /*sdh*/,
                                             const PX_ChangeRecord * pcr,
                                             fl_ContainerLayout* * /* psfh */ )
{
	UT_DebugOnly<PT_AttrPropIndex> indexAP = pcr->getIndexAP();
	UT_DEBUGMSG(("MIQ: PL_ListenerCoupleCloser::PopulateStruxAfter() indexAP %d pcr.type:%d \n",
		     (PT_AttrPropIndex)indexAP, pcr->getType() ));
    return true;
}




bool
PL_ListenerCoupleCloser::populateAfter( fl_ContainerLayout* sfh,
                                        const PX_ChangeRecord * pcr )
{
	UT_DebugOnly<PT_AttrPropIndex> indexAP = pcr->getIndexAP();
	UT_DEBUGMSG(("MIQ: PL_ListenerCoupleCloser::PopulateAfter() indexAP %d pcr.type:%d \n",
		     (PT_AttrPropIndex)indexAP, pcr->getType() ));
	switch (pcr->getType())
	{
        case PX_ChangeRecord::PXT_InsertSpan:
        {
            return true;
        }
        case PX_ChangeRecord::PXT_InsertObject:
        {
            const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *>(pcr);
			PT_AttrPropIndex api = pcr->getIndexAP();
			switch (pcro->getObjectType())
            {
                case PTO_Bookmark:
                    if( !m_bookmarkUnclosedStack.empty() )
                    {
                        PD_Bookmark a( getDocument(), api );
                        if( shouldClose( a.getID(), a.isEnd(), m_bookmarkUnclosedStack ) )
                        {
                            return m_delegate->populate( sfh, pcr );
                        }
                        break;
                    }
                case PTO_RDFAnchor:
                    if( !m_rdfUnclosedAnchorStack.empty() )
                    {
                        RDFAnchor a( getDocument(), api );
                        UT_DEBUGMSG(("MIQ: PL_ListenerCoupleCloser::PopulateAfter() rdfid:%s \n",
                                     a.getID().c_str() ));
                        if( shouldClose( a.getID(), a.isEnd(), m_rdfUnclosedAnchorStack ) )
                        {
                            return m_delegate->populate( sfh, pcr );
                        }
                        break;
                    }
                default:
                    break;
            }
            
            return true;
        }
        default:
            return true;
    }
    return true;
}

bool PL_ListenerCoupleCloser::AfterContentListener::populate( fl_ContainerLayout* sfh,
                                                              const PX_ChangeRecord * pcr )
{
    return m_self->populateAfter( sfh, pcr );
}

bool
PL_ListenerCoupleCloser::AfterContentListener::populateStrux( pf_Frag_Strux* sdh,
                                                              const PX_ChangeRecord * pcr,
                                                              fl_ContainerLayout* * psfh )
{
    return m_self->populateStruxAfter( sdh, pcr, psfh );
}

bool PL_ListenerCoupleCloser::AfterContentListener::isFinished()
{
    return m_self->m_rdfUnclosedAnchorStack.empty()
        && m_self->m_bookmarkUnclosedStack.empty();
}

/****************************************/
/****************************************/
/****************************************/

bool
PL_ListenerCoupleCloser::populateStruxBefore( pf_Frag_Strux* /*sdh*/,
                                             const PX_ChangeRecord * pcr,
                                             fl_ContainerLayout* * /* psfh */ )
{
	UT_DebugOnly<PT_AttrPropIndex> indexAP = pcr->getIndexAP();
	UT_DEBUGMSG(("MIQ: PL_ListenerCoupleCloser::PopulateStruxBefore() indexAP %d pcr.type:%d \n",
		     (PT_AttrPropIndex)indexAP, pcr->getType() ));

    
    return true;
}

bool
PL_ListenerCoupleCloser::shouldOpen( const std::string& id,
                                     bool /*isEnd*/,
                                     stringlist_t& sl )
{
    stringlist_t::iterator iter = find( sl.begin(), sl.end(), id );
    if( iter != sl.end() )
    {
        sl.erase( iter );
        return true;
    }
    return false;
}

bool
PL_ListenerCoupleCloser::populateBefore( fl_ContainerLayout* sfh,
                                        const PX_ChangeRecord * pcr )
{
	UT_DebugOnly<PT_AttrPropIndex> indexAP = pcr->getIndexAP();
	UT_DEBUGMSG(("MIQ: PL_ListenerCoupleCloser::PopulateBefore() indexAP %d pcr.type:%d \n",
		     (PT_AttrPropIndex)indexAP, pcr->getType() ));
	switch (pcr->getType())
	{
        case PX_ChangeRecord::PXT_InsertSpan:
        {
            return true;
        }
        case PX_ChangeRecord::PXT_InsertObject:
        {
            const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *>(pcr);
			PT_AttrPropIndex api = pcr->getIndexAP();
			switch (pcro->getObjectType())
            {
                case PTO_Bookmark:
                    if( !m_bookmarkUnopenedStack.empty() )
                    {
                        PD_Bookmark a( getDocument(), api );
                        if( shouldOpen( a.getID(), a.isEnd(), m_bookmarkUnopenedStack ))
                            return m_delegate->populate( sfh, pcr );
                        break;
                    }
                case PTO_RDFAnchor:
                    if( !m_rdfUnopenedAnchorStack.empty() )
                    {
                        RDFAnchor a( getDocument(), api );
                        UT_DEBUGMSG(("MIQ: PL_ListenerCoupleCloser::PopulateBefore() rdfid:%s \n",
                                     a.getID().c_str() ));
                        if( shouldOpen( a.getID(), a.isEnd(), m_rdfUnopenedAnchorStack ))
                            return m_delegate->populate( sfh, pcr );
                        
                        // stringlist_t::iterator iter = find( m_rdfUnopenedAnchorStack.begin(),
                        //                                     m_rdfUnopenedAnchorStack.end(),
                        //                                     a.getID() );
                        // if( iter != m_rdfUnopenedAnchorStack.end() )
                        // {
                        //     m_rdfUnopenedAnchorStack.erase( iter );
                        //     return m_delegate->populate( sfh, pcr );
                        // }
                        break;
                    }
                default:
                    break;
            }
            
            return true;
        }
        default:
            return true;
    }
    return true;
}


bool PL_ListenerCoupleCloser::BeforeContentListener::populate( fl_ContainerLayout* sfh,
                                                              const PX_ChangeRecord * pcr )
{
    return m_self->populateBefore( sfh, pcr );
}

bool
PL_ListenerCoupleCloser::BeforeContentListener::populateStrux( pf_Frag_Strux* sdh,
                                                              const PX_ChangeRecord * pcr,
                                                              fl_ContainerLayout* * psfh )
{
    return m_self->populateStruxBefore( sdh, pcr, psfh );
}

bool PL_ListenerCoupleCloser::BeforeContentListener::isFinished()
{
    return m_self->m_rdfUnopenedAnchorStack.empty()
        && m_self->m_bookmarkUnopenedStack.empty();
}


/****************************************/
/****************************************/
/****************************************/


PL_FinishingListener*
PL_ListenerCoupleCloser::getAfterContentListener()
{
    return &m_AfterContentListener;
}

PL_FinishingListener*
PL_ListenerCoupleCloser::getBeforeContentListener()
{
    return &m_BeforeContentListener;
}


PL_FinishingListener*
PL_ListenerCoupleCloser::getNullContentListener()
{
    return &m_NullContentListener;
}

