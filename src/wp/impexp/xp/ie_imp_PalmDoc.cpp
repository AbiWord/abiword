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
#include "ie_imp_PalmDoc.h"
#include "pd_Document.h"
#include "ut_growbuf.h"
#include "xap_EncodingManager.h"

/*****************************************************************/
/*****************************************************************/

/*
  Import US-ASCII (actually Latin-1) data from a Palm Doc
  database file.  We allow either LF or CR or CRLF line
  termination.  Each line terminator is taken to be a
  paragraph break.
*/

static void
_zero_fill( register Byte *p, register int len )
{
    while ( len-- > 0 )
        *p++ = '\0';
}

/*****************************************************************/
/*****************************************************************/

#define X_CleanupIfError(error,exp)	do { if (((error)=(exp)) != UT_OK) goto Cleanup; } while (0)

UT_Error IE_Imp_PalmDoc::importFile(const char * szFilename)
{
	m_pdfp = fopen(szFilename, "rb");
	if (!m_pdfp)
	{
		UT_DEBUGMSG(("Could not open file %s\n",szFilename));
		return UT_IE_FILENOTFOUND;
	}
	
	UT_Error error;

	X_CleanupIfError(error,_writeHeader(m_pdfp));
	X_CleanupIfError(error,_parseFile(m_pdfp));

	error = UT_OK;

Cleanup:
	fclose(m_pdfp);
	return error;
}

#undef X_CleanupIfError

/*****************************************************************/
/*****************************************************************/

IE_Imp_PalmDoc::~IE_Imp_PalmDoc()
{
}

IE_Imp_PalmDoc::IE_Imp_PalmDoc(PD_Document * pDocument)
	: IE_Imp(pDocument)
{
	m_pdfp = 0;
	m_numRecords = 0;
	m_fileSize = 0;
	m_buf = new buffer;
	_zero_fill (m_buf->buf, BUFFER_SIZE);
	m_buf->len = BUFFER_SIZE;
	m_buf->position = 0;

	_selectSwap();
}

/*****************************************************************/
/*****************************************************************/

#define X_ReturnIfFail(exp,error)		do { bool b = (exp); if (!b) return (error); } while (0)
#define X_ReturnNoMemIfError(exp)	X_ReturnIfFail(exp,UT_IE_NOMEMORY)

UT_Error IE_Imp_PalmDoc::_writeHeader(FILE * /* m_pdfp */)
{
	X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Section, NULL));
	return UT_OK;
}

UT_Error IE_Imp_PalmDoc::_parseFile(FILE * m_pdfp)
{
	UT_GrowBuf gbBlock(1024);
	bool bEatLF = false;
	bool bEmptyFile = true;
	UT_UCSChar c;
	wchar_t wc;

	pdb_header	m_header;
	doc_record0	m_rec0;
	bool		bCompressed = false;
	int		num_records, rec_num;
	DWord		file_size, offset;

	fread( &m_header, PDB_HEADER_SIZE, 1, m_pdfp );
	if (strncmp( m_header.type,    DOC_TYPE,    sizeof(m_header.type) ) ||
	    strncmp( m_header.creator, DOC_CREATOR, sizeof(m_header.creator) ))
        {
		UT_DEBUGMSG(("This is not a DOC file!\n"));

		// Create an empty paragraph.
		X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Block, NULL));
		return UT_OK;
	}

	num_records = _swap_Word( m_header.numRecords ) - 1;

	fseek( m_pdfp, PDB_HEADER_SIZE, SEEK_SET );
	GET_DWord( m_pdfp, offset );
	fseek( m_pdfp, offset, SEEK_SET );
	fread( &m_rec0, sizeof(m_rec0), 1, m_pdfp );

	if ( _swap_Word( m_rec0.version ) == 2 )
		bCompressed = true;

	fseek( m_pdfp, 0, SEEK_END );
	file_size = ftell( m_pdfp );

	for (rec_num = 1; rec_num <= num_records; ++rec_num )
	{
		DWord next_offset;

		fseek( m_pdfp, PDB_HEADER_SIZE + PDB_RECORD_HEADER_SIZE * rec_num, SEEK_SET);
		GET_DWord( m_pdfp, offset );
		if( rec_num < num_records )
		{
			fseek( m_pdfp, PDB_HEADER_SIZE + PDB_RECORD_HEADER_SIZE * (rec_num + 1), SEEK_SET);
			GET_DWord( m_pdfp, next_offset );
		}
		else
			next_offset = file_size;

		fseek( m_pdfp, offset, SEEK_SET );

		// be overly cautious here
		_zero_fill (m_buf->buf, BUFFER_SIZE);
		m_buf->position = fread( m_buf->buf, 1, next_offset - offset, m_pdfp );

		if ( bCompressed )
			_uncompress( m_buf );

		m_buf->position = 0;

		while ( (m_buf->position) < (m_buf->len) )
		{
		  // don't copy over null chars
		        if (m_buf->buf[m_buf->position] == '\0')
			  {
			    ++m_buf->position;
			    continue;
			  }
			if( !m_Mbtowc.mbtowc( wc, m_buf->buf[m_buf->position] ) )
		 	   continue;
			c = (UT_UCSChar)wc;
			switch (c)
			{
			case (UT_UCSChar)'\r':
			case (UT_UCSChar)'\n':
			
				if ((c == (UT_UCSChar)'\n') && bEatLF)
				{
					bEatLF = false;
					break;
				}

				if (c == (UT_UCSChar)'\r')
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
					X_ReturnNoMemIfError(m_pDocument->appendSpan(gbBlock.getPointer(0), gbBlock.getLength()));
					gbBlock.truncate(0);
				}
				break;

			default:
				bEatLF = false;
				X_ReturnNoMemIfError(gbBlock.ins(gbBlock.getLength(),&c,1));
				break;
			}

			++m_buf->position;
		} 

	}
	if (gbBlock.getLength() > 0 || bEmptyFile)
	{
		// if we have text left over (without final CR/LF),
		// or if we read an empty file,
		// create a paragraph and emit the text now.
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

void IE_Imp_PalmDoc::pasteFromBuffer(PD_DocumentRange * pDocRange,
				     unsigned char * pData, UT_uint32 lenData)
{
}

/*****************************************************************/
/*****************************************************************/

bool IE_Imp_PalmDoc::RecognizeContents(const char * szBuf, UT_uint32 iNumbytes)
{
        pdb_header	* header;

	// not enough bytes to make a good enough guess
	if (!iNumbytes || (iNumbytes < sizeof (pdb_header)))
	  return(false);

	// evil, type unsafe cast
	header = (pdb_header *) szBuf;

	if (strncmp( header->type,    DOC_TYPE,    sizeof(header->type) ) ||
	    strncmp( header->creator, DOC_CREATOR, sizeof(header->creator) ))
        {
		return false;
	}

	return(true);
}

bool IE_Imp_PalmDoc::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".pdb") == 0);
}

