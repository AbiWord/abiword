
#ifndef IE_EXP_ABIWORD_1_H
#define IE_EXP_ABIWORD_1_H

#include "ie_exp.h"
#include "pl_Listener.h"
class PD_Document;
class ie_Exp_Listener;

// The exporter/writer for AbiWord file format version 1.

class IE_Exp_AbiWord_1 : public IE_Exp
{
public:
	IE_Exp_AbiWord_1(PD_Document * pDocument);
	~IE_Exp_AbiWord_1();

	virtual IEStatus	writeFile(const char * szFilename);
	void				write(const char * sz);
	void				write(const char * sz, UT_uint32 length);

protected:
	IEStatus			_writeDocument(void);
	
	ie_Exp_Listener *	m_pListener;
	PL_ListenerId		m_lid;

public:
	UT_Bool				m_error;
};

#endif /* IE_EXP_ABIWORD_1_H */
