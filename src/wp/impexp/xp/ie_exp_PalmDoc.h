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


#ifndef IE_EXP_PALMDOC_H
#define IE_EXP_PALMDOC_H

#include "limits.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ie_exp_Text.h"

// The exporter/writer for PalmDoc file format.

/*****************************************************************************
 *
 *	Define integral type Byte, Word, and DWord to match those on the
 *	Pilot being 8, 16, and 32 bits, respectively.
 *
 *****************************************************************************/

#define U8_MAX		255u		/* max  8-bit unsigned */
#if	UCHAR_MAX == U8_MAX
typedef unsigned char	Byte;
#else
#error machine does not seem to support an 8-bit integral type
#endif

#define	U16_MAX		65535u		/* max 16-bit unsigned */
#if	USHRT_MAX == U16_MAX
typedef unsigned short	Word;
#else
#error machine does not seem to support a 16-bit integral type
#endif

#define	U32_MAX		4294967295ul	/* max 32-bit unsigned */
#if	USHRT_MAX == U32_MAX
typedef unsigned short	DWord;
#elif	UINT_MAX  == U32_MAX
typedef unsigned int	DWord;
#elif	ULONG_MAX == U32_MAX
typedef unsigned long	DWord;
#else
#error machine does not seem to support a 32-bit integral type
#endif

#define RECORD_SIZE_MAX	4096	/* Pilots have a fixed 4K record size */
#define BUFFER_SIZE	4096
#define COUNT_BITS	3	
#define DISP_BITS	11	
#define	DOC_CREATOR	"REAd"
#define	DOC_TYPE	"TEXt"

/*****************************************************************************
 *
 *	PDB file header
 *
 *****************************************************************************/

#define	dmDBNameLength	32	/* 31 chars + 1 null terminator */

typedef struct {		/* 78 bytes total */
	char	name[ dmDBNameLength ];
	Word	attributes;
	Word	version;
	DWord	create_time;
	DWord	modify_time;
	DWord	backup_time;
	DWord	modificationNumber;
	DWord	appInfoID;
	DWord	sortInfoID;
	char	type[4];
	char	creator[4];
	DWord	id_seed;
	DWord	nextRecordList;
	Word	numRecords;
} pdb_header;

/*
** Some compilers pad structures out to DWord boundaries so using sizeof()
** doesn't give the intended result.
*/
#define PDB_HEADER_SIZE		78
#define	PDB_RECORD_HEADER_SIZE	8

/*****************************************************************************
 *
 *	Doc record 0
 *
 *****************************************************************************/

typedef struct {		/* 16 bytes total */
	Word	version;	/* 1 = plain text, 2 = compressed */
	Word	reserved1;
	DWord	doc_size;	/* in bytes, when uncompressed */
	Word	numRecords; 	/* text rec's only; = pdb_header.numRecords-1 */
	Word	rec_size;	/* usually RECORD_SIZE_MAX */
	DWord	reserved2;
} doc_record0;

/*****************************************************************************
 *
 *	Globals
 *
 *****************************************************************************/

typedef struct {
    Byte buf[BUFFER_SIZE];
    UT_uint32	len;
    UT_uint32	position;
} buffer;

#define	PUT_Word(f,n)	{  Word n1 = _swap_Word ( n ); fwrite( &n1, 2, 1, f ); }
#define	PUT_DWord(f,n)	{ DWord n1 = _swap_DWord( n ); fwrite( &n1, 4, 1, f ); }

/*****************************************************************/
/*****************************************************************/

class IE_Exp_PalmDoc : public IE_Exp_Text
{
public:
	IE_Exp_PalmDoc(PD_Document * pDocument);
	virtual ~IE_Exp_PalmDoc();

	static UT_Bool		RecognizeSuffix(const char * szSuffix);
	static UT_Error		StaticConstructor(PD_Document * pDocument,
						  IE_Exp ** ppie);
	static UT_Bool		GetDlgLabels(const char ** pszDesc,
				             const char ** pszSuffixList,
					     IEFileType * ft);
	static UT_Bool 		SupportsFileType(IEFileType ft);
	
protected:
	UT_Bool				_openFile(const char * szFilename);
	UT_uint32			_writeBytes(const UT_Byte * pBytes, UT_uint32 length);
	UT_Bool				_writeBytes(const UT_Byte * sz);
	UT_Bool				_closeFile(void);

        void				_selectSwap();
	void				_compress( buffer* );
	Byte*				_mem_find( Byte *t, int t_len, Byte *m, int m_len );
	void				_put_byte( buffer*, Byte, UT_Bool *space );
	void				_remove_binary( buffer* );
	Word				_swap_Word( Word );
	DWord				_swap_DWord( DWord );
	void				_uncompress( buffer* );
	void				_zero_fill( char*, int len );

private:
    
        FILE *				m_pdfp;
	pdb_header			m_header;
	doc_record0			m_rec0;
	unsigned long			m_index;
	DWord				m_recOffset;
        UT_uint32			m_numRecords;
	DWord				m_fileSize;
	buffer *			m_buf;
	UT_uint32			m_bufLen;
        UT_uint32			m_bufPosition;
	UT_Bool				m_littlendian;	

};

#endif /* IE_EXP_PALMDOC */
