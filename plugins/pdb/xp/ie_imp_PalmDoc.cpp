/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

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
_zero_fill(  Byte *p,  int len )
{
    while ( len-- > 0 )
        *p++ = '\0';
}

/*****************************************************************/
/*****************************************************************/

IE_Imp_PalmDoc_Sniffer::IE_Imp_PalmDoc_Sniffer (const char * _name) :
  IE_ImpSniffer(_name)
{
  // 
}

// supported suffixes
static IE_SuffixConfidence IE_Imp_PalmDoc_Sniffer__SuffixConfidence[] = {
	{ "pdb", 	UT_CONFIDENCE_PERFECT 	},
	{ "", 	UT_CONFIDENCE_ZILCH 	}
};

const IE_SuffixConfidence * IE_Imp_PalmDoc_Sniffer::getSuffixConfidence ()
{
	return IE_Imp_PalmDoc_Sniffer__SuffixConfidence;
}

const IE_MimeConfidence * IE_Imp_PalmDoc_Sniffer::getMimeConfidence () 
{
	// mimetypes once getMimeConfidence is implemented (need to check correctness)
	// "application/x-palm-database"
	// "application/vnd.palm"
	return NULL; 
}

UT_Confidence_t IE_Imp_PalmDoc_Sniffer::recognizeContents(const char * szBuf, 
											   UT_uint32 iNumbytes)
{
	const pdb_header	* header;

	// not enough bytes to make a good enough guess
	if (!iNumbytes || (iNumbytes < sizeof (pdb_header)))
	  return(UT_CONFIDENCE_ZILCH);

	// evil, type unsafe cast
	header = reinterpret_cast<const pdb_header *>(szBuf);

	if (strncmp( header->type,    DOC_TYPE,    sizeof(header->type) ) ||
	    strncmp( header->creator, DOC_CREATOR, sizeof(header->creator) ))
        {
		return UT_CONFIDENCE_ZILCH;
	}

	return(UT_CONFIDENCE_PERFECT);
}

UT_Error IE_Imp_PalmDoc_Sniffer::constructImporter(PD_Document * pDocument,
												   IE_Imp ** ppie)
{
	IE_Imp_PalmDoc * p = new IE_Imp_PalmDoc(pDocument);
	*ppie = p;
	return UT_OK;
}

