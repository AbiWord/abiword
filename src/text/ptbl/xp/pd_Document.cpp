
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
	IE_Imp::IEStatus ies;

	ies = IE_Imp::constructImporter(this,szFilename,&pie);
	if (ies != IE_Imp::IES_OK)
	{
		UT_DEBUGMSG(("PD_Document::readFromFile -- could not construct importer\n"));
		return UT_FALSE;
	}

	m_pPieceTable->setPieceTableState(pt_PieceTable::PTS_Loading);
	ies = pie->importFile(szFilename);
	delete pie;

	if (ies != IE_Imp::IES_OK)
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
	
	m_pPieceTable->setPieceTableState(pt_PieceTable::PTS_Editing);
	setClean();							// mark the document as not-dirty
	return UT_TRUE;
}

UT_Bool PD_Document::newDocument(void)
{
	m_pPieceTable = new pt_PieceTable(this);
	if (!m_pPieceTable)
	{
		UT_DEBUGMSG(("PD_Document::readFromFile -- could not construct piece table\n"));
		return UT_FALSE;
	}

	m_pPieceTable->setPieceTableState(pt_PieceTable::PTS_Loading);

#if 1
	// add just enough structure to empty document so we can edit
	appendStrux(PTX_Section,NULL);
	appendStrux(PTX_ColumnSet,NULL);

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

	appendStrux(PTX_Column,atts);
	appendStrux(PTX_Block,NULL);

	// need one character so the formatter will create the first page
	UT_UCSChar space = 0x0020;

	appendSpan(&space,1);
#endif

	m_pPieceTable->setPieceTableState(pt_PieceTable::PTS_Editing);
	setClean();							// mark the document as not-dirty
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

void PD_Document::dump(FILE * fp) const
{
	fprintf(fp,"Dump for %s:\n",m_szFilename);
	fprintf(fp,"  Document is %s\n",((m_bDirty) ? "DIRTY" : "CLEAN"));
	
	if (m_pPieceTable)
		m_pPieceTable->dump(fp);
	
}

UT_Bool PD_Document::insertSpan(PT_DocPosition dpos,
								  UT_Bool bLeftSide,
								  UT_UCSChar * p,
								  UT_uint32 length)
{
	return UT_TRUE;
}

UT_Bool PD_Document::deleteSpan(PT_DocPosition dpos,
								  UT_uint32 length)
{
	return UT_TRUE;
}

UT_Bool PD_Document::insertFmt(PT_DocPosition dpos1,
								 PT_DocPosition dpos2,
								 const XML_Char ** attributes,
								 const XML_Char ** properties)
{
	return UT_TRUE;
}

UT_Bool PD_Document::deleteFmt(PT_DocPosition dpos1,
								 PT_DocPosition dpos2,
								 const XML_Char ** attributes,
								 const XML_Char ** properties)
{
	return UT_TRUE;
}

UT_Bool PD_Document::insertStrux(PT_DocPosition dpos,
								   PTStruxType pts,
								   const XML_Char ** attributes,
								   const XML_Char ** properties)
{
	return UT_TRUE;
}

UT_Bool PD_Document::deleteStrux(PT_DocPosition dpos)
{
	return UT_TRUE;
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

UT_Bool PD_Document::getAttrProp(PT_VarSetIndex vsIndex, PT_AttrPropIndex indexAP,
								 const PP_AttrProp ** ppAP) const
{
	return m_pPieceTable->getAttrProp(vsIndex,indexAP,ppAP);
}

UT_Bool PD_Document::getSpanPtr(PL_StruxDocHandle sdh, UT_uint32 offset,
								const UT_UCSChar ** ppSpan, UT_uint32 * pLength) const
{
	return m_pPieceTable->getSpanPtr(sdh,offset,ppSpan,pLength);
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


