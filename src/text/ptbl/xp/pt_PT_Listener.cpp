/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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


#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include "pt_PieceTable.h"
#include "pf_Frag.h"
#include "pf_Frag_FmtMark.h"
#include "pf_Frag_Object.h"
#include "pf_Frag_Strux.h"
#include "pf_Frag_Strux_Block.h"
#include "pf_Frag_Strux_Section.h"
#include "pf_Frag_Text.h"
#include "pf_Fragments.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"


/*****************************************************************/
/*****************************************************************/

bool pt_PieceTable::tellListener(PL_Listener* pListener)
{
	return _tellAndMaybeAddListener(pListener, 0, false);
}

bool pt_PieceTable::addListener(PL_Listener* pListener,
								   PL_ListenerId listenerId)
{
	return _tellAndMaybeAddListener(pListener, listenerId, true);
}

bool pt_PieceTable::_tellAndMaybeAddListener(PL_Listener * pListener,
												PL_ListenerId listenerId,
												bool bAdd)
{
	// walk document and for each fragment, send a notification
	// to each layout.
  
	fl_ContainerLayout* sfh = 0;
	PT_DocPosition sum = 0;
	UT_uint32 blockOffset = 0;
	pf_Frag_Strux * pfs2 = NULL;
	bool bListensOnly = (pListener->getType() >= PTL_CollabExport);
	for (pf_Frag * pf = m_fragments.getFirst(); (pf); pf=pf->getNext())
	{
		switch (pf->getType())
		{
		case pf_Frag::PFT_Text:
			{
			        if(bListensOnly)
				{
			                break;
				}
				pf_Frag_Text * pft = static_cast<pf_Frag_Text *> (pf);
				PX_ChangeRecord * pcr = NULL;
				bool bStatus1 = false;
				bool bAddOffset = true;
				if(sfh != NULL)
				{
					bStatus1 = pft->createSpecialChangeRecord(&pcr,sum,blockOffset);
					UT_return_val_if_fail (bStatus1, false);
				}
				else
				{
					PT_DocPosition pos = pf->getPos();
					getStruxOfTypeFromPosition(listenerId,pos,PTX_Block,&sfh);

					getStruxOfTypeFromPosition(pos,PTX_Block,&pfs2);
					blockOffset = pos - pfs2->getPos() -1;
					bStatus1 = pft->createSpecialChangeRecord(&pcr,pos,blockOffset);
					UT_return_val_if_fail (bStatus1,false);
					// I do not understand at all why this was set to
					// false; if the doc contains a footnote section
					// followed by a text fragment and another
					// fragment, the text fragment and the fragment
					// after it are given identical block offsets !!!
					// Tomas, May, 12, 2003
					bAddOffset = true;
				}
				bool bStatus2 = pListener->populate(sfh,pcr);
				if (pcr)
					delete pcr;
				if (!bStatus2)
					return false;
				if(bAddOffset)
				{
					blockOffset += pf->getLength();
				}
			}
			break;
			
		case pf_Frag::PFT_Strux:
			{
				pfs2 = static_cast<pf_Frag_Strux *> (pf);
				sfh = 0;
			        if(bListensOnly)
				{
					pfs2->setFmtHandle(listenerId,sfh);
			                break;
				}
				PX_ChangeRecord * pcr = NULL;
				bool bStatus1 = pfs2->createSpecialChangeRecord(&pcr,sum);
				UT_return_val_if_fail (bStatus1, false);
				bool bStatus2 = pListener->populateStrux(pfs2,pcr,&sfh);

				// This can happen legally, for example when inserting a hdr/ftr strux
				// which was marked deleted in revisions mode -- such strux has no
				// corresponding layout element
				// UT_ASSERT_HARMLESS( sfh || !bAdd );
				if (bAdd && sfh)
				{
					pfs2->setFmtHandle(listenerId,sfh);
				}
				
				if (pcr)
					delete pcr;
				if (!bStatus2)
					return false;
				blockOffset = 0;
				if(isEndFootnote(pfs2))
				{
					sfh = NULL;
				}
			}
			break;

		case pf_Frag::PFT_Object:
			{
			        if(bListensOnly)
				{
			                break;
				}
				pf_Frag_Object * pfo = static_cast<pf_Frag_Object *> (pf);
				PX_ChangeRecord * pcr = NULL;
				bool bStatus1 = false;
				bool bAddOffset = true;
				if(sfh != NULL)
				{
					bStatus1 = pfo->createSpecialChangeRecord(&pcr,sum,blockOffset);
					UT_return_val_if_fail (bStatus1,false);
				}
				else
				{
					PT_DocPosition pos = pf->getPos();
					getStruxOfTypeFromPosition(listenerId,pos,PTX_Block,&sfh);
					pf_Frag_Strux* pfs = NULL;
					getStruxOfTypeFromPosition(pos,PTX_Block,&pfs);
					if(!pfs)
					  return false;
					blockOffset = pos - pfs->getPos() -1;
					bStatus1 = pfo->createSpecialChangeRecord(&pcr,pos,blockOffset);
					UT_return_val_if_fail (bStatus1, false);
					// I do not understand at all why this was set to
					// false; if the doc contains a footnote section
					// followed by a text fragment and another
					// fragment, the text fragment and the fragment
					// after it are given identical block offsets !!!
					// Tomas, May, 12, 2003
					bAddOffset = true;
				}

				UT_return_val_if_fail (bStatus1,false);
				bool bStatus2 = pListener->populate(sfh,pcr);
				if (pcr)
					delete pcr;
				if (!bStatus2)
					return false;
				if(bAddOffset)
				{
					blockOffset += pf->getLength();
				}
			}
			break;

		case pf_Frag::PFT_FmtMark:
			{
			        if(bListensOnly)
				{
			                break;
				}
				pf_Frag_FmtMark * pffm = static_cast<pf_Frag_FmtMark *> (pf);
				PX_ChangeRecord * pcr = NULL;
				bool bStatus1 = false;
				bool bAddOffset = true;
				if(sfh != NULL)
				{
					bStatus1 = pffm->createSpecialChangeRecord(&pcr,sum,blockOffset);
					UT_return_val_if_fail (bStatus1,false);
				}
				else
				{
					PT_DocPosition pos = pf->getPos();
					getStruxOfTypeFromPosition(listenerId,pos,PTX_Block,&sfh);
					getStruxOfTypeFromPosition(pos,PTX_Block,&pfs2);
					blockOffset = pos - pfs2->getPos() -1;
					bStatus1 = pffm->createSpecialChangeRecord(&pcr,pos,blockOffset);
					UT_return_val_if_fail (bStatus1, false);
					bAddOffset = false;
				}
				bool bStatus2 = pListener->populate(sfh,pcr);
				DELETEP(pcr);
				if (!bStatus2)
					return false;
				if(bAddOffset)
				{
					blockOffset += pf->getLength();
				}
			}
			break;
			
		case pf_Frag::PFT_EndOfDoc:
			// they don't get to know about this.
			break;
			
		default:
			UT_ASSERT_HARMLESS(0);
			return false;
		}

		sum += pf->getLength();
	}

	// TODO assert that sum == our cached value.
	
	return true;
}

