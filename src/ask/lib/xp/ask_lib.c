/* AbiSource Setup Kit
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
#include <string.h>

#include <zlib.h>

#include "ask.h"

/*
  TODO get rid of the following
*/

#define CHECK_ERR(err, msg) { \
    if (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", msg, err); \
        exit(1); \
    } \
}

const char* ASK_getBaseFileName(const char* pszFileName)
{
	const char* p = strrchr(pszFileName, '/');
	if (p)
	{
		return p+1;
	}
	else
	{
		return pszFileName;
	}
}

long ASK_getFileLength(const char* pszFileName)
{
	long iLengthOfFile;
	
	FILE* fp = fopen(pszFileName, "rb");
	if (!fp)
	{
		return -1;
	}

	if (0 != fseek(fp, 0, SEEK_END))
	{
		fclose(fp);
		
		return -1;
	}

	iLengthOfFile = ftell(fp);

	fclose(fp);

	return iLengthOfFile;
}

long ASK_readEntireFile(const char* pszFileName, unsigned char* pBytes, unsigned long iLen)
{
	FILE* fp = fopen(pszFileName, "rb");
	
	if (!fp)
	{
		return -1;
	}

	if (iLen != fread(pBytes, 1, iLen, fp))
	{
		fclose(fp);

		return -1;
	}

	fclose(fp);

	return iLen;
}

void ASK_dumpHexCBytes(FILE* fp, const unsigned char* pBytes, long iLen)
{
	long i;

	for (i=0; i<iLen; i++)
	{
		if (i
			&& ((i % 16) == 0))
		{
			fprintf(fp, "\n");
		}
		
		fprintf(fp, "0x%02x, ", pBytes[i]);
	}

	fprintf(fp, "\n");
}

long ASK_compressBuffer(const unsigned char* pOriginalBytes, long iLenOriginal, unsigned char* pDest, long iLenDest)
{
    z_stream c_stream; /* compression stream */
    int err;

    c_stream.zalloc = (alloc_func)0;
    c_stream.zfree = (free_func)0;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_stream.next_in  = (Bytef*)pOriginalBytes;
	c_stream.avail_in = iLenOriginal;
    c_stream.next_out = pDest;
	c_stream.avail_out = iLenDest;

	err = deflate(&c_stream, Z_NO_FLUSH);
	CHECK_ERR(err, "deflate");

	err = deflate(&c_stream, Z_FINISH);

    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");

	return c_stream.total_out;
}

void ASK_decompressBuffer(const unsigned char* pCompressedBytes, long iCompressedLength, unsigned char* pOriginalBytes, long iOriginalLength)
{
    int err;
    z_stream d_stream; /* decompression stream */

    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = (unsigned char*) pCompressedBytes;	/* cast away const */
    d_stream.avail_in = iCompressedLength;
    d_stream.next_out = pOriginalBytes;
	d_stream.avail_out = iOriginalLength;

    err = inflateInit(&d_stream);

	err = inflate(&d_stream, Z_NO_FLUSH);

    err = inflateEnd(&d_stream);
}

unsigned char* ASK_decompressFile(ASK_DataFile* pDataFile)
{
	unsigned char* pOriginalBytes;

	/*
	  We allocate one extra byte and zero the whole thing, just
	  in case this file is a text file and we want to later treat
	  it as a C string.  If this is not a text file, then this
	  is harmless, although a bit inefficient.
	*/
	pOriginalBytes = calloc(pDataFile->iOriginalLength + 1, 1);
	if (pDataFile->bNoCompress)
	{
		memcpy(pOriginalBytes, pDataFile->pCompressedBytes, pDataFile->iOriginalLength);
	}
	else
	{
		ASK_decompressBuffer(pDataFile->pCompressedBytes, pDataFile->iCompressedLength, pOriginalBytes, pDataFile->iOriginalLength);
	}

	return pOriginalBytes;
}

int ASK_decompressAndWriteFile(ASK_DataFile* pDataFile)
{
	FILE* fp;
	unsigned char* pOriginalBytes = ASK_decompressFile(pDataFile);

	if (pDataFile->bNoCopy)
	{
		/* skip it */
		return 0;
	}
	
	fp = fopen(pDataFile->szInstallPath, "wb");
	if (!fp)
	{
		return -1;
	}
	
	fwrite(pOriginalBytes, 1, pDataFile->iOriginalLength, fp);
	fclose(fp);

	ASK_setFileAttributes(pDataFile->szInstallPath, pDataFile->iAttributes);
	ASK_setFileModTime(pDataFile->szInstallPath, pDataFile->iModTime);

	return 0;
}

long ASK_getFileSetTotalSizeInBytes(ASK_FileSet* pFileSet)
{
	long len = 0;
	int i;

	for (i=0; i<pFileSet->iNumFilesInSet; i++)
	{
		len += pFileSet->aFiles[i]->iOriginalLength;
	}

	return len;
}

void ASK_convertBytesToString(long n, char* pszStrings)
{
	if (n < 1024)
	{
		sprintf(pszStrings, "%d Bytes", n);

		return;
	}

	if (n < (1024 * 1024))
	{
		sprintf(pszStrings, "%d KB", n / 1024);

		return;
	}

	sprintf(pszStrings, "%4.1f MB", n / (1024 * 1024.0));
}

