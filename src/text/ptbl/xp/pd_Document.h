
#ifndef PD_DOCUMENT_H
#define PD_DOCUMENT_H

#include <stdio.h>
#include "ut_types.h"
#include "pt_PieceTable.h"

// PD_Document is the representation for a document.

// TODO should the filename be UT_UCSChar rather than char ?

class PD_Document : public PT_PieceTable
{
public:
	PD_Document();
	~PD_Document();

	UT_Bool					readFromFile(const char * szFilename);
	UT_Bool					newDocument(void);
	const char *			getFilename(void) const;
	UT_Bool					isDirty(void) const;
	void					setClean(void);

	void					dump(FILE * fp) const;
	
protected:
	const char *			m_szFilename;
	UT_Bool					m_bDirty;
};


#endif /* PD_DOCUMENT_H */
