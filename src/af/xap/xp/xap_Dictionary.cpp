/* AbiSource Application Framework
 * Copyright (C) 1998,1999 AbiSource, Inc.
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
#include <string.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_growbuf.h"

#include "xap_Dictionary.h"

/*****************************************************************/
/*****************************************************************/

/*
	Dictionary is an alphabetic list of words, one per line.  
	Import/export as a plain text file formatted in UTF8.
	We allow either LF or CR or CRLF line termination.

	(This code shamelessly cribbed from the impexp for UTF8.)
*/

/*****************************************************************/
/*****************************************************************/

XAP_Dictionary::XAP_Dictionary(const char * szFilename)
	: m_hashWords(29)
{
	UT_ASSERT(szFilename && *szFilename);
	UT_cloneString((char *&)m_szFilename, szFilename);

	m_fp = 0;
	m_bDirty = UT_FALSE;
}

XAP_Dictionary::~XAP_Dictionary()
{
	if (m_fp)
		_closeFile();

	FREEP(m_szFilename);

	UT_HASH_PURGEDATA(UT_UCSChar *, m_hashWords);
}

const char * XAP_Dictionary::getShortName(void) const
{
	// TODO: get just the filename (no path), for use in UI
	return NULL;
}

/*****************************************************************/
/*****************************************************************/

UT_Bool XAP_Dictionary::_openFile(const char * mode)
{
	UT_ASSERT(!m_fp);

	// TODO add code to make a backup of the original file, if it exists.
	
	m_fp = fopen(m_szFilename,mode);
	return (m_fp != 0);
}

UT_uint32 XAP_Dictionary::_writeBytes(const UT_Byte * pBytes, UT_uint32 length)
{
	UT_ASSERT(m_fp);
	UT_ASSERT(pBytes);
	UT_ASSERT(length);
	
	return fwrite(pBytes,sizeof(UT_Byte),length,m_fp);
}

UT_Bool XAP_Dictionary::_writeBytes(const UT_Byte * sz)
{
	UT_ASSERT(m_fp);
	UT_ASSERT(sz);
	int length = strlen((const char *)sz);
	UT_ASSERT(length);
	
	return (_writeBytes(sz,length)==(UT_uint32)length);
}

UT_Bool XAP_Dictionary::_closeFile(void)
{
	if (m_fp)
		fclose(m_fp);
	m_fp = 0;
	return UT_TRUE;
}

