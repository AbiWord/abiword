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
#include "ut_alphahash.h"
#include "ut_stack.h"
#include "pt_Types.h"
#include "pp_TableAttrProp.h"
#include "px_ChangeHistory.h"
#include "pf_Fragments.h"
#include "pt_VarSet.h"
class pf_Frag_Object;
class pf_Frag_FmtMark;
class pf_Frag_Text;
class pf_Frag_Strux;
class pf_Frag_Strux_Block;
class PX_ChangeRecord_Span;
class PD_Style;

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
	bool					isDirty(void) const;
	
	bool					canDo(bool bUndo) const;
	bool					undoCmd(void);
	bool					redoCmd(void);

	bool					insertObject(PT_DocPosition dpos,
										 PTObjectType pto,
										 const XML_Char ** attributes,
										 const XML_Char ** properties);

	bool					insertObject(PT_DocPosition dpos,
										 PTObjectType pto,
										 const XML_Char ** attributes,
										 const XML_Char ** properties, pf_Frag_Object ** ppfo );

	bool					insertSpan(PT_DocPosition dpos,
									   const UT_UCSChar * p,
									   UT_uint32 length, fd_Field * pField = NULL);

	bool					deleteSpan(PT_DocPosition dpos1,
									   PT_DocPosition dpos2,
									   PP_AttrProp *p_AttrProp_Before);
	bool					insertSpan_norec(PT_DocPosition dpos,
											 const UT_UCSChar * p,
											 UT_uint32 length, fd_Field * pField = NULL);

	bool                	deleteFieldFrag(pf_Frag * pf);
	bool					changeSpanFmt(PTChangeFmt ptc,
										  PT_DocPosition dpos1,
										  PT_DocPosition dpos2,
										  const XML_Char ** attributes,
										  const XML_Char ** properties);

	bool					insertStrux(PT_DocPosition dpos,
										PTStruxType pts);

	bool					changeStruxFmt(PTChangeFmt ptc,
										   PT_DocPosition dpos1,
										   PT_DocPosition dpos2,
										   const XML_Char ** attributes,
										   const XML_Char ** properties,
										   PTStruxType pts);

	bool					insertFmtMark(PTChangeFmt ptc,
										  PT_DocPosition dpos,
										  PP_AttrProp *p_AttrProp)
		{
			return _insertFmtMarkFragWithNotify(ptc,dpos,p_AttrProp);
		}
	// the append- methods are only available while importing
	// the document.

	PD_Document *			getDocument(void);
	bool					appendStrux(PTStruxType pts, const XML_Char ** attributes);
	bool					appendFmt(const XML_Char ** attributes);
	bool					appendFmt(const UT_Vector * pVecAttributes);
	bool					appendSpan(UT_UCSChar * p, UT_uint32 length);
	bool					appendObject(PTObjectType pto, const XML_Char ** attributes);
	bool					appendFmtMark(void);
	bool					appendStyle(const XML_Char ** attributes);
	bool					removeStyle(const XML_Char * name);
	size_t					getStyleCount(void);

	bool					tellListener(PL_Listener * pListener);
	bool					tellListenerSubset(PL_Listener * pListener,
											   PD_DocumentRange * pDocRange);
	
	bool					addListener(PL_Listener * pListener,
										PL_ListenerId listenerId);
	
	bool					getAttrProp(PT_AttrPropIndex indexAP,
										const PP_AttrProp ** ppAP) const;
	bool					getSpanAttrProp(PL_StruxDocHandle sdh, UT_uint32 offset, bool bLeftSide,
											const PP_AttrProp ** ppAP) const;

	inline const UT_UCSChar *getPointer(PT_BufIndex bi) const
		{
			// the pointer that we return is NOT a zero-terminated
			// string.  the caller is responsible for knowing how
			// long the data is within the span/fragment.

			return m_varset.getPointer(bi);
		}

	bool					getSpanPtr(PL_StruxDocHandle sdh, UT_uint32 offset,
									   const UT_UCSChar ** ppSpan, UT_uint32 * pLength) const;
	bool					getBlockBuf(PL_StruxDocHandle sdh, UT_GrowBuf * pgb) const;

	bool					getBounds(bool bEnd, PT_DocPosition & docPos) const;
	PT_DocPosition			getStruxPosition(PL_StruxDocHandle sdh) const;
	PT_DocPosition			getFragPosition(const pf_Frag * pfToFind) const;
	
	bool					getFragFromPosition(PT_DocPosition docPos,
												pf_Frag ** ppf,
												PT_BlockOffset * pOffset) const;

	bool					getStruxOfTypeFromPosition(PL_ListenerId listenerId,
													   PT_DocPosition docPos,
													   PTStruxType pts,
													   PL_StruxFmtHandle * psfh) const;


	bool					getStruxOfTypeFromPosition(PT_DocPosition docPos,   
													   PTStruxType pts,  
													   PL_StruxDocHandle * sdh) const;

	bool					getStruxFromPosition(PL_ListenerId listenerId,
												 PT_DocPosition docPos,
												 PL_StruxFmtHandle * psfh) const;

	bool					getFragsFromPositions(PT_DocPosition dPos1, PT_DocPosition dPos2,
												  pf_Frag ** ppf1, PT_BlockOffset * pOffset1,
												  pf_Frag ** ppf2, PT_BlockOffset * pOffset2) const;
	
	bool					getStyle(const char * szName, PD_Style ** ppStyle) const;
	bool					enumStyles(UT_uint32 k,
									   const char ** pszName, const PD_Style ** ppStyle) const;

	void					clearIfAtFmtMark(PT_DocPosition dpos);

    pt_VarSet &             getVarSet(void) {return m_varset;};
    pf_Fragments &          getFragments(void) {return m_fragments;};
	
