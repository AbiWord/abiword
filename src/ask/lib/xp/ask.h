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

#ifndef ASK_H
#define ASK_H

#include <stdio.h>

#define ASK_MAX_PATH	1023

typedef struct
{
	unsigned char*	pCompressedBytes;
	unsigned long	iCompressedLength;
	unsigned long	iOriginalLength;
	char*			pszFileName;
	char*			pszRelPath;
	char*			pszDesktopShortcut;
	char*			pszProgramsShortcut;
	unsigned int	iAttributes;
	unsigned int	iModTime;
	int				bNoCopy;
	int				bNoRemove;
	int				bNoCompress;
	
	/* TODO bKeywords, hash */
	
	char			szInstallPath[ASK_MAX_PATH+1];
} ASK_DataFile;

extern ASK_DataFile*	g_aDataFiles[];
extern long				g_iNumDataFiles;
extern ASK_DataFile*	g_pReadMeFile;
extern ASK_DataFile*	g_pLicenseFile;
extern ASK_DataFile*	g_pGraphicFile;

typedef struct
{
	char*			pszName;
	char*			pszDefaultPath;
	char*			pszDirName;
	char*			pszKeyword;
	long			iNumFilesInSet;
	int				bFixedPath;
	ASK_DataFile**	aFiles;

	char			szInstallPath[ASK_MAX_PATH+1];
} ASK_FileSet;

extern ASK_FileSet*		g_aFileSets[];
extern long				g_iNumFileSets;

/*
  XP functions
*/
const char* ASK_getBaseFileName(const char* pszFileName);
long ASK_getFileLength(const char* pszFileName);
long ASK_readEntireFile(const char* pszFileName, unsigned char* pBytes, unsigned long iLen);
void ASK_dumpHexCBytes(FILE* fp, const unsigned char* pBytes, long iLen);
long ASK_compressBuffer(const unsigned char* pOriginalBytes, long iLenOriginal, unsigned char* pDest, long iLenDest);
unsigned char* ASK_decompressFile(ASK_DataFile* pDataFile);
void ASK_decompressBuffer(const unsigned char* pCompressedBytes, long iCompressedLength, unsigned char* pOriginalBytes, long iOriginalLength);
int ASK_decompressAndWriteFile(ASK_DataFile* pDataFile);

long ASK_getFileSetTotalSizeInBytes(ASK_FileSet* pFileSet);
long ASK_getFileSetTotalSizeInBytesToCopy(ASK_FileSet* pFileSet);
long ASK_getFileSetTotalFilesToCopy(ASK_FileSet* pFileSet);
void ASK_convertBytesToString(long n, char* pszStrings);
void ASK_convert64BitsToString(long high, long low, char* pszStrings);
int ASK_isFileNewer(char* pszFileName, unsigned int iModTime);

/*
  Functions implemented in platform code
*/

void ASK_createRemoveFile(char* pszName, char *pszProgramDir);
void ASK_registerForRemove( char *pszProgramDir );
unsigned int ASK_getFileModTime(const char* pszFileName);
void ASK_setFileModTime(const char* pszFileName, unsigned int iModTime);
unsigned int ASK_getFileAttributes(const char* pszFileName);
void ASK_setFileAttributes(const char* pszFileName, unsigned int iAttributes);
int ASK_getDiskFreeSpace(const char* pszDir, long *HighPart, long *LowPart);
void ASK_fixSlashes(char* pszPath);
int ASK_isDirectory(char* pszFile);
int ASK_fileExists(char* pszFile);
int ASK_verifyDirExists(char* szDir);

void ASK_CreateDesktopShortcuts(int iNumSets, ASK_FileSet** ppSets);

int ASK_DoScreen_welcome(char* pszHeading, char* pszIntro, char* pszFinePrint);
int ASK_DoScreen_readme(char* pszTitle, char* pszText);
int ASK_DoScreen_license(char* pszText);
int ASK_DoScreen_chooseDirForFileSet(ASK_FileSet* pSet);
int ASK_DoScreen_readyToCopy(int iNumSets, ASK_FileSet** ppSets);
int ASK_DoScreen_copy(int iNumSets, ASK_FileSet** ppSets);
int ASK_DoScreen_copyComplete(int iNumSets, ASK_FileSet** ppSets);

int ASK_YesNo(char* pszTitle, char* pszMessage);

void ASK_PopulateStartMenu(char* pszGroupName, int iNumSets, ASK_FileSet** ppSets);

void ASK_PlatformSpecificInstructions(FILE* fpOut);
int lib_main(int argc, char** argv);

#endif /* ASK_H */


