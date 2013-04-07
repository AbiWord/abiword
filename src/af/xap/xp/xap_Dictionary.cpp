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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <stdlib.h>
#include <string.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_growbuf.h"
#include "ut_hash.h"
#include "ut_vector.h"
#include "ut_string_class.h"

#include "xap_Dictionary.h"

/*****************************************************************/
/*****************************************************************/

/*
	Dictionary is an alphabetic list of words, one per line.  
	Import/export as a plain text file formatted in UTF-8.
	We allow either LF or CR or CRLF line termination.

	(This code shamelessly cribbed from the impexp for UTF-8.)
*/

/*****************************************************************/
/*****************************************************************/

XAP_Dictionary::XAP_Dictionary(const char * szFilename)
	: m_hashWords(29)
{
	UT_ASSERT(szFilename && *szFilename);
	m_szFilename = g_strdup(szFilename);

	m_fp = 0;
	m_bDirty = false;
}

XAP_Dictionary::~XAP_Dictionary()
{
	if (m_fp)
		_closeFile();

	FREEP(m_szFilename);

  	//UT_HASH_PURGEDATA(UT_UCSChar *, (&m_hashWords), g_free);
	m_hashWords.freeData();
#if 0
	UT_StringPtrMap::UT_Cursor _hc1(&m_hashWords);
	for (UT_UCSChar * _hval1 = const_cast<UT_UCSChar *>(reinterpret_cast<const UT_UCSChar *>(_hc1.first())); _hc1.is_valid(); _hval1 = const_cast<UT_UCSChar *>(reinterpret_cast<const UT_UCSChar *>(_hc1.next())) )
	{ 
		if (_hval1)
			g_free (_hval1);
	}
#endif
}

const char * XAP_Dictionary::getShortName(void) const
{
	// TODO: get just the filename (no path), for use in UI
	return NULL;
}

/*****************************************************************/
/*****************************************************************/

bool XAP_Dictionary::_openFile(const char * mode)
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

bool XAP_Dictionary::_writeBytes(const UT_Byte * sz)
{
	UT_ASSERT(m_fp);
	UT_ASSERT(sz);
	int length = strlen(reinterpret_cast<const char *>(sz));
	UT_ASSERT(length);
	
	return (_writeBytes(sz,length)==static_cast<UT_uint32>(length));
}

