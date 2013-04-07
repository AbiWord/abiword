/* AbiWord
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
 * "This product is not manufactured, approved, or supported by 
 * Corel Corporation or Corel Corporation Limited."
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ie_exp_WordPerfect.h"
#include "ie_impexp_WordPerfect.h"


#if defined(TOOLKIT_WIN) && !defined(__MINGW32__)
extern double rint(double x);
#endif /* _WIN32 */

IE_Exp_WordPerfect_Sniffer::IE_Exp_WordPerfect_Sniffer() 
    : IE_ExpSniffer(IE_MIMETYPE_WP_6)
{
}

IE_Exp_WordPerfect_Sniffer::~IE_Exp_WordPerfect_Sniffer()
{
}

bool IE_Exp_WordPerfect_Sniffer::recognizeSuffix (const char * szSuffix)
{
	// We can only write word documents, not templates
  return ( !g_ascii_strcasecmp(szSuffix,".wpd") || !g_ascii_strcasecmp(szSuffix, ".wp") ) ;
}

UT_Error IE_Exp_WordPerfect_Sniffer::constructExporter (PD_Document * pDocument,
													    IE_Exp ** ppie)
{
	IE_Exp_WordPerfect * p = new IE_Exp_WordPerfect(pDocument);
	*ppie = p;
	return UT_OK;
}

bool	IE_Exp_WordPerfect_Sniffer::getDlgLabels (const char ** pszDesc,
												  const char ** pszSuffixList,
												  IEFileType * ft)
{
	*pszDesc = "WordPerfect 6/7/8/9 (.wpd)";
	*pszSuffixList = "*.wpd";
	*ft = getFileType();
	return true;
}

/****************************************************************************/
/****************************************************************************/

IE_Exp_WordPerfect::IE_Exp_WordPerfect(PD_Document * pDocument)
  : IE_Exp (pDocument)
{
	m_desiredFontUseCount = 1;
}

IE_Exp_WordPerfect::~IE_Exp_WordPerfect() 
{
}

UT_Error IE_Exp_WordPerfect::_writeDocument(void)
{
	UT_DEBUGMSG(("WordPerfect Exporter: _writeDocument\n"));
	
	if (_writeHeader() != UT_OK)
			return UT_ERROR;
	
	m_pListener = _constructListener();
	if (!m_pListener)
		return UT_IE_NOMEMORY;
	
	if (getDocRange())
		getDoc()->tellListenerSubset(static_cast<PL_Listener *>(m_pListener),getDocRange());
	else
		getDoc()->tellListener(static_cast<PL_Listener *>(m_pListener));
	DELETEP(m_pListener);

	if (m_error == UT_OK)
	  {
	    _UT_String_overwrite(*m_buffer, PTR_TO_DOCUMENT_INDEX, m_ptrToDocument);
	    _UT_String_overwrite(*m_buffer, PTR_TO_FILESIZE_INDEX, m_buffer->length());
	    _UT_String_overwrite(*m_buffer, m_ptrDesiredFontUseCount, m_desiredFontUseCount);
	    
	    write(m_buffer->c_str(), m_buffer->length());

	    DELETEP(m_buffer);
	  }

	return ((m_error) ? UT_IE_COULDNOTWRITE : UT_OK);
}

PL_Listener * IE_Exp_WordPerfect::_constructListener(void)
{
	UT_DEBUGMSG(("WordPerfect Exporter: _constructListener\n"));	
	return new WordPerfect_Listener(getDoc(), this);
}

void IE_Exp_WordPerfect::_UT_String_add(UT_String &s, int i)
{
	// long integers are stored in reverse byte order!  
	for (size_t j=0; j<sizeof(i); j++)
		s += (char)(i>>8*j);
}

void IE_Exp_WordPerfect::_UT_String_add(UT_String &s, short i)
{
	// short integers are stored in reverse byte order!  
	for (size_t j=0; j<sizeof(i); j++)
		s += (char)(i>>8*j);
}

void IE_Exp_WordPerfect::_UT_String_overwrite(UT_String &s, int pos, int i)
{
	char *p = const_cast<char *>(s.c_str());
	p += pos;

	for (size_t j=0; j<sizeof(i); j++)
		*p++ = (char)(i>>8*j);
}

