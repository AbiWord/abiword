
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
