 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
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
class pf_Frag_Text;
class pf_Frag_Strux;


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
	
	UT_Bool					undoCmd(void);
	UT_Bool					redoCmd(void);

	UT_Bool					insertSpan(PT_DocPosition dpos,
									   UT_UCSChar * p,
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

	UT_Bool					addListener(PL_Listener * pListener,
										PL_ListenerId listenerId);
	
	UT_Bool					getAttrProp(PT_AttrPropIndex indexAP,
										const PP_AttrProp ** ppAP) const;
	UT_Bool					getSpanAttrProp(PL_StruxDocHandle sdh, UT_uint32 offset,
											const PP_AttrProp ** ppAP) const;

	const UT_UCSChar *		getPointer(PT_BufIndex bi) const;
	UT_Bool					getSpanPtr(PL_StruxDocHandle sdh, UT_uint32 offset,
									   const UT_UCSChar ** ppSpan, UT_uint32 * pLength) const;

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

	void					dump(FILE * fp) const;
	
protected:

	UT_Bool					_createStrux(PTStruxType pts,
										 PT_AttrPropIndex indexAP,
										 pf_Frag_Strux ** ppfs);

	void					_insertStrux(pf_Frag * pf,
										 PT_BlockOffset fragOffset,
										 pf_Frag_Strux * pfsNew);

	PT_Differences			_isDifferentFmt(pf_Frag * pf, UT_uint32 fragOffset, PT_AttrPropIndex indexAP);
	
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
	UT_Bool					_doTheDo(const PX_ChangeRecord * pcr);
	UT_Bool					_struxHasContent(pf_Frag_Strux * pfs) const;
	UT_Bool					_unlinkStrux_Block(pf_Frag_Strux * pfs,
											   pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);

	UT_Bool					_deleteSpanWithNotify(PT_DocPosition dpos,
												  pf_Frag_Text * pft, UT_uint32 fragOffset,
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
	
	void					_setTemporarySpanFmt(PT_AttrPropIndex indexNewAP,
												 PT_DocPosition dpos);

	UT_Bool					_setTemporarySpanFmtWithNotify(PTChangeFmt ptc,
														   PT_DocPosition dpos,
														   const XML_Char ** attributes,
														   const XML_Char ** properties);
	

	PTState					m_pts;		/* are we loading or editing */
	pt_VarSet				m_varset;
	px_ChangeHistory		m_history;
	pf_Fragments			m_fragments;
	UT_Bool					m_bHaveTemporarySpanFmt;
	PT_AttrPropIndex		m_indexAPTemporarySpanFmt;
	PT_DocPosition			m_dposTemporarySpanFmt;
	
	struct {
		PT_AttrPropIndex	m_indexCurrentInlineAP;
	} loading;							/* stuff only valid while m_pts==PTS_Loading */

	PD_Document *			m_pDocument; /* back pointer to our document */
};

#endif /* PT_PIECETABLE_H */
