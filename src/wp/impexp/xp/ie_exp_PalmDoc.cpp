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
	: IE_Exp_Text(pDocument), m_pdfp(0), m_numRecords(0),
	  m_fileSize(0)
{
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

#ifdef ENABLE_PLUGINS

// completely generic code to allow this to be a plugin

#include "xap_Module.h"

#define SUPPORTS_ABI_VERSION(a,b,c) (((a==0)&&(b==7)&&(c==15)) ? 1 : 0)

// we use a reference-counted sniffer
static IE_Exp_PalmDoc_Sniffer * m_sniffer = 0;
static UT_sint32 m_refs = 0;

ABI_FAR extern "C"
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_refs && !m_sniffer)
	{
		m_sniffer = new IE_Exp_PalmDoc_Sniffer ();
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

	mi->name = "PalmDoc Exporter";
	mi->desc = "Export PalmDoc Documents";
	mi->version = "0.7.15";
	mi->author = "Abi the Ant";
	mi->usage = "No Usage";

	IE_Exp::registerExporter (m_sniffer);
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
	IE_Exp::unregisterExporter (m_sniffer);
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

bool IE_Exp_PalmDoc_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!UT_stricmp(szSuffix,".pdb"));
}

UT_Error IE_Exp_PalmDoc_Sniffer::constructExporter(PD_Document * pDocument,
											   IE_Exp ** ppie)
{
	IE_Exp_PalmDoc * p = new IE_Exp_PalmDoc(pDocument);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_PalmDoc_Sniffer::getDlgLabels(const char ** pszDesc,
										  const char ** pszSuffixList,
										  IEFileType * ft)
{
	*pszDesc = "PalmDoc (.pdb)";
	*pszSuffixList = "*.pdb";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

bool IE_Exp_PalmDoc::_openFile(const char * szFilename)
{
    UT_ASSERT(!m_pdfp);

    m_pdfp = fopen(szFilename, "wb");
    m_index = 0x406f8000;
    m_recOffset = 0x00001000;

    if (m_pdfp != 0)
    {
        /********** create and write m_header **********************************/

        UT_DEBUGMSG(("Creating PDF header....\n"));
        _zero_fill( m_header.name, sizeof(m_header.name) );
        strncpy( m_header.name, szFilename, sizeof(m_header.name) - 1 );
        if ( strlen( szFilename ) > sizeof(m_header.name) - 1 )
            strncpy( m_header.name + sizeof(m_header.name) - 4, "...", 3 );
        m_header.attributes		= 0;
        m_header.version		= 0;

	// TODO: make these report the actual # of seconds since 
	// TODO: January 1, 1904 (in 4 bytes, no less)
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
        m_header.numRecords		= 0;		/* placeholder - value will be added later*/

        fwrite( &m_header, PDB_HEADER_SIZE, 1, m_pdfp );

        UT_DEBUGMSG(("Creating rec0 offset & index....\n"));
        PUT_DWord( m_pdfp, m_recOffset );
        PUT_DWord( m_pdfp, m_index++ );

        UT_DEBUGMSG(("Creating rec0....\n"));
        fseek( m_pdfp, m_recOffset, SEEK_SET);

        m_rec0.version		= _swap_Word (2); /* 1 = plain text, 2 = compressed text */
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

        _compress( m_buf );

        fseek( m_pdfp, PDB_HEADER_SIZE + PDB_RECORD_HEADER_SIZE * m_numRecords, SEEK_SET );
        PUT_DWord( m_pdfp, m_recOffset );
        PUT_DWord( m_pdfp, m_index++ );

        fseek( m_pdfp, m_recOffset, SEEK_SET );
        fwrite( m_buf->buf, m_buf->len, 1, m_pdfp);
        m_recOffset = ftell( m_pdfp );
        m_numRecords++;
        m_fileSize += BUFFER_SIZE;

	delete m_buf;
        m_buf = new buffer;
        m_buf->len = BUFFER_SIZE;
        m_buf->position = 0;

        xxx_UT_DEBUGMSG(("m_numRecords = %d\n", m_numRecords));
        _writeBytes( pBytes + i, length - i );
    }

    return length;
}

bool IE_Exp_PalmDoc::_writeBytes(const UT_Byte * sz)
{
    UT_ASSERT(m_pdfp);
    UT_ASSERT(sz);
    int length = strlen((const char *)sz);
    UT_ASSERT(length);
    
    return (_writeBytes(sz,length)==(UT_uint32)length);
}

bool IE_Exp_PalmDoc::_closeFile(void)
{
    UT_ASSERT(m_pdfp);
    UT_ASSERT(m_buf);
    xxx_UT_DEBUGMSG(("m_numRecords = %d\n", m_numRecords));

    _compress( m_buf );

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

    fclose(m_pdfp);
    UT_DEBUGMSG(("File closed....\n"));

    m_pdfp = 0;
    return true;
}

/*****************************************************************/
/*****************************************************************/

void IE_Exp_PalmDoc::_selectSwap()
{
    union { char c[2];  Word n; }  w;
    strncpy(  w.c, "\1\2",     2 );

    if ( w.n == 0x0201 )
        m_littlendian = true;
    else
        m_littlendian = false;

}  

void IE_Exp_PalmDoc::_compress( buffer *b )
{
    UT_uint16 i, current = 0;
    bool   space = false;

    buffer *original = new buffer;
    original->len = b->len;
    original->position = b->position;
    memcpy( (void *)original->buf, (void *)b->buf , BUFFER_SIZE );

    UT_uint16 look_ahead;
    //bytes to look ahead for type A codes, should be 7 unless near end of buffer

    UT_uint16 copy_ahead;	//bytes ahead to actually copy for type A codes, max 8

    Byte window[2048];
    // window to search for type B codes, max 2047 unless near beginning of buffer

//    UT_uint16 copy_back;	//number of bytes to copy for type B codes, range 3-10

    b->position= 0;

    UT_DEBUGMSG(("Enter compress loop.\n"));
    UT_DEBUGMSG(("Original position = %i.\n", original->position));
    while ( current < (original->position) )
    {
        UT_DEBUGMSG(("Compress loop.  Checking hex character %X\n", original->buf[current]));
        /*  If last char was a space, see if current char can be squished into it */
        if ( space )
        {
            UT_DEBUGMSG(("Previous character was a space.\n"));
            space = false;

            if ( original->buf[current] >= 0x40 && original->buf[current] <= 0x7F )
            {
                UT_DEBUGMSG(("Current character fit into previous space.\n"));
                //current char squished into space
                b->buf[ b->position++ ] = original->buf[current] | 0x80;
                ++current;
            }
            else
            {
                UT_DEBUGMSG(("Current character did not fit into previous space.\n"));
                b->buf[ b->position++ ] = ' ';
                //can't fit space with current char - reprocess current char
            }

            continue;
        }

        /*  Check current character if it is a space */
        if ( original->buf[current] == ' ' )
        {
            UT_DEBUGMSG(("Current character is a space.\n"));
            space = true;
            ++current;
            continue;
        }

        /*  Check the current & next seven bytes for high characters */
        UT_DEBUGMSG(("Checking for high characters.\n"));
        if (( original->position - current ) >= 7 )
            look_ahead = 7;
        else
            look_ahead = original->position - current - 1;

        copy_ahead = 0;
        UT_DEBUGMSG(("look_ahead=%i.\n", look_ahead));
        UT_DEBUGMSG(("copy_ahead=%i.\n", copy_ahead));

        for ( i = 0 ; i <= look_ahead ; ++i )
        {
            if ( original->buf[current + i] > 0x7f )
            {
                UT_DEBUGMSG(("Found a high character.\n"));
                copy_ahead = i + 1;		//found high character
            }
        }

        UT_DEBUGMSG(("copy_ahead=%i.\n", copy_ahead));
        if ( copy_ahead > 0 )
        {
            b->buf[ b->position++ ] = (unsigned char)copy_ahead;	//number of bytes ahead to copy
            for ( i = 0 ; i < copy_ahead ; ++i)
                b->buf[ b->position++ ] = original->buf[current];
            ++current;
            continue;
        }

        /* Check for matching sequence in previous 2047 bytes */
	/* Create sliding window */
        UT_DEBUGMSG(("current=%i.\n", current));
        if ( current < 2047)
        {
            memcpy( (void *)window, (void *)original->buf, (size_t)current );
        }
        else
        {
            memcpy( (void *)window, (void *) &original->buf[ current - 2047], 2048 );
        }

        // TODO: Search for matches in sliding window

        UT_DEBUGMSG(("Current character copied verbatim.\n"));
        b->buf[ b->position++ ] = original->buf[current];
        ++current;
    }
    UT_DEBUGMSG(("Exit compress loop.\n"));
    delete (original);
}

void IE_Exp_PalmDoc::_zero_fill(  char *p,  int len )
{
    while ( len-- > 0 )
        *p++ = '\0';
}

Byte* IE_Exp_PalmDoc::_mem_find(  Byte *t, int t_len,  Byte *m, int m_len )
{
     int i;
    for ( i = t_len - m_len + 1; i > 0; --i, ++t )
        if ( *t == *m && !memcmp( t, m, m_len ) )
            return t;
    return 0;
}

Word IE_Exp_PalmDoc::_swap_Word( Word r )
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

DWord IE_Exp_PalmDoc::_swap_DWord( DWord r )
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
