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


#ifndef PD_DOCUMENT_H
#define PD_DOCUMENT_H

// TODO should the filename be UT_UCSChar rather than char ?

#include <stdio.h>
#include "ut_types.h"
#include "ut_vector.h"
#include "ut_alphahash.h"
#include "xad_Document.h"
#include "ut_xml.h"
#include "pt_Types.h"
#include "pl_Listener.h"
#include "ie_types.h"
#include "fp_PageSize.h"

class UT_ByteBuf;
class UT_GrowBuf;
class pt_PieceTable;
class PP_AttrProp;
class pf_Frag_Strux;
class PX_ChangeRecord;
class PD_Style;
class fd_Field;
class fl_AutoNum;
class fp_PageSize;

#ifdef PT_TEST
#include "ut_test.h"
#endif

enum
{
	PD_SIGNAL_UPDATE_LAYOUT
};

/*!
 PD_Document is the representation for a document.
*/

class PD_Document : public AD_Document
{
public:
	PD_Document();

	virtual UT_Error	       	readFromFile(const char * szFilename, int ieft);
	virtual UT_Error	       	newDocument(void);
	virtual bool			isDirty(void) const;

	virtual bool			canDo(bool bUndo) const;
	virtual bool			undoCmd(UT_uint32 repeatCount);
	virtual bool			redoCmd(UT_uint32 repeatCount);

	UT_Error   				saveAs(const char * szFilename, int ieft);
	UT_Error	       			save(void);

	void					beginUserAtomicGlob(void);
	void					endUserAtomicGlob(void);
	
	bool					insertObject(PT_DocPosition dpos,
										 PTObjectType pto,
										 const XML_Char ** attributes,
										 const XML_Char ** properties);
	bool					insertObject(PT_DocPosition dpos,
										 PTObjectType pto,
										 const XML_Char ** attributes,
										 const XML_Char ** properties, fd_Field ** pField );

	bool					insertSpan(PT_DocPosition dpos,
									   const UT_UCSChar * p,
									   UT_uint32 length,
									   PP_AttrProp *p_AttrProp = NULL);
	bool					deleteSpan(PT_DocPosition dpos1,
									   PT_DocPosition dpos2,
									   PP_AttrProp *p_AttrProp_Before = NULL);

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


	// the append- methods are only available while importing
	// the document.

	bool					appendStrux(PTStruxType pts, const XML_Char ** attributes);
	bool					appendFmt(const XML_Char ** attributes);
	bool					appendFmt(const UT_Vector * pVecAttributes);
	bool					appendSpan(UT_UCSChar * p, UT_uint32 length);
	bool					appendObject(PTObjectType pto, const XML_Char ** attributes);
	bool					appendStyle(const XML_Char ** attributes);
	
	bool					tellListener(PL_Listener * pListener);
	bool					tellListenerSubset(PL_Listener * pListener,
											   PD_DocumentRange * pDocRange);
	bool					addListener(PL_Listener * pListener, PL_ListenerId * pListenerId);
	bool					removeListener(PL_ListenerId listenerId);
	bool					signalListeners(UT_uint32 iSignal) const;
	bool					notifyListeners(pf_Frag_Strux * pfs, const PX_ChangeRecord * pcr) const;
	bool					notifyListeners(pf_Frag_Strux * pfs,
											pf_Frag_Strux * pfsNew,
											const PX_ChangeRecord * pcr) const;

	bool					getAttrProp(PT_AttrPropIndex indexAP, const PP_AttrProp ** ppAP) const;
	bool					getSpanAttrProp(PL_StruxDocHandle sdh, UT_uint32 offset, bool bLeftSide,
											const PP_AttrProp ** ppAP) const;
	const UT_UCSChar *		getPointer(PT_BufIndex bi) const; /* see warning on this function */
	bool					getSpanPtr(PL_StruxDocHandle sdh, UT_uint32 offset,
									   const UT_UCSChar ** ppSpan, UT_uint32 * pLength) const;
	bool					getBlockBuf(PL_StruxDocHandle sdh, UT_GrowBuf * pgb) const;

