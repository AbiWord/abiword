/* -*- c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1999 AbiSource, Inc.
 * Copyright (C) 2003 Tomas Frydrych <tomas@frydrych.uklinux.net> 
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


/* RTF importer by Peter Arnold <petera@intrinsica.co.uk> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <math.h>
#include <locale.h>

#include "ut_iconv.h"
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_math.h"
#include "ut_path.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_units.h"
#include "ie_types.h"
#include "ie_imp_RTF.h"
#include "pd_Document.h"
#include "xap_EncodingManager.h"
#include "ie_impGraphic.h"
#include "fg_Graphic.h"
#include "fg_GraphicRaster.h"
#include "fg_GraphicVector.h"
#include "ut_bytebuf.h"
#include "ut_rand.h"
#include "pd_Style.h"
#include "fv_View.h"
#include "fl_AutoLists.h"
#include "wv.h" // for wvLIDToLangConverter

class fl_AutoNum;

/*!
  This macros allow the use of an iconv fallback name if needed.
 */
#define CPNAME_OR_FALLBACK(destination,name,fallbackname) \
{  \
	static char* cpname = NULL; \
	if (!cpname)    \
	{       \
		UT_iconv_t cd = UT_iconv_open(name,name);     \
		if (!UT_iconv_isValid(cd)) \
		{ \
			cpname = fallbackname;\
		} \
		else \
		{ \
			cpname = name;  \
			UT_iconv_close(cd); \
		} \
	} \
	destination = cpname;  \
}


static const UT_uint32 MAX_KEYWORD_LEN = 256;
// This should probably be defined in pt_Types.h
static const UT_uint32 PT_MAX_ATTRIBUTES = 8;


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

IE_Imp_RTF_Sniffer::IE_Imp_RTF_Sniffer ()
	: IE_ImpSniffer(IE_IMPEXPNAME_RTF)
{
	// 
}

UT_Confidence_t IE_Imp_RTF_Sniffer::supportsMIME (const char * szMIME)
{
	if (UT_strcmp (IE_FileInfo::mapAlias (szMIME), IE_MIME_RTF) == 0)
		{
			return UT_CONFIDENCE_GOOD;
		}
	return UT_CONFIDENCE_ZILCH;
}

UT_Confidence_t IE_Imp_RTF_Sniffer::recognizeContents(const char * szBuf,
													  UT_uint32 iNumbytes)
{
	if ( iNumbytes < 5 )
	{
		return(UT_CONFIDENCE_ZILCH);
	}
	if ( strncmp( szBuf, "{\\rtf", 5 ) == 0 )
	{
		return(UT_CONFIDENCE_PERFECT) ;
	}
	return(UT_CONFIDENCE_ZILCH);
}

UT_Confidence_t IE_Imp_RTF_Sniffer::recognizeSuffix(const char * szSuffix)
{
	if (UT_stricmp(szSuffix,".rtf") == 0)
		return UT_CONFIDENCE_PERFECT;
	return UT_CONFIDENCE_ZILCH;
}

UT_Error IE_Imp_RTF_Sniffer::constructImporter(PD_Document * pDocument,
											   IE_Imp ** ppie)
{
	IE_Imp_RTF * p = new IE_Imp_RTF(pDocument);
	*ppie = p;
	return UT_OK;
}

bool	IE_Imp_RTF_Sniffer::getDlgLabels(const char ** pszDesc,
										 const char ** pszSuffixList,
										 IEFileType * ft)
{
	*pszDesc = "Rich Text Format (.rtf)";
	*pszSuffixList = "*.rtf";
	*ft = getFileType();
	return true;
}

//////////////////////////////////////////////////////////////////
// List class definitions
//////////////////////////////////////////////////////////////////
RTF_msword97_level::RTF_msword97_level(RTF_msword97_list * pmsword97List, UT_uint32 level )
{
	m_levelStartAt = 1;
#if 0
	m_AbiLevelID = UT_rand();
	while(m_AbiLevelID < 10000)
		m_AbiLevelID = UT_rand();
#else
	m_AbiLevelID = m_sLastAssignedLevelID++;
#endif
	m_pParaProps = NULL;
	m_pCharProps = NULL;
	m_pbParaProps = NULL;
	m_pbCharProps = NULL;
	m_pMSWord97_list = pmsword97List;
	m_localLevel = level;
	m_bStartNewList = false;
	m_listDelim = "%L";
	m_cLevelFollow = '\0';
	m_bRestart = true;
}

// Static data members must be initialized at file scope.
UT_uint32 RTF_msword97_level::m_sLastAssignedLevelID = 100000;
UT_uint32 RTF_msword97_level::m_sPreviousLevel =0;


RTF_msword97_level::~RTF_msword97_level(void)
{
	DELETEP(m_pParaProps);
	DELETEP(m_pCharProps);
	DELETEP(m_pbParaProps);
	DELETEP(m_pbCharProps);
}

void RTF_msword97_level::buildAbiListProperties( const char ** szListID,
								 const char ** szParentID,
								 const char ** szLevel,
								 const char ** szStartat,
								 const char ** szFieldFont,
								 const char ** szListDelim,
								 const char ** szListDecimal,
								 const char ** szAlign,
								 const char ** szIndent,
								 const char ** szListStyle)
{
	static UT_String buf;
	static UT_String ListID, ParentID, Level, StartAt, FieldFont, ListDelim, ListDecimal,Align,Indent;
//
// Start with List ID.
//
//
// HACKALERT - this is ugly. We need to restart the list numbering if
//   - m_bRestart is set AND
//   - the previous list inserted into the document was a higher level.
//   I use a static member variable to determine what level list was
//   entered previously.
//   TODO: this assumes that the document is not structured like
//   1. Some level one text
//      a. Some level two text
//   2. Some more level one text
//          i. Some level three text
//      b. Some more level two text - note the label was not reset!!!
//
	if (m_bRestart && (m_sPreviousLevel < m_localLevel))
	{
		m_AbiLevelID = m_sLastAssignedLevelID++;
	}
	m_sPreviousLevel = m_localLevel;
	UT_String_sprintf(buf,"%d",m_AbiLevelID);
	ListID = buf;
	*szListID = ListID.c_str();
	buf[0] = 0;
//
// Now Parent ID.
//
	UT_uint32 iParentID = 0;
	if(m_localLevel> 0 && !m_bStartNewList)
	{
		iParentID = m_pMSWord97_list->m_RTF_level[m_localLevel-1]->m_AbiLevelID;
	}
	UT_String_sprintf(buf,"%d", iParentID);
	ParentID = buf;
    *szParentID = ParentID.c_str();
	buf[0] = 0;
//
// level
//
	UT_String_sprintf(buf,"%d",m_localLevel);
    Level = buf;
    *szLevel = Level.c_str();
    buf[0] = 0;
//
// Start At.
//
	UT_String_sprintf(buf,"%d",m_levelStartAt);
    StartAt = buf;
    *szStartat = StartAt.c_str();
    buf[0] = 0;
//
// List Style
//
    List_Type abiListType;
	if(m_RTFListType == 0)
    {
        abiListType = NUMBERED_LIST;
	}
	else if( m_RTFListType == 1)
	{
        abiListType = UPPERROMAN_LIST;
	}
    else if(m_RTFListType == 2)
	{
        abiListType = LOWERROMAN_LIST;
	}
    else if( m_RTFListType == 3)
	{
        abiListType = UPPERCASE_LIST;
	}
	else if (m_RTFListType == 4)
	{
        abiListType = LOWERCASE_LIST;
	}
    else if (m_RTFListType == 5)
	{
        abiListType = UPPERCASE_LIST;
	}
    else if (m_RTFListType == 23)
	{
		*szStartat = "1";
        abiListType = BULLETED_LIST;
	}
    else if (m_RTFListType == 23 + IMPLIES_LIST)
	{
		*szStartat = "1";
        abiListType = IMPLIES_LIST;
	}
	else if (m_RTFListType == 45)
	{
		abiListType = HEBREW_LIST;
	}
	else
	{
		abiListType = NUMBERED_LIST;
	}


	fl_AutoLists al;
    *szListStyle = al.getXmlList(abiListType);
//
// Field Font
//
    FieldFont = "NULL";
    if(m_pParaProps &&  m_pParaProps->m_pszFieldFont)
	    FieldFont = m_pParaProps->m_pszFieldFont;
    if(abiListType == BULLETED_LIST)
    {
        FieldFont = "Symbol";
    }
    if(abiListType == IMPLIES_LIST)
    {
        FieldFont = "Symbol";
    }
    *szFieldFont = FieldFont.c_str();
//
// List Delim
//
    *szListDelim = m_listDelim.c_str();
//
// List Decimal
//
	*szListDecimal = ".";
//
// szAlign - left position of the paragraph
//
	if(m_pbParaProps && m_pbParaProps->bm_indentLeft)
	{
		Align = UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(m_pParaProps->m_indentLeft)/1440);
	}
	else
	{
		Align = UT_convertInchesToDimensionString(DIM_IN, (static_cast<double>(m_localLevel))*0.5);
	}
	*szAlign = Align.c_str();
//
// szIndent - offset from the left position for the listlabel
//
	if(m_pbParaProps && m_pbParaProps->bm_indentLeft)
	{
		Indent = UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(m_pParaProps->m_indentFirst)/1440);
	}
	else
	{
		Indent = "-0.3in";
	}
	*szIndent = Indent.c_str();
}

/*!
  Parse the leveltext and levelnumbers values.
  The idea is to translate the strings into AbiWord compatible format.

  From the RTF standard: "... a level three number such as '1.a.(i)' would
  generate the following RTF: '{\leveltext \'07\'00.\'01.(\'02)'}' where \'07
  is the string length, the \'00 \'01 and \'02 are level place holders, and the
  rest is surrounding text. The corresponding levelnumbers would be
  {\levelnumbers \'01\'03\'06} because the level place holders have indices
  1,3,6.

  In AbiWord, either this level resuses the parent label and adds on more text, or
  starts a new label. So we can only get 1.a.(i) if the parent label is 1.a. or 1.a

  \todo look up the parent label and be more precise about what is added by this label.
 */
bool RTF_msword97_level::ParseLevelText(const UT_String & szLevelText,const UT_String & szLevelNumbers, UT_uint32 iLevel)
{
	//read the text string into a int array, set the place holders to
	//values less than zero.
	UT_sint32 iLevelText[1000];
	char const * pText = szLevelText.c_str();
	UT_sint32 ilength = 0;
	UT_sint32 icurrent = 0;
	UT_sint32 istrlen = szLevelText.size();
	bool bIsUnicode;
	while (pText[0] != '\0')
	{
		bIsUnicode = ((pText[0] == '\\') && (pText[1] == '\'') && UT_UCS4_isdigit(pText[2]) && UT_UCS4_isdigit(pText[3]));
		// A broken exporter writes some junk at the beginning of the string.
		// Look for the first \'nn string
		if ((bIsUnicode) && (ilength == 0))
		{
			ilength = 10*(pText[2] - '0') + (pText[3] - '0');
			pText += 3;
		}
		else if (ilength >0)
		{
			// I have read the string length, I can read off the values.
			if (bIsUnicode)
			{
				iLevelText[icurrent++] = -10*(pText[2] - '0') - (pText[3] - '0');
				pText += 3;
			}
			else
			{
				iLevelText[icurrent++] = pText[0];
			}
		}
		// sanity check
		if (istrlen <= pText - szLevelText.c_str())
		{
			UT_DEBUGMSG(("RTF: parsed past the end of leveltext string %s\n",szLevelText.c_str()));
			return false;
		}
		pText++;
	}
	if(ilength != icurrent)
	{
		ilength = icurrent;
	}
	// Find the occurance of a previous level place holder
	for (icurrent = ilength-1; icurrent>=0; icurrent--)
	{
		if(-iLevelText[icurrent] >= 0 && (-iLevelText[icurrent] < static_cast<UT_sint32>(iLevel)))
		{
			break;
		}
	}
	if (icurrent < 0)
	{
		// No previous level place holder
		UT_DEBUGMSG(("RTF Import: No previous level - restart list \n"));
		m_bStartNewList = true;
	}
	else
	{
		//TODO - find the end of the previous level place holder label
	}
	// Stuff the rest of the string in JUST this level in listDelim

	m_listDelim = "";
	bool bFound = false;
	for (icurrent++; icurrent < ilength; icurrent++)
	{
		if (iLevelText[icurrent]<=0 && !bFound)
		{

#if 0
// Matti's code
			m_listDelim += "%L";
			UT_return_val_if_fail(-iLevelText[icurrent] == static_cast<UT_sint32>(iLevel), false);
#endif
			if(-iLevelText[icurrent] == static_cast<UT_sint32>(iLevel))
			{
				m_listDelim += "%L";
				bFound = true;
				continue;
			}
		}
		else if ( bFound && (iLevelText[icurrent] >= 0))
		{
			m_listDelim += static_cast<char>(iLevelText[icurrent]);
		}
		else if(bFound && iLevelText[icurrent] < 0)
		{
			break;
		}
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////////////////

RTF_msword97_list::RTF_msword97_list(IE_Imp_RTF * pie_rtf)
{
	m_RTF_listID = 0;
	m_RTF_listTemplateID = 0;
	m_pie_rtf = pie_rtf;
	for(UT_uint32 i=0; i< 9 ; i++)
	{
		m_RTF_level[i] = new RTF_msword97_level(this,i);
	}
}

RTF_msword97_list::~RTF_msword97_list(void)
{
	m_RTF_listID = 0;
	m_RTF_listTemplateID = 0;
	for(UT_uint32 i=0; i< 9 ; i++)
	{
		delete m_RTF_level[i];
	}
}

//////////////////////////////////////////////////////////////////////////////////////////

RTF_msword97_listOverride::RTF_msword97_listOverride(IE_Imp_RTF * pie_rtf )
{
	// Ideally, the default ID should be 0 which is a reserved ID in
	// the spec, but OpenOffice uses it, so use -1 instead (which
	// should be OK: spec sez 1-2000 is valid).
	m_RTF_listID = (UT_uint32)-1;

	m_OverrideCount = 0;
	m_pParaProps = NULL;
m_pParaProps = NULL;
	m_pbParaProps = NULL;
	m_pbCharProps = NULL;
	m_pie_rtf = pie_rtf;
	m_pList = NULL;
}

RTF_msword97_listOverride::~RTF_msword97_listOverride(void)
{
	DELETEP(m_pParaProps);
	DELETEP(m_pParaProps);
	DELETEP(m_pbParaProps);
	DELETEP(m_pbCharProps);
}

/*!
 * This function sets the pointer to the list structure for this override.
 * It just scans the defined lists and looks for a matching Identifier.
 */
bool RTF_msword97_listOverride::setList(void)
{
	UT_sint32 count = m_pie_rtf->get_vecWord97ListsCount();
	UT_sint32 i = 0;
	for(i=0; i< count; i++)
	{
		RTF_msword97_list * pList = m_pie_rtf->get_vecWord97NthList(i);
		if(pList->m_RTF_listID == m_RTF_listID)
		{
			m_pList = pList;
			return true;
		}
	}
	return false;
}
/*!
 * This returns returns a pointer to the tabstop vector defined in the list level.
 */
UT_Vector * RTF_msword97_listOverride::getTabStopVect(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return &(pLevel->m_pParaProps->m_tabStops);
}
/*!
 * This returns returns a pointer to the tab Type vector defined in the list level.
 */
UT_Vector * RTF_msword97_listOverride::getTabTypeVect(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return &(pLevel->m_pParaProps->m_tabTypes);
}
/*!
 * This returns returns a pointer to the tab Leadervector defined in the list level.
 */
UT_Vector * RTF_msword97_listOverride::getTabLeaderVect(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return &(pLevel->m_pParaProps->m_tabLeader);
}
/*!
 * This function returns true is there is a tab defined in the list definition.
 */
bool RTF_msword97_listOverride::isTab(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pbParaProps->bm_tabStops);
}
/*!
 * This function returns true if deleted is changed in the list definition.
 */
bool RTF_msword97_listOverride::isDeletedChanged(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pbCharProps->bm_deleted);
}
/*!
 * This function returns the Deleted state in the list definition
 */
bool RTF_msword97_listOverride::getDeleted(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pCharProps->m_deleted);
}
/*!
 * This function returns true if Bold is changed in the list definition.
 */
bool RTF_msword97_listOverride::isBoldChanged(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pbCharProps->bm_bold);
}
/*!
 * This function the bold state in the List definition.
 */
bool RTF_msword97_listOverride::getBold(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pCharProps->m_bold);
}
/*!
 * This function returns true if Italic is changed in the list definition.
 */
bool RTF_msword97_listOverride::isItalicChanged(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pbCharProps->bm_italic);
}
/*!
 * This function returns the Italic state in the list definition.
 */
bool RTF_msword97_listOverride::getItalic(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pCharProps->m_italic);
}
/*!
 * This function returns true if Underline is changed in the list definition.
 */
bool RTF_msword97_listOverride::isUnderlineChanged(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pbCharProps->bm_underline);
}
/*!
 * This function returns the Underline state in the list definition.
 */
bool RTF_msword97_listOverride::getUnderline(UT_uint32 iLevel)
{

	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pCharProps->m_underline);
}
/*!
 * This function returns true if Strikeout is changed in the list definition.
 */
bool RTF_msword97_listOverride::isStrikeoutChanged(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pbCharProps->bm_strikeout);
}
/*!
 * This function returns the Strikeout state in the list definition.
 */
bool RTF_msword97_listOverride::getStrikeout(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pCharProps->m_strikeout);
}
/*!
 * This function returns true if Superscript is changed in the list definition.
 */
bool RTF_msword97_listOverride::isSuperscriptChanged(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pbCharProps->bm_superscript);
}
/*!
 * This function returns the Superscript state in the list definition.
 */
bool RTF_msword97_listOverride::getSuperscript(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pCharProps->m_superscript);
}
/*!
 * This function returns true if Superscript Position is changed in the list
 * definition.
 */
bool RTF_msword97_listOverride::isSuperscriptPosChanged(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pbCharProps->bm_superscript_pos);
}
/*!
 * This function returns the Superscript Position in the list definition.
 */
double RTF_msword97_listOverride::getSuperscriptPos(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pCharProps->m_superscript_pos);
}
/*!
 * This function returns true if Subscript is changed in the list definition.
 */
bool RTF_msword97_listOverride::isSubscriptChanged(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pbCharProps->bm_subscript);
}
/*!
 * This function returns the Subscript state in the list definition.
 */
bool RTF_msword97_listOverride::getSubscript(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pCharProps->m_subscript);
}
/*!
 * This function returns the Subscript state in the list definition.
 */
bool RTF_msword97_listOverride::isSubscriptPosChanged(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pbCharProps->bm_subscript_pos);
}
/*!
 * This function returns the Subscript state in the list definition.
 */
double RTF_msword97_listOverride::getSubscriptPos(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pCharProps->m_subscript_pos);
}
/*!
 * This function returns true if Fontsize is changed in the list definition.
 */
bool RTF_msword97_listOverride::isFontSizeChanged(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pbCharProps->bm_fontSize);
}
/*!
 * This function returns the Fontsize in the list definition.
 */
double RTF_msword97_listOverride::getFontSize(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pCharProps->m_fontSize);
}
/*!
 * This function returns true if the Hascolor state has changed.
 */
bool RTF_msword97_listOverride::isHasColourChanged(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pbCharProps->bm_hasColour);
}
/*!
 * This function returns the Hascolor state in the list definition.
 */
bool RTF_msword97_listOverride::getHasColour(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pCharProps->m_hasColour);
}
/*!
 * This function returns true if ColourNumber is changed in the list definition.
 */
bool RTF_msword97_listOverride::isColourNumberChanged(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pbCharProps->bm_colourNumber);
}
/*!
 * This function returns the ColourNumber in the list definition.
 */
UT_uint32 RTF_msword97_listOverride::getColourNumber(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pCharProps->m_colourNumber);
}
/*!
 * This function returns true if HasBgcolour is changed in the list definition.
 */
bool RTF_msword97_listOverride::isHasBgColourChanged(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pbCharProps->bm_hasBgColour);
}
/*!
 * This function returns the HasBgcolour state in the list definition.
 */
bool RTF_msword97_listOverride::getHasBgColour(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pCharProps->m_hasBgColour);
}
/*!
 * This function returns true if BgColourNumber is changed in the list definition.
 */
bool RTF_msword97_listOverride::isBgColourNumberChanged(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pbCharProps->bm_bgcolourNumber);
}
/*!
 * This function returns the BgColourNumber  in the list definition.
 */
UT_uint32 RTF_msword97_listOverride::getBgColourNumber(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pCharProps->m_bgcolourNumber);
}
/*!
 * This function returns true if FontNumber is changed in the list definition.
 */
bool RTF_msword97_listOverride::isFontNumberChanged(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pbCharProps->bm_fontNumber);
}
/*!
 * This function returns the FontNumber if changed in the list definition.
 */
UT_uint32 RTF_msword97_listOverride::getFontNumber(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return (pLevel->m_pCharProps->m_fontNumber);
}


/*!
 * This method returns all the stuff Abi needs to reconstruct a list
 */
void RTF_msword97_listOverride::buildAbiListProperties( const char ** szListID,
								 const char ** szParentID,
								 const char ** szLevel,
								 const char ** szStartat,
								 const char ** szFieldFont,
								 const char ** szListDelim,
								 const char ** szListDecimal,
								 const char ** szAlign,
								 const char ** szIndent,
								 const char ** szListStyle,
								 UT_uint32 iLevel)
{
	m_pList->m_RTF_level[iLevel]->buildAbiListProperties(szListID, szParentID, szLevel, szStartat, szFieldFont,
									 szListDelim, szListDecimal, szAlign, szIndent, szListStyle);

}

// constructor class to tell if a character property has been defined in a list structure.
RTFProps_bCharProps::RTFProps_bCharProps(void):
	bm_deleted(false),
	bm_bold(false),
	bm_italic(false),
	bm_underline(false),
	bm_overline(false),
	bm_strikeout(false),
	bm_topline(false),
	bm_botline(false),
	bm_superscript(false),
	bm_superscript_pos(false),
	bm_subscript(false),
	bm_subscript_pos(false),
	bm_fontSize(false),
	bm_fontNumber(false),
	bm_hasColour(false),
	bm_colourNumber(false),
	bm_hasBgColour(false),
	bm_bgcolourNumber(false),
	bm_listTag(false),
	bm_RTL(false),
	bm_dirOverride(false)
{
}

RTFProps_bCharProps::~RTFProps_bCharProps(void)
{
}

// These are set true if changed in list definitions.
RTFProps_bParaProps::RTFProps_bParaProps(void):
    bm_justification(false),
	bm_spaceBefore(false),
	bm_spaceAfter(false),
	bm_indentLeft(false),
	bm_indentRight(false),
	bm_indentFirst(false),
	bm_lineSpaceVal(false),
	bm_lineSpaceExact(false),
	bm_tabStops(false),
	bm_tabTypes(false),
	bm_tabLeader(false),
	bm_curTabType(false),
	bm_curTabLeader(false),
	bm_rtfListTable(false),
	bm_dom_dir(false)
{
}

RTFProps_bParaProps::~RTFProps_bParaProps(void)
{
}

//////////////////////////////////////////////////////////////////////////
// End List definitions
//////////////////////////////////////////////////////////////////////////



// Font table items
RTFFontTableItem::RTFFontTableItem(FontFamilyEnum fontFamily, int charSet, int codepage, FontPitch pitch,
									unsigned char* panose, char* pFontName, char* pAlternativeFontName)
{
	m_family = fontFamily;
	m_charSet = charSet;
	m_codepage = codepage;
	m_szEncoding = 0;
	m_pitch = pitch;
	memcpy(m_panose, panose, 10*sizeof(unsigned char));
	m_pFontName = pFontName;
	m_pAlternativeFontName = pAlternativeFontName;

	// Set charset/codepage converter
	if (m_codepage && m_charSet)
	{
		UT_DEBUGMSG(("RTF Font has codepage *and* charset\n"));
		UT_ASSERT_NOT_REACHED();
	}
	else if (m_codepage)
	{
		// These are the valid values from the documentation:
		// TODO Many are not supported by iconv
		switch (m_codepage)
		{
			// 437	United States IBM
			// 708	Arabic (ASMO 708)
		case 708:
			m_szEncoding = "ASMO-708";	// ISO-8859-6
			break;
			// 709	Arabic (ASMO 449+, BCON V4)
			// 710	Arabic (Transparent Arabic)
			// 711	Arabic (Nafitha Enhanced)
			// 720	Arabic (Transparent ASMO)
			// 819	Windows 3.1 (United States & Western Europe)
		case 819:
			m_szEncoding = "CP819";	// ISO-8859-1
			break;
			// 850	IBM Multilingual
		case 850:
			m_szEncoding = "CP850";
			break;
			// 852	Eastern European
			// 860	Portuguese
			// 862	Hebrew
			// 863	French Canadian
			// 864	Arabic
			// 865	Norwegian
			// 866	Soviet Union
		case 866:
			m_szEncoding = "CP866";
			break;
			// 932	Japanese
		case 932:
			m_szEncoding = "CP932";
			break;
			// 936  Chinese: Simplified
		case 936:
			CPNAME_OR_FALLBACK(m_szEncoding,"CP936","GBK");
			break;
			// 950  Chinese: Traditional
		case 950:
			CPNAME_OR_FALLBACK(m_szEncoding,"CP950","BIG5");
			break;
			// 1250	Windows 3.1 (Eastern European)
		case 1250:
			m_szEncoding = "CP1250";	// MS-EE
			break;
			// 1251	Windows 3.1 (Soviet Union)
		case 1251:
			m_szEncoding = "CP1251";	// MS-CYRL
			break;

			// These were produced by MS WordPad 5.0 on Win2K
			// TODO What do we do with negative values?
			// -8534 - Devanagari	(57002)
			// -8533 - Bengali		(57003)
			// -8532 - Tamil		(57004)
			// -8531 - Telugu		(57005)
			// -8530 - Assamese		(57006)
			// -8529 - Oriya		(57007)
			// -8528 - Kannada		(57008)
			// -8527 - Malayalam	(57009)
			// -8526 - Gujarathi	(57010)
			// -8525 - Panjabi		(57011)
			// -7536 - Georgian		(58000)
			// -7535 - Armenian		(58001)
		default:
			m_szEncoding = XAP_EncodingManager::get_instance()->charsetFromCodepage(m_codepage);
		}
	}
	else if (m_charSet)
	{
		switch (m_charSet)
		{
			case 0:		// ANSI_CHARSET
				m_szEncoding = "CP1252";	// MS-ANSI
				break;
			case 2:		// SYMBOL_CHARSET
				UT_DEBUGMSG(("RTF Font charset 'Symbol' not implemented\n"));
				break;
			case 77:    // Source Vlad Harchev from OpenOffice
				m_szEncoding = "MACINTOSH";
				break;
			case 128:	// SHIFTJIS_CHARSET
				m_szEncoding = "CP932";
				break;
			case 129:	// Hangul - undocumented?
				m_szEncoding = "CP949";
				break;
			case 130:   // Source Vlad Harchev from OpenOffice
				m_szEncoding = "CP1361";
				break;
			case 134:	// Chinese GB - undocumented?
				CPNAME_OR_FALLBACK(m_szEncoding,"CP936","GBK");
				break;
			case 136:	// Chinese BIG5 - undocumented?
				CPNAME_OR_FALLBACK(m_szEncoding,"CP950","BIG5");
				break;
			case 161:	// GREEK_CHARSET
				m_szEncoding = "CP1253";	// MS-GREEK
				break;
			case 162:	// TURKISH_CHARSET
				m_szEncoding = "CP1254";	// MS-TURK
				break;
			case 163:	// Vietnamese - undocumented?
				m_szEncoding = "CP1258";
				break;
			// TODO What is different?  Iconv only supports one MS Hebrew codepage.
			case 181:	// HEBREWUSER_CHARSET
				UT_DEBUGMSG(("RTF Font charset 'HEBREWUSER'??\n"));
			case 177:	// HEBREW_CHARSET
				m_szEncoding = "CP1255";	// MS-HEBR
				break;
			// TODO What is different?  Iconv only supports one MS Arabic codepage.
			case 178:	// ARABICSIMPLIFIED_CHARSET
				UT_DEBUGMSG(("RTF Font charset 'ARABICSIMPLIFIED'??\n"));
				m_szEncoding = "CP1256";	// MS-ARAB
				break;
			case 179:	// ARABICTRADITIONAL_CHARSET
				UT_DEBUGMSG(("RTF Font charset 'ARABICTRADITIONAL'??\n"));
				m_szEncoding = "CP1256";	// MS-ARAB
				break;
			case 180:	// ARABICUSER_CHARSET
				UT_DEBUGMSG(("RTF Font charset 'ARABICUSER'??\n"));
				m_szEncoding = "CP1256";	// MS-ARAB
				break;
			case 186:	// Baltic - undocumented?
				m_szEncoding = "CP1257";
				break;
			case 204:	// CYRILLIC_CHARSET
				m_szEncoding = "CP1251";	// MS-CYRL
				break;
			case 222:	// Thai - undocumented?
				m_szEncoding = "CP874";
				break;
			case 238:	// EASTERNEUROPE_CHARSET
				m_szEncoding = "CP1250";	// MS-EE
				break;
			case 254:	// PC437_CHARSET
				// TODO What is this and can iconv do it?
				// TODO It seems to be "OEM United States" "IBM437"
				// TODO Maybe same as code page 1252
				UT_DEBUGMSG(("RTF Font charset 'PC437'??\n"));
				UT_ASSERT(UT_NOT_IMPLEMENTED);
				break;
			case 255:	// OEM_CHARSET
				// TODO Can iconv do this?
				UT_DEBUGMSG(("RTF Font charset 'OEM'??\n"));
				UT_ASSERT(UT_NOT_IMPLEMENTED);
				break;
			default:
				UT_DEBUGMSG(("RTF Font charset unknown: %d\n", m_charSet));
				// don't assert like we used to do. just silently ignore.
		}
	}
}

RTFFontTableItem::~RTFFontTableItem()
{
	free(m_pFontName);
	free(m_pAlternativeFontName);
}

// Character properties
RTFProps_CharProps::RTFProps_CharProps(void)
{
	m_deleted = false;
	m_bold = false;
	m_italic = false;
	m_underline = false;
	m_overline = false;
	m_topline = false;
	m_botline = false;
	m_strikeout = false;
	m_superscript = false;
	m_superscript_pos = 0.0;
	m_subscript = false;
	m_subscript_pos = 0.0;
	m_fontSize = 12.0;
	m_fontNumber = 0;
	m_hasColour = false;
	m_colourNumber = 0;
	m_hasBgColour = false;
	m_bgcolourNumber = 0;
	m_styleNumber = -1;
	m_listTag = 0;
	m_szLang = 0;
	m_RTL = false;
	m_dirOverride = FRIBIDI_TYPE_UNSET;
}

RTFProps_CharProps::~RTFProps_CharProps(void)
{
}

// Paragraph properties
RTFProps_ParaProps::RTFProps_ParaProps(void)
{
	m_justification = pjLeft;
	m_spaceBefore = 0;
	m_spaceAfter = 0;
	m_indentLeft = 0;
	m_indentRight = 0;
	m_indentFirst = 0;
	m_lineSpaceExact = false;
	m_lineSpaceVal = 240;
	m_isList = false;
	m_level = 0;
	memset(&m_pszStyle, 0, sizeof(m_pszStyle));
	m_rawID = 0;
	m_rawParentID = 0;
	memset(m_pszListDecimal,0,sizeof(m_pszListDecimal)) ;
	memset(m_pszListDelim,0,sizeof(m_pszListDelim)) ;
	memset(m_pszFieldFont,0,sizeof(m_pszFieldFont)) ;
	m_startValue = 0;
	m_curTabType = FL_TAB_LEFT;
	m_curTabLeader = FL_LEADER_NONE;
	m_iOverride = 0;
	m_iOverrideLevel = 0;
	m_styleNumber = -1;
	m_RTL = false;
	m_tableLevel = 1; // Has to be 1 because the RTF spec has itap defaulting 
	                  // to this value
}

RTFProps_ParaProps& RTFProps_ParaProps::operator=(const RTFProps_ParaProps& other)
{
	if (this != &other)
	{
		m_tabStops.clear();
		m_tabTypes.clear();
		m_tabLeader.clear();
		m_justification = other.m_justification;
		m_spaceBefore = other.m_spaceBefore;
		m_spaceAfter = other.m_spaceAfter;
		m_indentLeft = other.m_indentLeft;
		m_indentRight = other.m_indentRight;
		m_indentFirst = other.m_indentFirst;
		m_lineSpaceVal = other.m_lineSpaceVal;
		m_lineSpaceExact = other.m_lineSpaceExact;

		if (other.m_tabStops.getItemCount() > 0)
		{
			for (UT_uint32 i = 0; i < other.m_tabStops.getItemCount(); i++)
			{
				m_tabStops.addItem(other.m_tabStops.getNthItem(i));
			}
		}
		if (other.m_tabTypes.getItemCount() > 0)
		{
			for (UT_uint32 i = 0; i < other.m_tabTypes.getItemCount(); i++)
			{
				m_tabTypes.addItem(other.m_tabTypes.getNthItem(i));
			}
		}
		if (other.m_tabLeader.getItemCount() > 0)
		{
			for (UT_uint32 i = 0; i < other.m_tabLeader.getItemCount(); i++)
			{
				m_tabLeader.addItem(other.m_tabLeader.getNthItem(i));
			}
		}
		UT_ASSERT(m_tabStops.getItemCount() ==
					other.m_tabTypes.getItemCount() );
		UT_ASSERT(m_tabStops.getItemCount() ==
					other.m_tabLeader.getItemCount() );

		m_isList = other.m_isList;
		m_level = other.m_level;
		strcpy(static_cast<char *>(m_pszStyle), static_cast<const char *>(other.m_pszStyle));
		m_rawID = other.m_rawID;
		m_rawParentID = other.m_rawParentID;
		strcpy(static_cast<char *>(m_pszListDecimal), static_cast<const char *>(other.m_pszListDecimal));
		strcpy(static_cast<char *>(m_pszListDelim), static_cast<const char *>(other.m_pszListDelim));
		strcpy(static_cast<char *>(m_pszFieldFont), static_cast<const char *>(other.m_pszFieldFont));
		m_startValue = other.m_startValue;
		m_iOverride = other.m_iOverride;
		m_iOverrideLevel = other.m_iOverrideLevel;
		if(m_tabTypes.getItemCount() > 0)
		{
			UT_uint32 dum = reinterpret_cast<UT_uint32>(m_tabTypes.getNthItem(0));
			m_curTabType = static_cast<eTabType>(dum);
			dum = reinterpret_cast<UT_uint32>(m_tabLeader.getNthItem(0));
			m_curTabLeader = static_cast<eTabLeader>(dum);
		}
		else
		{
			m_curTabType = FL_TAB_LEFT;
			m_curTabLeader = FL_LEADER_NONE;
		}
		m_rtfListTable = other.m_rtfListTable;
		m_styleNumber = other.m_styleNumber;
	}

	m_RTL = other.m_RTL;
	m_tableLevel = other.m_tableLevel;
	return *this;
}

