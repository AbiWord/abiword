
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
									   UT_Bool bLeftSide,
									   UT_UCSChar * p,
									   UT_uint32 length);

	UT_Bool					deleteSpan(PT_DocPosition dpos,
									   UT_Bool bLeftSide1,
									   UT_Bool bLeftSide2,
									   UT_uint32 length);

	UT_Bool					changeSpanFmt(PTChangeFmt ptc,
										  PT_DocPosition dpos1,
										  UT_Bool bLeftSide1,
										  PT_DocPosition dpos2,
										  UT_Bool bLeftSide2,
										  const XML_Char ** attributes,
										  const XML_Char ** properties);

	UT_Bool					insertStrux(PT_DocPosition dpos,
										UT_Bool bLeftSide,
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
	
	UT_Bool					getStruxFromPosition(PL_ListenerId listenerId,
												 PT_DocPosition docPos,
												 PL_StruxFmtHandle * psfh) const;
	UT_Bool					getTextFragFromPosition(PT_DocPosition docPos,
													UT_Bool bLeftSide,
													pf_Frag_Strux ** ppfs,
													pf_Frag_Text ** ppft,
													PT_BlockOffset * pOffset) const;

	// TODO add stuff for objects like in-line images.

	void					dump(FILE * fp) const;
	
protected:

	UT_Bool					_createStrux(PTStruxType pts,
										 PT_AttrPropIndex indexAP,
										 pf_Frag_Strux ** ppfs);

	void					_insertStrux(pf_Frag_Strux * pfsPrev,
										 pf_Frag_Text * pft,
										 PT_BlockOffset fragOffset,
										 UT_Bool bLeftSide,
										 pf_Frag_Strux * pfsNew);

	UT_Bool					_insertSpan(pf_Frag_Text * pft,
										PT_BufIndex bi,
										UT_Bool bLeftSide,
										PT_BlockOffset fragOffset,
										UT_uint32 length);

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

	UT_Bool					_deleteSpanWithNotify(PT_DocPosition dpos, UT_Bool bLeftSide,
												  pf_Frag_Text * pft, UT_uint32 fragOffset,
												  UT_uint32 length,
												  pf_Frag_Strux * pfs,
												  pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);
	UT_Bool					_deleteStruxWithNotify(PT_DocPosition dpos, UT_Bool bLeftSide,
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
