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

#ifndef IE_EXP_MIF_H
#define IE_EXP_MIF_H

#include "ie_exp.h"
#include "pl_Listener.h"

class PD_Document;
class s_MIF_Listener;

// the exporter/writer for MIF

class ABI_EXPORT IE_Exp_MIF_Sniffer : public IE_ExpSniffer
{
	friend class IE_Exp;

public:
	IE_Exp_MIF_Sniffer (const char * name);
	virtual ~IE_Exp_MIF_Sniffer () {}

	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructExporter (PD_Document * pDocument,
										IE_Exp ** ppie);
};

class ABI_EXPORT IE_Exp_MIF : public IE_Exp
{
 public:
	IE_Exp_MIF(PD_Document *pDocument);
	virtual ~IE_Exp_MIF();
	
protected:
	virtual UT_Error	_writeDocument(void);

 private:	
	s_MIF_Listener *	m_pListener;
};

#endif /* IE_EXP_MIF_H */