RTFProps_ImageProps::RTFProps_ImageProps()
{
	sizeType = ipstNone;
	wGoal = hGoal = width = height = 0;
	scaleX = scaleY = 100;
}

RTFProps_CellProps::RTFProps_CellProps()
{
	m_bVerticalMerged = false;
	m_bVerticalMergedFirst = false;
	m_iCellx = 0;
}

RTFProps_CellProps& RTFProps_CellProps::operator=(const RTFProps_CellProps& other)
{
	if (this != &other)
	{
		 m_bVerticalMerged = other.m_bVerticalMerged;
		 m_bVerticalMergedFirst = other. m_bVerticalMergedFirst;
		 m_iCellx = other.m_iCellx;
	}
	return *this;
}

RTFProps_TableProps::RTFProps_TableProps()
{
	m_bAutoFit = false;
}

RTFProps_TableProps& RTFProps_TableProps::operator=(const RTFProps_TableProps& other)
{
	if (this != &other)
	{
		m_bAutoFit = other.m_bAutoFit;
	}
	return *this;
}

RTFProps_SectionProps::RTFProps_SectionProps()
{
	m_numCols = 1;
	m_breakType = sbkPage;    /* the default in RTF is page section break */
	m_pageNumFormat = pgDecimal;
	m_bColumnLine = false;
	m_leftMargTwips = 1800;
	m_rightMargTwips = 1800;
	m_topMargTwips = 1440;
	m_bottomMargTwips = 1440;
	m_headerYTwips = 0;
	m_footerYTwips = 0;
    m_colSpaceTwips = 0;
	m_dir = FRIBIDI_TYPE_UNSET;
}

RTFStateStore::RTFStateStore()
{
	m_destinationState = rdsNorm;
	m_internalState = risNorm;
	m_unicodeAlternateSkipCount = 1;
	m_unicodeInAlternate = 0;
}


/*****************************************************************/
/*****************************************************************/

IE_Imp_RTF::IE_Imp_RTF(PD_Document * pDocument)
:	IE_Imp(pDocument),
	m_gbBlock(1024),
	m_groupCount(0),
	m_newParaFlagged(false),
	m_newSectionFlagged(false),
	m_cbBin(0),
	m_pImportFile(NULL),
	deflangid(0),
	m_mbtowc (XAP_EncodingManager::get_instance()->getNative8BitEncodingName()),
	m_TableControl(pDocument),
	m_lastCellSDH(NULL),
	m_bNestTableProps(false),
	m_bParaWrittenForSection(false),
	m_bCellBlank(true),
	m_bEndTableOpen(false),
	m_iHyperlinkOpen(0),
	m_bBidiDocument(false),
	m_pPasteBuffer(NULL),
	m_lenPasteBuffer(0),
	m_pCurrentCharInPasteBuffer(NULL),
	m_numLists(0),
	m_currentHdrID(0),
	m_currentFtrID(0),
	m_currentHdrEvenID(0),
	m_currentFtrEvenID(0),
	m_currentHdrFirstID(0),
	m_currentFtrFirstID(0),
	m_currentHdrLastID(0),
	m_currentFtrLastID(0),
	m_parsingHdrFtr(false),
	m_icurOverride(0),
	m_icurOverrideLevel(0),
	m_szFileDirName(NULL),
	m_bAppendAnyway(false),
	m_bInFootnote(false),
	m_iDepthAtFootnote(0)
{
	if(m_vecAbiListTable.getItemCount() != 0)
	{
		UT_VECTOR_PURGEALL(_rtfAbiListTable *,m_vecAbiListTable);
	}
	m_mbtowc.setInCharset(XAP_EncodingManager::get_instance()->getNativeEncodingName());
	m_hyperlinkBase.clear();
}


IE_Imp_RTF::~IE_Imp_RTF()
{
	// Empty the state stack
	while (m_stateStack.getDepth() > 0)
	{
		RTFStateStore* pItem = NULL;
		m_stateStack.pop((void**)(&pItem));
		delete pItem;
	}

	// and the font table (can't use the macro as we allow NULLs in the vector
	UT_sint32 size = m_fontTable.getItemCount();
	UT_sint32 i =0;
	for (i = size-1; i>=0; i--)
	{
		RTFFontTableItem* pItem = static_cast<RTFFontTableItem*>(m_fontTable.getNthItem(i));
		delete pItem;
	}

	// and the styleName table.

	size = m_styleTable.getItemCount();
	for (i = 0; i < size; i++)
	{
		char * pItem = static_cast<char *>(m_styleTable.getNthItem(i));
		FREEP(pItem);
	}
	UT_VECTOR_PURGEALL(_rtfAbiListTable *,m_vecAbiListTable);
	UT_VECTOR_PURGEALL(RTFHdrFtr *, m_hdrFtrTable);
	UT_VECTOR_PURGEALL(RTF_msword97_list *, m_vecWord97Lists);
	UT_VECTOR_PURGEALL(RTF_msword97_listOverride *, m_vecWord97ListOverride);
	UT_DEBUGMSG(("SEVIOR:DOing last close \n"));
	while(getTable() && getTable()->wasTableUsed())
	{
		CloseTable();
	}
	FREEP (m_szFileDirName);
}

UT_Error IE_Imp_RTF::importFile(const char * szFilename)
{
	m_newParaFlagged = true;
	m_newSectionFlagged = true;

	m_szFileDirName = UT_strdup (szFilename);
	// UT_basename returns a point INSIDE the passed string.
	// the trick is to truncate the string by setting the char pointed
	// by tmp to NULL. This IS useful code. (2 LOC)
	char * tmp = const_cast<char *>(UT_basename (m_szFileDirName));
	*tmp = 0;
	FILE *fp = fopen(szFilename, "r");
	if (!fp)
	{
		UT_DEBUGMSG(("Could not open file %s\n",szFilename));
		return UT_errnoToUTError ();
	}

	UT_Error error = _writeHeader(fp);

	if (!error)
	{
		error = _parseFile(fp);
		m_bAppendAnyway = true;
		_appendHdrFtr ();
	}

	fclose(fp);

	return error;
}


UT_Error IE_Imp_RTF::_writeHeader(FILE * /*fp*/)
{
		return UT_OK;
}


static bool digVal(char ch, int& value, int base)
{
	value = ch - '0';

	return (value >= 0) && (value < base);
}

static bool hexVal(char c, int& value)
{
	bool ok = true;

	if (isdigit(c))
	{
		ok = digVal(c, value, 10);
	}
	else if (islower(c))
	{
		ok = (c >= 'a' && c <= 'f');
		value = c - 'a' + 10;
	}
	else
	{
		ok = (c >= 'A' && c <= 'F');
		value = static_cast<char>(c) - 'A' + 10;
	}

	return ok;
}

ie_imp_cell * IE_Imp_RTF::getCell(void)
{
	UT_return_val_if_fail(getTable(),NULL);
	return getTable()->getCurCell();
}

ie_imp_table * IE_Imp_RTF::getTable(void)
{
	return m_TableControl.getTable();
}

void IE_Imp_RTF::OpenTable(void)
{
	if(!m_pImportFile)
	{
		return;
	}
	if(!m_bParaWrittenForSection)
	{
		FlushStoredChars(true);
	}
	else
	{
		FlushStoredChars();
	}
	m_TableControl.OpenTable();
	getDoc()->appendStrux(PTX_SectionTable,NULL);
	UT_DEBUGMSG(("SEVIOR: Appending Table strux to doc nestdepth %d \n", m_TableControl.getNestDepth()));
	PL_StruxDocHandle sdh = getDoc()->getLastStruxOfType(PTX_SectionTable);
	UT_DEBUGMSG(("SEVIOR: Table strux sdh is %x \n",sdh));
	getTable()->setTableSDH(sdh);
	getTable()->OpenCell();
	getDoc()->appendStrux(PTX_SectionCell,NULL);
	sdh = getDoc()->getLastStruxOfType(PTX_SectionCell);
	getCell()->setCellSDH(sdh);
	m_currentRTFState.m_cellProps = RTFProps_CellProps();
	m_currentRTFState.m_tableProps = RTFProps_TableProps();
	m_lastCellSDH = NULL; // This is in the table structure and can be deleted from there.
	m_bCellBlank = true;
}


void IE_Imp_RTF::CloseTable(void)
{
//
// Close table removes extraneous struxes like unmatched PTX_SectionCell's
//
	if(!m_pImportFile && (getTable() == NULL))
	{
		return;
	}
	if(getTable() && getTable()->wasTableUsed())
	{
		UT_DEBUGMSG(("SEVIOR: Table used appened end Table, block \n"));
		if(m_lastCellSDH != NULL )
		{
			getDoc()->insertStruxNoUpdateBefore(m_lastCellSDH,PTX_EndTable,NULL);
			getDoc()->deleteStruxNoUpdate(m_lastCellSDH);
			if(m_bCellBlank)
			{
				m_bEndTableOpen = true;
			}
		}
		m_TableControl.CloseTable();
		if(m_lastCellSDH == NULL)
		{
			getDoc()->appendStrux(PTX_EndTable,NULL);
			m_bEndTableOpen = true;
		}
		m_lastCellSDH = NULL;
	}
	else if(getTable())
	{
		if(m_lastCellSDH != NULL )
		{
			getDoc()->deleteStruxNoUpdate(m_lastCellSDH);
			m_lastCellSDH = NULL;
		}
		m_TableControl.CloseTable();
		UT_DEBUGMSG(("SEVIOR: Table not used. \n"));
	}
	else
	{
		if(m_lastCellSDH != NULL )
		{
			getDoc()->deleteStruxNoUpdate(m_lastCellSDH);
			m_lastCellSDH = NULL;
		}
	}
}

void IE_Imp_RTF::HandleCell(void)
{
	if(!m_pImportFile)
	{
		return;
	}
	if(m_bCellBlank && (m_gbBlock.getLength() == 0))
	{
		getDoc()->appendStrux(PTX_Block,NULL);
	}
	else
	{
//
// Flush out anything we've been holding
//	
		FlushStoredChars();
	}
	if(getTable() == NULL)
	{
		OpenTable();
	}
	PL_StruxDocHandle sdh = getDoc()->getLastStruxOfType(PTX_SectionCell);
	ie_imp_cell * pCell = getTable()->getNthCellOnRow(getTable()->getPosOnRow());
	UT_ASSERT(sdh);
	if(!pCell)
	{
//
// Cell class doesn't exist so create it.
//
		UT_sint32 pos  = getTable()->OpenCell();
		getTable()->setPosOnRow(pos);
		UT_DEBUGMSG(("SEVIOR: created cell %x for posOnRow %d \n",getCell(),getTable()->getPosOnRow()));

	}
	UT_DEBUGMSG(("SEVIOR: set cell sdh %x  at pos %d on row %d \n",sdh,getTable()->getPosOnRow(),getTable()->getRow()));
	getTable()->setNthCellOnThisRow(getTable()->getPosOnRow());
	if(!getCell()->isMergedAbove())
	{
		getCell()->setCellSDH(sdh);
		UT_DEBUGMSG(("SEVIOR: At posOnRow %d cellx %d \n",getTable()->getPosOnRow(),getCell()->getCellX()));
		getTable()->incPosOnRow();
		FlushStoredChars();		
		UT_DEBUGMSG(("SEVIOR: Non posonrow %d \n",getTable()->getPosOnRow()));
		getDoc()->appendStrux(PTX_EndCell,NULL);
//
// Look to see if this is just has a cell/endCell with no content. If so
// repair it.
//
		PL_StruxDocHandle sdhEndCell = reinterpret_cast<PL_StruxDocHandle>(getDoc()->getLastStruxOfType(PTX_EndCell));
		if(getDoc()->isStruxBeforeThis(sdhEndCell,PTX_SectionCell))
		{
			getDoc()->insertStruxNoUpdateBefore(sdhEndCell,PTX_Block,NULL);
		}
		getTable()->CloseCell();
		getDoc()->appendStrux(PTX_SectionCell,NULL);
		m_lastCellSDH = getDoc()->getLastStruxOfType(PTX_SectionCell);
	}
	else
	{
		getTable()->incPosOnRow();
	}
	m_bCellBlank = true;
}

void IE_Imp_RTF::FlushCellProps(void)
{
	if(!m_pImportFile)
	{
		return;
	}

	getCell()->setMergeAbove( m_currentRTFState.m_cellProps.m_bVerticalMerged );
	getCell()->setFirstVerticalMerge( m_currentRTFState.m_cellProps.m_bVerticalMergedFirst );
}


void IE_Imp_RTF::FlushTableProps(void)
{
	if(!m_pImportFile)
	{
		return;
	}
	getTable()->setAutoFit( m_currentRTFState.m_tableProps.m_bAutoFit );
}

void IE_Imp_RTF::HandleCellX(UT_sint32 cellx)
{
	if(!m_pImportFile)
	{
		return;
	}

	if(getTable() == NULL)
	{
		OpenTable();
	}
	UT_sint32 iRow = 0;
	bool bNewCell = true;
//
// Look to see if a cell with cellx already exists on the current row. If so set the
// current cell pointer to point to it.
//

	iRow = getTable()->getRow();
	ie_imp_cell * pOldCell = getTable()->getCellAtRowColX(iRow,cellx);
	if(pOldCell)
	{
		bNewCell = false;
		getTable()->setCell(pOldCell);
	}
	if(!pOldCell)
	{
		pOldCell = getTable()->getNthCellOnRow(getTable()->getCellXOnRow());
		UT_DEBUGMSG(("SEVIOR: Looking for cellx num %d on row %d found %x \n",getTable()->getCellXOnRow(),iRow,pOldCell));
		if(pOldCell)
		{
			bNewCell = false;
			getTable()->setCell(pOldCell);
		}
	}
	if(bNewCell)
	{
		getTable()->OpenCell();
		UT_DEBUGMSG(("SEVIOR: created cell %x for cellx %d on row \n",getCell(),cellx,getTable()->getRow()));
	}
	getTable()->setCellX(cellx);
	UT_DEBUGMSG(("set cellx for class %x to %d \n",getCell(),cellx));
	FlushCellProps();
	ResetCellAttributes();
	getTable()->incCellXOnRow();
}

void IE_Imp_RTF::HandleRow(void)
{
	if(!m_pImportFile)
	{
		return;
	}

	UT_DEBUGMSG(("ie_imp_RTF: Handle Row now \n"));
	m_TableControl.NewRow();
}
		
void IE_Imp_RTF::HandleFootnote(void)
{
	m_bInFootnote = true;
	m_iDepthAtFootnote = m_stateStack.getDepth();
	const XML_Char * attribs[3] ={"footnote-id",NULL,NULL};
	UT_String footpid;
	UT_String_sprintf(footpid,"%i",m_iLastFootnoteId);
	attribs[1] = footpid.c_str();
	UT_DEBUGMSG(("ie_imp_RTF: Handle Footnote now \n"));

	if(m_pImportFile)
	{
		getDoc()->appendStrux(PTX_SectionFootnote,attribs);
		getDoc()->appendStrux(PTX_Block,NULL);
	}
	else
	{
		getDoc()->insertStrux(m_dposPaste,PTX_SectionFootnote,attribs,NULL);
		m_dposPaste++;
		getDoc()->insertStrux(m_dposPaste,PTX_Block);
		m_dposPaste++;
	}
}

UT_Error IE_Imp_RTF::_parseText()
{
	bool ok = true;
    int cNibble = 2;
	int b = 0;
	unsigned char c;

	// remember the depth of stack on entry, and if the depth of stack
	// drops bellow this level return (this is so that we can call
	// this method recursively)
	UT_uint32 iRTFStackDepth = m_stateStack.getDepth();
	UT_DEBUGMSG(("IE_Imp_RTF::_parseText: stack depth %d\n", iRTFStackDepth));
	
	
	while (ok  &&  m_stateStack.getDepth() >= iRTFStackDepth && ReadCharFromFile(&c))
	{
		if (m_currentRTFState.m_internalState == RTFStateStore::risBin)
		{
			// if we're parsing binary data, handle it directly
			ok = ParseChar(c);
		}
		else
		{
			switch (c)
			{
			case '{':
				ok = PushRTFState();
				if (!ok) {
					UT_DEBUGMSG(("PushRTFState()\n"));
				}
				break;
			case '}':
				ok = PopRTFState();
				if (!ok) {
					UT_DEBUGMSG(("PopRTFState()\n"));
				}
				break;
			case '\\':
				ok = ParseRTFKeyword();
				if (!ok) {
					UT_DEBUGMSG(("ParseRTFKeyword()\n"));
				}
				break;
			default:
				if (m_currentRTFState.m_internalState == RTFStateStore::risNorm)
				{
					ok = ParseChar(c);
					if (!ok) {
						UT_DEBUGMSG(("ParseChar()\n"));
					}
				}
				else
				{
					UT_return_val_if_fail(m_currentRTFState.m_internalState == RTFStateStore::risHex, UT_ERROR);

					b = b << 4;
					int digit;

					// hexval calls digval
					ok = hexVal(static_cast<char>(c), digit);
					b += digit;

					cNibble--;
					if (!cNibble  &&  ok)
					{
						ok = ParseChar(b,0);
						if (!ok) {
							UT_DEBUGMSG(("ParseChar()\n"));
						}
						cNibble = 2;
						b = 0;
						m_currentRTFState.m_internalState = RTFStateStore::risNorm;
						// actually don't handle the following space since
						// this is NOT a delimiter
						// see bug #886
					}
				}
			}
		}
	}

	if (ok)
	{
		ok = FlushStoredChars(true);
		if (!ok) {
			UT_DEBUGMSG(("FlushStoredChars()\n"));
		}
	}
	return ok ? UT_OK : UT_ERROR;

}

/*
   Scans the entire document for any rtl tokens and set m_bBidiDocument
   accordingly. Please note that this results in a minimal performance
   loss (a fraction of a second on a 30 page doc), and saves us much
   work and time if the document is LTR-only.
*/
UT_Error IE_Imp_RTF::_isBidiDocument()
{
	UT_return_val_if_fail(m_pImportFile, UT_ERROR);

	char buff[8192 + 1];
	char * token = NULL;
	
	size_t iBytes = fread((void*)&buff[0], 1, sizeof(buff) - 1, m_pImportFile);

	UT_DEBUGMSG(("IE_Imp_RTF::_isBidiDocument: looking for RTL tokens\n"));
	while (iBytes)
	{
		buff[iBytes] = 0;
		token = strstr(buff, "rtlsect");
		if(token)
		{
			break;
		}
		
		token = strstr(buff, "rtlpar");
		if(token)
			break;

		token = strstr(buff, "rtlch");
		if(token)
			break;

		iBytes = fread((void*)&buff[0], 1, sizeof(buff) - 1, m_pImportFile);	
	}

	if(token)
	{
		UT_DEBUGMSG(("IE_Imp_RTF::_isBidiDocument: found rtl token [%s]\n", token));
		m_bBidiDocument = true;
	}
	else
	{
		UT_DEBUGMSG(("IE_Imp_RTF::_isBidiDocument: no rtl token found\n"));
		m_bBidiDocument = false;
	}
	
	
	// reset the file pointer to the beginning
	if(fseek(m_pImportFile, 0, SEEK_SET))
		return UT_ERROR;

	UT_DEBUGMSG(("IE_Imp_RTF::_isBidiDocument: looking for RTL tokens -- done\n"));
	return UT_OK;
}



UT_Error IE_Imp_RTF::_parseFile(FILE* fp)
{
	m_pImportFile = fp;

	m_currentRTFState.m_internalState = RTFStateStore::risNorm;
	m_currentRTFState.m_destinationState = RTFStateStore::rdsNorm;
	m_currentHdrID = 0;
	m_currentFtrID = 0;
	m_currentHdrEvenID = 0;
	m_currentFtrEvenID = 0;
	m_currentHdrFirstID = 0;
	m_currentFtrFirstID = 0;
	m_currentHdrLastID = 0;
	m_currentFtrLastID = 0;
#if 0
	if(m_pImportFile && UT_OK != _isBidiDocument())
		return UT_ERROR;
#endif
	return _parseText();
}

/*****************************************************************/
/*****************************************************************/


// flush any remaining text in the previous para and flag
// a new para to be started.  Don't actually start a new
// para as it may turn out to be empty
//
bool IE_Imp_RTF::StartNewPara()
{

	bool ok = FlushStoredChars(true);
	m_newParaFlagged = true;

	// need to reset any left-over direction override
	m_currentRTFState.m_charProps.m_dirOverride = FRIBIDI_TYPE_UNSET;
	return ok;
}


// flush any remaining text in the previous sction and
// flag a new section to be started.  Don't actually
// start a new section as it may turn out to be empty
//
bool IE_Imp_RTF::StartNewSection()
{
	bool ok = FlushStoredChars(true);

	m_newParaFlagged = true;
	m_newSectionFlagged = true;

	return ok;
}


// add a new character.  Characters are actually cached and
// inserted into the document in batches - see FlushStoredChars
//
bool IE_Imp_RTF::AddChar(UT_UCSChar ch)
{
	if(!m_gbBlock.ins(m_gbBlock.getLength(), reinterpret_cast<UT_GrowBufElement*>(&ch), 1))
		return false;

	return true;
}


// flush any stored text into the document
//
bool IE_Imp_RTF::FlushStoredChars(bool forceInsertPara)
{

	// start a new para if we have to
	bool ok = true;
	if (m_newSectionFlagged && (forceInsertPara || (m_gbBlock.getLength() > 0)) )
	{
		ok = ApplySectionAttributes();
		m_newSectionFlagged = false;
	}
	if (ok  && m_newParaFlagged  &&  (forceInsertPara  ||  (m_gbBlock.getLength() > 0)) )
	{
		bool bSave = m_newParaFlagged;
		m_newParaFlagged = false;
		ok = ApplyParagraphAttributes();
		if(m_gbBlock.getLength() == 0)
		{
//
// This forces empty lines to have the same height as the previous line
//
			m_newParaFlagged = bSave;
			if(m_pImportFile != NULL)
			{
				getDoc()->appendFmtMark();
			}
		}
		m_newParaFlagged = false;
	}

	if (ok  &&  (m_gbBlock.getLength() > 0))
	{
		if(ok && m_bCellBlank && (getTable() != NULL))
		{
			getDoc()->appendStrux(PTX_Block,NULL);
			m_bCellBlank = false;
			m_bEndTableOpen = false;
		}
		else if( ok && m_bEndTableOpen)
		{
			getDoc()->appendStrux(PTX_Block,NULL);
			m_bEndTableOpen = false;
		}
		ok = ApplyCharacterAttributes();
		m_bCellBlank = false;
	}
	if( ok && m_bInFootnote && m_stateStack.getDepth() <m_iDepthAtFootnote)
	{
		if(m_pImportFile)
		{
			getDoc()->appendStrux(PTX_EndFootnote,NULL);
		}
		else
		{
			ok = getDoc()->insertStrux(m_dposPaste,PTX_EndFootnote);
			m_dposPaste++;
		}
		m_bInFootnote = false;
		m_iDepthAtFootnote = 0;
	}
	return ok;
}


// Get a font out of the font table, making sure we dont attempt to access off the end
RTFFontTableItem* IE_Imp_RTF::GetNthTableFont(UT_uint32 fontNum)
{
	if (fontNum < m_fontTable.getItemCount())
	{
		return static_cast<RTFFontTableItem*>(m_fontTable.getNthItem(fontNum));
	}
	else
	{
		return NULL;
	}
}


// Get a colour out of the colour table, making sure we dont attempt to access off the end
UT_uint32 IE_Imp_RTF::GetNthTableColour(UT_uint32 colNum)
{
	if (colNum < m_colourTable.getItemCount())
	{
		return reinterpret_cast<UT_uint32>(m_colourTable.getNthItem(colNum));
	}
	else
	{
		return 0;	// black
	}
}

UT_sint32 IE_Imp_RTF::GetNthTableBgColour(UT_uint32 colNum)
{
	if (colNum < m_colourTable.getItemCount())
	{
		return reinterpret_cast<UT_sint32>(m_colourTable.getNthItem(colNum));
	}
	else
	{
		return -1;	// invalid
	}
}

// Pushes the current state RTF state onto the state stack
//
bool IE_Imp_RTF::PushRTFState(void)
{
	// Create a new object to store the state in
	RTFStateStore* pState = new RTFStateStore;
	if (pState == NULL)
	{
	    UT_DEBUGMSG (("PushRTFState(): no state\n"));
	    return false;
	}
	*pState = m_currentRTFState;
	m_stateStack.push(pState);

	// Reset the current state
	m_currentRTFState.m_internalState = RTFStateStore::risNorm;
	return true;
}



// Pops the state off the top of the RTF stack and set the current state to it.
//
bool IE_Imp_RTF::PopRTFState(void)
{
	RTFStateStore* pState = NULL;
	m_stateStack.pop(reinterpret_cast<void**>(&pState));

	if (pState != NULL)
	{
		bool ok = FlushStoredChars();
		m_currentRTFState = *pState;
		delete pState;

		m_currentRTFState.m_unicodeInAlternate = 0;
		return ok;
	}
	else
	{
		UT_return_val_if_fail(pState != NULL, false);	// state stack should not be empty
		return true; // was false
	}
}


// Process a single character from the RTF stream
//
bool IE_Imp_RTF::ParseChar(UT_UCSChar ch,bool no_convert)
{
    // Have we reached the end of the binary skip?
	if (m_currentRTFState.m_internalState == RTFStateStore::risBin  && --m_cbBin <= 0)
	{
		m_currentRTFState.m_internalState = RTFStateStore::risNorm;
	}

	switch (m_currentRTFState.m_destinationState)
	{
		case RTFStateStore::rdsSkip:
			// Toss this character.
			return true;
		case RTFStateStore::rdsNorm:
			if (m_currentRTFState.m_unicodeInAlternate > 0)
			{
				m_currentRTFState.m_unicodeInAlternate--;
				return true;
			}
			// Insert a character into the story
            if ((ch >= 32  ||  ch == 9 || ch == UCS_FF || ch == UCS_LF || ch == UCS_VTAB)  &&  !m_currentRTFState.m_charProps.m_deleted)
			{
				if (no_convert==0 && ch<=0xff)
				{
					UT_UCS4Char wc;
					// TODO Doesn't handle multibyte encodings (CJK)
					if (m_mbtowc.mbtowc(wc,static_cast<UT_Byte>(ch)))
						return AddChar(wc);
				} else
					return AddChar(ch);
			}
		default:
			// handle other destinations....
			return true;
	}
	UT_DEBUGMSG (("went thru all ParseChar() withou doing anything\n"));
	return true;
}


/*!
  Reads and proccesses a RTF control word and its parameter
  \return false if something goes wrong.
  \desc Read and handle the RTF keyword. Commands are dispatched by calling
  TranslateKeyword
  \fixme This is too generic: keywords are most of the time contextual
  so context should be taken care of.
  \see IE_Imp_RTF::ReadKeyword, IE_Imp_RTF::TranslateKeyword
*/
bool IE_Imp_RTF::ParseRTFKeyword()
{
	unsigned char keyword[MAX_KEYWORD_LEN];
	long parameter = 0;
	bool parameterUsed = false;
	if (ReadKeyword(keyword, &parameter, &parameterUsed, MAX_KEYWORD_LEN))
	{
		xxx_UT_DEBUGMSG(("SEVIOR: keyword = %s  par= %d \n",keyword,parameter));
		bool bres = TranslateKeyword(keyword, parameter, parameterUsed);
//		if(!bres)
//		{
//			xxx_UT_DEBUGMSG(("SEVIOR: %s error in translation \n",keyword));
//		}
		return bres;
	}
	else
		return false;
}


/*!
  Read a keyword from the file.
  \retval pKeyword the keyword buffer whose len is in keywordBuffLen
  Can not be NULL on input.
  \retval pParam the keyword parameter as specified by the RTF spec. 0
  is there is no param.
  \retval pParamUsed true if the keyword does really have a param. false
  otherwise (pParam is 0 then).
  \param keywordBuffLen the length of the pKeyword memory block
  \return false if any problem
  \desc This function parse and read the keyword. It is called if a
  \\ is encountered in the flow. *pKeyword never contains the \\
 */
bool IE_Imp_RTF::ReadKeyword(unsigned char* pKeyword, long* pParam, bool* pParamUsed, UT_uint32 keywordBuffLen)
{
	bool fNegative = false;
	*pParam = 0;
	*pParamUsed = false;
	*pKeyword = 0;
	const unsigned int max_param = 256;
	unsigned char parameter[max_param];
	unsigned int count = 0;

	// Read the first character of the control word
	unsigned char ch;
	if (!ReadCharFromFileWithCRLF(&ch))
		return false;

	UT_return_val_if_fail(keywordBuffLen > 1, false);
	--keywordBuffLen;

	// If it's a control symbol there is no delimiter, its just one character
	if (!isalpha(ch))
	{
		pKeyword[0] = ch;
		pKeyword[1] = 0;
		return true;
	}

	// Read in the rest of the control word
	while (isalpha(ch))
	{
		if (0 == --keywordBuffLen)
		{
			UT_DEBUGMSG(("Keyword too large. Bogus RTF!\n"));
			return false;
		}

		*pKeyword = ch;
		pKeyword++;
		if (!ReadCharFromFileWithCRLF(&ch))
			return false;
	}
	*pKeyword = 0;

    // If the delimeter was '-' then the following parameter is negative
    if (ch == '-')
    {
        fNegative = true;
		if (!ReadCharFromFileWithCRLF(&ch))
			return false;
    }

    // Read the numeric parameter (if there is one)
	if (isdigit(ch))
	{
		*pParamUsed = true;
		while (isdigit(ch))
		{
			// Avoid buffer overflow
			if (count == max_param )
			{
				UT_DEBUGMSG(("Parameter too large. Bogus RTF!\n"));
				return false;
			}

			parameter[count++] = ch;
			if (!ReadCharFromFileWithCRLF(&ch))
				return false;
		}
		parameter[count] = 0;

		*pParam = atol(reinterpret_cast<char*>(&parameter[0]));
		if (fNegative)
			*pParam = -*pParam;
	}

	// If the delimeter was non-whitespace then this character is part of the following text!
	if ((ch != ' ') && (ch != 10) && (ch != 13))
	{
		SkipBackChar(ch);
	}

	return true;
}

/*!
  Reads a character from the file. Doesn't ignore CR and LF
  \retval pCh the char read
  \return false if an error occured.
  \see IE_Imp_RTF::ReadCharFromFile
*/
bool IE_Imp_RTF::ReadCharFromFileWithCRLF(unsigned char* pCh)
{	
	bool ok = false;

	if (m_pImportFile)					// if we are reading a file
	{
		if (fread(pCh, 1, sizeof(unsigned char), m_pImportFile) > 0)
		{
			ok = true;
		}
	}
	else								// else we are pasting from a buffer
	{
		if (m_pCurrentCharInPasteBuffer < m_pPasteBuffer+m_lenPasteBuffer)
		{
			*pCh = *m_pCurrentCharInPasteBuffer++;
			ok = true;
		}
	}

	return ok;
}

/*!
  Reads a character from the file ignoring CR and LF
  \retval pCh the char read
  \return false if an error occured.
  \see IE_Imp_RTF::ReadCharFromFileWithCRLF
*/
bool IE_Imp_RTF::ReadCharFromFile(unsigned char* pCh)
{
	// line feed and cr should be ignored in RTF files
	do
	{
		if (ReadCharFromFileWithCRLF(pCh) == false)
		{
			return false;
		}
	} while (*pCh == 10  ||  *pCh == 13);

	return true;

}


/*!
  Push a char back to the stream.
  \param ch the char to push back
  \return false if any problem
  \desc Push back the char ch to the stream so it can be re-read
  after. Since we use buffered stdio from lib C, there should be
  no noticeable I/O impact.
  \fixme check that ungetc() works under MacOS
 */
bool IE_Imp_RTF::SkipBackChar(unsigned char ch)
{
	if (m_pImportFile)					// if we are reading a file
	{
		// TODO - I've got a sneaking suspicion that this doesn't work on the Macintosh
		return (ungetc(ch, m_pImportFile) != EOF);
	}
	else								// else we are pasting from a buffer
	{
		bool bStatus = (m_pCurrentCharInPasteBuffer > m_pPasteBuffer);
		if (bStatus)
			m_pCurrentCharInPasteBuffer--;
		return bStatus;
	}
}


/*!
  Skip the current group
  \param  bConsumeLastBrace pass true to discard the last }
  \return false if any problem raised
  \desc This function read until the current group and all nested
  subgroups are passed. This allow skipping a chunk of the RTF file
  we do not understand.
 */
bool IE_Imp_RTF::SkipCurrentGroup(bool bConsumeLastBrace)
{
	int nesting = 1;
	unsigned char ch;

	do {
		if (!ReadCharFromFileWithCRLF(&ch))
			return false;

		if (ch == '{')
		{
			++nesting;
		}
		else if (ch == '}')
		{
			--nesting;
		}
	} while (nesting > 0);

	// to avoid corrupting the state stack
	// ( the caller indicates whether this is necesary or not)
	if (!bConsumeLastBrace)
		SkipBackChar(ch);

	return true;
}


/*!
  Stuff the current group into the buffer
  \param  buf the buffer to stuff RTF in.
  \return false if any problem raised
  \desc This function read until the current group and all nested
  subgroups are passed and stuff them into the buffer. This allow saving a
  chunk of the RTF file for future use.
 */