void IE_Exp_WordPerfect::_UT_String_add_chars(UT_String &s, char *c, int count)
{
	for (int j=0; j<count; j++)
		s += c[j];
}

UT_Error IE_Exp_WordPerfect::_writeHeader()
{
	int i = 0;
	UT_DEBUGMSG(("WordPerfect Exporter::_writeHeader\n"));
	
	m_buffer = new UT_String();
	
	*m_buffer += (char)0xFF;						// -1
	*m_buffer += "WPC";								// WPC
	_UT_String_add(*m_buffer, (int)0x00000000);		// pointer to document area; THIS IS A DUMMY VALUE!
	*m_buffer += (char)0x01;						// program that created the file, WordPerfect itself = 1, we choose also 1 :-)
	*m_buffer += (char)0x0A;						// file type, WordPerfect document had type 0x0A
	*m_buffer += (char)0x02;						// major version of file, WP 8.0 = 2
	*m_buffer += (char)0x01;						// minor version of file, WP 8.0 = 1
	_UT_String_add(*m_buffer, (short)0x0000);		// encryption key; 0 means no encryption is used
	_UT_String_add(*m_buffer, (short)0x0200);		// offset to the index header = 0x200
	*m_buffer += char(0x05);						// reserved; beginning of extended file header, fixed entry count = 5
	*m_buffer += (char)0x00;						// reserved; completeness indicator
	_UT_String_add(*m_buffer, (short)0x0000);		// reserved; offset to proprietary data
	_UT_String_add(*m_buffer, (int)0x00000000);		// file size, not including pad charachters at EOF; THIS IS A DUMMY VALUE!

	for (i=0; i<488; i++) { *m_buffer += (char)0x00; } // extended header, 488 bytes
		
	// Index and Packet Data Areas follow
	
	*m_buffer += (char)0x02;						// index header; flags = 2
	*m_buffer += (char)0x00;						// index header; reserved = 0
	_UT_String_add(*m_buffer, (short)0x0005);		// index header; number of indexes in index block, we write 5 packets
	for (i=0; i<10; i++) { *m_buffer += (char)0x00; } // index header; reserved = 0 x 10
	
		
	// FIXME: clean me up
	m_ptrDesiredFontUseCount = m_buffer->length() + 2;

	// This is a piece of the index header which I can't figure out yet
	// When I do figure out what it all means, I'll replace it with some nice header-writing functions
	char magic[192] = 
	{
		// Desired Font
		(char)0x00, (char)0x55, 
		(char)0x01, (char)0x00, (char)0x00, (char)0x00, (char)0x4E, (char)0x00, (char)0x00, (char)0x00, (char)0x46, (char)0x02, (char)0x00, (char)0x00, 
		
		// Initial Font
		(char)0x09, (char)0x25, (char)0x01, (char)0x00, 
		(char)0x00, (char)0x00, (char)0x06, (char)0x00, (char)0x00, (char)0x00, (char)0x94, (char)0x02, (char)0x00, (char)0x00, 
		
		// Style Data
		(char)0x0B, (char)0x30, (char)0x02, (char)0x00, (char)0x00, (char)0x00, 
		(char)0x28, (char)0x00, (char)0x00, (char)0x00, (char)0x9A, (char)0x02, (char)0x00, (char)0x00, 
		
		// Prefix Time Stamp
		(char)0x08, (char)0x5E, (char)0x01, (char)0x00, (char)0x00, (char)0x00, (char)0x0C, (char)0x00, 
		(char)0x00, (char)0x00, (char)0xC2, (char)0x02, 
		
		// Extra Desired Font Information
		(char)0x00, (char)0x00,
		(char)0x28, (char)0x00, (char)0xD6, (char)0x1E, (char)0xC3, (char)0x0f, (char)0x39, (char)0x08, (char)0x00, (char)0x00, 
		(char)0x11, (char)0x09, (char)0x00, (char)0x00, (char)0x00, (char)0x5A, (char)0x00, (char)0x1B, (char)0x01, (char)0x00, (char)0x8B, (char)0x14, (char)0x36, (char)0x00, (char)0x54, (char)0x00, 
		(char)0x69, (char)0x00, (char)0x6D, (char)0x00, (char)0x65, (char)0x00, (char)0x73, (char)0x00, (char)0x20, (char)0x00, (char)0x4E, (char)0x00, (char)0x65, (char)0x00, (char)0x77, (char)0x00, 
		(char)0x20, (char)0x00, (char)0x52, (char)0x00, (char)0x6F, (char)0x00, (char)0x6D, (char)0x00, (char)0x61, (char)0x00, (char)0x6E, (char)0x00, (char)0x20, (char)0x00, (char)0x52, (char)0x00, 
		(char)0x65, (char)0x00, (char)0x67, (char)0x00, (char)0x75, (char)0x00, (char)0x6C, (char)0x00, (char)0x61, (char)0x00, (char)0x72, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, 
		(char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x01, (char)0x00, (char)0x01, (char)0x00, (char)0x58, (char)0x02, (char)0x01, (char)0x00, (char)0x00, (char)0x00, (char)0x04, (char)0x00, 
		(char)0x28, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, 
		(char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x01, (char)0x12, (char)0x02, (char)0x00, (char)0x24, (char)0x00, (char)0xA1, (char)0x00, (char)0x00, (char)0x00, (char)0xA1, (char)0x00, 
		(char)0x00, (char)0x00, (char)0x50, (char)0xA5, (char)0x4E, (char)0x25, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x08, (char)0x00, 
	};
		
	// add the magic index contents to get a nice and working header
	_UT_String_add_chars(*m_buffer, magic, 192);

	// we're done writing the header, save this position as the start of document		
	m_ptrToDocument = m_buffer->length();
	
	// write some global document properties now
	_handleGlobalOn();
	_handleGlobalOff();
	
	return UT_OK;
}

void IE_Exp_WordPerfect::_handleGlobalOn()
{
	UT_DEBUGMSG(("IE_Exp_WordPerfect::_handleGlobalOn\n"));
	
	short size = 1 + 1 + 2 + 1 + 1 + 2 + 2 + 2 + 1 + 2 + 1;
	
	*m_buffer += WP_TOP_STYLE_GROUP;
	*m_buffer += (char)0x0A;
	_UT_String_add(*m_buffer, size);
	*m_buffer += (char)0x83; // flags = PRFXID+3 (encased function) - FIXME: FIND OUT WHAT THIS VALUE SHOULD BE INSTEAD OF COPYING IT FROM A STANDARD WP FILE
	*m_buffer += (char)0x01; // number of PIDs = 1
	_UT_String_add(*m_buffer, (short)0x03); // style PID (type = 30) - FIXME: REMOVE HARDCODED VALUE OF 3
	_UT_String_add(*m_buffer, (short)0x03); // size of non deletable information = 3
	_UT_String_add(*m_buffer, (short)0x00); // hash of this Global On - FIXME: howto create the HASH ???
	*m_buffer += (char)0x21; //  system style number; 0x21 = document
	_UT_String_add(*m_buffer, size);
	*m_buffer += WP_TOP_STYLE_GROUP;
}

void IE_Exp_WordPerfect::_handleGlobalOff()
{
	UT_DEBUGMSG(("IE_Exp_WordPerfect::_handleGlobalOff\n"));

	short size = 1 + 1 + 2 + 1 + 2 + 2 + 1;
	
	*m_buffer += WP_TOP_STYLE_GROUP;
	*m_buffer += (char)0x0B;
	_UT_String_add(*m_buffer, size);
	*m_buffer += (char)0x03; // flags = 3 (encased function)
	_UT_String_add(*m_buffer, (short)0x00); // size of non deletable information = 0
	_UT_String_add(*m_buffer, size);
	*m_buffer += WP_TOP_STYLE_GROUP;
}

/****************************************************************************/
/****************************************************************************/

WordPerfect_Listener::WordPerfect_Listener(PD_Document * pDocument,
										   IE_Exp_WordPerfect * pie)
	: m_pDocument(pDocument),
	  m_pie(pie)
{
	UT_DEBUGMSG(("WordPerfect Listener::constructor\n"));	

	m_bInBlock = false;
	m_paragraphJustification = WP_PARAGRAPH_GROUP_JUSTIFICATION_LEFT;
}

bool WordPerfect_Listener::populate(fl_ContainerLayout* /*sfh*/,
									const PX_ChangeRecord * pcr)
{
	switch (pcr->getType())
	{
		case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *>(pcr);

			PT_BufIndex bi = pcrs->getBufIndex();
			PT_AttrPropIndex api = pcr->getIndexAP();
			if (api)
			{
				_openSpan(api);
			}

			_outputData(m_pDocument->getPointer(bi),pcrs->getLength());
			
			if (api)
			{
				_closeSpan();
			}

			return true;
		}

		case PX_ChangeRecord::PXT_InsertObject:
		{
		}

		case PX_ChangeRecord::PXT_InsertFmtMark:
			return true;

		default:
			UT_ASSERT(0);
			return false;
	}
}

bool WordPerfect_Listener::populateStrux(pf_Frag_Strux* /*sdh*/,
									   const PX_ChangeRecord * pcr,
									   fl_ContainerLayout* * psfh)
{
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *>(pcr);
	*psfh = 0; // we don't need it.

	switch (pcrx->getStruxType())
	{
		case PTX_Section:
		{
				UT_DEBUGMSG(("WordPerfect Listener::_populateStrux PTX_Section\n"));
				return true;
		}
		case PTX_SectionHdrFtr:
		{
				UT_DEBUGMSG(("WordPerfect Listener::_populateStrux PTX_SectionHdrFtr\n"));
				return true;
		}
		case PTX_Block:
		{
				UT_DEBUGMSG(("WordPerfect Listener::_populateStrux PTX_Block\n"));
				_closeBlock();
				_openBlock(pcr->getIndexAP());
				return true;
		}
		default:
			UT_ASSERT_NOT_REACHED();
			return false;
	}
}

bool WordPerfect_Listener::change(fl_ContainerLayout* /*sfh*/,
								const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT_NOT_REACHED();						// this function is not used.
	return false;
}

bool WordPerfect_Listener::insertStrux(fl_ContainerLayout* /*sfh*/,
									   const PX_ChangeRecord * /*pcr*/,
									   pf_Frag_Strux* /*sdh*/,
									   PL_ListenerId /* lid */,
									   void (* /*pfnBindHandles*/)(pf_Frag_Strux* /* sdhNew */,
																   PL_ListenerId /* lid */,
																   fl_ContainerLayout* /* sfhNew */))
{
	UT_ASSERT_NOT_REACHED();		   // this function is not used.
	return false;
}

bool WordPerfect_Listener::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT_NOT_REACHED();
	return false;
}

