
#ifndef PD_DOCUMENT_H
#define PD_DOCUMENT_H

// TODO should the filename be UT_UCSChar rather than char ?

#include <stdio.h>
#include "ut_types.h"
#include "ut_vector.h"
#include "xmlparse.h"
#include "pt_Types.h"
#include "pl_Listener.h"
class pt_PieceTable;



// PD_Document is the representation for a document.

class PD_Document
{
public:
	PD_Document();
	~PD_Document();

	UT_Bool					readFromFile(const char * szFilename);
	UT_Bool					newDocument(void);
	const char *			getFilename(void) const;
	UT_Bool					isDirty(void) const;
	void					setClean(void);

	UT_Bool					insertSpan(PT_DocPosition dpos,
									   UT_Bool bLeftSide,
									   UT_UCSChar * p,
									   UT_uint32 length);
	UT_Bool					deleteSpan(PT_DocPosition dpos,
									   UT_uint32 length);

	UT_Bool					insertFmt(PT_DocPosition dpos1,
									  PT_DocPosition dpos2,
									  const XML_Char ** attributes,
									  const XML_Char ** properties);
	UT_Bool					deleteFmt(PT_DocPosition dpos1,
									  PT_DocPosition dpos2,
									  const XML_Char ** attributes,
									  const XML_Char ** properties);

	UT_Bool					insertStrux(PT_DocPosition dpos,
										PTStruxType pts,
										const XML_Char ** attributes,
										const XML_Char ** properties);
	UT_Bool					deleteStrux(PT_DocPosition dpos);

	// the append- methods are only available while importing
	// the document.

	UT_Bool					appendStrux(PTStruxType pts, const XML_Char ** attributes);
	UT_Bool					appendFmt(const XML_Char ** attributes);
	UT_Bool					appendFmt(const UT_Vector * pVecAttributes);
	UT_Bool					appendSpan(UT_UCSChar * p, UT_uint32 length);

	UT_Bool					addListener(PL_Listener * pListener, PL_ListenerId * pListenerId);
	UT_Bool					removeListener(PL_ListenerId listenerId);
	
	void					dump(FILE * fp) const;
	
protected:
	const char *			m_szFilename;
	UT_Bool					m_bDirty;
	pt_PieceTable *			m_pPieceTable;
	UT_Vector				m_vecListeners;
};


#endif /* PD_DOCUMENT_H */
