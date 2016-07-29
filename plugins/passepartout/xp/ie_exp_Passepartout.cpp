/* AbiWord xml2ps export plugin
 * Copyright (C) 2004 David Bolack
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

#include <string.h>

#include "ut_string.h"
#include "ut_locale.h"
#include "pt_Types.h"
#include "fd_Field.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "ut_string_class.h"

#include "ie_exp_Passepartout.h"

/*****************************************************************/
/*****************************************************************/

class IE_Exp_Passepartout : public IE_Exp
{
public:
	IE_Exp_Passepartout(PD_Document * pDocument);
	virtual ~IE_Exp_Passepartout() {}

protected:
	virtual PL_Listener *	_constructListener(void);
	virtual UT_Error	_writeDocument(void);

 private:
	PL_Listener *		m_pListener;
};

//////////////////////////////////////////////////////////////////
// a private listener class to help us translate the document
// into a passepartout stream.
//////////////////////////////////////////////////////////////////

class Passepartout_Listener : public PL_Listener
{
public:
	Passepartout_Listener(PD_Document * pDocument,
			      IE_Exp_Passepartout * pie);
	virtual ~Passepartout_Listener();

	virtual bool		populate(fl_ContainerLayout* sfh,
								 const PX_ChangeRecord * pcr);

	virtual bool		populateStrux(pf_Frag_Strux* sdh,
									  const PX_ChangeRecord * pcr,
									  fl_ContainerLayout* * psfh);

	virtual bool		change(fl_ContainerLayout* sfh,
							   const PX_ChangeRecord * pcr);

	virtual bool		insertStrux(fl_ContainerLayout* sfh,
									const PX_ChangeRecord * pcr,
									pf_Frag_Strux* sdh,
									PL_ListenerId lid,
									void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
															PL_ListenerId lid,
															fl_ContainerLayout* sfhNew));

	virtual bool		signal(UT_uint32 iSignal);

protected:
	virtual void		_outputData(const UT_UCSChar * p, UT_uint32 length);
	void				_closeBlock(void);
	void				_openBlock(PT_AttrPropIndex api);
	void				_closeFont(void);
	void				_openFont(PT_AttrPropIndex api);

 private:

 	PD_Document *		m_pDocument;
	IE_Exp_Passepartout *		m_pie;
	bool				m_bInBlock;
	bool				m_inFont;
	bool				m_inParagraph;
        bool                 m_bWasSpace;
};

/*****************************************************************/
/*****************************************************************/

IE_Exp_Passepartout_Sniffer::IE_Exp_Passepartout_Sniffer (const char * /*name*/)
	: IE_ExpSniffer(IE_IMPEXPNAME_TEXT, true)
{
}

UT_Confidence_t IE_Exp_Passepartout_Sniffer::supportsMIME (const char * szMIME)
{
  if (strncmp (szMIME, "text/xml2ps", 11) == 0)
    return UT_CONFIDENCE_PERFECT;
  return UT_CONFIDENCE_ZILCH;
}

/*!
  Check filename extension for filetypes we support
 \param szSuffix Filename extension
 */
bool IE_Exp_Passepartout_Sniffer::recognizeSuffix(const char * szSuffix)
{
  return (!g_ascii_strcasecmp(szSuffix,".xml2ps"));
}

UT_Error IE_Exp_Passepartout_Sniffer::constructExporter(PD_Document * pDocument,
							IE_Exp ** ppie)
{
  *ppie = new IE_Exp_Passepartout(pDocument);
  return UT_OK;
}

