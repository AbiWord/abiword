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

#include "ut_xml.h"

#include "ask.h"

long g_iNumDataFiles;
long g_iNumFileSets;
int g_iReadMeNum = -1;
int g_iLicenseNum = -1;
int g_iGraphicNum = -1;

struct data_file_list_node
{
	int iNum;
	struct data_file_list_node* pNext;
};

struct fileset
{
	char szName[511+1];
	char szDefaultPath[511+1];
	char szDirName[511+1];
	char szKeyword[255+1];
	struct data_file_list_node* pHead;
	int iNumFiles;
	int	bFixedPath;

	struct fileset* pNext;
};

struct fileset* pCurrentSet;
struct fileset* pFirstSet;
struct fileset* pLastSet;

static void startElement(void *userData, const XML_Char *name, const XML_Char **atts)
{
	FILE* fpOut = (FILE*) userData;

	if (0 == strcmp(name, "fileset"))
	{
		const char* pszFileSetName = NULL;
		const char* pszDefaultPath = NULL;
		const char* pszDirName = NULL;
		const char* pszKeyword = NULL;
		int         bFixedPath = 0;
		const char** ppAtt = atts;

		if (pCurrentSet)
		{
			fprintf(stderr, "<fileset> tags may not be nested\n");

			exit(-1);
		}
		
		while (ppAtt[0])
		{
			if (0 == strcmp(ppAtt[0], "name"))
			{
				pszFileSetName = ppAtt[1];
			}
			else if (0 == strcmp(ppAtt[0], "defaultpath"))
			{
				pszDefaultPath = ppAtt[1];
			}
			else if (0 == strcmp(ppAtt[0], "dirname"))
			{
				pszDirName = ppAtt[1];
			}
			else if (0 == strcmp(ppAtt[0], "keyword"))
			{
				pszKeyword = ppAtt[1];
			}
			else if (0 == strcmp(ppAtt[0], "fixedpath"))
			{
				bFixedPath = 1;
			}

			ppAtt++;
			ppAtt++;
		}

		if (!pszFileSetName)
		{
			fprintf(stderr, "ERROR:  missing 'name' attribute on <fileset> tag\n");

			exit(-1);
		}

		pCurrentSet = calloc(1, sizeof(*pCurrentSet));
		if (!pCurrentSet)
		{
			fprintf(stderr, "ERROR: calloc failure on calloc(struct fileset)\n");

			exit(-1);
		}

		strcpy(pCurrentSet->szName, pszFileSetName);
		if (pszDefaultPath)
		{
			strcpy(pCurrentSet->szDefaultPath, pszDefaultPath);
		}
		else
		{
			pCurrentSet->szDefaultPath[0] = 0;
		}
		
		if (pszDirName)
		{
			strcpy(pCurrentSet->szDirName, pszDirName);
		}
		else
		{
			pCurrentSet->szDirName[0] = 0;
		}
		
		if (pszKeyword)
		{
			strcpy(pCurrentSet->szKeyword, pszKeyword);
		}
		else
		{
			pCurrentSet->szKeyword[0] = 0;
		}
		
		pCurrentSet->iNumFiles = 0;
		pCurrentSet->bFixedPath = bFixedPath;
		
		if (pLastSet)
		{
			pLastSet->pNext = pCurrentSet;
		}
		else
		{
			pFirstSet = pLastSet = pCurrentSet;
		}

		pLastSet = pCurrentSet;
		pLastSet->pNext = NULL;

		g_iNumFileSets++;
	}
	else if (0 == strcmp(name, "datafile"))
	{
		int iDataFileNum = g_iNumDataFiles++;
		long iOriginalLength;
		long iCompressedLength;
		unsigned char* pOriginalBytes = NULL;
		unsigned char* pCompressedBytes = NULL;
		const char* pszDataFileName = NULL;
		const char* pszRename = NULL;
		const char* pszRelPath = NULL;
		const char* pszDesktopShortcut = NULL;
		const char* pszProgramsShortcut = NULL;
		int bNoCopy = 0;
		int bNoRemove = 0;
		int bNoCompress = 0;

		const char** ppAtt = atts;
		while (ppAtt[0])
		{
			if (0 == strcmp(ppAtt[0], "name"))
			{
				pszDataFileName = ppAtt[1];
			}
			else if (0 == strcmp(ppAtt[0], "rename"))
			{
				pszRename = ppAtt[1];
			}
			else if (0 == strcmp(ppAtt[0], "nocopy"))
			{
				bNoCopy = 1;
			}
			else if (0 == strcmp(ppAtt[0], "noremove"))
			{
				bNoRemove = 1;
			}
			else if (0 == strcmp(ppAtt[0], "nocompress"))
			{
				bNoCompress = 1;
			}
			else if (0 == strcmp(ppAtt[0], "graphic"))
			{
				g_iGraphicNum = iDataFileNum;
			}
			else if (0 == strcmp(ppAtt[0], "relpath"))
			{
				pszRelPath = ppAtt[1];
			}
			else if (0 == strcmp(ppAtt[0], "readme"))
			{
				g_iReadMeNum = iDataFileNum;
			}
			else if (0 == strcmp(ppAtt[0], "license"))
			{
				g_iLicenseNum = iDataFileNum;
			}
			else if (0 == strcmp(ppAtt[0], "desktop_shortcut"))
			{
				pszDesktopShortcut = ppAtt[1];
			}
			else if (0 == strcmp(ppAtt[0], "programs_shortcut"))
			{
				pszProgramsShortcut = ppAtt[1];
			}

			ppAtt++;
			ppAtt++;
		}

		if (!pCurrentSet)
		{
			fprintf(stderr, "ERROR:  <datafile> tag must not appear outside of <fileset>\n");

			exit(-1);
		}
		
		if (!pszDataFileName)
		{
			fprintf(stderr, "ERROR:  missing 'name' attribute on <datafile> tag\n");

			exit(-1);
		}
		
		iOriginalLength = ASK_getFileLength(pszDataFileName);
		if (iOriginalLength < 0)
		{
			fprintf(stderr, "Could not determine length of file %s\n", pszDataFileName);

			exit(-1);
		}
		
		pOriginalBytes = malloc(iOriginalLength);
		if (!pOriginalBytes)
		{
			fprintf(stderr, "Could not malloc %ld bytes for data file %s\n", iOriginalLength, pszDataFileName);

			exit(-1);
		}

		pCompressedBytes = malloc(iOriginalLength);
		if (!pCompressedBytes)
		{
			fprintf(stderr, "Could not malloc %ld bytes for data file %s\n", iOriginalLength, pszDataFileName);

			exit(-1);
		}
		
		if (iOriginalLength != ASK_readEntireFile(pszDataFileName, pOriginalBytes, iOriginalLength))
		{
			fprintf(stderr, "Could not read file %s\n", pszDataFileName);

			exit(-1);
		}

		if (bNoCompress)
		{
			iCompressedLength = iOriginalLength;
			memcpy(pCompressedBytes, pOriginalBytes, iOriginalLength);
		}
		else
		{
			iCompressedLength = ASK_compressBuffer(pOriginalBytes, iOriginalLength, pCompressedBytes, iOriginalLength);
			if (iCompressedLength < 0)
			{
				fprintf(stderr, "Failed compress.\n");

				exit(-1);
			}
		}

		fprintf(fpOut, "unsigned char _data%05d_compressed_bytes[] = {\n", iDataFileNum);
		ASK_dumpHexCBytes(fpOut, pCompressedBytes, iCompressedLength);
		fprintf(fpOut, "};\n");

		fprintf(fpOut, "ASK_DataFile _data%05d = {\n", iDataFileNum);
		fprintf(fpOut, "\t_data%05d_compressed_bytes,\n", iDataFileNum);
		fprintf(fpOut, "\t%ld,\n", iCompressedLength);
		fprintf(fpOut, "\t%ld,\n", iOriginalLength);
		if (pszRename)
		{
			fprintf(fpOut, "\t\"%s\",\n", ASK_getBaseFileName(pszRename));
		}
		else
		{
			fprintf(fpOut, "\t\"%s\",\n", ASK_getBaseFileName(pszDataFileName));
		}
		if (pszRelPath)
		{
			fprintf(fpOut, "\t\"%s\",\n", pszRelPath);
		}
		else
		{
			fprintf(fpOut, "\tNULL,\n");
		}
		
		if (pszDesktopShortcut)
		{
			fprintf(fpOut, "\t\"%s\",\n", pszDesktopShortcut);
		}
		else
		{
			fprintf(fpOut, "\tNULL,\n");
		}
		if (pszProgramsShortcut)
		{
			fprintf(fpOut, "\t\"%s\",\n", pszProgramsShortcut);
		}
		else
		{
			fprintf(fpOut, "\tNULL,\n");
		}
		fprintf(fpOut, "\t0x%x,\n", ASK_getFileAttributes(pszDataFileName));
		fprintf(fpOut, "\t0x%x,\n", ASK_getFileModTime(pszDataFileName));
		fprintf(fpOut, "\t%d,\n", bNoCopy);
		fprintf(fpOut, "\t%d,\n", bNoRemove);
		fprintf(fpOut, "\t%d\n", bNoCompress);
		fprintf(fpOut, "};\n");
		
		fprintf(fpOut, "\n");

		{
			struct data_file_list_node* pNode = calloc(1, sizeof(struct data_file_list_node));
			/* TODO outofmem */
			pNode->iNum = iDataFileNum;
			pNode->pNext = pCurrentSet->pHead;
			pCurrentSet->pHead = pNode;
		}

		if (pOriginalBytes)
		{
			free(pOriginalBytes);
		}

		if (pCompressedBytes)
		{
			free(pCompressedBytes);
		}
	}
}

