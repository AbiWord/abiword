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

#include <stdlib.h>
#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "ut_misc.h"
#include "ut_units.h"
#include "pt_Types.h"
#include "ie_exp_RTF.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "xap_App.h"
#include "pd_Style.h"

/*****************************************************************/
/*****************************************************************/

#include "ie_exp_RTF_listenerWriteDoc.h"
#include "ie_exp_RTF_listenerGetProps.h"

/*****************************************************************/
/*****************************************************************/

IE_Exp_RTF::IE_Exp_RTF(PD_Document * pDocument)
	: IE_Exp(pDocument)
{
	m_error = 0;
	m_pListenerWriteDoc = NULL;
	m_pListenerGetProps = NULL;
	m_lid = 0;
	m_bNeedUnicodeText = UT_FALSE;
	m_braceLevel = 0;
	m_bLastWasKeyword = UT_FALSE;
}

IE_Exp_RTF::~IE_Exp_RTF()
{
	UT_VECTOR_PURGEALL(char *,m_vecColors);
//	UT_VECTOR_PURGEALL(.... *,m_vecFonts);
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Exp_RTF::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".rtf") == 0);
}

IEStatus IE_Exp_RTF::StaticConstructor(PD_Document * pDocument,
									   IE_Exp ** ppie)
{
	IE_Exp_RTF * p = new IE_Exp_RTF(pDocument);
	*ppie = p;
	return IES_OK;
}

UT_Bool	IE_Exp_RTF::GetDlgLabels(const char ** pszDesc,
								 const char ** pszSuffixList,
								 IEFileType * ft)
{
	*pszDesc = "Rich Text Format (.rtf)";
	*pszSuffixList = "*.rtf";
	*ft = IEFT_RTF;
	return UT_TRUE;
}

UT_Bool IE_Exp_RTF::SupportsFileType(IEFileType ft)
{
	return (IEFT_RTF == ft);
}
	  
/*****************************************************************/
/*****************************************************************/

IEStatus IE_Exp_RTF::writeFile(const char * szFilename)
{
	UT_ASSERT(m_pDocument);
	UT_ASSERT(szFilename && *szFilename);

	if (!_openFile(szFilename))
		return IES_CouldNotOpenForWriting;

	IEStatus status = _writeDocument();
	if (status == IES_OK)
		_closeFile();
	else
		_abortFile();

	// Note: we let our caller worry about resetting the dirty bit
	// Note: on the document and possibly updating the filename.
	
	return status;
}

void IE_Exp_RTF::write(const char * sz)
{
	if (m_error)
		return;
	m_error |= ! _writeBytes((UT_Byte *)sz);
	return;
}

void IE_Exp_RTF::write(const char * sz, UT_uint32 length)
{
	if (m_error)
		return;
	if (_writeBytes((UT_Byte *)sz,length) != length)
		m_error = UT_TRUE;
	
	return;
}

/*****************************************************************/
/*****************************************************************/

IEStatus IE_Exp_RTF::_writeDocument(void)
{
	// The overall syntax for an RTF file is:
	//
	// <file> := '{' <header> <document> '}'
	//
	// We are responsible for everything except for <document>;
	// We let _ListenerWriteDoc() deal with that.
	//
	// create and install a listener to sniff over the
	// document and find things that we need to write
	// into the rtf header.  this includes the font table
	// and the color table.

	_addColor("000000");				// load black as color 0.
		
	m_pListenerGetProps = new s_RTF_ListenerGetProps(m_pDocument,this);
	if (!m_pListenerGetProps)
		return IES_NoMemory;
	if (!m_pDocument->addListener(static_cast<PL_Listener *>(m_pListenerGetProps),&m_lid))
		return IES_Error;
	m_pDocument->removeListener(m_lid);
	DELETEP(m_pListenerGetProps);

	// write rtf header

	if (!_write_rtf_header())
		return IES_CouldNotWriteToFile;
	
	// create and install a listener to receive the document
	// and write its content in rtf.
	
	m_pListenerWriteDoc = new s_RTF_ListenerWriteDoc(m_pDocument,this);
	if (!m_pListenerWriteDoc)
		return IES_NoMemory;
	if (!m_pDocument->addListener(static_cast<PL_Listener *>(m_pListenerWriteDoc),&m_lid))
		return IES_Error;
	m_pDocument->removeListener(m_lid);
	DELETEP(m_pListenerWriteDoc);

	// write any rtf trailer matter

	if (!_write_rtf_trailer())
		return IES_CouldNotWriteToFile;
	
	return ((m_error) ? IES_CouldNotWriteToFile : IES_OK);
}

