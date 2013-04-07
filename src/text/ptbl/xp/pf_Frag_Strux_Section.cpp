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


#include "pf_Frag_Strux_Section.h"
#include "px_ChangeRecord.h"
#include "px_CR_Strux.h"

/*****************************************************************/
/*****************************************************************/

pf_Frag_Strux_Section::pf_Frag_Strux_Section(pt_PieceTable * pPT,
											 PT_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_Section,pf_FRAG_STRUX_SECTION_LENGTH,indexAP)
{
	m_struxType =  PTX_Section;
}

pf_Frag_Strux_Section::~pf_Frag_Strux_Section()
{
}

pf_Frag_Strux_SectionHdrFtr::pf_Frag_Strux_SectionHdrFtr(pt_PieceTable * pPT,
											 PT_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_SectionHdrFtr,pf_FRAG_STRUX_SECTION_LENGTH,indexAP)
{
	m_struxType =  PTX_SectionHdrFtr;
}

pf_Frag_Strux_SectionHdrFtr::~pf_Frag_Strux_SectionHdrFtr()
{
}

pf_Frag_Strux_SectionEndnote::pf_Frag_Strux_SectionEndnote(pt_PieceTable * pPT,
											 PT_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_SectionEndnote,pf_FRAG_STRUX_SECTION_LENGTH,indexAP)
{
	m_struxType =  PTX_SectionEndnote;
}

pf_Frag_Strux_SectionEndnote::~pf_Frag_Strux_SectionEndnote()
{
}

pf_Frag_Strux_SectionTable::pf_Frag_Strux_SectionTable(pt_PieceTable * pPT,
											 PT_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_SectionTable,pf_FRAG_STRUX_SECTION_LENGTH,indexAP)
{
	m_struxType =  PTX_SectionTable;
}

pf_Frag_Strux_SectionTable::~pf_Frag_Strux_SectionTable()
{
}

pf_Frag_Strux_SectionCell::pf_Frag_Strux_SectionCell(pt_PieceTable * pPT,
											 PT_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_SectionCell,pf_FRAG_STRUX_SECTION_LENGTH,indexAP)
{
	m_struxType =  PTX_SectionCell;
}

pf_Frag_Strux_SectionCell::~pf_Frag_Strux_SectionCell()
{
}


pf_Frag_Strux_SectionFootnote::pf_Frag_Strux_SectionFootnote(pt_PieceTable * pPT,
											 PT_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_SectionFootnote,pf_FRAG_STRUX_SECTION_LENGTH,indexAP)
{
	m_struxType =  PTX_SectionFootnote;
}

pf_Frag_Strux_SectionFootnote::~pf_Frag_Strux_SectionFootnote()
{
}


pf_Frag_Strux_SectionAnnotation::pf_Frag_Strux_SectionAnnotation(pt_PieceTable * pPT,
											 PT_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_SectionAnnotation,pf_FRAG_STRUX_SECTION_LENGTH,indexAP)
{
	m_struxType =  PTX_SectionAnnotation;
}

pf_Frag_Strux_SectionAnnotation::~pf_Frag_Strux_SectionAnnotation()
{
}

pf_Frag_Strux_SectionMarginnote::pf_Frag_Strux_SectionMarginnote(pt_PieceTable * pPT,
											 PT_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_SectionHdrFtr,pf_FRAG_STRUX_SECTION_LENGTH,indexAP)
{
	m_struxType =  PTX_SectionMarginnote;
}

pf_Frag_Strux_SectionMarginnote::~pf_Frag_Strux_SectionMarginnote()
{
}

pf_Frag_Strux_SectionFrame::pf_Frag_Strux_SectionFrame(pt_PieceTable * pPT,
											 PT_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_SectionHdrFtr,pf_FRAG_STRUX_SECTION_LENGTH,indexAP)
{
	m_struxType =  PTX_SectionFrame;
}

pf_Frag_Strux_SectionFrame::~pf_Frag_Strux_SectionFrame()
{
}