static void endElement(void *userData, const XML_Char *name)
{
	FILE* fpOut = (FILE*) userData;
	
	if (0 == strcmp(name, "fileset"))
	{
		pCurrentSet = NULL;
	}
}

static void charData(void* userData, const XML_Char *s, int len)
{
	FILE* fpOut = (FILE*) userData;
	
}

void parseInputFile(char* pszInputFile, FILE* fpOut)
{
	XML_Parser parser = NULL;
	FILE *fp = NULL;
	int done = 0;
	char buf[4096];

	fp = fopen(pszInputFile, "r");
	if (!fp)
	{
		fprintf(stderr, "Could not open input file (%s)\n", pszInputFile);
		exit(-1);
	}
	
	parser = XML_ParserCreate(NULL);
	XML_SetUserData(parser, fpOut);
	XML_SetElementHandler(parser, startElement, endElement);
	XML_SetCharacterDataHandler(parser, charData);

	while (!done)
	{
		size_t len = fread(buf, 1, sizeof(buf), fp);
		done = (len < sizeof(buf));

		if (!XML_Parse(parser, buf, len, done)) 
		{
			fprintf(stderr, "%s at line %d\n",
						 XML_ErrorString(XML_GetErrorCode(parser)),
						 XML_GetCurrentLineNumber(parser));
			exit(-1);
		}
	} 

	if (parser)
	{
		XML_ParserFree(parser);
	}
	
	if (fp)
	{
		fclose(fp);
	}
}

