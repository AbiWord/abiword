/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

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

#ifndef PT_PIECETABLE_H
#define PT_PIECETABLE_H

#include <stdio.h>
#include <list>
#include "ut_types.h"
#include "ut_growbuf.h"
#include "ut_stack.h"
#include "pt_Types.h"
#include "pp_TableAttrProp.h"
#include "pf_Fragments.h"
#include "pt_VarSet.h"
#include "pp_Revision.h"
#include "px_ChangeHistory.h"

class pf_Frag_Object;
class pf_Frag_FmtMark;
class pf_Frag_Text;
class pf_Frag_Strux;
class pf_Frag_Strux_Block;
class pf_Frag_Strux_Section;
class PX_ChangeRecord_Span;
class PD_Style;
class PL_ListenerCoupleCloser;
class fl_ContainerLayout;

#ifdef PT_TEST
#include "ut_test.h"
#endif

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
// pt_PieceTable implements a "Piece Table" as described/suggested
// by .../dev/design/PieceTable.html

class ABI_EXPORT pt_PieceTable
{
	friend class PX_ChangeRecord;
public:
	pt_PieceTable(PD_Document * pDocument);
	~pt_PieceTable();

	void					setPieceTableState(PTState pts);
	PTState					getPieceTableState() const {return m_pts;}
	void					beginMultiStepGlob(void);
	void					endMultiStepGlob(void);
	void					beginUserAtomicGlob(void);
	void					endUserAtomicGlob(void);

	void					setClean(void);
	bool					isDirty(void) const;

	bool					canDo(bool bUndo) const;
	UT_uint32                               undoCount(bool bUndo) const;
	bool					undoCmd(void);
	bool					redoCmd(void);
	bool					getNthUndo(PX_ChangeRecord ** ppcr, UT_uint32 undoNdx) const
								{ return m_history.getNthUndo(ppcr, undoNdx); }
	bool					getUndo(PX_ChangeRecord ** ppcr, bool bStatic=false) const
								{ return m_history.getUndo(ppcr, bStatic); }
	bool					getRedo(PX_ChangeRecord ** ppcr) const
								{ return m_history.getRedo(ppcr); }
	void                    clearUndo() {m_history.clearHistory();}

    static void		s_getLocalisedStyleName(const char *szStyle, std::string &utf8);
	static const char *s_getUnlocalisedStyleName(const char *szLocStyle);

protected:
	bool					_realInsertObject(PT_DocPosition dpos,
											  PTObjectType pto,
											  const PP_PropertyVector & attributes,
											  const PP_PropertyVector & properties);

	bool					_realInsertObject(PT_DocPosition dpos,
											  PTObjectType pto,
											  const PP_PropertyVector & attributes,
											  const PP_PropertyVector & properties,
											  pf_Frag_Object ** ppfo );

	bool					_realInsertSpan(PT_DocPosition dpos,
											const UT_UCSChar * p,
											UT_uint32 length,
											const PP_PropertyVector & attributes,
											const PP_PropertyVector & properties,
											fd_Field * pField = NULL,
											bool bAddChangeRec = true);

	bool					_realDeleteSpan(PT_DocPosition dpos1,
											PT_DocPosition dpos2,
											PP_AttrProp *p_AttrProp_Before,
											bool bDeleteTableStruxes,
											bool bDontGlob=false);

#if 0
	// this is for fields and so should not be needed with revisions
	bool					_realInsertSpan_norec(PT_DocPosition dpos,
											 const UT_UCSChar * p,
											 UT_uint32 length, fd_Field * pField = NULL);
	bool                	_realDeleteFieldFrag(pf_Frag * pf);
#endif

	// this one I am not sure about
	void                	_realDeleteHdrFtrStrux(pf_Frag_Strux * pfs);

	bool					_realChangeSpanFmt(PTChangeFmt ptc,
											   PT_DocPosition dpos1,
											   PT_DocPosition dpos2,
											   const PP_PropertyVector & attributes,
											   const PP_PropertyVector & properties,
											   bool bRevisionDelete);

	bool					_realInsertStrux(PT_DocPosition dpos,
											 PTStruxType pts,
											 const PP_PropertyVector & attributes,
											 const PP_PropertyVector & properties,
											 pf_Frag_Strux ** ppfs_ret);

