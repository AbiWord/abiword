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
#include "pf_Frag_Object.h"
#include "pf_Frag_Strux.h"
#include "pf_Frag_Strux_Block.h"
#include "pf_Frag_Strux_Section.h"
#include "pf_Frag_Text.h"
#include "pf_Fragments.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_Object.h"
#include "px_ChangeRecord_Span.h"
#include "px_ChangeRecord_SpanChange.h"
#include "px_ChangeRecord_Strux.h"


#define NrElements(a)		(sizeof(a)/sizeof(a[0]))

/*****************************************************************/
/*****************************************************************/

UT_Bool pt_PieceTable::addListener(PL_Listener * pListener,
								   PL_ListenerId listenerId)
{
	// walk document and for each fragment, send a notification
	// to each layout.

	PL_StruxFmtHandle sfh = 0;
	PT_DocPosition sum = 0;
	
	for (pf_Frag * pf = m_fragments.getFirst(); (pf); pf=pf->getNext())
	{
		switch (pf->getType())
		{
		case pf_Frag::PFT_Text:
			{
				pf_Frag_Text * pft = static_cast<pf_Frag_Text *> (pf);
				PX_ChangeRecord * pcr = NULL;
				UT_Bool bStatus1 = pft->createSpecialChangeRecord(&pcr,sum);
				UT_ASSERT(bStatus1);
				UT_Bool bStatus2 = pListener->populate(sfh,pcr);
				if (pcr)
					delete pcr;
				if (!bStatus2)
					return UT_FALSE;
			}
			break;
			
		case pf_Frag::PFT_Strux:
			{
				pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *> (pf);
				PL_StruxDocHandle sdh = (PL_StruxDocHandle)pf;
				sfh = 0;
				PX_ChangeRecord * pcr = NULL;
				UT_Bool bStatus1 = pfs->createSpecialChangeRecord(&pcr,sum);
				UT_ASSERT(bStatus1);
				UT_Bool bStatus2 = pListener->populateStrux(sdh,pcr,&sfh);
				pfs->setFmtHandle(listenerId,sfh);
				
				if (pcr)
					delete pcr;
				if (!bStatus2)
					return UT_FALSE;
			}
			break;

		case pf_Frag::PFT_Object:
			{
				pf_Frag_Object * pfo = static_cast<pf_Frag_Object *> (pf);
				PX_ChangeRecord * pcr = NULL;
				UT_Bool bStatus1 = pfo->createSpecialChangeRecord(&pcr,sum);
				UT_ASSERT(bStatus1);
				UT_Bool bStatus2 = pListener->populate(sfh,pcr);
				if (pcr)
					delete pcr;
				if (!bStatus2)
					return UT_FALSE;
			}
			break;
			
		case pf_Frag::PFT_EndOfDoc:
			// they don't get to know about this.
			break;
			
		default:
			UT_ASSERT(0);
			return UT_FALSE;
		}

		sum += pf->getLength();
	}

	// TODO assert that sum == our cached value.
	
	return UT_TRUE;
}