pf_Frag_Strux_SectionEndFootnote::pf_Frag_Strux_SectionEndFootnote(pt_PieceTable * pPT,
											 PT_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_SectionFootnote,pf_FRAG_STRUX_SECTION_LENGTH,indexAP)
{
	m_struxType =  PTX_EndFootnote;
}

pf_Frag_Strux_SectionEndFootnote::~pf_Frag_Strux_SectionEndFootnote()
{
}



pf_Frag_Strux_SectionEndAnnotation::pf_Frag_Strux_SectionEndAnnotation(pt_PieceTable * pPT,
											 PT_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_EndAnnotation,pf_FRAG_STRUX_SECTION_LENGTH,indexAP)
{
	m_struxType =  PTX_EndAnnotation;
}

pf_Frag_Strux_SectionEndAnnotation::~pf_Frag_Strux_SectionEndAnnotation()
{
}


pf_Frag_Strux_SectionEndEndnote::pf_Frag_Strux_SectionEndEndnote(pt_PieceTable * pPT,
											 PT_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_SectionEndnote,pf_FRAG_STRUX_SECTION_LENGTH,indexAP)
{
	m_struxType =  PTX_EndEndnote;
}

pf_Frag_Strux_SectionEndEndnote::~pf_Frag_Strux_SectionEndEndnote()
{
}


pf_Frag_Strux_SectionEndTable::pf_Frag_Strux_SectionEndTable(pt_PieceTable * pPT,
											 PT_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_SectionHdrFtr,pf_FRAG_STRUX_SECTION_LENGTH,indexAP)
{
	m_struxType =  PTX_EndTable;
}

pf_Frag_Strux_SectionEndTable::~pf_Frag_Strux_SectionEndTable()
{
}

pf_Frag_Strux_SectionEndCell::pf_Frag_Strux_SectionEndCell(pt_PieceTable * pPT,
											 PT_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_SectionHdrFtr,pf_FRAG_STRUX_SECTION_LENGTH,indexAP)
{
	m_struxType =  PTX_EndCell;
}

pf_Frag_Strux_SectionEndCell::~pf_Frag_Strux_SectionEndCell()
{
}

pf_Frag_Strux_SectionEndMarginnote::pf_Frag_Strux_SectionEndMarginnote(pt_PieceTable * pPT,
											 PT_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_SectionHdrFtr,pf_FRAG_STRUX_SECTION_LENGTH,indexAP)
{
	m_struxType =  PTX_EndMarginnote;
}

pf_Frag_Strux_SectionEndMarginnote::~pf_Frag_Strux_SectionEndMarginnote()
{
}

pf_Frag_Strux_SectionEndFrame::pf_Frag_Strux_SectionEndFrame(pt_PieceTable * pPT,
											 PT_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_SectionHdrFtr,pf_FRAG_STRUX_SECTION_LENGTH,indexAP)
{
	m_struxType =  PTX_EndFrame;
}

pf_Frag_Strux_SectionEndFrame::~pf_Frag_Strux_SectionEndFrame()
{
}



pf_Frag_Strux_SectionTOC::pf_Frag_Strux_SectionTOC(pt_PieceTable * pPT,
											 PT_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_SectionHdrFtr,pf_FRAG_STRUX_SECTION_LENGTH,indexAP)
{
	m_struxType =  PTX_SectionTOC;
}

pf_Frag_Strux_SectionTOC::~pf_Frag_Strux_SectionTOC()
{
}


pf_Frag_Strux_SectionEndTOC::pf_Frag_Strux_SectionEndTOC(pt_PieceTable * pPT,
											 PT_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_SectionHdrFtr,pf_FRAG_STRUX_SECTION_LENGTH,indexAP)
{
	m_struxType =  PTX_EndTOC;
}

pf_Frag_Strux_SectionEndTOC::~pf_Frag_Strux_SectionEndTOC()
{
}





