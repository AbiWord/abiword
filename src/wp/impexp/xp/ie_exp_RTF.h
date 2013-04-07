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


#ifndef IE_EXP_RTF_H
#define IE_EXP_RTF_H

#include "ie_exp.h"
#include "ut_vector.h"
#include "ut_hash.h"
#include "ut_misc.h"
#include "ut_iconv.h"
#include "pl_Listener.h"
#include "fl_AutoLists.h"
#include "fl_AutoNum.h"

class PD_Document;
class PD_Style;
class PP_AttrProp;
class s_RTF_ListenerWriteDoc;
class s_RTF_ListenerGetProps;
class s_RTF_AttrPropAdapter;
class s_RTF_AttrPropAdapter_Style;
class ie_exp_RTF_MsWord97List;
class ie_exp_RTF_MsWord97ListSimple;
class ie_exp_RTF_MsWord97ListMulti;
class ie_exp_RTF_ListOveride;
class _rtf_font_info;

// The exporter/writer for RTF file format (based upon spec version 1.5).

class ABI_EXPORT IE_Exp_RTF_Sniffer : public IE_ExpSniffer
{
	friend class IE_Exp;

public:
	IE_Exp_RTF_Sniffer ();
	virtual ~IE_Exp_RTF_Sniffer () {}

	UT_Confidence_t supportsMIME (const char * szMIME);

	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructExporter (PD_Document * pDocument,
										IE_Exp ** ppie);
};

/*
 * this is for exporting to RTF understood by attic software
 * like WordPad and probably Word6.0.
 */
class ABI_EXPORT IE_Exp_RTF_attic_Sniffer : public IE_ExpSniffer
{
	friend class IE_Exp;

public:
	IE_Exp_RTF_attic_Sniffer ();
	virtual ~IE_Exp_RTF_attic_Sniffer () {}

	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructExporter (PD_Document * pDocument,
										IE_Exp ** ppie);
};

// hack for "msword" export

class ABI_EXPORT IE_Exp_MsWord_Hack_Sniffer : public IE_ExpSniffer
{
	friend class IE_Exp;

public:
	IE_Exp_MsWord_Hack_Sniffer ();
	virtual ~IE_Exp_MsWord_Hack_Sniffer () {}

	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
				   const char ** szSuffixList,
				   IEFileType * ft);
	virtual UT_Error constructExporter (PD_Document * pDocument,
					    IE_Exp ** ppie);
};

struct NumberedStyle;

class ABI_EXPORT IE_Exp_RTF : public IE_Exp
{
	friend class s_RTF_ListenerWriteDoc;
	friend class s_RTF_ListenerGetProps;

public:
	IE_Exp_RTF(PD_Document * pDocument);
	IE_Exp_RTF(PD_Document * pDocument,bool atticFormat);
	virtual ~IE_Exp_RTF();
	ie_exp_RTF_MsWord97ListMulti * getNthMultiLevel(UT_uint32 i) const;
	ie_exp_RTF_MsWord97ListSimple * getNthSimple(UT_uint32 i) const;
	ie_exp_RTF_ListOveride * getNthOveride(UT_uint32 i) const;
	UT_uint32 getMultiLevelCount(void) const;
	UT_uint32 getSimpleListCount(void) const;
	UT_uint32 getOverideCount(void)  const;
	UT_uint32 getMatchingOverideNum(UT_uint32 ID);
	void exportHdrFtr(const char * pszHdrFtr , const char * pszHdrFtrID,const char * pszKeyword);
	UT_BidiCharType isCharRTL() {return m_CharRTL;}
	void setCharRTL(UT_BidiCharType t) {m_CharRTL = t;}
	void     setByteBuf(UT_ByteBuf * pBuf)
	{ _setByteBuf(pBuf);}
	UT_ByteBuf * getByteBuf(void)
	{ return _getByteBuf();}
protected:
	virtual UT_Error	_writeDocument(void);
	UT_Error	        _writeDocumentLocal(bool bSkipHeader);
	UT_sint32			_findColor(const char * szColor) const;
	UT_sint32           _findOrAddColor (const char * szColor);
	void				_addColor(const char * szColor);
	void				_rtf_open_brace(void);
	void				_rtf_close_brace(void);
	bool                _rtf_reopen_brace(void);
	void				_rtf_keyword(const char * szKey);
	void				_rtf_keyword(const char * szKey, UT_sint32 d);
	void				_rtf_keyword_space(const char * szKey, UT_sint32 d);
	void                _rtf_keyword(const char * szKey, const char * szValue);
	void				_rtf_nonascii_hex2(UT_sint32 d);
	void				_rtf_nonascii_hex2(UT_sint32 d, UT_String & pStr);
	void				_rtf_keyword_hex2(const char * szKey, UT_sint32 d);
	void				_rtf_keyword_ifnotdefault(const char * szKey, const char * szValue, UT_sint32 defaultValue);
	void				_rtf_keyword_ifnotdefault_twips(const char * szKey, const char * szValue, UT_sint32 defaultValue);
	void				_rtf_semi(void);
	void				_rtf_fontname(const char * szFontName);
	void				_rtf_chardata(const char * pbuf, UT_uint32 buflen);
	void				_rtf_chardata(const std::string& buf);
	void				_rtf_pcdata(UT_UTF8String &sPCData, bool bSupplyUC=false, UT_uint32 iAltChars=1);
	void				_rtf_pcdata(const std::string & szPCData, bool bSupplyUC=false, UT_uint32 iAltChars=1);
	void				_rtf_pcdata(const char * szPCData, bool bSupplyUC=false, UT_uint32 iAltChars=1);
	void				_rtf_nl(void);
	const gchar *    _getStyleProp(s_RTF_AttrPropAdapter_Style * pADStyle,
									  const s_RTF_AttrPropAdapter * apa,
									  const char * szProp);