bool IE_Imp_RTF::StuffCurrentGroup(UT_ByteBuf & buf)
{
	int nesting = 1;
	unsigned char ch;

	// add an intial bracket as it is supposed to have a final
	ch = '{';
	buf.append(&ch, 1);

	do {
		if (!ReadCharFromFileWithCRLF(&ch))
			return false;

		if (ch == '{')
		{
			++nesting;
		}
		else if (ch == '}')
		{
			--nesting;
		}
		buf.append(&ch, 1);
	} while (nesting > 0);

	// we don't want the last }
	SkipBackChar(ch);

	return true;
}

static IEGraphicFileType
iegftForRTF(IE_Imp_RTF::PictFormat format)
{
	switch (format) 
		{
		case IE_Imp_RTF::picPNG:  return IEGFT_PNG;
		case IE_Imp_RTF::picJPEG: return IEGFT_JPEG;
		case IE_Imp_RTF::picBMP:  return IEGFT_BMP;
		case IE_Imp_RTF::picWMF:  return IEGFT_WMF;
		default:
			break;
		}

	return IEGFT_Unknown;
}

/*!
  Load the picture data
  \param format the Picture Format.
  \param image_name the name of the image. Must be unique.
  \param imgProps the RTF properties for the image.
  \return true if success, otherwise false.
  \desc Load the picture data from the flow. Will move the file position
  and assume proper RTF file structure. It will take care of inserting
  the picture into the document.
  \todo TODO: We assume the data comes in hex. Check this assumption
  as we might have to handle binary data as well
  \see IE_Imp_RTF::HandlePicture
*/
bool IE_Imp_RTF::LoadPictData(PictFormat format, const char * image_name,
							  struct RTFProps_ImageProps & imgProps)
{
	// first, we load the actual data into a buffer
	bool ok;

	const UT_uint16 chars_per_byte = 2;
	const UT_uint16 BITS_PER_BYTE = 8;
	const UT_uint16 bits_per_char = BITS_PER_BYTE / chars_per_byte;

	UT_ByteBuf * pictData = new UT_ByteBuf();
	UT_uint16 chLeft = chars_per_byte;
	UT_Byte pic_byte = 0;

	unsigned char ch;

	if (!ReadCharFromFile(&ch))
		return false;

	while (ch != '}')
	{
		int digit;

		if (!hexVal(ch, digit))
			return false;

		pic_byte = (pic_byte << bits_per_char) + digit;

		// if we have a complete byte, we put it in the buffer
		if (--chLeft == 0)
		{
			pictData->append(&pic_byte, 1);
			chLeft = chars_per_byte;
			pic_byte = 0;
		}

		if (!ReadCharFromFile(&ch))
			return false;
	}

	// We let the caller handle this
	SkipBackChar(ch);

	// TODO: investigate whether pictData is leaking memory or not
	IE_ImpGraphic * pGraphicImporter = NULL;

	UT_Error error = IE_ImpGraphic::constructImporter(pictData, iegftForRTF(format), &pGraphicImporter);

	if ((error == UT_OK) && pGraphicImporter)
	{
		FG_Graphic* pFG = 0;

		// TODO: according with IE_ImpGraphic header, we shouldn't free
		// TODO: the buffer. Confirm that.
		error = pGraphicImporter->importGraphic(pictData, &pFG);
		DELETEP(pGraphicImporter);

		if (error != UT_OK || !pFG)
		{
			UT_DEBUGMSG(("Error parsing embedded PNG\n"));
			// Memory for pictData was destroyed if not properly loaded.
			return false;
		}

		UT_ByteBuf * buf = 0;
		buf = static_cast<FG_GraphicRaster *>(pFG)->getRaster_PNG();
		imgProps.width = static_cast<UT_uint32>(pFG->getWidth ());
		imgProps.height = static_cast<UT_uint32>(pFG->getHeight ());
		// Not sure whether this is the right way, but first, we should
		// insert any pending chars
		if (!FlushStoredChars(true))
		{
			UT_DEBUGMSG(("Error flushing stored chars just before inserting a picture\n"));
			delete pictData;
			return false;
		}

		ok = InsertImage (buf, image_name, imgProps);
		if (!ok)
		{
			delete pictData;
			return false;
		}
	}
	else
	{
		// if we're not inserting a graphic, we should destroy the buffer
		UT_DEBUGMSG (("no translator found: %d\n", error));
		delete pictData;
	}

	return true;
}


/*!
  \param buf the buffer the image content is in.
  \param image_name the image name inside the XML file or the piecetable.
  \param imgProps the RTF image properties.
  \return true is successful.
  Insert and image at the current position.
  Check whether we are pasting or importing
 */
bool IE_Imp_RTF::InsertImage (const UT_ByteBuf * buf, const char * image_name,
							  const struct RTFProps_ImageProps & imgProps)
{
	UT_String propBuffer;
	double wInch = 0.0f;
	double hInch = 0.0f;
	bool resize = false;
	if ((m_pImportFile != NULL) || (m_parsingHdrFtr))
	{
		// non-null file, we're importing a doc
		// Now, we should insert the picture into the document

		const char * mimetype = NULL;
		mimetype = UT_strdup("image/png");

		switch (imgProps.sizeType)
		{
		case RTFProps_ImageProps::ipstGoal:
			UT_DEBUGMSG (("Goal\n"));
			resize = true;
			wInch = static_cast<double>(imgProps.wGoal) / 1440.0f;
			hInch = static_cast<double>(imgProps.hGoal) / 1440.0f;
			break;
		case RTFProps_ImageProps::ipstScale:
			UT_DEBUGMSG (("Scale: x=%d, y=%d, w=%d, h=%d\n", imgProps.scaleX, imgProps.scaleY, imgProps.width, imgProps.height));
			resize = true;
			if ((imgProps.wGoal != 0) && (imgProps.hGoal != 0)) {
				// want image scaled against the w&h specified, not the image's natural size
				wInch = ((static_cast<double>(imgProps.scaleX) / 100.0f) * (imgProps.wGoal/ 1440.0f));
				hInch = ((static_cast<double>(imgProps.scaleY) / 100.0f) * (imgProps.hGoal/ 1440.0f));
			} else {
				wInch = ((static_cast<double>(imgProps.scaleX) / 100.0f) * (imgProps.width));
				hInch = ((static_cast<double>(imgProps.scaleY) / 100.0f) * (imgProps.height));
			}
			break;
		default:
			resize = false;
			break;
		}

		if (resize) {
			UT_DEBUGMSG (("resizing...\n"));
			char * old_locale = setlocale(LC_NUMERIC, "C");
			UT_String_sprintf(propBuffer, "width:%fin; height:%fin",
							  wInch, hInch);
			setlocale(LC_NUMERIC, old_locale);
			UT_DEBUGMSG (("props are %s\n", propBuffer.c_str()));
		}

		const XML_Char* propsArray[5];
		propsArray[0] = static_cast<const XML_Char *>("dataid");
		propsArray[1] = static_cast<const XML_Char *>(image_name);
		if (resize)
		{
			propsArray[2] = static_cast<const XML_Char *>("props");
			propsArray[3] = propBuffer.c_str();
			propsArray[4] = NULL;
		}
		else
		{
			propsArray[2] = NULL;
		}
		UT_DEBUGMSG(("SEVIOR: Appending Object 2 m_bCellBlank %d m_bEndTableOpen %d \n",m_bCellBlank,m_bEndTableOpen));
		if(m_bCellBlank || m_bEndTableOpen)
		{
			getDoc()->appendStrux(PTX_Block,NULL);
			m_bCellBlank = false;
			m_bEndTableOpen = false;
		}

		if (!getDoc()->appendObject(PTO_Image, propsArray))
		{
			FREEP(mimetype);
			return false;
		}

		if (!getDoc()->createDataItem(image_name, false,
									  buf, static_cast<const void*>(mimetype), NULL))
		{
			// taken care of by createDataItem
			//FREEP(mimetype);
			return false;
		}
	}
	else
	{
		// null file, we're pasting an image. this really makes
		// quite a difference to the piece table

		// Get a unique name for image.
		UT_ASSERT_HARMLESS(image_name);
		UT_String szName;

		if( !image_name)
		{
			image_name = "image_z";
		}
		UT_uint32 ndx = 0;
		for (;;)
		{
			UT_String_sprintf(szName, "%s_%d", image_name, ndx);
			if (!getDoc()->getDataItemDataByName(szName.c_str(), NULL, NULL, NULL))
			{
				break;
			}
			ndx++;
		}
//
// Code from fg_GraphicsRaster.cpp
//
		/*
		  Create the data item
		*/
		const char * mimetype = NULL;
		mimetype = UT_strdup("image/png");
		bool bOK = false;
		bOK = getDoc()->createDataItem(szName.c_str(), false, buf, static_cast<const void *>(mimetype), NULL);
		UT_return_val_if_fail(bOK, false);
		/*
		  Insert the object into the document.
		*/
		bool resize = false;

		switch (imgProps.sizeType)
		{
		case RTFProps_ImageProps::ipstGoal:
			UT_DEBUGMSG (("Goal\n"));
			resize = true;
			wInch = static_cast<double>(imgProps.wGoal) / 1440.0f;
			hInch = static_cast<double>(imgProps.hGoal) / 1440.0f;
			break;
		case RTFProps_ImageProps::ipstScale:
			UT_DEBUGMSG (("Scale: x=%d, y=%d, w=%d, h=%d\n", imgProps.scaleX, imgProps.scaleY, imgProps.width, imgProps.height));
			resize = true;
			wInch = ((static_cast<double>(imgProps.scaleX) / 100.0f) * imgProps.width);
			hInch = ((static_cast<double>(imgProps.scaleY) / 100.0f) * imgProps.height);
			break;
		default:
			resize = false;
			break;
		}

		if (resize)
		{
			UT_DEBUGMSG (("resizing...\n"));
			char * old_locale = setlocale(LC_NUMERIC, "C");
			UT_String_sprintf(propBuffer, "width:%fin; height:%fin",
							  wInch, hInch);
			setlocale(LC_NUMERIC, old_locale);
			UT_DEBUGMSG (("props are %s\n", propBuffer.c_str()));
		}

		const XML_Char* propsArray[5];
		propsArray[0] = static_cast<const XML_Char *>("dataid");
		propsArray[1] = static_cast<const XML_Char *>(szName.c_str());
		if (resize)
		{
			propsArray[2] = static_cast<const XML_Char *>("props");
			propsArray[3] = propBuffer.c_str();
			propsArray[4] = NULL;
		}
		else
		{
			propsArray[2] = NULL;
		}
		getDoc()->insertObject(m_dposPaste, PTO_Image, propsArray, NULL);
		m_dposPaste++;
	}
	return true;
}

/*!
  Handle a picture in the current group
  \return false if failed
  \desc Once the \\pict has been read, hande the picture contained in
  the current group. Calls LoadPictData
  \see IE_Imp_RTF::LoadPictData
  \fixme TODO handle image size and other options in the future
 */
bool IE_Imp_RTF::HandlePicture()
{
	// this method loads a picture from the file
	// and insert it in the document.
	static UT_uint32 nImage = 0;

	unsigned char ch;
	bool bPictProcessed = false;
	PictFormat format = picNone;

	unsigned char keyword[MAX_KEYWORD_LEN];
	long parameter = 0;
	bool parameterUsed = false;
	RTFProps_ImageProps imageProps;

	do {
		if (!ReadCharFromFile(&ch))
			return false;

		switch (ch)
		{
		case '\\':
			UT_return_val_if_fail(!bPictProcessed, false);
			// Process keyword

			if (!ReadKeyword(keyword, &parameter, &parameterUsed, MAX_KEYWORD_LEN))
			{
				UT_DEBUGMSG(("Unexpected EOF during RTF import?\n"));
			}

			if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pngblip") == 0)
			{
				format = picPNG;
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "jpegblip") == 0)
			{
				format = picJPEG;
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "picwgoal") == 0)
			{
				if (parameterUsed)
					{
						if ((imageProps.sizeType == RTFProps_ImageProps::ipstNone))
							imageProps.sizeType = RTFProps_ImageProps::ipstGoal;
						imageProps.wGoal = parameter;
					}
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pichgoal") == 0)
			{
				if (parameterUsed)
					{
						if ((imageProps.sizeType == RTFProps_ImageProps::ipstNone))
							imageProps.sizeType = RTFProps_ImageProps::ipstGoal;
						imageProps.hGoal = parameter;
					}
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "picscalex") == 0)
			{
				if ((imageProps.sizeType == RTFProps_ImageProps::ipstNone)
					|| (imageProps.sizeType == RTFProps_ImageProps::ipstScale))
				{
					if ((parameterUsed) && (parameter != 100))
					{
						imageProps.sizeType = RTFProps_ImageProps::ipstScale;
						imageProps.scaleX = static_cast<unsigned short>(parameter);
					}
				}
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "picscaley") == 0)
			{
				if ((imageProps.sizeType == RTFProps_ImageProps::ipstNone)
					|| (imageProps.sizeType == RTFProps_ImageProps::ipstScale))
				{
					if ((parameterUsed) && (parameter != 100))
					{
						imageProps.sizeType = RTFProps_ImageProps::ipstScale;
						imageProps.scaleY = static_cast<unsigned short>(parameter);
					}
				}
			}
			break;
		case '{':
			UT_return_val_if_fail(!bPictProcessed, false);

			// We won't handle nested groups, at least in this version,
			// we just skip them
			SkipCurrentGroup(true);
			break;
		case '}':
			// check if a pict was found ( and maybe inserted )
			// as this would be the last iteration
			if (!bPictProcessed)
			{
				UT_DEBUGMSG(("Bogus RTF: \\pict group without a picture\n"));
				return false;
			}
			break;
		default:
			UT_return_val_if_fail(!bPictProcessed, false);
			// It this a conforming rtf, this should be the pict data
			// if we know how to handle this format, we insert the picture

			UT_String image_name;
			UT_String_sprintf(image_name,"image_%d",++nImage);

			// the first char belongs to the picture too
			SkipBackChar(ch);

			if (!LoadPictData(format, image_name.c_str(), imageProps))
				if (!SkipCurrentGroup())
					return false;

			bPictProcessed = true;
		}
	} while (ch != '}');

	// The last brace is put back on the stream, so that the states stack
	// doesn't get corrupted
	SkipBackChar(ch);

	return true;
}

/*!
  Handle a object in the current group
  \return false if failed
  \desc Once the \\object has been read, handle the object contained in
  the current group.
  \todo in the future this method should load an object from the file
  and insert it in the document. To fix some open bugs, we just
  skip all the data and do nothing
 */
bool IE_Imp_RTF::HandleObject()
{
	UT_DEBUGMSG(("TODO: Handle \\object keyword properly\n"));
	return SkipCurrentGroup();
}

/*!
  Handle a RTF field
  \return false if failed
  \desc Once the \\field has been read, handle the object contained in
  the current group. This is really tricky as fields are really
  hard to handle since most writers do whatever they want, including
  RTF code interleaved with field instruction. Thank you Microsoft
  (sorry for the rant). Call IE_Imp_RTF::_parseFldinstBlock to
  parse field instructions
  See p44 for specs.
  \see IE_Imp_RTF::_parseFldinstBlock
 */
bool IE_Imp_RTF::HandleField()
{
	RTFTokenType tokenType;
	unsigned char keyword[MAX_KEYWORD_LEN];
	bool ok = false;
	long parameter = 0;
	bool paramUsed = false;
	bool bUseResult = false;  // true if field instruction can not be used
	int nested = 0;           // nesting level

	int rtfFieldAttr = 0;

	typedef enum {
		fldAttrDirty = 1,
		fldAttrEdit = 2,
		fldAttrLock = 4,
		fldAttrPriv = 8
	} RTFFieldAttr;


	UT_uint32 iHyperlinkOpen = m_iHyperlinkOpen;
	
	tokenType = NextToken (keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN);
	if (tokenType == RTF_TOKEN_ERROR)
	{
		return false;
	}

	// read the optional attribute for the field.
	while (tokenType == RTF_TOKEN_KEYWORD)
	{
		if (strcmp (reinterpret_cast<char*>(&keyword[0]), "flddirty") == 0)
		{
			rtfFieldAttr &= fldAttrDirty;
		}
		else if (strcmp (reinterpret_cast<char*>(&keyword[0]), "fldedit") == 0)
		{
			rtfFieldAttr &= fldAttrEdit;
		}
		else if (strcmp (reinterpret_cast<char*>(&keyword[0]), "fldlock") == 0)
		{
			rtfFieldAttr &= fldAttrLock;
		}
		else if (strcmp (reinterpret_cast<char*>(&keyword[0]), "fldpriv") == 0)
		{
			rtfFieldAttr &= fldAttrPriv;
		}
		else
		{
			UT_DEBUGMSG (("RTF: Invalid keyword '%s' in field\n"));
			// don't return as we simply skip it
		}


		tokenType = NextToken (keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN);
	}

	// field instruction
	if (tokenType == RTF_TOKEN_OPEN_BRACE)
	{
		UT_ByteBuf fldBuf;
		XML_Char * xmlField = NULL;
		bool gotStarKW = false;
		bool gotFldinstKW = false;
		// bUseResult will to be set to false if we encounter a field
		// instruction we know about. Otherwise, we use the result by default
		bUseResult = true;
		// since we enter a brace group, we push the RTFState.
		PushRTFState ();
		nested = 0;
		do
		{
			tokenType = NextToken (keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN);
			switch (tokenType)
			{
			case RTF_TOKEN_ERROR:
				UT_ASSERT_NOT_REACHED();
				return false;
				break;
			case RTF_TOKEN_KEYWORD:
				if (strcmp(reinterpret_cast<const char *>(&keyword[0]), "*") == 0)
				{
					if (gotStarKW)
					{
						UT_DEBUGMSG (("RTF: was not supposed to get '*' here\n"));
					}
					gotStarKW = true;
				}
				else if (strcmp(reinterpret_cast<const char *>(&keyword[0]), "fldinst") == 0)
				{
					if (!gotStarKW)
					{
						UT_DEBUGMSG (("Ohoh, we were not supposed to get a 'fldinst' without a '*'. Go ahead.\n"));
					}
					gotFldinstKW = true;
				}
				else if (strcmp(reinterpret_cast<const char *>(&keyword[0]), "\\") == 0)
				{
					fldBuf.append (keyword, 1);
				}
				break;
			case RTF_TOKEN_OPEN_BRACE:
				nested++;
				PushRTFState ();
				break;
			case RTF_TOKEN_CLOSE_BRACE:
				nested--;
				PopRTFState ();
				break;
			case RTF_TOKEN_DATA:
				// add data to the field
				fldBuf.append (keyword, 1);
				break;
			default:
				break;
			}
		}
		while ((tokenType != RTF_TOKEN_CLOSE_BRACE) || (nested >= 0));
		bool isXML = false;
		xmlField = _parseFldinstBlock (fldBuf, xmlField, isXML);
		bUseResult = (xmlField == NULL) && (!isXML);
		if (!bUseResult)
		{
			bool ok;
			ok = _appendField (xmlField);
			UT_ASSERT_HARMLESS (ok);
			// we own xmlField, so we delete it after use.
			FREEP (xmlField);
		}
	}
	else
	{
		UT_DEBUGMSG (("RTF: Field instruction not present. Found '%s' in stream\n", keyword));
		UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
		// continue
	}

	tokenType = NextToken (keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN);
	if (tokenType == RTF_TOKEN_ERROR)
	{
		return false;
	}

	// field result
	// TODO: push and pop the state as expected.
	if (tokenType == RTF_TOKEN_OPEN_BRACE)
	{
		PushRTFState();
		tokenType = NextToken (keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN);
		if (tokenType == RTF_TOKEN_ERROR)
		{
			return false;
		}
		if (tokenType == RTF_TOKEN_KEYWORD)
		{
			// here we expect fldrslt keyword, nothing else
			if (strcmp (reinterpret_cast<char*>(&keyword[0]), "fldrslt") != 0)
			{
				UT_DEBUGMSG (("RTF: Invalid keyword '%s' in field\n"));
				// don't return as we simply skip it
			}
		}
		
		// The original code parsing the result was not enough: the
		// field result can contain full-blown rtf markup, including
		// other fields, etc. That means that we have to parse it just
		// like we do ordinary text.
		if(bUseResult)
		{
			if(UT_OK != _parseText())
				return false;
		}
	}
	else
	{
		UT_DEBUGMSG (("RTF: Field result not present. Found '%s' in stream. Ignoring.\n", keyword));
		// UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
		// continue
	}

	if(m_iHyperlinkOpen > iHyperlinkOpen)
	{
		FlushStoredChars(true);
		getDoc()->appendObject(PTO_Hyperlink,NULL);
		m_iHyperlinkOpen--;
		UT_ASSERT( m_iHyperlinkOpen == iHyperlinkOpen );
	}
		
	return true;
}


/*!
  \param buf the buffer that contains the RTF.
  \param xmlField the XML attributes for the field.
  \param isXML whether xmlField is used or not.
  \see IE_Imp_RTF::HandleField
 */
XML_Char *IE_Imp_RTF::_parseFldinstBlock (UT_ByteBuf & buf, XML_Char *xmlField, bool & isXML)
{
	// this is quite complex as field instructions are not really document in the RTF specs.
	// we will guess as much us possible.
	// thing that complexify is that a field instruction can contain nested block and
	// DATA blocks. We will try to limit first to the standard fields.

	// Here are a couple of examples from the spec
	/*
	  {\field {\*\fldinst AUTHOR \\*MERGEFORMAT    }{\fldrslt Joe Smith}}\par\pard
	  {\field{\*\fldinst time \\@ "h:mm AM/PM"}{\fldrslt 8:12 AM}}
	  {\field{\*\fldinst NOTEREF _RefNumber } {\fldrslt 1}}
	  {\field{\*\fldinst NOTEREF _RefNumber \fldalt } {\fldrslt I}}
	*/
	// Here is an example from StarOffice 5.2 export: an Hyperlink
	/*
	  {\field{\*\fldinst HYPERLINK "http://www.sas.com/techsup/download/misc/cleanwork.c" }
	  {\fldrslt \*\cs21\cf1\ul http://www.sas.com/techsup/download/misc/cleanwork.c}}
	*/
	// This time it is an image: StarOffice exports images in .jpg as a separate file
	/*
	  {\field\fldpriv{\*\fldinst{\\import sv8968971.jpg}}{\fldrslt }}
	*/
	/*
	  OpenOffice/StarOffice 6 do this in a similar way. See OpenOffice bug 2244.
	*/
	/* Microsoft doc on field usages in Word is at:
	   <http://support.microsoft.com/support/word/usage/fields/>
	*/

	/* IMPORTANT: field results can contain full-blown rtf markup,
	   incuding embeded field, etc. For instnace the result for a TOC
	   field contains hyperlinks that allow jumping from the TOC to a
	   relevant page. This means that the result cannot be simply
	   pasted into the document
	*/

	char *instr;
	char *newBuf;
	UT_String Instr;
	UT_uint32  len;
	isXML = false;

	// buffer is empty, nothing to parse
	if (buf.getLength() == 0)
	{
		FREEP (xmlField);
		return NULL;
	}

	len = buf.getLength ();
	const UT_Byte *pBuf = buf.getPointer (0);

	newBuf =  static_cast<char *>(malloc (sizeof (char) * (len + 1)));
	memcpy (newBuf, pBuf, len);
	newBuf [len] = 0;
	Instr = newBuf;
	instr = const_cast<char *>(Instr.c_str());
	instr = strtok (instr, " \\{}"); // This writes a NULL into Instr somewhere
	                                 // I assume this is OK since the char storage
	                                 // Within the class is contiguous.
	if (instr == NULL)
	{
		free (newBuf);
		free (xmlField);
		return NULL;
	}

	switch (*instr)
	{
	case 'A':
		if (strcmp (instr, "AUTHOR") == 0)
		{
			UT_DEBUGMSG (("RTF: AUTHOR fieldinst not handled yet\n"));
		}
		break;
	case 'F':
		if (strcmp (instr, "FILENAME") == 0)
		{
			// TODO handle parameters
			xmlField = UT_strdup ("file_name");
			UT_ASSERT_HARMLESS (xmlField);
			isXML = (xmlField != NULL);
		}
		break;
	case 'H':
		if (strcmp (instr, "HYPERLINK") == 0)
		{
			xxx_UT_DEBUGMSG (("RTF: HYPERLINK fieldinst not handled yet\n"));
			// set these so that HandleField outputs the result
			xmlField = NULL;
			isXML = false;
			
			instr = strtok(0, " \\{}");
			const XML_Char *new_atts[3];

			new_atts[0] = "xlink:href";
			UT_String href;
			if ( !strcmp(instr, "l") )
			{
				instr = strtok (NULL, " \\{}");
				href = "#";
			}
			else
			{
				href.clear();
			}

			// the full address is enclosed in quotation marks,
			// which need to be removed
			if(*instr == '\"')
				instr++;

			if(instr[strlen(instr)-1])
				instr[strlen(instr)-1] = 0;

			href += instr;

			UT_String full_href;

			const char * s = href.c_str();

			if(*s != '#' && !UT_isUrl(s))
			{
				// TODO we are dealing with a relative URL; until AW
				// can handle relative URLs we will convert it into an
				// absolute one
				full_href = m_hyperlinkBase;
				const char * s2 = full_href.c_str();
				
				if(*s != '/' && s2[strlen(s2)-1] != '/')
				{
					full_href += '/';
					full_href += s;
				}
				else if(*s == '/' && s2[strlen(s2)-1] == '/')
				{
					full_href += (s+1);
				}
				else
				{
					full_href += s;
				}
				
				s = full_href.c_str();
			}
			
			new_atts[1] = s;
			new_atts[2] = 0;

			FlushStoredChars(true);
			getDoc()->appendObject(PTO_Hyperlink,new_atts);
			m_iHyperlinkOpen++;
		}
		break;
	case 'I':
		if (strcmp (instr, "INCLUDEPICTURE") == 0)
		{
			UT_DEBUGMSG (("RTF: INCLUDEPICTURE fieldinst not handled yet\n"));
		}
		break;
	case 'P':
		if (strcmp (instr, "PAGE") == 0)
		{
			xmlField = UT_strdup ("page_number");
			UT_ASSERT_HARMLESS (xmlField);
			isXML = (xmlField != NULL);
		}
		else if (strcmp (instr, "PRIVATE") == 0)
		{
			UT_DEBUGMSG (("RTF: PRIVATE fieldinst not handled yet\n"));
		}
		break;
	case 'N':
		if (strcmp (instr, "NUMCHARS") == 0)
		{
			xmlField = UT_strdup ("char_count");
			UT_ASSERT_HARMLESS (xmlField);
			isXML = (xmlField != NULL);
		}
		// this one have been found with ApplixWare exported RTF.
		else if (strcmp (instr, "NUMPAGES") == 0)
		{
			xmlField = UT_strdup ("page_count");
			UT_ASSERT_HARMLESS (xmlField);
			isXML = (xmlField != NULL);
		}
		else if (strcmp (instr, "NUMWORDS") == 0)
		{
			xmlField = UT_strdup ("word_count");
			UT_ASSERT_HARMLESS (xmlField);
			isXML = (xmlField != NULL);
		}
		break;
	case 'S':
		if (strcmp (instr, "SAVEDATE") == 0)
		{
			xmlField = UT_strdup ("date_dfl");
			UT_ASSERT_HARMLESS (xmlField);
			isXML = (xmlField != NULL);
		}
		break;
	case 'T':
		if (strcmp (instr, "TIME") == 0)
		{
			// Some Parameters from MS Word 2000 output

			if(strstr(newBuf,"dddd, MMMM dd, yyyy") != NULL)
			{
				xmlField = UT_strdup("date");
				UT_ASSERT_HARMLESS (xmlField);
				isXML = (xmlField != NULL);
			}
			else if( strstr(newBuf,"m/d/yy") != NULL)
			{
				xmlField = UT_strdup("date_ddmmyy");
				UT_ASSERT_HARMLESS (xmlField);
				isXML = (xmlField != NULL);
			}
			else if( strstr(newBuf,"MMMM d, yyyy") != NULL)
			{
				xmlField = UT_strdup("date_mdy");
				UT_ASSERT (xmlField);
				isXML = (xmlField != NULL);
			}
			else if( strstr(newBuf,"MMM d, yy") != NULL)
			{
				xmlField = UT_strdup("date_mthdy");
				UT_ASSERT_HARMLESS (xmlField);
				isXML = (xmlField != NULL);
			}
			else if( strstr(newBuf,"MMM d, yy") != NULL)
			{
				xmlField = UT_strdup("date_mthdy");
				UT_ASSERT_HARMLESS (xmlField);
				isXML = (xmlField != NULL);
			}
			else if( strstr(newBuf,"MM-d-yy") != NULL)
			{
				xmlField = UT_strdup("date_ntdfl");
				UT_ASSERT_HARMLESS (xmlField);
				isXML = (xmlField != NULL);
			}
			else if( strstr(newBuf,"HH:mm:ss") != NULL)
			{
				xmlField = UT_strdup("time_miltime");
				UT_ASSERT_HARMLESS (xmlField);
				isXML = (xmlField != NULL);
			}
			else if( strstr(newBuf,"h:mm:ss am/pm") != NULL)
			{
				xmlField = UT_strdup("time_ampm");
				UT_ASSERT_HARMLESS (xmlField);
				isXML = (xmlField != NULL);
			}
//
// Make this the second last one since it's not unique
//
			else if( strstr(newBuf,"dddd") != NULL)
			{
				xmlField = UT_strdup("date_wkday");
				UT_ASSERT_HARMLESS (xmlField);
				isXML = (xmlField != NULL);
			}
			else
			{
				xmlField = UT_strdup ("time");
				UT_ASSERT_HARMLESS (xmlField);
				isXML = (xmlField != NULL);
			}
		}
		else if (strcmp (instr, "TOC") == 0)
		{
			// Table-of-contents field
			UT_DEBUGMSG (("RTF: TOC fieldinst not handled yet\n"));
		}
		
		break;
	case 'd':
		if (strcmp (instr, "date") == 0)
		{
			// TODO handle parameters
			xmlField = UT_strdup ("date");
			UT_ASSERT_HARMLESS (xmlField);
			isXML = (xmlField != NULL);
		}
		break;
	case '\\':
		/* mostly StarOffice RTF fields */
		if (strcmp (instr, "\\filename") == 0)
		{
			xmlField = UT_strdup ("file_name");
			UT_ASSERT_HARMLESS (xmlField);
			isXML = (xmlField != NULL);
		}
		else if (strcmp (instr, "\\import") == 0)
		{
			// need to read the filename.
			UT_DEBUGMSG (("importing StarOffice image\n"));

			if (m_szFileDirName	!= NULL)
			{
				char * fileName = NULL;
				char * tok  = strtok (NULL, " ");
				fileName = UT_catPathname (m_szFileDirName, tok);
				UT_DEBUGMSG (("fileName is %s\n", fileName));

				bool ok = FlushStoredChars ();
				if (ok)
				{
					// insert the image in the piece table AFTER flushing
					// current output
					IE_ImpGraphic * pGraphicImporter = NULL;
					FG_Graphic* pFG;
					UT_Error error = IE_ImpGraphic::constructImporter(fileName, IEGFT_JPEG, &pGraphicImporter);

					// load file to buffer
					if (error == UT_OK)
					{
						RTFProps_ImageProps imgProps;
						error = pGraphicImporter->importGraphic(fileName, &pFG);
						DELETEP(pGraphicImporter);
						UT_ByteBuf * buf;
						buf = static_cast<FG_GraphicRaster *>(pFG)->getRaster_PNG();
						ok = InsertImage (buf, fileName, imgProps);
					}
					else
					{
						UT_DEBUGMSG (("RTF: error while importing SO image: %d\n", error));
					}
				}
				else
				{
					UT_DEBUGMSG (("RTF: we don't know the current filename path\n"));
				}
				FREEP (fileName);
			}
		}
		else if (strcmp (instr, "\\page") == 0)
		{
			xmlField = UT_strdup ("page_number");
			UT_ASSERT_HARMLESS (xmlField);
			isXML = (xmlField != NULL);
		}
		break;
	default:
		UT_DEBUGMSG (("RTF: unhandled fieldinstr %s\n", instr));
		break;
	}
	free (newBuf);
	return xmlField;
}


/*!
  Handle a header
  \retvalue header return the created header, for information
  purpose since it belongs to the header/footer table
  \note it does not set the RTF state.
 */
bool IE_Imp_RTF::HandleHeaderFooter(RTFHdrFtr::HdrFtrType hftype, UT_uint32 & headerID)
{
	RTFHdrFtr * header;
	UT_DEBUGMSG(("SEVIOR: Doing handle header/footer \n"));
	header = new RTFHdrFtr ();
	header->m_type = hftype;
	UT_uint32 id = 0;
	while(id < 10000)
	{
		id  = UT_rand();
	}

	header->m_id = id;

	m_hdrFtrTable.addItem (header);
	headerID = header->m_id;

	switch (hftype)
	{
	case RTFHdrFtr::hftHeader:
		UT_DEBUGMSG(("RTF: \\header stuffed into %d\n",headerID));
		m_currentHdrID = headerID;
		break;
	case RTFHdrFtr::hftHeaderEven:
		UT_DEBUGMSG(("RTF: \\header Even stuffed into %d\n",headerID));
		m_currentHdrEvenID = headerID;
		break;
	case RTFHdrFtr::hftHeaderFirst:
		UT_DEBUGMSG(("RTF: \\header First stuffed into %d\n",headerID));
		m_currentHdrFirstID = headerID;
		break;
	case RTFHdrFtr::hftHeaderLast:
		UT_DEBUGMSG(("RTF: \\header Last stuffed into %d\n",headerID));
		m_currentHdrLastID = headerID;
		break;
	case RTFHdrFtr::hftFooter:
		UT_DEBUGMSG(("RTF: \\footer stuffed into %d\n",headerID));
		m_currentFtrID = headerID;
		break;
	case RTFHdrFtr::hftFooterEven:
		UT_DEBUGMSG(("RTF: \\footer Even stuffed into %d\n",headerID));
		m_currentFtrEvenID = headerID;
		break;
	case RTFHdrFtr::hftFooterFirst:
		UT_DEBUGMSG(("RTF: \\footer stuffed into %d\n",headerID));
		m_currentFtrFirstID = headerID;
		break;
	case RTFHdrFtr::hftFooterLast:
		UT_DEBUGMSG(("RTF: \\footer stuffed into %d\n",headerID));
		m_currentFtrLastID = headerID;
		break;
	default:
		UT_ASSERT_NOT_REACHED();
	}

	// read the whole group content and put it into a buffer to
	// decode it later, when appending footer to the document.
	return StuffCurrentGroup (header->m_buf);
}


