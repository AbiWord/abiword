/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


#ifndef IE_EXP_ABIWORD_1_H
#define IE_EXP_ABIWORD_1_H

#include "ie_exp.h"
#include "pl_Listener.h"
class PD_Document;
class s_AbiWord_1_Listener;

// The exporter/writer for AbiWord file format version 1.

class IE_Exp_AbiWord_1 : public IE_Exp
{
public:
	IE_Exp_AbiWord_1(PD_Document * pDocument);
	virtual ~IE_Exp_AbiWord_1();

	virtual IEStatus	writeFile(const char * szFilename);
	void				write(const char * sz);
	void				write(const char * sz, UT_uint32 length);

	static UT_Bool		RecognizeSuffix(const char * szSuffix);
	static IEStatus		StaticConstructor(PD_Document * pDocument,
										  IE_Exp ** ppie);
	static UT_Bool		GetDlgLabels(const char ** pszDesc,
									 const char ** pszSuffixList,
									 IEFileType * ft);
	static UT_Bool 		SupportsFileType(IEFileType ft);
	
protected:
	IEStatus			_writeDocument(void);
	
	s_AbiWord_1_Listener *	m_pListener;
	PL_ListenerId		m_lid;

public:
	UT_Bool				m_error;
};

#endif /* IE_EXP_ABIWORD_1_H */