void WordPerfect_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
{
	UT_ByteBuf bBuf;
	const UT_UCSChar * pData;

	int mbLen;
	char pC[MY_MB_LEN_MAX+1];

	for (pData=data; (pData<data+length); ++pData)
	{
		//UT_DEBUGMSG(("WordPerfect Listener: output char: %c\n", *pData));
	
		if (*pData == ' ')
		{
			*(m_pie->m_buffer) += (char)0x80; // Soft Space  --  FIXME: use defines in ie_imp_wordperfect.h
		}
		else if (*pData == UCS_FF)
		{
			*(m_pie->m_buffer) += (char)0xC7; // Hard OEP  --  FIXME: use defines in ie_imp_wordperfect.h
		}
		else if (*pData == '\t')
		{
			_handleTabGroup((char)0x11); // Left Tab  --  FIXME: support more complete TAB definitions
		}
		else
		{
			if (*pData > 127)
			{
				// FIXME: handle chars > 127!!!
				continue;
			}
			
			if (!_wctomb(pC,mbLen,*pData))
			{
				//*UT_ASSERT_HARMLESS(!m_bIs16Bit);
				mbLen=1;
				pC[0]='?';
				m_wctomb.initialize();
			}
			UT_ASSERT_HARMLESS(mbLen>=1);
			pC[mbLen] = '\0';
			*(m_pie->m_buffer) += pC;
		}
	}
}

