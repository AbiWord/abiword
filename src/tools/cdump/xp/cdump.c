/* AbiSource Build Tools
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

long _getFileLength(const char* pszFileName)
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

long _readEntireFile(const char* pszFileName, unsigned char* pBytes, unsigned long iLen)
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

void _dumpHexCBytes(FILE* fp, const unsigned char* pBytes, long iLen)
{
	long i;

	for (i=0; i<iLen; i++)
	{
		if (i
			&& ((i % 16) == 0))
		{
			fprintf(fp, "\n");
		}
		
		fprintf(fp, "0x%02x,", pBytes[i]);
	}

	fprintf(fp, "\n");
}

int main(int argc, char** argv)
{
	long iLen;
	unsigned char* pBytes;
	
	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s datafile arrayname\n", argv[0]);

		return -1;
	}

	iLen = _getFileLength(argv[1]);
	pBytes = malloc(iLen);

	_readEntireFile(argv[1], pBytes, iLen);
	
	printf("unsigned char %s[] = {\n", argv[2]);
	_dumpHexCBytes(stdout, pBytes, iLen);
	printf("};\n");

	return 0;
}
