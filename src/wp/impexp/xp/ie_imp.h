
#ifndef IE_IMP_H
#define IE_IMP_H

#include "ut_types.h"
class PD_Document;

// IE_Imp defines the abstract base class for file importers.

class IE_Imp
{
public:
	typedef enum _IEStatus { IES_OK,
							 IES_Error,
							 IES_FileNotFound,
							 IES_NoMemory,
							 IES_UnknownType,
							 IES_BogusDocument } IEStatus;

	// constructs an importer of the right type based upon
	// either the filename or sniffing the file.  caller is
	// responsible for destroying the importer when finished
	// with it.
	
	static IEStatus		constructImporter(PD_Document * pDocument,
										  const char * szFilename,
										  IE_Imp ** ppie);

protected:
	IE_Imp(PD_Document * pDocument);
public:
	virtual ~IE_Imp();
	virtual IEStatus	importFile(const char * szFilename) = 0;

protected:
	PD_Document *		m_pDocument;
};

#endif /* IE_IMP_H */