/*****************************************************************/
/*****************************************************************/
#if 0
void s_RTF_Listener::_handleStyles(void)
{
	UT_Bool bWroteOpenStyleSection = UT_FALSE;

	const char * szName;
	const PD_Style * pStyle;

	for (UT_uint32 k=0; (m_pDocument->enumStyles(k,&szName,&pStyle)); k++)
	{
		if (!pStyle->isUsed())
			continue;

		if (!bWroteOpenStyleSection)
		{
			m_pie->write("<styles>\n");
			bWroteOpenStyleSection = UT_TRUE;
		}

		PT_AttrPropIndex api = pStyle->getIndexAP();
		_openTag("s","/",UT_TRUE,api);
	}

	if (bWroteOpenStyleSection)
		m_pie->write("</styles>\n");

	return;
}

void s_RTF_Listener::_handleDataItems(void)
{
	UT_Bool bWroteOpenDataSection = UT_FALSE;

	const char * szName;
	const UT_ByteBuf * pByteBuf;

	UT_ByteBuf bb64(1024);

	for (UT_uint32 k=0; (m_pDocument->enumDataItems(k,NULL,&szName,&pByteBuf,NULL)); k++)
	{
		if (!bWroteOpenDataSection)
		{
			m_pie->write("<data>\n");
			bWroteOpenDataSection = UT_TRUE;
		}

		if (UT_Base64Encode(&bb64, pByteBuf))
		{
			m_pie->write("<d name=\"");
			m_pie->write(szName);
			m_pie->write("\">\n");

			// break up the Base64 blob as a series lines
			// like MIME does.

			UT_uint32 jLimit = bb64.getLength();
			UT_uint32 jSize;
			UT_uint32 j;
			for (j=0; j<jLimit; j+=72)
			{
				jSize = MyMin(72,(jLimit-j));
				m_pie->write((const char *)bb64.getPointer(j),jSize);
				m_pie->write("\n");
			}
			m_pie->write("</d>\n");
		}
	}

	if (bWroteOpenDataSection)
		m_pie->write("</data>\n");

	return;
}
#endif
/*****************************************************************/
/*****************************************************************/

UT_sint32 IE_Exp_RTF::_findColor(const char * szColor) const
{
	if (!szColor || !*szColor)
		return 0;						// black
	
	UT_uint32 k;
	UT_uint32 kLimit = m_vecColors.getItemCount();

	for (k=0; k<kLimit; k++)
	{
		const char * sz = (const char *)m_vecColors.getNthItem(k);
		if (UT_stricmp(sz,szColor) == 0)
			return k;
	}

	return -1;
}

void IE_Exp_RTF::_addColor(const char * szColor)
{
	UT_ASSERT(szColor && *szColor && (_findColor(szColor)==-1));

	char * sz = NULL;
	UT_cloneString(sz,szColor);
	if (sz)
		m_vecColors.addItem(sz);
	return;
}

/*****************************************************************/
/*****************************************************************/

void IE_Exp_RTF::_rtf_open_brace(void)
{
	m_braceLevel++;
	write("{");
	m_bLastWasKeyword = UT_FALSE;
}

