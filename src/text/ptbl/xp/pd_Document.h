
#ifndef PD_DOCUMENT_H
#define PD_DOCUMENT_H

// PD_Document is the representation for a document.

// TODO should the filename be UT_CSChar rather than char ?

class PD_Document
{
public:
	PD_Document();
	~PD_Document();

	UT_Bool					readFromFile(const char * szFilename);
	UT_Bool					newDocument(void);
	UT_Bool					isDirty(void) const;
	void					setClean(void);

	void					dump(FILE * fp) const;
	
protected:
	char *					m_szFilename;
	UT_Bool					m_bDirty;
};


#endif /* PD_DOCUMENT_H */