bool XAP_Dictionary::_closeFile(void)
{
	if (m_fp)
		fclose(m_fp);
	m_fp = 0;
	return true;
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

bool XAP_Dictionary::load(void)
{
	UT_ASSERT(m_hashWords.size() == 0);

	if (!_openFile("r"))
		return false;

	if (!_parseUTF8())
		_abortFile();
	else
		_closeFile();

	m_bDirty = false;
//
// Hardwire in some words that should be in the English Language :-)
//
	addWord("AbiWord");
	addWord("AbiSource");
	return true;
}

#define X_ReturnIfFail(exp)		do { bool b = (exp); if (!b) return (false); } while (0)

bool XAP_Dictionary::_parseUTF8(void)
{
	UT_GrowBuf gbBlock(1024);
	bool bEatLF = false;
	gchar buf[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	gint len;

	while (fread(buf, 1, sizeof(gchar), m_fp) > 0)
	{
		switch (buf[0])
		{
		case '\r':
		case '\n':
			if ((buf[0] == '\n') && bEatLF)
			{
				bEatLF = false;
				break;
			}

			if (buf[0] == '\r')
			{
				bEatLF = true;
			}
			
			// we interprete either CRLF, CR, or LF as a word delimiter.
			
			if (gbBlock.getLength() > 0)
			{
				X_ReturnIfFail(addWord(reinterpret_cast<UT_UCS4Char*>(gbBlock.getPointer(0)), gbBlock.getLength()));
				gbBlock.truncate(0);
			}
			break;

		default:
			bEatLF = false;

			len = g_utf8_next_char(buf) - buf;
			if (len > 1) {
				fread (buf + 1, len - 1, sizeof (gchar), m_fp);
			}
			UT_UCSChar uc = g_utf8_get_char(buf);
			X_ReturnIfFail(gbBlock.ins(gbBlock.getLength(),reinterpret_cast<UT_GrowBufElement*>(&uc),1));
			break;
		}
	} 

	if (gbBlock.getLength() > 0)
	{
		X_ReturnIfFail(addWord(reinterpret_cast<UT_UCS4Char*>(gbBlock.getPointer(0)), gbBlock.getLength()));
	}

	return true;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

bool XAP_Dictionary::save(void)
{
	if (!m_bDirty)
		return true;

	if (!_openFile("w"))
		return false;

	UT_GenericVector<UT_UCSChar *> * pVec = m_hashWords.enumerate();
	UT_ASSERT(pVec);

	UT_uint32 size = pVec->size();

	for (UT_uint32 i = 0; i < size; i++)
	{
		UT_UCSChar * pWord = pVec->getNthItem(i);
		_outputUTF8(pWord, UT_UCS4_strlen(pWord));
		_writeBytes(reinterpret_cast<const UT_Byte *>("\n"));
	}

	_closeFile();

	delete pVec;
	m_bDirty = false;

	return true;
}

void XAP_Dictionary::_outputUTF8(const UT_UCSChar * data, UT_uint32 length)
{
	UT_String buf;
	const UT_UCSChar * pData;

	for (pData = data; (pData<data+length); /**/)
	{
		if (*pData > 0x007f)
		{
			gchar outbuf[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
			g_unichar_to_utf8(*pData++, outbuf);
			buf += outbuf;
		}
		else
		{
			buf += (char)*pData++;
		}
	}

	_writeBytes(reinterpret_cast<const UT_Byte *>(buf.c_str()),buf.size());
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

bool XAP_Dictionary::addWord(const UT_UCSChar * pWord, UT_uint32 len)
{
	char * key = static_cast<char *>(UT_calloc(len+1, sizeof(char)));
	UT_UCSChar * copy = static_cast<UT_UCSChar *>(UT_calloc(len+1, sizeof(UT_UCSChar)));
    if (!key || !copy)
	{
		UT_DEBUGMSG(("mem failure adding word to dictionary\n"));
		FREEP(key);
		FREEP(copy);
		return false;
	}
	UT_uint32 i = 0;
	for (i = 0; i < len; i++)
	{
		UT_UCSChar currentChar;
		currentChar = pWord[i];
		// map smart quote apostrophe to ASCII right single quote
		if (currentChar == UCS_RQUOTE) currentChar = '\'';
		key[i] = static_cast<char>(static_cast<unsigned char>(pWord[i]));
		copy[i] = currentChar;
		xxx_UT_DEBUGMSG(("addWord: key[%d] = %c %d \n",i,key[i],key[i]));
		if(key[i] == 0)
		{
			break;
		}
	}
	key[i] = 0;
//
// Get exactly the same length.
//
	char * key2 = g_strdup(key);
	copy[i] = 0;
#if 0
//
// Useful debugging code
//
	char * ucs_dup = static_cast<char *>(UT_calloc(2*len+1, sizeof(char)));
	UT_UCS4_strcpy_to_char( ucs_dup, copy);
	UT_DEBUGMSG(("Inserting word %s with key %s into hash \n",ucs_dup,key));
	FREEP(ucs_dup);

#endif
	if(!m_hashWords.insert(key2,copy))
		FREEP(copy);
	
	FREEP(key);
	FREEP(key2);

	// TODO: is this right?
	m_bDirty = true;
	return true;
}

bool XAP_Dictionary::addWord(const char * word)
{
	UT_sint32 len = strlen(word);
	if(len <=0)
	{
		return false;
	}
	UT_UCSChar * ucs_dup = static_cast<UT_UCSChar *>(UT_calloc(len+1, sizeof(UT_UCSChar)));
	UT_UCS4_strcpy_char(ucs_dup, word);
	addWord(ucs_dup,len);
	FREEP(ucs_dup);
	return true;
}


/*!
 * Returns true if the word given is found in the users custom dictionary.
\param const UT_UCSChar * pWord the word to look for suggestion for
\param UT_uint32 len the length of the word
\returns UT_Vector * pVecSuggestions this a vector of suggestions.
The returner is responsible for deleting these words. 
*/
void XAP_Dictionary::suggestWord(UT_GenericVector<UT_UCSChar *> * pVecSuggestions, const UT_UCSChar * pWord, UT_uint32 len)
{
  //
  // Get the words in the local dictionary
  //
  UT_GenericVector<UT_UCSChar *> * pVec = m_hashWords.enumerate();
  UT_ASSERT(pVec);
  UT_uint32 i=0;
  UT_uint32 count = pVec->getItemCount();
  //
  // Turn our word into a NULL teminated string
  //
  UT_UCSChar * pszWord = static_cast<UT_UCSChar*>(UT_calloc(len+1, sizeof(UT_UCSChar)));
  for(i=0; i< len; i++)
  {
    pszWord[i] = pWord[i];
  }
  pszWord[len] = 0;
  //
  // Loop over all the words in our custom doctionary and add them to the 
  //the suggestions if they're possibilities.
  //
  for(i=0; i< count; i++)
  {
    UT_UCSChar * pszDict = pVec->getNthItem(i);
    UT_UCSChar * pszReturn = NULL;
    float lenDict = static_cast<float>(UT_UCS4_strlen(pszDict));
    UT_uint32 wordInDict = countCommonChars(pszDict,pszWord);
    UT_uint32 dictInWord = countCommonChars(pszWord,pszDict);
    float flen = static_cast<float>(len);
    float frac1 = (static_cast<float>(wordInDict)) / flen;
    float frac2 = (static_cast<float>(dictInWord)) / lenDict;

    if((frac1 > 0.8) && (frac2 > 0.8))
    {
	  UT_UCS4_cloneString(&pszReturn, pszDict);
	  pVecSuggestions->addItem(pszReturn);
    }
  }
  FREEP(pszWord);
  DELETEP(pVec);
}

/*!
 * This method counts the number of common characters in pszNeedle found in
 * pszHaystack. Every time character in pszNeedle is found in pszHaystack the 
 * score is incremented by 1.
 */
UT_uint32 XAP_Dictionary::countCommonChars( UT_UCSChar *pszHaystack,UT_UCSChar * pszNeedle)
{
    UT_uint32 lenNeedle =  UT_UCS4_strlen(pszNeedle);
    UT_UCSChar oneChar[2];
    oneChar[1] = 0;
    UT_uint32 i=0;
    UT_uint32 score =0;
    for(i=0; i< lenNeedle; i++)
    {
      oneChar[0] = pszNeedle[i];
      if(UT_UCS4_strstr(pszHaystack,oneChar) != 0)
      {
	  score++;
      }
    }
    return score;
}

bool XAP_Dictionary::isWord(const UT_UCSChar * pWord, UT_uint32 len) const
{
	char * key = static_cast<char*>(UT_calloc(len+1, sizeof(char)));
	if (!key)
	{
		UT_DEBUGMSG(("mem failure looking up word in dictionary\n"));
		FREEP(key);
		return false;
	}
	UT_uint32 i =0;
	for (i = 0; i < len; i++)
	{
		key[i] = static_cast<char>(static_cast<unsigned char>( pWord[i] ));
		xxx_UT_DEBUGMSG(("isword key[%d] = %c %d \n",i,key[i],key[i]));
		if(key[i] == 0)
			break;
	}
	key[i] = 0;
	char * key2 = g_strdup(key);
	bool contains = m_hashWords.contains (key2, NULL);
	FREEP(key);
	FREEP(key2);
	return contains;
}