// Test the keyword against all the known handlers
bool IE_Imp_RTF::TranslateKeyword(unsigned char* pKeyword, long param, bool fParam)
{
	// switch on the first char to reduce the number of string comparisons
	// NB. all RTF keywords are lowercase.
	// after handling the keyword, return true
	switch (*pKeyword)
	{
	case 'a':
		if (strcmp(reinterpret_cast<char*>(pKeyword), "ansicpg") == 0)
		{
			const char *szEncoding = XAP_EncodingManager::get_instance()->charsetFromCodepage(static_cast<UT_uint32>(param));
			m_mbtowc.setInCharset(szEncoding);
			getDoc()->setEncodingName(szEncoding);
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "abitopline") == 0)
		{
			return HandleTopline(true);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "abibotline") == 0)
		{
			return HandleBotline(true);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "ansi") == 0)
		{
			// this is charset Windows-1252
			const char *szEncoding = XAP_EncodingManager::get_instance()->charsetFromCodepage(1252);
			m_mbtowc.setInCharset(szEncoding);
			getDoc()->setEncodingName(szEncoding);
			return true;
		}
		break;
	case 'b':
		if (strcmp(reinterpret_cast<char*>(pKeyword), "b") == 0)
		{
			// bold - either on or off depending on the parameter
			return HandleBold(fParam ? false : true);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "bullet") == 0)
		{
			return ParseChar(UCS_BULLET);
		}
		break;

	case 'c':
		if (strcmp(reinterpret_cast<char*>(pKeyword), "colortbl") == 0)
		{
			return ReadColourTable();
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "cf") == 0)
		{
			return HandleColour(fParam ? param : 0);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "cb") == 0)
		{
			return HandleBackgroundColour (fParam ? param : 0);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "cols") == 0)
		{
			m_currentRTFState.m_sectionProps.m_numCols = static_cast<UT_uint32>(param);
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "colsx") == 0)
		{
			m_currentRTFState.m_sectionProps.m_colSpaceTwips = static_cast<UT_uint32>(param);
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "column") == 0) // column break
		{
			return ParseChar(UCS_VTAB);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "chdate") == 0)
		{
			return _appendField ("date");
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "chtime") == 0)
		{
			return _appendField ("time");
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "chdpl") == 0)
		{
			const XML_Char * attribs[3] ={"param",NULL,NULL};
			attribs[1] = "%A, %B %d, %Y";
			return _appendField ("datetime_custom", attribs);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "chdpa") == 0)
		{
			const XML_Char * attribs[3] ={"param",NULL,NULL};
			attribs[1] = "%a, %b %d, %Y";
			return _appendField ("datetime_custom");
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "chpgn") == 0)
		{
			return _appendField ("page_number");
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "chftn") == 0)
		{
			const XML_Char * attribs[3] ={"footnote-id",NULL,NULL};
			UT_String footpid;
			UT_uint32 pid = 0;
			if(m_bInFootnote)
			{
				UT_String_sprintf(footpid,"%i",m_iLastFootnoteId);
				attribs[1] = footpid.c_str();
				return _appendField ("footnote_anchor",attribs);
			}
			else
			{
				while (pid < AUTO_LIST_RESERVED)
					pid = UT_rand();
				m_iLastFootnoteId = pid;
				UT_String_sprintf(footpid,"%i",m_iLastFootnoteId);
				attribs[1] = footpid.c_str();
				return _appendField ("footnote_ref",attribs);
			}
		}
 		else if (strcmp(reinterpret_cast<char*>(pKeyword), "cs") == 0)
 		{
 			m_currentRTFState.m_charProps.m_styleNumber = param;
 			return true;
 		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "cell") == 0)
		{
			UT_DEBUGMSG(("SEVIOR: Processing cell \n"));
			HandleCell();
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "cellx") == 0)
		{
			HandleCellX(param);
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "clvmrg") == 0)
		{
			m_currentRTFState.m_cellProps.m_bVerticalMerged = true;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "clvmgf") == 0)
		{
			m_currentRTFState.m_cellProps.m_bVerticalMergedFirst = true;
			return true;
		}
		break;

	case 'd':
		if (strcmp(reinterpret_cast<char*>(pKeyword), "deleted") == 0)
		{
			// bold - either on or off depending on the parameter
			return HandleDeleted(fParam ? false : true);
		}
		else if (strcmp(reinterpret_cast<char *>(pKeyword),"dn") == 0)
		{
			// subscript with position. Default is 6.
			// superscript: see up keyword
			return HandleSubscriptPosition (fParam ? param : 6);
		}
		break;
	case 'e':
		if (strcmp(reinterpret_cast<char*>(pKeyword), "emdash") == 0)
		{
			return ParseChar(UCS_EM_DASH);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "endash") == 0)
		{
			return ParseChar(UCS_EN_DASH);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "emspace") == 0)
		{
			return ParseChar(UCS_EM_SPACE);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "enspace") == 0)
		{
			return ParseChar(UCS_EN_SPACE);
		}
		break;

	case 'f':
		if (strcmp(reinterpret_cast<char*>(pKeyword), "fonttbl") == 0)
		{
			// read in the font table
			return ReadFontTable();
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "fs") == 0)
		{
			return HandleFontSize(fParam ? param : 24);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "f") == 0)
		{
			return HandleFace(fParam ? param : 0); // TODO read the deff prop and use that instead of 0
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "fi") == 0)
		{
			m_currentRTFState.m_paraProps.m_indentFirst = param;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "field") == 0)
		{
			return HandleField ();
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "footer") == 0)
		{
			UT_uint32 footerID = 0;
			return HandleHeaderFooter (RTFHdrFtr::hftFooter, footerID);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "footerf") == 0)
		{
			UT_uint32 footerID = 0;
			return HandleHeaderFooter (RTFHdrFtr::hftFooterFirst, footerID);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "footerr") == 0)
		{
			UT_uint32 footerID = 0;
			return HandleHeaderFooter (RTFHdrFtr::hftFooter, footerID);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "footerl") == 0)
		{
			UT_uint32 footerID = 0;
			return HandleHeaderFooter (RTFHdrFtr::hftFooterEven, footerID);
		}
		else if( strcmp(reinterpret_cast<char *>(pKeyword), "footery") == 0)
		{
			// Height of the footer in twips
			m_currentRTFState.m_sectionProps.m_footerYTwips = param;
		}
		else if( strcmp(reinterpret_cast<char *>(pKeyword), "footnote") == 0)
		{
			HandleFootnote();
			return true;
		}
		break;
	case 'h':
		if( strcmp(reinterpret_cast<char *>(pKeyword), "headery") == 0)
		{
			// Height ot the header in twips
			m_currentRTFState.m_sectionProps.m_headerYTwips = param;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "header") == 0)
		{
			UT_uint32 headerID = 0;
			return HandleHeaderFooter (RTFHdrFtr::hftHeader, headerID);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "headerf") == 0)
		{
			UT_uint32 headerID = 0;
			return HandleHeaderFooter (RTFHdrFtr::hftHeaderFirst, headerID);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "headerr") == 0)
		{
			UT_uint32 headerID = 0;
			return HandleHeaderFooter (RTFHdrFtr::hftHeader, headerID);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "headerl") == 0)
		{
			UT_uint32 headerID = 0;
			return HandleHeaderFooter (RTFHdrFtr::hftHeaderEven, headerID);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "highlight") == 0)
		{
			return HandleBackgroundColour(param);
		}
		break;
	case 'i':
		if (strcmp(reinterpret_cast<char*>(pKeyword), "i") == 0)
		{
			// italic - either on or off depending on the parameter
			return HandleItalic(fParam ? false : true);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "info") == 0)
		{
			// TODO Ignore document info for the moment
			m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "ilvl") == 0)
		{
			m_currentRTFState.m_paraProps.m_iOverrideLevel = static_cast<UT_uint32>(param);
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "itap") == 0)
		{
			if(!m_pImportFile)
			{
				return true;
			}
			m_currentRTFState.m_paraProps.m_tableLevel = param;
//
// Look to see if the nesting level of our tables has changed.
//
			UT_DEBUGMSG(("SEVIOR!!! itap m_tableLevel %d nestDepth %d \n",m_currentRTFState.m_paraProps.m_tableLevel,m_TableControl.getNestDepth()));
			if(m_currentRTFState.m_paraProps.m_tableLevel > m_TableControl.getNestDepth())
			{
				while(m_currentRTFState.m_paraProps.m_tableLevel > m_TableControl.getNestDepth())
				{
					UT_DEBUGMSG(("SEVIOR: Doing itap OpenTable \n"));
					OpenTable();
				}
			}
			else if(m_currentRTFState.m_paraProps.m_tableLevel < m_TableControl.getNestDepth())
			{
				while(m_currentRTFState.m_paraProps.m_tableLevel < m_TableControl.getNestDepth())
				{
					CloseTable();
				}
			}
			UT_DEBUGMSG(("SEVIOR!!! After itap m_tableLevel %d nestDepth %d \n",m_currentRTFState.m_paraProps.m_tableLevel,m_TableControl.getNestDepth()));
			return true;
		}
		break;

	case 'l':
		if (strcmp(reinterpret_cast<char*>(pKeyword), "lquote") == 0)
		{
			return ParseChar(UCS_LQUOTE);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "ldblquote") == 0)
		{
			return ParseChar(UCS_LDBLQUOTE);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "li") == 0)
		{
			m_currentRTFState.m_paraProps.m_indentLeft = param;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "line") == 0)
		{
			return ParseChar(UCS_LF);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "linebetcol") == 0)
		{
			m_currentRTFState.m_sectionProps.m_bColumnLine = true;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "lang") == 0)
		{
			UT_DEBUGMSG(("DOM: lang code (0x%x, %s)\n", param, wvLIDToLangConverter(static_cast<unsigned short>(param))));
			// mark language for spell checking
			m_currentRTFState.m_charProps.m_szLang = wvLIDToLangConverter(static_cast<unsigned short>(param));
			return true;
		}
		else if( strcmp(reinterpret_cast<char*>(pKeyword),"listoverridetable") == 0)
		{
			return ReadListOverrideTable();
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "listtext") == 0)
		{
			// This paragraph is a member of a list.
			SkipCurrentGroup( false);
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "ls") == 0)
		{
			// This paragraph is a member of a list.
			m_currentRTFState.m_paraProps.m_iOverride = static_cast<UT_uint32>(param);
			m_currentRTFState.m_paraProps.m_isList = true;
			return true;
		}
		else if(strcmp(reinterpret_cast<char*>(pKeyword), "landscape") == 0)
		{
//
// Just set landscape mode.
//
			getDoc()->m_docPageSize.setLandscape();
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "ltrpar") == 0)
		{
			xxx_UT_DEBUGMSG(("rtf imp.: ltrpar\n"));
			m_currentRTFState.m_paraProps.m_RTL = false;
			//reset doc bidi attribute
			m_bBidiDocument = false;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "ltrsect") == 0)
		{
			xxx_UT_DEBUGMSG(("rtf imp.: ltrsect\n"));
			m_currentRTFState.m_sectionProps.m_dir = FRIBIDI_TYPE_LTR;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "ltrch") == 0)
		{
			xxx_UT_DEBUGMSG(("rtf imp.: ltrch\n"));
			m_currentRTFState.m_charProps.m_RTL = false;
			m_bBidiDocument = m_currentRTFState.m_charProps.m_RTL ^ m_currentRTFState.m_paraProps.m_RTL;
			return true;
		}
		break;

	case 'm':
		if (strcmp(reinterpret_cast<char *>(pKeyword), "mac") == 0)
		{
			// TODO some iconv's may have a different name - "MacRoman"
			// TODO EncodingManager should handle encoding names
			m_mbtowc.setInCharset("MACINTOSH");
			getDoc()->setEncodingName("MacRoman");
			return true;
		}


		if( strcmp(reinterpret_cast<char *>(pKeyword), "marglsxn") == 0 )
		{
			// Left margin of section
			m_currentRTFState.m_sectionProps.m_leftMargTwips = param;
		}
		else if ( strcmp(reinterpret_cast<char *>(pKeyword), "margl") == 0 )
			{
				m_sectdProps.m_leftMargTwips = param ;
			}

		else if( strcmp(reinterpret_cast<char *>(pKeyword), "margrsxn") == 0 )
		{
			// Right margin of section
			m_currentRTFState.m_sectionProps.m_rightMargTwips = param;
		}
		else if ( strcmp(reinterpret_cast<char *>(pKeyword), "margr") == 0 )
			{
				m_sectdProps.m_rightMargTwips = param;
			}

		else if ( strcmp(reinterpret_cast<char *>(pKeyword), "margtsxn") == 0 )
		{
			// top margin of section
			m_currentRTFState.m_sectionProps.m_topMargTwips = param;
		}
		else if ( strcmp(reinterpret_cast<char *>(pKeyword), "margt") == 0 )
			{
				m_sectdProps.m_topMargTwips = param;
			}

		else if( strcmp(reinterpret_cast<char *>(pKeyword), "margbsxn") == 0 )
		{
			// bottom margin of section
			m_currentRTFState.m_sectionProps.m_bottomMargTwips = param;
		}
		else if( strcmp(reinterpret_cast<char *>(pKeyword), "margb") == 0 )
			{
				m_sectdProps.m_bottomMargTwips = param;
			}
		break;

	case 'n':
		if( strcmp(reinterpret_cast<char *>(pKeyword), "nestrow") == 0 )
		{
			HandleRow();
			return true;
		}
		else if( strcmp(reinterpret_cast<char *>(pKeyword), "nestcell") == 0 )
		{
			UT_DEBUGMSG(("SEVIOR: Processing nestcell \n"));
			HandleCell();
			return true;
		}
		else if( strcmp(reinterpret_cast<char *>(pKeyword), "nonesttables") == 0 )
		{
			//
			// skip this!
			//
			UT_DEBUGMSG(("SEVIOR: doing nonesttables \n"));
			SkipCurrentGroup();
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword),"noproof") == 0)
		{
			// Set language to none for \noproof
			// TODO actually implement proofing flag separate to language setting
			UT_DEBUGMSG(("HIPI: RTF import keyword \\noproof\n"));
			// mark language for spell checking
			m_currentRTFState.m_charProps.m_szLang = "-none-";
			return true;
		}
		break;
	case 'o':
		if (strcmp(reinterpret_cast<char*>(pKeyword),"ol") == 0)
		{
			return HandleOverline(fParam ? (param != 0) : true);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "object") == 0)
		{
			// get picture
			return HandleObject();
		}
		break;
	case 'p':
		if (strcmp(reinterpret_cast<char*>(pKeyword), "par") == 0)
		{
			// start new paragraph, continue current attributes
			return StartNewPara();
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "plain") == 0)
		{
			// reset character attributes
			return ResetCharacterAttributes();
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "pard") == 0)
		{
			// reset paragraph attributes
			bool bres = ResetParagraphAttributes();

			return bres;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "page") == 0)
		{
			return ParseChar(UCS_FF);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "pntext") == 0 )
		{
			//
			// skip this!
			//
			//SkipCurrentGroup( false);
			m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "pict") == 0)
		{
			// get picture
			return HandlePicture();
		}
		else if (strcmp(reinterpret_cast<char *>(pKeyword), "pc") == 0)
		{
			m_mbtowc.setInCharset(XAP_EncodingManager::get_instance()->charsetFromCodepage(437));
			return true;
		}
		else if (strcmp(reinterpret_cast<char *>(pKeyword), "pca") == 0)
		{
			m_mbtowc.setInCharset(XAP_EncodingManager::get_instance()->charsetFromCodepage(850));
			return true;
		}
		else if (strcmp(reinterpret_cast<char *>(pKeyword), "paperw") == 0)
		{
//
// Just set the page width
//
			double height = getDoc()->m_docPageSize.Height(DIM_IN);
			double width = (static_cast<double>(param))/1440.0;
			getDoc()->m_docPageSize.Set(width,height,DIM_IN);
		}
		else if (strcmp(reinterpret_cast<char *>(pKeyword), "paperh") == 0)
		{
//
// Just set the page height
//
			double width = getDoc()->m_docPageSize.Width(DIM_IN);
			double height = (static_cast<double>(param))/1440.0;
			getDoc()->m_docPageSize.Set(width,height,DIM_IN);
		}
		break;
	case 'q':
		if (strcmp(reinterpret_cast<char*>(pKeyword), "ql") == 0)
		{
			return SetParaJustification(RTFProps_ParaProps::pjLeft);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "qc") == 0)
		{
			return SetParaJustification(RTFProps_ParaProps::pjCentre);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "qr") == 0)
		{
			return SetParaJustification(RTFProps_ParaProps::pjRight);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "qj") == 0)
		{
			return SetParaJustification(RTFProps_ParaProps::pjFull);
		}
		break;

	case 'r':
		if (strcmp(reinterpret_cast<char*>(pKeyword), "rquote") == 0)
		{
			return ParseChar(UCS_RQUOTE);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "rdblquote") == 0)
		{
			return ParseChar(UCS_RDBLQUOTE);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "ri") == 0)
		{
			m_currentRTFState.m_paraProps.m_indentRight = param;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "rtf") == 0)
		{
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "rtlpar") == 0)
		{
			xxx_UT_DEBUGMSG(("rtf imp.: rtlpar\n"));
			m_currentRTFState.m_paraProps.m_RTL = true;
			// reset the doc bidi attribute
			m_bBidiDocument = false;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "rtlsect") == 0)
		{
			UT_DEBUGMSG(("rtf imp.: rtlsect\n"));
			m_currentRTFState.m_sectionProps.m_dir = FRIBIDI_TYPE_RTL;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "rtlch") == 0)
		{
			xxx_UT_DEBUGMSG(("rtf imp.: rtlch\n"));
			m_currentRTFState.m_charProps.m_RTL = true;
			m_bBidiDocument = 	m_currentRTFState.m_charProps.m_RTL ^ m_currentRTFState.m_paraProps.m_RTL;
			return true;
		}
		else if(strcmp(reinterpret_cast<char*>(pKeyword), "row") == 0)
		{
			HandleRow();
			return true;
		}
		break;

	case 's':
		if (strcmp(reinterpret_cast<char*>(pKeyword), "s")==0)
		{
			m_currentRTFState.m_paraProps.m_styleNumber = param;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "stylesheet") == 0)
		{
			return HandleStyleDefinition();
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "strike") == 0  ||  strcmp(reinterpret_cast<char*>(pKeyword), "striked") == 0)
		{
			return HandleStrikeout(fParam ? (param != 0) : true);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "sect") == 0 )
		{
			return StartNewSection();
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "sectd") == 0 )
		{
			return ResetSectionAttributes();
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "sa") == 0)
		{
			m_currentRTFState.m_paraProps.m_spaceAfter = param;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "sb") == 0)
		{
			m_currentRTFState.m_paraProps.m_spaceBefore = param;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "sbknone") == 0)
		{
			m_currentRTFState.m_sectionProps.m_breakType = RTFProps_SectionProps::sbkNone;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "sbkcol") == 0)
		{
			m_currentRTFState.m_sectionProps.m_breakType = RTFProps_SectionProps::sbkColumn;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "sbkpage") == 0)
		{
			m_currentRTFState.m_sectionProps.m_breakType = RTFProps_SectionProps::sbkPage;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "sbkeven") == 0)
		{
			m_currentRTFState.m_sectionProps.m_breakType = RTFProps_SectionProps::sbkEven;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "sbkodd") == 0)
		{
			m_currentRTFState.m_sectionProps.m_breakType = RTFProps_SectionProps::sbkOdd;
			return true;
		}
   		else if (strcmp(reinterpret_cast<char*>(pKeyword), "sp") == 0) // Some shape thing!
		{
			UT_DEBUGMSG (("ignoring sp\n"));
			m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "sn") == 0) // Some shape thing!
		{
			UT_DEBUGMSG (("ignoring sn\n"));
			m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "sv") == 0) // Some shape thing!
		{
			UT_DEBUGMSG (("ignoring sp\n"));
			m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "sl") == 0)
		{
			if (!fParam  ||  param == 0)
				m_currentRTFState.m_paraProps.m_lineSpaceVal = 360;
			else
				m_currentRTFState.m_paraProps.m_lineSpaceVal = param;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "slmult") == 0)
		{
			m_currentRTFState.m_paraProps.m_lineSpaceExact = (!fParam  ||  param == 0);
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "super") == 0)
		{
			return HandleSuperscript(fParam ? false : true);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "sub") == 0)
		{
			return HandleSubscript(fParam ? false : true);
		}
		break;

	case 't':
		if (strcmp(reinterpret_cast<char*>(pKeyword), "tab") == 0)
		{
			return ParseChar('\t');
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "tx") == 0)
		{
			UT_return_val_if_fail(fParam, false);	// tabstops should have parameters
			bool bres = AddTabstop(param,
								   m_currentRTFState.m_paraProps.m_curTabType,
								   m_currentRTFState.m_paraProps.m_curTabLeader);
			m_currentRTFState.m_paraProps.m_curTabType = FL_TAB_LEFT;
//			m_currentRTFState.m_paraProps.m_curTabLeader = FL_LEADER_NONE;
			return bres;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "tb") == 0)
		{
			UT_return_val_if_fail(fParam, false);	// tabstops should have parameters

			bool bres = AddTabstop(param,FL_TAB_BAR,
								   m_currentRTFState.m_paraProps.m_curTabLeader);
			m_currentRTFState.m_paraProps.m_curTabType = FL_TAB_BAR;
//			m_currentRTFState.m_paraProps.m_curTabLeader = FL_LEADER_NONE;
			return bres;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "tqr") == 0)
		{
			m_currentRTFState.m_paraProps.m_curTabType = FL_TAB_RIGHT;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "tqc") == 0)
		{
			m_currentRTFState.m_paraProps.m_curTabType = FL_TAB_CENTER;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "tqdec") == 0)
		{
			m_currentRTFState.m_paraProps.m_curTabType = FL_TAB_DECIMAL;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "tldot") == 0)
		{
			m_currentRTFState.m_paraProps.m_curTabLeader = FL_LEADER_DOT;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "tlhyph") == 0)
		{
			m_currentRTFState.m_paraProps.m_curTabLeader = FL_LEADER_HYPHEN;
			return true;
		}
		else if(strcmp(reinterpret_cast<char *>(pKeyword), "trautofit") == 0)
		{
			if(getTable())
			{
				if(param==1)
				{
					getTable()->setAutoFit(true);
				}
			}
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "tlul") == 0)
		{
			m_currentRTFState.m_paraProps.m_curTabLeader = FL_LEADER_UNDERLINE;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "tleq") == 0)
		{
			m_currentRTFState.m_paraProps.m_curTabLeader = FL_LEADER_EQUALSIGN;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "trowd") == 0)
		{
			if(getTable() == NULL)
			{
				OpenTable();
				m_currentRTFState.m_paraProps.m_tableLevel = m_TableControl.getNestDepth();
			}
//
// Look to see if the nesting level of our tables has changed.
//
			if(m_currentRTFState.m_paraProps.m_tableLevel > m_TableControl.getNestDepth())
			{
				UT_DEBUGMSG(("At trowd m_tableLevel %d nestDepth %d \n",m_currentRTFState.m_paraProps.m_tableLevel,m_TableControl.getNestDepth()));
				while(m_currentRTFState.m_paraProps.m_tableLevel > m_TableControl.getNestDepth())
				{
					UT_DEBUGMSG(("SEVIOR: Doing pard OpenTable \n"));
					OpenTable();
				}
				UT_DEBUGMSG(("After trowd m_tableLevel %d nestDepth %d \n",m_currentRTFState.m_paraProps.m_tableLevel,m_TableControl.getNestDepth()));
			}
			else if(m_currentRTFState.m_paraProps.m_tableLevel < m_TableControl.getNestDepth())
			{
				UT_DEBUGMSG(("At trowd m_tableLevel %d nestDepth %d \n",m_currentRTFState.m_paraProps.m_tableLevel,m_TableControl.getNestDepth()));
				while(m_currentRTFState.m_paraProps.m_tableLevel < m_TableControl.getNestDepth())
				{
					UT_DEBUGMSG(("SEVIOR:Close Table trowd1  \n"));
					CloseTable();
					m_bCellBlank = true;
				}
				UT_DEBUGMSG(("After trowd m_tableLevel %d nestDepth %d \n",m_currentRTFState.m_paraProps.m_tableLevel,m_TableControl.getNestDepth()));
			}
//
// Look to see if m_bNestTableProps is true for nested tables.
//
// changed the 1 to 0, because get getNestDepth() returns 0 for
// unnested table -- see ie_Table.cpp (Tomas, May 8, 2003)			
			else if((m_TableControl.getNestDepth() > 1) && !m_bNestTableProps)
			{
				while(m_TableControl.getNestDepth() > 1)
				{
					UT_DEBUGMSG(("SEVIOR:Close Table trowd2 \n"));
					CloseTable();
					m_bCellBlank = true;
				}
				m_currentRTFState.m_paraProps.m_tableLevel = 1;
			}
//
// If a trowd appears without  a preceding \cell we close the previous table
//
			else if(!m_bCellBlank && !m_bNestTableProps)
			{
				UT_DEBUGMSG(("After trowd closing table coz no cell detected\n"));
				CloseTable();
			}
			m_bNestTableProps = false;
			ResetCellAttributes();
			ResetTableAttributes();
		}
		break;

	case 'u':
		if (strcmp(reinterpret_cast<char*>(pKeyword), "ul") == 0        ||  strcmp(reinterpret_cast<char*>(pKeyword), "uld") == 0  ||
			strcmp(reinterpret_cast<char*>(pKeyword), "uldash") == 0    ||  strcmp(reinterpret_cast<char*>(pKeyword), "uldashd") == 0  ||
			strcmp(reinterpret_cast<char*>(pKeyword), "uldashdd") == 0  ||  strcmp(reinterpret_cast<char*>(pKeyword), "uldb") == 0  ||
			strcmp(reinterpret_cast<char*>(pKeyword), "ulth") == 0      ||  strcmp(reinterpret_cast<char*>(pKeyword), "ulw") == 0  ||
			strcmp(reinterpret_cast<char*>(pKeyword), "ulwave") == 0)
		{
			return HandleUnderline(fParam ? (param != 0) : true);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword), "ulnone") == 0)
		{
			return HandleUnderline(0);
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword),"uc") == 0)
		{
			// "\uc<n>" defines the number of chars immediately following
			// any "\u<u>" unicode character that are needed to represent
			// a reasonable approximation for the unicode character.
			// generally, this is done by stripping off accents from latin-n
			// characters so that they fold into latin1.
			//
			// the spec says that we need to allow any arbitrary length
			// of chars for this and that we need to maintain a stack of
			// these lengths (as content is nested within {} groups) so
			// that different 'destinations' can have different approximations
			// or have a local diversion for a hard-to-represent character
			// or something like that.
			//
			// this is bullshit (IMHO) -- jeff

			m_currentRTFState.m_unicodeAlternateSkipCount = param;
			m_currentRTFState.m_unicodeInAlternate = 0;
			return true;
		}
		else if (strcmp(reinterpret_cast<char*>(pKeyword),"u") == 0)
		{
			bool bResult = ParseChar(static_cast<UT_UCSChar>(param));
			m_currentRTFState.m_unicodeInAlternate = m_currentRTFState.m_unicodeAlternateSkipCount;
			return bResult;
		}
		else if (strcmp(reinterpret_cast<char *>(pKeyword),"up") == 0)
		{
			// superscript with position. Default is 6.
			// subscript: see dn keyword
			return HandleSuperscriptPosition (fParam ? param : 6);
		}
		break;

	case '*':
		if (strcmp(reinterpret_cast<char*>(pKeyword), "*") == 0)
		{
			unsigned char keyword_star[MAX_KEYWORD_LEN];
			long parameter_star = 0;
			bool parameterUsed_star = false;

			if (ReadKeyword(keyword_star, &parameter_star, &parameterUsed_star,
							MAX_KEYWORD_LEN))
		    {
				if( strcmp(reinterpret_cast<char*>(keyword_star), "\\")== 0)
				{
					if (ReadKeyword(keyword_star, &parameter_star, &parameterUsed_star,
									MAX_KEYWORD_LEN))
		            {
						if( strcmp(reinterpret_cast<char*>(keyword_star),"ol") == 0)
						{
							return HandleOverline(parameterUsed_star ?
												  (parameter_star != 0): true);
						}
						else if( strcmp(reinterpret_cast<char*>(keyword_star),"pn") == 0)
						{
							return HandleLists( m_currentRTFState.m_paraProps.m_rtfListTable);
						}
						else if( strcmp(reinterpret_cast<char*>(keyword_star),"listtable") == 0)
						{
							return ReadListTable();
						}
						else if( strcmp(reinterpret_cast<char*>(keyword_star),"listoverridetable") == 0)
						{
							return ReadListOverrideTable();
						}
						else if( strcmp(reinterpret_cast<char*>(keyword_star),"abilist") == 0)
						{
							return HandleAbiLists();
						}
						else if( strcmp(reinterpret_cast<char*>(keyword_star),"topline") == 0)
						{
							return HandleTopline(parameterUsed_star ?
												  (parameter_star != 0): true);
						}
						else if( strcmp(reinterpret_cast<char*>(keyword_star),"botline") == 0)
						{
							return HandleBotline(parameterUsed_star ?
												  (parameter_star != 0): true);
						}
						else if( strcmp(reinterpret_cast<char*>(keyword_star),"listtag") == 0)
						{
							return HandleListTag(parameter_star);
						}
						else if (strcmp(reinterpret_cast<char*>(keyword_star),"shppict") == 0)
						{
							UT_DEBUGMSG (("ignoring shppict\n"));
							m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
							return true;
						}
						else if (strcmp(reinterpret_cast<char*>(keyword_star),"shpinst") == 0)
						{
							UT_DEBUGMSG (("ignoring shpinst\n"));
							return true;
						}
						else if (strcmp(reinterpret_cast<char*>(keyword_star),"nesttableprops") == 0)
						{
							UT_DEBUGMSG(("SEVIOR: Doing nestableprops opentable \n"));
							m_bNestTableProps = true;
							// OpenTable();
							return true;
						}
						else if (strcmp(reinterpret_cast<char*>(keyword_star), "bkmkstart") == 0)
						{
							return HandleBookmark (RBT_START);
						}
						else if (strcmp(reinterpret_cast<char*>(keyword_star), "bkmkend") == 0)
						{
							return HandleBookmark (RBT_END);
						}
#if 1
//
// Fixme I need to be able to handle footnotes inside tables in RTF
//
						else if (strcmp(reinterpret_cast<char*>(keyword_star), "footnote") == 0)
						{
							HandleFootnote();
							return true;
						}
#endif
//
// Decode our own field extensions
//
						else if (strstr(reinterpret_cast<char*>(keyword_star), "abifieldD") != NULL)
						{
							char * pszField = strstr(reinterpret_cast<char *>(keyword_star),"D");
							pszField++;
							char * pszAbiField = UT_strdup(pszField);
							char * pszD = strstr(pszAbiField,"D");
							if(pszD)
							{
								*pszD = '_';
								UT_DEBUGMSG(("Appending Abi field %s \n",pszAbiField));
								return _appendField(pszAbiField);
							}
							FREEP(pszAbiField);
						}
						else if (strcmp(reinterpret_cast<char*>(keyword_star), "hlinkbase") == 0)
						{
							m_hyperlinkBase.clear();
							unsigned char ch = 0;

							if(!ReadCharFromFile(&ch))
								return false;

							while (ch != '}')
							{
								m_hyperlinkBase += ch;
								if(!ReadCharFromFile(&ch))
								   return false;
							}
							
							PopRTFState();
							return true;
						}
					}
				}
				UT_DEBUGMSG (("RTF: star keyword %s not handled\n", keyword_star));
		    }

			// Ignore all other \* tags
			// TODO different destination (all unhandled at the moment, so enter skip mode)
			//m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip; was this
			SkipCurrentGroup();
			return true;
		}
		break;
	case '\'':
		if (strcmp(reinterpret_cast<char*>(pKeyword), "\'") == 0)
		{
			m_currentRTFState.m_internalState = RTFStateStore::risHex;
			return true;
		}
		break;
	case '{':
	case '}':
	case '\\':
		ParseChar(*pKeyword);
		return true;
		break;
	case '~':
		ParseChar(UCS_NBSP);
		return true;
		break;
	case '-':
		// TODO handle optional hyphen. Currently simply ignore them.
		xxx_UT_DEBUGMSG (("RTF: TODO handle optionnal hyphen\n"));
		return true;
		break;
	case '_':
		// currently simply make a standard hyphen
		ParseChar('-');	// TODO - make these optional and nonbreaking
		return true;
		break;
	case '\r':	// see bug 2174 ( Cocoa RTF)
	case '\n':
		return StartNewPara();
		break;
	}

	xxx_UT_DEBUGMSG (("RTF: unhandled keyword %s\n", pKeyword));
	return true;
}