bool IE_Exp_Passepartout_Sniffer::getDlgLabels(const char ** pszDesc,
									  const char ** pszSuffixList,
									  IEFileType * ft)
{
	*pszDesc = "PassepartoutToo (.xml2ps)";
	*pszSuffixList = "*.xml2ps";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

IE_Exp_Passepartout::IE_Exp_Passepartout(PD_Document * pDocument)
	: IE_Exp(pDocument),
	  m_pListener(NULL)	  
{
  m_error = UT_OK;
}

PL_Listener * IE_Exp_Passepartout::_constructListener(void)
{
	return new Passepartout_Listener(getDoc(),this);
}

UT_Error IE_Exp_Passepartout::_writeDocument(void)
{
	m_pListener = _constructListener();
	if (!m_pListener)
		return UT_IE_NOMEMORY;

	if (getDocRange())
		getDoc()->tellListenerSubset(static_cast<PL_Listener *>(m_pListener),getDocRange());
	else
		getDoc()->tellListener(static_cast<PL_Listener *>(m_pListener));
	DELETEP(m_pListener);

	return ((m_error) ? UT_IE_COULDNOTWRITE : UT_OK);
}

/*****************************************************************/
/*****************************************************************/

/*!
  Output text buffer to stream
 \param data Buffer to output
 \param length Size of buffer
 */
void Passepartout_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
{
  if (!m_bInBlock)
    return;
  
  UT_UTF8String sBuf;
  const UT_UCSChar * pData;
  
  sBuf.reserve(length);
  for (pData=data; (pData<data+length); /**/)
    {
      switch (*pData)
	{
	case '<':
	  sBuf += "&lt;";
	  pData++;
	  break;
	  
	case '>':
	  sBuf += "&gt;";
	  pData++;
	  break;
	  
	case '&':
	  sBuf += "&amp;";
	  pData++;
	  break;
	  
	case UCS_LF:					// LF -- representing a Forced-Line-Break
	  sBuf += "<br/>";
	  pData++;
	  break;
	  
	case ' ':
	case '\t':
	  if(m_bWasSpace)
	    {
	      sBuf += "&nbsp;";
	      pData++;
	    }
	  else
	    {
	      // just tack on a single space to the textrun
	      m_bWasSpace = true;
	      sBuf += " ";
	      pData++;
	    }
	  break;
	  
	default:
	  sBuf.appendUCS4(pData, 1);
	  pData++;
	}
    }
  
  m_pie->write(sBuf.utf8_str(),sBuf.byteLength());
}

void Passepartout_Listener::_closeBlock(void)
{
        if (m_inFont)
          _closeFont();

	if (!m_bInBlock)
		return;
	
	m_pie->write("</para>\n");

	m_bInBlock = false;
}

void Passepartout_Listener::_openBlock(PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = NULL;

	const char* pszLeftMargin = NULL;
	const char* pszRightMargin = NULL;
	const char* pszTopMargin = NULL;
	const char* pszBottomMargin = NULL;
        const char* pszFontFamily = NULL;
        const char* pszFontSize = NULL;
	const char* pszParaAlign = NULL;
	const char* pszParaLineHeight = NULL;

	if(m_bInBlock)
	  _closeBlock();

	m_bInBlock = true;

        UT_UTF8String TempStr;

        double pszLeftMarginDouble, pszRightMarginDouble, pszTopMarginDouble,pszBottomMarginDouble;

	bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);

//	Get a bunch of block level and other default properties.
//	Note, these don't seem to exist in the piecetable until someone alters them. need a
//	better way to get the defaults.

	pAP->getProperty("page-margin-left", (const gchar *&)pszLeftMargin);
	pAP->getProperty("page-margin-right", (const gchar *&)pszRightMargin);
	pAP->getProperty("page-margin-top", (const gchar *&)pszTopMargin);
	pAP->getProperty("page-margin-bottom", (const gchar *&)pszBottomMargin);
        pAP->getProperty("font-family", (const gchar *&) pszFontFamily);
        pAP->getProperty("font-size", (const gchar *&) pszFontSize );
	pAP->getProperty("text-align", (const gchar *&) pszParaAlign );
	pAP->getProperty("line-height", (const gchar *&) pszParaLineHeight );


// 	Insert defaults if we don't have any values. Note that these come from hardcoded values from pp_Property.cpp and
//	Probably should be found via a function/method at a later date.

        pszLeftMarginDouble   = UT_convertToPoints(pszLeftMargin);
        pszRightMarginDouble  = UT_convertToPoints(pszRightMargin);
        pszTopMarginDouble    = UT_convertToPoints(pszTopMargin);
        pszBottomMarginDouble = UT_convertToPoints(pszBottomMargin);

	if ( !pszLeftMargin )
	  pszLeftMarginDouble = 1;
	if ( !pszRightMargin )
	  pszRightMarginDouble = 1;
	if ( !pszTopMargin )
	  pszTopMarginDouble = 1;
	if ( !pszBottomMargin )
	  pszBottomMarginDouble = 1;

	UT_LocaleTransactor locale(LC_NUMERIC, "C");

	if (bHaveProp && pAP)
	{
		m_pie->write("<para");
                
                if ( pszFontFamily != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" font-family=\"%s\"", pszFontFamily );
                   m_pie->write(TempStr.utf8_str());
                 }
		else
                 {
                   TempStr = UT_UTF8String_sprintf(" font-family=\"%s\"", "Times New Roman" );
                   m_pie->write(TempStr.utf8_str());
                 }

                if ( pszFontSize != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" font-size=\"%s\"", pszFontSize );
                   m_pie->write(TempStr.utf8_str());
                 }
		else
                 {
                   TempStr = UT_UTF8String_sprintf(" font-size=\"%s\"", "12pt" );
                   m_pie->write(TempStr.utf8_str());
                 }

                if ( pszParaAlign != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" align=\"%s\"", pszParaAlign );
                   m_pie->write(TempStr.utf8_str());
                 }
		else
                 {
                   TempStr = UT_UTF8String_sprintf(" align=\"%s\"", "right" );
                   m_pie->write(TempStr.utf8_str());
                 }
		
                if ( pszParaLineHeight != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" line-height=\"%s\"", pszParaLineHeight );
                   m_pie->write(TempStr.utf8_str());
                 }
		else
                 {
                   TempStr = UT_UTF8String_sprintf(" line-height=\"%s\"", "1.0" );
                   m_pie->write(TempStr.utf8_str());
                 }

               	if( pszTopMargin != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" margin-top=\"%gpt\"", pszTopMarginDouble );
  		   m_pie->write(TempStr.utf8_str());
                 }
		if( pszBottomMargin != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" margin-bottom=\"%gpt\"", pszBottomMarginDouble );
  		   m_pie->write(TempStr.utf8_str());
                 }
		if( pszLeftMargin != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" margin-right=\"%gpt\"", pszRightMarginDouble );
  		   m_pie->write(TempStr.utf8_str());
                 }
		if( pszRightMargin != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" margin-left=\"%gpt\"", pszLeftMarginDouble );
  		   m_pie->write(TempStr.utf8_str());
                 }
                m_pie->write(">");
	}
        else
        {
		m_pie->write("<para>\n");
	}

}