	bool					_realChangeStruxFmt(PTChangeFmt ptc,
												PT_DocPosition dpos1,
												PT_DocPosition dpos2,
												const gchar ** attributes,
												const gchar ** properties,
												PTStruxType pts,
												bool bRevisionDelete);

	bool                    _realChangeStruxForLists(pf_Frag_Strux* sdh,
													 const char * pszParentID,
													 bool bRevisionDelete);

    bool                    _realChangeSectionAttsNoUpdate(pf_Frag_Strux * pfStrux, const char * attr, const char * attvalue);



	/******************************************************************
	    these are the new revisions aware methods
	*/
public:
	bool					insertObject(PT_DocPosition dpos,
										 PTObjectType pto,
										 const PP_PropertyVector & attributes,
										 const PP_PropertyVector & properties);

	bool					insertObject(PT_DocPosition dpos,
										 PTObjectType pto,
										 const gchar ** attributes,
										 const gchar ** properties, pf_Frag_Object ** ppfo );

	bool					insertSpan(PT_DocPosition dpos,
									   const UT_UCSChar * p,
									   UT_uint32 length, fd_Field * pField = NULL,
									   bool bAddChangeRec = true);

    pf_Frag* getEndOfBlock( PT_DocPosition currentpos, PT_DocPosition endpos );
    // bool deleteSpanChangeTrackingAreWeMarkingDeltaMerge( PT_DocPosition startpos,
    //                                                      PT_DocPosition endpos );
    pf_Frag_Strux* inSameBlock( PT_DocPosition startpos, PT_DocPosition endpos );
    // bool changeTrackingAddParaAttribute( pf_Frag_Strux* pfs,
    //                                      const char* attr,
    //                                      std::string v );
    // bool deleteSpanChangeTrackingMaybeMarkParagraphEndDeletion( PT_DocPosition currentpos,
    //                                                             PT_DocPosition endpos );

	bool					deleteSpan(PT_DocPosition dpos1,
									   PT_DocPosition dpos2,
									   PP_AttrProp *p_AttrProp_Before,
									   UT_uint32 &iRealDeleteCount,
									   bool bDontGlob=false);


	bool					deleteSpan(PT_DocPosition dpos1,
									   PT_DocPosition dpos2,
									   PP_AttrProp *p_AttrProp_Before,
									   UT_uint32 &iRealDeleteCount,
									   bool bDeleteTableStruxes,
									   bool bDontGlob);
	bool                    createAndSendCR(PT_DocPosition  dpos,
											UT_sint32 iType,bool bSave, UT_Byte iGlob);

	bool                    createAndSendDocPropCR( const gchar ** pAtts, const gchar ** pProps);

	bool					deleteSpanWithTable(PT_DocPosition dpos1,
												PT_DocPosition dpos2,
												PP_AttrProp *p_AttrProp_Before,
												UT_uint32 &iRealDeleteCount,
												bool bDeleteTableStrux);

	bool                	deleteFieldFrag(pf_Frag * pf);

	void                	deleteHdrFtrStrux(pf_Frag_Strux * pfs);

	bool					changeSpanFmt(PTChangeFmt ptc,
										  PT_DocPosition dpos1,
										  PT_DocPosition dpos2,
										  const PP_PropertyVector & attributes,
										  const PP_PropertyVector & properties);

	bool					insertStrux(PT_DocPosition dpos,
										PTStruxType pts,
										pf_Frag_Strux ** ppfs_ret = 0);

	bool					insertStrux(PT_DocPosition dpos,
										PTStruxType pts,
										const gchar ** attributes,
										const gchar ** properties,
										pf_Frag_Strux ** ppfs_ret = 0
										);

	bool					changeStruxFmt(PTChangeFmt ptc,
										   PT_DocPosition dpos1,
										   PT_DocPosition dpos2,
										   const gchar ** attributes,
										   const gchar ** properties,
										   PTStruxType pts=PTX_StruxDummy);


	bool					changeStruxFmtNoUndo(PTChangeFmt ptc,
										   pf_Frag_Strux * pfs,
										   const gchar ** attributes,
										   const gchar ** properties);

	bool                    changeStruxFormatNoUpdate(PTChangeFmt ptc, pf_Frag_Strux * pfs,const gchar ** attributes);

	bool                    changeObjectFormatNoUpdate(PTChangeFmt ptc, pf_Frag_Object * pfo,const gchar ** attributes,const gchar ** properties);