#ifdef PT_TEST
	UT_TestStatus			__test_VerifyCoalescedFrags(FILE * fp) const;
	void					__dump(FILE * fp) const;
	px_ChangeHistory*		getChangeHistory(void)
		{ return &m_history; }

#endif /* PT_TEST */
	
protected:

	bool					_tellAndMaybeAddListener(PL_Listener * pListener,
													 PL_ListenerId listenerId,
													 bool bAdd);
	
	void					_captureActiveSpan(pf_Frag_Strux_Block * pfsBlock);
	PT_AttrPropIndex		_chooseIndexAP(pf_Frag * pf, PT_BlockOffset fragOffset);
	bool					_canCoalesceInsertSpan(PX_ChangeRecord_Span * pcrSpan) const;

	bool					_createStrux(PTStruxType pts,
										 PT_AttrPropIndex indexAP,
										 pf_Frag_Strux ** ppfs);

	void					_insertStrux(pf_Frag * pf,
										 PT_BlockOffset fragOffset,
										 pf_Frag_Strux * pfsNew);

	bool					_insertObject(pf_Frag * pf,
										  PT_BlockOffset fragOffset,									 
										  PTObjectType pto,
										  PT_AttrPropIndex indexAP,
                                          pf_Frag_Object * &pfo);
	
	bool					_createObject(PTObjectType pto,
										  PT_AttrPropIndex indexAP,
										  pf_Frag_Object ** ppfo);
	
	bool					_insertSpan(pf_Frag * pf,
										PT_BufIndex bi,
										PT_BlockOffset fragOffset,
										UT_uint32 length,
										PT_AttrPropIndex indexAP,
                                        fd_Field * pField = NULL);

	bool					_deleteSpan(pf_Frag_Text * pft, UT_uint32 fragOffset,
										PT_BufIndex bi, UT_uint32 length,
										pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);
	void					_unlinkFrag(pf_Frag * pf,
										pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);
	bool					_getStruxFromPosition(PT_DocPosition docPos,
												  pf_Frag_Strux ** ppfs) const;
	bool					_getStruxOfTypeFromPosition(PT_DocPosition dpos,
														PTStruxType pts,
														pf_Frag_Strux ** ppfs) const;
	bool					_doTheDo(const PX_ChangeRecord * pcr, bool bUndo);
	bool					_struxHasContent(pf_Frag_Strux * pfs) const;
	bool					_unlinkStrux(pf_Frag_Strux * pfs,
										 pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);
	bool					_unlinkStrux_Block(pf_Frag_Strux * pfs,
											   pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);
	bool					_unlinkStrux_Section(pf_Frag_Strux * pfs,
												 pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);

	bool					_deleteSpan_norec(PT_DocPosition dpos,
											  pf_Frag_Text * pft, UT_uint32 fragOffset,
											  UT_uint32 length,
											  pf_Frag_Strux * pfs,
											  pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);
	bool					_canCoalesceDeleteSpan(PX_ChangeRecord_Span * pcrSpan) const;

	bool					_deleteSpanWithNotify(PT_DocPosition dpos,
												  pf_Frag_Text * pft, UT_uint32 fragOffset,
												  UT_uint32 length,
												  pf_Frag_Strux * pfs,
												  pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);


	bool					_isSimpleDeleteSpan(PT_DocPosition dpos1,
												PT_DocPosition dpos2) const;
    void					_tweakFieldSpan(PT_DocPosition& dpos1,
                                            PT_DocPosition& dpos2) const;
	bool					_tweakDeleteSpanOnce(PT_DocPosition& dpos1, 
												 PT_DocPosition& dpos2,
												 UT_Stack * pstDelayStruxDelete) const;
	bool					_tweakDeleteSpan(PT_DocPosition& dpos1, 
											 PT_DocPosition& dpos2,
											 UT_Stack * pstDelayStruxDelete) const;
	bool					_deleteFormatting(PT_DocPosition dpos1,
											  PT_DocPosition dpos2);

	bool					_deleteComplexSpan(PT_DocPosition dpos1,
											   PT_DocPosition dpos2);


	bool					_deleteComplexSpan_norec(PT_DocPosition dpos1,
													 PT_DocPosition dpos2);
	
	bool					_deleteObject(pf_Frag_Object * pfo,
										  pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);
	
	bool					_deleteObjectWithNotify(PT_DocPosition dpos,
													pf_Frag_Object * pfo, UT_uint32 fragOffset,
													UT_uint32 length,
													pf_Frag_Strux * pfs,
													pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);


	
	bool					_deleteObject_norec(PT_DocPosition dpos,
												pf_Frag_Object * pfo, UT_uint32 fragOffset,
												UT_uint32 length,
												pf_Frag_Strux * pfs,
												pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);

	bool					_deleteStruxWithNotify(PT_DocPosition dpos,
												   pf_Frag_Strux * pfs,
												   pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);

	bool					_deleteStrux_norec(PT_DocPosition dpos,
											   pf_Frag_Strux * pfs,
											   pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);


	bool					_fmtChangeStrux(pf_Frag_Strux * pfs,
											PT_AttrPropIndex indexNewAP);
	
	bool					_fmtChangeStruxWithNotify(PTChangeFmt ptc,
													  pf_Frag_Strux * pfs,
													  const XML_Char ** attributes,
													  const XML_Char ** properties);

	bool					_fmtChangeSpan(pf_Frag_Text * pft, UT_uint32 fragOffset, UT_uint32 length,
										   PT_AttrPropIndex indexNewAP,
										   pf_Frag ** ppfNewEnd, UT_uint32 * pfragOffsetNewEnd);
	
	bool					_fmtChangeSpanWithNotify(PTChangeFmt ptc,
													 pf_Frag_Text * pft, UT_uint32 fragOffset,
													 PT_DocPosition dpos,
													 UT_uint32 length,
													 const XML_Char ** attributes,
													 const XML_Char ** properties,
													 pf_Frag_Strux * pfs,
													 pf_Frag ** ppfNewEnd,
													 UT_uint32 * pfragOffsetNewEnd);

	bool					_fmtChangeObject(pf_Frag_Object * pfo,
											 PT_AttrPropIndex indexNewAP,
											 pf_Frag ** ppfNewEnd,
											 UT_uint32 * pfragOffsetNewEnd);

	bool					_fmtChangeObjectWithNotify(PTChangeFmt ptc,
													   pf_Frag_Object * pfo, UT_uint32 fragOffset,
													   PT_DocPosition dpos,
													   UT_uint32 length,
													   const XML_Char ** attributes,
													   const XML_Char ** properties,
													   pf_Frag_Strux * pfs,
													   pf_Frag ** ppfNewEnd,
													   UT_uint32 * pfragOffsetNewEnd);
	
	bool					_getStruxFromFrag(pf_Frag * pfStart, pf_Frag_Strux ** ppfs) const;
	UT_uint32				_computeBlockOffset(pf_Frag_Strux * pfs,pf_Frag * pfTarget) const;

	bool					_loadBuiltinStyles(void);
	bool					_createBuiltinStyle(const char * szName, const XML_Char ** attributes);

	bool					_insertFmtMarkFragWithNotify(PTChangeFmt ptc,
														 PT_DocPosition dpos,
														 const XML_Char ** attributes,
														 const XML_Char ** properties);
	bool					_insertFmtMarkFragWithNotify(PTChangeFmt ptc,
														 PT_DocPosition dpos,
														 PP_AttrProp *p_AttrProp);
	bool					_insertFmtMark(pf_Frag * pf, UT_uint32 fragOffset, PT_AttrPropIndex api);
	bool					_insertFmtMarkAfterBlockWithNotify(pf_Frag_Strux * pfsBlock,
															   PT_DocPosition dpos,
															   PT_AttrPropIndex api);
	bool					_deleteFmtMarkWithNotify(PT_DocPosition dpos, pf_Frag_FmtMark * pffm,
													 pf_Frag_Strux * pfs,
													 pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);
	bool					_deleteFmtMark(pf_Frag_FmtMark * pffm,
										   pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);
	bool					_fmtChangeFmtMarkWithNotify(PTChangeFmt ptc, pf_Frag_FmtMark * pffm,
														PT_DocPosition dpos, 
														const XML_Char ** attributes, const XML_Char ** properties,
														pf_Frag_Strux * pfs,
														pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);
	bool					_fmtChangeFmtMark(pf_Frag_FmtMark * pffm,
											  PT_AttrPropIndex indexNewAP,
											  pf_Frag ** ppfNewEnd,
											  UT_uint32 * pfragOffsetNewEnd);
	bool					_computeFmtMarkForNewBlock(pf_Frag_Strux * pfsBlock,
													   pf_Frag * pfCurrent, PT_BlockOffset fragOffset,
													   PT_AttrPropIndex * pFmtMarkAP);
	bool					_getSpanAttrPropHelper(pf_Frag * pf, const PP_AttrProp ** ppAP) const;
	bool					_lastUndoIsThisFmtMark(PT_DocPosition dpos);

	bool					_changePointWithNotify(PT_DocPosition dpos);

	PTState					m_pts;		/* are we loading or editing */
	pt_VarSet				m_varset;
	px_ChangeHistory		m_history;
	pf_Fragments			m_fragments;
	UT_AlphaHashTable		m_hashStyles;
	
	struct {
		PT_AttrPropIndex	m_indexCurrentInlineAP;
	} loading;							/* stuff only valid while m_pts==PTS_Loading */

	PD_Document *			m_pDocument; /* back pointer to our document */
};

#endif /* PT_PIECETABLE_H */