bool IE_Imp_RTF::buildCharacterProps(UT_String & propBuffer)
{
	UT_String tempBuffer;
	// bold
	propBuffer += "font-weight:";
	propBuffer += m_currentRTFState.m_charProps.m_bold ? "bold" : "normal";

	// italic
	propBuffer += "; font-style:";
	propBuffer += m_currentRTFState.m_charProps.m_italic ? "italic" : "normal";

	// underline & overline & strike-out
	propBuffer += "; text-decoration:";
	static UT_String decors;
	decors.clear();
	if (m_currentRTFState.m_charProps.m_underline)
	{
		decors += "underline ";
	}
	if (m_currentRTFState.m_charProps.m_strikeout)
	{
		decors += "line-through ";
	}
	if(m_currentRTFState.m_charProps.m_overline)
	{
		decors += "overline ";
	}
	if(m_currentRTFState.m_charProps.m_topline)
	{
		decors += "topline ";
	}
	if(m_currentRTFState.m_charProps.m_botline)
	{
		decors += "bottomline";
	}
	if(!m_currentRTFState.m_charProps.m_underline  &&
	   !m_currentRTFState.m_charProps.m_strikeout &&
	   !m_currentRTFState.m_charProps.m_overline &&
	   !m_currentRTFState.m_charProps.m_topline &&
	   !m_currentRTFState.m_charProps.m_botline)
	{
		decors = "none";
	}
	propBuffer += decors.c_str();

	//superscript and subscript
	propBuffer += "; text-position:";
	if (m_currentRTFState.m_charProps.m_superscript)
	{
		if (m_currentRTFState.m_charProps.m_superscript != 0.0)
		{
			UT_DEBUGMSG (("RTF: TODO: Handle text position in pt.\n"));
		}
		propBuffer += "superscript";
	}
	else if (m_currentRTFState.m_charProps.m_subscript)
	{
		if (m_currentRTFState.m_charProps.m_subscript != 0.0)
		{
			UT_DEBUGMSG (("RTF: TODO: Handle text position in pt.\n"));
		}
		propBuffer += "subscript";
	}
	else
	{
		propBuffer += "normal";
	}
	// font size
	UT_String_sprintf(tempBuffer, "; font-size:%spt", std_size_string(static_cast<float>(m_currentRTFState.m_charProps.m_fontSize)));
	propBuffer += tempBuffer;
	// typeface
	RTFFontTableItem* pFont = GetNthTableFont(m_currentRTFState.m_charProps.m_fontNumber);
	if (pFont != NULL)
	{
		propBuffer += "; font-family:";

		// see bug 2633 - we get a font entry like this (unika.rtf):
		// {\f83\fnil\fcharset0\fprq0{\*\panose 00000000000000000000} ;}
		// note the empty slot after the panose entry
		// later it gets referenced: {\b\f83\fs24\cf1\cgrid0 Malte Cornils
		// this turns those into "Times New Roman" for now, as a hack to keep from crashing
		if ( pFont->m_pFontName != NULL )
			propBuffer += pFont->m_pFontName;
		else
			propBuffer += "Times New Roman";
	}
	if (m_currentRTFState.m_charProps.m_hasColour)
	{
		// colour, only if one has been set. See bug 1324
		UT_uint32 colour = GetNthTableColour(m_currentRTFState.m_charProps.m_colourNumber);
		UT_String_sprintf(tempBuffer, "; color:%06x", colour);
		propBuffer += tempBuffer;
	}

	if (m_currentRTFState.m_charProps.m_hasBgColour)
	{
		// colour, only if one has been set. See bug 1324
		UT_sint32 bgColour = GetNthTableBgColour(m_currentRTFState.m_charProps.m_bgcolourNumber);

		if (bgColour != -1) // invalid and should be white
		{
			UT_String_sprintf(tempBuffer, "; bgcolor:%06x", bgColour);
			propBuffer += tempBuffer;
		}
	}
	if(m_currentRTFState.m_charProps.m_listTag != 0)
	{
// List Tag to hang lists off
		UT_String_sprintf(tempBuffer, "; list-tag:%d",m_currentRTFState.m_charProps.m_listTag);
		propBuffer += tempBuffer;
	}

	if(m_currentRTFState.m_charProps.m_szLang != 0)
	{
		propBuffer += "; lang:";
		propBuffer += m_currentRTFState.m_charProps.m_szLang;
	}

	if(m_currentRTFState.m_charProps.m_dirOverride == FRIBIDI_TYPE_LTR)
	{
		propBuffer += ";dir-override:ltr";
	}
	else if(m_currentRTFState.m_charProps.m_dirOverride == FRIBIDI_TYPE_RTL)
	{
		propBuffer += ";dir-override:rtl";
	}
	
	return true;
}

// in non-bidi doc we just append the current format and text; in bidi
// documents we crawl over the text looking for neutral characters;
// when we find one, we see if the implied override is identical to
// directional properties on either side of the character: if yes, we
// leave the character as it is; if not we issued the override. This
// saves us inserting overrides on most space characters in the document
bool IE_Imp_RTF::_appendSpan()
{
	const XML_Char* pProps = "props";
	const XML_Char* pStyle = PT_STYLE_ATTRIBUTE_NAME;
	UT_String prop_basic;
	buildCharacterProps(prop_basic);

	UT_String prop_ltr = prop_basic;
	prop_ltr += ";dir-override:ltr";
	
	UT_String prop_rtl = prop_basic;
	prop_rtl += ";dir-override:rtl";
	

	const XML_Char* propsArray[5];
	propsArray[0] = pProps;
	propsArray[1] = prop_basic.c_str();
	propsArray[2] = NULL;
	propsArray[3] = NULL;
	propsArray[4] = NULL;

	if(m_currentRTFState.m_charProps.m_styleNumber >= 0
	   && (UT_uint32)m_currentRTFState.m_charProps.m_styleNumber < m_styleTable.size())
	{
		propsArray[2] = pStyle;
		propsArray[3] = static_cast<const char *>(m_styleTable[m_currentRTFState.m_charProps.m_styleNumber]);
	}
	
	UT_UCS4Char * p;
	UT_uint32 iLen = m_gbBlock.getLength();

	if(m_bBidiDocument)
	{
		FriBidiCharType iOverride = FRIBIDI_TYPE_UNSET, cType, cLastType = FRIBIDI_TYPE_UNSET, cNextType;
		UT_uint32 iLast = 0;
		UT_UCS4Char c = *(reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(0)));
	
		cType = fribidi_get_type(c);
	
		for(UT_uint32 i = 0; i < iLen; i++)
		{
			if(i < iLen - 1 )
			{
				c = *(reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(i+1)));
				cNextType = fribidi_get_type(c);
			}
			else
			{
				cNextType = FRIBIDI_TYPE_UNSET;
			}
		
		
			if(FRIBIDI_IS_NEUTRAL(cType))
			{
				if(!m_currentRTFState.m_charProps.m_RTL
				   && iOverride != FRIBIDI_TYPE_LTR
				   && (cLastType != FRIBIDI_TYPE_LTR || cNextType != FRIBIDI_TYPE_LTR))
				{
					if(i - iLast > 0)
					{
						p = reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(iLast));
						if(!getDoc()->appendFmt(propsArray))
							return false;
					
						if(!getDoc()->appendSpan(p, i - iLast))
							return false;
					}
					iOverride = FRIBIDI_TYPE_LTR;
					propsArray[1] = prop_ltr.c_str();
					iLast = i;
				}
				else if(m_currentRTFState.m_charProps.m_RTL
						&& iOverride != FRIBIDI_TYPE_RTL
						&& (cLastType != FRIBIDI_TYPE_RTL || cNextType != FRIBIDI_TYPE_RTL))
				{
					if(i - iLast > 0)
					{
						p = reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(iLast));
						if(!getDoc()->appendFmt(propsArray))
							return false;

						if(!getDoc()->appendSpan(p, i - iLast))
							return false;
					}
					iOverride = FRIBIDI_TYPE_RTL;
					propsArray[1] = prop_rtl.c_str();
					iLast = i;
				}
			}
			else
			{
				// strong character; if we previously issued an override,
				// we need to cancel it
				if(iOverride != FRIBIDI_TYPE_UNSET)
				{
					if(i - iLast > 0)
					{
						p = reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(iLast));
						if(!getDoc()->appendFmt(propsArray))
							return false;
					
						if(!getDoc()->appendSpan(p, i - iLast))
							return false;
					}
					iOverride = FRIBIDI_TYPE_UNSET;
					propsArray[1] = prop_basic.c_str();
					iLast = i;
				}
			}

			cLastType = cType;
			cType = cNextType;
		}

		// insert what is left over
		if(iLen - iLast > 0)
		{
			p = reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(iLast));
			if(!getDoc()->appendFmt(propsArray))
				return false;
					
			if(!getDoc()->appendSpan(p, iLen - iLast))
				return false;
		}
	}
	else
	{
		// not a bidi doc, just do it the simple way
		p = reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(0));
		if(!getDoc()->appendFmt(propsArray))
			return false;
					
		if(!getDoc()->appendSpan(p, iLen))
			return false;
		
	}
	
	return true;
}
	
bool IE_Imp_RTF::_insertSpan()
{
	const XML_Char* pProps = "props";
	const XML_Char* pStyle = PT_STYLE_ATTRIBUTE_NAME;
	UT_String prop_basic;
	buildCharacterProps(prop_basic);

	UT_String prop_ltr = prop_basic;
	prop_ltr += ";dir-override:ltr";
	
	UT_String prop_rtl = prop_basic;
	prop_rtl += ";dir-override:rtl";
	

	const XML_Char* propsArray[5];
	propsArray[0] = pProps;
	propsArray[1] = prop_basic.c_str();
	propsArray[2] = NULL;
	propsArray[3] = NULL;
	propsArray[4] = NULL;
	
	UT_UCS4Char * p;
	UT_uint32 iLen = m_gbBlock.getLength();

	if(m_currentRTFState.m_charProps.m_styleNumber >= 0
	   && (UT_uint32)m_currentRTFState.m_charProps.m_styleNumber < m_styleTable.size())
	{
		propsArray[2] = pStyle;
		propsArray[3] = static_cast<const char *>(m_styleTable[m_currentRTFState.m_charProps.m_styleNumber]);
	}
	
	if(m_bBidiDocument)
	{
		FriBidiCharType iOverride = FRIBIDI_TYPE_UNSET, cType, cLastType = FRIBIDI_TYPE_UNSET, cNextType;
		UT_uint32 iLast = 0;
		UT_UCS4Char c = *(reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(0)));
	
		cType = fribidi_get_type(c);
	
		for(UT_uint32 i = 0; i < iLen; i++)
		{
			if(i < iLen - 1 )
			{
				c = *(reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(i+1)));
				cNextType = fribidi_get_type(c);
			}
			else
			{
				cNextType = FRIBIDI_TYPE_UNSET;
			}
		
		
			if(FRIBIDI_IS_NEUTRAL(cType))
			{
				if(!m_currentRTFState.m_charProps.m_RTL
				   && iOverride != FRIBIDI_TYPE_LTR
				   && (cLastType != FRIBIDI_TYPE_LTR || cNextType != FRIBIDI_TYPE_LTR))
				{
					if(i - iLast > 0)
					{
						p = reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(iLast));
						if(!getDoc()->insertSpan(m_dposPaste, p ,i - iLast))
							return false;
						
						if(!getDoc()->changeSpanFmt(PTC_AddFmt, m_dposPaste, m_dposPaste + i - iLast,
													propsArray,NULL))
							return false;
						
						m_dposPaste += i - iLast;						
					}
					iOverride = FRIBIDI_TYPE_LTR;
					propsArray[1] = prop_ltr.c_str();
					iLast = i;
				}
				else if(m_currentRTFState.m_charProps.m_RTL
						&& iOverride != FRIBIDI_TYPE_RTL
						&& (cLastType != FRIBIDI_TYPE_RTL || cNextType != FRIBIDI_TYPE_RTL))
				{
					if(i - iLast > 0)
					{
						p = reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(iLast));
						if(!getDoc()->insertSpan(m_dposPaste, p ,i - iLast))
							return false;
						
						if(!getDoc()->changeSpanFmt(PTC_AddFmt, m_dposPaste, m_dposPaste + i - iLast,
													propsArray,NULL))
							return false;
						
						m_dposPaste += i - iLast;						
					}
					iOverride = FRIBIDI_TYPE_RTL;
					propsArray[1] = prop_rtl.c_str();
					iLast = i;
				}
			}
			else
			{
				// strong character; if we previously issued an override,
				// we need to cancel it
				if(iOverride != FRIBIDI_TYPE_UNSET)
				{
					if(i - iLast > 0)
					{
						p = reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(iLast));
						if(!getDoc()->insertSpan(m_dposPaste, p ,i - iLast))
							return false;
						
						if(!getDoc()->changeSpanFmt(PTC_AddFmt, m_dposPaste, m_dposPaste + i - iLast,
													propsArray,NULL))
							return false;
						
						m_dposPaste += i - iLast;						
					}
					iOverride = FRIBIDI_TYPE_UNSET;
					propsArray[1] = prop_basic.c_str();
					iLast = i;
				}
			}

			cLastType = cType;
			cType = cNextType;
		}

		// insert what is left over
		if(iLen - iLast > 0)
		{
			p = reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(iLast));
			if(!getDoc()->insertSpan(m_dposPaste, p ,iLen - iLast))
				return false;
						
			if(!getDoc()->changeSpanFmt(PTC_AddFmt, m_dposPaste, m_dposPaste + iLen - iLast,
										propsArray,NULL))
				return false;
						
			m_dposPaste += iLen - iLast;						
		}
	}
	else
	{
		// not a bidi doc, just do it the simple way
		p = reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(0));
		if(!getDoc()->insertSpan(m_dposPaste, p ,iLen))
			return false;
						
		if(!getDoc()->changeSpanFmt(PTC_AddFmt, m_dposPaste, m_dposPaste + iLen,
									propsArray,NULL))
			return false;
						
		m_dposPaste += iLen;						
	}
	
	return true;
}

bool IE_Imp_RTF::ApplyCharacterAttributes()
{
	bool ok;
	if(m_gbBlock.getLength() > 0)
	{
		if ((m_pImportFile) || (m_parsingHdrFtr))	// if we are reading from a file or parsing headers and footers
		{
			ok = _appendSpan();
		}
		else								// else we are pasting from a buffer
		{
			ok = _insertSpan();
		}
		m_gbBlock.truncate(0);
		return ok;
	}
	else
	{
		const XML_Char* pProps = "props";
		const XML_Char* pStyle = PT_STYLE_ATTRIBUTE_NAME;
		UT_String propBuffer;
		buildCharacterProps(propBuffer);

		const XML_Char* propsArray[5];
		propsArray[0] = pProps;
		propsArray[1] = propBuffer.c_str();
		propsArray[2] = NULL;
		propsArray[3] = NULL;
		propsArray[4] = NULL;

		if(m_currentRTFState.m_charProps.m_styleNumber >= 0
		   && (UT_uint32)m_currentRTFState.m_charProps.m_styleNumber < m_styleTable.size())
		{
			propsArray[2] = pStyle;
			propsArray[3] = static_cast<const char *>(m_styleTable[m_currentRTFState.m_charProps.m_styleNumber]);
		}
		
		if ((m_pImportFile) || (m_parsingHdrFtr))	// if we are reading from a file or parsing headers and footers
		{
			ok = getDoc()->appendFmt(propsArray);
			ok = ok && getDoc()->appendFmtMark();
		}
		else								// else we are pasting from a buffer
		{
			ok = getDoc()->changeSpanFmt(PTC_AddFmt,
												m_dposPaste,m_dposPaste,
												propsArray,NULL);
		}
		return ok;
	}
}


bool IE_Imp_RTF::ResetCharacterAttributes()
{
	bool ok = FlushStoredChars();

	m_currentRTFState.m_charProps = RTFProps_CharProps();

	return ok;
}

/*!
 *    OK if we are pasting into the text we have to decide if the list we paste
 *    should be a new list or an old list. The user might want to swap paragraphs
 *    in a list for example.

 *    Use the following algorithim to decide. If the docpos of the paste is
 *    within a list of the same ID as our list or if the docpos is immediately
 *    before or after a list of the same ID reuse the ID. Otherwise change it.
*/
UT_uint32 IE_Imp_RTF::mapID(UT_uint32 id)
{
	UT_uint32 mappedID = id;
	if(id == 0)
	{
		UT_ASSERT_NOT_REACHED();
		return id;
	}
	if (m_pImportFile)  // if we are reading a file - dont remap the ID
	{
	        return id;
	}
//
// Handle case of no id in any lists. If this is the case no need to remap
//
	fl_AutoNum * pAuto = getDoc()->getListByID(id);
	if(pAuto == NULL)
	{
	        return id;
	}
	///
	/// Now look to see if the ID has been remapped.
	///
	UT_uint32 i,j;
	for(i=0; i<m_numLists; i++)
	{
		if(getAbiList(i)->orig_id == id)
		{
			if(getAbiList(i)->hasBeenMapped == true )
			{
				mappedID =  getAbiList(i)->mapped_id;
			}
			else
			    ///
			    /// Do the remapping!
			    ///
			{
				fl_AutoNum * pMapAuto = NULL;
				UT_uint32 nLists = getDoc()->getListsCount();
				UT_uint32 highestLevel = 0;
				PL_StruxDocHandle sdh;
				getDoc()->getStruxOfTypeFromPosition(m_dposPaste, PTX_Block,&sdh);
				for(j=0; j< nLists; j++)
				{
					fl_AutoNum * pAuto = getDoc()->getNthList(j);
					if(pAuto->isContainedByList(sdh) == true)
					{
						if(highestLevel < pAuto->getLevel())
						{
							highestLevel = pAuto->getLevel();
							pMapAuto = pAuto;
						}
					}
				}
				if(pMapAuto == NULL )
					mappedID = UT_rand();
				else if( getAbiList(i)->level <= pMapAuto->getLevel() && pMapAuto->getID() != 0)
					mappedID = pMapAuto->getID();
				else
					mappedID = UT_rand();
				getAbiList(i)->hasBeenMapped = true;
				getAbiList(i)->mapped_id = mappedID;
				if(highestLevel > 0)
				{
					getAbiList(i)->mapped_parentid =  getAbiList(i)->orig_parentid;
				}
				else
				{
					getAbiList(i)->mapped_parentid = 0;
					getAbiList(i)->orig_parentid = 0;
					getAbiList(i)->level = 1;
				}
			}

			///
			/// Now look to see if the parent ID has been remapped, if so update mapped_parentid
			///
			for(j = 0;  j<m_numLists; j++)
			{
				if(getAbiList(j)->orig_id == getAbiList(i)->orig_parentid)
				{
					getAbiList(i)->mapped_parentid = getAbiList(j)->mapped_id;
				}
			}

		}
	}
	return mappedID;

}

/*!
 *   OK if we are pasting into the text we have to decide if the list we paste
 *   should be a new list or an old list. The user might want to swap paragraphs
 *   for example.
 */
UT_uint32 IE_Imp_RTF::mapParentID(UT_uint32 id)
{
  //
  // For the parent ID we have to look to see if the parent ID has been
  // remapped or if the id
  //
	UT_uint32 mappedID;
	mappedID = id;
	if (m_pImportFile)  // if we are reading a file
	{
		return id;
	}
	UT_uint32 i;
	for(i=0; i<m_numLists ; i++)
	{
		if(getAbiList(i)->orig_id == id)
			break;
	}
	if( i < m_numLists && getAbiList(i)->orig_id == id)
	{
	    mappedID =  getAbiList(i)->mapped_id;
	}
	return mappedID;
}

bool IE_Imp_RTF::ApplyParagraphAttributes()
{
	const XML_Char* attribs[PT_MAX_ATTRIBUTES*2 + 1];
	UT_uint32 attribsCount=0;

//
// Look to see if the nesting level of our tables has changed.
//
	if(m_pImportFile)
	{
		if(m_currentRTFState.m_paraProps.m_tableLevel > m_TableControl.getNestDepth())
		{
			if(m_bParaWrittenForSection)
			{
				UT_DEBUGMSG(("At Apply Paragraph m_tableLevel %d nestDepth %d \n",m_currentRTFState.m_paraProps.m_tableLevel,m_TableControl.getNestDepth()));
				while(m_currentRTFState.m_paraProps.m_tableLevel > m_TableControl.getNestDepth())
				{
					UT_DEBUGMSG(("SEVIOR: Doing pard OpenTable \n"));
					m_bCellBlank = false;
					m_bEndTableOpen = false;
					OpenTable();
				}
				UT_DEBUGMSG(("After Apply Paragraph m_tableLevel %d nestDepth %d \n",m_currentRTFState.m_paraProps.m_tableLevel,m_TableControl.getNestDepth()));
			}
		}
		else if(m_currentRTFState.m_paraProps.m_tableLevel < m_TableControl.getNestDepth())
		{
			UT_DEBUGMSG(("At Apply Paragraph m_tableLevel %d nestDepth %d \n",m_currentRTFState.m_paraProps.m_tableLevel,m_TableControl.getNestDepth()));
			while(m_currentRTFState.m_paraProps.m_tableLevel < m_TableControl.getNestDepth())
			{
				CloseTable();
				if(m_bCellBlank)
				{
					m_bEndTableOpen = true;
				}
			}
			UT_DEBUGMSG(("After Apply Paragraph m_tableLevel %d nestDepth %d \n",m_currentRTFState.m_paraProps.m_tableLevel,m_TableControl.getNestDepth()));
		}
	}
	m_bParaWrittenForSection = true;


	UT_String propBuffer;
	UT_String tempBuffer;
	bool bWord97List = m_currentRTFState.m_paraProps.m_isList && isWord97Lists();
	bool bAbiList = m_currentRTFState.m_paraProps.m_isList && ( 0 != m_currentRTFState.m_paraProps.m_rawID);
	bWord97List = bWord97List && !bAbiList;
	RTF_msword97_listOverride * pOver = NULL;
	UT_uint32 iLevel = 0;
	UT_uint32 iOverride = 0;
//
// Need to get some pointers to add List tabs to the tab definitions.
//
	if(bWord97List)
	{
		iOverride = m_currentRTFState.m_paraProps.m_iOverride;
		iLevel = m_currentRTFState.m_paraProps.m_iOverrideLevel;
		pOver = _getTableListOverride(iOverride);
	}

	// tabs
	if ((pOver != NULL && pOver->isTab(iLevel)))
	{
//
// The Word 97 RTF list definition has some extra tab stops. Add them here.
//
		if(pOver->isTab(iLevel))
		{
			UT_uint32 i = 0;
			UT_uint32 count = pOver->getTabStopVect(iLevel)->getItemCount();
			for(i=0; i< count; i++)
			{
				m_currentRTFState.m_paraProps.m_tabStops.addItem(pOver->getTabStopVect(iLevel)->getNthItem(i));
				m_currentRTFState.m_paraProps.m_tabTypes.addItem(pOver->getTabTypeVect(iLevel)->getNthItem(i));
				m_currentRTFState.m_paraProps.m_tabLeader.addItem(pOver->getTabLeaderVect(iLevel)->getNthItem(i));
			}
		}
	}
	if(m_currentRTFState.m_paraProps.m_tabStops.getItemCount() > 0)
	{
		UT_ASSERT(m_currentRTFState.m_paraProps.m_tabStops.getItemCount() ==
					m_currentRTFState.m_paraProps.m_tabTypes.getItemCount() );
		UT_ASSERT(m_currentRTFState.m_paraProps.m_tabStops.getItemCount() ==
					m_currentRTFState.m_paraProps.m_tabLeader.getItemCount() );
		propBuffer += "tabstops:";
		for (UT_uint32 i = 0; i < m_currentRTFState.m_paraProps.m_tabStops.getItemCount(); i++)
		{
			if (i > 0)
				propBuffer += ",";

			UT_sint32 tabTwips = reinterpret_cast<UT_sint32>(m_currentRTFState.m_paraProps.m_tabStops.getNthItem(i));
			double tabIn = tabTwips/(20.0*72.);
			UT_uint32 idum = reinterpret_cast<UT_uint32>(m_currentRTFState.m_paraProps.m_tabTypes.getNthItem(i));
			eTabType tabType = static_cast<eTabType>(idum);
			idum = reinterpret_cast<UT_uint32>((m_currentRTFState.m_paraProps.m_tabLeader.getNthItem(i)));
			eTabLeader tabLeader = static_cast<eTabLeader>(idum);
			char  cType = ' ';
			switch(tabType)
			{
			case FL_TAB_LEFT:
				cType ='L';
				break;
			case FL_TAB_RIGHT:
				cType ='R';
				break;
			case FL_TAB_CENTER:
				cType ='C';
				break;
			case FL_TAB_DECIMAL:
				cType ='D';
				break;
			case FL_TAB_BAR:
				cType ='B';
				break;
			default:
				UT_ASSERT_NOT_REACHED();
			}
			char cLeader = '0' + static_cast<char>(tabLeader);
			UT_String_sprintf(tempBuffer, "%s/%c%c", UT_convertInchesToDimensionString(DIM_IN,tabIn,"04"),cType,cLeader);
			propBuffer += tempBuffer;
		}

		propBuffer += "; ";
	}

	// justification
	propBuffer += "text-align:";
	switch (m_currentRTFState.m_paraProps.m_justification)
	{
		case RTFProps_ParaProps::pjCentre:
			propBuffer += "center";
			break;
		case RTFProps_ParaProps::pjRight:
			propBuffer += "right";
			break;
		case RTFProps_ParaProps::pjFull:
			propBuffer += "justify";
			break;
		default:
			UT_ASSERT_NOT_REACHED();	// so what is it?
		case RTFProps_ParaProps::pjLeft:
			propBuffer += "left";
			break;
	}
	propBuffer += "; ";

	// indents - first, left and right, top and bottom
	UT_String_sprintf(tempBuffer, "margin-top:%s; ",		UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(m_currentRTFState.m_paraProps.m_spaceBefore)/1440));
	propBuffer += tempBuffer;
	UT_String_sprintf(tempBuffer, "margin-bottom:%s; ",	UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(m_currentRTFState.m_paraProps.m_spaceAfter)/1440));
	propBuffer += tempBuffer;

	propBuffer += "dom-dir:";
	if(m_currentRTFState.m_paraProps.m_RTL)
		propBuffer += "rtl; ";
	else
		propBuffer += "ltr; ";

	//
	// Filled from List deefinition
	//
	if(!bWord97List || bAbiList)
	{
		UT_String_sprintf(tempBuffer, "margin-left:%s; ", UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(m_currentRTFState.m_paraProps.m_indentLeft)/1440));
		propBuffer += tempBuffer;
	}
	UT_String_sprintf(tempBuffer, "margin-right:%s; ", UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(m_currentRTFState.m_paraProps.m_indentRight)/1440));
	propBuffer += tempBuffer;
//
// Filled from List definition
//
	if(!bWord97List || bAbiList)
	{
		UT_String_sprintf(tempBuffer, "text-indent:%s; ", UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(m_currentRTFState.m_paraProps.m_indentFirst)/1440));
		propBuffer += tempBuffer;
	}
	// line spacing
	if (m_currentRTFState.m_paraProps.m_lineSpaceExact)
	{
		// ABIWord doesn't (yet) support exact line spacing we'll just fall back to single
		UT_String_sprintf(tempBuffer, "line-height:1.0;");
	}
	else
	{
		UT_String_sprintf(tempBuffer, "line-height:%s;",	UT_convertToDimensionlessString(fabs(m_currentRTFState.m_paraProps.m_lineSpaceVal/240)));
	}

	propBuffer += tempBuffer;


	// Lists. If the paragraph has a list element handle it.
	UT_String szLevel;
	UT_String szStyle;
	UT_String szListID;
	UT_String szParentID;
	UT_uint32 id = 0,pid = 0,startValue = 0;
//
// This is for our own extensions to RTF.
//
	if( bAbiList )
	{
	  //
	  // First off assemble the list attributes
	  //
		id = mapID(m_currentRTFState.m_paraProps.m_rawID);
		UT_String_sprintf(szListID,"%d",id);
		pid = mapParentID(m_currentRTFState.m_paraProps.m_rawParentID);
		UT_String_sprintf(szParentID,"%d",pid);
		if(pid == 0)
			m_currentRTFState.m_paraProps.m_level = 1;
		UT_String_sprintf(szLevel,"%d",m_currentRTFState.m_paraProps.m_level);

		attribs[attribsCount++] = PT_LISTID_ATTRIBUTE_NAME;
		attribs[attribsCount++] = szListID.c_str();
		attribs[attribsCount++] = PT_PARENTID_ATTRIBUTE_NAME;
		attribs[attribsCount++] = szParentID.c_str();
		attribs[attribsCount++] = PT_LEVEL_ATTRIBUTE_NAME;
		attribs[attribsCount++] = szLevel.c_str();
		attribs[attribsCount] = NULL;
	}

//
// This is for Word97 Lists
//
	if(bWord97List && pOver)
	{
//
// Now get the properties we've painstakingly put together.
//
		const char * szListID = NULL;
		const char * szParentID = NULL;
		const char * szLevel = NULL;
		const char * szStartat = NULL;
		const char * szFieldFont = NULL;
		const char * szListDelim = NULL;
		const char * szListDecimal = NULL;
		const char * szAlign = NULL;
		const char * szIndent = NULL;
		const char * szListStyle = NULL;
		pOver->buildAbiListProperties( &szListID, &szParentID, &szLevel, &szStartat, &szFieldFont,
									   &szListDelim, &szListDecimal, &szAlign, &szIndent,
									   &szListStyle, iLevel);

//
// fix up indents.
//
		UT_String val;
		double firstLine = UT_convertToInches(szAlign);
		double leftIndent = -firstLine + 0.01;
		if(szIndent && *szIndent)
		{
			leftIndent = UT_convertToInches(szIndent);
		}
		else
		{
			val =  UT_formatDimensionedValue(leftIndent,"in");
			szIndent = val.c_str();
		}
		if((firstLine + leftIndent) < 0.0)
		{
			leftIndent = -firstLine +0.01;
			val =  UT_formatDimensionedValue(leftIndent,"in");
			szIndent = val.c_str();
		}
//
// Got attributes
//
		attribs[attribsCount++] = PT_LISTID_ATTRIBUTE_NAME;
		attribs[attribsCount++] = szListID;
		attribs[attribsCount++] = PT_PARENTID_ATTRIBUTE_NAME;
		attribs[attribsCount++] = szParentID;
		attribs[attribsCount++] = PT_LEVEL_ATTRIBUTE_NAME;
		attribs[attribsCount++] = szLevel;
		attribs[attribsCount]   = NULL;

//
// Next do character properties redefined in this list
//
		// bold
		if(pOver->isBoldChanged(iLevel))
		{
			propBuffer += "font-weight:";
			if ( pOver->getBold(iLevel) )
				propBuffer +=  "bold";
			else
				propBuffer += "normal";
			propBuffer += ";";
		}
		// italic
		if(pOver->isItalicChanged(iLevel))
		{
			propBuffer += " font-style:";
			if ( pOver->getItalic(iLevel) )
				propBuffer += "italic";
			else
				propBuffer += "normal";
			propBuffer += ";";
		}
		// underline & overline & strike-out
		if(pOver->isUnderlineChanged(iLevel) || pOver->isStrikeoutChanged(iLevel))
		{
			propBuffer += "; text-decoration:";
			static UT_String decors;
			decors.clear();
			if (pOver->getUnderline(iLevel))
			{
				decors += "underline ";
			}
			if (pOver->getStrikeout(iLevel))
			{
				decors += "line-through ";
			}
			if(!pOver->getUnderline(iLevel)  &&
			   !pOver->getStrikeout(iLevel))
			{
				decors = "none";
			}
			propBuffer += decors.c_str();
			propBuffer += ";";
		}
		//superscript and subscript
		if(pOver->isSuperscriptChanged(iLevel) || pOver->isSubscriptChanged(iLevel))
		{
			propBuffer += " text-position:";
			if (pOver->getSuperscript(iLevel))
			{
				if (pOver->getSuperscriptPos(iLevel) != 0.0)
				{
					UT_DEBUGMSG (("RTF: TODO: Handle text position in pt.\n"));
				}
				propBuffer += "superscript;";
			}
			else if (pOver->getSubscript(iLevel))
			{
				if (pOver->getSubscriptPos(iLevel) != 0.0)
				{
					UT_DEBUGMSG (("RTF: TODO: Handle text position in pt.\n"));
				}
				propBuffer += "subscript;";
			}
			else
			{
				propBuffer += "normal;";
			}
		}

		// font size
		if(pOver->isFontSizeChanged(iLevel))
		{
			UT_String_sprintf(tempBuffer, " font-size:%spt;", std_size_string(static_cast<float>(pOver->getFontSize(iLevel))));
			propBuffer += tempBuffer;
			UT_DEBUGMSG(("RTF: IMPORT!!!!! font sized changed in override %d \n",pOver->getFontSize(iLevel)));
		}
		// typeface
		if(pOver->isFontNumberChanged(iLevel))
		{
			RTFFontTableItem* pFont = GetNthTableFont(pOver->getFontNumber(iLevel));
			if (pFont != NULL)
			{
				propBuffer += " font-family:";
				propBuffer += pFont->m_pFontName;
				propBuffer += ";";
			}
		}
		// Foreground Colour
		if(pOver->isHasColourChanged(iLevel))
		{
			if (pOver->getHasColour(iLevel))
			{
				// colour, only if one has been set. See bug 1324
				UT_uint32 colour = GetNthTableColour(pOver->getColourNumber(iLevel));
				UT_String_sprintf(tempBuffer, " color:%06x;", colour);
				propBuffer += tempBuffer;
			}
		}
		// BackGround Colour
		if (pOver->isHasBgColourChanged(iLevel))
		{
			if(pOver->getHasBgColour(iLevel))
			{
				// colour, only if one has been set. See bug 1324
				UT_sint32 bgColour = GetNthTableBgColour(pOver->getBgColourNumber(iLevel));
				if (bgColour != -1) // invalid and should be white
				{
					UT_String_sprintf(tempBuffer, " bgcolor:%06x;", bgColour);
					propBuffer += tempBuffer;
				}
			}
		}
		//
		// Now handle the List properties
		//

		UT_String_sprintf(tempBuffer,"list-style:%s;",szListStyle);
		propBuffer += tempBuffer;
		UT_String_sprintf(tempBuffer, "list-decimal:%s; ",szListDecimal);
		propBuffer += tempBuffer;
		UT_String_sprintf(tempBuffer, "list-delim:%s; ",szListDelim);
		propBuffer += tempBuffer;
		UT_String_sprintf(tempBuffer, "field-font:%s; ",szFieldFont);
		propBuffer += tempBuffer;
		UT_String_sprintf(tempBuffer, "start-value:%s; ",szStartat);
		propBuffer += tempBuffer;
		UT_String_sprintf(tempBuffer, "margin-left:%s; ",szAlign);
		propBuffer += tempBuffer;
		UT_String_sprintf(tempBuffer, "text-indent:%s;", szIndent); // Note last entry has no ;
		propBuffer += tempBuffer;
	}


	if(bAbiList)
	{
		//
		// Now handle the Abi List properties
		//
		UT_String_sprintf(tempBuffer,"list-style:%s;",m_currentRTFState.m_paraProps.m_pszStyle);
		UT_String_sprintf(szStyle,"%s",m_currentRTFState.m_paraProps.m_pszStyle);
		propBuffer += tempBuffer;
		UT_String_sprintf(tempBuffer, "list-decimal:%s; ",m_currentRTFState.m_paraProps.m_pszListDecimal);
		propBuffer += tempBuffer;
		UT_String_sprintf(tempBuffer, "list-delim:%s; ",m_currentRTFState.m_paraProps.m_pszListDelim);
		propBuffer += tempBuffer;
		UT_String_sprintf(tempBuffer, "field-font:%s; ",m_currentRTFState.m_paraProps.m_pszFieldFont);
		propBuffer += tempBuffer;
		startValue = m_currentRTFState.m_paraProps.m_startValue;
		UT_String_sprintf(tempBuffer, "start-value:%d",startValue);
		propBuffer += tempBuffer;
	}
	// Style name
	if( static_cast<UT_uint32>(m_currentRTFState.m_paraProps.m_styleNumber) < m_styleTable.size() &&(m_currentRTFState.m_paraProps.m_styleNumber >= 0) )
	{
		UT_uint32 styleNumber = m_currentRTFState.m_paraProps.m_styleNumber;
		const char * styleName = static_cast<const char *>(m_styleTable[styleNumber]);
		attribs[attribsCount++] = PT_STYLE_ATTRIBUTE_NAME;
		attribs[attribsCount++] = styleName;
		attribs[attribsCount]   = NULL;
	}