#include "pl_ListenerCoupleCloser.h"
#include <set>
#include <boost/bind.hpp>
#include <boost/function.hpp>
typedef boost::function< bool (PT_DocPosition, PT_DocPosition, PT_DocPosition, PL_Listener*)> f_WalkRangeFinished_t;

static bool finishedFunctorEndOfRage( PT_DocPosition /*rangeStartPos*/,
                                      PT_DocPosition rangeEndPos,
                                      PT_DocPosition curPos,
                                      PL_Listener* /*pListener*/ )
{
    if( curPos >= rangeEndPos )
        return true;
    return false;
}

static bool finishedFunctorFinishingListener( PT_DocPosition /*rangeStartPos*/,
                                              PT_DocPosition /*rangeEndPos*/,
                                              PT_DocPosition /*curPos*/,
                                              PL_Listener* /*pListener*/,
                                              PL_FinishingListener* fl )
{
    if( fl->isFinished() )
        return true;
    return false;
}


typedef std::set< pf_Frag::PFType > m_fragtypecol_t;
static m_fragtypecol_t& _getTellListenerSubsetWalkRangeVisitAllFragments()
{
    static m_fragtypecol_t col;
    if( col.empty() )
    {
        col.insert( pf_Frag::PFT_Text );
        col.insert( pf_Frag::PFT_Object );
        col.insert( pf_Frag::PFT_Strux );
        col.insert( pf_Frag::PFT_EndOfDoc );
        col.insert( pf_Frag::PFT_FmtMark );
    }
    return col;
}


