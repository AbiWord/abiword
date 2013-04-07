/* -*- c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 2004 Hubert Figuiere
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


#ifndef _IE_IMP_RTFPARSE_H_
#define _IE_IMP_RTFPARSE_H_

#define MAX_KEYWORD_LEN 256

class IE_Imp_RTF;


typedef enum {
	CONTEXT_FRAME,
	CONTEXT_FONT_FORM_PROP,
	CONTEXT_STYLE_SHEET,
	NO_CONTEXT
} RTF_CONTEXT;

#include "ie_imp_RTFKeywordIDs.h"

typedef struct {
	const char * keyword;
	bool hasParam;
	bool longParam;
	RTF_CONTEXT  context;
	RTF_KEYWORD_ID id;
} _rtf_keyword;



/*!
  This class is meant to implement callback for a standard group parser.

  \note even though the methods could have been made as pure virtual, they
  are not to ease subclassing parsers by "just what we need"
 */
class ABI_EXPORT IE_Imp_RTFGroupParser
{
public:
	IE_Imp_RTFGroupParser()
		: m_nested(1) {}
	virtual ~IE_Imp_RTFGroupParser()
		{ }
	/// called when parsing is in error
	virtual bool tokenError(IE_Imp_RTF * ie);
	/// called to parse a KeyWord
	virtual bool tokenKeyword(IE_Imp_RTF * ie, RTF_KEYWORD_ID kwID,
							  UT_sint32 param, bool paramUsed);
	/// called when a brace opens. Must be called by override
	virtual bool tokenOpenBrace(IE_Imp_RTF * ie);
	/// called when a brace close. Must be called by override
	virtual bool tokenCloseBrace(IE_Imp_RTF * ie);
	/// called to parse PCDATA to an UT_UTF8String
	virtual bool tokenData(IE_Imp_RTF * ie, UT_UTF8String & data);

	/// called at the end of the group parsing to finalize stuff
	virtual bool finalizeParse(void);
	/// return the nesting level
	int nested(void) const
		{ return m_nested; }
protected:
	/// Netsing level, increase when a brace opens and decrease
	/// when a brace close
	int m_nested;
};

#endif
