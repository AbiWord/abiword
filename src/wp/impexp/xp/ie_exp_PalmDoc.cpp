/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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


#include "string.h"

#include "ie_exp_PalmDoc.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"

IE_Exp_PalmDoc::IE_Exp_PalmDoc(PD_Document * pDocument)
	: IE_Exp_Text(pDocument)
{
    m_pdfp = 0;
    m_numRecords = 0;
    m_fileSize = 0;
    m_buf = new buffer;
    m_buf->len = BUFFER_SIZE;
    m_buf->position = 0;

    _selectSwap();
}

IE_Exp_PalmDoc::~IE_Exp_PalmDoc()
{
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Exp_PalmDoc::RecognizeSuffix(const char * szSuffix)
{
    return (UT_stricmp(szSuffix,".pdb") == 0);
}

UT_Error IE_Exp_PalmDoc::StaticConstructor(PD_Document * pDocument,
	IE_Exp ** ppie)
{
    *ppie = new IE_Exp_PalmDoc(pDocument);
    return UT_OK;
}

UT_Bool	IE_Exp_PalmDoc::GetDlgLabels(const char ** pszDesc,
	const char ** pszSuffixList,
	IEFileType * ft)
{
    *pszDesc = "Palm Document (.pdb)";
    *pszSuffixList = "*.pdb";
    *ft = IEFT_PalmDoc;
    return UT_TRUE;
}

UT_Bool IE_Exp_PalmDoc::SupportsFileType(IEFileType ft)
{
    return (IEFT_PalmDoc == ft);
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Exp_PalmDoc::_openFile(const char * szFilename)
{
    UT_ASSERT(!m_pdfp);

    m_pdfp = fopen(szFilename, "wb");
    m_index = 0x406f8000;
    m_recOffset = 0x00001000;

    if (m_pdfp != 0)
    {
        /********** create and write m_header **********************************/

        UT_DEBUGMSG(("Creating PDF header....\n"));
        _zero_fill( m_header.name, sizeof m_header.name );
        strncpy( m_header.name, szFilename, sizeof m_header.name - 1 );
        if ( strlen( szFilename ) > sizeof m_header.name - 1 )
            strncpy( m_header.name + sizeof m_header.name - 4, "...", 3 );
        m_header.attributes		= 0;
        m_header.version		= 0;
        strncpy( (char*)&m_header.create_time, "\x06\xD1\x44\xAE", 4 );
        strncpy( (char*)&m_header.modify_time, "\x06\xD1\x44\xAE", 4 );
        m_header.backup_time		= 0;
        m_header.modificationNumber	= 0;
        m_header.appInfoID		= 0;
        m_header.sortInfoID		= 0;
        strncpy( m_header.type,    DOC_TYPE,    sizeof m_header.type );
        strncpy( m_header.creator, DOC_CREATOR, sizeof m_header.creator );
        m_header.id_seed		= 0;
        m_header.nextRecordList		= 0;
        m_header.numRecords		= 0;		/* placeholder - will be added later*/

        fwrite( &m_header, PDB_HEADER_SIZE, 1, m_pdfp );

        UT_DEBUGMSG(("Creating rec0 offset & index....\n"));
        PUT_DWord( m_pdfp, m_recOffset );
        PUT_DWord( m_pdfp, m_index++ );

        UT_DEBUGMSG(("Creating rec0....\n"));
        fseek( m_pdfp, m_recOffset, SEEK_SET);

        m_rec0.version		= 0x0100;	/* 1 = uncompressed, 2 = compressed */
        m_rec0.reserved1	= 0;
        m_rec0.doc_size		= 0;
        m_rec0.numRecords	= 0;
        m_rec0.rec_size		= _swap_Word( RECORD_SIZE_MAX );
        m_rec0.reserved2	= 0;

        fwrite( &m_rec0, sizeof m_rec0, 1, m_pdfp );

        m_recOffset = ftell( m_pdfp );
        m_numRecords++;
        xxx_UT_DEBUGMSG(("m_numRecords = %d\n", m_numRecords));
    }

    return (m_pdfp !=0);

}

UT_uint32 IE_Exp_PalmDoc::_writeBytes(const UT_Byte * pBytes, UT_uint32 length)
{
    UT_ASSERT(m_pdfp);
    UT_ASSERT(pBytes);
    UT_ASSERT(length);
    UT_ASSERT(m_buf);

    UT_uint32 i;

    if ( (m_buf->position + length) <= m_buf->len)
    {
        UT_DEBUGMSG(("Copying into buffer....\n"));

        for (i = 0; i < length; i++)
        {
            m_buf->buf[ m_buf->position + i ] = pBytes[ i ];
        }

        m_buf->position += i;
        xxx_UT_DEBUGMSG(("m_buf->position = %d\n", m_buf->position));
        xxx_UT_DEBUGMSG(("m_numRecords = %d\n", m_numRecords));
    }
    else
    {
        UT_DEBUGMSG(("Copying into buffer & writing to file....\n"));
        for (i = 0; i < (m_buf->len - m_buf->position); i++)
        {
            m_buf->buf[ m_buf->position + i ] = pBytes[ i ];
        }

        m_buf->position += i;
/*
        _remove_binary ( m_buf );
        _compress( m_buf );
*/
        fseek( m_pdfp, PDB_HEADER_SIZE + PDB_RECORD_HEADER_SIZE * m_numRecords, SEEK_SET );
        PUT_DWord( m_pdfp, m_recOffset );
        PUT_DWord( m_pdfp, m_index++ );

        fseek( m_pdfp, m_recOffset, SEEK_SET );
        fwrite( m_buf->buf, m_buf->len, 1, m_pdfp);
        m_recOffset = ftell( m_pdfp );
        m_numRecords++;
        m_fileSize += BUFFER_SIZE;

        m_buf = new buffer;
        m_buf->len = BUFFER_SIZE;
        m_buf->position = 0;

        xxx_UT_DEBUGMSG(("m_numRecords = %d\n", m_numRecords));
        _writeBytes( pBytes + i, length - i );
    }

    return length;
}

UT_Bool IE_Exp_PalmDoc::_writeBytes(const UT_Byte * sz)
{
    UT_ASSERT(m_pdfp);
    UT_ASSERT(sz);
    int length = strlen((const char *)sz);
    UT_ASSERT(length);
    
    return (_writeBytes(sz,length)==(UT_uint32)length);
}

UT_Bool IE_Exp_PalmDoc::_closeFile(void)
{
    UT_ASSERT(m_pdfp);
    UT_ASSERT(m_buf);
    xxx_UT_DEBUGMSG(("m_numRecords = %d\n", m_numRecords));
/*
    _remove_binary ( m_buf );
    _compress( m_buf );
*/
    UT_DEBUGMSG(("Writing final buffer offset & index to file....\n"));
    fseek( m_pdfp, PDB_HEADER_SIZE + PDB_RECORD_HEADER_SIZE * m_numRecords, SEEK_SET );
    PUT_DWord( m_pdfp, m_recOffset );
    PUT_DWord( m_pdfp, m_index++ );

    UT_DEBUGMSG(("Writing final buffer to file....\n"));
    fseek( m_pdfp, m_recOffset, SEEK_SET );
    UT_DEBUGMSG(("m_recOffset = %d\n", m_recOffset));
    UT_DEBUGMSG(("m_buf->len = %d\n", m_buf->len));
    UT_DEBUGMSG(("m_buf->position = %d\n", m_buf->position));
    fwrite( m_buf->buf, m_buf->position, 1, m_pdfp);

    m_numRecords++;
    xxx_UT_DEBUGMSG(("m_numRecords = %d\n", m_numRecords));
    m_fileSize += m_buf->position;

    UT_DEBUGMSG(("Updating file header....\n"));
    m_header.numRecords = _swap_Word( m_numRecords );

    fseek( m_pdfp, 0, SEEK_SET);
    fwrite( &m_header, PDB_HEADER_SIZE, 1, m_pdfp );

    UT_DEBUGMSG(("Updating rec0....\n"));
    xxx_UT_DEBUGMSG(("m_fileSize = %d\n", m_fileSize));
    xxx_UT_DEBUGMSG(("m_numRecords = %d\n", m_numRecords));
    m_rec0.doc_size	= _swap_DWord( m_fileSize);
    m_rec0.numRecords	= _swap_Word( m_numRecords - 1 );

    fseek( m_pdfp, 4096, SEEK_SET);
    fwrite( &m_rec0, sizeof m_rec0, 1, m_pdfp );

    UT_DEBUGMSG(("Closing file....\n"));
    fclose(m_pdfp);

    m_pdfp = 0;
    return UT_TRUE;
}

/*****************************************************************/
/*****************************************************************/

void IE_Exp_PalmDoc::_selectSwap()
{
    union { char c[2];  Word n; }  w;
    strncpy(  w.c, "\1\2",     2 );

    if ( w.n == 0x0201 )
        m_littlendian = UT_TRUE;
    else
        m_littlendian = UT_FALSE;

}  

void IE_Exp_PalmDoc::_compress( buffer *b )
{
    UT_uint32 i, j;
    UT_Bool space = UT_FALSE;

    Byte *buf_orig;
    Byte *p;	/* walking test hit; works up on successive matches */
    Byte *p_prev;
    Byte *head;	/* current test string */
    Byte *tail;	/* 1 past the current test buffer */
    Byte *end;	/* 1 past the end of the input buffer */

    p = p_prev = head = buf_orig = b->buf;
    tail = head + 1;
    end = b->buf + b->len;

    b = new buffer;
    b->len = 0;

    /* loop, absorbing one more char from the input buffer on each pass */
    while ( head != end ) {
        /* establish where the scan can begin */
        if ( head - p_prev > (( 1 << DISP_BITS )-1) )
            p_prev = head - (( 1 << DISP_BITS )-1);

        /* scan in the previous data for a match */
        p = _mem_find( p_prev, tail - p_prev, head, tail - head );

        /* on a mismatch or end of buffer, issued codes */
            if ( !p || p == head || tail - head > ( 1 << COUNT_BITS ) + 2
            || tail == end
        ) {
            /* issued the codes */
            /* first, check for short runs */
            if ( tail - head < 4 )
                _put_byte( b, *head++, &space );
            else {
                unsigned dist = head - p_prev;
                unsigned compound = (dist << COUNT_BITS)
                    + tail - head - 4;

                if ( dist >= ( 1 << DISP_BITS ) ||
                    tail - head - 4 > 7
                )
                    fprintf( stderr,
                        "error: dist overflow\n");

                /* for longer runs, issue a run-code */
                /* issue space char if required */
                if ( space )
                    b->buf[ b->len++ ] = ' ', space = UT_FALSE;

                b->buf[ b->len++ ] = 0x80 + ( compound >> 8 );
                b->buf[ b->len++ ] = compound & 0xFF;
                head = tail - 1;/* and start again */
            }
            p_prev = buf_orig;	/* start search again */
        } else
            p_prev = p;		/* got a match */

        /* when we get to the end of the buffer, don't inc past the */
        /* end; this forces the residue chars out one at a time */
        if ( tail != end )
            ++tail;
    }
    free( buf_orig );

    if ( space )
        b->buf[ b->len++ ] = ' ';	/* add left-over space */

    /* final scan to merge consecutive high chars together */
    for ( i = j = 0; i < b->len; ++i, ++j ) {
        b->buf[ j ] = b->buf[ i ];

        /* skip run-length codes */
        if ( b->buf[ j ] >= 0x80 && b->buf[ j ] < 0xC0 )
            b->buf[ ++j ] = b->buf[ ++i ];

        /* if we hit a high char marker, look ahead for another */
        else if ( b->buf[ j ] == '\1' ) {
            b->buf[ j + 1 ] = b->buf[ i + 1 ];
            while ( i + 2 < b->len &&
                b->buf[ i + 2 ] == 1 && b->buf[ j ] < 8
            ) {
                b->buf[ j ]++;
                b->buf[ j + b->buf[ j ] ] = b->buf[ i + 3 ];
                i += 2;
            }
            j += b->buf[ j ];
            ++i;
        }
    }
    b->len = j;
}

void IE_Exp_PalmDoc::_zero_fill( register char *p, register int len )
{
    while ( len-- > 0 )
        *p++ = '\0';
}

void IE_Exp_PalmDoc::_put_byte( register buffer *b, Byte c, UT_Bool *space )
{
    if ( *space ) {
        *space = UT_FALSE;
        /*
        ** There is an outstanding space char: see if we can squeeze it
        ** in with an ASCII char.
        */
        if ( c >= 0x40 && c <= 0x7F ) {
            b->buf[ b->len++ ] = c ^ 0x80;
            return;
        }
        b->buf[ b->len++ ] = ' ';	/* couldn't squeeze it in */
    } else if ( c == ' ' ) {
        *space = UT_FALSE;
        return;
    }

    if ( c >= 1 && c <= 8 || c >= 0x80 )
        b->buf[ b->len++ ] = '\1';

    b->buf[ b->len++ ] = c;
}

void IE_Exp_PalmDoc::_remove_binary( buffer *b )
{
    buffer *new_buf = new buffer;
    UT_uint32 i, j;
    for ( i = j = 0; i < b->len; ++i ) {
        if ( b->buf[ i ] < 9 )		/* discard really low ASCII */
            continue;
        switch ( b->buf[ i ] ) {

            case '\r':
                if ( i < b->len - 1 && b->buf[ i + 1 ] == '\n' )
                    continue;	/* CR+LF -> LF */
				/* no break; */

            case '\f':
                new_buf->buf[ j ] = '\n';
                break;

            default:
                new_buf->buf[ j ] = b->buf[ i ];
        }
        ++j;
    }
    memcpy(b->buf, new_buf->buf, j);
    b->len = j;
    delete new_buf;
}

Byte* IE_Exp_PalmDoc::_mem_find( register Byte *t, int t_len, register Byte *m, int m_len )
{
    register int i;
    for ( i = t_len - m_len + 1; i > 0; --i, ++t )
        if ( *t == *m && !memcmp( t, m, m_len ) )
            return t;
    return 0;
}

Word IE_Exp_PalmDoc::_swap_Word( Word r )
{
    if (m_littlendian)
    {
        UT_DEBUGMSG(("System is Little Endian - byte swapping required.\n"));
        return (r >> 8) | (r << 8);
    }
    else
    {
        UT_DEBUGMSG(("System is Big Endian - no byte swapping required.\n"));
        return r;
    }
}

DWord IE_Exp_PalmDoc::_swap_DWord( DWord r )
{
    if (m_littlendian)
    {
        UT_DEBUGMSG(("System is Little Endian - byte swapping required.\n"));
        return ( (r >> 24) & 0x00FF ) | (r << 24) | ( (r >> 8) & 0xFF00 ) | ( (r << 8) & 0xFF0000 );
    }
    else
    {
        UT_DEBUGMSG(("System is Big Endian - no byte swapping required.\n"));
        return r;
    }
}