void WordPerfect_Listener::_closeBlock()
{
	UT_DEBUGMSG(("WordPerfect Listener::_closeBlock\n"));
	
	if (!m_bInBlock)
	{
		return;
	}
	
	*(m_pie->m_buffer) += (char)0xCC; // Hard EOL  --  FIXME: use defines in ie_imp_wordperfect.h
	m_bInBlock = false;
}

void WordPerfect_Listener::_openBlock(PT_AttrPropIndex api)
{
	UT_DEBUGMSG(("WordPerfect Listener::_openBlock\n"));

	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	
	if (bHaveProp && pAP)
	{
		const gchar * szValue;
		char oldJustification = m_paragraphJustification; // remember the old setting to only change it if it has really changed
		
		if (pAP->getProperty("text-align", szValue))
		{
			if (!strcmp("left", szValue))
			{
				m_paragraphJustification = WP_PARAGRAPH_GROUP_JUSTIFICATION_LEFT;
			} 
			else if (!strcmp("right", szValue))
			{
				m_paragraphJustification = WP_PARAGRAPH_GROUP_JUSTIFICATION_RIGHT;
			}
			else if (!strcmp("center", szValue))
			{
				m_paragraphJustification = WP_PARAGRAPH_GROUP_JUSTIFICATION_CENTER;
			}
			else if (!strcmp("justify", szValue))
			{
				m_paragraphJustification = WP_PARAGRAPH_GROUP_JUSTIFICATION_FULL;
			}
		} 
		else
		{
			// no text alignment specified, default to left
			m_paragraphJustification = WP_PARAGRAPH_GROUP_JUSTIFICATION_LEFT;
		}

		if (oldJustification != m_paragraphJustification)
		{
			_handleParagraphJustification(m_paragraphJustification);
		}
		
		// add more block options below
		// ...
	}

	m_bInBlock = true;
}

