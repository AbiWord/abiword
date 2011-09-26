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
                    m_rdfAnchorStack.push_back(a.getID());
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

bool
PL_ListenerCoupleCloser::populateClose( PL_StruxFmtHandle sfh,
                                        const PX_ChangeRecord * pcr )
{
    PT_AttrPropIndex indexAP = pcr->getIndexAP();
	UT_DEBUGMSG(("MIQ: PL_ListenerCoupleCloser::PopulateClose() indexAP %d pcr.type:%d \n",
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
                    if( !m_rdfAnchorStack.empty() )
                    {
                        const PP_AttrProp * pAP = NULL;
                        getDocument()->getAttrProp(api,&pAP);
                        RDFAnchor a(pAP);
                        UT_DEBUGMSG(("MIQ: PL_ListenerCoupleCloser::PopulateClose() rdfid:%s \n",
                                     a.getID().c_str() ));
                        stringlist_t::iterator iter = find( m_rdfAnchorStack.begin(),
                                                            m_rdfAnchorStack.end(),
                                                            a.getID() );
                        if( iter != m_rdfAnchorStack.end() )
                        {
                            m_rdfAnchorStack.erase( iter );
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


bool
PL_ListenerCoupleCloser::populateStrux( PL_StruxDocHandle /*sdh*/,
                                        const PX_ChangeRecord * pcr,
                                        PL_StruxFmtHandle * /* psfh */ )
{
    return true;
}

bool
PL_ListenerCoupleCloser::populateStruxClose( PL_StruxDocHandle /*sdh*/,
                                             const PX_ChangeRecord * pcr,
                                             PL_StruxFmtHandle * /* psfh */ )
{
    return true;
}



void
PL_ListenerCoupleCloser::setDelegate( PL_Listener* delegate )
{
    m_delegate = delegate;
}

bool
PL_ListenerCoupleCloser::isFinished()
{
    return m_rdfAnchorStack.empty();
}