void Passepartout_Listener::_closeFont(void)
{
	if (!m_inFont)
		return;

	m_pie->write("</font>");

	m_inFont = false;
}

void Passepartout_Listener::_openFont(PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = NULL;

        const char* pszFontFamily = NULL;
        const char* pszFontSize = NULL;

	if(m_inFont)
	  _closeFont();

        m_inFont = true;

        UT_UTF8String TempStr;

	bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);

//	Get a bunch of block level and other default properties.

        pAP->getProperty("font-family", (const gchar *&) pszFontFamily);
        pAP->getProperty("font-size", (const gchar *&) pszFontSize );

	if (bHaveProp && pAP)
	{
		m_pie->write("<font");
                
                if ( pszFontFamily != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" font-family=\"%s\"", pszFontFamily );
                   m_pie->write(TempStr.utf8_str());
                 }
		else
                 {
                   TempStr = UT_UTF8String_sprintf(" font-family=\"%s\"", "Times New Roman" );
                   m_pie->write(TempStr.utf8_str());
                 }

                if ( pszFontSize != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" font-size=\"%s\"", pszFontSize );
                   m_pie->write(TempStr.utf8_str());
                 }
		else
                 {
                   TempStr = UT_UTF8String_sprintf(" font-size=\"%s\"", "12pt" );
                   m_pie->write(TempStr.utf8_str());
                 }

                m_pie->write(">");
	}
        else
        {
		m_pie->write("<font>\n");
	}

}

/***************************************************************/
/***************************************************************/

Passepartout_Listener::Passepartout_Listener(PD_Document * pDocument,
					     IE_Exp_Passepartout * pie)
	: m_pDocument(pDocument),
	  m_pie(pie),
	  m_bInBlock(false),
	  m_bWasSpace(false)
{
	PT_AttrPropIndex api = m_pDocument->getAttrPropIndex();
	const PP_AttrProp * pAP = NULL;
	const char* pszLeftMargin = NULL;
	const char* pszRightMargin = NULL;
	const char* pszTopMargin = NULL;
	const char* pszBottomMargin = NULL;
        const char* pszFontFamily = NULL;
        const char* pszFontSize = NULL;

        UT_UTF8String TempStr;

        double pszLeftMarginDouble, pszRightMarginDouble, pszTopMarginDouble,pszBottomMarginDouble;

	bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);


	m_inFont	= false;
	m_inParagraph	= false;


//	Get a bunch of block level and other default properties.

	pAP->getProperty("margin-left", (const gchar *&)pszLeftMargin);
	pAP->getProperty("margin-right", (const gchar *&)pszRightMargin);
	pAP->getProperty("margin-top", (const gchar *&)pszTopMargin);
	pAP->getProperty("margin-bottom", (const gchar *&)pszBottomMargin);
        pAP->getProperty("font-family", (const gchar *&) pszFontFamily);
        pAP->getProperty("font-size", (const gchar *&) pszFontSize );


