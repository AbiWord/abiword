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


#ifndef IE_IMP_PALMDOC_H
#define IE_IMP_PALMDOC_H

#include <stdio.h>
#include "ie_imp.h"
#include"ut_mbtowc.h"
class PD_Document;

// The importer/reader for Palm Doc Database Files.

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

#define	GET_Word(f,n)	{ fread( &n, 2, 1, f ); n = _swap_Word ( n ); }
#define	GET_DWord(f,n)	{ fread( &n, 4, 1, f ); n = _swap_DWord( n ); }

class IE_Imp_PalmDoc_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;

public:
	IE_Imp_PalmDoc_Sniffer() {}
	virtual ~IE_Imp_PalmDoc_Sniffer() {}

	virtual bool recognizeContents (const char * szBuf, 
									UT_uint32 iNumbytes);
	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);

};

class IE_Imp_PalmDoc : public IE_Imp
{
public:
	IE_Imp_PalmDoc(PD_Document * pDocument);
	~IE_Imp_PalmDoc();

	virtual UT_Error	importFile(const char * szFilename);
	virtual void		pasteFromBuffer(PD_DocumentRange * pDocRange,
										unsigned char * pData, UT_uint32 lenData);
	
protected:
	UT_Error			_parseFile(FILE * fp);
	UT_Error			_writeHeader(FILE * fp);
	UT_Mbtowc 			m_Mbtowc;

        void				_selectSwap();
	void				_uncompress( buffer* );
	Byte*				_mem_find( Byte *t, int t_len, Byte *m, int m_len );
	Word				_swap_Word( Word );
	DWord				_swap_DWord( DWord );

private:
    
        FILE *				m_pdfp;
	pdb_header			m_header;
	doc_record0			m_rec0;
	unsigned long		m_index;
	DWord				m_recOffset;
        UT_uint32			m_numRecords;
	DWord				m_fileSize;
	buffer *			m_buf;
	UT_uint32			m_bufLen;
        UT_uint32			m_bufPosition;
	bool				m_littlendian;	
};

#endif /* IE_IMP_PALMDOC_H */
