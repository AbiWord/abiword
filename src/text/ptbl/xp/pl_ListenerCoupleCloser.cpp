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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
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
}



/****************************************/
/****************************************/
/****************************************/

bool
PL_ListenerCoupleCloser::populateStrux( PL_StruxDocHandle /*sdh*/,
                                        const PX_ChangeRecord * pcr,
                                        PL_StruxFmtHandle * /* psfh */ )
{
    return true;
}

bool
PL_ListenerCoupleCloser::populate(PL_StruxFmtHandle /* sfh */,
                                  const PX_ChangeRecord * pcr)
{
	PT_AttrPropIndex indexAP = pcr->getIndexAP();
	UT_DEBUGMSG(("MIQ: PL_ListenerCoupleCloser::Populate() indexAP %d pcr.type:%d \n",
                 indexAP, pcr->getType() ));
	switch (pcr->getType())
	{
        case PX_ChangeRecord::PXT_InsertSpan:
        {
            const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *>(pcr);
            UT_uint32 len = pcrs->getLength();
            return true;
        }
        case PX_ChangeRecord::PXT_InsertObject:
        {
            const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *>(pcr);
			PT_AttrPropIndex api = pcr->getIndexAP();
			switch (pcro->getObjectType())
            {
                case PTO_RDFAnchor:
                {
                    const PP_AttrProp * pAP = NULL;
                    getDocument()->getAttrProp(api,&pAP);
                    RDFAnchor a(pAP);
                    std::string xmlid = a.getID();
                    
                    if( a.isEnd() )
                    {
                        stringlist_t::iterator iter = find( m_rdfUnclosedAnchorStack.begin(),
                                                            m_rdfUnclosedAnchorStack.end(),
                                                            xmlid );
                        if( iter == m_rdfUnclosedAnchorStack.end() )
                        {
                            // closing an rdf anchor which was not opened in range.
                            m_rdfUnopenedAnchorStack.push_back( xmlid );
                        }
                        else
                        {
                            m_rdfUnclosedAnchorStack.erase( iter );
                        }
                    }
                    else
                    {
                        m_rdfUnclosedAnchorStack.push_back( xmlid );
                    }
                    
                    break;
                }
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
PL_ListenerCoupleCloser::populateStruxAfter( PL_StruxDocHandle /*sdh*/,
                                             const PX_ChangeRecord * pcr,
                                             PL_StruxFmtHandle * /* psfh */ )
{
    PT_AttrPropIndex indexAP = pcr->getIndexAP();
	UT_DEBUGMSG(("MIQ: PL_ListenerCoupleCloser::PopulateStruxAfter() indexAP %d pcr.type:%d \n",
                 indexAP, pcr->getType() ));
    return true;
}

bool
PL_ListenerCoupleCloser::populateAfter( PL_StruxFmtHandle sfh,
                                        const PX_ChangeRecord * pcr )
{
    PT_AttrPropIndex indexAP = pcr->getIndexAP();
	UT_DEBUGMSG(("MIQ: PL_ListenerCoupleCloser::PopulateAfter() indexAP %d pcr.type:%d \n",
                 indexAP, pcr->getType() ));
	switch (pcr->getType())
	{
        case PX_ChangeRecord::PXT_InsertSpan:
        {
            const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *>(pcr);
            UT_uint32 len = pcrs->getLength();
            return true;
        }
        case PX_ChangeRecord::PXT_InsertObject:
        {
            const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *>(pcr);
			PT_AttrPropIndex api = pcr->getIndexAP();
			switch (pcro->getObjectType())
            {
                case PTO_RDFAnchor:
                    if( !m_rdfUnclosedAnchorStack.empty() )
                    {
                        const PP_AttrProp * pAP = NULL;
                        getDocument()->getAttrProp(api,&pAP);
                        RDFAnchor a(pAP);
                        UT_DEBUGMSG(("MIQ: PL_ListenerCoupleCloser::PopulateAfter() rdfid:%s \n",
                                     a.getID().c_str() ));
                        stringlist_t::iterator iter = find( m_rdfUnclosedAnchorStack.begin(),
                                                            m_rdfUnclosedAnchorStack.end(),
                                                            a.getID() );
                        if( iter != m_rdfUnclosedAnchorStack.end() )
                        {
                            m_rdfUnclosedAnchorStack.erase( iter );
                            return m_delegate->populate( sfh, pcr );
                        }
                        break;
                    }
            }
            
            return true;
        }
        default:
            return true;
    }
    return true;
}

bool PL_ListenerCoupleCloser::AfterContentListener::populate( PL_StruxFmtHandle sfh,
                                                              const PX_ChangeRecord * pcr )
{
    return m_self->populateAfter( sfh, pcr );
}

bool
PL_ListenerCoupleCloser::AfterContentListener::populateStrux( PL_StruxDocHandle sdh,
                                                              const PX_ChangeRecord * pcr,
                                                              PL_StruxFmtHandle * psfh )
{
    return m_self->populateStruxAfter( sdh, pcr, psfh );
}

bool PL_ListenerCoupleCloser::AfterContentListener::isFinished()
{
    return m_self->m_rdfUnclosedAnchorStack.empty();
}

/****************************************/
/****************************************/
/****************************************/

bool
PL_ListenerCoupleCloser::populateStruxBefore( PL_StruxDocHandle /*sdh*/,
                                             const PX_ChangeRecord * pcr,
                                             PL_StruxFmtHandle * /* psfh */ )
{
    PT_AttrPropIndex indexAP = pcr->getIndexAP();
	UT_DEBUGMSG(("MIQ: PL_ListenerCoupleCloser::PopulateStruxBefore() indexAP %d pcr.type:%d \n",
                 indexAP, pcr->getType() ));

    
    return true;
}

bool
PL_ListenerCoupleCloser::populateBefore( PL_StruxFmtHandle sfh,
                                        const PX_ChangeRecord * pcr )
{
    PT_AttrPropIndex indexAP = pcr->getIndexAP();
	UT_DEBUGMSG(("MIQ: PL_ListenerCoupleCloser::PopulateBefore() indexAP %d pcr.type:%d \n",
                 indexAP, pcr->getType() ));
	switch (pcr->getType())
	{
        case PX_ChangeRecord::PXT_InsertSpan:
        {
            const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *>(pcr);
            UT_uint32 len = pcrs->getLength();
            return true;
        }
        case PX_ChangeRecord::PXT_InsertObject:
        {
            const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *>(pcr);
			PT_AttrPropIndex api = pcr->getIndexAP();
			switch (pcro->getObjectType())
            {
                case PTO_RDFAnchor:
                    if( !m_rdfUnopenedAnchorStack.empty() )
                    {
                        const PP_AttrProp * pAP = NULL;
                        getDocument()->getAttrProp(api,&pAP);
                        RDFAnchor a(pAP);
                        UT_DEBUGMSG(("MIQ: PL_ListenerCoupleCloser::PopulateBefore() rdfid:%s \n",
                                     a.getID().c_str() ));
                        stringlist_t::iterator iter = find( m_rdfUnopenedAnchorStack.begin(),
                                                            m_rdfUnopenedAnchorStack.end(),
                                                            a.getID() );
                        if( iter != m_rdfUnopenedAnchorStack.end() )
                        {
                            m_rdfUnopenedAnchorStack.erase( iter );
                            return m_delegate->populate( sfh, pcr );
                        }
                        break;
                    }
            }
            
            return true;
        }
        default:
            return true;
    }
    return true;
}


bool PL_ListenerCoupleCloser::BeforeContentListener::populate( PL_StruxFmtHandle sfh,
                                                              const PX_ChangeRecord * pcr )
{
    return m_self->populateBefore( sfh, pcr );
}

bool
PL_ListenerCoupleCloser::BeforeContentListener::populateStrux( PL_StruxDocHandle sdh,
                                                              const PX_ChangeRecord * pcr,
                                                              PL_StruxFmtHandle * psfh )
{
    return m_self->populateStruxBefore( sdh, pcr, psfh );
}

bool PL_ListenerCoupleCloser::BeforeContentListener::isFinished()
{
    return m_self->m_rdfUnopenedAnchorStack.empty();
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

