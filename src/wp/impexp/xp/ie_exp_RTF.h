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


#ifndef IE_EXP_RTF_H
#define IE_EXP_RTF_H

#include "ie_exp.h"
#include "ut_vector.h"
#include "ut_misc.h"
#include "pl_Listener.h"
class PD_Document;
class PP_AttrProp;
class s_RTF_ListenerWriteDoc;
class s_RTF_ListenerGetProps;
class _rtf_font_info;

// The exporter/writer for RTF file format (based upon spec version 1.5).

class IE_Exp_RTF : public IE_Exp
{
	friend class s_RTF_ListenerWriteDoc;
	friend class s_RTF_ListenerGetProps;

public:
	IE_Exp_RTF(PD_Document * pDocument);
	virtual ~IE_Exp_RTF();

	static UT_Bool		RecognizeSuffix(const char * szSuffix);
	static UT_Error		StaticConstructor(PD_Document * pDocument,
										  IE_Exp ** ppie);
	static UT_Bool		GetDlgLabels(const char ** pszDesc,
									 const char ** pszSuffixList,
									 IEFileType * ft);
	static UT_Bool 		SupportsFileType(IEFileType ft);
	
protected:
	virtual UT_Error	_writeDocument(void);

	UT_sint32			_findColor(const char * szColor) const;
	void				_addColor(const char * szColor);
	void				_rtf_open_brace(void);
	void				_rtf_close_brace(void);
	void				_rtf_keyword(const char * szKey);
	void				_rtf_keyword(const char * szKey, UT_sint32 d);
	void				_rtf_nonascii_hex2(UT_sint32 d);
	void				_rtf_keyword_hex2(const char * szKey, UT_sint32 d);
	void				_rtf_keyword_ifnotdefault(const char * szKey, const char * szValue, UT_sint32 defaultValue);
	void				_rtf_keyword_ifnotdefault_twips(const char * szKey, const char * szValue, UT_sint32 defaultValue);
	void				_rtf_semi(void);
	void				_rtf_fontname(const char * szFontName);
	void				_rtf_chardata(const char * pbuf, UT_uint32 buflen);
	void				_rtf_nl(void);
	UT_Bool				_write_rtf_header(void);
	UT_Bool				_write_rtf_trailer(void);
	UT_sint32			_findFont(const _rtf_font_info * pfi) const;
	void				_addFont(const _rtf_font_info * pfi);
	void				_rtf_compute_font_properties(const _rtf_font_info * pfi,
													 const char ** p_sz_font_name,
													 const char ** p_sz_rtf_family,
													 int * p_rtf_pitch,
													 UT_Bool * pbTrueType) const;
	
	s_RTF_ListenerWriteDoc *	m_pListenerWriteDoc;
	s_RTF_ListenerGetProps *	m_pListenerGetProps;
	UT_Vector					m_vecColors;			/* vector of "const char * szColor" */
	UT_Vector					m_vecFonts;				/* vector of struct _font */
	UT_Bool						m_bNeedUnicodeText;		/* doc has unicode chars */
	UT_sint32					m_braceLevel;			/* nesting depth of {} braces */
	UT_Bool						m_bLastWasKeyword;		/* just wrote a keyword, so need space before text data */
};

/*****************************************************************/
/*****************************************************************/

class _rtf_font_info
{
public:
	_rtf_font_info(const PP_AttrProp * pSpanAP,
				   const PP_AttrProp * pBlockAP,
				   const PP_AttrProp * pSectionAP)
		{
			m_pSpanAP = pSpanAP;
			m_pBlockAP = pBlockAP;
			m_pSectionAP = pSectionAP;
		}

	~_rtf_font_info(void)
		{
		}

	UT_Bool _is_same(const PP_AttrProp * pSpanAP,
					 const PP_AttrProp * pBlockAP,
					 const PP_AttrProp * pSectionAP) const
		{
			return ((pSpanAP==m_pSpanAP) && (pBlockAP==m_pBlockAP) && (pSectionAP==m_pSectionAP));
		}

	UT_Bool _is_same(const _rtf_font_info * pfi) const
		{
			return _is_same(pfi->m_pSpanAP,pfi->m_pBlockAP,pfi->m_pSectionAP);
		}
	
	const PP_AttrProp * m_pSpanAP;
	const PP_AttrProp * m_pBlockAP;
	const PP_AttrProp * m_pSectionAP;
};

#endif /* IE_EXP_RTF_H */
