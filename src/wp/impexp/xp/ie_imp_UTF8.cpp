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


#include <stdio.h>
#include <stdlib.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ie_imp_UTF8.h"
#include "pd_Document.h"
#include "ut_growbuf.h"

/*****************************************************************/
/*****************************************************************/

#ifdef ENABLE_PLUGINS

// completely generic code to allow this to be a plugin

#include "xap_Module.h"

#define SUPPORTS_ABI_VERSION(a,b,c) (((a==0)&&(b==7)&&(c==15)) ? 1 : 0)

// we use a reference-counted sniffer
static IE_Imp_UTF8_Sniffer * m_sniffer = 0;
static UT_sint32 m_refs = 0;

ABI_FAR extern "C"
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_refs && !m_sniffer)
	{
		m_sniffer = new IE_Imp_UTF8_Sniffer ();
		m_refs++;
	}
	else if (m_refs && m_sniffer)
	{
		m_refs++;
	}
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	mi->name = "UTF8 Importer";
	mi->desc = "Import UTF8 Documents";
	mi->version = "0.7.15";
	mi->author = "Abi the Ant";
	mi->usage = "No Usage";

	IE_Imp::registerImporter (m_sniffer);
	return 1;
}

ABI_FAR extern "C"
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
	mi->name = 0;
	mi->desc = 0;
	mi->version = 0;
	mi->author = 0;
	mi->usage = 0;

	UT_ASSERT (m_refs && m_sniffer);

	m_refs--;
	IE_Imp::unregisterImporter (m_sniffer);
	if (!m_refs)
	{
		delete m_sniffer;
		m_sniffer = 0;
	}

	return 1;
}

ABI_FAR extern "C"
int abi_plugin_supports_version (UT_uint32 major, UT_uint32 minor, 
								 UT_uint32 release)
{
	return SUPPORTS_ABI_VERSION(major, minor, release);
}

#endif

/*****************************************************************/
/*****************************************************************/

bool IE_Imp_UTF8_Sniffer::recognizeContents(const char * szBuf, UT_uint32 iNumbytes)
{
	bool bSuccess = false;
	const unsigned char *p = reinterpret_cast<const unsigned char *>(szBuf);

	while (p < reinterpret_cast<const unsigned char *>(szBuf + iNumbytes))
	{
		int len;
		
		if ((*p & 0x80) == 0)				// ASCII
		{
			++p;
			continue;
		}
		else if ((*p & 0xc0) == 0x80)			// not UTF-8
		{
			return false;
		}
		else if (*p == 0xfe || *p == 0xff)		// BOM markers?  RFC2279 says illegal
		{
			//UT_DEBUGMSG(("  BOM?\n"));
			++p;
			continue;
		}
		else if ((*p & 0xfe) == 0xfc)			// lead byte in 6-byte sequence
			len = 6;
		else if ((*p & 0xfc) == 0xf8)			// lead byte in 5-byte sequence
			len = 5;
		else if ((*p & 0xf8) == 0xf0)			// lead byte in 4-byte sequence
			len = 4;
		else if ((*p & 0xf0) == 0xe0)			// lead byte in 3-byte sequence
			len = 3;
		else if ((*p & 0xe0) == 0xc0)			// lead byte in 2-byte sequence
			len = 2;
		else	
		{
			// the above code covers all cases - if we reach here the logic is wrong
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return false;
		}
	
		while (--len)
		{
			++p;
			if (p >= reinterpret_cast<const unsigned char *>(szBuf + iNumbytes))
			{
				//UT_DEBUGMSG(("  out of data!\n"));
				break;
			}
			if ((*p & 0xc0) == 0x80)
				bSuccess = true;
			else
				return false;
		}
		++p;
	}
	
	return bSuccess;
}

bool IE_Imp_UTF8_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".utf8") == 0);
}

UT_Error IE_Imp_UTF8_Sniffer::constructImporter(PD_Document * pDocument,
												IE_Imp ** ppie)
{
	IE_Imp_UTF8 * p = new IE_Imp_UTF8(pDocument);
	*ppie = p;
	return UT_OK;
}