	bool                    changeStruxForLists(pf_Frag_Strux* sdh,
												const char * pszParentID);
    bool                    changeSectionAttsNoUpdate(pf_Frag_Strux * pfStrux, const char * attr, const char * attvalue);
	bool                    deleteStruxNoUpdate(pf_Frag_Strux* sdh);
	bool                    deleteFragNoUpdate(pf_Frag * pf);
	bool                    deleteStruxWithNotify(pf_Frag_Strux* sdh);
	bool                    insertStruxNoUpdateBefore(pf_Frag_Strux* sdh, PTStruxType pts,const gchar ** attributes );
	bool                    changeLastStruxFmtNoUndo(PT_DocPosition dpos, PTStruxType pts,
													 const gchar ** attrs, const gchar ** props,
													 bool bSkipEmbededSections);

	bool                    changeLastStruxFmtNoUndo(PT_DocPosition dpos, PTStruxType pts,
													 const gchar ** attrs, const gchar * props,
													 bool bSkipEmbededSections);


	/**********************	END OF REVISIONS AWARE METHODS ******/
	bool					insertFmtMark(PTChangeFmt ptc,
										  PT_DocPosition dpos,
										  PP_AttrProp *p_AttrProp)
		{
			return _insertFmtMarkFragWithNotify(ptc,dpos,p_AttrProp);
		}
	bool                    deleteFmtMark(PT_DocPosition dpos);

	// the append- methods are only available while importing
	// the document.

	PD_Document *			getDocument(void);
	bool					appendStrux(PTStruxType pts, const PP_PropertyVector & attributes, pf_Frag_Strux ** ppfs_ret = 0);
	bool					appendStruxFmt(pf_Frag_Strux * pfs, const gchar ** attributes);
	bool                    appendLastStruxFmt(PTStruxType pts, const gchar ** attrs, const gchar ** props,
											   bool bSkipEmbededSections);
	bool                    appendLastStruxFmt(PTStruxType pts, const gchar ** attrs, const gchar * props,
											   bool bSkipEmbededSections);

	bool					appendFmt(const gchar ** attributes);
	bool					appendFmt(const PP_PropertyVector & vecAttributes);
	bool					appendSpan(const UT_UCSChar * p, UT_uint32 length);
	bool					appendObject(PTObjectType pto, const PP_PropertyVector & attributes);
	bool					appendFmtMark(void);
	bool					appendStyle(const PP_PropertyVector & attributes);

	bool					insertStruxBeforeFrag(pf_Frag * pF, PTStruxType pts,
												  const PP_PropertyVector & attributes,
                                                  pf_Frag_Strux ** ppfs_ret = 0);
	bool					insertSpanBeforeFrag(pf_Frag * pF, const UT_UCSChar * p, UT_uint32 length);
	bool					insertObjectBeforeFrag(pf_Frag * pF, PTObjectType pto,
												   const PP_PropertyVector & attributes);
	bool					insertFmtMarkBeforeFrag(pf_Frag * pF);
	bool					insertFmtMarkBeforeFrag(pf_Frag * pF, const PP_PropertyVector & attributes);

	bool					removeStyle(const gchar * name);
	size_t					getStyleCount(void) const;

	bool					tellListener(PL_Listener * pListener);
	bool					tellListenerSubset( PL_Listener * pListener,
											    PD_DocumentRange * pDocRange,
                                                PL_ListenerCoupleCloser* closer = 0 );

	bool					addListener(PL_Listener * pListener,
										PL_ListenerId listenerId);

	bool					getAttrProp(PT_AttrPropIndex indexAP,
										const PP_AttrProp ** ppAP) const;
	bool					getSpanAttrProp(pf_Frag_Strux* sdh, UT_uint32 offset, bool bLeftSide,
											const PP_AttrProp ** ppAP) const;

	inline const UT_UCSChar *getPointer(PT_BufIndex bi) const
		{
			// the pointer that we return is NOT a zero-terminated
			// string.  the caller is responsible for knowing how
			// long the data is within the span/fragment.

			return m_varset.getPointer(bi);
		}

	bool					getBlockBuf(pf_Frag_Strux* sdh, UT_GrowBuf * pgb) const;

