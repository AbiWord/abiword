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


#ifndef PT_PIECETABLE_H
#define PT_PIECETABLE_H

#include <stdio.h>
#include "ut_types.h"
#include "ut_growbuf.h"
#include "pt_Types.h"
#include "pp_TableAttrProp.h"
#include "px_ChangeHistory.h"
#include "pf_Fragments.h"
#include "pt_VarSet.h"
class pf_Frag_Object;
class pf_Frag_Text;
class pf_Frag_Strux;
class pf_Frag_Strux_Block;
class PX_ChangeRecord_Span;

#ifdef PT_TEST
#include "ut_test.h"
#endif

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
// pt_PieceTable implements a "Piece Table" as described/suggested
// by .../dev/design/PieceTable.html

class pt_PieceTable
{
public:
	pt_PieceTable(PD_Document * pDocument);
	~pt_PieceTable();

	void					setPieceTableState(PTState pts);

	void					beginMultiStepGlob(void);
	void					endMultiStepGlob(void);
	void					beginUserAtomicGlob(void);
	void					endUserAtomicGlob(void);

	void					setClean(void);
	UT_Bool					isDirty(void) const;
	
	UT_Bool					canDo(UT_Bool bUndo) const;
	UT_Bool					undoCmd(void);
	UT_Bool					redoCmd(void);

	UT_Bool					insertObject(PT_DocPosition dpos,
										 PTObjectType pto,
										 const XML_Char ** attributes,
										 const XML_Char ** properties);

	UT_Bool					insertSpan(PT_DocPosition dpos,
									   const UT_UCSChar * p,
									   UT_uint32 length);

	UT_Bool					deleteSpan(PT_DocPosition dpos1,
									   PT_DocPosition dpos2);

	UT_Bool					changeSpanFmt(PTChangeFmt ptc,
										  PT_DocPosition dpos1,
										  PT_DocPosition dpos2,
										  const XML_Char ** attributes,
										  const XML_Char ** properties);

	UT_Bool					insertStrux(PT_DocPosition dpos,
										PTStruxType pts);

	UT_Bool					changeStruxFmt(PTChangeFmt ptc,
										   PT_DocPosition dpos1,
										   PT_DocPosition dpos2,
										   const XML_Char ** attributes,
										   const XML_Char ** properties,
										   PTStruxType pts);

	// the append- methods are only available while importing
	// the document.

	UT_Bool					appendStrux(PTStruxType pts, const XML_Char ** attributes);
	UT_Bool					appendFmt(const XML_Char ** attributes);
	UT_Bool					appendFmt(const UT_Vector * pVecAttributes);
	UT_Bool					appendSpan(UT_UCSChar * p, UT_uint32 length);
	UT_Bool					appendObject(PTObjectType pto, const XML_Char ** attributes);

	UT_Bool					addListener(PL_Listener * pListener,
										PL_ListenerId listenerId);
	
	UT_Bool					getAttrProp(PT_AttrPropIndex indexAP,
										const PP_AttrProp ** ppAP) const;
	UT_Bool					getSpanAttrProp(PL_StruxDocHandle sdh, UT_uint32 offset,
											const PP_AttrProp ** ppAP) const;

	const UT_UCSChar *		getPointer(PT_BufIndex bi) const;
	UT_Bool					getSpanPtr(PL_StruxDocHandle sdh, UT_uint32 offset,
									   const UT_UCSChar ** ppSpan, UT_uint32 * pLength) const;
	UT_Bool					getBlockBuf(PL_StruxDocHandle sdh, UT_GrowBuf * pgb) const;

	UT_Bool					getBounds(UT_Bool bEnd, PT_DocPosition & docPos) const;
	PT_DocPosition			getStruxPosition(PL_StruxDocHandle sdh) const;
	PT_DocPosition			getFragPosition(const pf_Frag * pfToFind) const;
	
	UT_Bool					getFragFromPosition(PT_DocPosition docPos,
												pf_Frag ** ppf,
												PT_BlockOffset * pOffset) const;
	UT_Bool					getStruxFromPosition(PL_ListenerId listenerId,
												 PT_DocPosition docPos,
												 PL_StruxFmtHandle * psfh) const;
	UT_Bool					getStruxOfTypeFromPosition(PL_ListenerId listenerId,
													   PT_DocPosition docPos,
													   PTStruxType pts,
													   PL_StruxFmtHandle * psfh) const;

	void					clearTemporarySpanFmt(void);
	
	// TODO add stuff for objects like in-line images.

#ifdef PT_TEST
	UT_TestStatus			__test_VerifyCoalescedFrags(FILE * fp) const;
	void					__dump(FILE * fp) const;
#endif /* PT_TEST */
	
protected:

	void					_captureActiveSpan(pf_Frag_Strux_Block * pfsBlock);
	PT_AttrPropIndex		_chooseIndexAP(pf_Frag * pf, PT_BlockOffset fragOffset);
	UT_Bool					_canCoalesceInsertSpan(PX_ChangeRecord_Span * pcrSpan) const;

	UT_Bool					_createStrux(PTStruxType pts,
										 PT_AttrPropIndex indexAP,
										 pf_Frag_Strux ** ppfs);

