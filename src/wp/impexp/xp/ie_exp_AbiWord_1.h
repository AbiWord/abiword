 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

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
	virtual ~IE_Exp_AbiWord_1();

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
