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


#ifndef IE_EXP_XML2PS_H
#define IE_EXP_XML2PS_H

#include "ie_exp.h"
#include "pl_Listener.h"
#include "ut_wctomb.h"

#define MY_MB_LEN_MAX 6

class PD_Document;
class XML2PS_Listener;

// The exporter/writer for Plain XML2PS Files.

class ABI_EXPORT IE_Exp_XML2PS_Sniffer : public IE_ExpSniffer
{
	friend class IE_Exp;

public:
	IE_Exp_XML2PS_Sniffer ();
	virtual ~IE_Exp_XML2PS_Sniffer () {}

	UT_Confidence_t supportsMIME (const char * szMIME);

	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructExporter (PD_Document * pDocument,
										IE_Exp ** ppie);
};

// The exporter/writer for Plain XML2PS Files with selectable encoding.

class ABI_EXPORT IE_Exp_EncodedXML2PS_Sniffer : public IE_ExpSniffer
{
	friend class IE_Exp;

public:
	IE_Exp_EncodedXML2PS_Sniffer ();
	virtual ~IE_Exp_EncodedXML2PS_Sniffer () {}

	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructExporter (PD_Document * pDocument,
										IE_Exp ** ppie);
};

class ABI_EXPORT IE_Exp_XML2PS : public IE_Exp
{
public:
	IE_Exp_XML2PS(PD_Document * pDocument, bool bEncoded=false);
	IE_Exp_XML2PS(PD_Document * pDocument, const char * encoding);
	virtual ~IE_Exp_XML2PS() {}

protected:
	virtual PL_Listener *	_constructListener(void);
	virtual UT_Error	_writeDocument(void);
	virtual bool		_openFile(const char * szFilename);
	bool				_doEncodingDialog(const char *szEncoding);
	void				_setEncoding(const char *szEncoding);

 private:
	PL_Listener *		m_pListener;
	bool				m_bIsEncoded;
	const char *		m_szEncoding;
	bool m_bExplicitlySetEncoding;
	bool				m_bIs16Bit;
	bool                m_bUnicode;
	bool				m_bBigEndian;
	bool				m_bUseBOM;
};

//////////////////////////////////////////////////////////////////
// a private listener class to help us translate the document
// into a XML2PS stream.
//////////////////////////////////////////////////////////////////

class ABI_EXPORT XML2PS_Listener : public PL_Listener
{
public:
	XML2PS_Listener(PD_Document * pDocument,
					IE_Exp_XML2PS * pie,
					bool bToClipboard = false,
					const char *szEncoding = 0,
					bool bIs16Bit = false,
 				    bool m_bUnicode = false,
					bool bUseBOM = false,
					bool bBigEndian = false);
	virtual ~XML2PS_Listener();

	virtual bool		populate(PL_StruxFmtHandle sfh,
								 const PX_ChangeRecord * pcr);

	virtual bool		populateStrux(PL_StruxDocHandle sdh,
									  const PX_ChangeRecord * pcr,
									  PL_StruxFmtHandle * psfh);

	virtual bool		change(PL_StruxFmtHandle sfh,
							   const PX_ChangeRecord * pcr);

	virtual bool		insertStrux(PL_StruxFmtHandle sfh,
									const PX_ChangeRecord * pcr,
									PL_StruxDocHandle sdh,
									PL_ListenerId lid,
									void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
															PL_ListenerId lid,
															PL_StruxFmtHandle sfhNew));

	virtual bool		signal(UT_uint32 iSignal);

protected:
	virtual int			_wctomb(char * pC, int & length, UT_UCS4Char wc) { return m_wctomb.wctomb(pC,length,wc); }
	void				_genBOM(void);
	void				_genLineBreak(void);
	virtual void		_outputData(const UT_UCSChar * p, UT_uint32 length);
	void				_closeBlock(void);
	void				_openBlock(PT_AttrPropIndex api);
	void				_closeFont(void);
	void				_openFont(PT_AttrPropIndex api);
	void                _handleDirMarker(PT_AttrPropIndex apiSpan);

 private:
	enum _dirOverride
	{
		DO_LTR,
		DO_RTL,
		DO_UNSET
	};
	
	

 	PD_Document *		m_pDocument;
	IE_Exp_XML2PS *		m_pie;
	UT_Wctomb 			m_wctomb;
	char				m_mbBOM[MY_MB_LEN_MAX];
	int					m_iBOMLen;
	char				m_mbLineBreak[MY_MB_LEN_MAX*2];
	int					m_iLineBreakLen;
	bool				m_bInBlock;
	bool				m_inFont;
	bool				m_inParagraph;
	bool				m_bToClipboard;
	bool				m_bFirstWrite;
	const char *		m_szEncoding;
	bool				m_bIs16Bit;
	bool				m_bBigEndian;
	bool                m_bUnicode;
	bool				m_bUseBOM;
	bool				m_bBreakExtra;
	_dirOverride        m_eDirOverride;
	_dirOverride        m_eDirMarkerPending;
	_dirOverride        m_eSectionDir;
	_dirOverride        m_eDocDir;
};

#endif /* IE_EXP_XML2PS_H */