	void					_insertStrux(pf_Frag * pf,
										 PT_BlockOffset fragOffset,
										 pf_Frag_Strux * pfsNew);

	UT_Bool					_insertObject(pf_Frag * pf,
										  PT_BlockOffset fragOffset,									 
										  PTObjectType pto,
										  PT_AttrPropIndex indexAP);
	
	UT_Bool					_createObject(PTObjectType pto,
										  PT_AttrPropIndex indexAP,
										  pf_Frag_Object ** ppfo);
	
	UT_Bool					_insertSpan(pf_Frag * pf,
										PT_BufIndex bi,
										PT_BlockOffset fragOffset,
										UT_uint32 length,
										PT_AttrPropIndex indexAP);

	UT_Bool					_deleteSpan(pf_Frag_Text * pft, UT_uint32 fragOffset,
										PT_BufIndex bi, UT_uint32 length,
										pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);
	void					_unlinkFrag(pf_Frag * pf,
										pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);
	UT_Bool					_getStruxFromPosition(PT_DocPosition docPos,
												  pf_Frag_Strux ** ppfs) const;
	UT_Bool					_getStruxOfTypeFromPosition(PT_DocPosition dpos,
														PTStruxType pts,
														pf_Frag_Strux ** ppfs) const;
	UT_Bool					_doTheDo(const PX_ChangeRecord * pcr, UT_Bool bUndo);
	UT_Bool					_struxHasContent(pf_Frag_Strux * pfs) const;
	UT_Bool					_unlinkStrux_Block(pf_Frag_Strux * pfs,
											   pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);

	UT_Bool					_deleteSpanWithNotify(PT_DocPosition dpos,
												  pf_Frag_Text * pft, UT_uint32 fragOffset,
												  UT_uint32 length,
												  pf_Frag_Strux * pfs,
												  pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);
	UT_Bool					_canCoalesceDeleteSpan(PX_ChangeRecord_Span * pcrSpan) const;
	
	UT_Bool					_deleteObject(pf_Frag_Object * pfo,
										  pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);
	
	UT_Bool					_deleteObjectWithNotify(PT_DocPosition dpos,
													pf_Frag_Object * pfo, UT_uint32 fragOffset,
													UT_uint32 length,
													pf_Frag_Strux * pfs,
													pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);

	UT_Bool					_deleteStruxWithNotify(PT_DocPosition dpos,
												   pf_Frag_Strux * pfs,
												   pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);

	UT_Bool					_fmtChangeStrux(pf_Frag_Strux * pfs,
											PT_AttrPropIndex indexNewAP);
	
	UT_Bool					_fmtChangeStruxWithNotify(PTChangeFmt ptc,
													  pf_Frag_Strux * pfs,
													  const XML_Char ** attributes,
													  const XML_Char ** properties);

	UT_Bool					_fmtChangeSpan(pf_Frag_Text * pft, UT_uint32 fragOffset, UT_uint32 length,
										   PT_AttrPropIndex indexNewAP,
										   pf_Frag ** ppfNewEnd, UT_uint32 * pfragOffsetNewEnd);
	
	UT_Bool					_fmtChangeSpanWithNotify(PTChangeFmt ptc,
													 pf_Frag_Text * pft, UT_uint32 fragOffset,
													 PT_DocPosition dpos,
													 UT_uint32 length,
													 const XML_Char ** attributes,
													 const XML_Char ** properties,
													 pf_Frag_Strux * pfs,
													 pf_Frag ** ppfNewEnd,
													 UT_uint32 * pfragOffsetNewEnd);

	UT_Bool					_fmtChangeObject(pf_Frag_Object * pfo,
											 PT_AttrPropIndex indexNewAP,
											 pf_Frag ** ppfNewEnd,
											 UT_uint32 * pfragOffsetNewEnd);

	UT_Bool					_fmtChangeObjectWithNotify(PTChangeFmt ptc,
													   pf_Frag_Object * pfo, UT_uint32 fragOffset,
													   PT_DocPosition dpos,
													   UT_uint32 length,
													   const XML_Char ** attributes,
													   const XML_Char ** properties,
													   pf_Frag_Strux * pfs,
													   pf_Frag ** ppfNewEnd,
													   UT_uint32 * pfragOffsetNewEnd);
	
	UT_Bool					_haveTempSpanFmt(PT_DocPosition * pdpos, PT_AttrPropIndex * papi) const;
	UT_Bool					_setTemporarySpanFmtWithNotify(PTChangeFmt ptc,
														   PT_DocPosition dpos,
														   const XML_Char ** attributes,
														   const XML_Char ** properties);
	void					_chooseBaseIndexAPForTempSpan(pf_Frag * pf, PT_BlockOffset fragOffset,
														  PT_AttrPropIndex * papi) const;
	

	PTState					m_pts;		/* are we loading or editing */
	pt_VarSet				m_varset;
	px_ChangeHistory		m_history;
	pf_Fragments			m_fragments;
	
	struct {
		PT_AttrPropIndex	m_indexCurrentInlineAP;
	} loading;							/* stuff only valid while m_pts==PTS_Loading */

	PD_Document *			m_pDocument; /* back pointer to our document */
};

#endif /* PT_PIECETABLE_H */