//
// If there are character properties defined now write them into our buffer
//

//
// Remove the trailing ";" if needed.
//
	UT_sint32 eol = propBuffer.size();
	while(eol >= 0 && (propBuffer[eol] == ' ' || propBuffer[eol] == 0))
	{
		eol--;
	}
	if(propBuffer[eol] == ';')
	{
		propBuffer[eol] = 0;
	}
	attribs[attribsCount++] = PT_PROPS_ATTRIBUTE_NAME;
//
// if we are reading a file or parsing header and footers
// and we're in a list, append char props to this.
//
	if ( ((m_pImportFile) || (m_parsingHdrFtr)) && (bAbiList || bWord97List ))
	{
		buildCharacterProps(propBuffer);
		UT_DEBUGMSG(("SEVIOR: propBuffer = %s \n",propBuffer.c_str()));
	}
	attribs[attribsCount++] = propBuffer.c_str();
	attribs[attribsCount++] = NULL;

	if ((m_pImportFile) || (m_parsingHdrFtr)) // if we are reading a file or parsing header and footers
	{
		if(bAbiList || bWord97List )
		{
			bool bret = getDoc()->appendStrux(PTX_Block, attribs);
			m_bEndTableOpen = false;
			m_bCellBlank = false;
			getDoc()->appendFmtMark();
			//
			// Insert a list-label field??
			//
			const XML_Char* fielddef[3];
			fielddef[0] ="type";
			fielddef[1] = "list_label";
			fielddef[2] = NULL;
			bret =   getDoc()->appendObject(PTO_Field,fielddef);
			UT_UCSChar cTab = UCS_TAB;
//
// Put the tab back in.
//
			getDoc()->appendSpan(&cTab,1);
			return bret;
		}
		else
		{
			xxx_UT_DEBUGMSG(("SEVIOR: Apply Para's append strux \n"));
			bool ok = getDoc()->appendStrux(PTX_Block, attribs);
			m_bEndTableOpen = false;
			m_bCellBlank = false;
			return ok;
		}
	}
	else
	{
		bool bSuccess = true;
		if(bAbiList && (m_pImportFile == NULL))
		{
			bSuccess = getDoc()->insertStrux(m_dposPaste,PTX_Block);
			m_dposPaste++;
			//
			// Put the tab back in.
			//
			UT_UCSChar cTab = UCS_TAB;
			getDoc()->insertSpan(m_dposPaste,&cTab,1);
			m_dposPaste++;
			PL_StruxDocHandle sdh_cur;
			UT_uint32 j;
			fl_AutoNum * pAuto = getDoc()->getListByID(id);
			if(pAuto == NULL)
			/*
			* Got to create a new list here.
			* Old one may have been cut out or ID may have
			* been remapped.
			*/
			{
				List_Type lType = NOT_A_LIST;
				fl_AutoLists al;
				UT_uint32 size_xml_lists = al.getXmlListsSize();
				for(j=0; j< size_xml_lists; j++)
				{
					if( UT_XML_strcmp(szStyle.c_str(),al.getXmlList(j)) ==0)
					{
						break;
					}
				}
				if(j < size_xml_lists)
					lType = static_cast<List_Type>(j);
				else
					lType = static_cast<List_Type>(0);
				pAuto = new fl_AutoNum(id, pid, lType, startValue,static_cast<XML_Char *>(m_currentRTFState.m_paraProps.m_pszListDelim),static_cast<XML_Char *>(m_currentRTFState.m_paraProps.m_pszListDecimal), getDoc());
				getDoc()->addList(pAuto);
				pAuto->fixHierarchy();
			}
			bSuccess = getDoc()->getStruxOfTypeFromPosition(m_dposPaste,PTX_Block,&sdh_cur);
			///
			/// Now insert this into the pAuto List
			///
			pAuto->addItem(sdh_cur);
			if(pid != 0)
			{
				pAuto->findAndSetParentItem();
				pAuto->markAsDirty();
			}
			bSuccess = getDoc()->changeStruxFmt(PTC_AddFmt,m_dposPaste,m_dposPaste,attribs, NULL,PTX_Block);
		}
		else if(m_pImportFile == NULL)
		{
			bSuccess = getDoc()->insertStrux(m_dposPaste,PTX_Block);
			m_dposPaste++;
			bSuccess = getDoc()->changeStruxFmt(PTC_AddFmt,m_dposPaste,m_dposPaste, attribs,NULL,PTX_Block);
			//
			// Now check if this strux has associated list element. If so stop the list!
			//
			PL_StruxDocHandle sdh = NULL;
			getDoc()->getStruxOfTypeFromPosition(m_dposPaste,PTX_Block,&sdh);
			UT_uint32 nLists = getDoc()->getListsCount();
			bool bisListItem = false;
			//
			// Have to loop so that multi-level lists get stopped. Each StopList removes
			// the sdh from the next highest level.
			//
			do
			{
				fl_AutoNum * pAuto = NULL;
				bisListItem = false;
				for(UT_uint32 i=0; (i< nLists && !bisListItem); i++)
				{
					pAuto = getDoc()->getNthList(i);
					if(pAuto)
						bisListItem = pAuto->isItem(sdh);
				}
				//
				// We've created a list element where we should not. Stop it now!!
				//
				if(bisListItem)
				{
					UT_DEBUGMSG(("SEVIOR: Stopping list at %x \n",sdh));
					getDoc()->StopList(sdh);
				}
			}
			while(bisListItem);
		}
		return bSuccess;
	}
}


bool IE_Imp_RTF::ResetCellAttributes(void)
{
	bool ok = FlushStoredChars();
	m_currentRTFState.m_cellProps = RTFProps_CellProps();
	return ok;
}


bool IE_Imp_RTF::ResetTableAttributes(void)
{
	bool ok = FlushStoredChars();
	m_currentRTFState.m_tableProps = RTFProps_TableProps();
	return ok;
}

bool IE_Imp_RTF::ResetParagraphAttributes()
{
	UT_DEBUGMSG(("Reset Para Attributes \n"));
	bool ok = FlushStoredChars();
	m_currentRTFState.m_paraProps = RTFProps_ParaProps();

	return ok;
}


bool IE_Imp_RTF::ResetSectionAttributes()
{
	bool ok = FlushStoredChars();

	// not quite correct. a sectd will reset the section defaults
	// to the previously acquired page defaults

	// margr, margl, margt, margb, paperh, gutter

	m_currentRTFState.m_sectionProps = m_sectdProps ;
	m_bParaWrittenForSection = false;
	return ok;
}


bool IE_Imp_RTF::ApplySectionAttributes()
{
	XML_Char* pProps = "props";
	UT_String propBuffer;
	UT_String tempBuffer;
	UT_String szHdrID;
	UT_String szFtrID;
	UT_String szHdrEvenID;
	UT_String szFtrEvenID;
	UT_String szHdrFirstID;
	UT_String szFtrFirstID;
	UT_String szHdrLastID;
	UT_String szFtrLastID;
	short paramIndex = 0;

	UT_DEBUGMSG (("Applying SectionAttributes\n"));

	// columns
	UT_String_sprintf(tempBuffer, "columns:%d", m_currentRTFState.m_sectionProps.m_numCols);
	propBuffer += tempBuffer;

	if (m_currentRTFState.m_sectionProps.m_bColumnLine)
	{
		propBuffer += "; column-line:on";
	}
	{
		switch (m_currentRTFState.m_sectionProps.m_breakType) {
		case RTFProps_SectionProps::sbkNone:
//			propBuffer += "; ";
			break;
		case RTFProps_SectionProps::sbkColumn:
			break;
		case RTFProps_SectionProps::sbkPage:
			break;
		case RTFProps_SectionProps::sbkEven:
			break;
		case RTFProps_SectionProps::sbkOdd:
			break;
		default:
			UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
		}
	}
	if(true /*m_currentRTFState.m_sectionProps.m_leftMargTwips != 0*/)
	{
		propBuffer += "; page-margin-left:";
		double inch = static_cast<double>(m_currentRTFState.m_sectionProps.m_leftMargTwips)/1440.;
		UT_String sinch;
		char * old_locale = setlocale(LC_NUMERIC, "C");
		UT_String_sprintf(sinch,"%fin",inch);
		setlocale(LC_NUMERIC, old_locale);
		propBuffer += sinch;
	}
	if(true /*m_currentRTFState.m_sectionProps.m_rightMargTwips != 0*/)
	{
		propBuffer += "; page-margin-right:";
		double inch = static_cast<double>(m_currentRTFState.m_sectionProps.m_rightMargTwips)/1440.;
		UT_String sinch;
		char * old_locale = setlocale(LC_NUMERIC, "C");
		UT_String_sprintf(sinch,"%fin",inch);
		setlocale(LC_NUMERIC, old_locale);
		propBuffer += sinch;
	}
	if(true /*m_currentRTFState.m_sectionProps.m_topMargTwips != 0*/)
	{
		propBuffer += "; page-margin-top:";
		double inch = static_cast<double>(m_currentRTFState.m_sectionProps.m_topMargTwips)/1440.;
		UT_String sinch;
		char * old_locale = setlocale(LC_NUMERIC, "C");
		UT_String_sprintf(sinch,"%fin",inch);
		setlocale(LC_NUMERIC, old_locale);
		propBuffer += sinch;
	}
	if(true /*m_currentRTFState.m_sectionProps.m_bottomMargTwips != 0*/)
	{
		propBuffer += "; page-margin-bottom:";
		double inch = static_cast<double>(m_currentRTFState.m_sectionProps.m_bottomMargTwips)/1440.;
		UT_String sinch;
		char * old_locale = setlocale(LC_NUMERIC, "C");
		UT_String_sprintf(sinch,"%fin",inch);
		setlocale(LC_NUMERIC, old_locale);
		propBuffer += sinch;
	}
	if(true /*m_currentRTFState.m_sectionProps.m_colSpaceTwips != 0*/)
	{
		propBuffer += "; column-gap:";
		double inch = static_cast<double>(m_currentRTFState.m_sectionProps.m_colSpaceTwips)/1440.;
		UT_String sinch;
		char * old_locale = setlocale(LC_NUMERIC, "C");
		UT_String_sprintf(sinch,"%fin",inch);
		setlocale(LC_NUMERIC, old_locale);
		propBuffer += sinch;
	}
	if(m_currentRTFState.m_sectionProps.m_headerYTwips != 0)
	{
		UT_sint32 sheader = 0;
//
// The RTF spec is to define a fixed height for the header. We calculate
// the header height as Top margin - header margin.
//
// So the header margin = topmargin - header height.
//
		if(m_currentRTFState.m_sectionProps.m_topMargTwips != 0)
		{
			sheader = m_currentRTFState.m_sectionProps.m_headerYTwips;
			if(sheader < 0)
			{
				sheader = 0;
			}
		}
		propBuffer += "; page-margin-header:";
		double inch = static_cast<double>(sheader)/1440.;
		UT_String sinch;
		char * old_locale = setlocale(LC_NUMERIC, "C");
		UT_String_sprintf(sinch,"%fin",inch);
		setlocale(LC_NUMERIC, old_locale);
		propBuffer += sinch;
	}
	if(m_currentRTFState.m_sectionProps.m_footerYTwips != 0)
	{
		UT_sint32 sfooter = 0;
//
// The RTF spec is to define a fixed height for the footer. We calculate
// the footer height as Bottom margin - footer margin.
//
// So the footer margin = bottom margin - footer height.
//
		if(m_currentRTFState.m_sectionProps.m_bottomMargTwips != 0)
		{
			sfooter = m_currentRTFState.m_sectionProps.m_bottomMargTwips - m_currentRTFState.m_sectionProps.m_footerYTwips;
			if(sfooter < 0)
			{
				sfooter = 0;
			}
		}
		propBuffer += "; page-margin-footer:";
		double inch = static_cast<double>(sfooter)/1440.;
		UT_String sinch;
		char * old_locale = setlocale(LC_NUMERIC, "C");
		UT_String_sprintf(sinch,"%fin",inch);
		setlocale(LC_NUMERIC, old_locale);
		propBuffer += sinch;
	}
	UT_DEBUGMSG(("SEVIOR: propBuffer = %s \n",propBuffer.c_str()));

	if(m_currentRTFState.m_sectionProps.m_dir != FRIBIDI_TYPE_UNSET)
	{
		const char r[] = "rtl";
		const char l[] = "ltr";
		const char ar[] = "right";
		const char al[] = "left";
		const char * d, * a;
		if(m_currentRTFState.m_sectionProps.m_dir == FRIBIDI_TYPE_RTL)
		{
			d = r;
			a = ar;
		}
		else
		{
			d = l;
			a = al;
		}

		UT_String_sprintf(tempBuffer, "; dom-dir:%s; text-align:%s",d,a);
        propBuffer += tempBuffer;
        UT_DEBUGMSG(("Apply sect prop: [%s]\n", tempBuffer.c_str()));
	}

	UT_DEBUGMSG(("SEVIOR: propBuffer = %s \n",propBuffer.c_str()));

	const XML_Char* propsArray[15];
	propsArray[0] = pProps;
	propsArray[1] = propBuffer.c_str();
	paramIndex = 2;
	if (m_currentHdrID != 0)
	{
		UT_DEBUGMSG (("Applying header\n"));
		propsArray [paramIndex] = "header";
		paramIndex++;
		UT_String_sprintf (szHdrID, "hdr%u", m_currentHdrID);
		propsArray [paramIndex] = szHdrID.c_str();
		paramIndex++;
	}
	if (m_currentHdrEvenID != 0)
	{
		UT_DEBUGMSG (("Applying header even\n"));
		propsArray [paramIndex] = "header-even";
		paramIndex++;
		UT_String_sprintf (szHdrEvenID, "hdrevn%u", m_currentHdrEvenID);
		propsArray [paramIndex] = szHdrEvenID.c_str();
		paramIndex++;
	}
	if (m_currentHdrFirstID != 0)
	{
		UT_DEBUGMSG (("Applying header first\n"));
		propsArray [paramIndex] = "header-first";
		paramIndex++;
		UT_String_sprintf (szHdrFirstID, "hdrfst%u", m_currentHdrFirstID);
		propsArray [paramIndex] = szHdrFirstID.c_str();
		paramIndex++;
	}
	if (m_currentHdrLastID != 0)
	{
		UT_DEBUGMSG (("Applying header last\n"));
		propsArray [paramIndex] = "header-last";
		paramIndex++;
		UT_String_sprintf (szHdrLastID, "hdrlst%u", m_currentHdrLastID);
		propsArray [paramIndex] = szHdrLastID.c_str();
		paramIndex++;
	}
	if (m_currentFtrID != 0)
	{
		UT_DEBUGMSG (("Applying footer\n"));
		propsArray [paramIndex] = "footer";
		paramIndex++;
		UT_String_sprintf (szFtrID, "ftr%u", m_currentFtrID);
		propsArray [paramIndex] = szFtrID.c_str();
		paramIndex++;
	}
	if (m_currentFtrEvenID != 0)
	{
		UT_DEBUGMSG (("Applying footer even\n"));
		propsArray [paramIndex] = "footer-even";
		paramIndex++;
		UT_String_sprintf (szFtrEvenID, "ftrevn%u", m_currentFtrEvenID);
		propsArray [paramIndex] = szFtrEvenID.c_str();
		paramIndex++;
	}
	if (m_currentFtrFirstID != 0)
	{
		UT_DEBUGMSG (("Applying footer first\n"));
		propsArray [paramIndex] = "footer-first";
		paramIndex++;
		UT_String_sprintf (szFtrFirstID, "ftrfst%u", m_currentFtrFirstID);
		propsArray [paramIndex] = szFtrFirstID.c_str();
		paramIndex++;
	}
	if (m_currentFtrLastID != 0)
	{
		UT_DEBUGMSG (("Applying footer last\n"));
		propsArray [paramIndex] = "footer-last";
		paramIndex++;
		UT_String_sprintf (szFtrLastID, "ftrlst%u", m_currentFtrLastID);
		propsArray [paramIndex] = szFtrLastID.c_str();
		paramIndex++;
	}
	UT_ASSERT (paramIndex < 15);
	propsArray [paramIndex] = NULL;

	if ((m_pImportFile) || (m_parsingHdrFtr)) // if we are reading a file or parsing a header and footer
	{
		return getDoc()->appendStrux(PTX_Section, propsArray);
	}
	else
	{
		// Add a block before the section so there's something content
		// can be inserted into.
		bool bSuccess = getDoc()->insertStrux(m_dposPaste,PTX_Block);

		if (bSuccess)
		{
			bSuccess = getDoc()->insertStrux(m_dposPaste,PTX_Section);
			if (bSuccess)
			{
				m_dposPaste++;
				bSuccess = getDoc()->changeStruxFmt(PTC_AddFmt,m_dposPaste,m_dposPaste,
													   propsArray,NULL,PTX_Section);
			}
		}
		return bSuccess;
	}
}

//////////////////////////////////////////////////////////////////////////////
// List Table reader
/////////////////////////////////////////////////////////////////////////////

/*!
 * This is a general purpose parameter reader. It returns the value of a keyword
 * surrounded by a brace. For a construct of the form...
 * {\mehere fred}
 *         ^
 *         Current read point.
 * It will return fred and swallow the closing brace.
 * For a construct of the form
 * {\mehere fred;}
 *         ^
 *         Current read point.
 * It will return fred and swallow the closing brace and semicolon.
 * For a construct of the form
 * {\mehere {\key1 fred;} {\key2 fred2} {\key3 {\key4 fred}}}
 *         ^
 *         Current read point.
 * It will return {\key1 fred;} {\key2 fred2} {\key3 {\key4 fred}}  and
 * swallow the closing brace.
 * returns NULL on error.
 */
char * IE_Imp_RTF::getCharsInsideBrace(void)
{
	unsigned static char keyword[MAX_KEYWORD_LEN];
	unsigned char ch;

	// OK scan through the text until a closing delimeter is
	// found

	UT_sint32 count = 0;
	UT_uint32 nesting = 1;
	while(nesting > 0)
	{
		if (!ReadCharFromFile(&ch))
			return NULL;
        if( nesting == 1 && (ch == '}'  || ch == ';'))
		{
			nesting--;
		}
		else
		{
			if(ch == '{')
			{
				nesting++;
			}
			if(ch == '}')
			{
				nesting--;
			}
			keyword[count++] = ch;
		}
	}
	if(ch == ';')
	{
//
// Swallow closing brace if ";}" finishes
//
		if (!ReadCharFromFile(&ch))
			return NULL;
//
// if character is not a '}' put it back in the input stream.
//
		if(ch != '}')
		{
			SkipBackChar(ch);
		}
	}
	keyword[count++] = 0;
	return reinterpret_cast<char*>(&keyword[0]);
}


bool IE_Imp_RTF::ReadListTable()
{
//
// Ensure the list tables are empty to start.
//
	UT_VECTOR_PURGEALL(RTF_msword97_list*, m_vecWord97Lists);
	unsigned char keyword[MAX_KEYWORD_LEN];
	unsigned char ch;
	long parameter = 0;
	bool paramUsed = false;
	UT_uint32 nesting = 1;
	while (nesting >0) // Outer loop
	{
		if (!ReadCharFromFile(&ch))
		{
			return false;
		}
		if(ch == '{')  //new list or listoverride?
		{
			if (!ReadCharFromFile(&ch))
			{
				return false;
			}
			if(!ReadKeyword(keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN))
			{
				return false;
			}
			if (strcmp(reinterpret_cast<char*>(&keyword[0]), "list") == 0)
			{
				if(!HandleTableList())
					return false;
			}
		}
		else if(ch == '}')
		{
			nesting--;
		}
	}
	return true;
}

/*!
 * This method parses out the \list item in a list tabledefinition.
 */
bool IE_Imp_RTF::HandleTableList(void)
{
	unsigned char keyword[MAX_KEYWORD_LEN];
	unsigned char ch;
	long parameter = 0;
	bool paramUsed = false;
	UT_uint32 nesting = 1;
	UT_uint32 levelCount = 0;
//
// Increment list counting vector
//
	RTF_msword97_list * pList = new  RTF_msword97_list(this);
	m_vecWord97Lists.addItem(static_cast<void *>(pList));
//
// OK Parse this \list
//
	while(nesting > 0) // Outer loop
	{
		if (!ReadCharFromFile(&ch))
			return false;
		if(ch == '{')  // listlevel
		{
			if (!ReadCharFromFile(&ch))
			{
				return false;
			}
			if(!ReadKeyword(keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN))
			{
				return false;
			}
			if(strcmp(reinterpret_cast<char*>(&keyword[0]),"listlevel") == 0)
			{
				HandleListLevel(pList,levelCount);
				levelCount++;
			}
			else if(strcmp(reinterpret_cast<char*>(&keyword[0]),"listid") == 0)
			{
				pList->m_RTF_listID = static_cast<UT_uint32>(parameter);
			}
			else
			{
				char * szLevelText = getCharsInsideBrace();
				if(!szLevelText)
					return false;
			}
		}
		else if(ch == '}')
		{
			nesting--;
		}
		else
		{
			if(!ReadKeyword(keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN))
			{
				return false;
			}
			if(strcmp(reinterpret_cast<char*>(&keyword[0]),"listtemplateid") == 0)
			{
				pList->m_RTF_listTemplateID = parameter;
			}
			else if(strcmp(reinterpret_cast<char*>(&keyword[0]),"listid") == 0)
			{
				pList->m_RTF_listID = static_cast<UT_uint32>(parameter);
			}
			else
			{
				UT_DEBUGMSG(("SEVIOR: keyword %s found and ignored in listtable definition \n",keyword));
			}
		}
	}
	return true;
}

/*!
 * This method parses out the list table definition of listlevel's
 */
bool IE_Imp_RTF::HandleListLevel(RTF_msword97_list * pList, UT_uint32 levelCount  )
{
	unsigned char keyword[MAX_KEYWORD_LEN];
	unsigned char ch;
	long parameter = 0;
	bool paramUsed = false;
	UT_uint32 nesting = 1;
	UT_String szLevelNumbers;
	UT_String szLevelText;
//
// OK define this in the data structure.
//
	RTF_msword97_level * pLevel = new RTF_msword97_level(pList, levelCount);
    RTFProps_ParaProps * pParas =  new RTFProps_ParaProps();
	RTFProps_CharProps *  pChars = new	RTFProps_CharProps();
    RTFProps_bParaProps * pbParas =  new RTFProps_bParaProps();
	RTFProps_bCharProps *  pbChars = new	RTFProps_bCharProps();
	pLevel->m_pParaProps = pParas;
	pLevel->m_pCharProps = pChars;
	pLevel->m_pbParaProps = pbParas;
	pLevel->m_pbCharProps = pbChars;
	pList->m_RTF_level[levelCount] = pLevel;
#if 1 // Sevior use this!! The other method can lead to inccorect results upon
	// import. If we export RTF list ID starting at 10000 they might clash
    // with these later.
	pLevel->m_AbiLevelID = UT_rand();
	while(pLevel->m_AbiLevelID < 10000)
		pLevel->m_AbiLevelID = UT_rand();
#else
	pLevel->m_AbiLevelID = pLevel->m_sLastAssignedLevelID++;
#endif
	while(nesting > 0)
	{
		if (!ReadCharFromFile(&ch))
			return false;
		if(ch == '{')  // levelnumber and leveltext
		{
			if (!ReadCharFromFile(&ch))
				return false;
			if(!ReadKeyword(keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN))
			{
				return false;
			}
			if(strcmp(reinterpret_cast<char*>(&keyword[0]),"levelnumbers") == 0)
			{
				szLevelNumbers = getCharsInsideBrace();
			}
			else if(strcmp(reinterpret_cast<char*>(&keyword[0]),"leveltext") == 0)
			{
				szLevelText = getCharsInsideBrace();
			}
			else
			{
				getCharsInsideBrace();
			}
		}
		else if(ch == '}')  // Probabally finished here.
		{
			nesting--;
		}
		else
		{
			if(!ReadKeyword(keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN))
			{
				return false;
			}
			if(strcmp(reinterpret_cast<char*>(&keyword[0]),"levelnfc") == 0) // RTF list Type
			{
				pLevel->m_RTFListType = static_cast<UT_uint32>(parameter);
			}
			else if(strcmp(reinterpret_cast<char*>(&keyword[0]),"levelnfcn") == 0)  // Not in my docs
			{
			}
			else if(strcmp(reinterpret_cast<char*>(&keyword[0]),"leveljc") == 0) // Justification
			{
			}
			else if(strcmp(reinterpret_cast<char*>(&keyword[0]),"leveljcn") == 0) // Not in my docs
			{
			}
			else if(strcmp(reinterpret_cast<char*>(&keyword[0]),"levelfollow") == 0) // Tab following
			{
				switch (parameter)
				{
				case 0: // Tab
					pLevel->m_cLevelFollow = '\t';
					break;
				case 1: // Space
					pLevel->m_cLevelFollow = ' ';
					break;
				case 2: // Nothing
					pLevel->m_cLevelFollow = '\0';
					break;
				default:
					UT_ASSERT_NOT_REACHED();
					break;
				}
			}
			else if(strcmp(reinterpret_cast<char*>(&keyword[0]),"levelstartat") == 0)
			{
				pLevel->m_levelStartAt = static_cast<UT_uint32>(parameter);
			}
			else if(strcmp(reinterpret_cast<char*>(&keyword[0]),"levelspace") == 0) // ignore
			{
			}
			else if(strcmp(reinterpret_cast<char*>(&keyword[0]),"levelindent") == 0) // ignore
			{
			}
			else if(strcmp(reinterpret_cast<char*>(&keyword[0]), "levelnorestart") ==0)
			{
				pLevel->m_bRestart = (parameter == 1);
			}
//
// OK parse againgst all the character and allowed paragraph properties.
//
			else
			{
				if(!ParseCharParaProps(static_cast<unsigned char *>(keyword), parameter, paramUsed,pChars,pParas,pbChars,pbParas))
					return false;
			}
		}
	}
	if(pLevel->m_RTFListType != 23)
	{
		pLevel->ParseLevelText(szLevelText,szLevelNumbers, levelCount);
	}
	else
	{
		pLevel->m_listDelim = "%L";
		if(strstr(szLevelText.c_str(),"u-3913") != 0)
		{
			pLevel->m_RTFListType = 23; // Bulleted List
		}
		if(strstr(szLevelText.c_str(),"u-3880") != 0)
		{
			pLevel->m_RTFListType = 23 + IMPLIES_LIST; // IMPLIES List
		}
	}
	return true;
}

/*!
 * OK this method parses the RTF against all the character and paragraph properties.
 * and fills the pointers to the character and paragraph classes.
 * These are used by the list table
 * reader.
 */
bool IE_Imp_RTF::ParseCharParaProps( unsigned char * pKeyword, long param, bool fParam, RTFProps_CharProps * pChars, RTFProps_ParaProps * pParas, RTFProps_bCharProps * pbChars, RTFProps_bParaProps * pbParas)
{
	if (strcmp(reinterpret_cast<char*>(pKeyword), "b") == 0) // bold
	{
		pbChars->bm_bold = true;
		pChars->m_bold = fParam ? false : true;
		return true;
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "cf") == 0) // color
	{
		pChars->m_hasColour = true;
		pbChars->bm_hasColour = true;
		pbChars->bm_colourNumber = true;
		pChars->m_colourNumber = static_cast<UT_uint32>(param);
		return true;
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "cb") == 0) // background color
	{
		pbChars->bm_bgcolourNumber = true;
		return HandleU32CharacterProp(static_cast<UT_uint32>(param), &(pChars->m_bgcolourNumber));
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "deleted") == 0) // deleted
	{
		pbChars->bm_deleted = true;
		return HandleBoolCharacterProp(fParam ? false : true, &(pChars->m_deleted));
	}
	else if (strcmp(reinterpret_cast<char *>(pKeyword),"dn") == 0) // subscript with position
	{
		// subscript with position. Default is 6.
		// superscript: see up keyword
		bool ok;
		UT_uint32 pos = (UT_uint32) (fParam ? param : 6);
		ok = HandleBoolCharacterProp((pos != 0) ? true : false, &(pChars->m_superscript));
		if (ok)
		{
			pbChars->bm_superscript_pos = true;
			ok = HandleFloatCharacterProp (pos*0.5, &(pChars->m_superscript_pos));
		}
		return ok;
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "fs") == 0)
	{
		pbChars->bm_fontSize = true;
		return HandleFloatCharacterProp ((fParam ? param : 24)*0.5, &(pChars->m_fontSize));
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "f") == 0)
	{
		UT_uint32 fontNumber = (UT_uint32) (fParam ? param : 0);
		RTFFontTableItem* pFont = GetNthTableFont(fontNumber);
		if (pFont != NULL && pFont->m_szEncoding)
			m_mbtowc.setInCharset(pFont->m_szEncoding);

		pbChars->bm_fontNumber = true;
		return HandleU32CharacterProp(fontNumber, &(pChars->m_fontNumber));
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "fi") == 0)
	{
		pParas->m_indentFirst = param;
		pbParas->bm_indentFirst = true;
		return true;
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "i") == 0)
	{
		// italic - either on or off depending on the parameter
		pbChars->bm_italic = true;
		return HandleBoolCharacterProp((fParam ? false : true), &(pChars->m_italic));
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "li") == 0)
	{
		pbParas->bm_indentLeft = true;
		pParas->m_indentLeft = param;
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "listtag") == 0)
	{
		pbChars->bm_listTag = true;
		pChars->m_listTag = param;
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword),"ol") == 0)
	{
		pbChars->bm_overline = true;
		return HandleBoolCharacterProp((fParam ? (param != 0) : true), &(pChars->m_overline));
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "ql") == 0)
	{
		pbParas->bm_justification = true;
		pParas->m_justification = RTFProps_ParaProps::pjLeft;
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "qc") == 0)
	{
		pbParas->bm_justification = true;
		pParas->m_justification = RTFProps_ParaProps::pjCentre;
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "qr") == 0)
	{
		pbParas->bm_justification = true;
		pParas->m_justification = RTFProps_ParaProps::pjRight;
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "qj") == 0)
	{
		pbParas->bm_justification = true;
		pParas->m_justification = RTFProps_ParaProps::pjFull;
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "ri") == 0)
	{
		pbParas->bm_indentRight = true;
		pParas->m_indentRight = param;
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "strike") == 0  ||  strcmp(reinterpret_cast<char*>(pKeyword), "striked") == 0)
	{
		pbChars->bm_strikeout = true;
		return HandleBoolCharacterProp((fParam ? (param != 0) : true), &(pChars->m_strikeout));
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "sa") == 0)
	{
		pbParas->bm_spaceAfter = true;
		pParas->m_spaceAfter = param;
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "sb") == 0)
	{
		pbParas->bm_spaceBefore = true;
		pParas->m_spaceBefore = param;
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "sl") == 0)
	{
		pbParas->bm_lineSpaceVal = true;
		if (!fParam  ||  param == 0)
		{
			pParas->m_lineSpaceVal = 360;
		}
		else
		{
			pParas->m_lineSpaceVal = param;
		}
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "slmult") == 0)
	{
		pbParas->bm_lineSpaceExact = true;
		pParas->m_lineSpaceExact = (!fParam  ||  param == 0);
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "super") == 0)
	{
		pbChars->bm_superscript = true;
		return HandleBoolCharacterProp((fParam ? false : true), &(pChars->m_superscript));
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "sub") == 0)
	{
		pbChars->bm_subscript = true;
		return HandleBoolCharacterProp((fParam ? false : true), &(pChars->m_subscript));
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "tx") == 0)
	{
		UT_return_val_if_fail(fParam, false);	// tabstops should have parameters
		bool bres = AddTabstop(param,
							   pParas->m_curTabType,
							   pParas->m_curTabLeader,pParas);
		pParas->m_curTabType = FL_TAB_LEFT;
		pParas->m_curTabLeader = FL_LEADER_NONE;
		pbParas->bm_curTabType = true;
		pbParas->bm_curTabLeader = true;
		return bres;
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "tb") == 0)
	{
		UT_return_val_if_fail(fParam, false);	// tabstops should have parameters

		bool bres = AddTabstop(param,FL_TAB_BAR,
							   pParas->m_curTabLeader,pParas);
		pParas->m_curTabType = FL_TAB_LEFT;
		pParas->m_curTabLeader = FL_LEADER_NONE;
		pbParas->bm_curTabType = true;
		pbParas->bm_curTabLeader = true;
		return bres;
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "jclisttab") == 0)
	{
		UT_DEBUGMSG(("SEVIOR: jclisttab found ignore for now \n"));
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "tqr") == 0)
	{
		pbParas->bm_curTabType = true;
		pParas->m_curTabType = FL_TAB_RIGHT;
		return true;
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "tqc") == 0)
	{
		pbParas->bm_curTabType = true;
		pParas->m_curTabType = FL_TAB_CENTER;
		return true;
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "tqdec") == 0)
	{
		pbParas->bm_curTabType = true;
		pParas->m_curTabType = FL_TAB_DECIMAL;
		return true;
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "tldot") == 0)
	{
		pbParas->bm_curTabLeader = true;
		pParas->m_curTabLeader = FL_LEADER_DOT;
		return true;
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "tlhyph") == 0)
	{
		pbParas->bm_curTabLeader = true;
		pParas->m_curTabLeader = FL_LEADER_HYPHEN;
		return true;
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "tlul") == 0)
	{
		pbParas->bm_curTabLeader = true;
		pParas->m_curTabLeader = FL_LEADER_UNDERLINE;
		return true;
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "tleq") == 0)
	{
		pbParas->bm_curTabLeader = true;
		pParas->m_curTabLeader = FL_LEADER_EQUALSIGN;
		return true;
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "ul") == 0        ||  strcmp(reinterpret_cast<char*>(pKeyword), "uld") == 0  ||
			 strcmp(reinterpret_cast<char*>(pKeyword), "uldash") == 0    ||  strcmp(reinterpret_cast<char*>(pKeyword), "uldashd") == 0  ||
			 strcmp(reinterpret_cast<char*>(pKeyword), "uldashdd") == 0  ||  strcmp(reinterpret_cast<char*>(pKeyword), "uldb") == 0  ||
			 strcmp(reinterpret_cast<char*>(pKeyword), "ulth") == 0      ||  strcmp(reinterpret_cast<char*>(pKeyword), "ulw") == 0  ||
			 strcmp(reinterpret_cast<char*>(pKeyword), "ulwave") == 0)
	{
		pbChars->bm_underline = true;
		return HandleBoolCharacterProp((fParam ? (param != 0) : true), &(pChars->m_underline));
	}
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "ulnone") == 0)
	{
		pbChars->bm_underline = true;
		return HandleBoolCharacterProp(false, &(pChars->m_underline));
	}
	else if (strcmp(reinterpret_cast<char *>(pKeyword),"up") == 0)
	{
		// superscript with position. Default is 6.
		// subscript: see dn keyword
		bool ok;
		UT_uint32 pos = (UT_uint32) (fParam ? param : 6);
		pbChars->bm_superscript = true;
		pChars->m_superscript = (pos != 0) ? true : false ;
		pbChars->bm_superscript_pos = true;
		ok = HandleFloatCharacterProp (pos*0.5, &(pChars->m_superscript_pos));
		return ok;
	}
	return true;
}