	bool				_write_rtf_header(void);
	bool				_write_rtf_trailer(void);

	void                            _clearStyles();
	void                            _selectStyles();
	UT_uint32                       _getStyleNumber(const PD_Style * pStyle);
	UT_uint32                       _getStyleNumber(const gchar * szStyleName);

	void                            _write_prop_ifnotdefault(const PD_Style * pStyle, const gchar * szPropName, const char * szRTFName);
	void                            _write_prop_ifyes(const PD_Style * pStyle, const gchar * szPropName, const char * szRTFName);
	void                            _write_tabdef(const char * szTabStops);
	void                            _write_charfmt(const s_RTF_AttrPropAdapter &);

	void                            _write_parafmt(const PP_AttrProp * pSpanAP,
												   const PP_AttrProp * pBlockAP,
												   const PP_AttrProp * pSectionAP,
												   bool & bStartedList, pf_Frag_Strux* sdh, UT_uint32 &iCurrID,
												   bool &bIsListBlock, UT_sint32 iNestLevel);

	void                            _write_style_fmt(const PD_Style *);
	void                            _write_stylesheets(void);
	void                            _write_listtable(void);
    void                            _output_MultiLevelRTF(ie_exp_RTF_MsWord97ListMulti * pMulti);
    void                            _output_SimpleListRTF(ie_exp_RTF_MsWord97ListSimple * pSimple);
    void                            _output_OveridesRTF(ie_exp_RTF_ListOveride * pOver, UT_uint32 i);
	void                            _output_ListRTF(fl_AutoNum * pAuto, UT_uint32 iLevel);
	void                            _output_LevelText(fl_AutoNum * pAuto, UT_uint32 iLevel,UT_UCSChar bulletsym);
	void                            _get_LeftRight_Side(UT_String & LeftSide, UT_String & RightSide);
	void                            _generate_level_Text(fl_AutoNum * pAuto,UT_String & LevelText,UT_String &LevelNumbers, UT_uint32 & lenText, UT_uint32 & ifoundLevel);

	void                            _output_revision(const s_RTF_AttrPropAdapter & apa, bool bPara,pf_Frag_Strux* sdh,
													 UT_sint32 iNestLevel, bool & bStartedList,  bool &bIsListBlock,
													 UT_uint32 &iCurrID);

	UT_sint32			_findFont(const s_RTF_AttrPropAdapter * apa) const;
	UT_sint32			_findFont(const _rtf_font_info * pfi) const;
	void				_addFont(const _rtf_font_info * pfi);

 private:
	static bool s_escapeString(UT_UTF8String &sOutStr, UT_UCS4String &sInStr,
	                           UT_uint32 iAltChars);
	static bool s_escapeString(UT_UTF8String &sOutStr, const char * szInStr,
                               UT_uint32 iSize, UT_uint32 iAltChars);
	static bool s_escapeString( std::string& outStr, const std::string& inStr,
	                           UT_uint32 iAltChars = 1 );
	static std::string s_escapeString( const std::string& inStr, UT_uint32 iAltChars = 1 );
    static std::string s_escapeXMLString( const std::string& inStr );