UT_Error IE_Imp_PalmDoc::StaticConstructor(PD_Document * pDocument,
										IE_Imp ** ppie)
{
	IE_Imp_PalmDoc * p = new IE_Imp_PalmDoc(pDocument);
	*ppie = p;
	return UT_OK;
}

bool	IE_Imp_PalmDoc::GetDlgLabels(const char ** pszDesc,
				     const char ** pszSuffixList,
				     IEFileType * ft)
{
	*pszDesc = "Palm Document (.pdb)";
	*pszSuffixList = "*.pdb";
	*ft = IEFT_PalmDoc;
	return true;
}

bool IE_Imp_PalmDoc::SupportsFileType(IEFileType ft)
{
	return (IEFT_PalmDoc == ft);
}

/*****************************************************************/
/*****************************************************************/

void IE_Imp_PalmDoc::_selectSwap()
{
    union { char c[2];  Word n; }  w;
    strncpy(  w.c, "\1\2",     2 );

    if ( w.n == 0x0201 )
        m_littlendian = true;
    else
        m_littlendian = false;

}  

Word IE_Imp_PalmDoc::_swap_Word( Word r )
{
    if (m_littlendian)
    {
        return (r >> 8) | (r << 8);
    }
    else
    {
        return r;
    }
}

DWord IE_Imp_PalmDoc::_swap_DWord( DWord r )
{
    if (m_littlendian)
    {
        return ( (r >> 24) & 0x00FF ) | (r << 24) | ( (r >> 8) & 0xFF00 ) | ( (r << 8) & 0xFF0000 );
    }
    else
    {
        return r;
    }
}

void IE_Imp_PalmDoc::_uncompress( buffer *m_buf )
{
	buffer *m_new_buf = new buffer;
	UT_uint16 i, j;
	Byte c;

	// set all of these to 0 initially
	_zero_fill (m_new_buf->buf, BUFFER_SIZE);

	for (i = j = 0; i < m_buf->position; )
	{
		c = m_buf->buf[ i++ ];

		if ( c >= 1 && c <= 8 )
			while ( c-- )
				m_new_buf->buf[ j++ ] = m_buf->buf[ i++ ];

		else if ( c <= 0x7F )
			m_new_buf->buf[ j++ ] = c;

		else if ( c >= 0xC0 )
		{
			m_new_buf->buf[ j++ ] = ' ';
			m_new_buf->buf[ j++ ] = c ^ 0x80;
		}
		else
		{
			int di, n;
			c = (c << 8) + m_buf->buf[ i++ ];
			di = (c & 0x3FFF) >> COUNT_BITS;
			for (n = (c & ((1 << COUNT_BITS) - 1)) + 3; n--; ++j )
				m_new_buf->buf[ j ] = m_new_buf->buf[ j - di ];
		}
	}


	memcpy( (void *)m_buf->buf, (void *)m_new_buf->buf, (size_t) j );

	m_buf->position = j;
	delete( m_new_buf );
}
