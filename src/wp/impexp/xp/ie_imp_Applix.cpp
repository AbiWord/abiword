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


#include <stdio.h>
#include <stdlib.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ie_imp_Applix.h"
#include "pd_Document.h"
#include "ut_growbuf.h"
#include "xap_EncodingManager.h"

/*****************************************************************/
/*****************************************************************/

/*
 * Import Applix Word documents
 * 
 * One field per line, interesting text fields start after the <WP
 * Tag. Text fields ("<T") are immediately preceded by <P fields
 * 
 * For now, we only care about the stuff in-between the <start_flow>
 * And <end_flow> tags
 *
 * Interested in Plain-text import only right now. More complex import
 * To come
 *
 * Please, someone pick up on this and do a POW! - Dom
 */

/*****************************************************************/
/*****************************************************************/

#define X_CleanupIfError(error,exp)	do { if (((error)=(exp)) != UT_OK) goto Cleanup; } while (0)

UT_Error IE_Imp_Applix::importFile(const char * szFilename)
{
	FILE *fp = fopen(szFilename, "r");
	if (!fp)
	{
		UT_DEBUGMSG(("Could not open file %s\n",szFilename));
		return UT_IE_FILENOTFOUND;
	}
	
	UT_Error error;

	X_CleanupIfError(error,_writeHeader(fp));
	X_CleanupIfError(error,_parseFile(fp));

	error = UT_OK;

Cleanup:
	fclose(fp);
	return error;
}

#undef X_CleanupIfError

/*****************************************************************/
/*****************************************************************/

IE_Imp_Applix::~IE_Imp_Applix()
{
}

IE_Imp_Applix::IE_Imp_Applix(PD_Document * pDocument)
  : IE_Imp(pDocument), m_bLastWasP(false), m_bInT(false)
{
}

/*****************************************************************/
/*****************************************************************/

#define X_ReturnIfFail(exp,error)		do { bool b = (exp); if (!b) return (error); } while (0)
#define X_ReturnNoMemIfError(exp)	X_ReturnIfFail(exp,UT_IE_NOMEMORY)

UT_Error IE_Imp_Applix::_writeHeader(FILE * /* fp */)
{
	X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Section, NULL));

	return UT_OK;
}

// the applix tags that i know about && maybe handle
typedef enum {APPLIX_T,
			  GLOBALS_T,
			  START_STYLES_T,
			  STYLE_T,
			  COLOR_T,
			  END_STYLES_T,
			  START_FLOW_T,			  
			  WP400_T,
			  T_T,
			  P_T,
			  END_FLOW_T,
			  START_VARS_T,
			  V_T,
			  END_VARS_T,
			  END_DOCUMENT_T,
			  NOT_A_TAG, 
			  tag_Unknown} Applix_tag_t;

typedef struct {
	char * name;
	Applix_tag_t tag;
} Applix_mapping_t;

static Applix_mapping_t axwords[] =
{
	{"Applix", APPLIX_T},
	{"Globals", GLOBALS_T},
	{"start_styles", START_STYLES_T},
	{"style", STYLE_T},
	{"color", COLOR_T},
	{"end_styles", END_STYLES_T},
	{"start_flow", START_FLOW_T},
	{"WP400", WP400_T},
	{"T", T_T},
	{"P", P_T},
	{"end_flow", END_FLOW_T},
	{"start_vars", START_VARS_T},
	{"V", V_T},
	{"end_vars", END_VARS_T},
	{"end_document", END_DOCUMENT_T}
};

// these should both be case insensitive
#define AX_STR_CMP(x, y) UT_strcmp((x), (y))
#define AX_STRN_CMP(x, y, z) strncmp((x), (y), (z))

#define APPLIX_LINE_LENGTH 80 // applix does 80 to a line
#define nAxWords (sizeof(axwords) / sizeof(axwords[1]))

static Applix_tag_t
s_name_2_tag (const char *name, size_t n)
{
	if (!name || !n)
		return NOT_A_TAG;
	
	// simple lookup. TODO: make me faster (HT + BTree probably)
	for (size_t i = 0; i < nAxWords; i++)
	{
		if (!AX_STRN_CMP (name, axwords[i].name, n))
		{
			xxx_UT_DEBUGMSG(("DOM: applix tag is %s\n", axwords[i].name));
			return axwords[i].tag;
		}
	}
	
	return tag_Unknown;
}

// must free returned string
static Applix_tag_t
s_getTagName(const char *str, size_t len)
{
	if (!len || !str)
		return NOT_A_TAG;

	// innocent, just for fast pointer comparison
	char * ptr = (char *)str;
	
	xxx_UT_DEBUGMSG(("DOM: Applix string: %s (%d)\n", str, len));
	
	if(*ptr == '<')
	{
		ptr++;
		while(ptr && !UT_UCS_isspace(*ptr) && (*ptr != '>'))
		{
			ptr++;
		}
		if (ptr)
		{
			char buf [APPLIX_LINE_LENGTH + 1];

			size_t n = ptr - str - 1;
			strncpy (buf, str+1, n);
			buf[n] = 0;

			xxx_UT_DEBUGMSG(("DOM: calling with %s\n", buf));

			return s_name_2_tag(buf, n);
		}
	}
	return NOT_A_TAG;
}

UT_Error IE_Imp_Applix::_parseFile(FILE * fp)
{
	char buf [APPLIX_LINE_LENGTH + 1];
	size_t len = 0;

	while (!feof (fp))
	{
		fgets (buf, APPLIX_LINE_LENGTH, fp);
		if (buf)
		{
			len = strlen(buf);

			// todo: make me more robust
			// grammars? we don't need no stinkin' grammars! ;-(
			s_getTagName(buf, len);
		}
	}

	return UT_OK;
}

#undef X_ReturnNoMemIfError
#undef X_ReturnIfFail

/*****************************************************************/
/*****************************************************************/

void IE_Imp_Applix::pasteFromBuffer(PD_DocumentRange * pDocRange,
								  unsigned char * pData, UT_uint32 lenData)
{
	return;
}

/*****************************************************************/
/*****************************************************************/

bool IE_Imp_Applix::RecognizeContents(const char * szBuf, UT_uint32 iNumbytes)
{
	// this should be suffecient, at least for my liking
	const char * magic = "<Applix Words>";

	if (!AX_STRN_CMP(szBuf, magic, strlen(magic)))
		return true;
	return false;
}

bool IE_Imp_Applix::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".aw") == 0);
}

UT_Error IE_Imp_Applix::StaticConstructor(PD_Document * pDocument,
										IE_Imp ** ppie)
{
	IE_Imp_Applix * p = new IE_Imp_Applix(pDocument);
	*ppie = p;
	return UT_OK;
}

bool	IE_Imp_Applix::GetDlgLabels(const char ** pszDesc,
									const char ** pszSuffixList,
									IEFileType * ft)
{
	// TOOD: get the real filename extension used
	*pszDesc = "Applix Word (.aw)";
	*pszSuffixList = "*.aw";
	*ft = IEFT_APPLIX;
	return true;
}

bool IE_Imp_Applix::SupportsFileType(IEFileType ft)
{
	return (IEFT_APPLIX == ft);
}