void WordPerfect_Listener::_openSpan(PT_AttrPropIndex api)
{
	if (!m_bInBlock)
	{
		return;
	}

	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);

	if (bHaveProp && pAP)
	{
		const gchar * szValue;
		
		if (
			(pAP->getProperty("text-position", szValue))
			&& !strcmp(szValue, "superscript")
			)
		{
			_handleAttributeOn((char) 5);  // FIXME: use defines
		}

		if (
			(pAP->getProperty("text-position", szValue))
			&& !strcmp(szValue, "subscript")
			)
		{
			_handleAttributeOn((char) 6);  // FIXME: use defines
		}
		
		if (
			(pAP->getProperty("font-style", szValue))
			&& !strcmp(szValue, "italic")
			)
		{
			_handleAttributeOn((char) 8);  // FIXME: use defines
		}
		
		if (
			(pAP->getProperty("font-weight", szValue))
			&& !strcmp(szValue, "bold")
			)
		{
			_handleAttributeOn((char) 12);  // FIXME: use defines
		}
		
		if (
			(pAP->getProperty("text-decoration", szValue))
			)
		{
			const gchar* pszDecor = szValue;

			gchar* p;
			if (!(p = g_strdup(pszDecor)))
			{
				// TODO outofmem
			}

			UT_return_if_fail(p || !pszDecor);
			gchar*	q = strtok(p, " ");

			while (q)
			{
				if (0 == strcmp(q, "line-through"))
				{
					_handleAttributeOn((char) 13);  // FIXME: use defines
				}

				q = strtok(NULL, " ");
			}

			FREEP(p);
		}
		
		if (
			(pAP->getProperty("text-decoration", szValue))
			)
		{
			const gchar* pszDecor = szValue;

			gchar* p;
			if (!(p = g_strdup(pszDecor)))
			{
				// TODO outofmem
			}

			UT_return_if_fail(p || !pszDecor);
			gchar*	q = strtok(p, " ");

			while (q)
			{
				if (0 == strcmp(q, "underline"))
				{
					_handleAttributeOn((char) 14);  // FIXME: use defines
				}

				q = strtok(NULL, " ");
			}

			FREEP(p);
		}

		if (
			(pAP->getProperty("color", szValue))
		    || (pAP->getProperty("font-size", szValue))
		    || (pAP->getProperty("font-family", szValue))
			|| (pAP->getProperty("bgcolor", szValue))
			)
		{
			const gchar* pszColor = NULL;
			const gchar* pszBgColor = NULL;
			const gchar* pszFontSize = NULL;
			const gchar* pszFontFamily = NULL;

			pAP->getProperty("color", pszColor);
			pAP->getProperty("font-size", pszFontSize);
			pAP->getProperty("font-family", pszFontFamily);
			pAP->getProperty("bgcolor", pszBgColor);
			
			if (pszColor)
			{
				// ...
			}
			
			if (pszFontSize)
			{
				UT_LocaleTransactor lt (LC_NUMERIC, "C");
				_handleFontSizeChange(UT_convertToPoints(pszFontSize));
			}
			
			if (pszFontFamily)
			{
				// ...
			}
			
			if (pszBgColor)
			{
				// ...
			}
		}

		m_pAP_Span = pAP;
	}
}

