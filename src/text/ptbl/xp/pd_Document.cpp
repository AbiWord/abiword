
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
	return UT_FALSE;
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
}

