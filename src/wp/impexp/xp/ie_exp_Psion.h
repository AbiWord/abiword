/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2000,2001 Frodo Looijaard <frodol@dds.nl>
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


#ifndef IE_EXP_PSION_H
#define IE_EXP_PSION_H

#include "ie_exp.h"
#include "pl_Listener.h"
#include "psiconv/data.h"

class PD_Document;
class s_Psion_Listener;

// The exporter/writer for Psion Files.

class IE_Exp_Psion: public IE_Exp
{
	// This friend declaration needs to be cleaned up, I think.
	friend class s_Psion_Listener;

public:
	// Constructors and destructor
	IE_Exp_Psion(PD_Document * pDocument);
	virtual ~IE_Exp_Psion(void);

protected:
	// Overriding methods from the base class
	virtual UT_Error _writeDocument(void);

	// New data
	s_Psion_Listener * m_pListener;
	psiconv_text_and_layout m_paragraphs;

	// New methods
	virtual psiconv_file _createPsionFile(void) = 0;
};

class IE_Exp_Psion_TextEd : public IE_Exp_Psion
{
public:
	// Constructors and destructor
	IE_Exp_Psion_TextEd(PD_Document * pDocument);
	virtual ~IE_Exp_Psion_TextEd(void);

	// Overriding methods from the base class
	static UT_Bool RecognizeSuffix(const char * szSuffix);
	static UT_Error StaticConstructor(PD_Document * pDocument,
	                                  IE_Exp ** ppie);
	static UT_Bool GetDlgLabels(const char ** pszDesc,
	                           const char ** pszSuffixList,
	                           IEFileType * ft);
	static UT_Bool SupportsFileType(IEFileType ft);

protected:
	// New methods
	virtual psiconv_file _createPsionFile(void);
};

class IE_Exp_Psion_Word : public IE_Exp_Psion
{
public:
	// Constructors and destructor
	IE_Exp_Psion_Word(PD_Document * pDocument);
	virtual ~IE_Exp_Psion_Word(void);

	// Overriding methods from the base class
	static UT_Bool RecognizeSuffix(const char * szSuffix);
	static UT_Error StaticConstructor(PD_Document * pDocument,
	                                  IE_Exp ** ppie);
	static UT_Bool GetDlgLabels(const char ** pszDesc,
	                           const char ** pszSuffixList,
	                           IEFileType * ft);
	static UT_Bool SupportsFileType(IEFileType ft);

protected:
	// New methods
	virtual psiconv_file _createPsionFile(void);
};

#endif /* IE_EXP_PSION_H */
