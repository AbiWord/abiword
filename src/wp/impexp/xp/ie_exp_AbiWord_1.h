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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


#ifndef IE_EXP_ABIWORD_1_H
#define IE_EXP_ABIWORD_1_H

#include "ie_exp_XML.h"
#include "pl_Listener.h"
class PD_Document;
class s_AbiWord_1_Listener;

// The exporter/writer for AbiWord file format version 1.

class ABI_EXPORT IE_Exp_AbiWord_1_Sniffer : public IE_ExpSniffer
{
	friend class IE_Exp;

public:
	IE_Exp_AbiWord_1_Sniffer ();
	virtual ~IE_Exp_AbiWord_1_Sniffer () {}

	UT_Confidence_t supportsMIME (const char * szMIME);

	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructExporter (PD_Document * pDocument,
										IE_Exp ** ppie);
};

class ABI_EXPORT IE_Exp_AbiWord_1 : public IE_Exp_XML
{
public:
	IE_Exp_AbiWord_1(PD_Document * pDocument, bool isTemplate = false, bool isCompressed = false);
	virtual ~IE_Exp_AbiWord_1();

protected:
	virtual UT_Error	_writeDocument(void);

private:
	bool m_bIsTemplate;
	bool m_bIsCompressed;
	s_AbiWord_1_Listener *	m_pListener;
};

#endif /* IE_EXP_ABIWORD_1_H */
