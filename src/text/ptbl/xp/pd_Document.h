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
#ifdef HAVE_GNOME_XML2
#include <libxml/parser.h>
#else
#include "xmlparse.h"
#endif
#include "pt_Types.h"
#include "pl_Listener.h"
#include "ie_types.h"

class UT_ByteBuf;
class UT_GrowBuf;
class pt_PieceTable;
class PP_AttrProp;
class pf_Frag_Strux;
class PX_ChangeRecord;
class PD_Style;
class fd_Field;
class fl_AutoNum;


#ifdef PT_TEST
#include "ut_test.h"
#endif

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
// PD_Document is the representation for a document.

enum
{
	PD_SIGNAL_UPDATE_LAYOUT
};

class PD_Document : public AD_Document
{
public:
	PD_Document();

	virtual UT_Error	       	readFromFile(const char * szFilename, int ieft);
	virtual UT_Error	       	newDocument(void);
	virtual UT_Bool			isDirty(void) const;

	virtual UT_Bool			canDo(UT_Bool bUndo) const;
	virtual UT_Bool			undoCmd(UT_uint32 repeatCount);
	virtual UT_Bool			redoCmd(UT_uint32 repeatCount);

	UT_Error   				saveAs(const char * szFilename, int ieft);
	UT_Error	       			save(void);

	void					beginUserAtomicGlob(void);
	void					endUserAtomicGlob(void);
	
	UT_Bool					insertObject(PT_DocPosition dpos,
										 PTObjectType pto,
										 const XML_Char ** attributes,
										 const XML_Char ** properties);
	UT_Bool					insertObject(PT_DocPosition dpos,
										 PTObjectType pto,
										 const XML_Char ** attributes,
										 const XML_Char ** properties, fd_Field ** pField );

	UT_Bool					insertSpan(PT_DocPosition dpos,
									   const UT_UCSChar * p,
									   UT_uint32 length,
									   PP_AttrProp *p_AttrProp = NULL);
	UT_Bool					deleteSpan(PT_DocPosition dpos1,
									   PT_DocPosition dpos2,
									   PP_AttrProp *p_AttrProp_Before = NULL);

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
	UT_Bool					appendStyle(const XML_Char ** attributes);
	
	UT_Bool					tellListener(PL_Listener * pListener);
	UT_Bool					tellListenerSubset(PL_Listener * pListener,
											   PD_DocumentRange * pDocRange);
	UT_Bool					addListener(PL_Listener * pListener, PL_ListenerId * pListenerId);
	UT_Bool					removeListener(PL_ListenerId listenerId);
	UT_Bool					signalListeners(UT_uint32 iSignal) const;
	UT_Bool					notifyListeners(pf_Frag_Strux * pfs, const PX_ChangeRecord * pcr) const;
	UT_Bool					notifyListeners(pf_Frag_Strux * pfs,
											pf_Frag_Strux * pfsNew,
											const PX_ChangeRecord * pcr) const;

	UT_Bool					getAttrProp(PT_AttrPropIndex indexAP, const PP_AttrProp ** ppAP) const;
	UT_Bool					getSpanAttrProp(PL_StruxDocHandle sdh, UT_uint32 offset, UT_Bool bLeftSide,
											const PP_AttrProp ** ppAP) const;
	const UT_UCSChar *		getPointer(PT_BufIndex bi) const; /* see warning on this function */
	UT_Bool					getSpanPtr(PL_StruxDocHandle sdh, UT_uint32 offset,
									   const UT_UCSChar ** ppSpan, UT_uint32 * pLength) const;
	UT_Bool					getBlockBuf(PL_StruxDocHandle sdh, UT_GrowBuf * pgb) const;

	UT_Bool					getBounds(UT_Bool bEnd, PT_DocPosition & docPos) const;
	PT_DocPosition			getStruxPosition(PL_StruxDocHandle sdh) const;
	UT_Bool					getStruxFromPosition(PL_ListenerId listenerId,
												 PT_DocPosition docPos,
												 PL_StruxFmtHandle * psfh) const;
	UT_Bool					getStruxOfTypeFromPosition(PL_ListenerId listenerId,
													   PT_DocPosition docPos,
													   PTStruxType pts,
													   PL_StruxFmtHandle * psfh) const;

	// data items

	UT_Bool					createDataItem(const char * szName, UT_Bool bBase64, const UT_ByteBuf * pByteBuf,
										   void* pToken, void ** ppHandle);
	UT_Bool					getDataItemDataByName(const char * szName,
												  const UT_ByteBuf ** ppByteBuf, void** ppToken, void ** ppHandle) const;
	UT_Bool					setDataItemToken(void* pHandle, void* pToken);
	UT_Bool					getDataItemData(void * pHandle,
											const char ** pszName, const UT_ByteBuf ** ppByteBuf, void** ppToken) const;
	UT_Bool					enumDataItems(UT_uint32 k,
										  void ** ppHandle, const char ** pszName, const UT_ByteBuf ** ppByteBuf, void** ppToken) const;
	
	// styles

	UT_Bool					getStyle(const char * szName, PD_Style ** ppStyle) const;
	UT_Bool					enumStyles(UT_uint32 k,
										  const char ** pszName, const PD_Style ** ppStyle) const;

	void					clearIfAtFmtMark(PT_DocPosition dpos);

	const char *                            getFileName() { return m_szFilename; }
	UT_uint32                               getLastType() { return m_lastSavedAsType; }
        UT_Bool                 updateFields(void);
        UT_Bool                 getField(PL_StruxDocHandle sdh, 
					 UT_uint32 offset,
                                     fd_Field * &pField);
	void                    setDontChangeInsPoint(void);
	void                    allowChangeInsPoint(void);
	
	// List Functions
	fl_AutoNum *    getListByID(UT_uint32 id) const;
	fl_AutoNum *    getNthList(UT_uint32 i) const; 
	UT_Bool		enumLists(UT_uint32 k, fl_AutoNum ** pAutoNum);
	UT_uint32       getListsCount(void) const; 
	void            addList(fl_AutoNum * pAutoNum);
	UT_Bool		appendList(const XML_Char ** attributes);
	UT_Bool		fixListHierarchy(void);
	void	 	removeList(fl_AutoNum * pAutoNum);
	void            listUpdate(PL_StruxDocHandle sdh);
	void            StopList(PL_StruxDocHandle sdh);
	void            disableListUpdates(void);
	void            enableListUpdates(void);
	void            updateDirtyLists(void);
#ifdef PT_TEST
	void					__dump(FILE * fp) const;
#endif
	
protected:
	~PD_Document();

	void					_setClean(void);
	void					_destroyDataItemData(void);

	pt_PieceTable *			        m_pPieceTable;
	UT_Vector				m_vecListeners;
	UT_Vector				m_vecLists;
	
	UT_AlphaHashTable		        m_hashDataItems;

	IEFileType				m_lastSavedAsType;
};


#endif /* PD_DOCUMENT_H */