	bool					getBounds(bool bEnd, PT_DocPosition & docPos) const;
	PT_DocPosition			getStruxPosition(PL_StruxDocHandle sdh) const;
	bool					getStruxFromPosition(PL_ListenerId listenerId,
												 PT_DocPosition docPos,
												 PL_StruxFmtHandle * psfh) const;
	bool					getStruxOfTypeFromPosition(PL_ListenerId listenerId,
													   PT_DocPosition docPos,
													   PTStruxType pts,
													   PL_StruxFmtHandle * psfh) const;
        bool                                 getStruxOfTypeFromPosition(PT_DocPosition, PTStruxType pts, PL_StruxDocHandle * sdh) const;

bool getNextStruxOfType(PL_StruxDocHandle sdh,PTStruxType pts,
					PL_StruxDocHandle * nextsdh);

bool getPrevStruxOfType(PL_StruxDocHandle sdh,PTStruxType pts,
					PL_StruxDocHandle * prevsdh);

	// data items

	bool					createDataItem(const char * szName, bool bBase64, const UT_ByteBuf * pByteBuf,
										   void* pToken, void ** ppHandle);
	bool					getDataItemDataByName(const char * szName,
												  const UT_ByteBuf ** ppByteBuf, void** ppToken, void ** ppHandle) const;
	bool					setDataItemToken(void* pHandle, void* pToken);
	bool					getDataItemData(void * pHandle,
											const char ** pszName, const UT_ByteBuf ** ppByteBuf, void** ppToken) const;
	bool					enumDataItems(UT_uint32 k,
										  void ** ppHandle, const char ** pszName, const UT_ByteBuf ** ppByteBuf, void** ppToken) const;
	
	// styles

	bool					getStyle(const char * szName, PD_Style ** ppStyle) const;
	bool					enumStyles(UT_uint32 k,
										  const char ** pszName, const PD_Style ** ppStyle) const;

	void					clearIfAtFmtMark(PT_DocPosition dpos);

	const char *                            getFileName() { return m_szFilename; }
	UT_uint32                               getLastType() { return m_lastSavedAsType; }
        bool                 updateFields(void);
        bool                 getField(PL_StruxDocHandle sdh, 
					 UT_uint32 offset,
                                     fd_Field * &pField);
	void                    setDontChangeInsPoint(void);
	void                    allowChangeInsPoint(void);
	bool                 isPieceTableChanging(void);
        void                    notifyPieceTableChangeStart(void);
        void                    notifyPieceTableChangeEnd(void);
	
	// List Functions
	fl_AutoNum *    getListByID(UT_uint32 id) const;
	fl_AutoNum *    getNthList(UT_uint32 i) const; 
	bool		enumLists(UT_uint32 k, fl_AutoNum ** pAutoNum);
	UT_uint32       getListsCount(void) const; 
	void            addList(fl_AutoNum * pAutoNum);
	bool		appendList(const XML_Char ** attributes);
	bool		fixListHierarchy(void);
	void	 	removeList(fl_AutoNum * pAutoNum,PL_StruxDocHandle sdh );
	void            listUpdate(PL_StruxDocHandle sdh);
	void            StopList(PL_StruxDocHandle sdh);
	void            disableListUpdates(void);
	void            enableListUpdates(void);
	void            updateDirtyLists(void);
	bool         areListUpdatesAllowed(void);

	void            setDoingPaste(void);
	void            clearDoingPaste(void);
	bool         isDoingPaste(void);
	
	fp_PageSize     m_docPageSize;
	void            setDefaultPageSize(void);
	const char *    getDefaultPageSize(void);
	bool		setPageSizeFromFile(const XML_Char ** attributes);

#ifdef PT_TEST
	void					__dump(FILE * fp) const;
#endif
	
protected:
	~PD_Document();

	void					_setClean(void);
	void					_destroyDataItemData(void);
	bool                                 m_ballowListUpdates;
	pt_PieceTable *			        m_pPieceTable;
	UT_Vector				m_vecListeners;
	UT_Vector				m_vecLists;
	
	UT_AlphaHashTable		        m_hashDataItems;

	IEFileType				m_lastSavedAsType;
	bool                                 m_bPieceTableChanging;
	bool                                 m_bDoingPaste;
};


#endif /* PD_DOCUMENT_H */













