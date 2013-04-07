/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2002 Marc Maurer (uwog@uwog.net)
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

/* See bug 1764
 * This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#ifndef IE_EXP_WP_H
#define IE_EXP_WP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ut_types.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_units.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ie_exp.h"
#include "ut_growbuf.h"
#include "ut_wctomb.h"
#include "pl_Listener.h"
#include "px_ChangeRecord.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "pp_AttrProp.h"
#include "pd_Document.h"
#include "pt_Types.h"
#include "ut_locale.h"

#include "xap_Frame.h"
#include "xap_EncodingManager.h"

#define MY_MB_LEN_MAX 6

#define PTR_TO_DOCUMENT_INDEX 4
#define PTR_TO_FILESIZE_INDEX 20

// The exporter for WordPerfect 6/7/8/9 documents.

class IE_Exp_WordPerfect_Sniffer : public IE_ExpSniffer
{
	friend class IE_Exp;
	friend class IE_Exp_WordPerfect;

public:
	IE_Exp_WordPerfect_Sniffer();
	virtual ~IE_Exp_WordPerfect_Sniffer();

	virtual bool			recognizeSuffix (const char * szSuffix);
	virtual bool			getDlgLabels (const char ** szDesc,
										  const char ** szSuffixList,
										  IEFileType * ft);
	virtual UT_Error		constructExporter (PD_Document * pDocument,
											   IE_Exp ** ppie);
};

class IE_Exp_WordPerfect : public IE_Exp
{
	friend class WordPerfect_Listener;

public:
	IE_Exp_WordPerfect(PD_Document * pDocument);
	~IE_Exp_WordPerfect();
protected:
	virtual PL_Listener *	_constructListener(void);
	virtual UT_Error		_writeDocument(void);
	UT_String *				m_buffer;
	UT_uint16				m_desiredFontUseCount;
private:
	UT_Error				_writeHeader();
	void					_UT_String_add(UT_String &s, int i);
	void					_UT_String_add(UT_String &s, short i);
	void					_UT_String_overwrite(UT_String &s, int pos, int i);
	void					_UT_String_add_chars(UT_String &s, char *c, int count);

	void					_handleGlobalOn();
	void					_handleGlobalOff();

	PL_Listener *			m_pListener;
	int						m_ptrToDocument;
	int						m_ptrDesiredFontUseCount;
};

class WordPerfect_Listener : public PL_Listener
{
public:
	WordPerfect_Listener(PD_Document * pDocument,
						 IE_Exp_WordPerfect * pie);
	virtual ~WordPerfect_Listener(){};

	virtual bool			populate(fl_ContainerLayout* sfh,
									 const PX_ChangeRecord * pcr);

	virtual bool			populateStrux(pf_Frag_Strux* sdh,
										  const PX_ChangeRecord * pcr,
										  fl_ContainerLayout* * psfh);

	virtual bool			change(fl_ContainerLayout* sfh,
								   const PX_ChangeRecord * pcr);

	virtual bool			insertStrux(fl_ContainerLayout* sfh,
										const PX_ChangeRecord * pcr,
										pf_Frag_Strux* sdh,
										PL_ListenerId lid,
										void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																PL_ListenerId lid,
																fl_ContainerLayout* sfhNew));

	virtual bool			signal(UT_uint32 iSignal);
protected:
	virtual int				_wctomb(char * pC, int & length, UT_UCS4Char wc) { return m_wctomb.wctomb(pC,length,wc); }
	virtual void			_outputData(const UT_UCSChar * p, UT_uint32 length);
	void					_closeBlock();
	void					_openBlock(PT_AttrPropIndex api);
	void					_openSpan(PT_AttrPropIndex api);
	void					_closeSpan();
	void					_handleVariableGroup(char group, char subgroup, char flags, short sizeNonDelData, char * nonDelData);
	void					_handleTabGroup(char tabDef);
	void					_handleAttributeOn(char attribute);
	void					_handleAttributeOff(char attribute);
	void					_handleFontSizeChange(double points);
	void					_handleParagraphJustification(char mode);
private:
	PD_Document *			m_pDocument;
	IE_Exp_WordPerfect *	m_pie;
	UT_Wctomb	 			m_wctomb;
	const PP_AttrProp*		m_pAP_Span;

	bool					m_bInBlock;
	char					m_paragraphJustification;
};

#endif /* IE_EXP_WP_H */