void WordPerfect_Listener::_closeSpan()
{
	const PP_AttrProp * pAP = m_pAP_Span;

	if (pAP)
	{
		const gchar * szValue;
		
		if (
			(pAP->getProperty("text-position", szValue))
			&& !strcmp(szValue, "superscript")
			)
		{
			_handleAttributeOff((char) 5);  // FIXME: use defines
		}

		if (
			(pAP->getProperty("text-position", szValue))
			&& !strcmp(szValue, "subscript")
			)
		{
			_handleAttributeOff((char) 6);  // FIXME: use defines
		}
		
		if (
			(pAP->getProperty("font-style", szValue))
			&& !strcmp(szValue, "italic")
			)
		{
			_handleAttributeOff((char) 8);  // FIXME: use defines
		}

		if (
			(pAP->getProperty("font-weight", szValue))
			&& !strcmp(szValue, "bold")
			)
		{
			_handleAttributeOff((char) 12); // FIXME: use defines
		}

		if (
			(pAP->getProperty("text-decoration", szValue))
			)
		{
			const gchar* pszDecor = szValue;

			gchar* p;
			if (!(p = g_strdup(pszDecor)))
			{
				// TODO outofmem
			}

			UT_return_if_fail(p || !pszDecor);
			gchar*	q = strtok(p, " ");

			while (q)
			{
				if (0 == strcmp(q, "line-through"))
				{
					_handleAttributeOff((char) 13);  // FIXME: use defines
				}

				q = strtok(NULL, " ");
			}

			FREEP(p);
		}
		
		if (
			(pAP->getProperty("text-decoration", szValue))
			)
		{
			const gchar* pszDecor = szValue;

			gchar* p;
			if (!(p = g_strdup(pszDecor)))
			{
				// TODO outofmem
			}

			UT_return_if_fail(p || !pszDecor);
			gchar*	q = strtok(p, " ");

			while (q)
			{
				if (0 == strcmp(q, "underline"))
				{
					_handleAttributeOff((char) 14);  // FIXME: use defines
				}

				q = strtok(NULL, " ");
			}

			FREEP(p);
		}		
	}
}

void WordPerfect_Listener::_handleVariableGroup(char group, char subgroup, char flags, short sizeNonDelData, char * /*nonDelData*/)
{
	UT_DEBUGMSG(("WordPerfect Listener: _handleVariableGroup\n"));

	short size = 1 + 1 + 2 + 1 + 2 + sizeNonDelData + 0 + 2 + 1;

	*(m_pie->m_buffer) += group;
	*(m_pie->m_buffer) += subgroup;
	m_pie->_UT_String_add(*(m_pie->m_buffer), size);
	*(m_pie->m_buffer) += flags;
	m_pie->_UT_String_add(*(m_pie->m_buffer), sizeNonDelData);
	// nondeletable data with length sizeNonDelData goes here
	m_pie->_UT_String_add(*(m_pie->m_buffer), size);
	*(m_pie->m_buffer) += group;
}

