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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ut_types.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "ut_misc.h"
#include "pd_Document.h"
#include "xad_Document.h"
#include "pt_PieceTable.h"
#include "pl_Listener.h"
#include "ie_imp.h"
#include "ie_exp.h"
#include "pf_Frag_Strux.h"
#include "pd_Style.h"
#include "pf_Frag_Object.h"
#include "pf_Frag.h"
#include "fd_Field.h"
#include "fl_BlockLayout.h"
#include "fl_Layout.h"
#include "fl_DocLayout.h"
#include "fv_View.h"
#include "fl_AutoNum.h"


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

struct _dataItemPair
{
	UT_ByteBuf* pBuf;
	void*		pToken;
};

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

PD_Document::PD_Document()
  : AD_Document(), m_hashDataItems(11)
{
	m_pPieceTable = NULL;

	// perhaps this should be a magic "unknown" or "NULL" value,
	// but now we just depend on save() never being called without
	// a previous saveAs() (which specifies a type)
	m_lastSavedAsType = IEFT_AbiWord_1;
	m_ballowListUpdates = UT_FALSE;
        m_bPieceTableChanging = UT_FALSE;
}

PD_Document::~PD_Document()
{
	if (m_szFilename)
		free((void *)m_szFilename);
	if (m_pPieceTable)
		delete m_pPieceTable;

	_destroyDataItemData();

	// we do not purge the contents of m_vecListeners
	// since these are not owned by us.
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

UT_Error PD_Document::readFromFile(const char * szFilename, int ieft)
{
	if (!szFilename || !*szFilename)
	{
		UT_DEBUGMSG(("PD_Document::readFromFile -- invalid filename\n"));
		return UT_INVALIDFILENAME;
	}

	m_pPieceTable = new pt_PieceTable(this);
	if (!m_pPieceTable)
	{
		UT_DEBUGMSG(("PD_Document::readFromFile -- could not construct piece table\n"));
		return UT_NOPIECETABLE;
	}

	IE_Imp * pie = NULL;
	UT_Error errorCode;

	errorCode = IE_Imp::constructImporter(this, szFilename, (IEFileType) ieft, &pie, &m_lastSavedAsType);
	if (errorCode)
	{
		UT_DEBUGMSG(("PD_Document::readFromFile -- could not construct importer\n"));
		return errorCode;
	}

	m_pPieceTable->setPieceTableState(PTS_Loading);
	errorCode = pie->importFile(szFilename);
	delete pie;

	if (errorCode)
	{
		UT_DEBUGMSG(("PD_Document::readFromFile -- could not import file\n"));
		return errorCode;
	}
	
	UT_ASSERT(!m_szFilename);
	if (!UT_cloneString((char *&)m_szFilename, szFilename))
	{
		UT_DEBUGMSG(("PD_Document::readFromFile -- no memory\n"));
		return UT_IE_NOMEMORY;
	}

	m_pPieceTable->setPieceTableState(PTS_Editing);
	updateFields();
	_setClean();							// mark the document as not-dirty
	return UT_OK;
}

UT_Error PD_Document::newDocument(void)
{
	m_pPieceTable = new pt_PieceTable(this);
	if (!m_pPieceTable)
	{
		UT_DEBUGMSG(("PD_Document::newDocument -- could not construct piece table\n"));
		return UT_NOPIECETABLE;
	}

	m_pPieceTable->setPieceTableState(PTS_Loading);

	// add just enough structure to empty document so we can edit

	appendStrux(PTX_Section,NULL);
	appendStrux(PTX_Block,NULL);

	m_pPieceTable->setPieceTableState(PTS_Editing);
	_setClean();							// mark the document as not-dirty
	return UT_OK;
}

UT_Error PD_Document::saveAs(const char * szFilename, int ieft)
{
	if (!szFilename)
		return UT_SAVE_NAMEERROR;
	
	IE_Exp * pie = NULL;
	UT_Error errorCode;

	errorCode = IE_Exp::constructExporter(this, szFilename, (IEFileType) ieft, &pie, &m_lastSavedAsType);
	if (errorCode)
	{
		UT_DEBUGMSG(("PD_Document::Save -- could not construct exporter\n"));
		return UT_SAVE_EXPORTERROR;
	}

	errorCode = pie->writeFile(szFilename);
	delete pie;

	if (errorCode)
	{
		UT_DEBUGMSG(("PD_Document::Save -- could not write file\n"));
		return UT_SAVE_WRITEERROR;
	}

	// no file name currently set - make this filename the filename
	// stored for the doc
	if (m_szFilename)
	{
		free((void *) m_szFilename);
		m_szFilename = NULL;
	}
	
	char * szFilenameCopy = NULL;
	if (!UT_cloneString(szFilenameCopy,szFilename))
		return UT_SAVE_OTHERERROR;
	m_szFilename = szFilenameCopy;

	_setClean();
	return UT_OK;
}

UT_Error PD_Document::save(void)
{
	if (!m_szFilename || !*m_szFilename)
		return UT_SAVE_NAMEERROR;
	if (m_lastSavedAsType == IEFT_Unknown)
		return UT_EXTENSIONERROR;

	IE_Exp * pie = NULL;
	UT_Error errorCode;

	// make sure we don't cause extension problems

	const char* szSuffix = UT_pathSuffix(m_szFilename);
	if(szSuffix && strcmp(szSuffix, ".doc") == 0)
	{
	  UT_DEBUGMSG(("PD_Document::Save -- word extensions cause problems\n"));
	  return UT_EXTENSIONERROR;
	}

	errorCode = IE_Exp::constructExporter(this,m_szFilename,m_lastSavedAsType,&pie);
	if (errorCode)
	{
		UT_DEBUGMSG(("PD_Document::Save -- could not construct exporter\n"));
		return UT_SAVE_EXPORTERROR;
	}

	errorCode = pie->writeFile(m_szFilename);
	delete pie;

	if (errorCode)
	{
		UT_DEBUGMSG(("PD_Document::Save -- could not write file\n"));
		return UT_SAVE_WRITEERROR;
	}

	_setClean();
	return UT_OK;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

UT_Bool PD_Document::isDirty(void) const
{
	return m_pPieceTable->isDirty();
}

void PD_Document::_setClean(void)
{
	m_pPieceTable->setClean();
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

UT_Bool	PD_Document::insertObject(PT_DocPosition dpos,
										 PTObjectType pto,
										 const XML_Char ** attributes,
										 const XML_Char ** properties)
{
	return m_pPieceTable->insertObject(dpos, pto, attributes, properties);
}



UT_Bool	PD_Document::insertObject(PT_DocPosition dpos,
										 PTObjectType pto,
										 const XML_Char ** attributes,
										 const XML_Char ** properties, fd_Field ** pField)
{
	pf_Frag_Object * pfo = NULL;
	UT_Bool bres =  m_pPieceTable->insertObject(dpos, pto, attributes, properties, &pfo);
	*pField = pfo->getField();
	return bres;
}


UT_Bool PD_Document::insertSpan(PT_DocPosition dpos,
								  const UT_UCSChar * p,
								  UT_uint32 length,
								  PP_AttrProp *p_AttrProp)
{
	if(p_AttrProp)
	{
		m_pPieceTable->insertFmtMark(PTC_AddFmt, dpos, p_AttrProp);
	}

	return m_pPieceTable->insertSpan(dpos,p,length);
}

UT_Bool PD_Document::deleteSpan(PT_DocPosition dpos1,
								PT_DocPosition dpos2,
								PP_AttrProp *p_AttrProp_Before)
{
	return m_pPieceTable->deleteSpan(dpos1, dpos2, p_AttrProp_Before);
}												 

UT_Bool PD_Document::changeSpanFmt(PTChangeFmt ptc,
								   PT_DocPosition dpos1,
								   PT_DocPosition dpos2,
								   const XML_Char ** attributes,
								   const XML_Char ** properties)
{
	return m_pPieceTable->changeSpanFmt(ptc,dpos1,dpos2,attributes,properties);
}

UT_Bool PD_Document::insertStrux(PT_DocPosition dpos,
								 PTStruxType pts)
{
	return m_pPieceTable->insertStrux(dpos,pts);
}

UT_Bool PD_Document::changeStruxFmt(PTChangeFmt ptc,
									PT_DocPosition dpos1,
									PT_DocPosition dpos2,
									const XML_Char ** attributes,
									const XML_Char ** properties,
									PTStruxType pts)
{
	return m_pPieceTable->changeStruxFmt(ptc,dpos1,dpos2,attributes,properties,pts);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

UT_Bool PD_Document::appendStrux(PTStruxType pts, const XML_Char ** attributes)
{
	UT_ASSERT(m_pPieceTable);

	// can only be used while loading the document

	return m_pPieceTable->appendStrux(pts,attributes);
}

UT_Bool PD_Document::appendFmt(const XML_Char ** attributes)
{
	UT_ASSERT(m_pPieceTable);

	// can only be used while loading the document

	return m_pPieceTable->appendFmt(attributes);
}

UT_Bool PD_Document::appendFmt(const UT_Vector * pVecAttributes)
{
	UT_ASSERT(m_pPieceTable);
	
	// can only be used while loading the document

	return m_pPieceTable->appendFmt(pVecAttributes);
}

UT_Bool PD_Document::appendSpan(UT_UCSChar * pbuf, UT_uint32 length)
{
	UT_ASSERT(m_pPieceTable);
	
	// can only be used while loading the document

	return m_pPieceTable->appendSpan(pbuf,length);
}

UT_Bool PD_Document::appendObject(PTObjectType pto, const XML_Char ** attributes)
{
	UT_ASSERT(m_pPieceTable);
	
	// can only be used while loading the document

	return m_pPieceTable->appendObject(pto,attributes);
}

UT_Bool PD_Document::appendStyle(const XML_Char ** attributes)
{
	UT_ASSERT(m_pPieceTable);
	
	// can only be used while loading the document

	return m_pPieceTable->appendStyle(attributes);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

UT_Bool PD_Document::tellListener(PL_Listener* pListener)
{
	UT_ASSERT(pListener);
	UT_ASSERT(m_pPieceTable);

	return m_pPieceTable->tellListener(pListener);
}

UT_Bool PD_Document::tellListenerSubset(PL_Listener* pListener, PD_DocumentRange * pDocRange)
{
	UT_ASSERT(pListener);
	UT_ASSERT(m_pPieceTable);
	UT_ASSERT(pDocRange && pDocRange->m_pDoc==this);
	
	return m_pPieceTable->tellListenerSubset(pListener,pDocRange);
}

UT_Bool PD_Document::addListener(PL_Listener * pListener,
								 PL_ListenerId * pListenerId)
{
	UT_uint32 kLimit = m_vecListeners.getItemCount();
	UT_uint32 k;

	// see if we can recycle a cell in the vector.
	
	for (k=0; k<kLimit; k++)
		if (m_vecListeners.getNthItem(k) == 0)
		{
			(void)m_vecListeners.setNthItem(k,pListener,NULL);
			goto ClaimThisK;
		}

	// otherwise, extend the vector for it.
	
	if (m_vecListeners.addItem(pListener,&k) != 0)
		return UT_FALSE;				// could not add item to vector

  ClaimThisK:

	// propagate the listener to the PieceTable and
	// let it do its thing.
	UT_ASSERT(m_pPieceTable);

	m_pPieceTable->addListener(pListener,k);

	// give our vector index back to the caller as a "Listener Id".
	
	*pListenerId = k;
	return UT_TRUE;
}

UT_Bool PD_Document::removeListener(PL_ListenerId listenerId)
{
	return (m_vecListeners.setNthItem(listenerId,NULL,NULL) == 0);
}

UT_Bool PD_Document::signalListeners(UT_uint32 iSignal) const
{
	PL_ListenerId lid;
	PL_ListenerId lidCount = m_vecListeners.getItemCount();

	// for each listener in our vector, we send a notification.
	// we step over null listners (for listeners which have been
	// removed (views that went away)).
	
	for (lid=0; lid<lidCount; lid++)
	{
		PL_Listener * pListener = (PL_Listener *)m_vecListeners.getNthItem(lid);
		if (pListener)
		{
			pListener->signal(iSignal);
		}
	}

	return UT_TRUE;
}

UT_Bool PD_Document::notifyListeners(pf_Frag_Strux * pfs, const PX_ChangeRecord * pcr) const
{
	// notify listeners of a change.
	
#ifdef PT_TEST
	//pcr->__dump();
#endif

	PL_ListenerId lid;
	PL_ListenerId lidCount = m_vecListeners.getItemCount();

	// for each listener in our vector, we send a notification.
	// we step over null listners (for listeners which have been
	// removed (views that went away)).
	
	for (lid=0; lid<lidCount; lid++)
	{
		PL_Listener * pListener = (PL_Listener *)m_vecListeners.getNthItem(lid);
		if (pListener)
		{
			PL_StruxFmtHandle sfh = 0;
			if (pfs)
				sfh = pfs->getFmtHandle(lid);
			pListener->change(sfh,pcr);
		}
	}

	return UT_TRUE;
}

static void s_BindHandles(PL_StruxDocHandle sdhNew,
						  PL_ListenerId lid,
						  PL_StruxFmtHandle sfhNew)
{
	UT_ASSERT(sdhNew);
	UT_ASSERT(sfhNew);

	pf_Frag_Strux * pfsNew = (pf_Frag_Strux *)sdhNew;
	pfsNew->setFmtHandle(lid,sfhNew);
}

UT_Bool PD_Document::notifyListeners(pf_Frag_Strux * pfs,
									 pf_Frag_Strux * pfsNew,
									 const PX_ChangeRecord * pcr) const
{
	// notify listeners of a new strux.  this is slightly
	// different from the other one because we need to exchange
	// handles with the listener for the new strux.

#ifdef PT_TEST
	//pcr->__dump();
#endif
	
	PL_ListenerId lid;
	PL_ListenerId lidCount = m_vecListeners.getItemCount();

	// for each listener in our vector, we send a notification.
	// we step over null listeners (for listeners which have been
	// removed (views that went away)).
	
	for (lid=0; lid<lidCount; lid++)
	{
		PL_Listener * pListener = (PL_Listener *)m_vecListeners.getNthItem(lid);
		if (pListener)
		{
			PL_StruxDocHandle sdhNew = (PL_StruxDocHandle)pfsNew;
			PL_StruxFmtHandle sfh = pfs->getFmtHandle(lid);
			if (pListener->insertStrux(sfh,pcr,sdhNew,lid,s_BindHandles))
			{
				// verify that the listener used our callback
				UT_ASSERT(pfsNew->getFmtHandle(lid));
			}
		}
	}

	return UT_TRUE;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

UT_Bool PD_Document::getAttrProp(PT_AttrPropIndex indexAP, const PP_AttrProp ** ppAP) const
{
	return m_pPieceTable->getAttrProp(indexAP,ppAP);
}

const UT_UCSChar * PD_Document::getPointer(PT_BufIndex bi) const
{
	// the pointer that we return is NOT a zero-terminated
	// string.  the caller is responsible for knowing how
	// long the data is within the span/fragment.
	
	return m_pPieceTable->getPointer(bi);
}

UT_Bool PD_Document::getSpanPtr(PL_StruxDocHandle sdh, UT_uint32 offset,
								const UT_UCSChar ** ppSpan, UT_uint32 * pLength) const
{
	return m_pPieceTable->getSpanPtr(sdh,offset,ppSpan,pLength);
}

UT_Bool PD_Document::getBlockBuf(PL_StruxDocHandle sdh, UT_GrowBuf * pgb) const
{
	return m_pPieceTable->getBlockBuf(sdh,pgb);
}

UT_Bool PD_Document::getBounds(UT_Bool bEnd, PT_DocPosition & docPos) const
{
	return m_pPieceTable->getBounds(bEnd,docPos);
}

PT_DocPosition PD_Document::getStruxPosition(PL_StruxDocHandle sdh) const
{
	return m_pPieceTable->getStruxPosition(sdh);
}

UT_Bool PD_Document::getSpanAttrProp(PL_StruxDocHandle sdh, UT_uint32 offset, UT_Bool bLeftSide,
									 const PP_AttrProp ** ppAP) const
{
	return m_pPieceTable->getSpanAttrProp(sdh,offset,bLeftSide,ppAP);
}

UT_Bool PD_Document::getField(PL_StruxDocHandle sdh, UT_uint32 offset,
                               fd_Field * & pField)
{

	pf_Frag * pf = (pf_Frag *)sdh;
	UT_ASSERT(pf->getType() == pf_Frag::PFT_Strux);
	pf_Frag_Strux * pfsBlock = static_cast<pf_Frag_Strux *> (pf);
	UT_ASSERT(pfsBlock->getStruxType() == PTX_Block);

	UT_uint32 cumOffset = 0;
	pf_Frag_Text * pft = NULL;
	for (pf_Frag * pfTemp=pfsBlock->getNext(); (pfTemp); pfTemp=pfTemp->getNext())
	{
		cumOffset += pfTemp->getLength();
		if (offset < cumOffset)
		{
			switch (pfTemp->getType()) 
			{
			case pf_Frag::PFT_Text:
				pft = static_cast<pf_Frag_Text *> (pfTemp);
				pField = pft->getField();
				return UT_TRUE; // break out of loop
				break;
			default:
				return UT_FALSE;
				break;
			}
		}

	}
	return UT_FALSE;
}

UT_Bool PD_Document::getStruxFromPosition(PL_ListenerId listenerId,
										  PT_DocPosition docPos,
										  PL_StruxFmtHandle * psfh) const
{
	return m_pPieceTable->getStruxFromPosition(listenerId,docPos,psfh);
}

UT_Bool PD_Document::getStruxOfTypeFromPosition(PL_ListenerId listenerId,
												PT_DocPosition docPos,
												PTStruxType pts,
												PL_StruxFmtHandle * psfh) const
{
	return m_pPieceTable->getStruxOfTypeFromPosition(listenerId,docPos,pts,psfh);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void PD_Document::beginUserAtomicGlob(void)
{
	m_pPieceTable->beginUserAtomicGlob();
}

void PD_Document::endUserAtomicGlob(void)
{
	m_pPieceTable->endUserAtomicGlob();
}

UT_Bool PD_Document::canDo(UT_Bool bUndo) const
{
	return m_pPieceTable->canDo(bUndo);
}

UT_Bool PD_Document::undoCmd(UT_uint32 repeatCount)
{
	while (repeatCount--)
		if (!m_pPieceTable->undoCmd())
			return UT_FALSE;
	return UT_TRUE;
}

UT_Bool PD_Document::redoCmd(UT_uint32 repeatCount)
{
	while (repeatCount--)
		if (!m_pPieceTable->redoCmd())
			return UT_FALSE;
	return UT_TRUE;
}

///////////////////////////////////////////////////////////////////
// DataItems represent opaque (and probably binary) data found in
// the data-section of the document.  These are used, for example,
// to store the actual data of an image.  The inline image tag has
// a reference to a DataItem.

UT_Bool PD_Document::createDataItem(const char * szName, UT_Bool bBase64, const UT_ByteBuf * pByteBuf,
									void* pToken,
									void ** ppHandle)
{
	// verify unique name

	if (getDataItemDataByName(szName,NULL,NULL,NULL) == UT_TRUE)
		return UT_FALSE;				// invalid or duplicate name

	// set the actual DataItem's data using the contents of the ByteBuf.
	// we must copy it if we want to keep it.  bBase64 is TRUE if the
	// data is Base64 encoded.

	UT_ASSERT(pByteBuf);
	
	struct _dataItemPair* pPair = NULL;
	
	UT_ByteBuf * pNew = new UT_ByteBuf();
	if (!pNew)
		return UT_FALSE;
	
	if (bBase64)
	{
		if (!UT_Base64Decode(pNew,pByteBuf))
			goto Failed;
	}
	else
	{
		if (!pNew->ins(0,pByteBuf->getPointer(0),pByteBuf->getLength()))
			goto Failed;
	}

	pPair = new _dataItemPair();
	if (!pPair)
	{
		goto Failed;
	}
	
	pPair->pBuf = pNew;
	pPair->pToken = pToken;
	
	if (m_hashDataItems.addEntry(szName,NULL,(void *)pPair) == -1)
		goto Failed;

	// give them back a handle if they want one
	
	if (ppHandle)
	{
		UT_HashEntry * pHashEntry = m_hashDataItems.findEntry(szName);
		UT_ASSERT(pHashEntry);
		*ppHandle = (void *)pHashEntry;
	}
	
	return UT_TRUE;

Failed:
	if (pNew)
		delete pNew;
	return UT_FALSE;
}

UT_Bool PD_Document::getDataItemDataByName(const char * szName,
										   const UT_ByteBuf ** ppByteBuf,
										   void** ppToken,
										   void ** ppHandle) const
{
	UT_ASSERT(szName && *szName);
	
	UT_HashEntry * pHashEntry = m_hashDataItems.findEntry(szName);
	if (!pHashEntry)
		return UT_FALSE;

	struct _dataItemPair* pPair = (struct _dataItemPair*) pHashEntry->pData;
	UT_ASSERT(pPair);
	
	if (ppByteBuf)
	{
		*ppByteBuf = pPair->pBuf;
	}

	if (ppToken)
	{
		*ppToken = pPair->pToken;
	}

	if (ppHandle)
	{
		*ppHandle = (void *)pHashEntry;
	}
	
	return UT_TRUE;
}

UT_Bool PD_Document::setDataItemToken(void * pHandle,
									  void* pToken)
{
	UT_ASSERT(pHandle);
	
	UT_HashEntry * pHashEntry = (UT_HashEntry *)pHandle;

	struct _dataItemPair* pPair = (struct _dataItemPair*) pHashEntry->pData;
	UT_ASSERT(pPair);

	pPair->pToken = pToken;
	
	return UT_TRUE;
}

UT_Bool PD_Document::getDataItemData(void * pHandle,
									 const char ** pszName,
									 const UT_ByteBuf ** ppByteBuf,
									 void** ppToken) const
{
	UT_ASSERT(pHandle);
	
	UT_HashEntry * pHashEntry = (UT_HashEntry *)pHandle;

	struct _dataItemPair* pPair = (struct _dataItemPair*) pHashEntry->pData;
	UT_ASSERT(pPair);
	
	if (ppByteBuf)
	{
		*ppByteBuf = pPair->pBuf;
	}

	if (ppToken)
	{
		*ppToken = pPair->pToken;
	}

	if (pszName)
	{
		*pszName = pHashEntry->pszLeft;
	}
	
	return UT_TRUE;
}

UT_Bool PD_Document::enumDataItems(UT_uint32 k,
								   void ** ppHandle, const char ** pszName, const UT_ByteBuf ** ppByteBuf, void** ppToken) const
{
	// return the kth data item.

	UT_uint32 kLimit = m_hashDataItems.getEntryCount();
	if (k >= kLimit)
		return UT_FALSE;
	
	const UT_HashEntry * pHashEntry = m_hashDataItems.getNthEntryAlpha(k);
	UT_ASSERT(pHashEntry);

	if (ppHandle)
		*ppHandle = (void *)pHashEntry;

	struct _dataItemPair* pPair = (struct _dataItemPair*) pHashEntry->pData;
	UT_ASSERT(pPair);
	
	if (ppByteBuf)
	{
		*ppByteBuf = pPair->pBuf;
	}

	if (ppToken)
	{
		*ppToken = pPair->pToken;
	}
	
	if (pszName)
	{
		*pszName = pHashEntry->pszLeft;
	}
	
	return UT_TRUE;
}

void PD_Document::_destroyDataItemData(void)
{
	UT_uint32 kLimit = m_hashDataItems.getEntryCount();

	for (UT_uint32 k=0; (k<kLimit); k++)
	{
		UT_HashEntry * pHE = m_hashDataItems.getNthEntry(k);
		
		struct _dataItemPair* pPair = (struct _dataItemPair*) pHE->pData;
		UT_ASSERT(pPair);

		delete pPair->pBuf;
		FREEP(pPair->pToken);
		delete pPair;

		pHE->pData = NULL;
	}
}


///////////////////////////////////////////////////////////////////
// Styles represent named collections of formatting properties.

UT_Bool PD_Document::getStyle(const char * szName, PD_Style ** ppStyle) const
{
	return m_pPieceTable->getStyle(szName, ppStyle);
}

UT_Bool PD_Document::enumStyles(UT_uint32 k,
								const char ** pszName, const PD_Style ** ppStyle) const
{
	return m_pPieceTable->enumStyles(k, pszName, ppStyle);
}

//////////////////////////////////////////////////////////////////

void PD_Document::clearIfAtFmtMark(PT_DocPosition dpos)
{
	m_pPieceTable->clearIfAtFmtMark(dpos);
}

UT_Bool PD_Document::updateFields(void)
{
	//
	// Turn off Insertion point motion during this general update
	//
	setDontChangeInsPoint();
	pf_Frag * currentFrag = m_pPieceTable->getFragments().getFirst();
	UT_ASSERT(currentFrag);
	while (currentFrag!=m_pPieceTable->getFragments().getLast())
	{
		if (currentFrag->getType()==pf_Frag::PFT_Object)
		{
			pf_Frag_Object * pfo = static_cast<pf_Frag_Object *>
				(currentFrag);
			if (pfo->getObjectType()==PTO_Field)
			{
				UT_ASSERT (pfo->getField());
				pfo->getField()->update();
			}
		}
		currentFrag = currentFrag->getNext();
	}
	//
	// Restore insertion point motion
	//
	allowChangeInsPoint();
	return true;
}

void PD_Document::setDontChangeInsPoint(void)
{
	//
	// Notify all views that they cannot change the insertion point
	//
	pf_Frag * currentFrag = m_pPieceTable->getFragments().getFirst();
	//
	//Look for the first strux so we can notify the views via a strux listner
	//
	while (currentFrag!=m_pPieceTable->getFragments().getLast())
	{
		if (currentFrag->getType()==pf_Frag::PFT_Strux)
		{
			break;
		}
		currentFrag = currentFrag->getNext();
	}
	if(currentFrag!=m_pPieceTable->getFragments().getLast())
	{
		pf_Frag_Strux * pfs = ( pf_Frag_Strux *) currentFrag;
		PT_AttrPropIndex pAppIndex = pfs->getIndexAP();
		PT_DocPosition pos = getStruxPosition(pfs);
		const PX_ChangeRecord * pcr = new PX_ChangeRecord(PX_ChangeRecord::PXT_DontChangeInsPoint,pos,pAppIndex);
		notifyListeners(pfs, pcr);
		delete pcr;		
	}
	else
	{
		UT_DEBUGMSG(("setDontChangeInsPoint: No strux in document!! \n"));
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
}

void PD_Document::allowChangeInsPoint(void)
{
  //
  // Notify all views that they can change the insertion point
  //
	pf_Frag * currentFrag = m_pPieceTable->getFragments().getFirst();
	//
	//Look for the first strux so we can notify the views via a strux listner
	//
	while (currentFrag!=m_pPieceTable->getFragments().getLast())
	{
		if (currentFrag->getType()==pf_Frag::PFT_Strux)
		{
			break;
		}
		currentFrag = currentFrag->getNext();
	}
	if(currentFrag!=m_pPieceTable->getFragments().getLast())
	{
		pf_Frag_Strux * pfs = ( pf_Frag_Strux *) currentFrag;
		PT_AttrPropIndex pAppIndex = pfs->getIndexAP();
		PT_DocPosition pos = getStruxPosition(pfs);
		const PX_ChangeRecord * pcr = new PX_ChangeRecord(PX_ChangeRecord::PXT_AllowChangeInsPoint,pos,pAppIndex);
		notifyListeners(pfs, pcr);
		delete pcr;
	}
	else
	{
		UT_DEBUGMSG(("allowChangeInsPoint: No strux in document!! \n"));
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Step towards full thread safety

void PD_Document::notifyPieceTableChangeStart(void)
{
        m_bPieceTableChanging = UT_TRUE;
}


void PD_Document::notifyPieceTableChangeEnd(void)
{
        m_bPieceTableChanging = UT_FALSE;
}

UT_Bool PD_Document::isPieceTableChanging(void)
{
        return m_bPieceTableChanging;
}

////////////////////////////////////////////////////////////////
// List Vector Functions



fl_AutoNum * PD_Document::getListByID(UT_uint32 id) const
{
	UT_uint16 i = 0;
	UT_sint32 cnt = 0;
	fl_AutoNum * pAutoNum;

	cnt = m_vecLists.getItemCount();
	if ( cnt <= 0)
		return (fl_AutoNum *) NULL;
	UT_ASSERT(m_vecLists.getFirstItem());

	while (i<cnt)
	{
		pAutoNum = (fl_AutoNum *)m_vecLists[i];
		if (pAutoNum->getID() == id)
			return pAutoNum;
		i++;
	}
	
	return (fl_AutoNum *) NULL;
}

UT_Bool PD_Document::enumLists(UT_uint32 k, fl_AutoNum ** pAutoNum) 
{
	UT_uint32 kLimit = m_vecLists.getItemCount();
	if (k >= kLimit)
		return UT_FALSE;
	
	if (pAutoNum)
		*pAutoNum = (fl_AutoNum *)m_vecLists[k];
	
	return UT_TRUE;
}

fl_AutoNum * PD_Document::getNthList(UT_uint32 i) const
{
	UT_ASSERT(i >= 0);
	return (fl_AutoNum *)m_vecLists[i];
}

UT_uint32 PD_Document::getListsCount(void) const
{
	return m_vecLists.getItemCount();
}

void PD_Document::addList(fl_AutoNum * pAutoNum)
{
	UT_uint32 id = pAutoNum->getID();
	UT_uint32 i;
	UT_uint32 numlists = m_vecLists.getItemCount();
	for(i=0; i < numlists; i++)
	{
		fl_AutoNum * pAuto = (fl_AutoNum *) m_vecLists.getNthItem(i);
		if(pAuto->getID() == id)
			break;
	}
	if(i >= numlists)
		m_vecLists.addItem(pAutoNum);
}

void PD_Document::listUpdate(PL_StruxDocHandle sdh )
{
	//
	// Notify all views of a listupdate
	//
	UT_ASSERT(sdh);
	pf_Frag_Strux * pfs = (pf_Frag_Strux *) sdh;
	PT_AttrPropIndex pAppIndex = pfs->getIndexAP();
	//PT_DocPosition pos = getStruxPosition(sdh) + fl_BLOCK_STRUX_OFFSET;
	PT_DocPosition pos = getStruxPosition(sdh);
		const PX_ChangeRecord * pcr = new PX_ChangeRecord(PX_ChangeRecord::PXT_ListUpdate,pos,pAppIndex);
	notifyListeners(pfs, pcr);
	delete pcr;
}


void PD_Document::StopList(PL_StruxDocHandle sdh )
{
	//
	// Notify all views of a stoplist
	//
	pf_Frag_Strux * pfs = (pf_Frag_Strux *) sdh;
	PT_AttrPropIndex pAppIndex = pfs->getIndexAP();
	PT_DocPosition pos = getStruxPosition(sdh);
	const PX_ChangeRecord * pcr = new PX_ChangeRecord(PX_ChangeRecord::PXT_StopList,pos,pAppIndex);
	notifyListeners(pfs, pcr);
	delete pcr;
}


UT_Bool PD_Document::appendList(const XML_Char ** attributes)
{
	const XML_Char * szID=NULL, * szPid=NULL, * szType=NULL, * szStart=NULL, * szDelim=NULL, *szDec=NULL;
	UT_uint32 id, parent_id, start;
	List_Type type;
	
	for (const XML_Char ** a = attributes; (*a); a++)
	{
		if (UT_XML_stricmp(a[0],"id") == 0)
			szID = a[1];
		else if (UT_XML_stricmp(a[0], "parentid") == 0)
			szPid = a[1];
		else if (UT_XML_stricmp(a[0], "type") == 0)
			szType = a[1];
		else if (UT_XML_stricmp(a[0], "start-value") == 0)
			szStart = a[1];
		else if (UT_XML_stricmp(a[0], "list-delim") == 0)
			szDelim = a[1];
		else if (UT_XML_stricmp(a[0], "list-decimal") == 0)
			szDec = a[1];
	}

	if(!szID)
		return UT_FALSE;
	if(!szPid)
		return UT_FALSE;
	if(!szType)
		return UT_FALSE;
	if(!szStart)
		return UT_FALSE;
	if(!szDelim)
		return UT_FALSE;
	if(!szDec)
		szDec = (const XML_Char *) ".";
	id = atoi(szID);
	UT_uint32 i;
	UT_uint32 numlists = m_vecLists.getItemCount();
	for(i=0; i < numlists; i++)
	{
		fl_AutoNum * pAuto = (fl_AutoNum *) m_vecLists.getNthItem(i);
		if(pAuto->getID() == id)
			break;
	}
	if(i < numlists)
		return UT_TRUE; // List is already present
	parent_id = atoi(szPid);
	type = (List_Type)atoi(szType);
	start = atoi(szStart);

	fl_AutoNum * pAutoNum = new fl_AutoNum(id, parent_id, type, start, szDelim,szDec,this);
	addList(pAutoNum);
	
	return UT_TRUE;
}

UT_Bool PD_Document::areListUpdatesAllowed(void)
{
        return m_ballowListUpdates;
}
 
void PD_Document::disableListUpdates(void)
{
        m_ballowListUpdates = UT_FALSE;
}
   
void PD_Document::enableListUpdates(void)
{
        m_ballowListUpdates = UT_TRUE;
}
  
void PD_Document::updateDirtyLists(void)
{
	UT_uint32 iNumLists = m_vecLists.getItemCount();
	UT_uint32 i;
	fl_AutoNum * pAutoNum;
	for(i=0; i< iNumLists; i++)
	{
		pAutoNum = (fl_AutoNum *) m_vecLists.getNthItem(i);
		if(pAutoNum->isDirty() == UT_TRUE)
		{
			pAutoNum->update(0);
		}
	}
	for(i=0; i< iNumLists; i++)
	{
		pAutoNum = (fl_AutoNum *) m_vecLists.getNthItem(i);
		pAutoNum->findAndSetParentItem();
	}
}


UT_Bool PD_Document::fixListHierarchy(void)
{
	UT_uint32 iNumLists = m_vecLists.getItemCount();
	fl_AutoNum * pAutoNum;

	if (iNumLists == 0)
	{
		return UT_FALSE;
	}
	else
	{
		for (UT_uint32 i = 0; i < iNumLists; i++)
		{
			pAutoNum = (fl_AutoNum *)m_vecLists.getNthItem(i);
			pAutoNum->fixHierarchy(this);
		}
		return UT_TRUE;
	}
}

void PD_Document::removeList(fl_AutoNum * pAutoNum, PL_StruxDocHandle sdh )
{
	UT_ASSERT(pAutoNum);
	UT_sint32 ndx = m_vecLists.findItem(pAutoNum);
	UT_ASSERT(ndx >= 0);
	if (ndx != -1)
	{
		//
		// Notify all views of a remove List
		//
		pf_Frag_Strux * pfs = (pf_Frag_Strux *) sdh;
		PT_AttrPropIndex pAppIndex = pfs->getIndexAP();
		PT_DocPosition pos = getStruxPosition(sdh);
		const PX_ChangeRecord * pcr = new PX_ChangeRecord(PX_ChangeRecord::PXT_RemoveList,pos,pAppIndex);
		notifyListeners(pfs, pcr);
		delete pcr;						  
		m_vecLists.deleteNthItem(ndx);
	}
}








