
#include "string.h"

#include "ut_types.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ie_imp.h"
#include "ie_imp_AbiWord_1.h"
#include "ie_imp_MsWord_97.h"

IE_Imp::IE_Imp(PD_Document * pDocument)
{
	m_pDocument = pDocument;
}

IE_Imp::~IE_Imp()
{
}

/*****************************************************************/
/*****************************************************************/

IEStatus IE_Imp::constructImporter(PD_Document * pDocument,
								   const char * szFilename,
								   IE_Imp ** ppie)
{
	// construct an importer of the right type.
	// caller is responsible for deleting the importer object
	// when finished with it.
	
	UT_ASSERT(pDocument);
	UT_ASSERT(szFilename && *szFilename);
	UT_ASSERT(ppie);
	
	// TODO use some heuristic to decide what type of file
	// TODO we have been given.  Then instantiate the correct
	// TODO ie_imp sublcass to read it.  If we cannot decide
	// TODO what it is, return IES_UnknownType.
	//
	// TODO for now, we just assume AbiWord_1.

	int i = 0, nLen = strlen(szFilename);

	const char *pExt = szFilename;

	while ((*pExt != '.') && (i < nLen))
	{
		pExt++;
		i++;
	}

	if (pExt && (UT_stricmp(pExt, ".doc") == 0))
	{
		IE_Imp_MsWord_97 * p = new IE_Imp_MsWord_97(pDocument);
		if (!p)
			return IES_NoMemory;

		*ppie = p;
	}
	else
	{
		IE_Imp_AbiWord_1 * p = new IE_Imp_AbiWord_1(pDocument);
		if (!p)
			return IES_NoMemory;

		*ppie = p;
	}

	return IES_OK;
}