    PT_DocPosition          getPosEnd();
	bool					getBounds(bool bEnd, PT_DocPosition & docPos) const;
	PT_DocPosition			getStruxPosition(pf_Frag_Strux* sdh) const;
	PT_DocPosition			getFragPosition(const pf_Frag * pfToFind) const;

    bool dumpDoc( const char* msg, PT_DocPosition currentpos, PT_DocPosition endpos );

	bool					getFragFromPosition(PT_DocPosition docPos,
												pf_Frag ** ppf,
												PT_BlockOffset * pOffset) const;

	bool					getStruxOfTypeFromPosition(PL_ListenerId listenerId,
													   PT_DocPosition docPos,
													   PTStruxType pts,
													   fl_ContainerLayout* * psfh) const;

    pf_Frag_Strux*       getBlockFromPosition(PT_DocPosition pos) const;

	bool					getStruxOfTypeFromPosition(PT_DocPosition docPos,
													   PTStruxType pts,
													   pf_Frag_Strux* * sdh) const;

	bool					getStruxFromPosition(PL_ListenerId listenerId,
												 PT_DocPosition docPos,
												 fl_ContainerLayout* * psfh) const;

	bool					getFragsFromPositions(PT_DocPosition dPos1, PT_DocPosition dPos2,
												  pf_Frag ** ppf1, PT_BlockOffset * pOffset1,
												  pf_Frag ** ppf2, PT_BlockOffset * pOffset2) const;

	bool					getStyle(const char * szName, PD_Style ** ppStyle) const;

	bool					enumStyles(UT_uint32 k,
									   const char ** pszName, const PD_Style ** ppStyle) const;

	bool                    enumStyles(UT_GenericVector<PD_Style*> * & pStyles) const;
	
	const std::map<std::string, PD_Style *> & getAllStyles()const {return m_hashStyles;}
	bool                    isEndFootnote(pf_Frag * pf) const;
	bool                    isFootnote(pf_Frag * pf) const;
	bool                    isInsideFootnote(PT_DocPosition dpos, pf_Frag ** pfBegin = NULL) const;
	bool                    hasEmbedStruxOfTypeInRange(PT_DocPosition posStart, PT_DocPosition posEnd, 
													   PTStruxType iType) const;

	void					clearIfAtFmtMark(PT_DocPosition dpos);
    pt_VarSet &             getVarSet(void) {return m_varset;};
    pf_Fragments &          getFragments(void) {return m_fragments;};

	bool                    purgeFmtMarks();
	bool                    isDoingTheDo(void) const
	{	return m_bDoingTheDo;}

	void                    setDoNotTweakPosition(bool b) {m_bDoNotTweakPosition = b;}

	UT_uint32               getXID();
	UT_uint32               getTopXID() const {return m_iXID;}
	void                    setXIDThreshold(UT_uint32 i){m_iXID = i;}
	void                    fixMissingXIDs();
	UT_sint32               calcDocsize(void);
	void                    setCRNumber(UT_sint32 iCRNumber)
	{ m_iCurCRNumber = iCRNumber;}
#ifdef PT_TEST
	UT_TestStatus			__test_VerifyCoalescedFrags(FILE * fp) const;
	void					__dump(FILE * fp) const;
	px_ChangeHistory*		getChangeHistory(void) const
		{ return &m_history; }
#endif /* PT_TEST */

protected:

    pf_Frag_Strux*          _findLastStruxOfType(pf_Frag * pfStart,
                                                 PTStruxType pst,
                                                 PTStruxType* stopConditions,
                                                 bool bSkipEmbededSections );
    pf_Frag_Strux*          _findLastStruxOfType(pf_Frag * pfStart, PTStruxType pst, bool bSkipEmbeded);
	pf_Frag *               _findPrevHyperlink(pf_Frag * pfStart);
	pf_Frag *               _findNextHyperlink(pf_Frag * pfStart);

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
	bool                    _StruxIsNotTable(pf_Frag_Strux * pfs);
	bool					_deleteSpan(pf_Frag_Text * pft, UT_uint32 fragOffset,
										PT_BufIndex bi, UT_uint32 length,
										pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);
	void					_unlinkFrag(pf_Frag * pf,
										pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);
	bool					_getStruxFromPosition(PT_DocPosition docPos,
												  pf_Frag_Strux ** ppfs, bool bSkipFootnotes = false) const;
	bool					_getStruxOfTypeFromPosition(PT_DocPosition dpos,
														PTStruxType pts,
														pf_Frag_Strux ** ppfs) const;
    pf_Frag_Strux*          _getBlockFromPosition(PT_DocPosition pos) const;
    bool					_doTheDo(const PX_ChangeRecord * pcr, bool bUndo);
	bool					_struxHasContent(pf_Frag_Strux * pfs) const;
	bool					_struxIsEmpty(pf_Frag_Strux * pfs) const;
	bool					_unlinkStrux(pf_Frag_Strux * pfs,
										 pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);
	bool					_unlinkStrux_Block(pf_Frag_Strux * pfs,
											   pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);
	bool					_unlinkStrux_Section(pf_Frag_Strux * pfs,
												 pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);

