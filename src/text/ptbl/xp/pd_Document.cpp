
#include <stdio.h>
#include <memory.h>
#include <malloc.h>
#include <string.h>
#include "ut_types.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "pd_Document.h"
#include "ie_imp.h"

PD_Document::PD_Document()
{
	m_szFilename = NULL;
	m_bDirty = UT_FALSE;
}

PD_Document::~PD_Document()
{
	if (m_szFilename)
		free((void *)m_szFilename);
}


UT_Bool PD_Document::readFromFile(const char * szFilename)
{
	if (!szFilename || !*szFilename)
	{
		UT_DEBUGMSG(("PD_Document::readFromFile -- invalid filename\n"));
		return UT_FALSE;
	}

	IE_Imp * pie = NULL;
	IE_Imp::IEStatus ies;

	ies = IE_Imp::constructImporter(this,szFilename,&pie);
	if (ies != IES_OK)
	{
		UT_DEBUGMSG(("PD_Document::could not construct importer\n"));
		return UT_FALSE;
	}

	setPieceTableState(PTS_Loading);
	ies = pie->importFile(szFilename);
	delete pie;

	if (ies != IES_OK)
	{
		UT_DEBUGMSG(("PD_Document::could not import file\n"));
		return UT_FALSE;
	}
	
	UT_ASSERT(!m_szFilename);
	if (!UT_cloneString(m_szFilename, szFilename))
	{
		UT_DEBUGMSG(("PD_Document::readFromFile -- no memory\n"));
		return UT_FALSE;
	}
	
	setPieceTableState(PTS_Editing);
	setClean();							// mark the document as not-dirty
}

UT_Bool PD_Document::newDocument(void)
{
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

