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

#ifndef IE_EXP_UTF8_H
#define IE_EXP_UTF8_H

#include "ie_exp.h"
#include "pl_Listener.h"
class PD_Document;
class s_UTF8_Listener;

// The exporter/writer for Plain Text Files in UTF8.

class IE_Exp_UTF8_Sniffer : public IE_ExpSniffer
{
	friend class IE_Exp;

public:
	IE_Exp_UTF8_Sniffer () {}
	virtual ~IE_Exp_UTF8_Sniffer () {}

	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructExporter (PD_Document * pDocument,
										IE_Exp ** ppie);
};

class IE_Exp_UTF8 : public IE_Exp
{
public:
	IE_Exp_UTF8(PD_Document * pDocument);
	virtual ~IE_Exp_UTF8();
	
protected:
	virtual UT_Error	_writeDocument(void);
	
	s_UTF8_Listener *	m_pListener;
};

#endif /* IE_EXP_UTF8_H */
