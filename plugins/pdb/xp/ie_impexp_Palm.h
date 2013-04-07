/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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

#ifndef IE_IMPEXP_PALM_H
#define IE_IMPEXP_PALM_H

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut_types.h"

/*****************************************************************************
 *
 *	Define integral type Byte, Word, and DWord to match those on the
 *	Pilot being 8, 16, and 32 bits, respectively.
 *
 *****************************************************************************/

typedef UT_uint8 Byte;
typedef UT_uint16 Word;
typedef UT_uint32 DWord;

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

#define	PUT_Word(f,n)	{  Word n1 = _swap_Word ( n ); gsf_output_write(f, 2, (guint8*)&n1); }
#define	PUT_DWord(f,n)	{ DWord n1 = _swap_DWord( n ); gsf_output_write(f, 4, (guint8*)&n1); }

#define	GET_Word(f,n)	{ gsf_input_read( f, 2, (guint8*)&n ); n = _swap_Word ( n ); }
#define	GET_DWord(f,n)	{ gsf_input_read( f, 4, (guint8*)&n ); n = _swap_DWord( n ); }

#endif
