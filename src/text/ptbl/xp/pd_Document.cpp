 
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

#include <stdio.h>
#include <memory.h>
#include <malloc.h>
#include <string.h>
#include "ut_types.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "pd_Document.h"
#include "pt_PieceTable.h"
#include "pl_Listener.h"
#include "ie_imp.h"
#include "ie_exp.h"
#include "pf_Frag_Strux.h"

PD_Document::PD_Document()
{
	m_szFilename = NULL;
	m_bDirty = UT_FALSE;
	m_pPieceTable = NULL;
}

PD_Document::~PD_Document()
{
	if (m_szFilename)
		free((void *)m_szFilename);
	if (m_pPieceTable)
		delete m_pPieceTable;
	// we do not purge the contents of m_vecListeners
	// since these are now owned by us.
}

UT_Bool PD_Document::readFromFile(const char * szFilename)
{
	if (!szFilename || !*szFilename)
	{
		UT_DEBUGMSG(("PD_Document::readFromFile -- invalid filename\n"));
		return UT_FALSE;
	}

	m_pPieceTable = new pt_PieceTable(this);
	if (!m_pPieceTable)
	{
		UT_DEBUGMSG(("PD_Document::readFromFile -- could not construct piece table\n"));
		return UT_FALSE;
	}

	IE_Imp * pie = NULL;
	IEStatus ies;

	ies = IE_Imp::constructImporter(this,szFilename,&pie);
	if (ies != IES_OK)
	{
		UT_DEBUGMSG(("PD_Document::readFromFile -- could not construct importer\n"));
		return UT_FALSE;
	}

	m_pPieceTable->setPieceTableState(PTS_Loading);
	ies = pie->importFile(szFilename);
	delete pie;

	if (ies != IES_OK)
	{
		UT_DEBUGMSG(("PD_Document::readFromFile -- could not import file\n"));
		return UT_FALSE;
	}
	
	UT_ASSERT(!m_szFilename);
	if (!UT_cloneString((char *&)m_szFilename, szFilename))
	{
		UT_DEBUGMSG(("PD_Document::readFromFile -- no memory\n"));
		return UT_FALSE;
	}
	
	m_pPieceTable->setPieceTableState(PTS_Editing);
	setClean();							// mark the document as not-dirty
	return UT_TRUE;
}

UT_Bool PD_Document::newDocument(void)
{
	m_pPieceTable = new pt_PieceTable(this);
	if (!m_pPieceTable)
	{
		UT_DEBUGMSG(("PD_Document::newDocument -- could not construct piece table\n"));
		return UT_FALSE;
	}

	m_pPieceTable->setPieceTableState(PTS_Loading);

	// add just enough structure to empty document so we can edit
	appendStrux(PTX_Section,NULL);

	// need to set up a default column model, too
	const XML_Char * properties[] = 
	{
		"type",		"box", 
		"left",		"0in",
		"top",		"0in", 
		"width",	"*",
		"height",	"*", 
		0
	};

	const XML_Char** atts = properties;

	appendStrux(PTX_ColumnSet,NULL);
	appendStrux(PTX_Column,atts);
	appendStrux(PTX_Block,NULL);

	m_pPieceTable->setPieceTableState(PTS_Editing);
	setClean();							// mark the document as not-dirty
	return UT_TRUE;
}

UT_Bool PD_Document::saveAs(const char * szFilename, IEFileType ieft)
{
	if (!szFilename)
		return UT_FALSE;
	
	IE_Exp * pie = NULL;
	IEStatus ies;

	ies = IE_Exp::constructExporter(this,szFilename,ieft,&pie);
	if (ies != IES_OK)
	{
		UT_DEBUGMSG(("PD_Document::Save -- could not construct exporter\n"));
		return UT_FALSE;
	}

	ies = pie->writeFile(szFilename);
	delete pie;

	if (ies != IES_OK)
	{
		UT_DEBUGMSG(("PD_Document::Save -- could not write file\n"));
		return UT_FALSE;
	}

	// no file name currently set - make this filename the filename
	// stored for the doc
	if (!m_szFilename)
	{
		m_szFilename = new char[strlen(szFilename) + 1];
		strcpy(m_szFilename, szFilename);
	}
	
	setClean();
	return UT_TRUE;
}

UT_Bool PD_Document::save(IEFileType ieft)
{
	if (!m_szFilename || !*m_szFilename)
		return UT_FALSE;

	IE_Exp * pie = NULL;
	IEStatus ies;

	ies = IE_Exp::constructExporter(this,m_szFilename,ieft,&pie);
	if (ies != IES_OK)
	{
		UT_DEBUGMSG(("PD_Document::Save -- could not construct exporter\n"));
		return UT_FALSE;
	}

	ies = pie->writeFile(m_szFilename);
	delete pie;

	if (ies != IES_OK)
	{
		UT_DEBUGMSG(("PD_Document::Save -- could not write file\n"));
		return UT_FALSE;
	}

	setClean();
	return UT_TRUE;
}

const char * PD_Document::getFilename(void) const
{
	return m_szFilename;
}

UT_Bool PD_Document::isDirty(void) const
{
	return m_bDirty;
}

void PD_Document::setClean(void)
{
	m_bDirty = UT_FALSE;
}

UT_Bool PD_Document::insertSpan(PT_DocPosition dpos,
								  UT_UCSChar * p,
								  UT_uint32 length)
{
	return m_pPieceTable->insertSpan(dpos,p,length);
}

UT_Bool PD_Document::deleteSpan(PT_DocPosition dpos1,
								PT_DocPosition dpos2)
{
	return m_pPieceTable->deleteSpan(dpos1,dpos2);
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

void PD_Document::clearTemporarySpanFmt(void)
{
	m_pPieceTable->clearTemporarySpanFmt();
}

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

UT_Bool PD_Document::notifyListeners(pf_Frag_Strux * pfs, const PX_ChangeRecord * pcr) const
{
	// notify listeners of a change.
	
#ifdef PT_TEST
	pcr->__dump();
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

UT_Bool PD_Document::notifyListeners(pf_Frag_Strux * pfs,
									 pf_Frag_Strux * pfsNew,
									 const PX_ChangeRecord * pcr) const
{
	// notify listeners of a new strux.  this is slightly
	// different from the other one because we need to exchange
	// handles with the listener for the new strux.

#ifdef PT_TEST
	pcr->__dump();
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
			PL_StruxDocHandle sdhNew = (PL_StruxDocHandle)pfsNew;
			PL_StruxFmtHandle sfhNew = 0;
			PL_StruxFmtHandle sfh = pfs->getFmtHandle(lid);
			if (pListener->insertStrux(sfh,pcr,sdhNew,&sfhNew))
				pfsNew->setFmtHandle(lid,sfhNew);
		}
	}

	return UT_TRUE;
}

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

UT_Bool PD_Document::getSpanAttrProp(PL_StruxDocHandle sdh, UT_uint32 offset,
									 const PP_AttrProp ** ppAP) const
{
	return m_pPieceTable->getSpanAttrProp(sdh,offset,ppAP);
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

void PD_Document::beginUserAtomicGlob(void)
{
	m_pPieceTable->beginUserAtomicGlob();
}

void PD_Document::endUserAtomicGlob(void)
{
	m_pPieceTable->endUserAtomicGlob();
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