bool IE_Imp_PalmDoc_Sniffer::getDlgLabels(const char ** pszDesc,
										  const char ** pszSuffixList,
										  IEFileType * ft)
{
	*pszDesc = "Palm Document (.pdb)";
	*pszSuffixList = "*.pdb";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

#define X_CleanupIfError(error,exp)	do { if (((error)=(exp)) != UT_OK) goto Cleanup; } while (0)

UT_Error IE_Imp_PalmDoc::_loadFile(GsfInput * input)
{
	m_pdfp = (GsfInput*)g_object_ref (G_OBJECT (input));
	if (!m_pdfp)
	{
		return UT_ERROR;
	}
	
	UT_Error error;

	X_CleanupIfError(error,_writeHeader(m_pdfp));
	X_CleanupIfError(error,_parseFile(m_pdfp));

	error = UT_OK;

Cleanup:
	g_object_unref(G_OBJECT(m_pdfp));
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

UT_Error IE_Imp_PalmDoc::_writeHeader(GsfInput * /* m_pdfp */)
{
	X_ReturnNoMemIfError(appendStrux(PTX_Section, PP_NOPROPS));
	return UT_OK;
}

UT_Error IE_Imp_PalmDoc::_parseFile(GsfInput * pdfp)
{
	UT_GrowBuf gbBlock(1024);
	bool bEatLF = false;
	bool bEmptyFile = true;
	UT_UCSChar c;
	UT_UCS4Char wc;

	pdb_header	header;
	doc_record0	rec0;
	bool		bCompressed = false;
	int		num_records, rec_num;
	DWord		file_size, offset;

	gsf_input_read( pdfp, PDB_HEADER_SIZE, (guint8*)&header);
	if (strncmp( header.type,    DOC_TYPE,    sizeof(header.type) ) ||
	    strncmp( header.creator, DOC_CREATOR, sizeof(header.creator) ))
        {
		UT_DEBUGMSG(("This is not a DOC file!\n"));

		// Create an empty paragraph.
		X_ReturnNoMemIfError(appendStrux(PTX_Block, PP_NOPROPS));
		return UT_OK;
	}

	num_records = _swap_Word( header.numRecords ) - 1;

	gsf_input_seek( pdfp, PDB_HEADER_SIZE, G_SEEK_SET );
	GET_DWord( pdfp, offset );
	gsf_input_seek( pdfp, offset, G_SEEK_SET );
	gsf_input_read( pdfp, sizeof(rec0), (guint8*)&rec0);

	if ( _swap_Word( rec0.version ) == 2 )
		bCompressed = true;

	gsf_input_seek( pdfp, 0, G_SEEK_END );
	file_size = gsf_input_tell( pdfp );

	for (rec_num = 1; rec_num <= num_records; ++rec_num )
	{
		DWord next_offset;

		gsf_input_seek( pdfp, PDB_HEADER_SIZE + PDB_RECORD_HEADER_SIZE * rec_num, G_SEEK_SET);
		GET_DWord( pdfp, offset );
		if( rec_num < num_records )
		{
			gsf_input_seek( pdfp, PDB_HEADER_SIZE + PDB_RECORD_HEADER_SIZE * (rec_num + 1), G_SEEK_SET);
			GET_DWord( pdfp, next_offset );
		}
		else
			next_offset = file_size;

		gsf_input_seek( pdfp, offset, G_SEEK_SET );

		// be overly cautious here
		_zero_fill (m_buf->buf, BUFFER_SIZE);
		gsf_input_read(pdfp, next_offset - offset, m_buf->buf);
		m_buf->position = next_offset - offset;

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
			c = static_cast<UT_UCSChar>(wc);
			switch (c)
			{
			case static_cast<UT_UCSChar>('\r'):
			case static_cast<UT_UCSChar>('\n'):
			
				if ((c == static_cast<UT_UCSChar>('\n')) && bEatLF)
				{
					bEatLF = false;
					break;
				}

				if (c == static_cast<UT_UCSChar>('\r'))
				{
					bEatLF = true;
				}
		
				// we interprete either CRLF, CR, or LF as a paragraph break.
		
				// start a paragraph and emit any text that we
				// have accumulated.
				X_ReturnNoMemIfError(appendStrux(PTX_Block, PP_NOPROPS));
				bEmptyFile = false;
				if (gbBlock.getLength() > 0)
				{
					X_ReturnNoMemIfError(appendSpan(reinterpret_cast<const UT_UCSChar*>(gbBlock.getPointer(0)), gbBlock.getLength()));
					gbBlock.truncate(0);
				}
				break;

			default:
				bEatLF = false;
				X_ReturnNoMemIfError(gbBlock.ins(gbBlock.getLength(),reinterpret_cast<const UT_GrowBufElement *>(&c),1));
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
		X_ReturnNoMemIfError(appendStrux(PTX_Block, PP_NOPROPS));
		if (gbBlock.getLength() > 0)
			X_ReturnNoMemIfError(appendSpan(reinterpret_cast<const UT_UCSChar *>(gbBlock.getPointer(0)), gbBlock.getLength()));
	}

	return UT_OK;
}

#undef X_ReturnNoMemIfError
#undef X_ReturnIfFail

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

void IE_Imp_PalmDoc::_uncompress( buffer *buf )
{
	buffer *m_new_buf = new buffer;
	UT_uint16 i, j;
	Byte c;

	// set all of these to 0 initially
	_zero_fill (m_new_buf->buf, BUFFER_SIZE);

	for (i = j = 0; i < buf->position && j < BUFFER_SIZE; )
	{
		c = buf->buf[ i++ ];

		if ( c >= 1 && c <= 8 )
			while ( c-- && j < BUFFER_SIZE-1)
				m_new_buf->buf[ j++ ] = buf->buf[ i++ ];

		else if ( c <= 0x7F )
			m_new_buf->buf[ j++ ] = c;

		else if ( c >= 0xC0 && j < BUFFER_SIZE-2)
		{
			m_new_buf->buf[ j++ ] = ' ';
			m_new_buf->buf[ j++ ] = c ^ 0x80;
		}
		else
		  {
		    int di, n;
		    unsigned int temp_c = c;
		    // c--> temp_c //tomy 2001.11.13 
		    temp_c = (temp_c << 8) ;
		    temp_c = temp_c + buf->buf[ i++ ];
		    di = (temp_c & 0x3FFF) >> COUNT_BITS;
		    for (n = (temp_c & ((1 << COUNT_BITS) - 1)) + 3; n-- && j < BUFFER_SIZE
			   ; ++j )
		      m_new_buf->buf[ j ] = m_new_buf->buf[ j - di ];
		    temp_c = 0;
		  }
	}
	UT_ASSERT(j <= BUFFER_SIZE);

	memcpy( static_cast<void *>(buf->buf), static_cast<void *>(m_new_buf->buf), static_cast<size_t>(j) );

	buf->position = j;
	delete( m_new_buf );
}