/**
 * This is a static function instead of a member so that
 * boost::functors can be passed in to this function but boost headers
 * are not needed in pt_PieceTable.h
 * 
 */
static PT_DocPosition _tellListenerSubsetWalkRange(
    pt_PieceTable* pt,
    PL_Listener* pListener,
    PD_DocumentRange*  /*pDocRange*/,
    PT_DocPosition rangeStartPos,
    PT_DocPosition rangeEndPos,
    f_WalkRangeFinished_t finishedFunctor = finishedFunctorEndOfRage,
    m_fragtypecol_t& fragmentTypesToVisit = _getTellListenerSubsetWalkRangeVisitAllFragments(),
    bool walkForwards = true )
{
    xxx_UT_DEBUGMSG(("_tellListenerSubsetWalkRange(top) listener %p startpos %d endpos %d\n",
                 pListener, rangeStartPos, rangeEndPos ));
	fl_ContainerLayout* sfh = 0;
	UT_uint32 blockOffset = 0;

	pf_Frag * pf1 = NULL;
	PT_BlockOffset fragOffset1 = 0;
	PT_BlockOffset endOffset = 0;
    PT_DocPosition pfPos = rangeStartPos;
    if( !walkForwards )
        pfPos = rangeEndPos;
    
    if (!pt->getFragFromPosition( pfPos, &pf1, &fragOffset1 ))
    {
        xxx_UT_DEBUGMSG(("_tellListenerSubsetWalkRange(no frag!) listener %p startpos %d endpos %d\n",
                     pListener, rangeStartPos, rangeEndPos ));
        return true;
    }
    
	PT_DocPosition sum = rangeStartPos - fragOffset1;

    pf_Frag * pf = pf1;
	for ( ; (pf); )
	{
        if( fragmentTypesToVisit.count(pf->getType()))
        {
            switch (pf->getType())
            {
                case pf_Frag::PFT_Text:
                {
                    pf_Frag_Text * pft = static_cast<pf_Frag_Text *> (pf);
                    PX_ChangeRecord * pcr = NULL;
                    if( rangeEndPos < sum+pf->getLength() )
                        endOffset = (rangeEndPos - sum);
                    else
                        endOffset = pf->getLength();
                    bool bStatus1 = pft->createSpecialChangeRecord(&pcr,sum,blockOffset,fragOffset1,endOffset);
                    if(!bStatus1) {
                        UT_DEBUGMSG(("_tellListenerSubsetWalkRange(st1) a\n" ));
		    }
                    UT_return_val_if_fail (bStatus1,false);
                    bool bStatus2 = pListener->populate(sfh,pcr);
                    if (pcr)
                        delete pcr;
                    if (!bStatus2)
                        return false;
                    blockOffset += pf->getLength();
                    fragOffset1 = 0;
                }
                break;
			
                case pf_Frag::PFT_Strux:
                {
                    pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *> (pf);
                    pf_Frag_Strux* sdh = pfs;
                    sfh = 0;
                    PX_ChangeRecord * pcr = NULL;
                    bool bStatus1 = pfs->createSpecialChangeRecord(&pcr,sum);
                    if(!bStatus1) {
                        UT_DEBUGMSG(("_tellListenerSubsetWalkRange(st1) b\n" ));
		    }
                    UT_return_val_if_fail (bStatus1,false);
                    bool bStatus2 = pListener->populateStrux(sdh,pcr,&sfh);
                    if (pcr)
                        delete pcr;
                    if (!bStatus2)
                        return false;
                    blockOffset = 0;
                }
                break;

                case pf_Frag::PFT_Object:
                {
                    pf_Frag_Object * pfo = static_cast<pf_Frag_Object *> (pf);
                    PX_ChangeRecord * pcr = NULL;
                    bool bStatus1 = pfo->createSpecialChangeRecord(&pcr,sum,blockOffset);
                    if(!bStatus1) {
                        UT_DEBUGMSG(("_tellListenerSubsetWalkRange(st1) c\n" ));
		    }
                    UT_return_val_if_fail (bStatus1,false);
                    bool bStatus2 = pListener->populate(sfh,pcr);
                    if (pcr)
                        delete pcr;
                    if (!bStatus2)
                        return false;
                    blockOffset += pf->getLength();
                }
                break;

                case pf_Frag::PFT_FmtMark:
                {
                    pf_Frag_FmtMark * pffm = static_cast<pf_Frag_FmtMark *> (pf);
                    PX_ChangeRecord * pcr = NULL;
                    bool bStatus1 = pffm->createSpecialChangeRecord(&pcr,sum,blockOffset);
                    if(!bStatus1) {
                        UT_DEBUGMSG(("_tellListenerSubsetWalkRange(st1) d\n" ));
		    }
                    UT_return_val_if_fail (bStatus1,false);
                    bool bStatus2 = pListener->populate(sfh,pcr);
                    DELETEP(pcr);
                    if (!bStatus2)
                        return false;
                    blockOffset += pf->getLength();
                }
                break;
			
                case pf_Frag::PFT_EndOfDoc:
                    // they don't get to know about this.
                    break;
			
                default:
                    UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
                    return false;
            }
        }
        
        xxx_UT_DEBUGMSG(("_tellListenerSubsetWalkRange(loop) listener %p sum %d startpos %d endpos %d\n",
                     pListener, sum, rangeStartPos, rangeEndPos ));
        
		sum += pf->getLength();
        if( finishedFunctor( rangeStartPos, rangeEndPos, sum, pListener ) )
			break;

        if( walkForwards )
        {
            pf=pf->getNext();
        }
        else
        {
            pf=pf->getPrev();
        }
	}

    xxx_UT_DEBUGMSG(("_tellListenerSubsetWalkRange(done) listener %p startpos %d endpos %d\n",
                 pListener, rangeStartPos, rangeEndPos ));
    return sum;
}