	bool					_canCoalesceDeleteSpan(PX_ChangeRecord_Span * pcrSpan) const;

	bool					_deleteSpanWithNotify(PT_DocPosition dpos,
												  pf_Frag_Text * pft, UT_uint32 fragOffset,
												  UT_uint32 length,
												  pf_Frag_Strux * pfs,
												  pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd,
												  bool bAddChangeRec = true);

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

    bool                     _deleteComplexSpanHAR( pf_Frag_Object *pO,
                                                    PT_DocPosition dpos1,
                                                    PT_DocPosition dpos2,
                                                    UT_uint32& length,
                                                    PT_BlockOffset& fragOffset_First,
                                                    UT_uint32& lengthThisStep,
                                                    pf_Frag_Strux*& pfsContainer,
                                                    pf_Frag*& pfNewEnd,
                                                    UT_uint32& fragOffsetNewEnd,
                                                    const char* startAttrCSTR );

	bool					_deleteComplexSpan(PT_DocPosition & dpos1,
											   PT_DocPosition & dpos2,
											   UT_Stack *stDelayStruxDelete);


	bool					_deleteComplexSpan_norec(PT_DocPosition dpos1,
													 PT_DocPosition dpos2);

	bool					_deleteObject(pf_Frag_Object * pfo,
										  pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);

	bool					_deleteObjectWithNotify(PT_DocPosition dpos,
													pf_Frag_Object * pfo, UT_uint32 fragOffset,
													UT_uint32 length,
													pf_Frag_Strux * pfs,
													pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd,
                                                    bool bAddChangeRec = true);



	bool					_deleteObject_norec(PT_DocPosition dpos,
												pf_Frag_Object * pfo, UT_uint32 fragOffset,
												UT_uint32 length,
												pf_Frag_Strux * pfs,
												pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd);

	bool					_deleteStruxWithNotify(PT_DocPosition dpos,
												   pf_Frag_Strux * pfs,
												   pf_Frag ** ppfEnd,
												   UT_uint32 * pfragOffsetEnd,
												   bool bWithRec = true);

	bool                    _deleteHdrFtrsFromSectionStruxIfPresent(pf_Frag_Strux_Section * pfStruxSec);

	void                    _deleteHdrFtrStruxWithNotify( pf_Frag_Strux * pfFragStruxHdrFtr);

	bool                    _fixHdrFtrReferences(const gchar * pType, const gchar * pId, bool bNotional = false);

	bool					_fmtChangeStrux(pf_Frag_Strux * pfs,
											PT_AttrPropIndex indexNewAP);

	bool					_fmtChangeStruxWithNotify(PTChangeFmt ptc,
													  pf_Frag_Strux * pfs,
													  const gchar ** attributes,
													  const gchar ** properties,
													  bool bRevisionDelete);

	bool					_fmtChangeStruxWithNotify(PTChangeFmt ptc,
													  pf_Frag_Strux * pfs,
													  const gchar ** attributes,
													  const gchar ** properties,
													  bool bDoAll,
													  bool bRevisionDelete);

	bool					_fmtChangeSpan(pf_Frag_Text * pft, UT_uint32 fragOffset, UT_uint32 length,
										   PT_AttrPropIndex indexNewAP,
										   pf_Frag ** ppfNewEnd, UT_uint32 * pfragOffsetNewEnd);

	bool					_fmtChangeSpanWithNotify(PTChangeFmt ptc,
													 pf_Frag_Text * pft, UT_uint32 fragOffset,
													 PT_DocPosition dpos,
													 UT_uint32 length,
													 const PP_PropertyVector & attributes,
													 const PP_PropertyVector & properties,
													 pf_Frag_Strux * pfs,
													 pf_Frag ** ppfNewEnd,
													 UT_uint32 * pfragOffsetNewEnd,
													 bool bRevisionDelete);