int lib_main(int argc, char** argv)
{
	FILE* fpOut;
	int i;
	
	if (argc != 3)
	{
	    fprintf(stderr, "Usage:  %s inputfile outputfile\n", argv[0]);
		
		return -1;
	}

	fpOut = fopen(argv[2], "w");
	if (!fpOut)
	{
		fprintf(stderr, "Could not open output file (%s) for writing.\n", argv[2]);

		return -1;
	}

	fprintf(fpOut, "/* This file was automatically generated by the AbiSource Setup Kit. */\n");
	fprintf(fpOut, "#include \"ask.h\"\n\n");
	ASK_PlatformSpecificInstructions(fpOut);

    parseInputFile(argv[1], fpOut);

	fprintf(fpOut, "\nlong g_iNumDataFiles = %ld;\n", g_iNumDataFiles);
	fprintf(fpOut, "ASK_DataFile* g_aDataFiles[] = {\n");
	for (i=0; i<g_iNumDataFiles; i++)
	{
		fprintf(fpOut, "\t&_data%05d,\n", i);
	}
	fprintf(fpOut, "};\n");

	fprintf(fpOut, "\nlong g_iNumFileSets = %ld;\n", g_iNumFileSets);

	{
		int iSetNum = 0;
		
		pCurrentSet = pFirstSet;
		while (pCurrentSet)
		{
			long iNumFilesInSet = 0;
			fprintf(fpOut, "\nASK_DataFile* _fileset%05d_files[] = {\n", iSetNum);
			{
				struct data_file_list_node* pNode = pCurrentSet->pHead;
				while (pNode)
				{
					fprintf(fpOut, "\t&_data%05d,\n", pNode->iNum);

					iNumFilesInSet++;
					pNode = pNode->pNext;
				}
			}
			fprintf(fpOut, "};\n");

			fprintf(fpOut, "ASK_FileSet _fileset%05d = {\n", iSetNum);
			fprintf(fpOut, "\t\"%s\",\n", pCurrentSet->szName);
			fprintf(fpOut, "\t\"%s\",\n", pCurrentSet->szDefaultPath);
			fprintf(fpOut, "\t\"%s\",\n", pCurrentSet->szDirName);
			fprintf(fpOut, "\t\"%s\",\n", pCurrentSet->szKeyword);
			fprintf(fpOut, "\t%ld,\n", iNumFilesInSet);
			fprintf(fpOut, "\t%d,\n", pCurrentSet->bFixedPath);
			fprintf(fpOut, "\t_fileset%05d_files\n", iSetNum);
			fprintf(fpOut, "};\n");
			
			pCurrentSet = pCurrentSet->pNext;
			iSetNum++;
		}
	}

	fprintf(fpOut, "\n");

	fprintf(fpOut, "ASK_FileSet* g_aFileSets[] = {\n");
	for (i=0; i<g_iNumFileSets; i++)
	{
		fprintf(fpOut, "\t&_fileset%05d,\n", i);
	}
	fprintf(fpOut, "};\n");

	fprintf(fpOut, "\n");
	
	if (g_iReadMeNum >= 0)
	{
		fprintf(fpOut, "ASK_DataFile* g_pReadMeFile = &_data%05d;\n", g_iReadMeNum);
	}
	else
	{
		fprintf(fpOut, "ASK_DataFile* g_pReadMeFile = NULL;\n");
	}
	
	if (g_iLicenseNum >= 0)
	{
		fprintf(fpOut, "ASK_DataFile* g_pLicenseFile = &_data%05d;\n", g_iLicenseNum);
	}
	else
	{
		fprintf(fpOut, "ASK_DataFile* g_pLicenseFile = NULL;\n");
	}
	
	if (g_iGraphicNum >= 0)
	{
		fprintf(fpOut, "ASK_DataFile* g_pGraphicFile = &_data%05d;\n", g_iGraphicNum);
	}
	else
	{
		fprintf(fpOut, "ASK_DataFile* g_pGraphicFile = NULL;\n");
	}
	
	fprintf(fpOut, "\n");
	
	fclose(fpOut);

	/* TODO free the linked lists */
	
	return 0;
}
