/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Martin Sevior <msevior@physics.unimelb.edu.au>
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


#include "ie_imp_PasteListener.h"


/*!
 * This nifty little class allows all importers to also be used for pasting
 * into the document.
 * The idea is that we create a dummy document which we import to as usual
 * with the impoters.
 * After the Dummy document is completed we do a PD_Document::tellListener on
 * it with this class as the listner class.
 * This class translates all the populate().... and populateStrux()...
 * methods into insertSpan(..) insertStrux(...) methods at the current 
 * insertion point.
 *
 * Hey presto we have pasted into the current document. Pretty cool eh?
 */
IE_Imp_PasteListener::IE_Imp_PasteListener(PD_Document * pDocToPaste, PT_DocPosition insPoint) : 
	m_pPasteDocument(pDocToPaste),
	m_insPoint(insPoint)
{
}	
bool  IE_Imp_PasteListener::populate(PL_StruxFmtHandle /* sfh */,
					 const PX_ChangeRecord * pcr)
{
	return true;
}

bool  IE_Imp_PasteListener::populateStrux(PL_StruxDocHandle sdh,
									  const PX_ChangeRecord * pcr,
										  PL_StruxFmtHandle * /* psfh */)
{
	return true;
}

PD_Document * IE_Imp_PasteListener::getDoc(void) const
{
	return m_pPasteDocument;
}