bool pt_PieceTable::tellListenerSubset( PL_Listener * pListener,
                                        PD_DocumentRange * pDocRange,
                                        PL_ListenerCoupleCloser* closer )
{
	// walk the subset of the document in the given range
	// and send notifications.

    if( closer )
    {
        closer->setDocument( getDocument() );
        closer->setDelegate( pListener );
    }
    m_fragtypecol_t closerFragmentTypesToVisit;
    closerFragmentTypesToVisit.insert( pf_Frag::PFT_Object );
    closerFragmentTypesToVisit.insert( pf_Frag::PFT_Strux );
    

    //bool rc = 0;

    if( closer )
    {
        /*rc =*/ _tellListenerSubsetWalkRange( this,
                                           closer,
                                           pDocRange,
                                           pDocRange->m_pos1,
                                           pDocRange->m_pos2 );

        /**
         * Emit the start tag for things that are closed in the
         * selected range but are not opened in that range.
         *
         * Note that we walk backwards from the start of the range in
         * order to find the matching open tags as quickly as possible
         * even if the selection is at the end of a really large
         * document.
         *
         * A null delegate is used while we are walking backwards. If
         * we allowed the closer to emit to the real delegate then the
         * calls to populate() on the delegate would happen in reverse
         * document order. So we walk backwards to find the real
         * startPos that the closer needs and then refresh the closer
         * by walking the range again (in case it uses stacks which
         * were erased during the reverse walk), and then use the real
         * delegate and walk forwards from the correct startPos that
         * we just found. This is admittedly a bit tricky, but for a
         * 1000 page document we really really don't want to walk all
         * the way from the start, so walking the range twice is
         * likely to be a small trade off in performance.
         */
        if( PL_FinishingListener* cl = closer->getBeforeContentListener() )
        {
            bool walkForwards = false;
            f_WalkRangeFinished_t f = boost::bind( finishedFunctorFinishingListener, _1, _2, _3, _4, cl );

            PL_FinishingListener* nullListener = closer->getNullContentListener();
            closer->setDelegate( nullListener );
            PT_DocPosition startPos = _tellListenerSubsetWalkRange( this, cl,
                                                                    pDocRange, 0, pDocRange->m_pos1,
                                                                    f, closerFragmentTypesToVisit, walkForwards );

            closer->setDelegate( pListener );
            closer->reset();
            /*rc =*/ _tellListenerSubsetWalkRange( this,
                                               closer,
                                               pDocRange,
                                               pDocRange->m_pos1,
                                               pDocRange->m_pos2 );
            
            /*rc =*/ _tellListenerSubsetWalkRange( this, cl,
                                               pDocRange, startPos, pDocRange->m_pos1,
                                               f, closerFragmentTypesToVisit, walkForwards );
            
        }
    }
    
    /*rc =*/ _tellListenerSubsetWalkRange( this, pListener,
                                       pDocRange, pDocRange->m_pos1, pDocRange->m_pos2 );
    
    if( closer )
    {
        /**
         * emit the close tag for things that were left open in the range.
         */
        if( PL_FinishingListener* cl = closer->getAfterContentListener() )
        {
            f_WalkRangeFinished_t f = boost::bind( finishedFunctorFinishingListener, _1, _2, _3, _4, cl );
            /*rc =*/ _tellListenerSubsetWalkRange( this, cl,
                                               pDocRange, pDocRange->m_pos2, 0,
                                               f, closerFragmentTypesToVisit );
        }
    }


    // MIQ:2011, old code...
    // move to using the above walker to allow mulitpass processing....
#if 0    
    
	fl_ContainerLayout* sfh = 0;
	UT_uint32 blockOffset = 0;

	pf_Frag * pf1 = NULL;
	PT_BlockOffset fragOffset1 = 0;
	PT_BlockOffset endOffset = 0;
	
	if (!getFragFromPosition(pDocRange->m_pos1, &pf1, &fragOffset1))
		return true;

	PT_DocPosition sum = pDocRange->m_pos1 - fragOffset1;

    pf_Frag * pf = pf1;
	for ( ; (pf); pf=pf->getNext())
	{
		switch (pf->getType())
		{
		case pf_Frag::PFT_Text:
			{
				pf_Frag_Text * pft = static_cast<pf_Frag_Text *> (pf);
				PX_ChangeRecord * pcr = NULL;
				if (pDocRange->m_pos2 < sum+pf->getLength())
					endOffset = (pDocRange->m_pos2 - sum);
				else
					endOffset = pf->getLength();
				bool bStatus1 = pft->createSpecialChangeRecord(&pcr,sum,blockOffset,fragOffset1,endOffset);
				UT_return_val_if_fail (bStatus1,false);
				bool bStatus2 = pListener->populate(sfh,pcr);
				if (pcr)
					delete pcr;
				if (!bStatus2)
					return false;
				blockOffset += pf->getLength();
				fragOffset1 = 0;
			}
			break;
			
		case pf_Frag::PFT_Strux:
			{
				pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *> (pf);
				pf_Frag_Strux* sdh = pf;
				sfh = 0;
				PX_ChangeRecord * pcr = NULL;
				bool bStatus1 = pfs->createSpecialChangeRecord(&pcr,sum);
				UT_return_val_if_fail (bStatus1,false);
				bool bStatus2 = pListener->populateStrux(sdh,pcr,&sfh);
                if( closer )
                    closer->populateStrux(sdh,pcr,&sfh);
				if (pcr)
					delete pcr;
				if (!bStatus2)
					return false;
				blockOffset = 0;
			}
			break;

		case pf_Frag::PFT_Object:
			{
				pf_Frag_Object * pfo = static_cast<pf_Frag_Object *> (pf);
				PX_ChangeRecord * pcr = NULL;
				bool bStatus1 = pfo->createSpecialChangeRecord(&pcr,sum,blockOffset);
				UT_return_val_if_fail (bStatus1,false);
				bool bStatus2 = pListener->populate(sfh,pcr);
                if( closer )
                    closer->populate(sfh,pcr);
				if (pcr)
					delete pcr;
				if (!bStatus2)
					return false;
				blockOffset += pf->getLength();
			}
			break;

		case pf_Frag::PFT_FmtMark:
			{
				pf_Frag_FmtMark * pffm = static_cast<pf_Frag_FmtMark *> (pf);
				PX_ChangeRecord * pcr = NULL;
				bool bStatus1 = pffm->createSpecialChangeRecord(&pcr,sum,blockOffset);
				UT_return_val_if_fail (bStatus1,false);
				bool bStatus2 = pListener->populate(sfh,pcr);
				DELETEP(pcr);
				if (!bStatus2)
					return false;
				blockOffset += pf->getLength();
			}
			break;
			
		case pf_Frag::PFT_EndOfDoc:
			// they don't get to know about this.
			break;
			
		default:
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			return false;
		}

		sum += pf->getLength();
		if (sum >= pDocRange->m_pos2)
			break;
	}

    // MIQ:2011
    // The closer might want to inspect elements after the selected
    // range to find end tags to emit as well. This allows things with
    // a separate start and end element to have the end sent to the
    // pListener although that end might not itself be in the range.
    // Without this, we might have a bookmark-start but no matching
    // bookmark-end, which will make a generated document invalid.
    //
    if( closer )
    {
        closer->setDelegate( pListener );
        for ( ; (pf); pf=pf->getNext())
        {
            switch (pf->getType())
            {
                case pf_Frag::PFT_Strux:
                {
                    pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *> (pf);
                    pf_Frag_Strux* sdh = pf;
                    sfh = 0;
                    PX_ChangeRecord * pcr = NULL;
                    bool bStatus1 = pfs->createSpecialChangeRecord(&pcr,sum);
                    UT_return_val_if_fail (bStatus1,false);
                    closer->populateStruxClose(sdh,pcr,&sfh);
                    if (pcr)
                        delete pcr;
                    blockOffset = 0;
                }
                break;

                case pf_Frag::PFT_Object:
                {
                    pf_Frag_Object * pfo = static_cast<pf_Frag_Object *> (pf);
                    PX_ChangeRecord * pcr = NULL;
                    bool bStatus1 = pfo->createSpecialChangeRecord(&pcr,sum,blockOffset);
                    UT_return_val_if_fail (bStatus1,false);
                    closer->populateClose(sfh,pcr);
                    if (pcr)
                        delete pcr;
                    blockOffset += pf->getLength();
                }
                break;
            }
            
            if( closer->isFinished() )
                break;
        }
    }
#endif
    
	return true;
}