	bool					_fmtChangeObject(pf_Frag_Object * pfo,
											 PT_AttrPropIndex indexNewAP,
											 pf_Frag ** ppfNewEnd,
											 UT_uint32 * pfragOffsetNewEnd);

	bool					_fmtChangeObjectWithNotify(PTChangeFmt ptc,
													   pf_Frag_Object * pfo, UT_uint32 fragOffset,
													   PT_DocPosition dpos,
													   UT_uint32 length,
													   const PP_PropertyVector & attributes,
													   const PP_PropertyVector & properties,
													   pf_Frag_Strux * pfs,
													   pf_Frag ** ppfNewEnd,
													   UT_uint32 * pfragOffsetNewEnd,
													   bool bRevisionDelete);

	bool					_getStruxFromFrag(pf_Frag * pfStart, pf_Frag_Strux ** ppfs) const;
	bool					_getStruxFromFragSkip(pf_Frag * pfStart, pf_Frag_Strux ** ppfs) const;

	bool                    _getNextStruxAfterFragSkip(pf_Frag *pfStart, pf_Frag_Strux ** ppfs);

	bool                    _getStruxFromPositionSkip(PT_DocPosition docPos,
													  pf_Frag_Strux ** ppfs) const;

	UT_uint32				_computeBlockOffset(pf_Frag_Strux * pfs,pf_Frag * pfTarget) const;

	bool					_loadBuiltinStyles(void);
	bool					_createBuiltinStyle(const char * szName, bool bDisplayed, const gchar ** attributes);

	bool					_insertFmtMarkFragWithNotify(PTChangeFmt ptc,
														 PT_DocPosition dpos,
														 const PP_PropertyVector & attributes,
														 const PP_PropertyVector & properties);
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
														const PP_PropertyVector & attributes,
														const PP_PropertyVector & properties,
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
	bool                    _checkSkipFootnote(PT_DocPosition dpos1, PT_DocPosition dpos2, pf_Frag * pf_End = NULL) const;
	// helper methods for the appned and insert*BeforeFrag methods
	bool					_makeStrux(PTStruxType pts, const PP_PropertyVector & attributes,
									   pf_Frag_Strux * &pfs);
	bool					_makeObject(PTObjectType pto, const PP_PropertyVector & attributes,
										pf_Frag_Object * &pfo);
	bool					_makeFmtMark(pf_Frag_FmtMark * &pff);
	bool                    _makeFmtMark(pf_Frag_FmtMark * &pff, const PP_PropertyVector & attributes);
	UT_sint32               _getNextChangeRecordNumber(void)
	{ return m_iCurCRNumber++;}
	// implemented in pt_PT_InsertStrux.cpp
	bool                    _translateRevisionAttribute(PP_RevisionAttr & Revisions, PT_AttrPropIndex indexAP,
														PP_RevisionType eType,
														PP_PropertyVector & ppRevAttrib,
														PP_PropertyVector & ppRevProps,
														const PP_PropertyVector & ppAttrib,
														const PP_PropertyVector & ppProps);
	bool                    _insertNoteInEmbeddedStruxList(pf_Frag_Strux * pfsNew);

	PTState					m_pts;		/* are we loading or editing */
	pt_VarSet				m_varset;
	px_ChangeHistory		m_history;
	pf_Fragments			m_fragments;
	typedef std::map<std::string, PD_Style *> StyleMap;
	StyleMap m_hashStyles;

	struct {
		PT_AttrPropIndex	m_indexCurrentInlineAP;
	} loading;							/* stuff only valid while m_pts==PTS_Loading */

	PD_Document *			m_pDocument; /* back pointer to our document */

	UT_uint32               m_atomicGlobCount;
	bool                    m_bDoingTheDo;
	bool                    m_bDoNotTweakPosition;

	UT_uint32               m_iXID;
	UT_sint32               m_iCurCRNumber;
	struct embeddedStrux {
		pf_Frag_Strux * beginNote;
		pf_Frag_Strux * endNote;
		PTStruxType type;
	};

	std::list <embeddedStrux> m_embeddedStrux;
};

#endif /* PT_PIECETABLE_H */