//	Manipulate them a little. Not sure this is needed.

        pszLeftMarginDouble   = UT_convertToPoints(pszLeftMargin);
        pszRightMarginDouble  = UT_convertToPoints(pszRightMargin);
        pszTopMarginDouble    = UT_convertToPoints(pszTopMargin);
        pszBottomMarginDouble = UT_convertToPoints(pszBottomMargin);

        m_pie->write("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");

	UT_LocaleTransactor locale(LC_NUMERIC, "C");

	if (bHaveProp && pAP)
	{
		m_pie->write("<block-container");
                
                if ( pszFontFamily != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" font-family=\"%s\"", pszFontFamily );
                   m_pie->write(TempStr.utf8_str());
                 }
                if ( pszFontSize != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" font-size=\"%s\"", pszFontSize );
                   m_pie->write(TempStr.utf8_str());
                 }
		if( pszTopMargin != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" margin-top=\"%gpt\"", pszTopMarginDouble );
  		   m_pie->write(TempStr.utf8_str());
                 }
		if( pszBottomMargin != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" margin-bottom=\"%gpt\"", pszBottomMarginDouble );
  		   m_pie->write(TempStr.utf8_str());
                 }
		if( pszRightMargin != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" margin-right=\"%gpt\"", pszRightMarginDouble );
  		   m_pie->write(TempStr.utf8_str());
                 }
		if( pszLeftMargin != NULL )
                 {
                   TempStr = UT_UTF8String_sprintf(" margin-left=\"%gpt\"", pszLeftMarginDouble );
  		   m_pie->write(TempStr.utf8_str());
                 }
	                                   
                m_pie->write(">");
	}
        else
        {
		m_pie->write("<block-container>\n");
	}
}

Passepartout_Listener::~Passepartout_Listener()
{
  _closeBlock();
  m_pie->write("</block-container>\n");
}

/***************************************************************/
/***************************************************************/

bool Passepartout_Listener::populate(fl_ContainerLayout* /*sfh*/,
								  const PX_ChangeRecord * pcr)
{
	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *>(pcr);

			PT_AttrPropIndex api = pcr->getIndexAP();
			_closeFont();
			_openFont(api);

			PT_BufIndex bi = pcrs->getBufIndex();
			const UT_UCS4Char * pData = m_pDocument->getPointer(bi);
			_outputData(pData,pcrs->getLength());

			return true;
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return true;

	default:
		UT_ASSERT_HARMLESS(UT_TODO);
		return true;
	}
}

bool Passepartout_Listener::populateStrux(pf_Frag_Strux* /*sdh*/,
									   const PX_ChangeRecord * pcr,
									   fl_ContainerLayout* * psfh)
{
	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *>(pcr);
	*psfh = 0;							// we don't need it.

	switch (pcrx->getStruxType())
	{
	case PTX_SectionEndnote:
	case PTX_SectionHdrFtr:
	case PTX_Section:
		{
			_closeBlock();
			PT_AttrPropIndex api = pcr->getIndexAP();
			const PP_AttrProp * pAP = NULL;
			bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);

			if (bHaveProp && pAP)
			{
			}
			return true;
		}

	case PTX_Block:
		{
			PT_AttrPropIndex api = pcr->getIndexAP();
			_closeBlock();
			_openBlock(api);
			m_bInBlock = true;
                        return true;
		}

		// Be nice about these until we figure out what to do with 'em
	case PTX_SectionTable:
	case PTX_SectionCell:
	case PTX_EndTable:
	case PTX_EndCell:
	case PTX_EndFrame:
	case PTX_EndMarginnote:
	case PTX_EndFootnote:
	case PTX_SectionFrame:
	case PTX_SectionMarginnote:
	case PTX_SectionFootnote:
	case PTX_EndEndnote:
	    return true ;

	default:
		UT_ASSERT_HARMLESS(UT_TODO);
		return true;
	}
}

bool Passepartout_Listener::change(fl_ContainerLayout* /*sfh*/,
								const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT_NOT_REACHED();						// this function is not used.
	return false;
}

bool Passepartout_Listener::insertStrux(fl_ContainerLayout* /*sfh*/,
									 const PX_ChangeRecord * /*pcr*/,
									 pf_Frag_Strux* /*sdh*/,
									 PL_ListenerId /* lid */,
									 void (* /*pfnBindHandles*/)(pf_Frag_Strux* /* sdhNew */,
																 PL_ListenerId /* lid */,
																 fl_ContainerLayout* /* sfhNew */))
{
	UT_ASSERT_NOT_REACHED();						// this function is not used.
	return false;
}

bool Passepartout_Listener::signal(UT_uint32 /* iSignal */)
{
  UT_ASSERT_NOT_REACHED();
	return false;
}