void IE_Exp_RTF::_rtf_close_brace(void)
{
	m_braceLevel--;
	write("}");
	m_bLastWasKeyword = UT_FALSE;

	UT_ASSERT(m_braceLevel >= 0);
}

void IE_Exp_RTF::_rtf_keyword(const char * szKey)
{
	write("\\");
	write(szKey);
	m_bLastWasKeyword = UT_TRUE;
}

void IE_Exp_RTF::_rtf_keyword(const char * szKey, UT_sint32 d)
{
	write("\\");
	write(szKey);
	char buf[100];
	sprintf(buf,"%d",d);
	write(buf);
	m_bLastWasKeyword = UT_TRUE;
}

void IE_Exp_RTF::_rtf_keyword_hex2(const char * szKey, UT_sint32 d)
{
	write("\\");
	write(szKey);
	char buf[100];
	sprintf(buf,"%02x",d);
	write(buf);
	m_bLastWasKeyword = UT_TRUE;
}

void IE_Exp_RTF::_rtf_keyword_ifnotdefault(const char * szKey, const char * szValue, UT_sint32 defaultValue)
{
	if (!szValue || !*szValue)
		return;

	UT_sint32 d = atol(szValue);
	if (d == defaultValue)
		return;

	write("\\");
	write(szKey);
	char buf[100];
	sprintf(buf,"%d",d);
	write(buf);
	m_bLastWasKeyword = UT_TRUE;
}

void IE_Exp_RTF::_rtf_keyword_ifnotdefault_twips(const char * szKey, const char * szValue, UT_sint32 defaultValue)
{
	if (!szValue || !*szValue)
		return;

	// convert dimensioned value into twips (twentieths of a point) (aka 720 twips/inch)
	double dbl = UT_convertToPoints(szValue);
	UT_sint32 d = (UT_sint32)(dbl * 20.0);
	
	if (d == defaultValue)
		return;

	write("\\");
	write(szKey);
	char buf[100];
	sprintf(buf,"%d",d);
	write(buf);
	m_bLastWasKeyword = UT_TRUE;
}

void IE_Exp_RTF::_rtf_semi(void)
{
	write(";");
	m_bLastWasKeyword = UT_FALSE;
}

void IE_Exp_RTF::_rtf_chardata(const char * pbuf, UT_uint32 buflen)
{
	if (m_bLastWasKeyword)
		write(" ");
	write(pbuf,buflen);
	m_bLastWasKeyword = UT_FALSE;
}

UT_Bool IE_Exp_RTF::_write_rtf_header(void)
{
	// write <rtf-header>
	// return UT_FALSE on error

	_rtf_open_brace();
	_rtf_keyword("rtf",1);				// major version number of spec version 1.5
	
	_rtf_keyword("ansi");
//	_rtf_keyword("ansicpg",1252);		// TODO what CodePage do we want here ??

	_rtf_keyword("deff",0);				// default font is index 0 aka black

	// TODO write the "font table"....
	// TODO write the "file table"....

	_rtf_open_brace();
	_rtf_keyword("colortbl");
	UT_uint32 kLimit = m_vecColors.getItemCount();
	UT_uint32 k;
	for (k=0; k<kLimit; k++)
	{
		const char * szColor = (const char *)m_vecColors.getNthItem(k);
		UT_RGBColor localColor;
		UT_parseColor(szColor,localColor);
		_rtf_keyword("red",  localColor.m_red);
		_rtf_keyword("green",localColor.m_grn);
		_rtf_keyword("blue", localColor.m_blu);
		_rtf_semi();
	}
	_rtf_close_brace();

	// TODO write the "style sheets"...
	// TODO write the "list table"...
	// TODO write the "rev table"...

	return (m_error == 0);
}

UT_Bool IE_Exp_RTF::_write_rtf_trailer(void)
{
	_rtf_close_brace();
	return (m_error == 0);
}
