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

#ifndef IE_EXP_Applix_H
#define IE_EXP_Applix_H

#include "ie_exp.h"
#include "pl_Listener.h"

class PD_Document;
class s_Applix_Listener;

// the exporter/writer for Applix

class IE_Exp_Applix : public IE_Exp
{
 public:
        IE_Exp_Applix(PD_Document *pDocument);
	virtual ~IE_Exp_Applix();

	static bool		RecognizeSuffix(const char * szSuffix);
	static UT_Error 	StaticConstructor(PD_Document * pDocument,
						  IE_Exp ** ppie);
	static bool		GetDlgLabels(const char ** pszDesc,
					     const char ** pszSuffixList,
					     IEFileType * ft);
	static bool 		SupportsFileType(IEFileType ft);
	
protected:
	virtual UT_Error	_writeDocument(void);
	
	s_Applix_Listener *	m_pListener;
};

#endif /* IE_EXP_Applix_H */