bool IE_Imp_RTF::ReadListOverrideTable(void)
{
//
// Ensure the list tables are empty to start.
//
	UT_VECTOR_PURGEALL(RTF_msword97_listOverride*, m_vecWord97ListOverride);
	unsigned char keyword[MAX_KEYWORD_LEN];
	unsigned char ch;
	long parameter = 0;
	bool paramUsed = false;
	UT_uint32 nesting = 1;
	while (nesting >0) // Outer loop
	{
		if (!ReadCharFromFile(&ch))
			return false;
		if(ch == '{')  //new list or listoverride?
		{
			if (!ReadCharFromFile(&ch))
				return false;
			if(!ReadKeyword(keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN))
			{
				return false;
			}
			if (strcmp(reinterpret_cast<char*>(&keyword[0]), "listoverride") == 0)
			{
				if(!HandleTableListOverride())
				{
					return false;
				}
			}
		}
		else if(ch == '}')
		{
			nesting--;
		}
	}
	return true;
}

/*!
  Get list override of given id
  \param id Id of list override
  \return List override or NULL if not found

  The old code in ApplyParagraphAttributes would use the given
  id as an index to the vector of list overrides. But these
  can be given arbitrary ids from 1 to 2000, so the code
  would not always have worked. Also, and more relevant,
  this function handles an id of 0, as output by 
  StarWriter/OpenOffice
  even though it is not allowed in the spec.
*/
RTF_msword97_listOverride*
IE_Imp_RTF::_getTableListOverride(UT_uint32 id)
{
	UT_uint32 i;
	RTF_msword97_listOverride* pLOver;

	for (i = 0; i < m_vecWord97ListOverride.size(); i++)
	{
		pLOver = static_cast<RTF_msword97_listOverride *>(m_vecWord97ListOverride.getNthItem(i));
		if (id == pLOver->m_RTF_listID)
		{
			return pLOver;
		}
	}

	// Client requested a list override that was not defined.
	UT_ASSERT_NOT_REACHED();
	return NULL;
}

bool IE_Imp_RTF::HandleTableListOverride(void)
{
	unsigned char keyword[MAX_KEYWORD_LEN];
	unsigned char ch;
	long parameter = 0;
	bool paramUsed = false;
//
// OK define this in the data structure.
//
	RTF_msword97_listOverride * pLOver = new  RTF_msword97_listOverride(this);
//
// Increment override counting vector
//
	m_vecWord97ListOverride.addItem(static_cast<void *>(pLOver));
    RTFProps_ParaProps * pParas =  new RTFProps_ParaProps();
	RTFProps_CharProps *  pChars = new	RTFProps_CharProps();
    RTFProps_bParaProps * pbParas =  new RTFProps_bParaProps();
	RTFProps_bCharProps *  pbChars = new	RTFProps_bCharProps();
	pLOver->m_pParaProps = pParas;
	pLOver->m_pCharProps = pChars;
	pLOver->m_pbParaProps = pbParas;
	pLOver->m_pbCharProps = pbChars;

	UT_uint32 nesting = 1;
	while (nesting >0) // Outer loop
	{
		if (!ReadCharFromFile(&ch))
		{
			return false;
		}
		if(ch == '}')
		{
			nesting--;
		}
		else if(ch == '{')
		{
			nesting++;
		}
		else if(ch == '\\')
		{
			if(!ReadKeyword(keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN))
			{
				return false;
			}
			if(strcmp(reinterpret_cast<char*>(&keyword[0]),"listid") == 0)
			{
				pLOver->m_RTF_listID = static_cast<UT_uint32>(parameter);
				if(!pLOver->setList())
				{
					return false;
				}
			}
			else if(strcmp(reinterpret_cast<char*>(&keyword[0]),"listoverridecount")==0)
			{
				UT_DEBUGMSG(("SEVIOR: Found list listoverride count. Ignore for now\n"));
			}
			else if(strcmp(reinterpret_cast<char*>(&keyword[0]),"ls")== 0)
			{
				pLOver->m_RTF_listID = static_cast<UT_uint32>(parameter);
			}
		    else
			{
				ParseCharParaProps(reinterpret_cast<unsigned char *>(keyword), parameter,paramUsed,pChars,pParas,pbChars,pbParas);
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// Font table reader
//////////////////////////////////////////////////////////////////////////////

// Reads the RTF font table, storing it for future use
//
bool IE_Imp_RTF::ReadFontTable()
{
	// Ensure the font table is empty before we start
	if (m_fontTable.getItemCount() != 0) {
		UT_DEBUGMSG (("Font table already contains %d items !\n", m_fontTable.getItemCount()));
	}

// don't purge the vector as we may have several fonttable....
// just handle dupes as seen below.

//	UT_VECTOR_PURGEALL(RTFFontTableItem*, m_fontTable);

	unsigned char ch;
	if (!ReadCharFromFile(&ch))
		return false;

	if (ch == '\\')
	{
		SkipBackChar(ch);
		// one entry in the font table
		// TODO - Test one item font tables!
		if (!ReadOneFontFromTable())
			return false;
	}
	else
	{
		if ((ch != '{') && (ch != '}'))     // don't choke if there is no data (malformed RTF)
		{                                   // see bug 1383 and 1384
			return false;
		}

		// multiple entries in font table
		while (ch != '}')
		{
			if (ch != '{')
			{
					return false;
			}

			if (!ReadOneFontFromTable())
				return false;

			// now eat whitespace until we hit either '}' (end of font group) or '{' (another font item)
			do
			{
				if (!ReadCharFromFile(&ch))
					return false;
			} while (ch != '}'  &&  ch != '{');
		}
	}

	// Put the close group symbol back into the input stream
	return SkipBackChar(ch);
}


// Reads one font item from the font table in the RTF font table.  When called
// the file must be at the f of the 'f' (fontnum) keyword.
// Our life is made easier as the order of items in the table is specified.
//
bool IE_Imp_RTF::ReadOneFontFromTable()
{
	unsigned char keyword[MAX_KEYWORD_LEN];
	unsigned char ch;
	long parameter = 0;
	bool paramUsed = false;

	int nesting = 0;

	// run though the item reading in these values
	RTFFontTableItem::FontFamilyEnum fontFamily = RTFFontTableItem::ffNone;
	RTFFontTableItem::FontPitch pitch = RTFFontTableItem::fpDefault;
	UT_uint32 fontIndex = 0;
	int charSet = 0;
	int codepage = 0;
	unsigned char panose[10];
	memset(panose, 0, sizeof(unsigned char));
	char* pFontName = NULL;
	char* pAlternativeFontName = NULL;
	RTFTokenType tokenType;

	//TODO - this should be intialized once for the whole RTF reader.
	UT_StringPtrMap keywordMap;
	keywordMap.insert("fcharset",&charSet);
	keywordMap.insert("cpg",&codepage);
	//TODO - handle the other keywords
	int * pValue;

	tokenType = NextToken(keyword,&parameter,&paramUsed,MAX_KEYWORD_LEN,true);
	if (tokenType != RTF_TOKEN_KEYWORD || (strcmp(reinterpret_cast<char*>(&keyword[0]), "f") != 0))
	{
		return false;
	}
	else
	{
		fontIndex = parameter;
	}

    // ignore white space here to work around some broken docs. See 2719

	tokenType = NextToken(keyword,&parameter,&paramUsed,MAX_KEYWORD_LEN,true);
	if (tokenType != RTF_TOKEN_KEYWORD)
	{
		// Read the font family (must be specified), but we'll let it slide...
		// tokenType will be RTF_TOKEN_DATA, so we'll bypass the following while() loop
		fontFamily = RTFFontTableItem::ffNone;
	}
	else
	{
		if (strcmp(reinterpret_cast<char*>(&keyword[0]), "fnil") == 0)
			fontFamily = RTFFontTableItem::ffNone;
		else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "froman") == 0)
			fontFamily = RTFFontTableItem::ffRoman;
		else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "fswiss") == 0)
			fontFamily = RTFFontTableItem::ffSwiss;
		else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "fmodern") == 0)
			fontFamily = RTFFontTableItem::ffModern;
		else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "fscript") == 0)
			fontFamily = RTFFontTableItem::ffScript;
		else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "fdecor") == 0)
			fontFamily = RTFFontTableItem::ffDecorative;
		else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "ftech") == 0)
			fontFamily = RTFFontTableItem::ffTechnical;
		else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "fbidi") == 0)
			fontFamily = RTFFontTableItem::ffBiDirectional;
		else
		{
			fontFamily = RTFFontTableItem::ffNone;
		}
	}
	// Now (possibly) comes some optional keyword before the fontname
	while (tokenType != RTF_TOKEN_DATA || nesting > 0)
	{
    	tokenType = NextToken(keyword,&parameter,&paramUsed,MAX_KEYWORD_LEN,true);
		switch (tokenType)
		{
		case RTF_TOKEN_OPEN_BRACE:
			nesting ++;
			break;
		case RTF_TOKEN_CLOSE_BRACE:
			nesting --;
			break;
		case RTF_TOKEN_DATA:
			break;
		case RTF_TOKEN_KEYWORD:
			pValue = const_cast<int*>(static_cast<const int*>(keywordMap.pick(reinterpret_cast<char*>(&keyword[0]))));
			if (pValue != NULL)
			{
				UT_return_val_if_fail(paramUsed, false);
				*pValue = parameter;
			}
			if (strcmp(reinterpret_cast<char*>(&keyword[0]),"panose") == 0)
			{
				// These take the form of {\*\panose 020b0604020202020204}
				// Panose numbers are used to classify Latin-1 fonts for matching
				// If you are really interested, see
				// http://www.w3.org/TR/REC-CSS2/notes.html#panose
				if (!ReadCharFromFile(&ch))
				{
					return false;
				}

				if (isdigit(ch))
				{
					SkipBackChar(ch);
				}

				for (int i = 0; i < 10; i++)
				{
					// TODO: Matti Picus commented out the original code
					// since he read the web page above and realized it
					// is broken.  Perhaps someone can explain what the
					// original author's intent was? The code now just
					// grabs the second char from each byte of the number
					//
					//unsigned char buf[3] = "00";
					//if ( !ReadCharFromFile(&(buf[0]))  ||  !ReadCharFromFile(&(buf[1])) )
					//{
					//	return false;
					//}
					//unsigned char val = static_cast<unsigned char>(atoi(static_cast<char*>(buf)));
					//panose[i] = val;
					if ( !ReadCharFromFile(&ch)  ||  !ReadCharFromFile(&ch) )
					{
						return false;
					}
					panose[i] = ch;
				}
			}
			break;
		default:
			//TODO: handle errors
			break;
		}
	}
	if (nesting == -1)
	{
		UT_DEBUGMSG(("RTF: Font name not found in font definition %d",fontIndex));
	}
	// Now comes the font name, terminated by either a close brace or a slash or a semi-colon
	ch = keyword[0];
	int count = 0;
	/*
	    FIXME: CJK font names come in form \'aa\'cd\'ef - so we have to
	    parse \'HH correctly (currently we ignore them!) - VH
	*/
	while ( ch != '}'  &&  ch != '\\'  &&  ch != ';' && ch!= '{')
	{
		keyword[count++] = ch;
		if (!ReadCharFromFile(&ch))
		{
			return false;
		}
	}
	if (ch=='{')
	{
		++nesting;
	}

	keyword[count] = 0;
	/*work around "helvetica" font name -replace it with "Helvetic"*/
	if (!UT_stricmp(reinterpret_cast<char*>(&keyword[0]),"helvetica"))
	{
		strcpy(reinterpret_cast<char*>(&keyword[0]),"Helvetic");
	}

	if (!UT_cloneString(pFontName, reinterpret_cast<char*>(&keyword[0])))
	{
		// TODO outofmem
	}
	for (int i=0; i <= nesting; ++i)
	{
		// Munch the remaining control words down to the close brace
		while (ch != '}')
		{
			if (!ReadCharFromFile(&ch))
			{
				return false;
			}
			if (ch=='{')
			{
				++nesting;
			}
		}
		if (nesting>0 && i!=nesting) //we need to skip '}' we've just seen.
		{
			if (!ReadCharFromFile(&ch))
			{
				return false;
			}
		}
	}

	// Create the font entry and put it into the font table
	RTFFontTableItem* pNewFont = new RTFFontTableItem(fontFamily, charSet,
													  codepage, pitch,
													  panose, pFontName,
													  pAlternativeFontName);
	if (pNewFont == NULL)
	{
		return false;
	}

	// ensure that the font table is large enough for this index
	while (m_fontTable.getItemCount() <= fontIndex)
	{
		m_fontTable.addItem(NULL);
	}
	void* pOld = NULL;
	// some RTF files define the fonts several time. This is INVALID according to the
	// specifications. So we ignore it.
	if (m_fontTable[fontIndex] == NULL)
	{
		UT_sint32 res = m_fontTable.setNthItem(fontIndex, pNewFont, &pOld);
		UT_return_val_if_fail(res == 0, false);
		UT_return_val_if_fail(pOld == NULL, false);
	}
	else
	{
		UT_DEBUGMSG (("RTF: font %d (named %s) already defined. Ignoring\n", fontIndex, pFontName));
		DELETEP (pNewFont);
	}

	return true;
}




//////////////////////////////////////////////////////////////////////////////
// Colour table reader
//////////////////////////////////////////////////////////////////////////////

bool IE_Imp_RTF::ReadColourTable()
{
	// Ensure the table is empty before we start
	UT_return_val_if_fail(m_colourTable.getItemCount() == 0, false);

	unsigned char keyword[MAX_KEYWORD_LEN];
	unsigned char ch;
	long parameter = 0;
	bool paramUsed = false;
	if (!ReadCharFromFile(&ch))
		return false;

	while (ch != '}')
	{
		UT_uint32 colour = 0;
		bool tableError = false;
		while(ch == ' ')
		{
			if (!ReadCharFromFile(&ch))
				return false;
		}

		// Create a new entry for the colour table
 		if (ch == ';')
		{
			// Default colour required, black it is
			colour = 0;
		}
		else if(ch != '}')
		{
			if (ch == '\\')
			{
				// read colour definition
				long red = 0;
				long green = 0;
				long blue = 0;
				bool hasRed, hasGreen, hasBlue;
				hasRed = hasGreen = hasBlue = false;

				for (int i = 0; i < 3; i++)
				{
					// read Red, Green and Blue values (will be in that order).
					if (!ReadKeyword(keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN)) 
					{
						UT_DEBUGMSG (("ReadKeyword() failed in ReadColourTable()\n"));
						return false;
					}
					if (strcmp(reinterpret_cast<char*>(&keyword[0]), "red") == 0  &&  paramUsed)
					{
						if (!hasRed) {
							red = parameter;
							hasRed = true;
						}
						else {
							tableError = true;
						}
					}
					else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "green") == 0  &&  paramUsed)
					{
						if (!hasGreen) {
							green = parameter;
							hasGreen = true;
						}
						else {
							tableError = true;
						}
					}
					else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "blue") == 0  &&  paramUsed)
					{
						if (!hasBlue) {
							blue = parameter;
							hasBlue = true;
						}
						else {
							tableError = true;
						}
					}
					else 
					{
						tableError = true;
					}
					// Read slash at start of next keyword
					if (!ReadCharFromFile(&ch) 
							|| ((ch != '\\') && (ch != ';')))
					{
						tableError = true;
					}
				}
				colour = static_cast<unsigned char>(red << 16 | static_cast<unsigned char>(green << 8 | static_cast<unsigned char>(blue)));
			}
			else {
				tableError = true;
			}
		}

		if (tableError)
		{
			UT_DEBUGMSG (("RTF color Table error\n"));
			return false;
		}
		else if(ch!= '}')
		{
			m_colourTable.addItem(reinterpret_cast<void*>(colour));

			// Read in the next char
			if (!ReadCharFromFile(&ch))
				return false;
		}
	}

	// Put the '}' back into the input stream
	return SkipBackChar(ch);
}




//////////////////////////////////////////////////////////////////////////////
// List properties
//////////////////////////////////////////////////////////////////////////////

