#ifndef IE_EXP_BZ2ABW_H
#define IE_EXP_BZ2ABW_H

#include <stdio.h>
#include <bzlib.h>

#include "ie_exp_AbiWord_1.h"

class ABI_EXPORT IE_Exp_BZ2AbiWord_Sniffer : public IE_ExpSniffer
{
	friend class IE_Exp;

public:
	IE_Exp_BZ2AbiWord_Sniffer () {}
	virtual ~IE_Exp_BZ2AbiWord_Sniffer () {}

	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
				   const char ** szSuffixList,
				   IEFileType * ft);
	virtual UT_Error constructExporter (PD_Document * pDocument,
					    IE_Exp ** ppie);
};


class ABI_EXPORT IE_Exp_BZ2AbiWord : public IE_Exp_AbiWord_1
{
public:
	IE_Exp_BZ2AbiWord(PD_Document * pDocument);
	virtual ~IE_Exp_BZ2AbiWord();

protected:
	bool				_openFile(const char * szFilename);
	UT_uint32			_writeBytes(const UT_Byte * pBytes, UT_uint32 length);
	bool				_closeFile(void);

private:
	FILE   *m_fp;
	BZFILE *m_bzout;
};

#endif
