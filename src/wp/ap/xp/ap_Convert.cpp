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

#include <stdio.h>
#include <string.h>

#include "ap_Convert.h"
#include "ie_exp.h"
#include "ut_types.h"

//////////////////////////////////////////////////////////////////

AP_Convert::AP_Convert(void)
{
    m_iVerbose = 1;
}

AP_Convert::~AP_Convert(void)
{
}

/////////////////////////////////////////////////////////////////

void AP_Convert::convertTo(const char * szSourceFilename,
							IEFileType sourceFormat,
							const char * szTargetFilename,
							IEFileType targetFormat)
{
	PD_Document * pNewDoc = new PD_Document();
	UT_Error error;
	UT_ASSERT(pNewDoc);

	if (m_iVerbose > 1)
		printf("AbiWord: [%s] -> [%s]\tStarting conversion...\n", szSourceFilename, szTargetFilename);

	error = pNewDoc->readFromFile(szSourceFilename, sourceFormat);

	switch (error) {
	case UT_OK:
		error = pNewDoc->saveAs(szTargetFilename, targetFormat);
		
		switch (error) {
		case UT_OK:
			if (m_iVerbose > 1)
				printf("AbiWord: [%s] -> [%s]\tConversion ok!\n", szSourceFilename, szTargetFilename);
			break;
		case UT_SAVE_EXPORTERROR:
			if (m_iVerbose > 0)
				fprintf(stderr, "AbiWord: Uch! Are you sure that you've specified a valid exporter?\n");
			break;
		case UT_SAVE_WRITEERROR:
			if (m_iVerbose > 0)
				fprintf(stderr, "AbiWord: Uch! Could not write the file [%s]\n", szTargetFilename);
			break;
		default:
			if (m_iVerbose > 0)
				fprintf(stderr, "AbiWord: could not write the file [%s]\n", szTargetFilename);
		}

		break;
	case UT_INVALIDFILENAME:
		if (m_iVerbose > 0)
			fprintf(stderr, "AbiWord: [%s] is not a valid file name.\n", szSourceFilename);
		break;
	case UT_IE_NOMEMORY:
		if (m_iVerbose > 0)
			fprintf(stderr, "AbiWord: Arrrgh... I don't have enough memory!\n");
		break;
	case UT_NOPIECETABLE:
		// TODO
	default:
		if (m_iVerbose > 0)
			fprintf(stderr, "AbiWord: could not open the file [%s]\n", szSourceFilename);
	}


	UNREFP(pNewDoc);
}

void AP_Convert::convertTo(const char * szFilename, const char * szTargetSuffix)
{
	char ext[256] = ".";
	IEFileType ieft;
	char file[256];
	char *tmp;

	strncat(ext, szTargetSuffix, 255);
	ieft = IE_Exp::fileTypeForSuffix(ext);
	strncpy(file, szFilename, 255);

	tmp = strrchr(file, '.');
	if (tmp != NULL)
		*tmp = '\0';

	strncat(file, ext, 255 - strlen(file));

	convertTo(szFilename, IEFT_Unknown, file, ieft);
}

void AP_Convert::convertTo(const char * szFilename, const char * szSourceSuffix, const char * szTargetSuffix)
{
	char ext[256] = ".";
	char sourceExt[256] = ".";
	IEFileType ieft;
	IEFileType sourceIeft;
	char file[256];
	char *tmp;

	strncat(ext, szTargetSuffix, 255);
	strncat(sourceExt, szSourceSuffix, 255);
	ieft = IE_Exp::fileTypeForSuffix(ext);
	sourceIeft = IE_Exp::fileTypeForSuffix(sourceExt);
	strncpy(file, szFilename, 255);

	tmp = strrchr(file, '.');
	if (tmp != NULL)
		*tmp = '\0';

	strncat(file, ext, 255 - strlen(file));

	convertTo(szFilename, sourceIeft, file, ieft);
}

void AP_Convert::setVerbose(int level)
{
	if ((level >= 0) && (level <= 2))
		m_iVerbose = level;
}