bool IE_Imp_RTF::HandleLists(_rtfListTable & rtfTable )
{
	unsigned char keyword[MAX_KEYWORD_LEN];
	unsigned char ch;
	long parameter = 0;
	bool paramUsed = false;
	if (!ReadCharFromFile(&ch))
		return false;

	while (ch != '}') // Outer loop
	{
	  //SkipBackChar(ch); // Put char back in stream
		if(ch == '{')  // pntxta or pntxtb
		{
			if (!ReadCharFromFile(&ch))
				return false;
			if(!ReadKeyword(keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN))
			{
				return false;
			}
			else
			{
				if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pntxta") == 0)
				{
			  // OK scan through the text until a closing delimeter is
			  // found
					int count = 0;
					if (!ReadCharFromFile(&ch))
						return false;
					while ( ch != '}'  && ch != ';')
					{
						keyword[count++] = ch;
						if (!ReadCharFromFile(&ch))
							return false;
					}
					keyword[count++] = 0;
					strcpy(rtfTable.textafter,reinterpret_cast<char*>(&keyword[0]));
					UT_DEBUGMSG(("FOUND pntxta in stream, copied %s to input  \n",keyword));
				}
				else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pntxtb") == 0)
				{
			  // OK scan through the text until a closing delimeter is
			  // found
					int count = 0;
					if (!ReadCharFromFile(&ch))
						return false;
					while ( ch != '}'  && ch != ';' )
					{
						keyword[count++] = ch;
						if (!ReadCharFromFile(&ch))
							return false;
					}
					keyword[count++] = 0;
					strcpy(rtfTable.textbefore,reinterpret_cast<char*>(&keyword[0]));
					UT_DEBUGMSG(("FOUND pntxtb in stream,copied %s to input  \n",keyword));
				}
				else
				{
					UT_DEBUGMSG(("Unknown keyword %s found in List stream  \n",keyword));
				}
			}
			goto nextChar;
		}
		if(!ReadKeyword(keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN))
		{
		        return false;
		}
		else
		{
			if (strcmp(reinterpret_cast<char*>(&keyword[0]), "m_levelStartAt") == 0)
			{
				rtfTable.start_value = static_cast<UT_uint32>(parameter);
				UT_DEBUGMSG(("FOUND m_levelStartAt in stream \n"));
			}
			if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnstart") == 0)
			{
				rtfTable.start_value = static_cast<UT_uint32>(parameter);
				UT_DEBUGMSG(("FOUND pnstart in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnlvl") == 0)
			{
				rtfTable.level = static_cast<UT_uint32>(parameter);
				UT_DEBUGMSG(("FOUND pnlvl in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnlvlblt") == 0)
			{
				rtfTable.bullet = true;
				UT_DEBUGMSG(("FOUND pnlvlblt in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnlvlbody") == 0)
			{
				rtfTable.simple = true;
				UT_DEBUGMSG(("FOUND pnlvlbody in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnlvlcont") == 0)
			{
				rtfTable.continueList = true;
				UT_DEBUGMSG(("FOUND pnlvlcont in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnnumonce") == 0)
			{
				UT_DEBUGMSG(("FOUND pnnumonce in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnacross") == 0)
			{
				UT_DEBUGMSG(("FOUND pnacross in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnhang") == 0)
			{
				rtfTable.hangingIndent = true;
				UT_DEBUGMSG(("FOUND pnhang in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pncard") == 0)
			{
				rtfTable.type = NUMBERED_LIST;
				UT_DEBUGMSG(("FOUND pncard in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pndec") == 0)
			{
				rtfTable.type = NUMBERED_LIST;
				UT_DEBUGMSG(("FOUND pndec in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnucltr") == 0)
			{
				rtfTable.type = UPPERCASE_LIST;
				UT_DEBUGMSG(("FOUND pnucltr in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnuclrm") == 0)
			{
				rtfTable.type = UPPERROMAN_LIST;
				UT_DEBUGMSG(("FOUND pnucrm in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnlcltr") == 0)
			{
				rtfTable.type = LOWERCASE_LIST;
				UT_DEBUGMSG(("FOUND pnlctr in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnlclrm") == 0)
			{
				rtfTable.type = LOWERROMAN_LIST;
				UT_DEBUGMSG(("FOUND pnlcrm in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnord") == 0)
			{
				rtfTable.type = NUMBERED_LIST;
				UT_DEBUGMSG(("FOUND pnord in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnordt") == 0)
			{
				rtfTable.type = NUMBERED_LIST;
				UT_DEBUGMSG(("FOUND pnordt in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnb") == 0)
			{
				rtfTable.bold = true;
				UT_DEBUGMSG(("FOUND pnb in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pni") == 0)
			{
				rtfTable.italic = true;
				UT_DEBUGMSG(("FOUND pni in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pncaps") == 0)
			{
				rtfTable.caps = true;
				UT_DEBUGMSG(("FOUND pncaps in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnscaps") == 0)
			{
				rtfTable.scaps = true;
				UT_DEBUGMSG(("FOUND pnscaps in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnul") == 0)
			{
				rtfTable.underline = true;
				UT_DEBUGMSG(("FOUND pnul in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnuld") == 0)
			{
				rtfTable.underline = true;
				UT_DEBUGMSG(("FOUND pnuld in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnuldb") == 0)
			{
				rtfTable.underline = true;
				UT_DEBUGMSG(("FOUND pnuldb in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnulnone") == 0)
			{
				rtfTable.nounderline = true;
				UT_DEBUGMSG(("FOUND pnulnone in stream \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnulw") == 0)
			{
				 UT_DEBUGMSG(("FOUND pnulw in stream - ignore for now \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnstrike") == 0)
			{
				rtfTable.strike = true;
				UT_DEBUGMSG(("FOUND pnstrike in stream  \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pncf") == 0)
			{
				rtfTable.forecolor =  static_cast<UT_uint32>(parameter);
				UT_DEBUGMSG(("FOUND pncf in stream  \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnf") == 0)
			{
				rtfTable.font =  static_cast<UT_uint32>(parameter);
				UT_DEBUGMSG(("FOUND pnf in stream  \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnfs") == 0)
			{
				rtfTable.fontsize =  static_cast<UT_uint32>(parameter);
				UT_DEBUGMSG(("FOUND pnfs in stream  \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnindent") == 0)
			{
				rtfTable.indent =  static_cast<UT_uint32>(parameter);
				UT_DEBUGMSG(("FOUND pnindent in stream  \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnsp") == 0)
			{
				UT_DEBUGMSG(("FOUND pnsp in stream  - ignored for now \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnprev") == 0)
			{
				rtfTable.prevlist =  true;
				UT_DEBUGMSG(("FOUND pnprev in stream  \n"));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnqc") == 0)
			{
				UT_DEBUGMSG(("FOUND pnqc in stream - ignored for now \n"));
				// centered numbering
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnql") == 0)
			{
				UT_DEBUGMSG(("FOUND pnql in stream - ignored for now \n"));
				// left justified numbering
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnqr") == 0)
			{
				UT_DEBUGMSG(("FOUND pnqr in stream - ignored for now \n"));
				// right justified numbering
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "ls") == 0)
			{
				UT_DEBUGMSG(("FOUND ls in stream - override number \n",parameter));
				rtfTable.iWord97Override =  static_cast<UT_uint32>(parameter);
				// Word 97 list table identifier
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "ilvl") == 0)
			{
				UT_DEBUGMSG(("FOUND ilvl in stream - levelnumber \n",parameter));
				rtfTable.iWord97Level = static_cast<UT_uint32>(parameter);
				// Word 97 list level
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pnrnot") == 0)
			{
				UT_DEBUGMSG(("FOUND pnrnot in stream - ignored for now \n"));
				// Don't know this
			}
			else
			{
				UT_DEBUGMSG(("Unknown keyword %s found in List stream  \n",keyword));
			}
         nextChar:	if (!ReadCharFromFile(&ch))
			         return false;
		}
	}
	// Put the '}' back into the input stream
	return SkipBackChar(ch);
}



//////////////////////////////////////////////////////////////////////////////
// AbiList table reader
//////////////////////////////////////////////////////////////////////////////

bool IE_Imp_RTF::HandleAbiLists()
{
	unsigned char keyword[MAX_KEYWORD_LEN];
	unsigned char ch;
	long parameter = 0;
	bool paramUsed = false;
	if (!ReadCharFromFile(&ch))
		return false;

	while (ch != '}') // Outer loop
	{
		if(ch == '{')  // abiliststyle, abilistdecimal, abilistdelim
		{
			if (!ReadCharFromFile(&ch))
				return false;
			if(!ReadKeyword(keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN))
			{
				return false;
			}
			else
			{
				if (strcmp(reinterpret_cast<char*>(&keyword[0]), "abiliststyle") == 0)
				 {
			  // OK scan through the text until a closing delimeter is
			  // found
					 int count = 0;
					 if (!ReadCharFromFile(&ch))
						 return false;
					 while ( ch != '}'  && ch != ';')
					 {
						 keyword[count++] = ch;
						 if (!ReadCharFromFile(&ch))
							 return false;
					 }
					 keyword[count++] = 0;
					 strcpy(m_currentRTFState.m_paraProps.m_pszStyle,reinterpret_cast<char*>(&keyword[0]));
				 }
				 else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "abilistdecimal") == 0)
				 {
			  // OK scan through the text until a closing delimeter is
			  // found
					 int count = 0;
					 if (!ReadCharFromFile(&ch))
						 return false;
					 while ( ch != '}'  && ch != ';' )
					 {
						 keyword[count++] = ch;
						 if (!ReadCharFromFile(&ch))
							 return false;
					 }
					 keyword[count++] = 0;
					 strcpy(m_currentRTFState.m_paraProps.m_pszListDecimal,reinterpret_cast<char*>(&keyword[0]));
				 }
				 else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "abilistdelim") == 0)
				 {
			  // OK scan through the text until a closing delimeter is
			  // found
					 int count = 0;
					 if (!ReadCharFromFile(&ch))
						 return false;
					 while ( ch != '}'  && ch != ';' )
					 {
						 keyword[count++] = ch;
						 if (!ReadCharFromFile(&ch))
							 return false;
					 }
					 keyword[count++] = 0;
					 strcpy(m_currentRTFState.m_paraProps.m_pszListDelim,reinterpret_cast<char*>(&keyword[0]));
				 }
				 else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "abifieldfont") == 0)
				 {
			  // OK scan through the text until a closing delimeter is
			  // found
					 int count = 0;
					 if (!ReadCharFromFile(&ch))
						 return false;
					 while ( ch != '}'  && ch != ';' )
					 {
						 keyword[count++] = ch;
						 if (!ReadCharFromFile(&ch))
							 return false;
					 }
					 keyword[count++] = 0;
					 strcpy(m_currentRTFState.m_paraProps.m_pszFieldFont,reinterpret_cast<char*>(&keyword[0]));
				 }
				 else
				 {
					 UT_DEBUGMSG(("Unknown keyword %s found in List stream  \n",keyword));
				 }
			}
			goto nextChar;
		}
		if(!ReadKeyword(keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN))
		{
			return false;
		}
		else
		{
			if (strcmp(reinterpret_cast<char*>(&keyword[0]), "abistartat") == 0)
			{
				m_currentRTFState.m_paraProps.m_startValue= static_cast<UT_uint32>(parameter);
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "abilistid") == 0)
			{
				m_currentRTFState.m_paraProps.m_rawID = static_cast<UT_uint32>(parameter);
				m_currentRTFState.m_paraProps.m_isList = true;

			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "abilistparentid") == 0)
			{
				m_currentRTFState.m_paraProps.m_rawParentID = static_cast<UT_uint32>(parameter);
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "abilistlevel") == 0)
			{
				m_currentRTFState.m_paraProps.m_level = static_cast<UT_uint32>(parameter);
			}
			else
			{
				UT_DEBUGMSG(("Unknown keyword %s found in List stream  \n",keyword));
			}
		}
	nextChar:	if (!ReadCharFromFile(&ch))
		return false;
	}
	//
	// Increment the list mapping table if necessary
	//
	UT_uint32 i;
	if(m_currentRTFState.m_paraProps.m_rawID != 0)
	{
		for(i=0; i < m_numLists; i++)
		{
			if(m_currentRTFState.m_paraProps.m_rawID == getAbiList(i)->orig_id)
				break;
		}
		if(i >= m_numLists)
		{
			m_vecAbiListTable.addItem( static_cast<void *>(new _rtfAbiListTable));
			getAbiList(m_numLists)->orig_id = m_currentRTFState.m_paraProps.m_rawID ;
			getAbiList(m_numLists)->orig_parentid = m_currentRTFState.m_paraProps.m_rawParentID ;
			getAbiList(m_numLists)->level = m_currentRTFState.m_paraProps.m_level ;
			getAbiList(m_numLists)->hasBeenMapped = false;
			getAbiList(m_numLists)->start_value = 0;
			getAbiList(m_numLists)->mapped_id = 0;
			getAbiList(m_numLists)->mapped_parentid = 0;
			m_numLists++;
		}
	}

	// Put the '}' back into the input stream

	//return SkipBackChar(ch);
	return true;
}



//////////////////////////////////////////////////////////////////////////////
// Character Properties keyword handlers
//////////////////////////////////////////////////////////////////////////////

bool IE_Imp_RTF::HandleBoolCharacterProp(bool state, bool* pProp)
{
	bool ok = FlushStoredChars();
	*pProp = state;
	return ok;
}

bool IE_Imp_RTF::HandleDeleted(bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_deleted);
}

bool IE_Imp_RTF::HandleBold(bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_bold);
}

bool IE_Imp_RTF::HandleItalic(bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_italic);
}

bool IE_Imp_RTF::HandleUnderline(bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_underline);
}

bool IE_Imp_RTF::HandleOverline(bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_overline);
}


bool IE_Imp_RTF::HandleTopline(bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_topline);
}


bool IE_Imp_RTF::HandleBotline(bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_botline);
}

bool IE_Imp_RTF::HandleStrikeout(bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_strikeout);
}

bool IE_Imp_RTF::HandleSuperscript(bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_superscript);
}

// pos is in 1/2pt like in RTF
bool IE_Imp_RTF::HandleSuperscriptPosition(UT_uint32 pos)
{
	bool ok;
	ok = HandleBoolCharacterProp((pos != 0) ? true : false, &m_currentRTFState.m_charProps.m_superscript);
	if (ok)
	{
		ok = HandleFloatCharacterProp (pos*0.5, &m_currentRTFState.m_charProps.m_superscript_pos);
	}
	return ok;
}

bool IE_Imp_RTF::HandleSubscript(bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_subscript);
}

// pos is in 1/2pt like in RTF
bool IE_Imp_RTF::HandleSubscriptPosition(UT_uint32 pos)
{
	bool ok;
	ok = HandleBoolCharacterProp((pos != 0) ? true : false, &m_currentRTFState.m_charProps.m_subscript);
	if (ok)
	{
		ok = HandleFloatCharacterProp (pos*0.5, &m_currentRTFState.m_charProps.m_subscript_pos);
	}
	return ok;
}

bool IE_Imp_RTF::HandleFontSize(long sizeInHalfPoints)
{
	return HandleFloatCharacterProp (sizeInHalfPoints*0.5, &m_currentRTFState.m_charProps.m_fontSize);
}


bool IE_Imp_RTF::HandleFloatCharacterProp(double val, double* pProp)
{
	bool ok = FlushStoredChars();
	*pProp = val;
	return ok;
}

bool IE_Imp_RTF::HandleU32CharacterProp(UT_uint32 val, UT_uint32* pProp)
{
	bool ok = FlushStoredChars();
	*pProp = val;
	return ok;
}

bool IE_Imp_RTF::HandleListTag(long id)
{
	UT_uint32 sid = static_cast<UT_uint32>(id);
	return HandleU32CharacterProp(sid, &m_currentRTFState.m_charProps.m_listTag);
}

bool IE_Imp_RTF::HandleFace(UT_uint32 fontNumber)
{
	RTFFontTableItem* pFont = GetNthTableFont(fontNumber);
	if (pFont != NULL && pFont->m_szEncoding)
		m_mbtowc.setInCharset(pFont->m_szEncoding);

	return HandleU32CharacterProp(fontNumber, &m_currentRTFState.m_charProps.m_fontNumber);
}

bool IE_Imp_RTF::HandleColour(UT_uint32 colourNumber)
{
	if (HandleBoolCharacterProp(true, &m_currentRTFState.m_charProps.m_hasColour))
	{
		return HandleU32CharacterProp(colourNumber, &m_currentRTFState.m_charProps.m_colourNumber);
	}
	return false;
}

bool IE_Imp_RTF::HandleBackgroundColour(UT_uint32 colourNumber)
{
	if (HandleBoolCharacterProp(true, &m_currentRTFState.m_charProps.m_hasBgColour))
	{
		return HandleU32CharacterProp(colourNumber, &m_currentRTFState.m_charProps.m_bgcolourNumber);
	}
	return false;
}

bool IE_Imp_RTF::SetParaJustification(RTFProps_ParaProps::ParaJustification just)
{
	m_currentRTFState.m_paraProps.m_justification = just;

	return true;
}

bool IE_Imp_RTF::AddTabstop(UT_sint32 stopDist, eTabType tabType, eTabLeader tabLeader)
{
	m_currentRTFState.m_paraProps.m_tabStops.addItem(reinterpret_cast<void*>(stopDist));	// convert from twip to inch
	if(tabType >=FL_TAB_LEFT && tabType <= FL_TAB_BAR  )
	{
		m_currentRTFState.m_paraProps.m_tabTypes.addItem(reinterpret_cast<void*>(tabType));
	}
	else
	{
		m_currentRTFState.m_paraProps.m_tabTypes.addItem(reinterpret_cast<void*>(FL_TAB_LEFT));
	}
	if(tabLeader >= FL_LEADER_NONE  && tabLeader <= FL_LEADER_EQUALSIGN)
	{
		m_currentRTFState.m_paraProps.m_tabLeader.addItem(reinterpret_cast<void*>(tabLeader));
	}
	else
	{
		m_currentRTFState.m_paraProps.m_tabLeader.addItem(reinterpret_cast<void*>(FL_LEADER_NONE));
	}

	return true;
}



bool IE_Imp_RTF::AddTabstop(UT_sint32 stopDist, eTabType tabType, eTabLeader tabLeader,  RTFProps_ParaProps * pParas)
{
	pParas->m_tabStops.addItem(reinterpret_cast<void*>(stopDist));	// convert from twip to inch
	if(tabType >=FL_TAB_LEFT && tabType <= FL_TAB_BAR  )
	{
		pParas->m_tabTypes.addItem(reinterpret_cast<void*>(tabType));
	}
	else
	{
		pParas->m_tabTypes.addItem(reinterpret_cast<void*>(FL_TAB_LEFT));
	}
	if(tabLeader >= FL_LEADER_NONE  && tabLeader <= FL_LEADER_EQUALSIGN)
	{
		pParas->m_tabLeader.addItem(reinterpret_cast<void*>(tabLeader));
	}
	else
	{
		pParas->m_tabLeader.addItem(reinterpret_cast<void*>(FL_LEADER_NONE));
	}

	return true;
}

/*!
  Get the next token, put it into buf, if there is an error return RTF_TOKEN_ERROR
  otherwise returns the RTFTokenType
  If it is RTF_TOKEN_DATA, data is returned byte by byte.
  RTF_TOKEN_KEYWORD includes control words and control symbols
  (like hex char).
  It is up to the caller to distinguish beetween them and parse them.
  \retval pKeyword is the data
  \retval pParam is the keyword parameter if any, otherwise ""
  \retval pParamUsed is a flag to tell whether there is a parameter.
  \return the type of the next token parsed.
  \note Both pParam amd pParamUsed are only used if tokenType is
  RTF_TOKEN_KEYWORD
  \note this changes the state of the file
*/
IE_Imp_RTF::RTFTokenType IE_Imp_RTF::NextToken (unsigned char *pKeyword, long* pParam,
									bool* pParamUsed, UT_uint32 len, bool bIgnoreWhiteSpace)
{
	RTFTokenType tokenType = RTF_TOKEN_NONE;
	bool ok;

	UT_return_val_if_fail (pKeyword, RTF_TOKEN_NONE);
	UT_return_val_if_fail (len, RTF_TOKEN_NONE);
	UT_return_val_if_fail (pParamUsed, RTF_TOKEN_NONE);
	UT_return_val_if_fail (pParam, RTF_TOKEN_NONE);
	*pParam = 0;
	*pParamUsed = false;
	pKeyword [0] = ' ';

	if(bIgnoreWhiteSpace)
	{
		// see bug 1211 and bug 1207.rtf - invalid RTF coming in
		// this code instead violates the RTF spec in order to fix that problem,
		// but instead breaks other things like field values:
		// If a space delimits the control word, the space does not appear in the
		// document. Any characters following the delimiter, including spaces, will
		// appear in the document. For this reason, you should use spaces only
		// where necessary; do not use spaces merely to break up RTF code.

		// OK Sevior put in bool to choose this behaviour for some parts of documents
        // where we can work around this broken behaviour and still import the doc.

		while( pKeyword[0] == ' ')
		{
			if (!ReadCharFromFile(pKeyword))
				{
					tokenType = RTF_TOKEN_ERROR;
				}
		}
	}
	else
	{
		if (!ReadCharFromFile(pKeyword))
		{
			tokenType = RTF_TOKEN_ERROR;
		}
	}

	switch (*pKeyword)
	{
	case '\\':
		tokenType = RTF_TOKEN_KEYWORD;
		ok = ReadKeyword (pKeyword, pParam, pParamUsed, len);
		if (!ok)
		{
			tokenType = RTF_TOKEN_ERROR;
		}
		break;
	case '{':
		tokenType = RTF_TOKEN_OPEN_BRACE;
		break;
	case '}':
		tokenType = RTF_TOKEN_CLOSE_BRACE;
		break;
	default:
		tokenType = RTF_TOKEN_DATA;
		break;
	}

	return tokenType;
}


/*!
  Handle a bookmark keyword
 */
bool IE_Imp_RTF::HandleBookmark (RTFBookmarkType type)
{
	UT_Byte ch = 0;
	UT_String bookmarkName;

	xxx_UT_DEBUGMSG(("hub: HandleBookmark of type %d\n", type));


	while (ch != '}')
	{
		if (!ReadCharFromFile(&ch)) {
			return false;
		}
		if (ch != '}') {
			bookmarkName += ch;
		}
	}
	SkipBackChar (ch);

	const XML_Char * props [5];
	props [0] = "type";
	switch (type) {
	case RBT_START:
		props [1] = "start";
		break;
	case RBT_END:
		props [1] = "end";
		break;
	default:
		UT_ASSERT_NOT_REACHED();
		props [1] = NULL;
		break;
	}
	props [2] = "name";
	props [3] = bookmarkName.c_str();
	props [4] = NULL;
	UT_DEBUGMSG(("SEVIOR: Appending Object 3 m_bCellBlank %d m_bEndTableOpen %d \n",m_bCellBlank,m_bEndTableOpen));
	if(m_bCellBlank || m_bEndTableOpen)
	{
		getDoc()->appendStrux(PTX_Block,NULL);
		m_bCellBlank = false;
		m_bEndTableOpen = false;
	}

	if ((m_pImportFile != NULL) || (m_parsingHdrFtr)) 
	{
		getDoc()->appendObject(PTO_Bookmark, props);
	}
	else {
		getDoc()->insertObject(m_dposPaste, PTO_Bookmark, props, NULL);
		m_dposPaste++;
	}
	return true;
}


void IE_Imp_RTF::_appendHdrFtr ()
{
	UT_uint32 i;
	UT_uint32 numHdrFtr;
	const RTFHdrFtr * header;
	UT_String tempBuffer;
	const XML_Char* szType = NULL;

	UT_return_if_fail(m_pImportFile);

	numHdrFtr = m_hdrFtrTable.getItemCount();

	for (i = 0; i < numHdrFtr; i++)
	{
		header = static_cast<const RTFHdrFtr *>(m_hdrFtrTable[i]);

		m_pPasteBuffer = reinterpret_cast<const unsigned char *>(header->m_buf.getPointer (0));
		m_lenPasteBuffer = header->m_buf.getLength ();
		m_pCurrentCharInPasteBuffer = m_pPasteBuffer;
		m_dposPaste = FV_DOCPOS_EOD;
		const XML_Char* propsArray[9];
		UT_String hdrftrID;
		switch (header->m_type)
		{
		case RTFHdrFtr::hftHeader:
			UT_String_sprintf (tempBuffer, "hdr%u", header->m_id);
			szType = "header";
			break;
		case RTFHdrFtr::hftHeaderEven:
			UT_String_sprintf (tempBuffer, "hdrevn%u", header->m_id);
			szType = "header-even";
			break;
		case RTFHdrFtr::hftHeaderFirst:
			UT_String_sprintf (tempBuffer, "hdrfst%u", header->m_id);
			szType = "header-first";
			break;
		case RTFHdrFtr::hftHeaderLast:
			UT_String_sprintf (tempBuffer, "hdrlst%u", header->m_id);
			szType = "header-last";
			break;
		case RTFHdrFtr::hftFooter:
			UT_String_sprintf (tempBuffer, "ftr%u", header->m_id);
			szType = "footer";
			break;
		case RTFHdrFtr::hftFooterEven:
			UT_String_sprintf (tempBuffer, "ftrevn%u", header->m_id);
			szType = "footer-even";
			break;
		case RTFHdrFtr::hftFooterFirst:
			UT_String_sprintf (tempBuffer, "ftrfst%u", header->m_id);
			szType = "footer-first";
			break;
		case RTFHdrFtr::hftFooterLast:
			UT_String_sprintf (tempBuffer, "ftrlst%u", header->m_id);
			szType = "footer-last";
			break;
		default:
			UT_ASSERT_NOT_REACHED();
		}
		UT_DEBUGMSG (("id is %s\n", tempBuffer.c_str()));
		hdrftrID = tempBuffer;
		propsArray[0] = "type";
		propsArray[1] = szType;
		propsArray[2] = "id";
		propsArray[3] = tempBuffer.c_str();
		propsArray[4] = "listid";
		propsArray[5] = "0";
		propsArray[6] = "parentid";
		propsArray[7] = "0";
		propsArray[8] = NULL;

		if(!getDoc()->verifySectionID(hdrftrID.c_str()))
		{
			PL_StruxDocHandle sdh = getDoc()->getLastSectionSDH();
			getDoc()->changeStruxAttsNoUpdate(sdh,szType,hdrftrID.c_str());
		}
		getDoc()->appendStrux (PTX_SectionHdrFtr, propsArray);
		propsArray[0] = NULL;
		// actually it appears that we have to append a block for some cases.
		getDoc()->appendStrux(PTX_Block, propsArray);

		// tell that we are parsing headers and footers
		m_parsingHdrFtr = true;
		m_newParaFlagged = true;
		_parseFile (NULL);
		m_parsingHdrFtr = false;
	}
}

/*!
  Appends a field to the document.
  \param xmlField the field type value
  \return true if OK
 */
bool IE_Imp_RTF::_appendField (const XML_Char *xmlField, const XML_Char ** pszAttribs)
{
	bool ok;
	const XML_Char** propsArray = NULL;
	UT_String propBuffer;
	buildCharacterProps(propBuffer);

	const XML_Char * pStyle = NULL;
    const XML_Char * pStyleName = NULL;
	if(m_currentRTFState.m_charProps.m_styleNumber >= 0
	   && (UT_uint32)m_currentRTFState.m_charProps.m_styleNumber < m_styleTable.size())
	{
		pStyle = PT_STYLE_ATTRIBUTE_NAME;
		pStyleName = static_cast<const XML_Char *>(m_styleTable[m_currentRTFState.m_charProps.m_styleNumber]);
	}
		
	if(pszAttribs == NULL)
	{
		propsArray = static_cast<const XML_Char **>(UT_calloc(7, sizeof(XML_Char *)));		
		propsArray [0] = "type";
		propsArray [1] = xmlField;
		propsArray [2] = "props";
		propsArray [3] = propBuffer.c_str();
		propsArray [4] = pStyle;
		propsArray [5] = pStyleName;
		propsArray [6] = NULL;
	}
	else
	{
		UT_uint32 isize =0;
		while(pszAttribs[isize] != NULL)
		{
			isize++;
		}
		propsArray = static_cast<const XML_Char **>(UT_calloc(7+isize, sizeof(XML_Char *)));

		UT_uint32 iEmptyAttrib = 4;
		propsArray [0] = "type";
		propsArray [1] = xmlField;
		propsArray [2] = "props";
		propsArray [3] = propBuffer.c_str();
		propsArray [4] = NULL;
		propsArray [5] = NULL;

		if(pStyle)
		{
			propsArray[iEmptyAttrib++] = pStyle;
			propsArray[iEmptyAttrib++] = pStyleName;
		}
		
		
		UT_uint32 i = 0;
		for(i=0; i< isize;i++)
		{
			propsArray[iEmptyAttrib+i] = pszAttribs[i];
		}
		propsArray[iEmptyAttrib+isize] = NULL;
	}
	// TODO get text props to apply them to the field
	ok = FlushStoredChars (true);
	UT_return_val_if_fail (ok, false);
	if (m_pImportFile != NULL || m_bAppendAnyway)
	{
		xxx_UT_DEBUGMSG(("SEVIOR: Appending Object m_bCellBlank %d m_bEndTableOpen %d \n",m_bCellBlank,m_bEndTableOpen));
		if(m_bCellBlank || m_bEndTableOpen)
		{
			getDoc()->appendStrux(PTX_Block,NULL);
			m_bCellBlank = false;
			m_bEndTableOpen = false;
		}
		getDoc()->appendObject(PTO_Field, propsArray);
	}
	else
	{
		getDoc()->insertObject(m_dposPaste, PTO_Field, propsArray, NULL);
		m_dposPaste++;
	}
	free(propsArray);
	return ok;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void IE_Imp_RTF::pasteFromBuffer(PD_DocumentRange * pDocRange,
								 const unsigned char * pData, UT_uint32 lenData, const char * /* szEncoding */)
{
	UT_return_if_fail(getDoc() == pDocRange->m_pDoc);
	UT_return_if_fail(pDocRange->m_pos1 == pDocRange->m_pos2);

	m_newParaFlagged = false;
	m_newSectionFlagged = false;

	UT_DEBUGMSG(("Pasting %d bytes of RTF\n",lenData));

	m_pPasteBuffer = pData;
	m_lenPasteBuffer = lenData;
	m_pCurrentCharInPasteBuffer = pData;
	m_dposPaste = pDocRange->m_pos1;

	// to do a paste, we set the fp to null and let the
	// read-a-char routines know about our paste buffer.

	UT_return_if_fail(m_pImportFile==NULL);

	// note, we skip the _writeHeader() call since we don't
	// want to assume that selection starts with a section
	// break.
	_parseFile(NULL);

	m_pPasteBuffer = NULL;
	m_lenPasteBuffer = 0;
	m_pCurrentCharInPasteBuffer = NULL;

	return;
}


/*!
Define a new style, here is the formal syntax:
<style>	'{' <styledef>?<keycode>? <formatting> <additive>? <based>? <next>?
            <stylename>? ';' '}'
<styledef>	\s  |\*\cs  | \ds
<keycode>	'{' \keycode <keys> '}'
<additive>	\additive
<based>	\sbasedon
<next>	\snext
<autoupd>	\sautoupd
<hidden>	\shidden
<formatting>	(<brdrdef> | <parfmt> | <apoctl> | <tabdef> | <shading> | <chrfmt>)+
<stylename>	#PCDATA
<keys>	( \shift? & \ctrl? & \alt?) <key>
<key>	\fn | #PCDATA

The style definition is located within a {\stylesheet } sequence.

The minimum (useless) example would be {}
A more typical set of styles would be
{\stylesheet
    {\fs20 \sbasedon222\snext0{\*\keycode \shift\ctrl n} Normal;}
	{\s1\qr \fs20 \sbasedon0\snext1 FLUSHRIGHT;}
	{\s2\fi-720\li720\fs20\ri2880\sbasedon0\snext2 IND;}
}

*/

#define RTF_BASEDON_NONE		222		// the default

bool IE_Imp_RTF::HandleStyleDefinition(void)
{
	bool status = true;
	int nesting = 1;
	unsigned char ch;
	char * styleTypeP = "P";
	char * styleTypeC = "C";
	char * styleType = styleTypeP;
	
	UT_sint32 BasedOn[2000]; // 2000 styles. I know this should be a Vector.
	UT_sint32 FollowedBy[2000]; // 2000 styles. I know this should be a Vector.
	UT_sint32 styleCount = 0;
	UT_Vector vecStyles;
	RTFProps_ParaProps * pParas =  new RTFProps_ParaProps();
	RTFProps_CharProps *  pChars = new	RTFProps_CharProps();
	RTFProps_bParaProps * pbParas =  new RTFProps_bParaProps();
	RTFProps_bCharProps *  pbChars = new	RTFProps_bCharProps();

	static char  propBuffer[1024];
	propBuffer[0] = 0;

	const XML_Char* attribs[PT_MAX_ATTRIBUTES*2 + 1];
	UT_uint32 attribsCount=0;
	UT_String styleName = "";
	UT_sint32 styleNumber = 0;
	while (nesting>0 && status == true)
	{
        unsigned char keyword[MAX_KEYWORD_LEN];
        long parameter = 0;
	    bool parameterUsed = false;

		if (!ReadCharFromFile(&ch))
		    return false;

		switch(ch)
		{
		case '\\':
            status = ReadKeyword(keyword, &parameter, &parameterUsed, MAX_KEYWORD_LEN);
			if (!status)
			{
				return status;
			}
			else if (strcmp(reinterpret_cast<const char *>(&keyword[0]), "sbasedon") == 0)
			{
				if ((parameter != styleNumber) &&
					(parameter != RTF_BASEDON_NONE))
				{
//
// Have to deal with out of sequence styles. ie A style maybe basedon a style that
// has not yet been seen.
//
// So remember it and fill it later..
//
					BasedOn[styleCount] = static_cast<UT_sint32>(parameter);
					attribs[attribsCount++] = PT_BASEDON_ATTRIBUTE_NAME;
					attribs[attribsCount++] = NULL;
					attribs[attribsCount]   = NULL;
				}
				else if(0)
				{
					// TODO: Why is this code here? It left over from before the BasedOn array
					char * val = reinterpret_cast<char *>(m_styleTable.getNthItem(parameter));
					if (val != NULL)
					{
						attribs[attribsCount++] = PT_BASEDON_ATTRIBUTE_NAME;
						attribs[attribsCount++] = val;
						attribs[attribsCount]   = NULL;
					}
				}
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "snext") == 0)
			{
				if (parameter != styleNumber)
				{
//
// Have to deal with out of sequence styles. ie A style may have a followed-by style
// that has not yet been seen.
//
// So remember it and fill it later..
//
					FollowedBy[styleCount] = static_cast<UT_sint32>(parameter);
					attribs[attribsCount++] = PT_FOLLOWEDBY_ATTRIBUTE_NAME;
					attribs[attribsCount++] = NULL;
					attribs[attribsCount]   = NULL;
				}
				else if(parameter < styleNumber)
				{
					// TODO: Why is this code here? It left over from before the FollowedBy array
					char * val = static_cast<char *>(m_styleTable.getNthItem(parameter));
					if (val != NULL)
					{
	               		attribs[attribsCount++] = PT_FOLLOWEDBY_ATTRIBUTE_NAME;
						attribs[attribsCount++] = val;
						attribs[attribsCount]   = NULL;
					}
				}
			}
			else if ((strcmp(reinterpret_cast<char*>(&keyword[0]),  "s") == 0) ||
				     (strcmp(reinterpret_cast<char*>(&keyword[0]), "ds") == 0) ||
					 (strcmp(reinterpret_cast<char*>(&keyword[0]), "ts") == 0))
			{
				styleNumber = parameter;
				styleType = styleTypeP;
			}
			if (strcmp(reinterpret_cast<char*>(&keyword[0]), "cs") == 0)
			{
				styleNumber = parameter;
				styleType = styleTypeC;
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "*") == 0)
			{
//
// Get next keyword
//

			}
			else
			{
			    status = ParseCharParaProps(static_cast<unsigned char *>(keyword), parameter, parameterUsed,pChars,pParas,pbChars,pbParas);
			}
			break;
		case '{':
			nesting++;
			break;
		case '}':
			nesting--;
			break;
		default:
			// The only thing that should be left is the style name

			while (ch != '}' && ch != ';')
			{
				styleName += ch;
                if (!ReadCharFromFile(&ch))
		            return false;
				if (ch =='}')
				{
					UT_DEBUGMSG(("RTF: Badly formatted style name, no ';'"));
					nesting--;
				}
			}
			char * buffer  = UT_strdup(styleName.c_str());
			char * oldbuffer;
			m_styleTable.setNthItem(styleNumber,static_cast<void *>(buffer),reinterpret_cast<void **>(&oldbuffer));
			FREEP(oldbuffer);
			break;
		}
		if (nesting == 1)
		{
			// Reached the end of a single style definition.
			// Use it.
			buildAllProps(static_cast<char *>(&propBuffer[0]) ,pParas,pChars,pbParas,pbChars);
			attribs[attribsCount++] = PT_PROPS_ATTRIBUTE_NAME;
			attribs[attribsCount++] = static_cast<const char *>(&propBuffer[0]);

			attribs[attribsCount++] = PT_NAME_ATTRIBUTE_NAME;
			attribs[attribsCount++] = static_cast<const char *>(m_styleTable[styleNumber]);

			attribs[attribsCount++] = PT_TYPE_ATTRIBUTE_NAME;
			attribs[attribsCount++] = styleType;
//			attribs[attribsCount] = NULL;
//
// OK now we clone this and save it so we can set basedon's and followedby's
//
			UT_sint32 i =0;
			UT_Vector * pVecAttr = new UT_Vector();
			for( i= 0; i< static_cast<UT_sint32>(attribsCount); i++)
			{
				if(attribs[i] != NULL)
				{
					pVecAttr->addItem(static_cast<void *>(UT_strdup(attribs[i])));
				}
				else
				{
					pVecAttr->addItem(static_cast<void *>(NULL));
				}
			}
			vecStyles.addItem(static_cast<void *>(pVecAttr));

			// Reset
			styleCount++;
			attribsCount = 0;
			attribs[attribsCount] = NULL;
			styleNumber = 0;
			styleName = "";
			styleType = "P";
			DELETEP(pParas);
			DELETEP(pChars);
			DELETEP(pbParas);
			DELETEP(pbChars);
			pParas =  new RTFProps_ParaProps();
			pChars = new	RTFProps_CharProps();
			pbParas =  new RTFProps_bParaProps();
			pbChars = new	RTFProps_bCharProps();
			propBuffer[0] = 0;
		}
	}
//
// Finished Style definitions
//
	DELETEP(pParas);
	DELETEP(pChars);
	DELETEP(pbParas);
	DELETEP(pbChars);
//
// Now we loop through them all and write them into our document.
//
	UT_sint32 count = static_cast<UT_sint32>(vecStyles.getItemCount());
	UT_sint32 i = 0;
	for(i=0; i< count; i++)
	{
		// Reset
		attribsCount = 0;
		attribs[attribsCount] = NULL;
		UT_Vector * pCurStyleVec = static_cast<UT_Vector *>(vecStyles.getNthItem(i));
		UT_sint32 nAtts = static_cast<UT_sint32>(pCurStyleVec->getItemCount());
		UT_sint32 j = 0;
		const char * szName = NULL;

		while(j < nAtts)
		{
			const char * szAtt = static_cast<char *>(pCurStyleVec->getNthItem(j++));
			attribs[attribsCount++] = szAtt;
			if( UT_strcmp(szAtt, PT_NAME_ATTRIBUTE_NAME)== 0)
			{
				szName = static_cast<char *>(pCurStyleVec->getNthItem(j++));
				attribs[attribsCount++] = szName;
			}
			else if( UT_strcmp(szAtt, PT_BASEDON_ATTRIBUTE_NAME)== 0)
			{
				const char * szNext = static_cast<char *>(pCurStyleVec->getNthItem(j++));
				if(NULL == szNext)
				{
					UT_sint32 istyle = BasedOn[i];
					// must not mix static and dynamically allocated strings in the same
					// array, otherwise there is no way we can free it !!!
					//attribs[attribsCount++] = UT_strdup(static_cast<const char *>(m_styleTable[istyle]));
					attribs[attribsCount++] = static_cast<const char *>(m_styleTable[istyle]);
				}
				else
				{
					attribs[attribsCount++] = szNext;
				}
			}
			else if( UT_strcmp(szAtt, PT_FOLLOWEDBY_ATTRIBUTE_NAME)== 0)
			{
				const char * szNext = static_cast<char *>(pCurStyleVec->getNthItem(j++));
				if(NULL == szNext)
				{
					UT_sint32 istyle = FollowedBy[i];
					// must not mix static and dynamically allocated strings in the same
					// array, otherwise there is no way we can free it !!!
					// attribs[attribsCount++] = UT_strdup(static_cast<const char *>(m_styleTable[istyle]));
					attribs[attribsCount++] = static_cast<const char *>(m_styleTable[istyle]);
				}
				else
				{
					attribs[attribsCount++] = szNext;
				}
			}
			else
			{
				szAtt = static_cast<char *>(pCurStyleVec->getNthItem(j++));
				attribs[attribsCount++] = szAtt;
			}
			attribs[attribsCount] = NULL;
		}
//
// If style exists we have to redefine it like this
//
		PD_Style * pStyle = NULL;
		if(getDoc()->getStyle(szName, &pStyle))
		{
			pStyle->addAttributes(attribs);
			pStyle->getBasedOn();
			pStyle->getFollowedBy();
		}
		else
		{
			getDoc()->appendStyle(attribs);
		}
//
// OK Now delete all this allocated memory...
//
		for(j=0; j< nAtts; j++)
		{
			char * sz = static_cast<char *>(pCurStyleVec->getNthItem(j));
			if(sz != NULL)
				// MUST NOT USED delete[] on strings allocated by malloc/calloc !!!
				// delete [] sz;
				FREEP(sz);
		}
		delete pCurStyleVec;

	}
	status = PopRTFState();
	return status;

}

/*!
 * This method builds the property list from Paragraph and character classes pParas
 * and pChars
 */
bool IE_Imp_RTF::buildAllProps(char * propBuffer,  RTFProps_ParaProps * pParas,
							   RTFProps_CharProps * pChars,
							   RTFProps_bParaProps * pbParas,
							   RTFProps_bCharProps * pbChars)
{
//
// Tab stops.
//
	UT_String tempBuffer;
	UT_sint32 count =pParas->m_tabStops.getItemCount();
	if(count > 0)
		strcat(propBuffer, "tabstops:");
	UT_sint32 i=0;
	for (i = 0; i < count; i++)
	{
		if (i > 0)
			strcat(propBuffer, ",");

		UT_sint32 tabTwips = reinterpret_cast<UT_sint32>(pParas->m_tabStops.getNthItem(i));
		double tabIn = tabTwips/(20.0*72.);
		UT_uint32 idum = reinterpret_cast<UT_uint32>(pParas->m_tabTypes.getNthItem(i));
		eTabType tabType = static_cast<eTabType>(idum);
		idum = (UT_uint32) (pParas->m_tabLeader.getNthItem(i));
		eTabLeader tabLeader = static_cast<eTabLeader>(idum);
		char  cType = ' ';
		switch(tabType)
		{
		case FL_TAB_LEFT:
			cType ='L';
			break;
		case FL_TAB_RIGHT:
			cType ='R';
			break;
		case FL_TAB_CENTER:
			cType ='C';
			break;
		case FL_TAB_DECIMAL:
			cType ='D';
			break;
		case FL_TAB_BAR:
			cType ='B';
			break;
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
		char cLeader = '0' + static_cast<char>(tabLeader);
		UT_String_sprintf(tempBuffer, "%s/%c%c", UT_convertInchesToDimensionString(DIM_IN,tabIn,"04"),cType,cLeader);
		strcat(propBuffer, tempBuffer.c_str());
	}
	if( count > 0)
		strcat(propBuffer, "; ");
//
// Top and bottom paragraph margins
//
	if(pbParas->bm_spaceBefore)
	{
		UT_String_sprintf(tempBuffer, "margin-top:%s; ",		UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(pParas->m_spaceBefore)/1440));
		strcat(propBuffer, tempBuffer.c_str());
	}
	if(pbParas->bm_spaceAfter)
	{
		UT_String_sprintf(tempBuffer, "margin-bottom:%s; ",	UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(pParas->m_spaceAfter)/1440));
		strcat(propBuffer, tempBuffer.c_str());
	}
//
// Left and right margins
//
	if(pbParas->bm_indentLeft)
	{
		UT_String_sprintf(tempBuffer, "margin-left:%s; ",		UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(pParas->m_indentLeft)/1440));
		strcat(propBuffer, tempBuffer.c_str());
	}
	if(pbParas->bm_indentRight)
	{
		UT_String_sprintf(tempBuffer, "margin-right:%s; ",	UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(pParas->m_indentRight)/1440));
		strcat(propBuffer, tempBuffer.c_str());
	}
    //
	// line spacing
    //
	if (pParas->m_lineSpaceExact)
	{
		// ABIWord doesn't (yet) support exact line spacing we'll just fall back to single
		UT_String_sprintf(tempBuffer, "line-height:1.0;");
	}
	else
	{
		UT_String_sprintf(tempBuffer, "line-height:%s;",	UT_convertToDimensionlessString(fabs(pParas->m_lineSpaceVal/240)));
	}
	strcat(propBuffer, tempBuffer.c_str());
//
// Character Properties.
//
	// bold
	if(pbChars->bm_bold)
	{
		strcat(propBuffer, "font-weight:");
		strcat(propBuffer, pChars->m_bold ? "bold" : "normal");
		strcat(propBuffer,";");
	}
	// italic
	if(pbChars->bm_italic)
	{
		strcat(propBuffer, " font-style:");
		strcat(propBuffer, pChars->m_italic ? "italic" : "normal");
		strcat(propBuffer,";");
	}
	// underline & overline & strike-out
	if(pbChars->bm_underline || pbChars->bm_strikeout || pbChars->bm_overline
	   || pbChars->bm_topline || pbChars->bm_botline )
	{
		strcat(propBuffer, "; text-decoration:");
		static UT_String decors;
		decors.clear();
		if (pChars->m_underline)
		{
			decors += "underline ";
		}
		if (pChars->m_strikeout)
		{
			decors += "line-through ";
		}
		if (pChars->m_overline)
		{
			decors += "line-through ";
		}
		if (pChars->m_topline)
		{
			decors += "line-through ";
		}
		if (pChars->m_botline)
		{
			decors += "line-through ";
		}
		if(!pChars->m_underline  &&
		   !pChars->m_strikeout &&
		   !pChars->m_overline &&
		   !pChars->m_topline &&
		   !pChars->m_botline)
		{
			decors = "none";
		}
		strcat(propBuffer, decors.c_str());
		strcat(propBuffer,";");
	}
	//superscript and subscript
	if(pbChars->bm_superscript || pbChars->bm_subscript)
	{
		strcat(propBuffer, " text-position:");
		if (pChars->m_superscript)
		{
			if (pbChars->bm_superscript_pos)
			{
				UT_DEBUGMSG (("RTF: TODO: Handle text position in pt.\n"));
			}
			strcat(propBuffer, "superscript;");
		}
		else if (pChars->m_subscript)
		{
			if (pbChars->bm_subscript_pos)
			{
				UT_DEBUGMSG (("RTF: TODO: Handle text position in pt.\n"));
			}
			strcat(propBuffer, "subscript;");
		}
		else
		{
			strcat(propBuffer, "normal;");
		}
	}

	// font size
	if(pbChars->bm_fontSize)
	{
		UT_String_sprintf(tempBuffer, " font-size:%spt;", std_size_string(static_cast<float>(pChars->m_fontSize)));
		strcat(propBuffer, tempBuffer.c_str());
	}
	// typeface
	if(pbChars->bm_fontNumber)
	{
		RTFFontTableItem* pFont = GetNthTableFont(pChars->m_fontNumber);
		if (pFont != NULL)
		{
			strcat(propBuffer, " font-family:");
			strcat(propBuffer, pFont->m_pFontName);
			strcat(propBuffer, ";");
		}
	}
	// Foreground Colour
	if(pbChars->bm_hasColour)
	{
		if (pChars->m_hasColour)
		{
			// colour, only if one has been set. See bug 1324
			UT_uint32 colour = GetNthTableColour(pChars->m_colourNumber);
			UT_String_sprintf(tempBuffer, " color:%06x;", colour);
			strcat(propBuffer, tempBuffer.c_str());
		}
	}
	// BackGround Colour
	if (pbChars->bm_hasBgColour)
	{
		if(pbChars->bm_hasBgColour)
		{
			// colour, only if one has been set. See bug 1324
			UT_sint32 bgColour = GetNthTableBgColour(pChars->m_bgcolourNumber);
			if (bgColour != -1) // invalid and should be white
			{
				UT_String_sprintf(tempBuffer, " bgcolor:%06x;", bgColour);
				strcat(propBuffer, tempBuffer.c_str());
			}
		}
	}
// List Tag to hang lists off
	if(pbChars->bm_listTag)
	{
		UT_String_sprintf(tempBuffer, " list-tag:%d; ",pChars->m_listTag);
		strcat(propBuffer, tempBuffer.c_str());
	}
//
// Now remove any trailing ";"'s
//
	UT_sint32 eol = strlen(propBuffer);
	while(eol >= 0 && (propBuffer[eol] == ' ' || propBuffer[eol] == 0))
	{
		eol--;
	}
	if(propBuffer[eol] == ';')
	{
		propBuffer[eol] = 0;
	}
	return true;
}