	s_RTF_ListenerWriteDoc *	m_pListenerWriteDoc;
	UT_Vector					m_vecColors;			/* vector of "const char * szColor" */
	UT_Vector					m_vecFonts;				/* vector of struct _font */
	bool						m_bNeedUnicodeText;		/* doc has unicode chars */
	UT_sint32					m_braceLevel;			/* nesting depth of {} braces */
	bool						m_bLastWasKeyword;		/* just wrote a keyword, so need space before text data */
	bool						m_atticFormat; 		/* whether to use unicode for all characters >0xff or convert to native windows encoding*/
	UT_GenericStringMap<NumberedStyle*> m_hashStyles;
	/* Hash containing styles to be exported. The key is the
	   AbiWord style name. The value is a NumberedStyle object
	   (see the cpp file). */
	UT_Vector                   m_vecMultiLevel;
	UT_Vector                   m_vecSimpleList;
	UT_Vector                   m_vecOverides;

	UT_BidiCharType             m_CharRTL;
	UT_iconv_t                  m_conv;
};

/*****************************************************************/
/*****************************************************************/

/* This struct contains the RTF font info as needed for the
   font table. */
class ABI_EXPORT _rtf_font_info
{
public:
    _rtf_font_info();
	bool init(const s_RTF_AttrPropAdapter & apa, bool bDoFieldFont = false);
    bool init(const char * szfontName);
	virtual ~_rtf_font_info(void);
    bool _is_same(const _rtf_font_info & fi) const;
	const char * getFontFamily(void) const { return szFamily;}
	const char * getFontName(void) const { return m_szName.c_str();}
	int getFontCharset(void) const { return nCharset;}
	int getFontPitch(void) const { return nPitch;}
	bool isTrueType(void) const { return fTrueType;}
private:
    const gchar * szFamily;
    int nCharset;
    int nPitch;
    UT_String m_szName;
    bool fTrueType;
};

class ABI_EXPORT ie_exp_RTF_MsWord97List
{
 public:
	ie_exp_RTF_MsWord97List(fl_AutoNum * pAuto);
	virtual ~ie_exp_RTF_MsWord97List(void);
	fl_AutoNum * getAuto(void) const { return m_pAutoNum;}
	UT_uint32 getID(void) const {return m_Id;}
 private:
	fl_AutoNum * m_pAutoNum;
	UT_uint32 m_Id;
};

class ABI_EXPORT ie_exp_RTF_MsWord97ListSimple : public ie_exp_RTF_MsWord97List
{
 public:
	ie_exp_RTF_MsWord97ListSimple(fl_AutoNum * pAuto);
	~ie_exp_RTF_MsWord97ListSimple(void);
	bool isSimple(void) const { return true;}
	bool isMulti(void) const { return false;}
 private:
};

class ABI_EXPORT ie_exp_RTF_MsWord97ListMulti : public ie_exp_RTF_MsWord97List
{
 public:
	ie_exp_RTF_MsWord97ListMulti(fl_AutoNum * pAuto);
	~ie_exp_RTF_MsWord97ListMulti(void);
	bool isSimple(void) const { return false;}
	bool isMulti(void) const { return true;}
	void addLevel(UT_uint32 iLevel, ie_exp_RTF_MsWord97List * pMsWord97List);
	ie_exp_RTF_MsWord97List * getListAtLevel(UT_uint32 iLevel, UT_uint32 nthList);
	UT_uint32 getMatchingID(UT_uint32 listID);
 private:
	UT_Vector * m_vLevels[9];
};

class ABI_EXPORT ie_exp_RTF_ListOveride
{
public:
	ie_exp_RTF_ListOveride(fl_AutoNum * pAuto);
	~ie_exp_RTF_ListOveride(void);
	void setOverideID(UT_uint32 ID) {m_OverideID = ID;}
	UT_uint32 getOverideID(void) const { return m_OverideID;}
	fl_AutoNum * getAutoNum(void) const { return m_pAutoNum;}
	bool doesOverideMatch(UT_uint32 ID) const { return (ID == m_AbiListID);}
	UT_uint32 getAbiListID(void) const { return m_AbiListID;}
private:
	UT_uint32 m_AbiListID;
	UT_uint32 m_OverideID;
	fl_AutoNum * m_pAutoNum;
};
#endif /* IE_EXP_RTF_H */