void WordPerfect_Listener::_handleTabGroup(char tabDef)
{
	UT_DEBUGMSG(("WordPerfect Listener: _handleTabGroup\n"));

	*(m_pie->m_buffer) += WP_TOP_TAB_GROUP;
	*(m_pie->m_buffer) += tabDef;
	m_pie->_UT_String_add(*(m_pie->m_buffer), (short)10); // size
	*(m_pie->m_buffer) += (char)0x00;
	m_pie->_UT_String_add(*(m_pie->m_buffer), (short)0);
	m_pie->_UT_String_add(*(m_pie->m_buffer), (short)10); // size
	*(m_pie->m_buffer) += WP_TOP_TAB_GROUP;
}

void WordPerfect_Listener::_handleAttributeOn(char attribute)
{
	UT_DEBUGMSG(("WordPerfect Listener: _handleAttributeOn\n"));
	*(m_pie->m_buffer) += WP_TOP_ATTRIBUTE_ON;
	*(m_pie->m_buffer) += attribute;
	*(m_pie->m_buffer) += WP_TOP_ATTRIBUTE_ON;
}

void WordPerfect_Listener::_handleAttributeOff(char attribute)
{
	UT_DEBUGMSG(("WordPerfect Listener: _handleAttributeOff\n"));
	*(m_pie->m_buffer) += WP_TOP_ATTRIBUTE_OFF;
	*(m_pie->m_buffer) += attribute;
	*(m_pie->m_buffer) += WP_TOP_ATTRIBUTE_OFF;
}

void WordPerfect_Listener::_handleFontSizeChange(double points)
{
	UT_DEBUGMSG(("WordPerfect Listener: _handleFontSizeChange: points=%5.3f\n", points));
	
	*(m_pie->m_buffer) += WP_TOP_CHARACTER_GROUP;
	*(m_pie->m_buffer) += 0x1B; // font size change
	m_pie->_UT_String_add(*(m_pie->m_buffer), (short)21); // size
	*(m_pie->m_buffer) += 0x80; // flags = PRFXID, 0x80 seems to be NORMAL
	*(m_pie->m_buffer) += 0x01; // number of PIDs = 1
	m_pie->_UT_String_add(*(m_pie->m_buffer), (short)0x0001); // old desired typeface descriptor PID (type=55) - FIXME: hardcoded 1
	m_pie->_UT_String_add(*(m_pie->m_buffer), (short)0x0008); // size of non deletable-information = 8
	m_pie->_UT_String_add(*(m_pie->m_buffer), (short)rint((points*50.0f))); // desired font size (3600ths) ???
	m_pie->_UT_String_add(*(m_pie->m_buffer), (short)0x0000); // hash (matched typeface description), ???
	m_pie->_UT_String_add(*(m_pie->m_buffer), (short)++m_pie->m_desiredFontUseCount); // matched font index in font list
	m_pie->_UT_String_add(*(m_pie->m_buffer), (short)rint((points*50.0f))); // matched point size of font (3600ths) ???
	m_pie->_UT_String_add(*(m_pie->m_buffer), (short)21); // size
	*(m_pie->m_buffer) += WP_TOP_CHARACTER_GROUP;
}

void WordPerfect_Listener::_handleParagraphJustification(char mode)
{
	UT_DEBUGMSG(("WordPerfect Listener: _handleParagraphJustification\n"));
	
	*(m_pie->m_buffer) += WP_TOP_PARAGRAPH_GROUP;
	*(m_pie->m_buffer) += WP_PARAGRAPH_GROUP_JUSTIFICATION;
	m_pie->_UT_String_add(*(m_pie->m_buffer), (short)11); // size
	*(m_pie->m_buffer) += (char)0x00; // flags = 0
	m_pie->_UT_String_add(*(m_pie->m_buffer), (short)0x0001); // size of non-deletable information = 1
	*(m_pie->m_buffer) += mode; // new justification mode
	m_pie->_UT_String_add(*(m_pie->m_buffer), (short)11); // size
	*(m_pie->m_buffer) += WP_TOP_PARAGRAPH_GROUP;
}
