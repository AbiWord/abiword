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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
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
  
	PL_StruxFmtHandle sfh = 0;
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
					PL_StruxDocHandle sdh = NULL;
					getStruxOfTypeFromPosition(pos,PTX_Block,&sdh);
					pfs2 = (pf_Frag_Strux *) sdh;
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
				PL_StruxDocHandle sdh = (PL_StruxDocHandle)pf;
				sfh = 0;
			        if(bListensOnly)
				{
					pfs2->setFmtHandle(listenerId,sfh);
			                break;
				}
				PX_ChangeRecord * pcr = NULL;
				bool bStatus1 = pfs2->createSpecialChangeRecord(&pcr,sum);
				UT_return_val_if_fail (bStatus1, false);
				bool bStatus2 = pListener->populateStrux(sdh,pcr,&sfh);

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
					PL_StruxDocHandle sdh = NULL;
					getStruxOfTypeFromPosition(pos,PTX_Block,&sdh);
					pf_Frag_Strux * pfs = (pf_Frag_Strux *) sdh;
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
					PL_StruxDocHandle sdh = NULL;
					getStruxOfTypeFromPosition(pos,PTX_Block,&sdh);
					pfs2 = (pf_Frag_Strux *) sdh;
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

bool pt_PieceTable::tellListenerSubset(PL_Listener * pListener,
										  PD_DocumentRange * pDocRange)
{
	// walk the subset of the document in the given range
	// and send notifications.

	PL_StruxFmtHandle sfh = 0;
	UT_uint32 blockOffset = 0;

	pf_Frag * pf1 = NULL;
	PT_BlockOffset fragOffset1 = 0;
	PT_BlockOffset endOffset = 0;
	
	if (!getFragFromPosition(pDocRange->m_pos1, &pf1, &fragOffset1))
		return true;

	PT_DocPosition sum = pDocRange->m_pos1 - fragOffset1;
	
	for (pf_Frag * pf = pf1; (pf); pf=pf->getNext())
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
				PL_StruxDocHandle sdh = (PL_StruxDocHandle)pf;
				sfh = 0;
				PX_ChangeRecord * pcr = NULL;
				bool bStatus1 = pfs->createSpecialChangeRecord(&pcr,sum);
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

	return true;
}