bool	IE_Imp_UTF8_Sniffer::getDlgLabels(const char ** pszDesc,
										  const char ** pszSuffixList,
										  IEFileType * ft)
{
	*pszDesc = "UTF8 (.utf8)";
	*pszSuffixList = "*.utf8";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

/*
  Import a plain text file formatted in UTF8.
  We allow either LF or CR or CRLF line termination.
  Each line terminator is taken to be a paragraph break.
*/

/*****************************************************************/
/*****************************************************************/

static void _smashUTF8(UT_GrowBuf * pgb)
{
	// smash any utf8 sequences into a single ucs char

	// since we change the GrowBuf in-place, we
	// recompute the length and buffer pointer
	// to avoid accidents....

	for (UT_uint32 k=0; (k < pgb->getLength()); k++)
	{
		UT_uint16 * p = pgb->getPointer(k);
		UT_uint16 ck = *p;
		
		if (ck < 0x0080)						// latin-1
			continue;
		else if ((ck & 0x00f0) == 0x00f0)		// lead byte in 4-byte surrogate pair, ik
		{
			UT_ASSERT(UT_NOT_IMPLEMENTED);
			continue;
		}
		else if ((ck & 0x00e0) == 0x00e0)		// lead byte in 3-byte sequence
		{
			UT_ASSERT(k+2 < pgb->getLength());
			XML_Char buf[4];
			buf[0] = (XML_Char)p[0];
			buf[1] = (XML_Char)p[1];
			buf[2] = (XML_Char)p[2];
			buf[3] = 0;
			UT_UCSChar ucs = UT_decodeUTF8char(buf,3);
			pgb->overwrite(k,&ucs,1);
			pgb->del(k+1,2);
			continue;
		}
		else if ((ck & 0x00c0) == 0x00c0)		// lead byte in 2-byte sequence
		{
			UT_ASSERT(k+1 < pgb->getLength());
			XML_Char buf[3];
			buf[0] = (XML_Char)p[0];
			buf[1] = (XML_Char)p[1];
			buf[2] = 0;
			UT_UCSChar ucs = UT_decodeUTF8char(buf,2);
			pgb->overwrite(k,&ucs,1);
			pgb->del(k+1,1);
			continue;
		}
		else // ((ck & 0x0080) == 0x0080)		// trailing byte in multi-byte sequence
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			// let it remain as is...
			continue;
		}
	}
}

/*****************************************************************/
/*****************************************************************/

#define X_CleanupIfError(error,exp)	do { if (((error)=(exp)) != UT_OK) goto Cleanup; } while (0)

UT_Error IE_Imp_UTF8::importFile(const char * szFilename)
{
	FILE *fp = fopen(szFilename, "r");
	if (!fp)
	{
		UT_DEBUGMSG(("Could not open file %s\n",szFilename));
		return UT_IE_FILENOTFOUND;
	}
	
	UT_Error error;

	X_CleanupIfError(error,_writeHeader(fp));
	X_CleanupIfError(error,_parseFile(fp));

	error = UT_OK;

Cleanup:
	fclose(fp);
	return error;
}

#undef X_CleanupIfError

/*****************************************************************/
/*****************************************************************/

IE_Imp_UTF8::~IE_Imp_UTF8()
{
}

IE_Imp_UTF8::IE_Imp_UTF8(PD_Document * pDocument)
	: IE_Imp(pDocument)
{
}

/*****************************************************************/
/*****************************************************************/

#define X_ReturnIfFail(exp,error)		do { bool b = (exp); if (!b) return (error); } while (0)
#define X_ReturnNoMemIfError(exp)	X_ReturnIfFail(exp,UT_IE_NOMEMORY)

UT_Error IE_Imp_UTF8::_writeHeader(FILE * /* fp */)
{
	X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Section, NULL));

	return UT_OK;
}