void XAP_Dictionary::_abortFile(void)
{
	// abort the write.
	// TODO close the file and do any restore and/or cleanup.

	_closeFile();
	return;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

UT_Bool XAP_Dictionary::load(void)
{
	UT_ASSERT(m_hashWords.getEntryCount() == 0);

	if (!_openFile("r"))
		return UT_FALSE;

	if (!_parseUTF8())
		_abortFile();
	else
		_closeFile();

	m_bDirty = UT_FALSE;

	return UT_TRUE;
}

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

#define X_ReturnIfFail(exp)		do { UT_Bool b = (exp); if (!b) return (UT_FALSE); } while (0)

UT_Bool XAP_Dictionary::_parseUTF8(void)
{
	UT_GrowBuf gbBlock(1024);
	UT_Bool bEatLF = UT_FALSE;
	UT_Bool bSmashUTF8 = UT_FALSE;
	unsigned char c;

	while (fread(&c, 1, sizeof(c), m_fp) > 0)
	{
		switch (c)
		{
		case '\r':
		case '\n':
			if ((c == '\n') && bEatLF)
			{
				bEatLF = UT_FALSE;
				break;
			}

			if (c == '\r')
			{
				bEatLF = UT_TRUE;
			}
			
			// we interprete either CRLF, CR, or LF as a word delimiter.
			
			if (gbBlock.getLength() > 0)
			{
				if (bSmashUTF8)
					_smashUTF8(&gbBlock);

				X_ReturnIfFail(addWord(gbBlock.getPointer(0), gbBlock.getLength()));
				gbBlock.truncate(0);
				bSmashUTF8 = UT_FALSE;
			}
			break;

		default:
			bEatLF = UT_FALSE;

			// deal with plain character.  to simplify parsing logic,
			// we just stuff all text chars (latin-1 and utf8 escape
			// sequences) into the GrowBuf and will smash the utf8
			// sequences into unicode in a moment.

			if (c > 0x7f)
				bSmashUTF8 = UT_TRUE;
			UT_UCSChar uc = (UT_UCSChar) c;
			X_ReturnIfFail(gbBlock.ins(gbBlock.getLength(),&uc,1));
			break;
		}
	} 

	if (gbBlock.getLength() > 0)
	{
		// if we have text left over (without final CR/LF), emit it now
		if (bSmashUTF8)
			_smashUTF8(&gbBlock);

		X_ReturnIfFail(addWord(gbBlock.getPointer(0), gbBlock.getLength()));
	}

	return UT_TRUE;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

UT_Bool XAP_Dictionary::save(void)
{
	if (!m_bDirty)
		return UT_TRUE;

	if (!_openFile("w"))
		return UT_FALSE;

	UT_sint32 k;
	for (k=0; (k < m_hashWords.getEntryCount()); k++)
	{
		UT_HashEntry * pHE = m_hashWords.getNthEntryAlpha(k);
		UT_ASSERT(pHE);

		UT_UCSChar * pWord = (UT_UCSChar *) pHE->pData;
		_outputUTF8(pWord, UT_UCS_strlen(pWord));
		_writeBytes((UT_Byte *)"\n");
	}

	_closeFile();

	m_bDirty = UT_FALSE;

	return UT_TRUE;
}

void XAP_Dictionary::_outputUTF8(const UT_UCSChar * data, UT_uint32 length)
{
#define MY_BUFFER_SIZE		1024
#define MY_HIGHWATER_MARK	20
	char buf[MY_BUFFER_SIZE];
	char * pBuf;
	const UT_UCSChar * pData;

	for (pBuf=buf, pData=data; (pData<data+length); /**/)
	{
		if (pBuf >= (buf+MY_BUFFER_SIZE-MY_HIGHWATER_MARK))
		{
			_writeBytes((UT_Byte *)buf,(pBuf-buf));
			pBuf = buf;
		}

		if (*pData > 0x007f)
		{
			const XML_Char * s = UT_encodeUTF8char(*pData++);
			while (*s)
				*pBuf++ = *s++;
		}
		else
		{
			*pBuf++ = (UT_Byte)*pData++;
		}
	}

	if (pBuf > buf)
		_writeBytes((UT_Byte *)buf,(pBuf-buf));
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

UT_Bool XAP_Dictionary::addWord(const UT_UCSChar * pWord, UT_uint32 len)
{
	char * key = (char *) calloc(len+1, sizeof(char));
	UT_UCSChar * copy = (UT_UCSChar *) calloc(len+1, sizeof(UT_UCSChar));

	if (!key || !copy)
	{
		UT_DEBUGMSG(("mem failure adding word to dictionary\n"));
		FREEP(key);
		FREEP(copy);
		return UT_FALSE;
	}

	for (UT_uint32 i = 0; i < len; i++)
	{
		UT_UCSChar currentChar;
		currentChar = pWord[i];
		// map smart quote apostrophe to ASCII right single quote
		if (currentChar == UCS_RQUOTE) currentChar = '\'';
		key[i] = (char) currentChar;
		copy[i] = currentChar;
	}

	UT_sint32 iRes = m_hashWords.addEntry(key, NULL, (void*) copy);

	FREEP(key);

	if (iRes == 0)
	{
		m_bDirty = UT_TRUE;
		return UT_TRUE;
	}
	else
	{
		return UT_FALSE;
	}
}

UT_Bool XAP_Dictionary::isWord(const UT_UCSChar * pWord, UT_uint32 len) const
{
	char * key = (char*) calloc(len+1, sizeof(char));
	if (!key)
	{
		UT_DEBUGMSG(("mem failure looking up word in dictionary\n"));
		FREEP(key);
		return UT_FALSE;
	}

	for (UT_uint32 i = 0; i < len; i++)
	{
		key[i] = (char) pWord[i];
	}

	UT_HashEntry * pHE = m_hashWords.findEntry(key);

	FREEP(key);

	if (pHE != NULL)
		return UT_TRUE;
	else 
		return UT_FALSE;
}