UT_Error IE_Imp_UTF8::_parseFile(FILE * fp)
{
	UT_GrowBuf gbBlock(1024);
	bool bEatLF = false;
	bool bEmptyFile = true;
	bool bSmashUTF8 = false;
	unsigned char c;

	while (fread(&c, 1, sizeof(c), fp) > 0)
	{
		switch (c)
		{
		case '\r':
		case '\n':
			if ((c == '\n') && bEatLF)
			{
				bEatLF = false;
				break;
			}

			if (c == '\r')
			{
				bEatLF = true;
			}
			
			// we interprete either CRLF, CR, or LF as a paragraph break.
			
			// start a paragraph and emit any text that we
			// have accumulated.
			X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Block, NULL));
			bEmptyFile = false;
			if (gbBlock.getLength() > 0)
			{
				if (bSmashUTF8)
					_smashUTF8(&gbBlock);
				X_ReturnNoMemIfError(m_pDocument->appendSpan(gbBlock.getPointer(0), gbBlock.getLength()));
				gbBlock.truncate(0);
				bSmashUTF8 = false;
			}
			break;

		default:
			bEatLF = false;

			// deal with plain character.  to simplify parsing logic,
			// we just stuff all text chars (latin-1 and utf8 escape
			// sequences) into the GrowBuf and will smash the utf8
			// sequences into unicode in a moment.

			if (c > 0x7f)
				bSmashUTF8 = true;
			UT_UCSChar uc = (UT_UCSChar) c;
			X_ReturnNoMemIfError(gbBlock.ins(gbBlock.getLength(),&uc,1));
			break;
		}
	} 

	if (gbBlock.getLength() > 0 || bEmptyFile)
	{
		// if we have text left over (without final CR/LF),
		// or if we read an empty file,
		// create a paragraph and emit the text now.
		if (bSmashUTF8)
			_smashUTF8(&gbBlock);

		X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Block, NULL));
		if (gbBlock.getLength() > 0)
		    X_ReturnNoMemIfError(m_pDocument->appendSpan(gbBlock.getPointer(0), gbBlock.getLength()));
	}

	return UT_OK;
}

#undef X_ReturnNoMemIfError
#undef X_ReturnIfFail

/*****************************************************************/
/*****************************************************************/

void IE_Imp_UTF8::pasteFromBuffer(PD_DocumentRange * pDocRange,
								  unsigned char * pData, UT_uint32 lenData)
{
	UT_ASSERT(m_pDocument == pDocRange->m_pDoc);
	UT_ASSERT(pDocRange->m_pos1 == pDocRange->m_pos2);

	UT_GrowBuf gbBlock(1024);
	bool bEatLF = false;
	bool bSuppressLeadingParagraph = true;
	bool bInColumn1 = true;
	unsigned char * pc;

	PT_DocPosition dpos = pDocRange->m_pos1;
	
	for (pc=pData; (pc<pData+lenData); pc++)
	{
		unsigned char c = *pc;
		
		switch (c)
		{
		case '\r':
		case '\n':
			if ((c == '\n') && bEatLF)
			{
				bEatLF = false;
				break;
			}

			if (c == '\r')
			{
				bEatLF = true;
			}
			
			// we interprete either CRLF, CR, or LF as a paragraph break.
			
			if (gbBlock.getLength() > 0)
			{
				// flush out what we have
				m_pDocument->insertSpan(dpos, gbBlock.getPointer(0), gbBlock.getLength());
				dpos += gbBlock.getLength();
				gbBlock.truncate(0);
			}
			bInColumn1 = true;
			break;

		default:
			bEatLF = false;
			if (bInColumn1 && !bSuppressLeadingParagraph)
			{
				m_pDocument->insertStrux(dpos,PTX_Block);
				dpos++;
			}
			
			// deal with plain character.  to simplify parsing logic,
			// we just stuff all text chars (latin-1 and utf8 escape
			// sequences) into the GrowBuf and will smash the utf8
			// sequences into unicode in a moment.
			
			UT_UCSChar uc = (UT_UCSChar) c;
			gbBlock.ins(gbBlock.getLength(),&uc,1);

			bInColumn1 = false;
			bSuppressLeadingParagraph = false;
			break;
		}
	} 

	if (gbBlock.getLength() > 0)
	{
		// if we have text left over (without final CR/LF),
		m_pDocument->insertSpan(dpos, gbBlock.getPointer(0), gbBlock.getLength());
		dpos += gbBlock.getLength();
	}

	return;
}


