/* -*- c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*- */

/* AbiWord
 * Copyright (C) 1999 AbiSource, Inc.
 * Copyright (C) 2003 Tomas Frydrych <tomas@frydrych.uklinux.net>
 * Copyright (C) 2003 Martin Sevior <msevior@physics.unimelb.edu.au> 
 * Copyright (C) 2001, 2004, 2009, 2011 Hubert Figuiere <hub@figuiere.net>
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

/* RTF importer by Peter Arnold <petera@intrinsica.co.uk> */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <math.h>

#include "fl_TableLayout.h"
#include "ut_locale.h"
#include "ut_iconv.h"
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_math.h"
#include "ut_path.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_units.h"
#include "ut_std_vector.h"
#include "ie_types.h"
#include "ie_impexp_RTF.h"
#include "ie_imp_RTF.h"
#include "pd_Document.h"
#include "pd_DocumentRDF.h"
#include "pd_RDFSupport.h"
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
#include "pf_Frag.h"
#include "pf_Frag_Strux.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "fp_Run.h"
#include "wv.h" // for wvLIDToLangConverter
#include "ut_std_string.h"

#include "ie_imp_RTFParse.h"

#include <sstream>

class fl_AutoNum;


/** Ensure on input from RTF the list level is 0-8 */
inline UT_sint32 _sanitizeListLevel(UT_sint32 level)
{
	if(level > 8) {
		level = 8;
	}
	else if(level < 0) {
		level = 0;
	}
	return level;
}


/*!
  This macros allow the use of an iconv fallback name if needed.
 */
#define CPNAME_OR_FALLBACK(destination,name,fallbackname) \
{  \
	static const char* cpname = NULL; \
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


// This should probably be defined in pt_Types.h
// this used to be 8, which way to small ...
static const UT_uint32 PT_MAX_ATTRIBUTES = 20;


static char g_dbgLastKeyword [256];
static UT_sint32 g_dbgLastParam; 


UT_sint32 ABI_RTF_Annotation::sAnnotationNumber = 0;

UT_sint32 ABI_RTF_Annotation::newNumber()
{
	++sAnnotationNumber;
	return sAnnotationNumber;
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

IE_Imp_RTF_Sniffer::IE_Imp_RTF_Sniffer ()
	: IE_ImpSniffer(IE_IMPEXPNAME_RTF, true)
{
	// 
}

// supported suffixes
static IE_SuffixConfidence IE_Imp_RTF_Sniffer__SuffixConfidence[] = {
	{ "rtf", 	UT_CONFIDENCE_PERFECT 	},
	{ "doc", 	UT_CONFIDENCE_SOSO 		},
	{ "", 	UT_CONFIDENCE_ZILCH 	}
};

const IE_SuffixConfidence * IE_Imp_RTF_Sniffer::getSuffixConfidence ()
{
	return IE_Imp_RTF_Sniffer__SuffixConfidence;
}


// supported mimetypes
static IE_MimeConfidence IE_Imp_RTF_Sniffer__MimeConfidence[] = {
	{ IE_MIME_MATCH_FULL, 	IE_MIMETYPE_RTF, 		UT_CONFIDENCE_GOOD 	}, 
	{ IE_MIME_MATCH_FULL, 	"application/richtext",	UT_CONFIDENCE_GOOD 	}, 
	{ IE_MIME_MATCH_FULL, 	"text/richtext", 		UT_CONFIDENCE_GOOD 	}, 
	{ IE_MIME_MATCH_FULL, 	"text/rtf", 			UT_CONFIDENCE_GOOD 	}, 
	{ IE_MIME_MATCH_BOGUS, 	"", 					UT_CONFIDENCE_ZILCH }
};

const IE_MimeConfidence * IE_Imp_RTF_Sniffer::getMimeConfidence ()
{
	return IE_Imp_RTF_Sniffer__MimeConfidence;
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
RTF_msword97_level::RTF_msword97_level(RTF_msword97_list * pmsword97List, UT_uint32 level)
	: m_pParaProps(NULL)
	, m_pCharProps(NULL)
	, m_pbParaProps(NULL)
	, m_pbCharProps(NULL)

{
	m_levelStartAt = 1;
#if 0
	m_AbiLevelID = UT_rand();
	while(m_AbiLevelID < 10000)
		m_AbiLevelID = UT_rand();
#else
	//m_AbiLevelID = m_sLastAssignedLevelID++;
	UT_return_if_fail(pmsword97List);
	m_AbiLevelID = pmsword97List->m_pie_rtf->getDoc()->getUID(UT_UniqueId::List);
#endif
	m_pMSWord97_list = pmsword97List;
	m_localLevel = level;
	m_bStartNewList = false;
	m_listDelim = "%L";
	m_cLevelFollow = '\0';
	m_bRestart = true;
}

// Static data members must be initialized at file scope.
//UT_uint32 RTF_msword97_level::m_sLastAssignedLevelID = 100000;
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
	static std::string buf;
	static std::string ListID, ParentID, Level, StartAt, FieldFont, ListDelim, ListDecimal,Align,Indent;
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
		//m_AbiLevelID = m_sLastAssignedLevelID++;
		m_AbiLevelID = m_pMSWord97_list->m_pie_rtf->getDoc()->getUID(UT_UniqueId::List);
	}
	m_sPreviousLevel = m_localLevel;
	ListID = UT_std_string_sprintf("%d",m_AbiLevelID);
	*szListID = ListID.c_str();
//
// Now Parent ID.
//
	UT_uint32 iParentID = 0;
	// http://bugzilla.abisource.com/show_bug.cgi?id=12880
	// Check that m_pMSWord97_list is not NULL or kaboom. Assume this restart the list.
	if(m_localLevel> 0 && !m_bStartNewList && m_pMSWord97_list)
	{
		iParentID = m_pMSWord97_list->m_RTF_level[m_localLevel-1]->m_AbiLevelID;
	}
	ParentID = 	UT_std_string_sprintf("%d", iParentID);
    *szParentID = ParentID.c_str();
//
// level
//
    Level = UT_std_string_sprintf("%d",m_localLevel);
    *szLevel = Level.c_str();
//
// Start At.
//
    StartAt = UT_std_string_sprintf("%d",m_levelStartAt);
    *szStartat = StartAt.c_str();
//
// List Style
//
    FL_ListType abiListType;
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
	{
		FieldFont = m_pParaProps->m_pszFieldFont;
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
bool RTF_msword97_level::ParseLevelText(const std::string & szLevelText,const std::string & /*szLevelNumbers*/, UT_uint32 iLevel)
{
	//read the text string into a int array, set the place holders to
	//values less than zero.
	UT_sint32 iLevelText[1000];
	char const * pText = szLevelText.c_str();
	UT_sint32 ilength = 0;
	UT_sint32 icurrent = 0;
	UT_sint32 istrlen = szLevelText.size();
	bool bIsUnicode;
	while (pText[0] != '\0' && icurrent < 1000)
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

///////////////////////////////
// Paste Table class
///////////////////////////////
ABI_Paste_Table::ABI_Paste_Table(void):
	m_bHasPastedTableStrux(false),
	m_bHasPastedCellStrux(false),
	m_iRowNumberAtPaste(0),
	m_bHasPastedBlockStrux(false),
	m_iMaxRightCell(0),
	m_iCurRightCell(0),
	m_iCurTopCell(0),
	m_bPasteAfterRow(false),
	m_iPrevPasteTop(0),
	m_iNumRows(1)
{
}

ABI_Paste_Table::~ABI_Paste_Table(void)
{
}

ABI_RTF_Annotation::ABI_RTF_Annotation(void):
	m_iAnnNumber(0),
	m_sAuthor(""),
	m_sDate(""),
	m_sTitle(""),
	m_pInsertFrag(NULL),
	m_Annpos(0),
	m_iRTFLevel(0)
{
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
	m_pCharProps = NULL;
	m_pbParaProps = NULL;
	m_pbCharProps = NULL;
	m_pie_rtf = pie_rtf;
	m_pList = NULL;
}

RTF_msword97_listOverride::~RTF_msword97_listOverride(void)
{
	DELETEP(m_pParaProps);
	DELETEP(m_pCharProps);
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
std::vector<UT_sint32>* RTF_msword97_listOverride::getTabStopVect(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return &(pLevel->m_pParaProps->m_tabStops);
}
/*!
 * This returns returns a pointer to the tab Type vector defined in the list level.
 */
std::vector<eTabType>* RTF_msword97_listOverride::getTabTypeVect(UT_uint32 iLevel)
{
	RTF_msword97_level * pLevel = m_pList->m_RTF_level[iLevel];
	return &(pLevel->m_pParaProps->m_tabTypes);
}
/*!
 * This returns returns a pointer to the tab Leadervector defined in the list level.
 */
std::vector<eTabLeader>* RTF_msword97_listOverride::getTabLeaderVect(UT_uint32 iLevel)
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
	UT_return_val_if_fail(pLevel && pLevel->m_pbParaProps, false);
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
	UT_return_val_if_fail(pLevel && pLevel->m_pbCharProps, false);
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
	UT_return_val_if_fail(pLevel && pLevel->m_pbCharProps, false);
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
	UT_return_val_if_fail(pLevel && pLevel->m_pbCharProps, false);
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
	UT_return_val_if_fail(pLevel && pLevel->m_pbCharProps, false);
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
	UT_return_val_if_fail(pLevel && pLevel->m_pbCharProps, false);
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
	UT_return_val_if_fail(pLevel && pLevel->m_pbCharProps, false);
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
	UT_return_val_if_fail(pLevel && pLevel->m_pbCharProps, false);
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
	UT_return_val_if_fail(pLevel && pLevel->m_pbCharProps, false);
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
	UT_return_val_if_fail(pLevel && pLevel->m_pbCharProps, false);
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
	UT_return_val_if_fail(pLevel && pLevel->m_pbCharProps, false);
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
	UT_return_val_if_fail(pLevel && pLevel->m_pbCharProps, false);
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
	UT_return_val_if_fail(pLevel && pLevel->m_pbCharProps, false);
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
RTFFontTableItem::RTFFontTableItem(FontFamilyEnum fontFamily, int charSet, 
                                   int codepage, FontPitch pitch,
                                   const char* panose, const char*
                                   pFontName, const char* pAlternativeFontName)
{
	m_family = fontFamily;
	m_charSet = charSet;
	m_codepage = codepage;
	m_szEncoding = 0;
	m_pitch = pitch;
	if (panose)
		memcpy(m_panose, panose, 10*sizeof(unsigned char));
	// TODO KAY: If we run out of memory then m_pFontName and m_pAlternativeFontName,
	// get left as NULL. Could we throw an exception from here?
	m_pFontName = g_strdup(pFontName);
	m_pAlternativeFontName = g_strdup(pAlternativeFontName);

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

			// 437: IBM
		case 437:
			m_szEncoding = "CP437";
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
	else if (m_charSet != -1)  // -1 indicated "not defined".
	{
		switch (m_charSet)
		{
			case 0:		// ANSI_CHARSET
				m_szEncoding = "CP1252";	// MS-ANSI
				break;
			case 2:		// SYMBOL_CHARSET
				m_szEncoding = NULL;	// MS-ANSI
				UT_DEBUGMSG(("RTF Font charset 'Symbol' worked around \n"));
				break;
			case 77:    // Source Vlad Harchev from OpenOffice
				m_szEncoding = "MACINTOSH";
				break;
			case 78:    // fjf: some kind of Japanese, let's guess:
				m_szEncoding = "SJIS";
				break;
			case 102:    // fjf: some kind of Chinese, let's guess:
				CPNAME_OR_FALLBACK(m_szEncoding,"CP936","GBK");
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
				// fall through (for now)
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
				m_szEncoding = "CP437";
				break;
			case 255:	// OEM_CHARSET
				// TODO Can iconv do this?
				UT_DEBUGMSG(("RTF Font charset 'OEM'??\n"));
				UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
				break;
			default:
				UT_DEBUGMSG(("RTF Font charset unknown: %d\n", m_charSet));
				// don't assert like we used to do. just silently ignore.
		}
	}
}

RTFFontTableItem::~RTFFontTableItem()
{
	g_free(m_pFontName);
	g_free(m_pAlternativeFontName);
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
	m_dir = UT_BIDI_UNSET;
	m_dirOverride = UT_BIDI_UNSET;
	m_Hidden = false;
	m_eRevision = PP_REVISION_NONE;
	m_iCurrentRevisionId = 0;
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
	m_dir = UT_BIDI_UNSET;
	m_tableLevel = 1; // Has to be 1 because the RTF spec has itap defaulting 
	                  // to this value
	m_bInTable = false;
	m_eRevision = PP_REVISION_NONE;
	m_iCurrentRevisionId = 0;
	m_bMergeBordersShading = false;
	m_bBotBorder = false;
	m_iBotBorderStyle = 0; // Number to represent style of border
	m_iBotBorderCol = 0; // index to color table
	m_iBotBorderWidth = 10;  // Thickness in twips
	m_iBotBorderSpacing = 10; // Spacing to text in twips
	m_bLeftBorder =  false;
	m_iLeftBorderStyle = 0; // Number to represent style of border
	m_iLeftBorderCol = 0; // index to color table
	m_iLeftBorderWidth = 10;  // Thickness in twips
	m_iLeftBorderSpacing = 10; // Spacing to text in twips
	m_bRightBorder = false;
	m_iRightBorderStyle = 0; // Number to represent style of border
	m_iRightBorderCol = 0; // index to color table
	m_iRightBorderWidth = 10;  // Thickness in twips
	m_iRightBorderSpacing = 10; // Spacing to text in twips
	m_bTopBorder = false;
	m_iTopBorderStyle = 0; // Number to represent style of border
	m_iTopBorderCol = 0; // index to color table
	m_iTopBorderWidth = 10;  // Thickness in twips
	m_iTopBorderSpacing = 0; // Spacing to text in twips
	m_iCurBorder = -1; // 0=bot,1=left,2=right,3=top
	m_iShadingPattern = 0; // Number to represent the style of shading
	m_iShadingForeCol = -1; // The Foreground color
	m_iShadingBackCol = -1; // The Background color

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

		if (!other.m_tabStops.empty())
		{
			m_tabStops.insert(m_tabStops.end(), other.m_tabStops.begin(), other.m_tabStops.end());
		}
		if (!other.m_tabTypes.empty())
		{
			m_tabTypes.insert(m_tabTypes.end(), other.m_tabTypes.begin(), other.m_tabTypes.end());
		}
		if (!other.m_tabLeader.empty())
		{
			m_tabLeader.insert(m_tabLeader.end(), other.m_tabLeader.begin(), other.m_tabLeader.end());
		}
		UT_ASSERT_HARMLESS(m_tabStops.size() ==	other.m_tabTypes.size() );
		UT_ASSERT_HARMLESS(m_tabStops.size() ==	other.m_tabLeader.size() );

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
		if(!m_tabTypes.empty())
		{
			m_curTabType = m_tabTypes.at(0);
			m_curTabLeader = m_tabLeader.at(0);
		}
		else
		{
			m_curTabType = FL_TAB_LEFT;
			m_curTabLeader = FL_LEADER_NONE;
		}
		m_rtfListTable = other.m_rtfListTable;
		m_styleNumber = other.m_styleNumber;
		m_bInTable = other.m_bInTable;
		m_bMergeBordersShading = other.m_bMergeBordersShading;
		m_bBotBorder = other.m_bBotBorder;
		m_iBotBorderStyle = other.m_iBotBorderStyle;
		m_iBotBorderCol = other.m_iBotBorderCol;
		m_iBotBorderWidth = other.m_iBotBorderWidth;
		m_iBotBorderSpacing = other.m_iBotBorderSpacing;
		m_bLeftBorder = other.m_bLeftBorder;
		m_iLeftBorderStyle = other.m_iLeftBorderStyle;
		m_iLeftBorderCol = other.m_iLeftBorderCol;
		m_iLeftBorderWidth = other.m_iLeftBorderWidth;
		m_iLeftBorderSpacing = other.m_iLeftBorderSpacing;
		m_bRightBorder = other.m_bRightBorder;
		m_iRightBorderStyle = other.m_iRightBorderStyle;
		m_iRightBorderCol = other.m_iRightBorderCol;
		m_iRightBorderWidth = other.m_iRightBorderWidth;
		m_iRightBorderSpacing = other.m_iRightBorderSpacing;
		m_bTopBorder = other.m_bTopBorder;
		m_iTopBorderStyle = other.m_iTopBorderStyle;
		m_iTopBorderCol = other.m_iTopBorderCol;
		m_iTopBorderWidth = other.m_iTopBorderWidth;
		m_iTopBorderSpacing = other.m_iTopBorderSpacing;
		m_iCurBorder= other.m_iCurBorder;
		m_iShadingPattern = other.m_iShadingPattern;
		m_iShadingForeCol = other.m_iShadingForeCol;
		m_iShadingBackCol = other.m_iShadingBackCol;
	}

	m_dir = other.m_dir;
	m_tableLevel = other.m_tableLevel;

	return *this;
}

RTFProps_ImageProps::RTFProps_ImageProps()
{
	sizeType = ipstNone;
	wGoal = hGoal = width = height = 0;
	scaleX = scaleY = 100;
	bCrop = false;
	cropt = cropb = cropl = cropr = 0;
}

RTFProps_CellProps::RTFProps_CellProps()
{
	m_bVerticalMerged = false;
	m_bVerticalMergedFirst = false;
	m_bHorizontalMerged = false;
	m_bHorizontalMergedFirst = false;
	m_sCellProps.clear();
	m_iCurBorder = rtfCellBorderTop;
	m_bLeftBorder = false;
	m_bRightBorder = false;
	m_bTopBorder = false;
	m_bBotBorder= false;
	m_iCellx = 0;
}

RTFProps_CellProps& RTFProps_CellProps::operator=(const RTFProps_CellProps& other)
{
	if (this != &other)
	{
		 m_bVerticalMerged = other.m_bVerticalMerged;
		 m_bVerticalMergedFirst = other. m_bVerticalMergedFirst;
		 m_bHorizontalMerged = other.m_bHorizontalMerged;
		 m_bHorizontalMergedFirst = other. m_bHorizontalMergedFirst;
		 m_sCellProps = other.m_sCellProps;
		 m_iCurBorder = other.m_iCurBorder;
		 m_bLeftBorder = other.m_bLeftBorder;
		 m_bRightBorder = other.m_bRightBorder;
		 m_bTopBorder = other.m_bTopBorder;
		 m_bBotBorder= other.m_bBotBorder;
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
	m_headerYTwips = 720;
	m_footerYTwips = 720;
	m_gutterTwips = 0;
    m_colSpaceTwips = 0;
	m_dir = UT_BIDI_UNSET;
}


RTFStateStore::RTFStateStore()
{
	m_destinationState = rdsNorm;
	m_internalState = risNorm;
	m_unicodeAlternateSkipCount = 1;
	m_unicodeInAlternate = 0;
	m_bInKeywordStar = false;
}


RTFStateStore * RTFStateStore::clone(void)
{
	RTFStateStore * pNew = new RTFStateStore();
	pNew->m_destinationState = m_destinationState;
	pNew->m_charProps = m_charProps;
	pNew->m_paraProps = m_paraProps;
	pNew->m_sectionProps = m_sectionProps;
	pNew->m_cellProps = m_cellProps;
	pNew->m_tableProps = m_tableProps;
	pNew->m_unicodeAlternateSkipCount = m_unicodeAlternateSkipCount;
	pNew->m_unicodeInAlternate = m_unicodeInAlternate;
	pNew->m_revAttr = m_revAttr;
	return pNew;
}

/*****************************************************************/
/*****************************************************************/

IE_Imp_RTF::IE_Imp_RTF(PD_Document * pDocument)
:	IE_Imp(pDocument),
	m_gbBlock(1024),
	m_szFileDirName(NULL),
	m_groupCount(0),
	m_newParaFlagged(false),
	m_newSectionFlagged(false),
	m_cbBin(0),
	m_currentHdrID(-1),
	m_currentFtrID(-1),
	m_currentHdrEvenID(-1),
	m_currentFtrEvenID(-1),
	m_currentHdrFirstID(-1),
	m_currentFtrFirstID(-1),
	m_currentHdrLastID(-1),
	m_currentFtrLastID(-1),
	m_numLists(0),
	m_pImportFile(NULL),
	m_pPasteBuffer(NULL),
	m_lenPasteBuffer(0),
	m_pCurrentCharInPasteBuffer(NULL),
	deflangid(0),
	m_mbtowc (XAP_EncodingManager::get_instance()->getNative8BitEncodingName()),
	m_parsingHdrFtr(false),
	m_icurOverride(0),
	m_icurOverrideLevel(0),
	m_bAppendAnyway(false),
	m_TableControl(pDocument),
	m_lastCellSDH(NULL),
	m_bNestTableProps(false),
	m_bParaWrittenForSection(false),
	m_bCellBlank(true),
	m_bEndTableOpen(false),
	m_bInFootnote(false),
	m_iDepthAtFootnote(0),
	m_iLastFootnoteId((UT_uint32)pDocument->getUID(UT_UniqueId::Footnote)),
	m_iLastEndnoteId((UT_uint32)pDocument->getUID(UT_UniqueId::Endnote)),
	m_iHyperlinkOpen(0),
	m_iRDFAnchorOpen(0),
	m_bBidiMode(false),
	m_bFootnotePending(false),
	m_bFtnReferencePending(false),
	m_bNoteIsFNote(true),
	m_bStyleImportDone(false),
	m_bCellHandled(false),
	m_bContentFlushed(false),
	m_bRowJustPassed(false),
	m_iStackLevelAtRow(0),
	m_bDoCloseTable(false),
	m_iNoCellsSinceLastRow(0),
	m_bFieldRecognized(false),
	m_iIsInHeaderFooter(0),
	m_bSectionHasPara(false),
	m_bStruxInserted(false),
	m_bStruxImage(false),
	m_bFrameStruxIn(false),
	m_iAutoBidiOverride(UT_BIDI_UNSET),
	m_iBidiLastType(UT_BIDI_UNSET),
	m_iBidiNextType(UT_BIDI_UNSET),
	m_szDefaultEncoding(NULL),
	m_iDefaultFontNumber(-1),
	m_dPosBeforeFootnote(0),
	m_bMovedPos(true),
	m_pAnnotation(NULL),
	m_pDelayedFrag(NULL),
	m_posSavedDocPosition(0),
	m_bInAnnotation(false),
	m_bFrameTextBox(false),
	m_bParaActive(false),
	m_bCellActive(false),
	m_ctMoveID("")
{
	UT_DEBUGMSG(("New ie_imp_RTF %p \n",this));
	m_sImageName.clear();
	if (!IE_Imp_RTF::keywordSorted) {
		_initialKeywordSort();
	}

	if(!m_vecAbiListTable.empty())
	{
		UT_std_vector_purgeall(m_vecAbiListTable);
	}
	m_mbtowc.setInCharset(XAP_EncodingManager::get_instance()->getNativeEncodingName());
	m_hyperlinkBase.clear();
	m_pasteTableStack.push(NULL);

	m_XMLIDCreatorHandle = getDoc()->makeXMLIDCreator();
}


IE_Imp_RTF::~IE_Imp_RTF()
{
	// Empty the state stack
	UT_DEBUGMSG(("In RTF destructor %p \n",this));
	while (m_stateStack.getDepth() > 0)
	{
		RTFStateStore* pItem = NULL;
		m_stateStack.pop((void**)(&pItem));
		UT_DEBUGMSG(("Deleting item %p in RTF destructor \n",pItem));
		delete pItem;
	}
	UT_DEBUGMSG(("Closing pastetable In RTF destructor %p \n",this));
	closePastedTableIfNeeded();
	UT_DEBUGMSG(("Deleting fonts In RTF destructor %p \n",this));

	// and the font table (can't use the macro as we allow NULLs in the vector
	UT_sint32 size = m_fontTable.size();
	UT_sint32 i =0;
	for (i = size-1; i>=0; i--)
	{
		RTFFontTableItem* pItem = m_fontTable.at(i);
		delete pItem;
	}

	// and the styleName table.
	UT_DEBUGMSG(("Deleting styles In RTF destructor %p \n",this));

	UT_DEBUGMSG(("Purging In RTF destructor %p \n",this));
	UT_std_vector_purgeall(m_vecAbiListTable);
	UT_std_vector_purgeall(m_hdrFtrTable);
	UT_std_vector_purgeall(m_vecWord97Lists);
	UT_std_vector_purgeall(m_vecWord97ListOverride);
	UT_DEBUGMSG(("SEVIOR:DOing last close \n"));
	while(getTable() && getTable()->wasTableUsed())
	{
		CloseTable(true);
	}
	FREEP (m_szFileDirName);
}

UT_Error IE_Imp_RTF::_loadFile(GsfInput * fp)
{
	m_newParaFlagged = true;
	m_newSectionFlagged = true;

	m_szFileDirName = g_strdup (gsf_input_name (fp));
	if(m_szFileDirName == NULL)
		m_szFileDirName = g_strdup("/tmp");
	// UT_basename returns a point INSIDE the passed string.
	// the trick is to truncate the string by setting the char pointed
	// by tmp to NULL. This IS useful code. (2 LOC)
	char * tmp = const_cast<char *>(UT_basename (m_szFileDirName));
	*tmp = 0;

	UT_Error error = _writeHeader(fp);

	if (!error)
	{
		error = _parseFile(fp);
		m_bAppendAnyway = true;
		_appendHdrFtr ();
	}

	// check if the doc is empty or not
	if (getDoc()->getLastFrag() == NULL)
	{
		error = UT_IE_BOGUSDOCUMENT;
	}

	return error;
}


UT_Error IE_Imp_RTF::_writeHeader(GsfInput * /*fp*/)
{
		return UT_OK;
}


bool IE_Imp_RTF::digVal(char ch, int& value, int base)
{
	value = ch - '0';

	return (value >= 0) && (value < base);
}

bool IE_Imp_RTF::hexVal(char c, int& value)
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

/*!
 * Opens a table by inserting a table and cell strux. The boolean argument
 * bDontFlush is false by default. If true the current stored chars are
 * not flushed.
 */
void IE_Imp_RTF::OpenTable(bool bDontFlush)
{
	if(bUseInsertNotAppend())
	{
		return;
	}
	if(!m_bParaWrittenForSection)
	{
		if(!bDontFlush)
		{
			m_newParaFlagged = false;
			FlushStoredChars(true); // MES 30/3/2005
		}
	}
	else
	{
		if(!bDontFlush)
		{
			FlushStoredChars();
		}
	}
	if(m_bInFootnote)
	{
		UT_DebugOnly<bool> ok =true;
		if(!bUseInsertNotAppend())
		{
			if(m_bNoteIsFNote)  
				getDoc()->appendStrux(PTX_EndFootnote,NULL);
			else
				getDoc()->appendStrux(PTX_EndEndnote,NULL);
				
		}
		else
		{
			if(m_bNoteIsFNote)
				ok = insertStrux(PTX_EndFootnote);
			else
				ok = insertStrux(PTX_EndEndnote);
			UT_ASSERT(ok);
			if(	m_bMovedPos)
			{
				m_bMovedPos = false;
				m_dposPaste += m_dPosBeforeFootnote; // restore old position
			}
		}
		m_bInFootnote = false;
		m_iDepthAtFootnote = 0;
	}
	m_TableControl.OpenTable();
//
// Need to have a block to append a table to
//
	if((m_TableControl.getNestDepth() > 1) && m_bCellBlank)
	{
			UT_DEBUGMSG(("Append block 6 \n"));

			getDoc()->appendStrux(PTX_Block,NULL);
	}
	getDoc()->appendStrux(PTX_SectionTable,NULL);
	UT_DEBUGMSG(("SEVIOR: Appending Table strux to doc nestdepth %d \n", m_TableControl.getNestDepth()));
	UT_ASSERT( m_TableControl.getNestDepth() < 2);
	PT_DocPosition posEnd=0;
	getDoc()->getBounds(true,posEnd); // clean frags!
	pf_Frag_Strux* sdh = getDoc()->getLastStruxOfType(PTX_SectionTable);
	UT_DEBUGMSG(("SEVIOR: Table strux sdh is %p \n",sdh));
	getTable()->setTableSDH(sdh);
	getTable()->OpenCell();
	if(!bDontFlush)
	{
		FlushCellProps();
		ResetCellAttributes();
	}
	getDoc()->appendStrux(PTX_SectionCell,NULL);
	getDoc()->getBounds(true,posEnd); // clean frags!
	sdh = getDoc()->getLastStruxOfType(PTX_SectionCell);
	getCell()->setCellSDH(sdh);
	m_currentRTFState.m_cellProps = RTFProps_CellProps();
	m_currentRTFState.m_tableProps = RTFProps_TableProps();
	m_lastCellSDH = NULL; // This is in the table structure and can be deleted from there.
	m_bCellBlank = true;
//	m_iNoCellsSinceLastRow = 0;
}

/*!
 * This Method saves the information about the current row
 */
void IE_Imp_RTF::SaveRowInfo(void)
{
}

/*!
 * This Method Clears any information about the current row
 */
void IE_Imp_RTF::RemoveRowInfo(void)
{
}

UT_sint32 IE_Imp_RTF::getPasteDepth(void)
{
	return m_pasteTableStack.getDepth();
}

/*!
 * Close off pasted Tables
 */
void IE_Imp_RTF::closePastedTableIfNeeded(void)
{
	while(m_pasteTableStack.getDepth() > 0)
	{
		ABI_Paste_Table * pPaste = NULL;
		m_pasteTableStack.pop((void**)(&pPaste));
		if(pPaste != NULL)
		{
			if(pPaste->m_bHasPastedCellStrux && !pPaste->m_bHasPastedBlockStrux)
			{
				insertStrux(PTX_Block);
				UT_DEBUGMSG(("Paste block in destructor 1 \n"));
			}
			if(pPaste->m_bHasPastedCellStrux)
			{
				insertStrux(PTX_EndCell);
				UT_DEBUGMSG(("Paste EndCell in destructor 1 \n"));
			}
			if(!pPaste->m_bPasteAfterRow)
			{
//
// Now fill out any remaining cells needed to finish the row of the table
//
				UT_sint32 i = pPaste->m_iCurRightCell;
				std::string sTop =  UT_std_string_sprintf("%d",pPaste->m_iCurTopCell);
				std::string sBot =  UT_std_string_sprintf("%d",pPaste->m_iCurTopCell+1);
				std::string sCellProps;
				std::string sVal;
				std::string sDum;
				const gchar * attrs[3] = {"props",NULL,NULL};
				for(i = pPaste->m_iCurRightCell; i<pPaste->m_iMaxRightCell; i++)
				{
					sCellProps.clear();
					sVal = UT_std_string_sprintf("%d",i);
					sDum = "left-attach";
					UT_std_string_setProperty(sCellProps,sDum,sVal);
					sVal = UT_std_string_sprintf("%d",i+1);
					sDum = "right-attach";
					UT_std_string_setProperty(sCellProps,sDum,sVal);
					sDum = "top-attach";
					UT_std_string_setProperty(sCellProps,sDum,sTop);
					sDum = "bot-attach";
					UT_std_string_setProperty(sCellProps,sDum,sBot);
					
					attrs[1] = sCellProps.c_str();
					insertStrux(PTX_SectionCell,attrs,NULL);
					
					insertStrux(PTX_Block);
					
					insertStrux(PTX_EndCell);
				}
				if(pPaste->m_bHasPastedTableStrux)
				{
					insertStrux(PTX_EndTable);
					
					insertStrux(PTX_Block); 
				}
			}
			else
			{
//
// Close off pasted rows by incrementing the top and botton's of the cell's
// below
//
				UT_sint32 numRows = pPaste->m_iNumRows;
				pf_Frag_Strux* sdhCell = NULL;
				pf_Frag_Strux* sdhTable = NULL;
				pf_Frag_Strux* sdhEndTable = NULL;
				bool b = getDoc()->getStruxOfTypeFromPosition(m_dposPaste,PTX_SectionTable,&sdhTable);
				PT_DocPosition posTable = getDoc()->getStruxPosition(sdhTable);
				UT_ASSERT(b);
				sdhEndTable = getDoc()->getEndTableStruxFromTableSDH(sdhTable);
				UT_ASSERT(sdhEndTable);
				PT_DocPosition posEndTable = getDoc()->getStruxPosition(sdhEndTable);
				b = getDoc()->getStruxOfTypeFromPosition(m_dposPaste-1,PTX_SectionCell,&sdhCell);
				b = getDoc()->getNextStruxOfType(sdhCell,PTX_SectionCell,&sdhCell);
				std::string sTop;
				std::string sBot;
				const char * szVal = NULL;
				const gchar * sProps[5] = {NULL,NULL,NULL,NULL};
				PT_DocPosition posCell = 0;
				if(b)
				{
					posCell = getDoc()->getStruxPosition(sdhCell);
				}
				while(b && (posCell < posEndTable))
				{
					getDoc()->getPropertyFromSDH(sdhCell,
												 true,
												 PD_MAX_REVISION,
												 "top-attach", &szVal);
					UT_ASSERT(szVal);
					UT_sint32 iTop = atoi(szVal);
					iTop += numRows;
					sTop = UT_std_string_sprintf("%d",iTop);
					getDoc()->getPropertyFromSDH(sdhCell,
												 true,
												 PD_MAX_REVISION,
												 "bot-attach", &szVal);
					UT_ASSERT(szVal);
					UT_sint32 iBot = atoi(szVal);
					iBot += numRows;
					sBot = UT_std_string_sprintf("%d",iBot);
					UT_DEBUGMSG(("Change cell top to %d bot to %d \n",iTop,iBot));
					sProps[0] = "top-attach";
					sProps[1] = sTop.c_str();
					sProps[2] = "bot-attach";
					sProps[3] = sBot.c_str();
					getDoc()->changeStruxFmt(PTC_AddFmt,posCell+1,posCell+1,NULL,sProps,PTX_SectionCell);
					b = getDoc()->getNextStruxOfType(sdhCell,PTX_SectionCell,&sdhCell);
					if(b)
					{
						posCell = getDoc()->getStruxPosition(sdhCell);
					}
				}
//
// Now a change strux on the table to make it rebuild.
//
				sProps[0] = "list-tag";
				std::string sVal = UT_std_string_sprintf("%d",getDoc()->getUID(UT_UniqueId::List));
				sProps[1] = sVal.c_str();
				sProps[2] = NULL;
				sProps[3] = NULL;
				getDoc()->changeStruxFmt(PTC_AddFmt,posTable+1,posTable+1,NULL,sProps,PTX_SectionTable);
			}
			delete pPaste;
		}
	}
}
/*!
 * Closes the current table. Does all the book keeping of inserting
 * endstruxs and deleting used ones.
 */ 
void IE_Imp_RTF::CloseTable(bool bForce /* = false */)
{
//
// Close table removes extraneous struxes like unmatched PTX_SectionCell's
//
	if(!bForce && (bUseInsertNotAppend() || (getTable() == NULL)))
	{
		return;
	}
	if(getTable() && getTable()->wasTableUsed())
	{
		UT_DEBUGMSG(("SEVIOR: Table used appened end Table, block \n"));
		if(m_lastCellSDH != NULL )
		{
			getDoc()->insertStruxNoUpdateBefore(m_lastCellSDH,PTX_EndTable,NULL);
//
// Need this one for dp_Instructions. Sevior
//
			getDoc()->insertStruxNoUpdateBefore(m_lastCellSDH,PTX_Block,NULL);
			pf_Frag_Strux* cellSDH = m_lastCellSDH;
			getDoc()->deleteStruxNoUpdate(cellSDH);
			m_bEndTableOpen = true;
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
			pf_Frag_Strux* cellSDH = m_lastCellSDH;
			getDoc()->deleteStruxNoUpdate(cellSDH);
			m_lastCellSDH = NULL;
		}
		m_TableControl.CloseTable();
		m_bEndTableOpen = true;
		UT_DEBUGMSG(("SEVIOR: Table not used. \n"));
	}
	else
	{
		if(m_lastCellSDH != NULL )
		{
			pf_Frag_Strux* cellSDH = m_lastCellSDH;
			getDoc()->deleteStruxNoUpdate(cellSDH);
			m_lastCellSDH = NULL;
		}
	}
}

void IE_Imp_RTF::HandleCell(void)
{
//
// Look if the has been some text output before this with an open row. If
// so, close the table and make copy of the last cells.
//
	UT_DEBUGMSG(("Handle Cell \n"));
	if(m_bRowJustPassed && m_bDoCloseTable && (getTable()!= NULL))
	{
		UT_GenericVector<ie_imp_cell *> vecOldCells;
		UT_GenericVector<ie_imp_cell *> vecCopyCells;
		UT_sint32 row = getTable()->getRow();
		getTable()->getVecOfCellsOnRow(row-1, &vecOldCells);
		UT_sint32 i =0;
		for(i=0; i< vecOldCells.getItemCount();i++)
		{
			ie_imp_cell * pCell = vecOldCells.getNthItem(i);
			ie_imp_cell * pNewCell = new ie_imp_cell(NULL,NULL,NULL,0);
			pNewCell->copyCell(pCell);
			vecCopyCells.addItem(pNewCell);
		}
		UT_ASSERT_HARMLESS(vecOldCells.getItemCount() > 0);
		CloseTable();
		OpenTable(true);
		for(i=0; i< vecCopyCells.getItemCount();i++)
		{
			ie_imp_cell * pCopyCell = vecCopyCells.getNthItem(i);
			if(i>0)
			{
//
// Already open openned from OpenTable()
//
				getTable()->OpenCell();
			}
			ie_imp_cell * pCell = getTable()->getNthCellOnRow(i);
			pCell->copyCell(pCopyCell);
			xxx_UT_DEBUGMSG(("Got Cell number %d CellX %d \n",i,pCell->getCellX()));
		}
		UT_VECTOR_PURGEALL(ie_imp_cell *, vecCopyCells);
	}
	m_bRowJustPassed = false;
	m_bCellHandled = true;
	m_bDoCloseTable = false;
	m_iNoCellsSinceLastRow++;
	UT_DEBUGMSG(("Num Cell on row %d \n",m_iNoCellsSinceLastRow));
	if(bUseInsertNotAppend())
	{
		return;
	}
	if(m_bCellBlank && (m_gbBlock.getLength() == 0))
	{
	    UT_DEBUGMSG(("Append block 7 \n"));

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
	pf_Frag_Strux* sdh = getDoc()->getLastStruxOfType(PTX_SectionCell);
	ie_imp_cell * pCell = getTable()->getNthCellOnRow(getTable()->getPosOnRow());
	UT_return_if_fail(sdh);
	if(!pCell)
	{
//
// Cell class doesn't exist so create it.
//
		UT_sint32 pos  = getTable()->OpenCell();
		getTable()->setPosOnRow(pos);
		xxx_UT_DEBUGMSG(("SEVIOR: created cell %p for posOnRow %d \n",getCell(),getTable()->getPosOnRow()));
	}
	xxx_UT_DEBUGMSG(("SEVIOR: set cell %p sdh %p  at pos %d on row %d \n",getCell(),sdh,getTable()->getPosOnRow(),getTable()->getRow()));
	getTable()->setNthCellOnThisRow(getTable()->getPosOnRow());
	if(getCell()->isMergedAbove())
	{
		xxx_UT_DEBUGMSG(("Cell %x is merged Above \n"));
	} 
	if(getCell()->isMergedLeft())
	{
		xxx_UT_DEBUGMSG(("Cell %x is merged left \n"));
	} 

	if(!getCell()->isMergedAbove() && !getCell()->isMergedLeft())
	{
		getCell()->setCellSDH(sdh);
		xxx_UT_DEBUGMSG(("SEVIOR: At posOnRow %d cellx %d \n",getTable()->getPosOnRow(),getCell()->getCellX()));
		getTable()->incPosOnRow();
		FlushStoredChars();		
		xxx_UT_DEBUGMSG(("SEVIOR: Non posonrow %d \n",getTable()->getPosOnRow()));
		getDoc()->appendStrux(PTX_EndCell,NULL);
//
// Look to see if this is just has a cell/endCell with no content. If so
// repair it.
//
		pf_Frag_Strux* sdhEndCell = getDoc()->getLastStruxOfType(PTX_EndCell);
		if(getDoc()->isStruxBeforeThis(sdhEndCell,PTX_SectionCell))
		{
			UT_DEBUGMSG(("Insert Block before frag 1 \n"));
			getDoc()->insertStruxNoUpdateBefore(sdhEndCell,PTX_Block,NULL);
			const pf_Frag * pf = static_cast<const pf_Frag *>(sdhEndCell);
			getDoc()->insertFmtMarkBeforeFrag(const_cast<pf_Frag *>(pf));
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
	if(bUseInsertNotAppend())
	{
		return;
	}
	if(m_currentRTFState.m_cellProps.m_bVerticalMerged)
	{
		UT_DEBUGMSG(("Set merged above to cell %p \n",getCell()));
	}
	if(m_currentRTFState.m_cellProps.m_bHorizontalMerged)
	{
		UT_DEBUGMSG(("Set merged left to cell %p \n",getCell()));
	}
	getCell()->setMergeAbove( m_currentRTFState.m_cellProps.m_bVerticalMerged );
	getCell()->setFirstVerticalMerge( m_currentRTFState.m_cellProps.m_bVerticalMergedFirst );
	getCell()->setFirstHorizontalMerge( m_currentRTFState.m_cellProps.m_bHorizontalMergedFirst );
	getCell()->setMergeLeft( m_currentRTFState.m_cellProps.m_bHorizontalMerged );
	std::string sProp;
	std::string sVal;
	if(!m_currentRTFState.m_cellProps.m_bBotBorder)
	{
		sProp = "bot-style";
		sVal = "none";
		UT_std_string_setProperty(m_currentRTFState.m_cellProps.m_sCellProps,sProp,sVal);
	}
	if(!m_currentRTFState.m_cellProps.m_bTopBorder)
	{
		sProp = "top-style";
		sVal = "none";
		UT_std_string_setProperty(m_currentRTFState.m_cellProps.m_sCellProps,sProp,sVal);
	}
	if(!m_currentRTFState.m_cellProps.m_bLeftBorder)
	{
		sProp = "left-style";
		sVal = "none";
		UT_std_string_setProperty(m_currentRTFState.m_cellProps.m_sCellProps,sProp,sVal);
	}
	if(!m_currentRTFState.m_cellProps.m_bRightBorder)
	{
		sProp = "right-style";
		sVal = "none";
		UT_std_string_setProperty(m_currentRTFState.m_cellProps.m_sCellProps,sProp,sVal);
	}
	getCell()->addPropString( m_currentRTFState.m_cellProps.m_sCellProps );
	
}

/*!
 * Set a property, value pair in the supplied string. This is just a convience
 * wrapper function to use const char * strings
 */
void IE_Imp_RTF::_setStringProperty(std::string & sPropsString, 
                                    const char * szProp, const char * szVal)
{
	std::string sProp(szProp);
	std::string sVal(szVal);
	UT_std_string_setProperty(sPropsString,sProp,sVal);
}


void IE_Imp_RTF::FlushTableProps(void)
{
	if(bUseInsertNotAppend())
	{
		return;
	}
	getTable()->setAutoFit( m_currentRTFState.m_tableProps.m_bAutoFit );
}

void IE_Imp_RTF::HandleCellX(UT_sint32 cellx)
{
	if(bUseInsertNotAppend())
	{
		return;
	}

	if(getTable() == NULL)
	{
		OpenTable();
	}
//	UT_ASSERT_HARMLESS(cellx != 3652);
	UT_sint32 iRow = 0;
	bool bNewCell = true;
//
// Look to see if a cell with cellx already exists on the current row. If so set the
// current cell pointer to point to it.
//
	UT_DEBUGMSG(("Original cellx %d \n",cellx));
	iRow = getTable()->getRow();
	ie_imp_cell * pOldCell = getTable()->getCellAtRowColX(iRow,cellx);
	if(pOldCell && !m_currentRTFState.m_cellProps.m_bHorizontalMergedFirst && !m_currentRTFState.m_cellProps.m_bHorizontalMerged )
	{
		bNewCell = false;
		getTable()->setCell(pOldCell);
		cellx = pOldCell->getCellX();
	}
	if(!pOldCell)
	{
		pOldCell = getTable()->getNthCellOnRow(getTable()->getCellXOnRow());
		xxx_UT_DEBUGMSG(("SEVIOR: Looking for cellx num %d on row %d found %p \n",getTable()->getCellXOnRow(),iRow,pOldCell));
		if(pOldCell)
		{
			bNewCell = false;
			getTable()->setCell(pOldCell);
		}
	}
	if(bNewCell)
	{
		getTable()->OpenCell();
		xxx_UT_DEBUGMSG(("SEVIOR: created cell %p for cellx %d on row \n",getCell(),cellx,getTable()->getRow()));
	}
	UT_ASSERT_HARMLESS(cellx>1);
	getTable()->setCellX(cellx);
	UT_DEBUGMSG(("set cellx for class %p to %d \n",getCell(),cellx));
	getTable()->incCellXOnRow();
	FlushCellProps();
	ResetCellAttributes();
}

void IE_Imp_RTF::HandleRow(void)
{
	if(bUseInsertNotAppend())
	{
		return;
	}

	UT_DEBUGMSG(("ie_imp_RTF: Handle Row now NUm cells in row %d \n",m_iNoCellsSinceLastRow));
	if(m_iNoCellsSinceLastRow > 0)
	{
		m_TableControl.NewRow();
	}
	else
	{
		UT_DEBUGMSG(("One of those stupid rows without cells found. \n"));
		UT_DEBUGMSG(("Handle it now. RTF totally sucks. \n"));
		if(getTable()) {
			getTable()->removeCurrentRow();
			getDoc()->miniDump(m_lastCellSDH,8);
		}

		m_bCellBlank = true;
		UT_ASSERT_HARMLESS(0);
	}
//
// Need these for strange barely legal docs like that in bug 4111
//
	m_bCellHandled = false;
	m_bContentFlushed = false;
	m_bRowJustPassed = true;
	m_iStackLevelAtRow = m_stateStack.getDepth();
	m_bDoCloseTable = false;
	m_iNoCellsSinceLastRow = 0;
}


void IE_Imp_RTF::HandleNoteReference(void)
{
	// see if we have a reference marker pending ...
	const gchar * attribs[3] ={"footnote-id",NULL,NULL};

	if(!m_bNoteIsFNote)
	{
		attribs[0] = "endnote-id";
	}
	std::string footpid;
	if(m_bInFootnote && !m_bFtnReferencePending)
	{
		if(m_bNoteIsFNote)
		{
			footpid = UT_std_string_sprintf("%i",m_iLastFootnoteId);
		}
		else
		{
			footpid = UT_std_string_sprintf("%i",m_iLastEndnoteId);
		}
		attribs[1] = footpid.c_str();
		UT_DEBUGMSG(("Note anchor %s \n",footpid.c_str()));
		if(m_bNoteIsFNote)
		{
			_appendField ("footnote_anchor",attribs);
			return;
		}
		else
		{
			_appendField ("endnote_anchor",attribs);
			return;
		}
	}
	else if(m_bInFootnote && m_bFtnReferencePending)
	{
		// we have a pending reference mark; since the \footnote
		// has removed the RTF state, we need to temporarily
		// place the saved RTF state on the stack. We pop it afterwards
		RTFStateStore * pSaved = m_currentRTFState.clone();
		m_stateStack.push(pSaved);
		m_stateStack.push(&m_FootnoteRefState);
		m_currentRTFState = m_FootnoteRefState;
		if(m_bNoteIsFNote)
		{
			m_iLastFootnoteId = getDoc()->getUID(UT_UniqueId::Footnote);
			footpid = UT_std_string_sprintf("%i",m_iLastFootnoteId);
		}
		else
		{
			m_iLastEndnoteId = getDoc()->getUID(UT_UniqueId::Endnote);
			footpid = UT_std_string_sprintf("%i",m_iLastEndnoteId);
		}
		attribs[1] = footpid.c_str();
		UT_DEBUGMSG(("Note reference %s \n",footpid.c_str()));

		if(m_bNoteIsFNote)
		{
			_appendField ("footnote_ref",attribs);
		}
		else
		{
			_appendField ("endnote_ref",attribs);
		}
			
		m_bFtnReferencePending = false;

		// now we pop the saved state off and restore the current state
		RTFStateStore* pState = NULL;
		m_stateStack.pop(reinterpret_cast<void**>(&pState));
		m_stateStack.pop(reinterpret_cast<void**>(&pState));
		m_currentRTFState = *pState;
		DELETEP(pState);
	}
	else
	{
		m_bFtnReferencePending = true;
//
// Save current RTF state.
//
		m_FootnoteRefState = m_currentRTFState;
	}
}

void IE_Imp_RTF::HandleNote(void)
{
	
	m_bInFootnote = true;
	if(m_bFtnReferencePending)
	{
		HandleNoteReference();
	}
	else
	{
		// if there is no reference pending, this is a note with a
		// manually set marker; we have to flush characters before
		// inserting the footnote strux
		FlushStoredChars(true);
	}
	
	m_iDepthAtFootnote = m_stateStack.getDepth();
	const gchar * attribs[3] ={"footnote-id",NULL,NULL};

	if(!m_bNoteIsFNote)
	{
		attribs[0] = "endnote-id";
	}
	
	std::string footpid;
	if(m_bNoteIsFNote)
	{
	    footpid = UT_std_string_sprintf("%i",m_iLastFootnoteId);
	}
	else
	{
	    footpid = UT_std_string_sprintf("%i",m_iLastEndnoteId);
	}
	attribs[1] = footpid.c_str();
	UT_DEBUGMSG(("Note Strux ID = %s \n", footpid.c_str()));
	UT_DEBUGMSG(("ie_imp_RTF: Handle Footnote now \n"));

	if(!bUseInsertNotAppend())
	{
		if(m_bNoteIsFNote)
			getDoc()->appendStrux(PTX_SectionFootnote,attribs);
		else
			getDoc()->appendStrux(PTX_SectionEndnote,attribs);
			
		UT_DEBUGMSG(("Append block 8 \n"));
		getDoc()->appendStrux(PTX_Block,NULL);
	}
	else
	{
		if(m_bNoteIsFNote)
			insertStrux(PTX_SectionFootnote,attribs,NULL);
		else
			insertStrux(PTX_SectionEndnote,attribs,NULL);
			
		UT_DEBUGMSG((" Insert Block at 7 \n"));
		markPasteBlock();
		insertStrux(PTX_Block);
	}
}


void IE_Imp_RTF::HandleAnnotation(void)
{
	UT_return_if_fail(m_pAnnotation);
	if(m_bInAnnotation)
	{
		UT_DEBUGMSG(("Recursive call to HandleAnnotion \n"));
		return;
	}
	m_bInAnnotation = true;
	std::string sAnnNum = UT_std_string_sprintf("%d",m_pAnnotation->m_iAnnNumber);
	const char * ann_attrs[5] = {NULL,NULL,NULL,NULL,NULL};
	ann_attrs[0] = "annotation-id";
	ann_attrs[1] = sAnnNum.c_str();
	ann_attrs[2] = 0;
	ann_attrs[3] = 0;
	const char * pszAnn[7] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	UT_sint32 i = 0;
	if(m_pAnnotation->m_sAuthor.size() > 0)
	{
		pszAnn[i] = "annotation-author";
		i++;
		pszAnn[i] = m_pAnnotation->m_sAuthor.utf8_str();
		i++;
	}
	if(m_pAnnotation->m_sTitle.size() > 0)
	{
		pszAnn[i] = "annotation-title";
		i++;
		pszAnn[i] = m_pAnnotation->m_sTitle.utf8_str();
		i++;
	}
	if(m_pAnnotation->m_sDate.size() > 0)
	{
		pszAnn[i] = "annotation-date";
		i++;
		pszAnn[i] = m_pAnnotation->m_sDate.utf8_str();
		i++;
	}
	if(!bUseInsertNotAppend())
	{
		PD_Document * doc = getDoc();
		m_pDelayedFrag = m_pAnnotation->m_pInsertFrag->getNext();
		if(!m_pDelayedFrag)
			m_pDelayedFrag = doc->getLastFrag();
		UT_DEBUGMSG(("Delayed Frag set to %p \n",m_pDelayedFrag));
		ann_attrs[2] = PT_PROPS_ATTRIBUTE_NAME;
		UT_sint32 k = 0;
		std::string sProperties;
		for(k=0; k<i;k++)
		{
			sProperties += pszAnn[k];
			k++;
			sProperties += ":";
			sProperties += pszAnn[k];
			k++;
			if(k < i)
				sProperties += ";";
		}
		ann_attrs[3] = sProperties.c_str();
		UT_DEBUGMSG(("Appending annotation strux, props are %s \n", sProperties.c_str()));
		FlushStoredChars();
		if(!m_pDelayedFrag)
			m_pDelayedFrag = doc->getLastFrag();
		doc->insertStruxBeforeFrag(m_pDelayedFrag,PTX_SectionAnnotation,ann_attrs);
		doc->insertStruxBeforeFrag(m_pDelayedFrag,PTX_Block,NULL);
	}
	else
	{
		m_posSavedDocPosition = m_dposPaste;
		xxx_UT_DEBUGMSG(("Initial Saved doc Postion %d \n",	m_posSavedDocPosition));
		m_dposPaste = m_pAnnotation->m_Annpos+1;
		xxx_UT_DEBUGMSG((" Insert Annotation m_dposPaste %d \n",m_dposPaste));
		insertStrux(PTX_SectionAnnotation,ann_attrs,pszAnn);
		UT_DEBUGMSG((" Insert Block at 7 \n"));
		markPasteBlock();
		insertStrux(PTX_Block);
		xxx_UT_DEBUGMSG(("After first strux insert  Saved doc Postion %d \n",m_posSavedDocPosition));
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
	UT_sint32 iRTFStackDepth = m_stateStack.getDepth();
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
			if(m_bFootnotePending && c != '\\')
			{
				// not followed by a keyword, this is an ordinary
				// footnote
				m_bNoteIsFNote = true;
				HandleNote();
				m_bFootnotePending = false;
			}
			else if(m_bFootnotePending && c == '\\')
			{
				// need to see if the keyword is \ftnalt indicating
				// endnote
				unsigned char keyword[MAX_KEYWORD_LEN];
				UT_sint32 parameter = 0;
				bool parameterUsed = false;
				if (ReadKeyword(keyword, &parameter, &parameterUsed, MAX_KEYWORD_LEN))
				{
					if(0 == strcmp((const char*)&keyword[0], "ftnalt"))
					{
						UT_DEBUGMSG(("Have Endnote \n"));
						// we have an end-note
						m_bNoteIsFNote = false;
						HandleNote();
						m_bFootnotePending = false;
						continue;
					}
					else
					{
						// we have some other keyword ...
						m_bNoteIsFNote = true;
						HandleNote();
						m_bFootnotePending = false;
						
						TranslateKeyword(keyword, parameter, parameterUsed);
						continue;
					}
				}
				else
				{
					// something seriously wrong ...
					UT_DEBUGMSG(("RTF: could not read keyword (l: %d)\n", __LINE__));
					continue;
				}
			}
			else if(m_pAnnotation && (m_pAnnotation->m_iRTFLevel > 0) && !m_bInAnnotation && (c != '\\') && (c != '{') && (c != '}'))
			{
				SkipBackChar(c);
				HandleAnnotation();
				continue;
			}
			
			switch (c)
			{
			case '{':
				ok = PushRTFState();
				if (!ok) {
					UT_DEBUGMSG(("PushRTFState()\n"));
				}
				break;
			case '}':
			{
				ok = PopRTFState();
				if (!ok) {
					UT_DEBUGMSG(("PopRTFState() bug\n"));
					bool bCont = true;
					char lastc =c;
					while(ReadCharFromFile(&c) && bCont)
					{
						lastc = c;
						bCont = (c == '}');
					}
					if(lastc == '}') // reached end of file with extra "}"
					{
						ok = true;
						break;
					}
					return UT_IE_TRY_RECOVER; // try to finish the import anyway
				}
				setEncoding(); // Reset encoding from current state.
			}
				break;
			case '\\':
			{
				ok = ParseRTFKeyword();

				if (!ok) {
					UT_DEBUGMSG(("ParseRTFKeyword() failed import aborted \n"));
					UT_DEBUGMSG(("Last valid keyword was %s \n",g_dbgLastKeyword));
				}
				break;
			}
					
			default:
				if (m_currentRTFState.m_internalState == RTFStateStore::risNorm)
				{
					ok = ParseChar(c, false);
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
						ok = ParseChar(b, false);
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

		if(getLoadStylesOnly() && m_bStyleImportDone)
			break;
	}

	if (ok && !getLoadStylesOnly())
	{
		// do not force this -- a correctly formed rtf document should end in \par keyword
		// so m_newParaFlagged will be set, but there will be nothing in the buffer, so no
		// para should be output. If the doc is malformed and there is some stuff after
		// the last \par, it will be output.
		ok = FlushStoredChars(false);
		if (!ok) {
			UT_DEBUGMSG(("FlushStoredChars()\n"));
		}
	}

	/* m_stateStack.getDepth() == 0 if the functions PushRTFState and PopRTFState
	   have been called the same number of times. Each call to PushRTFState on an
	   opening bracket ("{") should be followed by a call to PopRTFState on the
	   corresponding closing bracket ("}"). This check is done only if the function
	   has not been called recursively (iRTFStackDepth == 0).*/
	UT_ASSERT((m_stateStack.getDepth() == 0) || (iRTFStackDepth > 0));

//	UT_DEBUGMSG(("dumping document\n"));
//	getDoc()->__dump(stderr);
	return ok ? UT_OK : UT_ERROR;

}

/*
   Scans the entire document for any rtl tokens and set m_bBidiMode
   accordingly. Please note that this results in a minimal performance
   loss (a fraction of a second on a 30 page doc), and saves us much
   work and time if the document is LTR-only.
*/
UT_Error IE_Imp_RTF::_isBidiDocument()
{
	UT_return_val_if_fail(m_pImportFile, UT_ERROR);

	char buff[8192 + 1];
	char * token = NULL;
	
	size_t iBytes = UT_MIN(8192, gsf_input_remaining(m_pImportFile));
	gsf_input_read(m_pImportFile, iBytes, (guint8*)buff);

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
		
		iBytes = UT_MIN(8192, gsf_input_remaining(m_pImportFile));
		gsf_input_read(m_pImportFile, iBytes, (guint8*)buff);
	}

	if(token)
	{
		UT_DEBUGMSG(("IE_Imp_RTF::_isBidiDocument: found rtl token [%s]\n", token));
		m_bBidiMode = true;
	}
	else
	{
		UT_DEBUGMSG(("IE_Imp_RTF::_isBidiDocument: no rtl token found\n"));
		m_bBidiMode = false;
	}
	
	
	// reset the file pointer to the beginning
	if(gsf_input_seek(m_pImportFile, 0, G_SEEK_SET))
		return UT_ERROR;

	UT_DEBUGMSG(("IE_Imp_RTF::_isBidiDocument: looking for RTL tokens -- done\n"));
	return UT_OK;
}



UT_Error IE_Imp_RTF::_parseFile(GsfInput* fp)
{
	m_pImportFile = fp;

	m_currentRTFState.m_internalState = RTFStateStore::risNorm;
	m_currentRTFState.m_destinationState = RTFStateStore::rdsNorm;
	m_currentHdrID = -1;
	m_currentFtrID = -1;
	m_currentHdrEvenID = -1;
	m_currentFtrEvenID = -1;
	m_currentHdrFirstID = -1;
	m_currentFtrFirstID = -1;
	m_currentHdrLastID = -1;
	m_currentFtrLastID = -1;
#if 0
	if(m_pImportFile && UT_OK != _isBidiDocument())
		return UT_ERROR;
#endif
	if(m_pImportFile && !getLoadStylesOnly())
	{
		// need to init docs Attributes and props
		getDoc()->setAttrProp(NULL);
	}
	
//
// OK Set the Default page size, in case it isn't set in RTF
//
	if(!getLoadStylesOnly() && !m_parsingHdrFtr)
	{
		double width = 12240./1440.; // default width in twips
		double height = 15840./1440; // default height in twips
		if(fp != NULL)
		{
			getDoc()->m_docPageSize.Set(width,height,DIM_IN);
		}
	}
	return _parseText();
}

/*****************************************************************/
/*****************************************************************/

bool IE_Imp_RTF::HandleParKeyword()
{
	// NB: the \par keyword really represents '\r' and concludes a paragraph rather than
	// begins it -- some paragraph properties are indicated by formating applied to the
	// \par keyword, for example revisions are. This means that we sometimes have to
	// change fmt of the last block

	if(!m_bSectionHasPara || m_newParaFlagged)
	{
		if(m_newSectionFlagged)
			ApplySectionAttributes();
		
		m_newSectionFlagged = false;

		ApplyParagraphAttributes();
		
		//getDoc()->appendStrux(PTX_Block,NULL); // FIXME 28/3/2005!
		m_newParaFlagged = false;
		m_bSectionHasPara = true;
	}
	std::string sProps;
	int attrsIdx = 0;
	const gchar * attrs[7] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	const gchar * props = NULL;
	std::string rev;

	UT_return_val_if_fail( buildCharacterProps(sProps), false);
	props = sProps.c_str();

	if(m_currentRTFState.m_charProps.m_eRevision != PP_REVISION_NONE)
	{
		std::string aStyle;
		
		if(m_currentRTFState.m_charProps.m_styleNumber >= 0
		   && static_cast<UT_uint32>(m_currentRTFState.m_charProps.m_styleNumber) < m_styleTable.size())
		{
			aStyle = m_styleTable[m_currentRTFState.m_charProps.m_styleNumber];
		}

		_formRevisionAttr(rev, sProps, aStyle);
		attrs[attrsIdx++] = "revision";
		attrs[attrsIdx++] = rev.c_str();
		props = NULL;
	}

	if((props && *props) || attrs[0])
	{
		if(m_pImportFile)
		{
			UT_return_val_if_fail(getDoc()->appendLastStruxFmt(PTX_Block, attrs, props,true), false);
		}
		else
		{
			if(!getDoc()->isEndTableAtPos(m_dposPaste))
			{
				UT_return_val_if_fail(getDoc()->changeLastStruxFmtNoUndo(m_dposPaste, PTX_Block, attrs, props, true),false );
			}
		}
		
	}
	
	return StartNewPara();
}

// flush any remaining text in the previous para and flag
// a new para to be started.  Don't actually start a new
// para as it may turn out to be empty
//
bool IE_Imp_RTF::StartNewPara()
{
	// force this, if new para is flagge:d (so we import empty paragraphs)
	// if it is not, then we do not flagged, then we do not want new block appended (it
	// has already been done somewhere else
	bool ok = FlushStoredChars(m_newParaFlagged);
	m_newParaFlagged = true;

	// need to reset any left-over direction override
	m_currentRTFState.m_charProps.m_dirOverride = UT_BIDI_UNSET;
	return ok;
}


// flush any remaining text in the previous sction and
// flag a new section to be started.  Don't actually
// start a new section as it may turn out to be empty
//
bool IE_Imp_RTF::StartNewSection()
{
	// force this, if new para is flagged (so we import empty paragraphs)
	// if it is not, then we do not flagged, then we do not want new block appended (it
	// has already been done somewhere else
	bool ok = FlushStoredChars(m_newParaFlagged);

	m_newSectionFlagged = true;
	m_newParaFlagged = true;
	m_bSectionHasPara = false;
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

/*!
 * returns true if we've pasted a table strux and have not yet pasted a cell
 * or we've pasted an endcell and have not yet pasted a cell
 */
bool IE_Imp_RTF::isPastedTableOpen(void)
{
	ABI_Paste_Table * pPaste = NULL;
	if(m_pasteTableStack.getDepth() == 0)
	{
		return false;
	}
	m_pasteTableStack.viewTop(reinterpret_cast<void **>(&pPaste));
	if(pPaste == NULL)
	{
		return false;
	}
	if(!pPaste->m_bHasPastedCellStrux)
	{
		return true;
	}
	if(!pPaste->m_bHasPastedTableStrux)
	{
		return false;
	}
	return false;
}

// flush any stored text into the document
//
bool IE_Imp_RTF::FlushStoredChars(bool forceInsertPara)
{

	// start a new para if we have to
	bool ok = true;
//
// Don't insert anything if we're between a table strux and a cell or between
// cell's
//
	xxx_UT_DEBUGMSG(("Level at check %d \n",m_stateStack.getDepth()));
	if(isPastedTableOpen() && !forceInsertPara)
	{
		return true;
	}
	if (m_newSectionFlagged && (forceInsertPara || (m_gbBlock.getLength() > 0)) )
	{
		m_bContentFlushed = true;
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
			if(!bUseInsertNotAppend())
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
			ok = ApplyParagraphAttributes();
			if(m_newParaFlagged || m_bCellBlank)
			{
				UT_DEBUGMSG(("Append block 10 \n"));
				if(m_pDelayedFrag)
				{
					getDoc()->insertStruxBeforeFrag(m_pDelayedFrag,PTX_Block,NULL);
				}
				else
				{
					getDoc()->appendStrux(PTX_Block,NULL);
				}
			}
			m_bSectionHasPara = true;
			m_bCellBlank = false;
			m_bEndTableOpen = false;
		}
		else if( ok && m_bEndTableOpen)
		{
			UT_DEBUGMSG(("Append block 11 \n"));

			if(m_pDelayedFrag)
			{
				getDoc()->insertStruxBeforeFrag(m_pDelayedFrag,PTX_Block,NULL);
			}
			else
			{
				getDoc()->appendStrux(PTX_Block,NULL);
			}
			m_bSectionHasPara = true;
			m_bEndTableOpen = false;
		}
		ok = ApplyCharacterAttributes();
		m_bCellBlank = false;
	}
	if( ok && m_bInFootnote && (m_stateStack.getDepth() < m_iDepthAtFootnote))
	{
		if(!bUseInsertNotAppend())
		{
			if(m_bNoteIsFNote)
				getDoc()->appendStrux(PTX_EndFootnote,NULL);
			else
				getDoc()->appendStrux(PTX_EndEndnote,NULL);
				
		}
		else
		{
			if(m_bNoteIsFNote)
				ok = insertStrux(PTX_EndFootnote);
			else
				ok = insertStrux(PTX_EndEndnote);
			if(	m_bMovedPos)
			{
				m_bMovedPos = false;
				m_dposPaste += m_dPosBeforeFootnote; // restore old position
			}
		}
		m_bInFootnote = false;
		m_iDepthAtFootnote = 0;
	}
	xxx_UT_DEBUGMSG(("Annotation level at check %d \n",m_stateStack.getDepth()));
    if(ok && m_bInAnnotation && m_pAnnotation && (m_stateStack.getDepth() < m_pAnnotation->m_iRTFLevel))
		{
			//
			// Wind up the annotation
			m_bInAnnotation = false;
			xxx_UT_DEBUGMSG(("Finishing up the annotation depth %d RTFlevel %d \n",m_stateStack.getDepth(), m_pAnnotation->m_iRTFLevel ));
			if(!bUseInsertNotAppend())
			{
				FlushStoredChars();
				getDoc()->insertStruxBeforeFrag(m_pDelayedFrag,PTX_EndAnnotation,NULL);
					
			}
			else
			{
				xxx_UT_DEBUGMSG(("Inserting EndAnnoation at %d \n",m_dposPaste));
				getDoc()->insertStrux(m_dposPaste,PTX_EndAnnotation,NULL,NULL);	
				if(m_posSavedDocPosition > m_dposPaste)
					m_posSavedDocPosition++;
				m_dposPaste++;

			}
			EndAnnotation();
			DELETEP(m_pAnnotation);
			m_pDelayedFrag = NULL;
			xxx_UT_DEBUGMSG(("After complete annotation Saved doc Postion %d \n",m_posSavedDocPosition));
			m_dposPaste = m_posSavedDocPosition;
			m_posSavedDocPosition = 0;
			xxx_UT_DEBUGMSG(("Annotation insert complete \n"));
		}
//	if( ok && m_bFrameOpen && (static_cast<UT_sint32>(m_stateStack.getDepth()) < m_iStackDepthAtFrame))	
//	{
//		HandleEndShape();
//	}
	return ok;
}


// Get a font out of the font table, making sure we dont attempt to access off the end
RTFFontTableItem* IE_Imp_RTF::GetNthTableFont(UT_sint32 fontNum)
{
	if (static_cast<UT_uint32>(fontNum) < m_fontTable.size())
	{
		return m_fontTable.at(fontNum);
	}
	else
	{
		return NULL;
	}
}


// Get a colour out of the colour table, making sure we dont attempt to access off the end
UT_uint32 IE_Imp_RTF::GetNthTableColour(UT_sint32 colNum)
{
	if (static_cast<UT_uint32>(colNum) < m_colourTable.size())
	{
		return m_colourTable.at(colNum);
	}
	else
	{
		return 0;	// black
	}
}

UT_sint32 IE_Imp_RTF::GetNthTableBgColour(UT_sint32 colNum)
{
	if (static_cast<UT_uint32>(colNum) < m_colourTable.size())
	{
		return m_colourTable.at(colNum);
	}
	else
	{
		return -1;	// invalid
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
				if (!no_convert && ch<=0xff)
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
	UT_DEBUGMSG (("went thru all ParseChar() without doing anything\n"));
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
	UT_sint32 parameter = 0;
	bool parameterUsed = false;
	if (ReadKeyword(keyword, &parameter, &parameterUsed, MAX_KEYWORD_LEN))
	{
		xxx_UT_DEBUGMSG(("SEVIOR: keyword = %s  par= %d \n",keyword,parameter));
		bool bres = TranslateKeyword(keyword, parameter, parameterUsed);
		if(!bres)
		{
			UT_DEBUGMSG(("SEVIOR: error in translation last valid %s \n",g_dbgLastKeyword));
		}
		return bres;
	}
	else
	{
		UT_DEBUGMSG(("Error in ReadKeyword Last vaild %s \n",g_dbgLastKeyword));
		return false;
	}
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
bool IE_Imp_RTF::ReadKeyword(unsigned char* pKeyword, UT_sint32* pParam, bool* pParamUsed, UT_uint32 keywordBuffLen)
{
	bool fNegative = false;
	*pParam = 0;
	*pParamUsed = false;
	*pKeyword = 0;
	const unsigned int max_param = 256;
	unsigned char parameter[max_param];
	unsigned int count = 0;
	unsigned char * savedKeyword = pKeyword;

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
		xxx_UT_DEBUGMSG(("|%c|\n",ch));
		*pKeyword = ch;
		pKeyword++;
		if (!ReadCharFromFileWithCRLF(&ch))
			return false;
	}
	*pKeyword = 0;
	xxx_UT_DEBUGMSG(("keyword %s \n",savedKeyword));
    // If the delimeter was '-' then the following parameter is negative
    if (ch == '-')
    {
        fNegative = true;
		if (!ReadCharFromFileWithCRLF(&ch))
			return false;
    }

    // Read the numeric parameter (if there is one)
	// According to the specs, a dttm parameter (e.g. \revdttm), which is a long, has the
	// individual bytes emited as ASCII characters
	//
	// Some * keywords have the numeric parameter after a space
	// We need to hand this special case. Stupid RTF!!
	//
	bool bLeadSpace = true;
	if (isdigit(ch) || (bLeadSpace && m_currentRTFState.m_bInKeywordStar && (ch == ' ')))
	{

		if(isdigit(ch))
		{
			bLeadSpace=false;
		}
		*pParamUsed = true;
		while (isdigit(ch) || (bLeadSpace && (ch == ' ' )))
		{

			if(isdigit(ch))
			{
				bLeadSpace=false;
			}

			xxx_UT_DEBUGMSG(("|%c|\n",ch));
			// Avoid buffer overflow
			if (count == max_param )
			{
				xxx_UT_DEBUGMSG(("Parameter too large. Bogus RTF!\n"));
				return false;
			}
			if(ch != ' ')
				parameter[count++] = ch;
			if (!ReadCharFromFileWithCRLF(&ch))
				return false;
		}
		parameter[count] = 0;
		xxx_UT_DEBUGMSG(("parameter %s \n",parameter));
		*pParam = atol(reinterpret_cast<char*>(&parameter[0]));
		if (fNegative)
			*pParam = -*pParam;
	}

	// If the delimeter was non-whitespace then this character is part of the following text!
	if ((ch != ' ') && (ch != 10) && (ch != 13))
	{
		SkipBackChar(ch);
	}

	strcpy(g_dbgLastKeyword, (const char *)savedKeyword);
	g_dbgLastParam = *pParam;
	xxx_UT_DEBUGMSG(("Valid Keyword %s Here \n",savedKeyword));
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
		if (gsf_input_read(m_pImportFile, 1, pCh) != NULL)
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

bool IE_Imp_RTF::ReadContentFromFile(UT_UTF8String & str)
{
	unsigned char pCh = 0;
	do
	{
		if (ReadCharFromFileWithCRLF(&pCh) == false)
		{
			return false;
		}
		if(pCh != 10 &&  pCh != 13 && pCh != '}')
			str += pCh;
	} while ((pCh == 10  ||  pCh == 13)  || (pCh != '}'));
	if(pCh == '}')
		SkipBackChar('}');
	return true;

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


UT_UCS4Char IE_Imp_RTF::ReadHexChar(void) 
{
	UT_UCS4Char wc = 0;
	unsigned char ch;
	int val;

	if (ReadCharFromFile(&ch))
	{
		if (hexVal(ch, val)) {
			wc = val << 4;
		}
		else {
			UT_DEBUGMSG(("invalid Hex %c\n", ch));
		}
		if (ReadCharFromFile(&ch))
		{
			if (hexVal(ch, val)) {
				wc += val;
			}
			else {
				UT_DEBUGMSG(("invalid Hex %c\n", ch));
			}
		}
	}
	return wc;
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
bool IE_Imp_RTF::SkipBackChar(unsigned char /*ch*/)
{
	if (m_pImportFile)					// if we are reading a file
	{
		// TODO - I've got a sneaking suspicion that this doesn't work on the Macintosh
		return (!gsf_input_seek(m_pImportFile, -1, G_SEEK_CUR));
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
	UT_sint32 parameter = 0;
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

	m_bFieldRecognized = false;

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
			UT_DEBUGMSG (("RTF: Invalid keyword '%s' in field\n", keyword));
			// don't return as we simply skip it
		}


		tokenType = NextToken (keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN);
	}

	// field instruction
	if (tokenType == RTF_TOKEN_OPEN_BRACE)
	{
		UT_ByteBuf fldBuf;
		gchar * xmlField = NULL;
		bool gotStarKW = false;
		// bUseResult will to be set to false if we encounter a field
		// instruction we know about. Otherwise, we use the result by default
		bUseResult = true;
		// since we enter a brace group, we push the RTFState.
		PushRTFState ();
		nested = 0;
		do
		{
			tokenType = NextToken (keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN,false);
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
			UT_DebugOnly<bool> ok;
			xxx_UT_DEBUGMSG(("Append field type %s \n",xmlField));
			ok = _appendField (xmlField);
			UT_ASSERT_HARMLESS (ok);
			// we own xmlField, so we delete it after use.
			FREEP (xmlField);
		}
	}
	else
	{
		xxx_UT_DEBUGMSG (("RTF: Field instruction not present. Found '%s' in stream\n", keyword));
		UT_ASSERT_HARMLESS (UT_SHOULD_NOT_HAPPEN);
		// continue
	}

	tokenType = NextToken (keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN,false);
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
				UT_DEBUGMSG (("RTF: Invalid keyword '%s' in field\n", keyword));
				// don't return as we simply skip it
			}
			else
			{
				if(m_bFieldRecognized && (m_iHyperlinkOpen== 0))
				{
					SkipCurrentGroup(false);
					return true;
				}
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
	else if(tokenType == RTF_TOKEN_CLOSE_BRACE)
	{
		PopRTFState ();
	}
	else
	{
		UT_DEBUGMSG (("RTF: Field result not present. Found '%s' in stream. Ignoring.\n", keyword));
		// UT_ASSERT_HARMLESS (UT_SHOULD_NOT_HAPPEN);
		// continue
	}

	if(m_iHyperlinkOpen > iHyperlinkOpen)
	{
		FlushStoredChars(true);

		if (!bUseInsertNotAppend()) 
		{
			if(m_bCellBlank || m_bEndTableOpen)
			{
				UT_DEBUGMSG(("Append block 14 \n"));

				if(m_pDelayedFrag)
				{
					getDoc()->insertStruxBeforeFrag(m_pDelayedFrag,PTX_Block,NULL);
				}
				else
				{
					getDoc()->appendStrux(PTX_Block,NULL);
				}	
			m_bCellBlank = false;
				m_bEndTableOpen = false;
			}
			getDoc()->appendObject(PTO_Hyperlink, NULL);
		}
		else 
		{
			if(m_iHyperlinkOpen ==1)
			{
				const gchar * props[] = {"list-tag","dummy",NULL};
				getDoc()->insertObject(m_dposPaste, PTO_Hyperlink, props, NULL);
				m_dposPaste++;
			}
			else
			{
				return false;
			}
		}
		m_iHyperlinkOpen--;
		UT_ASSERT_HARMLESS( m_iHyperlinkOpen == iHyperlinkOpen );
	}
		
	return true;
}


/*!
  \param buf the buffer that contains the RTF.
  \param xmlField the XML attributes for the field.
  \param isXML whether xmlField is used or not.
  \see IE_Imp_RTF::HandleField
 */
gchar *IE_Imp_RTF::_parseFldinstBlock (UT_ByteBuf & _buf, gchar *xmlField, bool & isXML)
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
	std::string Instr;
	UT_uint32  len;
	isXML = false;

	// buffer is empty, nothing to parse
	if (_buf.getLength() == 0)
	{
		FREEP (xmlField);
		return NULL;
	}

	len = _buf.getLength ();
	const UT_Byte *pBuf = _buf.getPointer (0);

	newBuf =  static_cast<char *>(g_try_malloc (sizeof (char) * (len + 1)));
	memcpy (newBuf, pBuf, len);
	newBuf [len] = 0;
	Instr = newBuf;
	instr = const_cast<char *>(Instr.c_str());
	instr = strtok (instr, " \\{}"); // This writes a NULL into Instr somewhere
	                                 // I assume this is OK since the char storage
	                                 // Within the class is contiguous.
	if (instr == NULL)
	{
		g_free (newBuf);
		g_free (xmlField);
		return NULL;
	}

	switch (*instr)
	{
	case 'A':
		if (strcmp (instr, "AUTHOR") == 0)
		{
			xmlField = g_strdup ("meta_creator");
			UT_ASSERT_HARMLESS (xmlField);
			isXML = (xmlField != NULL);
		}
		break;
	case 'C':
		if (strcmp (instr, "CREATEDATE") == 0)
		{
			xmlField = g_strdup ("meta_date");
			UT_ASSERT_HARMLESS (xmlField);
			isXML = (xmlField != NULL);
		}
		else if (strcmp (instr, "COMMENTS") == 0)
		{
			xmlField = g_strdup ("meta_description");
			UT_ASSERT_HARMLESS (xmlField);
			isXML = (xmlField != NULL);
		}
		break;
	case 'D':
		if (strcmp (instr, "DATE") == 0)
		{
			xmlField = g_strdup ("date");
			UT_ASSERT_HARMLESS (xmlField);
			isXML = (xmlField != NULL);
		}
		break;
	case 'F':
		if (strcmp (instr, "FILENAME") == 0)
		{
			// TODO handle parameters
			xmlField = g_strdup ("file_name");
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
			const gchar *new_atts[3];

			new_atts[0] = "xlink:href";
			std::string href;
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

			std::string full_href;

			const char * s = href.c_str();

			if(*s != '#' && !UT_go_path_is_uri(s))
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

			if (!bUseInsertNotAppend()) 
			{

				if(m_bCellBlank || m_bEndTableOpen)
				{
					UT_DEBUGMSG(("Append block 15 \n"));

					if(m_pDelayedFrag)
					{
						getDoc()->insertStruxBeforeFrag(m_pDelayedFrag,PTX_Block,NULL);
					}
					else
					{
						getDoc()->appendStrux(PTX_Block,NULL);
					}
					m_bCellBlank = false;
					m_bEndTableOpen = false;
				}
				getDoc()->appendObject(PTO_Hyperlink, new_atts);
			}
			else 
			{
				if(getDoc()->isInsertHyperLinkValid(m_dposPaste))
				{
						getDoc()->insertObject(m_dposPaste, PTO_Hyperlink, new_atts, NULL);
						m_dposPaste++;
				}
				else
				{
						break;
				}
			}
			m_iHyperlinkOpen++;
		}
		break;
	case 'I':
		if (strcmp (instr, "INCLUDEPICTURE") == 0)
		{
			UT_DEBUGMSG (("RTF: INCLUDEPICTURE fieldinst not handled yet\n"));
		}
		break;
	case 'K':
		if (strcmp (instr, "KEYWORDS") == 0)
		{
			xmlField = g_strdup ("meta_keywords");
			UT_ASSERT_HARMLESS (xmlField);
			isXML = (xmlField != NULL);
		}
		break;
	case 'P':
		if (strcmp (instr, "PAGE") == 0)
		{
			xmlField = g_strdup ("page_number");
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
			xmlField = g_strdup ("char_count");
			UT_ASSERT_HARMLESS (xmlField);
			isXML = (xmlField != NULL);
		}
		// this one have been found with ApplixWare exported RTF.
		else if (strcmp (instr, "NUMPAGES") == 0)
		{
			xmlField = g_strdup ("page_count");
			UT_ASSERT_HARMLESS (xmlField);
			isXML = (xmlField != NULL);
		}
		else if (strcmp (instr, "NUMWORDS") == 0)
		{
			xmlField = g_strdup ("word_count");
			UT_ASSERT_HARMLESS (xmlField);
			isXML = (xmlField != NULL);
		}
		break;
	case 'S':
		if (strcmp (instr, "SAVEDATE") == 0)
		{
			xmlField = g_strdup ("date_dfl");
			UT_ASSERT_HARMLESS (xmlField);
			isXML = (xmlField != NULL);
		}
		else if (strcmp (instr, "SUBJECT") == 0)
		{
			xmlField = g_strdup ("meta_subject");
			UT_ASSERT_HARMLESS (xmlField);
			isXML = (xmlField != NULL);
		}
		break;
	case 'T':
		if (strcmp (instr, "TEXTMETA") == 0)
		{
			std::string xmlid = "";
			const gchar* ppAtts[10] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

			UT_DEBUGMSG (("RTF: RDF opening text meta with original xmlid:%s\n", xmlid.c_str() ));
			PD_XMLIDCreatorHandle xidc = m_XMLIDCreatorHandle;
			xmlid = xidc->createUniqueXMLID( xmlid );
			UT_DEBUGMSG (("RTF: RDF opening text meta with updated  xmlid:%s\n", xmlid.c_str() ));

			ppAtts[0] = PT_XMLID;
			ppAtts[1] = xmlid.c_str();
			// sanity check
			ppAtts[2] = "this-is-an-rdf-anchor";
			ppAtts[3] = "yes";
//			ppAtts[4] = PT_RDF_END;
//			ppAtts[5] = "yes";

			getDoc()->appendObject(PTO_RDFAnchor, ppAtts);
			m_iRDFAnchorOpen++;

			
		}
		if (strcmp (instr, "TIME") == 0)
		{
			// Some Parameters from MS Word 2000 output

			if(strstr(newBuf,"dddd, MMMM dd, yyyy") != NULL)
			{
				xmlField = g_strdup("date");
				UT_ASSERT_HARMLESS (xmlField);
				isXML = (xmlField != NULL);
			}
			else if( strstr(newBuf,"m/d/yy") != NULL)
			{
				xmlField = g_strdup("date_ddmmyy");
				UT_ASSERT_HARMLESS (xmlField);
				isXML = (xmlField != NULL);
			}
			else if( strstr(newBuf,"MMMM d, yyyy") != NULL)
			{
				xmlField = g_strdup("date_mdy");
				UT_ASSERT_HARMLESS (xmlField);
				isXML = (xmlField != NULL);
			}
			else if( strstr(newBuf,"MMM d, yy") != NULL)
			{
				xmlField = g_strdup("date_mthdy");
				UT_ASSERT_HARMLESS (xmlField);
				isXML = (xmlField != NULL);
			}
			else if( strstr(newBuf,"MMM d, yy") != NULL)
			{
				xmlField = g_strdup("date_mthdy");
				UT_ASSERT_HARMLESS (xmlField);
				isXML = (xmlField != NULL);
			}
			else if( strstr(newBuf,"MM-d-yy") != NULL)
			{
				xmlField = g_strdup("date_ntdfl");
				UT_ASSERT_HARMLESS (xmlField);
				isXML = (xmlField != NULL);
			}
			else if( strstr(newBuf,"HH:mm:ss") != NULL)
			{
				xmlField = g_strdup("time_miltime");
				UT_ASSERT_HARMLESS (xmlField);
				isXML = (xmlField != NULL);
			}
			else if( strstr(newBuf,"h:mm:ss am/pm") != NULL)
			{
				xmlField = g_strdup("time_ampm");
				UT_ASSERT_HARMLESS (xmlField);
				isXML = (xmlField != NULL);
			}
//
// Make this the second last one since it's not unique
//
			else if( strstr(newBuf,"dddd") != NULL)
			{
				xmlField = g_strdup("date_wkday");
				UT_ASSERT_HARMLESS (xmlField);
				isXML = (xmlField != NULL);
			}
			else
			{
				xmlField = g_strdup ("time");
				UT_ASSERT_HARMLESS (xmlField);
				isXML = (xmlField != NULL);
			}
		}
		if (strcmp (instr, "TITLE") == 0)
		{
			xmlField = g_strdup ("meta_title");
			UT_ASSERT_HARMLESS (xmlField);
			isXML = (xmlField != NULL);
		}
		else if (strcmp (instr, "TOC") == 0)
		{
			// Table-of-contents field
			UT_DEBUGMSG (("RTF: TOC fieldinst not fully handled yet\n"));

#if 0
			if(!m_bParaWrittenForSection)
			{
				getDoc()->appendStrux(PTX_Block, NULL);
				m_bParaWrittenForSection = true;
			}

			getDoc()->appendStrux(PTX_SectionTOC, NULL);
			getDoc()->appendStrux(PTX_EndTOC, NULL);

			// DAL: hack
			xmlField = g_strdup ("");
			UT_ASSERT_HARMLESS (xmlField);
			isXML = (xmlField != NULL);
#endif
		}
		
		break;
	case 'd':
		if (strcmp (instr, "date") == 0)
		{
			// TODO handle parameters
			xmlField = g_strdup ("date");
			UT_ASSERT_HARMLESS (xmlField);
			isXML = (xmlField != NULL);
		}
		break;
	case '\\':
		/* mostly StarOffice RTF fields */
		if (strcmp (instr, "\\filename") == 0)
		{
			xmlField = g_strdup ("file_name");
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
				fileName = g_build_filename (m_szFileDirName, tok, NULL);
				UT_DEBUGMSG (("fileName is %s\n", fileName));

				bool ok = FlushStoredChars ();
				if (ok)
				{
					// insert the image in the piece table AFTER flushing
					// current output
					FG_Graphic* pFG;
					UT_Error error = IE_ImpGraphic::loadGraphic(fileName, IEGFT_JPEG, &pFG);

					// load file to buffer
					if (error == UT_OK && pFG)
					{
						RTFProps_ImageProps imgProps;
						ok = InsertImage (pFG, fileName, imgProps);
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
			xmlField = g_strdup ("page_number");
			UT_ASSERT_HARMLESS (xmlField);
			isXML = (xmlField != NULL);
		}
		break;
	default:
		UT_DEBUGMSG (("RTF: unhandled fieldinstr %s\n", instr));
		break;
	}
	g_free (newBuf);
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
	UT_uint32 id = getDoc()->getUID(UT_UniqueId::HeaderFtr);
#if 0
	while(id < 10000)
	{
		id  = UT_rand();
	}
#endif
	header->m_id = id;

	m_hdrFtrTable.push_back(header);
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
bool IE_Imp_RTF::TranslateKeyword(unsigned char* pKeyword, UT_sint32 param, bool fParam)
{
	// switch on the first char to reduce the number of string comparisons
	// NB. all RTF keywords are lowercase.
	// after handling the keyword, return true

	// When adding keywords expressing document properties, maker sure
	// that if we are only loading styles, these are ignored
	// (the docs say these can be scattered among the header tables)
	xxx_UT_DEBUGMSG(("Translating keyword %s \n",pKeyword));
	RTF_KEYWORD_ID keywordID = KeywordToID(reinterpret_cast<char *>(pKeyword));
	return TranslateKeywordID(keywordID, param, fParam);
}



bool IE_Imp_RTF::TranslateKeywordID(RTF_KEYWORD_ID keywordID, 
								  UT_sint32 param, bool fParam)
{
	switch (keywordID)
	{
	case RTF_KW_ansicpg:
	{
		const char *szEncoding = NULL;
		if(param == -1)
		{
			// IE issues this value on copy (ctrl+c), and I could not find out from anywhere what it is
			// supposed to mean; I will assume it means use the current system page
			szEncoding = XAP_EncodingManager::get_instance()->getNative8BitEncodingName();
		}
		else
		{
			szEncoding = XAP_EncodingManager::get_instance()->charsetFromCodepage(static_cast<UT_uint32>(param));
		}
		
		// Store the default encoding and activate it.
		m_szDefaultEncoding = szEncoding; 
		setEncoding();
		
		if(!getLoadStylesOnly()) {
			getDoc()->setEncodingName(szEncoding);
		}
		return true;
	}
	case RTF_KW_abitopline:
		return HandleTopline(true);
	case RTF_KW_abibotline:
		return HandleBotline(true);
	case RTF_KW_abinodiroverride:
	{
// this keyword will be immediately followed by either the
// ltrch or rtlch keyword, which we need to eat up ...
		unsigned char kwrd[MAX_KEYWORD_LEN];
		UT_sint32 par = 0;
		bool parUsed = false;
		bool ok = true;
		unsigned char c;
		
// swallow "\" first
		ok = ReadCharFromFileWithCRLF(&c);
		if (ok && ReadKeyword(kwrd, &par, &parUsed, MAX_KEYWORD_LEN))
		{
			if(!(0 == strncmp((const char*)&kwrd[0],"rtlch",MAX_KEYWORD_LEN) ||
				 0 == strncmp((const char*)&kwrd[0],"ltrch",MAX_KEYWORD_LEN)))
			{
				UT_DEBUGMSG(("RTF import: keyword \\%s found where \\ltrch"
							 " or \\rtlch expected\n", kwrd));
			}
		}
		UT_ASSERT(ok);
		xxx_UT_DEBUGMSG(("abinoveride found - swallowed keyword %s \n",kwrd));
		return true;
	}
	case RTF_KW_ansi:
	{
		// this is charset Windows-1252
		const char *szEncoding = XAP_EncodingManager::get_instance()->charsetFromCodepage(1252);
		m_mbtowc.setInCharset(szEncoding);
		if(!getLoadStylesOnly())
			getDoc()->setEncodingName(szEncoding);
		return true;
	}
	case RTF_KW_abirtl:
	{
		m_currentRTFState.m_charProps.m_dirOverride = UT_BIDI_RTL;
		return true;
	}
	case RTF_KW_abiltr:
	{
		m_currentRTFState.m_charProps.m_dirOverride = UT_BIDI_LTR;
		return true;
	}
	case RTF_KW_aendnotes:
		if(!getLoadStylesOnly())
		{
			const gchar * props[] = {"document-endnote-place-endsection", "1",
										NULL};
			getDoc()->setProperties(&props[0]);
		}
		break;
	case RTF_KW_aenddoc:
		if(!getLoadStylesOnly())
		{
			const gchar * props[] = {"document-endnote-place-enddoc", "1",
										NULL};
			getDoc()->setProperties(&props[0]);
		}
		break;
	case RTF_KW_aftnstart:
		if(!getLoadStylesOnly())
		{
			const gchar * props[] = {"document-endnote-initial", NULL,
										NULL};
			std::string i = UT_std_string_sprintf("%d",param);
			props[1] = i.c_str();
			getDoc()->setProperties(&props[0]);
		}
		break;
	case RTF_KW_aftnrestart:
		if(!getLoadStylesOnly())
		{
			
			const gchar * props[] = {"document-endnote-restart-section", "1",
										NULL};
			getDoc()->setProperties(&props[0]);
		}
		break;
	case RTF_KW_aftnnar:
		if(!getLoadStylesOnly())
		{
			const gchar * props[] = {"document-endnote-type", "numeric",
										NULL};
			getDoc()->setProperties(&props[0]);
		}
		break;
	case RTF_KW_aftnnalc:
		if(!getLoadStylesOnly())
		{
			const gchar * props[] = {"document-endnote-type", "lower",
										NULL};
			getDoc()->setProperties(&props[0]);
		}
		break;
	case RTF_KW_aftnnauc:
		if(!getLoadStylesOnly())
		{
			const gchar * props[] = {"document-endnote-type", "upper",
										NULL};
			getDoc()->setProperties(&props[0]);
		}
		break;
	case RTF_KW_aftnnrlc:
		if(!getLoadStylesOnly())
		{
			const gchar * props[] = {"document-endnote-type", "lower-roman",
										NULL};
			getDoc()->setProperties(&props[0]);
		}
		break;
	case RTF_KW_aftnnruc:
		if(!getLoadStylesOnly())
		{
			const gchar * props[] = {"document-footnote-type", "upper-roman",
										NULL};
			getDoc()->setProperties(&props[0]);
		}
		break;
	case RTF_KW_b:
		// bold - either on or off depending on the parameter
		return HandleBold(fParam ? false : true);
	case RTF_KW_bullet:
		return ParseChar(UCS_BULLET);
	case RTF_KW_brdrt:
		UT_DEBUGMSG(("Border Top set \n"));
		m_currentRTFState.m_paraProps.m_iCurBorder = (int) rtfBorderTop;
		m_currentRTFState.m_paraProps.m_bTopBorder = true;
		m_bCellActive = false;
		m_bParaActive = true;
		return true;
	case RTF_KW_brdrl:
		xxx_UT_DEBUGMSG(("Border left set \n"));
		m_currentRTFState.m_paraProps.m_iCurBorder = (int) rtfBorderLeft;
		m_currentRTFState.m_paraProps.m_bLeftBorder = true;
		m_bCellActive = false;
		m_bParaActive = true;
		return true;
	case RTF_KW_brdrb:
		xxx_UT_DEBUGMSG(("Border Bot set \n"));
		m_currentRTFState.m_paraProps.m_iCurBorder = (int) rtfBorderBot;
		m_currentRTFState.m_paraProps.m_bBotBorder = true;
		m_bCellActive = false;
		m_bParaActive = true;
		return true;
	case RTF_KW_brdrr:
		xxx_UT_DEBUGMSG(("Border Right set \n"));
		m_currentRTFState.m_paraProps.m_iCurBorder = (int) rtfBorderRight;
		m_currentRTFState.m_paraProps.m_bRightBorder = true;
		m_bCellActive = false;
		m_bParaActive = true;
		return true;
	case RTF_KW_brdrbtw:
		m_currentRTFState.m_paraProps.m_bMergeBordersShading = true;
		return true;
	case RTF_KW_brdrs:
		if(m_bCellActive)
		{
			if (m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderTop)
			{
				_setStringProperty(m_currentRTFState.m_cellProps.m_sCellProps,"top-style","solid");
			}
			else if (m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderLeft)
			{
				_setStringProperty(m_currentRTFState.m_cellProps.m_sCellProps,"left-style","solid");
			}
			else if (m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderBot)
			{
				_setStringProperty(m_currentRTFState.m_cellProps.m_sCellProps,"bot-style","solid");
			}
			else if (m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderRight)
			{
				_setStringProperty(m_currentRTFState.m_cellProps.m_sCellProps,"right-style","solid");
			}
		}
		else if(m_bParaActive)
		{
			if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderTop)
			{
				m_currentRTFState.m_paraProps.m_iTopBorderStyle = 1;
			}
			else if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderLeft)
			{
				m_currentRTFState.m_paraProps.m_iLeftBorderStyle = 1;
			}
			else if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderBot)
			{
				m_currentRTFState.m_paraProps.m_iBotBorderStyle = 1;
			}
			else if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderRight)
			{
				m_currentRTFState.m_paraProps.m_iRightBorderStyle = 1;
			}
		}
		return true;
	case RTF_KW_brdrdot:
		if(m_bCellActive)
		{
			if (m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderTop)
			{
				_setStringProperty(m_currentRTFState.m_cellProps.m_sCellProps,"top-style","dotted");
			}
			else if (m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderLeft)
			{
				_setStringProperty(m_currentRTFState.m_cellProps.m_sCellProps,"left-style","dotted");
			}
			else if (m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderBot)
			{
				_setStringProperty(m_currentRTFState.m_cellProps.m_sCellProps,"bot-style","dotted");
			}
			else if (m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderRight)
			{
				_setStringProperty(m_currentRTFState.m_cellProps.m_sCellProps,"right-style","dotted");
			}
		}
		else if(m_bParaActive)
		{
			if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderTop)
			{
				m_currentRTFState.m_paraProps.m_iTopBorderStyle = 2;
			}
			else if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderLeft)
			{
				m_currentRTFState.m_paraProps.m_iLeftBorderStyle = 2;
			}
			else if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderBot)
			{
				m_currentRTFState.m_paraProps.m_iBotBorderStyle = 2;
			}
			else if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderRight)
			{
				m_currentRTFState.m_paraProps.m_iRightBorderStyle = 2;
			}
		}
		return true;
	case RTF_KW_brdrdash:
		if(m_bCellActive)
		{
			if (m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderTop)
			{
				_setStringProperty(m_currentRTFState.m_cellProps.m_sCellProps,"top-style","dashed");
			}
			else if (m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderLeft)
			{
				_setStringProperty(m_currentRTFState.m_cellProps.m_sCellProps,"left-style","dashed");
			}
			else if (m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderBot)
			{
				_setStringProperty(m_currentRTFState.m_cellProps.m_sCellProps,"bot-style","dashed");
			}
			else if (m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderRight)
			{
				_setStringProperty(m_currentRTFState.m_cellProps.m_sCellProps,"right-style","dashed");
			}
		}
		else if(m_bParaActive)
		{
			if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderTop)
			{
				m_currentRTFState.m_paraProps.m_iTopBorderStyle = 3;
			}
			else if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderLeft)
			{
				m_currentRTFState.m_paraProps.m_iLeftBorderStyle = 3;
			}
			else if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderBot)
			{
				m_currentRTFState.m_paraProps.m_iBotBorderStyle = 3;
			}
			else if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderRight)
			{
				m_currentRTFState.m_paraProps.m_iRightBorderStyle = 3;
			}
		}
		
		return true;
	case RTF_KW_brdrw:
	{
		double dWidth = static_cast<double>(param)/1440; // convert to inches
		std::string sWidth;
		{
			UT_LocaleTransactor t(LC_NUMERIC, "C");
			sWidth = UT_std_string_sprintf("%fin",dWidth);
		}
		if(m_bCellActive)
		{
			if (m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderTop)
			{
				_setStringProperty(m_currentRTFState.m_cellProps.m_sCellProps,"top-thickness",sWidth.c_str());
			}
			else if (m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderLeft)
			{
				_setStringProperty(m_currentRTFState.m_cellProps.m_sCellProps,"left-thickness",sWidth.c_str());
			}
			else if (m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderBot)
			{
				_setStringProperty(m_currentRTFState.m_cellProps.m_sCellProps,"bot-thickness",sWidth.c_str());
			}
			else if (m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderRight)
			{
				_setStringProperty(m_currentRTFState.m_cellProps.m_sCellProps,"right-thickness",sWidth.c_str());
			}
		}
		else if(m_bParaActive)
		{
			if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderTop)
			{
				m_currentRTFState.m_paraProps.m_iTopBorderWidth = (int) param;
			}
			else if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderLeft)
			{
				m_currentRTFState.m_paraProps.m_iLeftBorderWidth = (int) param;
			}
			else if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderBot)
			{
				m_currentRTFState.m_paraProps.m_iBotBorderWidth = (int) param;
			}
			else if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderRight)
			{
				m_currentRTFState.m_paraProps.m_iRightBorderWidth = (int) param;
			}
		}
		return true;
	}
	case RTF_KW_brdrcf:
	{
		UT_sint32 iCol = static_cast<UT_sint32>(param);
		UT_uint32 colour = GetNthTableColour(iCol);
		std::string sColor = UT_std_string_sprintf("%06x", colour);
		if(m_bCellActive)
		{
			if (m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderTop)
			{
				_setStringProperty(m_currentRTFState.m_cellProps.m_sCellProps,"top-color",sColor.c_str());
			}
			else if (m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderLeft)
			{
				_setStringProperty(m_currentRTFState.m_cellProps.m_sCellProps,"left-color",sColor.c_str());
			}
			else if (m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderBot)
			{
				_setStringProperty(m_currentRTFState.m_cellProps.m_sCellProps,"bot-color",sColor.c_str());
			}
			else if (m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderRight)
			{
				_setStringProperty(m_currentRTFState.m_cellProps.m_sCellProps,"right-color",sColor.c_str());
			}
		}
		else if(m_bParaActive)
		{
			if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderTop)
			{
				m_currentRTFState.m_paraProps.m_iTopBorderCol = (int) param;
			}
			else if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderLeft)
			{
				m_currentRTFState.m_paraProps.m_iLeftBorderCol = (int) param;
			}
			else if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderBot)
			{
				m_currentRTFState.m_paraProps.m_iBotBorderCol = (int) param;
			}
			else if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderRight)
			{
				m_currentRTFState.m_paraProps.m_iRightBorderCol = (int) param;
			}
		}
		return true;
	}
	case RTF_KW_brsp:
		if(m_bParaActive)
		{
			if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderTop)
			{
				m_currentRTFState.m_paraProps.m_iTopBorderSpacing = (int) param;
			}
			else if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderLeft)
			{
				m_currentRTFState.m_paraProps.m_iLeftBorderSpacing = (int) param;
			}
			else if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderBot)
			{
				m_currentRTFState.m_paraProps.m_iBotBorderSpacing = (int) param;
			}
			else if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderRight)
			{
				m_currentRTFState.m_paraProps.m_iRightBorderSpacing = (int) param;
			}
		}
		return true;
	case RTF_KW_brdrnone:
		if(m_bCellActive)
		{
			if(m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderRight)
				m_currentRTFState.m_cellProps.m_bRightBorder = false;
			else if(m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderBot)
				m_currentRTFState.m_cellProps.m_bBotBorder = false;
			else if(m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderLeft)
				m_currentRTFState.m_cellProps.m_bLeftBorder = false;
			else if(m_currentRTFState.m_cellProps.m_iCurBorder == rtfCellBorderTop)
				m_currentRTFState.m_cellProps.m_bTopBorder = false;
		}
		if(m_bParaActive)
		{
			if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderTop)
			{
				m_currentRTFState.m_paraProps.m_bTopBorder = false;
				m_currentRTFState.m_paraProps.m_iTopBorderStyle = 0;
			}
			else if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderLeft)
			{
				m_currentRTFState.m_paraProps.m_bLeftBorder = false;
				m_currentRTFState.m_paraProps.m_iLeftBorderStyle = 0;
			}
			else if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderBot)
			{
				m_currentRTFState.m_paraProps.m_bBotBorder = false;
				m_currentRTFState.m_paraProps.m_iBotBorderStyle = 0;
			}
			else if (m_currentRTFState.m_paraProps.m_iCurBorder == (int) rtfBorderRight)
			{
				m_currentRTFState.m_paraProps.m_bRightBorder = false;
				m_currentRTFState.m_paraProps.m_iRightBorderStyle = 0;
			}
		}

		return true;

	case RTF_KW_colortbl:
		// It is import that we don't fail if this fail
		// Just continue
		if(!ReadColourTable()) {
			UT_DEBUGMSG(("RTF ERROR: ReadColourTable() failed.\n"));
		}
		// And this does not even warrant an attempt to recover.
		return true;
	case RTF_KW_cf:
		return HandleColour(fParam ? param : 0);
	case RTF_KW_cb:
		return HandleBackgroundColour (fParam ? param : 0);
	case RTF_KW_cols:
		m_currentRTFState.m_sectionProps.m_numCols = static_cast<UT_uint32>(param);
		return true;
	case RTF_KW_colsx:
		m_currentRTFState.m_sectionProps.m_colSpaceTwips = static_cast<UT_uint32>(param);
		return true;
	case RTF_KW_column:
		return ParseChar(UCS_VTAB);
	case RTF_KW_chdate:
		return _appendField ("date");
	case RTF_KW_chtime:
		return _appendField ("time");
	case RTF_KW_chdpl:
	{
		const gchar * attribs[3] ={"param",NULL,NULL};
		attribs[1] = "%A, %B %d, %Y";
		return _appendField ("datetime_custom", attribs);
	}
	case RTF_KW_chdpa:
	{
//		const gchar * attribs[3] ={"param",NULL,NULL};
//		attribs[1] = "%a, %b %d, %Y";
		return _appendField ("datetime_custom");
	}
	case RTF_KW_chpgn:
		return _appendField ("page_number");
	case RTF_KW_chftn:
		HandleNoteReference();
		break;
	case RTF_KW_cs:
		m_currentRTFState.m_charProps.m_styleNumber = param;
		return true;
	case RTF_KW_cell:
		UT_DEBUGMSG(("SEVIOR: Processing cell \n"));
		HandleCell();
		return true;
	case RTF_KW_cellx:
		HandleCellX(param);
		return true;
	case RTF_KW_clvmrg:
		xxx_UT_DEBUGMSG(("Found Vertical merge cell clvmrg \n"));
		m_currentRTFState.m_cellProps.m_bVerticalMerged = true;
		return true;
	case RTF_KW_clvmgf:
		xxx_UT_DEBUGMSG(("Found Vertical merge cell first clvmgf \n"));
		m_currentRTFState.m_cellProps.m_bVerticalMergedFirst = true;
		return true;
	case RTF_KW_clmrg:
		m_currentRTFState.m_cellProps.m_bHorizontalMerged = true;
		return true;
	case RTF_KW_clmgf:
		m_currentRTFState.m_cellProps.m_bHorizontalMergedFirst = true;
		return true;
	case RTF_KW_clbrdrt:
		xxx_UT_DEBUGMSG(("Border Top set \n"));
		m_currentRTFState.m_cellProps.m_iCurBorder = rtfCellBorderTop;
		m_currentRTFState.m_cellProps.m_bTopBorder = true;
		m_bCellActive = true;
		m_bParaActive = false;
		return true;
	case RTF_KW_clbrdrl:
		xxx_UT_DEBUGMSG(("Border left set \n"));
		m_currentRTFState.m_cellProps.m_iCurBorder = rtfCellBorderLeft;
		m_currentRTFState.m_cellProps.m_bLeftBorder = true;
		m_bCellActive = true;
		m_bParaActive = false;
		return true;
	case RTF_KW_clbrdrb:
		xxx_UT_DEBUGMSG(("Border Bot set \n"));
		m_currentRTFState.m_cellProps.m_iCurBorder = rtfCellBorderBot;
		m_currentRTFState.m_cellProps.m_bBotBorder = true;
		m_bCellActive = true;
		m_bParaActive = false;
		return true;
	case RTF_KW_clbrdrr:
		xxx_UT_DEBUGMSG(("Border Right set \n"));
		m_currentRTFState.m_cellProps.m_iCurBorder = rtfCellBorderRight;
		m_currentRTFState.m_cellProps.m_bRightBorder = true;
		m_bCellActive = true;
		m_bParaActive = false;
		return true;
	case RTF_KW_clcbpat:
	{
		UT_sint32 iCol = static_cast<UT_sint32>(param);
		UT_uint32 colour = GetNthTableColour(iCol);
		std::string sColor = UT_std_string_sprintf("%06x", colour);
		xxx_UT_DEBUGMSG(("Writing background color %s to properties \n",sColor.c_str()));
		_setStringProperty(m_currentRTFState.m_cellProps.m_sCellProps,"background-color",sColor.c_str());
	}
	case RTF_KW_cfpat:
	{
		m_currentRTFState.m_paraProps.m_iShadingPattern = 1;
		m_currentRTFState.m_paraProps.m_iShadingForeCol = (int) param;
	}
	case RTF_KW_cbpat:
	{
		m_currentRTFState.m_paraProps.m_iShadingPattern = 1;
		m_currentRTFState.m_paraProps.m_iShadingBackCol = (int) param;
	}
	break;
	case RTF_KW_deff: 
		if (fParam) {
			m_iDefaultFontNumber = param;
			m_currentRTFState.m_charProps.m_fontNumber = m_iDefaultFontNumber;	
		}
		break;
	case RTF_KW_dn:
		// subscript with position. Default is 6.
		// superscript: see up keyword
		return HandleSubscriptPosition (fParam ? param : 6);
	case RTF_KW_emdash:
		return ParseChar(UCS_EM_DASH);
	case RTF_KW_endash:
		return ParseChar(UCS_EN_DASH);
	case RTF_KW_emspace:
		return ParseChar(UCS_EM_SPACE);
	case RTF_KW_enspace:
		return ParseChar(UCS_EN_SPACE);
	case RTF_KW_endnotes:
	{
		const gchar * props[] = {"document-endnote-place-endsection", "1",
									NULL};
		getDoc()->setProperties(&props[0]);
	}
	break;
	case RTF_KW_enddoc:
	{
		const gchar * props[] = {"document-endnote-place-enddoc", "1",
									NULL};
		getDoc()->setProperties(&props[0]);
	}
	break;
	case RTF_KW_fonttbl:
		// read in the font table
		if(!ReadFontTable()) {
			UT_DEBUGMSG(("RTF ERROR: ReadFontTable() failed\n"));
		}
		return true;
	case RTF_KW_fs:
		return HandleFontSize(fParam ? param : 24);
	case RTF_KW_f:
		return HandleFace(fParam ? param : m_iDefaultFontNumber); 
	case RTF_KW_fi:
		m_currentRTFState.m_paraProps.m_indentFirst = param;
		return true;
	case RTF_KW_field:
		return HandleField ();
	case RTF_KW_fldrslt:
		if(m_bFieldRecognized && (m_iHyperlinkOpen== 0))
		{
//
// skip this until the next "}"
//
			SkipCurrentGroup();
		}
		else
		{
//
// Just parse the text found
//
			return true;
		}
		break;
	case RTF_KW_footer:
	{
		UT_uint32 footerID = 0;
		return HandleHeaderFooter (RTFHdrFtr::hftFooter, footerID);
	}
	case RTF_KW_footerf:
	{
		UT_uint32 footerID = 0;
		return HandleHeaderFooter (RTFHdrFtr::hftFooterFirst, footerID);
	}
	case RTF_KW_footerr:
	{
		UT_uint32 footerID = 0;
		return HandleHeaderFooter (RTFHdrFtr::hftFooterEven, footerID);
	}
	case RTF_KW_footerl:
	{
		UT_uint32 footerID = 0;
		return HandleHeaderFooter (RTFHdrFtr::hftFooter, footerID);
	}
	case RTF_KW_footery:
		// Height ot the header in twips
		m_currentRTFState.m_sectionProps.m_footerYTwips = param;
		break;
	case RTF_KW_gutter:
		// Gap between text and left (or right) margin in twips
		m_currentRTFState.m_sectionProps.m_gutterTwips = param;
		break;
	case RTF_KW_footnote:
		// can be both footnote and endnote ...
// No pasting footnotes/endnotes in HdrFtrs or footnotes/endnotes
		if(bUseInsertNotAppend())
		{
			XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
			if(pFrame == NULL)
			{
				m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
				return true;
			}
			FV_View * pView = static_cast<FV_View*>(pFrame->getCurrentView());
			if(pView == NULL)
			{
				m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
				return true;
			}
			if(pView->isHdrFtrEdit())
			{
				m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
				return true;
			}
			if(pView->isInFootnote(m_dposPaste) || pView->isInEndnote(m_dposPaste) )
		    {
				m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
				return true;
			}
		}

		m_bFootnotePending = true;
		return true;
	case RTF_KW_ftnalt:
		// should not be here, since this keyword is supposed to
		// follow \footnote and is handled separately
		UT_DEBUGMSG(("RTF Keyword \'ftnalt\' where it should not have been\n"));
		return true;
	case RTF_KW_ftnstart:
		if(!getLoadStylesOnly())
		{
			const gchar * props[] = {"document-footnote-initial", NULL,
										NULL};
			std::string i = UT_std_string_sprintf("%d",param);
			props[1] = i.c_str();
			getDoc()->setProperties(&props[0]);
		}
		break;
	case RTF_KW_ftnrstpg:
		if(!getLoadStylesOnly())
		{
			const gchar * props[] = {"document-footnote-restart-page", "1",
										NULL};
			getDoc()->setProperties(&props[0]);
		}
		break;
	case RTF_KW_ftnrestart:
		if(!getLoadStylesOnly())
		{
			const gchar * props[] = {"document-footnote-restart-section", "1",
										NULL};
			getDoc()->setProperties(&props[0]);
		}
		break;
	case RTF_KW_ftnnar:
		if(!getLoadStylesOnly())
		{
			const gchar * props[] = {"document-footnote-type", "numeric",
										NULL};
			getDoc()->setProperties(&props[0]);
		}
		break;
	case RTF_KW_ftnnalc:
		if(!getLoadStylesOnly())
		{
			const gchar * props[] = {"document-footnote-type", "lower",
										NULL};
			getDoc()->setProperties(&props[0]);
		}
		break;
	case RTF_KW_ftnnauc:
		if(!getLoadStylesOnly())
		{
			const gchar * props[] = {"document-footnote-type", "upper",
										NULL};
			getDoc()->setProperties(&props[0]);
		}
		break;
	case RTF_KW_ftnnrlc:
		if(!getLoadStylesOnly())
		{
			const gchar * props[] = {"document-footnote-type", "lower-roman",
										NULL};
			getDoc()->setProperties(&props[0]);
		}
		break;
	case RTF_KW_ftnnruc:
		if(!getLoadStylesOnly())
		{
			const gchar * props[] = {"document-footnote-type", "upper-roman",
										NULL};
			getDoc()->setProperties(&props[0]);
		}
		break;
	case RTF_KW_headery:
		// Height ot the header in twips
		m_currentRTFState.m_sectionProps.m_headerYTwips = param;
		break;
	case RTF_KW_header:
	{
		UT_uint32 headerID = 0;
		return HandleHeaderFooter (RTFHdrFtr::hftHeader, headerID);
	}
	case RTF_KW_headerf:
	{
		UT_uint32 headerID = 0;
		return HandleHeaderFooter (RTFHdrFtr::hftHeaderFirst, headerID);
	}
	case RTF_KW_headerr:
	{
		UT_uint32 headerID = 0;
		return HandleHeaderFooter (RTFHdrFtr::hftHeaderEven, headerID);
	}
	case RTF_KW_headerl:
	{
		UT_uint32 headerID = 0;
		return HandleHeaderFooter (RTFHdrFtr::hftHeader, headerID);
	}
	case RTF_KW_highlight:
		return HandleBackgroundColour(param);
	case RTF_KW_i:
		// italic - either on or off depending on the parameter
		return HandleItalic(fParam ? false : true);
	case RTF_KW_info:
		// TODO Ignore document info for the moment
		return HandleInfoMetaData();
	case RTF_KW_ilvl:
		m_currentRTFState.m_paraProps.m_iOverrideLevel = static_cast<UT_uint32>(_sanitizeListLevel(param));
		return true;
	case RTF_KW_intbl:
		UT_DEBUGMSG(("done intbl \n"));
		m_currentRTFState.m_paraProps.m_bInTable = true;
		return true;
	case RTF_KW_itap:
		if(bUseInsertNotAppend())
		{
			return true;
		}
		if(m_bInFootnote)
		{
			return true;
		}
		m_currentRTFState.m_paraProps.m_tableLevel = param;
//
// Look to see if the nesting level of our tables has changed.
//
		xxx_UT_DEBUGMSG(("SEVIOR!!! itap m_tableLevel %d nestDepth %d \n",m_currentRTFState.m_paraProps.m_tableLevel,m_TableControl.getNestDepth()));
		if(m_currentRTFState.m_paraProps.m_tableLevel > m_TableControl.getNestDepth())
		{
			while(m_currentRTFState.m_paraProps.m_tableLevel > m_TableControl.getNestDepth())
			{
				xxx_UT_DEBUGMSG(("SEVIOR: Doing itap OpenTable \n"));
				OpenTable();
			}
		}
		else if((m_currentRTFState.m_paraProps.m_tableLevel >= 0) && m_currentRTFState.m_paraProps.m_tableLevel < m_TableControl.getNestDepth())
		{
			while(m_currentRTFState.m_paraProps.m_tableLevel < m_TableControl.getNestDepth())
			{
				CloseTable();
			}

			if(param == 0)
			{
				// we closed the highest level table; in rtf table is equivalent to a block,
				// while our table is contained in a block
				// so we have to insert a block to get the same effect
				StartNewPara();
			}
		
		}
		xxx_UT_DEBUGMSG(("SEVIOR!!! After itap m_tableLevel %d nestDepth %d \n",m_currentRTFState.m_paraProps.m_tableLevel,m_TableControl.getNestDepth()));

		return true;
	case RTF_KW_lquote:
		return ParseChar(UCS_LQUOTE);
	case RTF_KW_ldblquote:
		return ParseChar(UCS_LDBLQUOTE);
	case RTF_KW_li:
		m_currentRTFState.m_paraProps.m_indentLeft = param;
		return true;
	case RTF_KW_line:
		return ParseChar(UCS_LF);
	case RTF_KW_linebetcol:
		m_currentRTFState.m_sectionProps.m_bColumnLine = true;
		return true;
	case RTF_KW_lang:
		xxx_UT_DEBUGMSG(("DOM: lang code (0x%p, %s)\n", param, wvLIDToLangConverter(static_cast<unsigned short>(param))));
		// mark language for spell checking
		m_currentRTFState.m_charProps.m_szLang = wvLIDToLangConverter(static_cast<unsigned short>(param));
		return true;
	case RTF_KW_listoverridetable:
		return ReadListOverrideTable();
	case RTF_KW_listtext:
		// This paragraph is a member of a list.
		SkipCurrentGroup( false);
		return true;
	case RTF_KW_ls:
		// This paragraph is a member of a list.
		m_currentRTFState.m_paraProps.m_iOverride = static_cast<UT_uint32>(param);
		m_currentRTFState.m_paraProps.m_isList = true;
		return true;
	case RTF_KW_landscape:
        // TODO
        // Just set landscape mode.
        //
		break;
	case RTF_KW_ltrpar:
		xxx_UT_DEBUGMSG(("rtf imp.: ltrpar\n"));
		m_currentRTFState.m_paraProps.m_dir = UT_BIDI_LTR;
		//reset doc bidi attribute
		m_bBidiMode = false;
		return true;
	case RTF_KW_ltrsect:
		xxx_UT_DEBUGMSG(("rtf imp.: ltrsect\n"));
		m_currentRTFState.m_sectionProps.m_dir = UT_BIDI_LTR;
		return true;
	case RTF_KW_ltrmark:
	case RTF_KW_ltrch:
		xxx_UT_DEBUGMSG(("rtf imp.: ltrch\n"));
		m_currentRTFState.m_charProps.m_dir = UT_BIDI_LTR;
		
		// we enter bidi mode if we encounter a character
		// formatting inconsistent with the base direction of the
		// paragraph; once in bidi mode, we have to stay there
		// until the end of the current pragraph
		m_bBidiMode = m_bBidiMode ||
			(m_currentRTFState.m_charProps.m_dir ^ m_currentRTFState.m_paraProps.m_dir);
		return true;
	case RTF_KW_mac:
		// TODO some iconv's may have a different name - "MacRoman"
		// TODO EncodingManager should handle encoding names
		m_mbtowc.setInCharset("MACINTOSH");
		if(!getLoadStylesOnly())
			getDoc()->setEncodingName("MacRoman");
		return true;
	case RTF_KW_marglsxn:
		// Left margin of section
		m_currentRTFState.m_sectionProps.m_leftMargTwips = param;
		break;
	case RTF_KW_margl:
		m_sectdProps.m_leftMargTwips = param ;

		// bug 9432: the \margl is document default, and we have to adjust the current
		// settings accordingly -- this assumes that \margl is not preceeded by \marglsxn,
		// but since \margl is document wide setting this should be true in normal rtf
		// documents (otherwise we would need to add m_b*Changed members to the sect props
		// for everyting).
		m_currentRTFState.m_sectionProps.m_leftMargTwips = param;
		break;
	case RTF_KW_margrsxn:
		// Right margin of section
		m_currentRTFState.m_sectionProps.m_rightMargTwips = param;
		break;
	case RTF_KW_margr:
		m_sectdProps.m_rightMargTwips = param;

		// bug 9432: the \margl is document default, and we have to adjust the current
		// settings accordingly -- this assumes that \margl is not preceeded by \marglsxn,
		// but since \margl is document wide setting this should be true in normal rtf
		// documents (otherwise we would need to add m_b*Changed members to the sect props
		// for everyting).
		m_currentRTFState.m_sectionProps.m_rightMargTwips = param;
		break;
	case RTF_KW_margtsxn:
		// top margin of section
		m_currentRTFState.m_sectionProps.m_topMargTwips = param;
		break;
	case RTF_KW_margt:
		m_sectdProps.m_topMargTwips = param;

		// bug 9432: the \margl is document default, and we have to adjust the current
		// settings accordingly -- this assumes that \margl is not preceeded by \marglsxn,
		// but since \margl is document wide setting this should be true in normal rtf
		// documents (otherwise we would need to add m_b*Changed members to the sect props
		// for everyting).
		m_currentRTFState.m_sectionProps.m_topMargTwips = param;
		break;
	case RTF_KW_margbsxn:
		// bottom margin of section
		m_currentRTFState.m_sectionProps.m_bottomMargTwips = param;
		break;
	case RTF_KW_margb:
		m_sectdProps.m_bottomMargTwips = param;

		// bug 9432: the \margl is document default, and we have to adjust the current
		// settings accordingly -- this assumes that \margl is not preceeded by \marglsxn,
		// but since \margl is document wide setting this should be true in normal rtf
		// documents (otherwise we would need to add m_b*Changed members to the sect props
		// for everyting).
		m_currentRTFState.m_sectionProps.m_bottomMargTwips = param;
		break;
	case RTF_KW_nestrow:
		HandleRow();
		return true;
	case RTF_KW_nestcell:
		UT_DEBUGMSG(("SEVIOR: Processing nestcell \n"));
		HandleCell();
		return true;
	case RTF_KW_nonesttables:
		//
		// skip this!
		//
		UT_DEBUGMSG(("SEVIOR: doing nonesttables \n"));
		SkipCurrentGroup();
		return true;
	case RTF_KW_nonshppict:
		// we ignore this one since we handle shppict.
		UT_DEBUGMSG(("Hub: skipping nonshppict\n"));
		SkipCurrentGroup(false);
		break;
	case RTF_KW_noproof:
		// Set language to none for \noproof
		// TODO actually implement proofing flag separate to language setting
		UT_DEBUGMSG(("HIPI: RTF import keyword \\noproof\n"));
		// mark language for spell checking
		m_currentRTFState.m_charProps.m_szLang = "-none-";
		return true;
	case RTF_KW_ol:
		return HandleOverline(fParam ? (param != 0) : true);
	case RTF_KW_object:
		// get picture
		return HandleObject();
	case RTF_KW_par:
		// start new paragraph, continue current attributes
		xxx_UT_DEBUGMSG(("Done par \n"));
		return HandleParKeyword();
	case RTF_KW_plain:
		// reset character attributes
		return ResetCharacterAttributes();
	case RTF_KW_pard:
	{
		// reset paragraph attributes
		xxx_UT_DEBUGMSG(("Done pard \n"));
		bool bres = ResetParagraphAttributes();
		
		return bres;
	}
	case RTF_KW_page:
		return ParseChar(UCS_FF);
	case RTF_KW_pntext:
		//
		// skip this!
		//
		//SkipCurrentGroup( false);
		m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
		return true;
	case RTF_KW_pict:
		// get picture
		UT_DEBUGMSG(("FOund a pict!!! \n"));
		return HandlePicture();
	case RTF_KW_pc:
		m_mbtowc.setInCharset(XAP_EncodingManager::get_instance()->charsetFromCodepage(437));
		return true;
	case RTF_KW_pca:
		m_mbtowc.setInCharset(XAP_EncodingManager::get_instance()->charsetFromCodepage(850));
		return true;
	case RTF_KW_paperw:
        //
        // Just set the page width
        //
		if(!getLoadStylesOnly())
		{
			double height = getDoc()->m_docPageSize.Height(DIM_IN);
			double width = (static_cast<double>(param))/1440.0;
			getDoc()->m_docPageSize.Set(width,height,DIM_IN);
			UT_DEBUGMSG(("Page set to width %f height %f \n",width,height));
		}
		break;
	case RTF_KW_paperh:
        //
		// Just set the page height
		//
		if(!getLoadStylesOnly())
		{
			double width = getDoc()->m_docPageSize.Width(DIM_IN);
			double height = (static_cast<double>(param))/1440.0;
			getDoc()->m_docPageSize.Set(width,height,DIM_IN);
			UT_DEBUGMSG(("Page set to width %f height %f \n",width,height));
		}
		break;

	case RTF_KW_ql:
		return SetParaJustification(RTFProps_ParaProps::pjLeft);
	case RTF_KW_qc:
		return SetParaJustification(RTFProps_ParaProps::pjCentre);
	case RTF_KW_qr:
		return SetParaJustification(RTFProps_ParaProps::pjRight);
	case RTF_KW_qj:
		return SetParaJustification(RTFProps_ParaProps::pjFull);

	case RTF_KW_rquote:
		return ParseChar(UCS_RQUOTE);
	case RTF_KW_rdblquote:
		return ParseChar(UCS_RDBLQUOTE);

	// various revision keywords
	case RTF_KW_deleted: // skip this -- it is redundant
	case RTF_KW_revised:
		return true;
		
	case RTF_KW_revauthdel:
		if(m_currentRTFState.m_revAttr.size())
			return true; // ignore this
		else
			return HandleRevisedText(PP_REVISION_DELETION, param);
	case RTF_KW_revauth:
		if(m_currentRTFState.m_revAttr.size())
			return true; // ignore this
		else
			return HandleRevisedText(PP_REVISION_ADDITION, param);
	case RTF_KW_crauth:
		if(m_currentRTFState.m_revAttr.size())
			return true; // ignore this
		else
			return HandleRevisedText(PP_REVISION_FMT_CHANGE, param);
		
	case RTF_KW_crdate:
	case RTF_KW_revdttmdel:
	case RTF_KW_revdttm:
		if(m_currentRTFState.m_revAttr.size())
			return true; // ignore this
		else
			return HandleRevisedTextTimestamp(param);
		
		
	case RTF_KW_ri:
		m_currentRTFState.m_paraProps.m_indentRight = param;
		return true;
	case RTF_KW_rtf:
		return true;
	case RTF_KW_rtlpar:
		xxx_UT_DEBUGMSG(("rtf imp.: rtlpar\n"));
		m_currentRTFState.m_paraProps.m_dir = UT_BIDI_RTL;
		// reset the doc bidi attribute
		m_bBidiMode = false;
		return true;
	case RTF_KW_rtlsect:
		UT_DEBUGMSG(("rtf imp.: rtlsect\n"));
		m_currentRTFState.m_sectionProps.m_dir = UT_BIDI_RTL;
		return true;

	case RTF_KW_rtlmark:
	case RTF_KW_rtlch:
		xxx_UT_DEBUGMSG(("rtf imp.: rtlch\n"));
		m_currentRTFState.m_charProps.m_dir = UT_BIDI_RTL;
		// we enter bidi mode if we encounter a character
		// formatting inconsistent with the base direction of the
		// paragraph; once in bidi mode, we have to stay there
		// until the end of the current pragraph
		m_bBidiMode = m_bBidiMode ||
			(m_currentRTFState.m_charProps.m_dir ^ m_currentRTFState.m_paraProps.m_dir);
		return true;
	case RTF_KW_row:
		HandleRow();
		return true;

	case RTF_KW_s:
		m_currentRTFState.m_paraProps.m_styleNumber = param;
		return true;
	case RTF_KW_stylesheet:
		return HandleStyleDefinition();
	case RTF_KW_strike:
	case RTF_KW_striked:
		return HandleStrikeout(fParam ? (param != 0) : true);
	case RTF_KW_sect:
		return StartNewSection();
	case RTF_KW_sectd:
		return ResetSectionAttributes();
	case RTF_KW_sa:
		m_currentRTFState.m_paraProps.m_spaceAfter = param;
		return true;
	case RTF_KW_sb:
		m_currentRTFState.m_paraProps.m_spaceBefore = param;
		return true;
	case RTF_KW_sbknone:
		m_currentRTFState.m_sectionProps.m_breakType = RTFProps_SectionProps::sbkNone;
		return true;
	case RTF_KW_sbkcol:
		m_currentRTFState.m_sectionProps.m_breakType = RTFProps_SectionProps::sbkColumn;
		return true;
	case RTF_KW_sbkpage:
		m_currentRTFState.m_sectionProps.m_breakType = RTFProps_SectionProps::sbkPage;
		return true;
	case RTF_KW_sbkeven:
		m_currentRTFState.m_sectionProps.m_breakType = RTFProps_SectionProps::sbkEven;
		return true;
	case RTF_KW_sbkodd:
		m_currentRTFState.m_sectionProps.m_breakType = RTFProps_SectionProps::sbkOdd;
		return true;
	case RTF_KW_shp:
// Found a positioned thingy
		HandleShape();
		return true;
 	case RTF_KW_sl:
		if (!fParam  ||  param == 0) {
			m_currentRTFState.m_paraProps.m_lineSpaceVal = 360;
		}
		else {
			m_currentRTFState.m_paraProps.m_lineSpaceVal = param;
		}
		return true;
	case RTF_KW_slmult:
		m_currentRTFState.m_paraProps.m_lineSpaceExact = (!fParam  ||  param == 0);
		return true;
	case RTF_KW_super:
		return HandleSuperscript(fParam ? false : true);
	case RTF_KW_sub:
		return HandleSubscript(fParam ? false : true);

	case RTF_KW_tab:
		return ParseChar('\t');
	case RTF_KW_tx:
	{
		UT_return_val_if_fail(fParam, false);	// tabstops should have parameters
		bool bres = AddTabstop(param,
							   m_currentRTFState.m_paraProps.m_curTabType,
							   m_currentRTFState.m_paraProps.m_curTabLeader);
		m_currentRTFState.m_paraProps.m_curTabType = FL_TAB_LEFT;
//			m_currentRTFState.m_paraProps.m_curTabLeader = FL_LEADER_NONE;
		return bres;
	}
	case RTF_KW_tb:
	{
		UT_return_val_if_fail(fParam, false);	// tabstops should have parameters

		bool bres = AddTabstop(param,FL_TAB_BAR,
							   m_currentRTFState.m_paraProps.m_curTabLeader);
		m_currentRTFState.m_paraProps.m_curTabType = FL_TAB_BAR;
//			m_currentRTFState.m_paraProps.m_curTabLeader = FL_LEADER_NONE;
		return bres;
	}
	case RTF_KW_tqr:
		m_currentRTFState.m_paraProps.m_curTabType = FL_TAB_RIGHT;
		return true;
	case RTF_KW_tqc:
		m_currentRTFState.m_paraProps.m_curTabType = FL_TAB_CENTER;
		return true;
	case RTF_KW_tqdec:
		m_currentRTFState.m_paraProps.m_curTabType = FL_TAB_DECIMAL;
		return true;
	case RTF_KW_tldot:
		m_currentRTFState.m_paraProps.m_curTabLeader = FL_LEADER_DOT;
		return true;
	case RTF_KW_tlhyph:
		m_currentRTFState.m_paraProps.m_curTabLeader = FL_LEADER_HYPHEN;
		return true;
	case RTF_KW_trautofit:
		if(getTable())
		{
			if(param==1)
			{
				getTable()->setAutoFit(true);
			}
		}
		return true;
	case RTF_KW_trleft:
		if(getTable())
		{
			double dLeftPos = static_cast<double>(param)/1440.0;
			std::string sLeftPos = UT_formatDimensionString(DIM_IN,dLeftPos,NULL);
			getTable()->setProp("table-column-leftpos",sLeftPos);
		}
		return true;
	case RTF_KW_tlul:
		m_currentRTFState.m_paraProps.m_curTabLeader = FL_LEADER_UNDERLINE;
		return true;
	case RTF_KW_tleq:
		m_currentRTFState.m_paraProps.m_curTabLeader = FL_LEADER_EQUALSIGN;
		return true;
	case RTF_KW_trowd:
		{
			m_bRowJustPassed = false;
			m_bDoCloseTable = false;
			UT_DEBUGMSG(("Doing trowd \n"));
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
				xxx_UT_DEBUGMSG(("At trowd m_tableLevel %d nestDepth %d \n",m_currentRTFState.m_paraProps.m_tableLevel,m_TableControl.getNestDepth()));
				while(m_currentRTFState.m_paraProps.m_tableLevel > m_TableControl.getNestDepth())
				{
					xxx_UT_DEBUGMSG(("SEVIOR: Doing pard OpenTable \n"));
					OpenTable();
				}
				xxx_UT_DEBUGMSG(("After trowd m_tableLevel %d nestDepth %d \n",m_currentRTFState.m_paraProps.m_tableLevel,m_TableControl.getNestDepth()));
			}
			else if((m_currentRTFState.m_paraProps.m_tableLevel >= 0) && m_currentRTFState.m_paraProps.m_tableLevel < m_TableControl.getNestDepth())
			{
				xxx_UT_DEBUGMSG(("At trowd m_tableLevel %d nestDepth %d \n",m_currentRTFState.m_paraProps.m_tableLevel,m_TableControl.getNestDepth()));
				while(m_currentRTFState.m_paraProps.m_tableLevel < m_TableControl.getNestDepth())
				{
					xxx_UT_DEBUGMSG(("SEVIOR:Close Table trowd1  \n"));
					CloseTable();
					m_bCellBlank = true;
				}
				xxx_UT_DEBUGMSG(("After trowd m_tableLevel %d nestDepth %d \n",m_currentRTFState.m_paraProps.m_tableLevel,m_TableControl.getNestDepth()));
			}
//
// Look to see if m_bNestTableProps is true for nested tables.
//
// To all RTF hackers, getting these 0's and 1's is extremely important.
// Don't change them unless you can verify that a huge range of RTF docs with
// tables (nested and unnested) get imported corectly. Martin 10/5/2003
			else if((m_TableControl.getNestDepth() > 0) && !m_bNestTableProps)
			{
				while(m_TableControl.getNestDepth() > 1)
				{
					xxx_UT_DEBUGMSG(("SEVIOR:Close Table trowd2 \n"));
					CloseTable();
					m_bCellBlank = true;
				}
				m_currentRTFState.m_paraProps.m_tableLevel = 1; 
			}

			//If a trowd appears without  a preceding \cell we close the previous table

			if(!m_bCellBlank && !m_bNestTableProps)
			{
				xxx_UT_DEBUGMSG(("After trowd closing table coz no cell detected -1\n"));
				CloseTable();
			}


// Another way of detecting if a trowd appears without a preceding \cell.
// Close the previous table. This should always work.
		
			else if(!m_bCellHandled && m_bContentFlushed)
			{
				UT_DEBUGMSG(("After trowd closing table coz no cell detected - 2\n"));
				CloseTable(); 
			}
			//   			m_bContentFlushed = false;
			m_bNestTableProps = false;
			ResetCellAttributes();
			ResetTableAttributes();
			break;
		}

	case RTF_KW_ul:
	case RTF_KW_uld:
	case RTF_KW_uldash:
	case RTF_KW_uldashd:
	case RTF_KW_uldashdd:
	case RTF_KW_uldb:
	case RTF_KW_ulth:
	case RTF_KW_ulw:
	case RTF_KW_ulwave:
		return HandleUnderline(fParam ? (param != 0) : true);
	case RTF_KW_ulnone:
		return HandleUnderline(0);
	case RTF_KW_uc:
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
	case RTF_KW_u:
	{
		/* RTF is limited to +/-32K ints so we need to use negative numbers for large unicode values.
		 * So, check for Unicode chars wrapped to negative values.
		 */
		static UT_UCS4Char buf = 0x10000;
		bool bResult;
		if (param < 0)
		{
			unsigned short tmp = (unsigned short) ((signed short) param);
			param = (UT_sint32) tmp;
		}
		if ((unsigned) param >= 0xD800 && (unsigned) param <= 0xDBFF)
		{
			buf = (UT_UCS4Char) param - 0xD800;
			buf <<= 10;
			buf += 0x10000;
			m_currentRTFState.m_unicodeInAlternate = m_currentRTFState.m_unicodeAlternateSkipCount;
			return true;
		}
		if ((unsigned) param >= 0xDC00 && (unsigned) param <= 0xDFFF)
		{
			buf += (UT_UCS4Char) param;
			buf -= 0xDC00;
			bResult = ParseChar(static_cast<UT_UCSChar>(buf));
			buf = 0x10000;
		}
		else
			bResult = ParseChar(static_cast<UT_UCSChar>(param));
		m_currentRTFState.m_unicodeInAlternate = m_currentRTFState.m_unicodeAlternateSkipCount;
		return bResult;
	}
	case RTF_KW_up:
		// superscript with position. Default is 6.
		// subscript: see dn keyword
		return HandleSuperscriptPosition (fParam ? param : 6);

	case RTF_KW_v:
		HandleHidden(fParam ? (param != 0) : true);
		break;
		
	case RTF_KW_STAR:
		return HandleStarKeyword();
		break;
	case RTF_KW_QUOTE:
		m_currentRTFState.m_internalState = RTFStateStore::risHex;
		return true;
		break;
	case RTF_KW_OPENCBRACE:
		ParseChar('{');
		return true;
		break;
	case RTF_KW_CLOSECBRACE:
		ParseChar('}');
		return true;
		break;
	case RTF_KW_BACKSLASH:
		ParseChar('\\');
		return true;
		break;
	case RTF_KW_TILDE:
		ParseChar(UCS_NBSP);
		return true;
		break;
	case RTF_KW_HYPHEN:
		// TODO handle optional hyphen. Currently simply ignore them.
		xxx_UT_DEBUGMSG (("RTF: TODO handle optionnal hyphen\n"));
		return true;
		break;
	case RTF_KW_UNDERSCORE:
		// currently simply make a standard hyphen
		ParseChar('-');	// TODO - make these optional and nonbreaking
		return true;
		break;
	case RTF_KW_CR:	// see bug 2174 ( Cocoa RTF)
	case RTF_KW_LF:
		return StartNewPara();
		break;
	default:
		xxx_UT_DEBUGMSG(("Unhandled keyword in dispatcher: %d\n", keywordID));
	}
	return true;
}




bool IE_Imp_RTF::HandleStarKeyword() 
{
	unsigned char keyword_star[MAX_KEYWORD_LEN];
	UT_sint32 parameter_star = 0;
	bool parameterUsed_star = false;
	xxx_UT_DEBUGMSG(("RTF Level in HandlStarKeyword %d \n",m_stateStack.getDepth()));	
	m_currentRTFState.m_bInKeywordStar = true;
	if (ReadKeyword(keyword_star, &parameter_star, &parameterUsed_star,
					MAX_KEYWORD_LEN))
	{
		xxx_UT_DEBUGMSG(("keyword_star %s read after * \n",keyword_star));

		if( strcmp(reinterpret_cast<char*>(keyword_star), "\\")== 0)
		{
			if (ReadKeyword(keyword_star, &parameter_star, &parameterUsed_star,
							MAX_KEYWORD_LEN))
			{

				xxx_UT_DEBUGMSG(("actual keyword_star %s read after * \n",keyword_star));
				RTF_KEYWORD_ID keywordID = KeywordToID(reinterpret_cast<char *>(keyword_star));
				switch (keywordID) {
				case RTF_KW_ol:
					return HandleOverline(parameterUsed_star ?
										  (parameter_star != 0): true);
					break;
				case RTF_KW_pn:
					return HandleLists( m_currentRTFState.m_paraProps.m_rtfListTable);
					break;
				case RTF_KW_listtable:
					return ReadListTable();
					break;
				case RTF_KW_listoverridetable:
					return ReadListOverrideTable();
					break;
				case RTF_KW_abilist:
					return HandleAbiLists();
					break;
				case RTF_KW_topline:
					return HandleTopline(parameterUsed_star ?
										 (parameter_star != 0): true);
					break;
				case RTF_KW_botline:
					return HandleBotline(parameterUsed_star ?
										 (parameter_star != 0): true);
					break;
				case RTF_KW_listtag:
					return HandleListTag(parameter_star);
					break;
				case RTF_KW_abicellprops:
					if(!bUseInsertNotAppend())
					{
						xxx_UT_DEBUGMSG (("ignoring abicellprops on file import \n"));
						m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
						return true;
					}
					if(m_iIsInHeaderFooter == 1)
					{
						m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
						return true;
					}
					return HandleAbiCell();
					break;
				case RTF_KW_abirevision:
					{
						// the abirevision keyword is enclosed in {}, having its own
						// stack; we however need to set the m_abiRevision parameter of
						// the parent stack
						bool bSuccess = PopRTFState();

						UT_return_val_if_fail( bSuccess, false );
						
						// OK scan through the text until a closing delimeter is
						// found
						unsigned char ch;
								
								
						while(ReadCharFromFile(&ch))
						{
							if(ch == '}')
								break;
									
							if(ch == '\\')
							{
								if (!ReadCharFromFile(&ch))
									return false;
							}

								
							m_currentRTFState.m_revAttr += ch;
						}

						return true;
					}
						
				case RTF_KW_abitableprops:
					if(!bUseInsertNotAppend())
					{
						UT_DEBUGMSG (("ignoring abictableprops on file import \n"));
						m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
						return true;
					}
					if(m_iIsInHeaderFooter == 1)
					{
						m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
						return true;
					}
					if(m_iIsInHeaderFooter == 0)
					{
						XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
						if(pFrame == NULL)
						{
							m_iIsInHeaderFooter =1;
							m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
							return true;
						}
						// TODO fix this as it appears to be a real hack. We shouldn't have access to 
						// this from importers.
						FV_View * pView = static_cast<FV_View*>(pFrame->getCurrentView());
						if(pView == NULL)
						{
							m_iIsInHeaderFooter =1;
							m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
							return true;
						}
						if(pView->isInEndnote() || pView->isInFootnote())
						{
							m_iIsInHeaderFooter =1;
							m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
							return true;
						}
						if(pView->isHdrFtrEdit() && (pView->isInTable() || m_pasteTableStack.getDepth() == 2) )
						{
//
// No nested Tables in header/footer
//
							m_iIsInHeaderFooter =1;
							m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
							return true;
						}

						m_iIsInHeaderFooter = 2;
					}
					return HandleAbiTable();
					break;
				case RTF_KW_abiendtable:
					if(!bUseInsertNotAppend())
					{
						UT_DEBUGMSG (("ignoring abiendtable on file import \n"));
						m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
						return true;
					}
					if(m_iIsInHeaderFooter == 1)
					{
						m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
						return true;
					}
					return HandleAbiEndTable();
					break;
				case RTF_KW_abiendcell:
					if(!bUseInsertNotAppend())
					{
						UT_DEBUGMSG (("ignoring abiendcell on file import \n"));
						m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
						return true;
					}
					if(m_iIsInHeaderFooter == 1)
					{
						m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
						return true;
					}
					return HandleAbiEndCell();
					break;
				case RTF_KW_abimathml:
					return HandleAbiMathml();
					break;
				case RTF_KW_abimathmldata:
					return CreateDataItemfromStream();
					break;
				case RTF_KW_abilatexdata:
					return CreateDataItemfromStream();
					break;
				case RTF_KW_abiembed:
					return HandleAbiEmbed();
					break;
				case RTF_KW_abiembeddata:
					return CreateDataItemfromStream();
					break;
				case RTF_KW_shppict:
					UT_DEBUGMSG (("handling shppict\n"));
					HandleShapePict();
					return true;
					break;
				case RTF_KW_shpinst:
					UT_DEBUGMSG(("Doing shpinst \n"));
					SkipCurrentGroup();
					/*
					m_iStackDepthAtFrame = m_stateStack.getDepth();
					m_bFrameOpen = true;
					*/
					return true;
					break;
				case RTF_KW_nesttableprops:
					UT_DEBUGMSG(("SEVIOR: Doing nestableprops opentable \n"));
					m_bNestTableProps = true;
					// OpenTable();
					return true;
					break;
				case RTF_KW_bkmkstart:
					return HandleBookmark (RBT_START);
					break;
				case RTF_KW_bkmkend:
					return HandleBookmark (RBT_END);
				case RTF_KW_rdfanchorstart:
					return HandleRDFAnchor (RBT_START);
					break;
				case RTF_KW_rdfanchorend:
					return HandleRDFAnchor (RBT_END);
				case RTF_KW_deltamoveid:
					return HandleDeltaMoveID();
				case RTF_KW_cs:
					UT_DEBUGMSG(("Found cs in readword stream just ignore \n"));
					return true;
					break;
#if 1
//
// Fixme I need to be able to handle footnotes inside tables in RTF
//
				case RTF_KW_footnote:
					if(bUseInsertNotAppend())
					{
						XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
						if(pFrame == NULL)
						{
							m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
							return true;
						}
						FV_View * pView = static_cast<FV_View*>(pFrame->getCurrentView());
						if(pView == NULL)
						{
							m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
							return true;
						}
						if(pView->isHdrFtrEdit())
						{
							m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
							return true;
						}
						if(pView->isInFootnote(m_dposPaste) || pView->isInEndnote(m_dposPaste) )
						{
							m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
							return true;
						}
					}
					//HandleFootnote();

					m_bFootnotePending = true;
					return true;
					break;
#endif
//
// Decode our own field extensions
//
				case RTF_KW_abifieldD:
				{
					char * pszField = strstr(reinterpret_cast<char *>(keyword_star),"D");
					pszField++;
					char * pszAbiField = g_strdup(pszField);
					char * pszD = strstr(pszAbiField,"D");
					if(pszD)
					{
						*pszD = '_';
						UT_DEBUGMSG(("Appending Abi field %s \n",pszAbiField));
						return _appendField(pszAbiField);
					}
					FREEP(pszAbiField);
					break;					
				}
				case RTF_KW_hlinkbase:
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
				case RTF_KW_revtbl:
					return ReadRevisionTable();
					
				case RTF_KW_atnid:
				{
					if(NULL == m_pAnnotation)
					{
						StartAnnotation();
						UT_DEBUGMSG(("found atnid without annotation\n"));
					}
					UT_UTF8String sContent;
					ReadContentFromFile(sContent);
					UT_DEBUGMSG(("atnid content %s \n", sContent.utf8_str()));
					m_pAnnotation->m_sAuthorId = sContent;
					return true;
				}
				case RTF_KW_rdf:
					return ReadRDFTriples();
						
				case RTF_KW_atnauthor:
				{
					//
					// Annotation Author
					UT_DEBUGMSG(("Handling atnauthor keyword \n"));
					if(NULL == m_pAnnotation)
					{
						StartAnnotation();
						UT_DEBUGMSG(("found atnauthor without annotation\n"));
					}
					UT_UTF8String sContent;
					ReadContentFromFile(sContent);
					UT_DEBUGMSG(("atnauthor content %s \n", sContent.utf8_str()));
					m_pAnnotation->m_sAuthor = sContent;
					return true;
				}
				case RTF_KW_atrfend:
					//return EndAnnotation();
					return true;
				case RTF_KW_atndate:
				{
					//
					// date of the annotation
					UT_DEBUGMSG(("Found annotation date %p \n",m_pAnnotation));
					if(NULL == m_pAnnotation)
					{
						UT_DEBUGMSG(("found atndate without annotation"));
						return true;
					}
					UT_UTF8String sContent;
					ReadContentFromFile(sContent);
					UT_DEBUGMSG(("annotation date is %s \n",sContent.utf8_str()));
					m_pAnnotation->m_sDate = sContent;
					return true;
				}
				case RTF_KW_annotation:
				{
					//
					// Annotation content
					UT_DEBUGMSG(("Found annotation content m_pAnnotation %p \n",m_pAnnotation));
					if(m_pAnnotation == NULL)
					{
						UT_DEBUGMSG(("found annotation without annotation"));
						return true;
					}
					//					if(m_pAnnotation->m_iAnnNumber != parameter_star)
					//	{
					//	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					//	return false;
					//	}
					m_pAnnotation->m_iRTFLevel = m_stateStack.getDepth();
					xxx_UT_DEBUGMSG(("Found annotation content depth %d \n",m_pAnnotation->m_iRTFLevel));
					return true;
				}
				case RTF_KW_atnref:
				{
					//
					// Annotation reference number
					UT_DEBUGMSG(("Write code to handle atnref \n"));
					xxx_UT_DEBUGMSG(("RTF Level inside atnref %d \n",m_stateStack.getDepth()));
					return true;
				}
				break;
				case RTF_KW_atrfstart:
				{
					//StartAnnotation();
					return true;
				}
				default:

					UT_DEBUGMSG (("RTF: default case star keyword %s not handled\n", keyword_star));
					break;
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

void IE_Imp_RTF::StartAnnotation()
{
	//
	// Start of Annotated region
	if(m_pAnnotation == NULL)
		m_pAnnotation = new ABI_RTF_Annotation();
	UT_DEBUGMSG(("created m_pAnnotation %p \n",m_pAnnotation));
	m_pAnnotation->m_iAnnNumber = ABI_RTF_Annotation::newNumber();
	std::string sAnnNum;
	sAnnNum = UT_std_string_sprintf("%d",m_pAnnotation->m_iAnnNumber);
	const gchar * attr[3] = {PT_ANNOTATION_NUMBER,NULL,NULL};
	attr[1] = sAnnNum.c_str();
	UT_DEBUGMSG(("Handling atrfstart number %d \n",m_pAnnotation->m_iAnnNumber));
	if(!bUseInsertNotAppend())
	{
		FlushStoredChars();
		getDoc()->appendObject(PTO_Annotation, attr);
		//
		// Remember the annotation field frag. We'll insert
		// the annotation content after this
		//
		m_pAnnotation->m_pInsertFrag = getDoc()->getLastFrag();
	}
	else
	{
		m_pAnnotation->m_Annpos = m_dposPaste;
		
	}
}


void IE_Imp_RTF::EndAnnotation()
{
	//
	// End of Annotated region
	UT_DEBUGMSG(("found annotation end \n"));
	if(NULL == m_pAnnotation)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}
	//if(m_pAnnotation->m_iAnnNumber != number)
	//{
	//	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	//	return false;
	//}
	std::string sAnnNum;
	sAnnNum = UT_std_string_sprintf("%d",m_pAnnotation->m_iAnnNumber);
	const gchar * attr[3] = {PT_ANNOTATION_NUMBER,NULL,NULL};
	attr[1] = sAnnNum.c_str();
	UT_DEBUGMSG(("found annotation end id %d  \n",m_pAnnotation->m_iAnnNumber));
	
	if(!bUseInsertNotAppend())
	{
		FlushStoredChars();
		getDoc()->appendObject(PTO_Annotation, NULL);
	}
	else
	{
		UT_DEBUGMSG(("Pasting EndAnnotation at %d  \n",m_dposPaste));
		bool bRet = getDoc()->insertObject(m_dposPaste, PTO_Annotation, NULL,NULL);
		
		if(bRet)
		{
			UT_DEBUGMSG(("Pasting End Annotation at %d saved pos %d \n",m_dposPaste,m_posSavedDocPosition));
			if(m_posSavedDocPosition >m_dposPaste )
				m_posSavedDocPosition++;
			m_dposPaste++;
			UT_DEBUGMSG(("Pasting Begin Annotation at %d dposPaste %d saved pos %d \n",m_pAnnotation->m_Annpos,m_dposPaste,m_posSavedDocPosition));
			
			bRet = getDoc()->insertObject(m_pAnnotation->m_Annpos, PTO_Annotation, attr, NULL);
			if(m_posSavedDocPosition >m_dposPaste )
				m_posSavedDocPosition++;
			m_dposPaste++;
		}
		
	}
}

void IE_Imp_RTF::_formRevisionAttr(std::string & attr, const std::string & props, const std::string & styleName)
{
	attr.clear();
	
	if(m_currentRTFState.m_charProps.m_eRevision == PP_REVISION_NONE)
	{
		return;
	}
	
	
	switch(m_currentRTFState.m_charProps.m_eRevision)
	{
		case PP_REVISION_DELETION:
			attr += '-';
			break;
		case PP_REVISION_FMT_CHANGE:
			attr += '!';
			break;
			
		case PP_REVISION_ADDITION:
		case PP_REVISION_ADDITION_AND_FMT:
		default:
			; // nothing
	}

	attr += UT_std_string_sprintf("%d", m_currentRTFState.m_charProps.m_iCurrentRevisionId);


	if(m_currentRTFState.m_charProps.m_eRevision == PP_REVISION_DELETION)
	{
		// ignore props
		return;
	}
	
	attr += '{';
	attr += props;
	attr += '}';
	if(!styleName.empty())
	{
		attr += '{';
		attr += PT_STYLE_ATTRIBUTE_NAME;
		attr += ';';
		attr += styleName;
		attr += '}';
	}
}



bool IE_Imp_RTF::buildCharacterProps(std::string & propBuffer)
{
	std::string tempBuffer;
	// bold
	propBuffer += "font-weight:";
	propBuffer += m_currentRTFState.m_charProps.m_bold ? "bold" : "normal";

	// italic
	propBuffer += "; font-style:";
	propBuffer += m_currentRTFState.m_charProps.m_italic ? "italic" : "normal";

	// hidden
	if(m_currentRTFState.m_charProps.m_Hidden)
	{
		propBuffer += "; display:none";
	}

	// underline & overline & strike-out
	propBuffer += "; text-decoration:";
	static std::string decors;
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
	// If Font is too big inside a table can trigger an infinite too during layout
	//
	//	UT_ASSERT(m_currentRTFState.m_charProps.m_fontSize < 120);
	propBuffer += UT_std_string_sprintf("; font-size:%spt", std_size_string(static_cast<float>(m_currentRTFState.m_charProps.m_fontSize)));
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
		propBuffer += UT_std_string_sprintf("; color:%06x", colour);
	}

	if (m_currentRTFState.m_charProps.m_hasBgColour)
	{
		// colour, only if one has been set. See bug 1324
		UT_sint32 bgColour = GetNthTableBgColour(m_currentRTFState.m_charProps.m_bgcolourNumber);

		if (bgColour != -1) // invalid and should be white
		{
			propBuffer += UT_std_string_sprintf("; bgcolor:%06x", bgColour);
		}
	}

	if(m_currentRTFState.m_charProps.m_listTag != 0)
	{
// List Tag to hang lists off
		propBuffer += UT_std_string_sprintf("; list-tag:%d",m_currentRTFState.m_charProps.m_listTag);
	}

	if(m_currentRTFState.m_charProps.m_szLang != 0)
	{
		propBuffer += "; lang:";
		propBuffer += m_currentRTFState.m_charProps.m_szLang;
	}

	if(m_currentRTFState.m_charProps.m_dirOverride == UT_BIDI_LTR)
	{
		propBuffer += ";dir-override:ltr";
	}
	else if(m_currentRTFState.m_charProps.m_dirOverride == UT_BIDI_RTL)
	{
		propBuffer += ";dir-override:rtl";
	}
	
	return true;
}


/*!
 * Returns true if we're pasting text rather than parsing a file
 */
bool IE_Imp_RTF::bUseInsertNotAppend(void)
{
	return ((m_pImportFile == NULL) && !m_parsingHdrFtr );
}

// in non-bidi doc we just append the current format and text; in bidi
// documents we crawl over the text looking for neutral characters;
// when we find one, we see if the implied override is identical to
// directional properties on either side of the character: if yes, we
// leave the character as it is; if not we issued the override. This
// saves us inserting overrides on most space characters in the document
bool IE_Imp_RTF::_appendSpan()
{
	const gchar* pProps = "props";
	const gchar* pRevs  = "revision";
	const gchar* pStyle = PT_STYLE_ATTRIBUTE_NAME;

	const gchar* propsArray[5] = {NULL, NULL, NULL, NULL, NULL};

	std::string prop_basic;
	std::string revision;
	buildCharacterProps(prop_basic);

	std::string prop_ltr;
	std::string prop_rtl;

	UT_uint32 iPropOffset = 0;

	bool bRevisedABI = (m_currentRTFState.m_revAttr.size() != 0);
	bool bRevisedRTF = !bRevisedABI && (m_currentRTFState.m_charProps.m_eRevision != PP_REVISION_NONE);

	propsArray[0] = bRevisedABI || bRevisedRTF ? pRevs : pProps;
	propsArray[1] = prop_basic.c_str();

	if(m_currentRTFState.m_charProps.m_styleNumber >= 0
	   && static_cast<UT_uint32>(m_currentRTFState.m_charProps.m_styleNumber) < m_styleTable.size())
	{
		propsArray[2] = pStyle;
		propsArray[3] = m_styleTable[m_currentRTFState.m_charProps.m_styleNumber].c_str();
	}

	if(bRevisedRTF)
	{
		_formRevisionAttr(revision, prop_basic, propsArray[3]);

		// the style attribute is inside the revision, clear it out of the props array
		propsArray[1] = revision.c_str();
		propsArray[2] = NULL;
		propsArray[3] = NULL;

		iPropOffset = 2;
	}
	else if(bRevisedABI)
	{
		propsArray[1] = m_currentRTFState.m_revAttr.utf8_str();
		propsArray[2] = NULL;
		propsArray[3] = NULL;

		iPropOffset = 2;
	}
	else
	{
		prop_ltr = prop_basic;
		prop_rtl = prop_basic;

		prop_ltr += ';';
		prop_rtl += ';';
	}

	prop_ltr += "dir-override:ltr";
	prop_rtl += "dir-override:rtl";
	
	
	UT_UCS4Char * p;
	UT_uint32 iLen = m_gbBlock.getLength();

	if(m_bBidiMode)
	{
		UT_BidiCharType cType;
		UT_uint32 iLast = 0;
		UT_UCS4Char c = *(reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(0)));
	
		cType = UT_bidiGetCharType(c);
	
		for(UT_uint32 i = 0; i < iLen; i++)
		{
			if(i < iLen - 1 )
			{
				c = *(reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(i+1)));
				m_iBidiNextType = UT_bidiGetCharType(c);
			}
			else
			{
				m_iBidiNextType = UT_BIDI_UNSET;
			}
		
		
			if(UT_BIDI_IS_NEUTRAL(cType))
			{
				if(m_currentRTFState.m_charProps.m_dir == UT_BIDI_LTR
				   && m_iAutoBidiOverride != UT_BIDI_LTR
				   && (m_iBidiLastType != UT_BIDI_LTR || m_iBidiNextType != UT_BIDI_LTR))
				{
					if(i - iLast > 0)
					{
						p = reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(iLast));
						if(m_pDelayedFrag)
						{
							    if(!getDoc()->insertFmtMarkBeforeFrag(m_pDelayedFrag,propsArray))
									return false;
								UT_DEBUGMSG(("Appending span before %p \n",m_pDelayedFrag));
								if(!getDoc()->insertSpanBeforeFrag(m_pDelayedFrag,p, i- iLast))
									return false;
						}
						else
						{
							if(!getDoc()->appendFmt(propsArray))
								return false;
					
							if(!getDoc()->appendSpan(p, i - iLast))
								return false;
						}
					}
					m_iAutoBidiOverride = UT_BIDI_LTR;
					propsArray[iPropOffset] = pProps;
					propsArray[iPropOffset + 1] = prop_ltr.c_str();
					iLast = i;
				}
				else if(m_currentRTFState.m_charProps.m_dir == UT_BIDI_RTL
						&& m_iAutoBidiOverride != UT_BIDI_RTL
						&& (m_iBidiLastType != UT_BIDI_RTL || m_iBidiNextType != UT_BIDI_RTL))
				{
					if(i - iLast > 0)
					{
						p = reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(iLast));
						if(m_pDelayedFrag)
						{
							    if(!getDoc()->insertFmtMarkBeforeFrag(m_pDelayedFrag,propsArray))
									return false;
								if(!getDoc()->insertSpanBeforeFrag(m_pDelayedFrag,p,i - iLast))
									return false;
						}
						else
						{
								if(!getDoc()->appendFmt(propsArray))
									return false;

								if(!getDoc()->appendSpan(p, i - iLast))
									return false;
						}
					}
					m_iAutoBidiOverride = UT_BIDI_RTL;
					propsArray[iPropOffset] = pProps;
					propsArray[iPropOffset + 1] = prop_rtl.c_str();
					iLast = i;
				}
			}
			else
			{
				// strong character; if we previously issued an override,
				// we need to cancel it
				if(m_iAutoBidiOverride != static_cast<UT_uint32>(UT_BIDI_UNSET))
				{
					if(i - iLast > 0)
					{
						p = reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(iLast));
						if(m_pDelayedFrag)
						{
							    if(!getDoc()->insertFmtMarkBeforeFrag(m_pDelayedFrag,propsArray))
									return false;
								if(!getDoc()->insertSpanBeforeFrag(m_pDelayedFrag,p,i - iLast))
									return false;
						}
						else
						{
							    if(!getDoc()->appendFmt(propsArray))
									return false;
					
								if(!getDoc()->appendSpan(p, i - iLast))
									return false;
						}
					}
					m_iAutoBidiOverride = UT_BIDI_UNSET;

					if(bRevisedABI || bRevisedRTF)
					{
						propsArray[iPropOffset] = NULL;
						propsArray[iPropOffset + 1] = NULL;
					}
					else
					{
						propsArray[1] = prop_basic.c_str();
					}
					
					iLast = i;
				}
			}

			m_iBidiLastType = cType;
			cType = m_iBidiNextType;
		}

		// insert what is left over
		if(iLen - iLast > 0)
		{
			p = reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(iLast));
			if(m_pDelayedFrag)
			{
				if(!getDoc()->insertFmtMarkBeforeFrag(m_pDelayedFrag,propsArray))
					return false;
				if(!getDoc()->insertSpanBeforeFrag(m_pDelayedFrag,p,iLen - iLast))
				   return false;
			}
			else
			{
				if(!getDoc()->appendFmt(propsArray))
					return false;
					
				if(!getDoc()->appendSpan(p, iLen - iLast))
					return false;
			}
		}
	}
	else
	{
		// not a bidi doc, just do it the simple way
		p = reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(0));
		if(m_pDelayedFrag)
		{
			if(!getDoc()->insertFmtMarkBeforeFrag(m_pDelayedFrag,propsArray))
				return false;
			if(!getDoc()->insertSpanBeforeFrag(m_pDelayedFrag,p,iLen))
			   return false;
		}
		else
		{
			if(!getDoc()->appendFmt(propsArray))
				return false;
			if(!getDoc()->appendSpan(p, iLen))
				return false;
		}
	}
	
	return true;
}
	
bool IE_Imp_RTF::_insertSpan()
{
	const gchar* pProps = "props";
	const gchar* pRevs  = "revision";
	const gchar* pStyle = PT_STYLE_ATTRIBUTE_NAME;

	const gchar* propsArray[5] = {NULL, NULL, NULL, NULL, NULL};

	std::string prop_basic;
	std::string revision;
	buildCharacterProps(prop_basic);

	std::string prop_ltr;
	std::string prop_rtl;

	UT_uint32 iPropOffset = 0;
	
	bool bRevised = m_currentRTFState.m_charProps.m_eRevision != PP_REVISION_NONE;

	propsArray[0] = bRevised ? pRevs : pProps;
	propsArray[1] = prop_basic.c_str();
	
	UT_UCS4Char * p;
	UT_uint32 iLen = m_gbBlock.getLength();

	if(m_currentRTFState.m_charProps.m_styleNumber >= 0
	   && static_cast<UT_uint32>(m_currentRTFState.m_charProps.m_styleNumber) < m_styleTable.size())
	{
		propsArray[2] = pStyle;
		propsArray[3] = m_styleTable[m_currentRTFState.m_charProps.m_styleNumber].c_str();
	}
	

	if(bRevised)
	{
		_formRevisionAttr(revision, prop_basic, propsArray[3]);

		// the style attribute is inside the revision, clear it out of the props array
		propsArray[1] = revision.c_str();
		propsArray[2] = NULL;
		propsArray[3] = NULL;

		iPropOffset = 2;
	}
	else
	{
		prop_ltr = prop_basic;
		prop_rtl = prop_basic;

		prop_ltr += ';';
		prop_rtl += ';';
	}

	prop_ltr += "dir-override:ltr";
	prop_rtl += "dir-override:rtl";
	
	

	if(m_bBidiMode)
	{
		UT_BidiCharType cType;
		UT_uint32 iLast = 0;
		UT_UCS4Char c = *(reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(0)));
	
		cType = UT_bidiGetCharType(c);
	
		for(UT_uint32 i = 0; i < iLen; i++)
		{
			if(i < iLen - 1 )
			{
				c = *(reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(i+1)));
				m_iBidiNextType = UT_bidiGetCharType(c);
			}
			else
			{
				m_iBidiNextType = UT_BIDI_UNSET;
			}
		
		
			if(UT_BIDI_IS_NEUTRAL(cType))
			{
				if(m_currentRTFState.m_charProps.m_dir == UT_BIDI_LTR
				   && m_iAutoBidiOverride != UT_BIDI_LTR
				   && (m_iBidiLastType != UT_BIDI_LTR || m_iBidiNextType != UT_BIDI_LTR))
				{
					if(i - iLast > 0)
					{
						p = reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(iLast));
						if(getDoc()->isFrameAtPos(m_dposPaste-1) || getDoc()->isTableAtPos(m_dposPaste-1) || getDoc()->isCellAtPos(m_dposPaste-1))
						{
							getDoc()->insertStrux(m_dposPaste,PTX_Block);
							m_dposPaste++;
						}
						if(!getDoc()->insertSpan(m_dposPaste, p ,i - iLast))
							return false;
						
						if(!getDoc()->changeSpanFmt(PTC_SetFmt, m_dposPaste,m_dposPaste+ i - iLast,
													propsArray,NULL))
							return false;
						
						m_dposPaste += i - iLast;	
						if(m_posSavedDocPosition > 0)
							m_posSavedDocPosition += i - iLast;
					}
					m_iAutoBidiOverride = UT_BIDI_LTR;
					propsArray[iPropOffset] = pProps;
					propsArray[iPropOffset + 1] = prop_ltr.c_str();
					iLast = i;
				}
				else if(m_currentRTFState.m_charProps.m_dir == UT_BIDI_RTL
						&& m_iAutoBidiOverride != UT_BIDI_RTL
						&& (m_iBidiLastType != UT_BIDI_RTL || m_iBidiNextType != UT_BIDI_RTL))
				{
					if(i - iLast > 0)
					{
						p = reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(iLast));
						if(!getDoc()->insertSpan(m_dposPaste, p ,i - iLast))
							return false;
						
						if(!getDoc()->changeSpanFmt(PTC_SetFmt, m_dposPaste,m_dposPaste+ i - iLast,
													propsArray,NULL))
							return false;
						m_dposPaste += i - iLast;
						if(m_posSavedDocPosition > 0)
							m_posSavedDocPosition  += i - iLast;
					}
					m_iAutoBidiOverride = UT_BIDI_RTL;
					propsArray[iPropOffset] = pProps;
					propsArray[iPropOffset + 1] = prop_rtl.c_str();
					iLast = i;
				}
			}
			else
			{
				// strong character; if we previously issued an override,
				// we need to cancel it
				if(m_iAutoBidiOverride != static_cast<UT_uint32>(UT_BIDI_UNSET))
				{
					if(i - iLast > 0)
					{
						p = reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(iLast));
						if(!getDoc()->insertSpan(m_dposPaste, p ,i - iLast))
							return false;
						
						if(!getDoc()->changeSpanFmt(PTC_SetFmt, m_dposPaste, m_dposPaste + i - iLast,
													propsArray,NULL))
							return false;
						
						m_dposPaste += i - iLast;						
						if(m_posSavedDocPosition > 0)
							m_posSavedDocPosition  += i - iLast;
					}
					m_iAutoBidiOverride = UT_BIDI_UNSET;
					if(bRevised)
					{
						propsArray[iPropOffset] = NULL;
						propsArray[iPropOffset + 1] = NULL;
					}
					else
					{
						propsArray[1] = prop_basic.c_str();
					}
					
					iLast = i;
				}
			}

			m_iBidiLastType = cType;
			cType = m_iBidiNextType;
		}

		// insert what is left over
		if(iLen - iLast > 0)
		{
			p = reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(iLast));
			if(!getDoc()->insertSpan(m_dposPaste, p ,iLen - iLast))
				return false;
						
			if(!getDoc()->changeSpanFmt(PTC_SetFmt, m_dposPaste, m_dposPaste + iLen - iLast,
										propsArray,NULL))
				return false;
						
			m_dposPaste += iLen - iLast;						
			if(m_posSavedDocPosition > 0)
				m_posSavedDocPosition  += iLen - iLast;
		}
	}
	else
	{
		// not a bidi doc, just do it the simple way
		p = reinterpret_cast<UT_UCS4Char*>(m_gbBlock.getPointer(0));
		if(getDoc()->isFrameAtPos(m_dposPaste-1) || getDoc()->isTableAtPos(m_dposPaste-1) || getDoc()->isCellAtPos(m_dposPaste-1))
		{
			getDoc()->insertStrux(m_dposPaste,PTX_Block);
			m_dposPaste++;
			if(m_posSavedDocPosition > 0)
				m_posSavedDocPosition++;
		}
		if(!getDoc()->insertSpan(m_dposPaste, p ,iLen))
			return false;
						
		if(!getDoc()->changeSpanFmt(PTC_SetFmt, m_dposPaste, m_dposPaste + iLen,
									propsArray,NULL))
			return false;
						
		m_dposPaste += iLen;						
		if(m_posSavedDocPosition > 0)
			m_posSavedDocPosition  += iLen;
	}
	
	return true;
}

bool IE_Imp_RTF::ApplyCharacterAttributes()
{
	bool ok = false;
	if(isBlockNeededForPasteTable())
	{
		ApplyParagraphAttributes();
	}
	if(m_gbBlock.getLength() > 0)
	{
		if (!bUseInsertNotAppend())	// if we are reading from a file or parsing headers and footers
		{
			ok = _appendSpan();
		}
		else								// else we are pasting from a buffer
		{
			if( m_currentRTFState.m_paraProps.m_isList && (m_dposPaste == m_dOrigPos))
			{
				ApplyParagraphAttributes(true);	
			}
			ok = _insertSpan();
		}
		m_gbBlock.truncate(0);
		m_bContentFlushed = true;
		return ok;
	}
	else
	{
		const gchar* pProps = "props";
		const gchar* pStyle = PT_STYLE_ATTRIBUTE_NAME;
		std::string propBuffer;
		buildCharacterProps(propBuffer);

		const gchar* propsArray[7];
		propsArray[0] = pProps;
		propsArray[1] = propBuffer.c_str();
		propsArray[2] = NULL;
		propsArray[3] = NULL;
		propsArray[4] = NULL;
		propsArray[5] = NULL;
		propsArray[6] = NULL;
		UT_uint32 iPos = 2;

		if(m_currentRTFState.m_charProps.m_styleNumber >= 0
		   && static_cast<UT_uint32>(m_currentRTFState.m_charProps.m_styleNumber) < m_styleTable.size())
		{
			propsArray[iPos++] = pStyle;
			propsArray[iPos++] = m_styleTable[m_currentRTFState.m_charProps.m_styleNumber].c_str();
		}

		if(m_currentRTFState.m_revAttr.size())
		{
			propsArray[iPos++] = "revision";
			propsArray[iPos++] = m_currentRTFState.m_revAttr.utf8_str();
		}
		
		if (!bUseInsertNotAppend())	// if we are reading from a file or parsing headers and footers
		{
			if(m_pDelayedFrag)
			{
				if(!getDoc()->insertFmtMarkBeforeFrag(m_pDelayedFrag,propsArray))
					ok = getDoc()->insertFmtMarkBeforeFrag(m_pDelayedFrag,propsArray);
				ok = ok && getDoc()->insertFmtMarkBeforeFrag(m_pDelayedFrag);
			}
			else
			{
				if(!getDoc()->appendFmt(propsArray))
					ok = getDoc()->appendFmt(propsArray);
				ok = ok && getDoc()->appendFmtMark();
			}
		}
		else								// else we are pasting from a buffer
		{
			ok = getDoc()->changeSpanFmt(PTC_SetFmt,
												m_dposPaste,m_dposPaste,
												propsArray,NULL);
		}
		return ok;
	}
}


bool IE_Imp_RTF::ResetCharacterAttributes()
{
//	bool ok = FlushStoredChars();
	bool ok = true;
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
	if (!bUseInsertNotAppend())  // if we are reading a file - dont remap the ID
	{
	        return id;
	}
//
// Handle case of no id in any lists. If this is the case no need to remap
//
	fl_AutoNum * pAuto1 = getDoc()->getListByID(id);
	if(pAuto1 == NULL)
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
			else if(!m_bStruxInserted)
			    ///
			    /// Do the remapping!
			    ///
			{
				fl_AutoNum * pMapAuto = NULL;
				UT_uint32 nLists = getDoc()->getListsCount();
				UT_uint32 highestLevel = 0;
				pf_Frag_Strux* sdh;
//
// Get the List Type
//
				FL_ListType myType = NOT_A_LIST;
				fl_AutoLists al;
				UT_uint32 size_xml_lists = al.getXmlListsSize();
				for(j=0; j < size_xml_lists; j++)
				{
					if( strcmp(m_currentRTFState.m_paraProps.m_pszStyle,al.getXmlList(j))==0)
						break;
				}
				if(j < size_xml_lists)
					myType = static_cast<FL_ListType>(j);

				getDoc()->getStruxOfTypeFromPosition(m_dposPaste, PTX_Block,&sdh);
				for(j=0; j< nLists; j++)
				{
					fl_AutoNum * pAuto = getDoc()->getNthList(j);
					if(pAuto->isContainedByList(sdh) == true)
					{
						if(highestLevel < pAuto->getLevel())
						{
							highestLevel = pAuto->getLevel();
							FL_ListType thisType = pAuto->getType();
							if(thisType == myType)
							{
								pMapAuto = pAuto;
							}
						}
					}
				}
				if(pMapAuto == NULL )
				{
					mappedID = getDoc()->getUID(UT_UniqueId::List);
				}
				else if( getAbiList(i)->level <= pMapAuto->getLevel() && pMapAuto->getID() != 0)
				{
					mappedID = pMapAuto->getID();
				}
				else
				{
					mappedID = getDoc()->getUID(UT_UniqueId::List);
				}
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
	if (!bUseInsertNotAppend())  // if we are reading a file
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

bool IE_Imp_RTF::ApplyParagraphAttributes(bool bDontInsert)
{
	const gchar* attribs1[PT_MAX_ATTRIBUTES*2 + 1];
	UT_uint32 attribsCount=0;

//
// Look to see if the nesting level of our tables has changed.
//
	if(!bUseInsertNotAppend())
	{
		if(m_currentRTFState.m_paraProps.m_tableLevel > m_TableControl.getNestDepth())
		{
			if(m_bParaWrittenForSection)
			{
				xxx_UT_DEBUGMSG(("At Apply Paragraph m_tableLevel %d nestDepth %d \n",m_currentRTFState.m_paraProps.m_tableLevel,m_TableControl.getNestDepth()));
				while((m_currentRTFState.m_paraProps.m_tableLevel > m_TableControl.getNestDepth()) && (m_currentRTFState.m_paraProps.m_tableLevel > 1))
				{
					xxx_UT_DEBUGMSG(("SEVIOR: Doing pard OpenTable \n"));
					m_bCellBlank = false;
					m_bEndTableOpen = false;
					OpenTable();
				}
				xxx_UT_DEBUGMSG(("After Apply Paragraph m_tableLevel %d nestDepth %d \n",m_currentRTFState.m_paraProps.m_tableLevel,m_TableControl.getNestDepth()));
			}
		}
		else if((m_currentRTFState.m_paraProps.m_tableLevel >= 0) && m_currentRTFState.m_paraProps.m_tableLevel < m_TableControl.getNestDepth())
		{
			xxx_UT_DEBUGMSG(("At Apply Paragraph m_tableLevel %d nestDepth %d \n",m_currentRTFState.m_paraProps.m_tableLevel,m_TableControl.getNestDepth()));
			while(m_currentRTFState.m_paraProps.m_tableLevel < m_TableControl.getNestDepth())
			{
				CloseTable();
				if(m_bCellBlank)
				{
					m_bEndTableOpen = true;
				}
			}
			xxx_UT_DEBUGMSG(("After Apply Paragraph m_tableLevel %d nestDepth %d \n",m_currentRTFState.m_paraProps.m_tableLevel,m_TableControl.getNestDepth()));
		}
	}
	m_bParaWrittenForSection = true;
//
// Determine if we've dropped out of a table
//
	if(getTable() != NULL)
	{
		if(!m_currentRTFState.m_paraProps.m_bInTable)
		{
			m_bDoCloseTable = true;
		}
	}

	std::string propBuffer;
	std::string tempBuffer;
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
			UT_uint32 count = pOver->getTabStopVect(iLevel)->size();
			for(i=0; i< count; i++)
			{
				m_currentRTFState.m_paraProps.m_tabStops.push_back(pOver->getTabStopVect(iLevel)->at(i));
				m_currentRTFState.m_paraProps.m_tabTypes.push_back(pOver->getTabTypeVect(iLevel)->at(i));
				m_currentRTFState.m_paraProps.m_tabLeader.push_back(pOver->getTabLeaderVect(iLevel)->at(i));
			}
		}
	}
	if(!m_currentRTFState.m_paraProps.m_tabStops.empty())
	{
		UT_ASSERT_HARMLESS(m_currentRTFState.m_paraProps.m_tabStops.size() ==
					m_currentRTFState.m_paraProps.m_tabTypes.size() );
		UT_ASSERT_HARMLESS(m_currentRTFState.m_paraProps.m_tabStops.size() ==
					m_currentRTFState.m_paraProps.m_tabLeader.size() );
		propBuffer += "tabstops:";
		for (UT_uint32 i = 0; i < m_currentRTFState.m_paraProps.m_tabStops.size(); i++)
		{
			if (i > 0)
				propBuffer += ",";

			UT_sint32 tabTwips = m_currentRTFState.m_paraProps.m_tabStops.at(i);
			double tabIn = tabTwips/(20.0*72.);
			eTabType tabType = m_currentRTFState.m_paraProps.m_tabTypes.at(i);
			eTabLeader tabLeader = m_currentRTFState.m_paraProps.m_tabLeader.at(i);
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
			propBuffer += UT_std_string_sprintf("%s/%c%c", UT_convertInchesToDimensionString(DIM_IN,tabIn,"04"),cType,cLeader);
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
			// fall through
		case RTFProps_ParaProps::pjLeft:
			propBuffer += "left";
			break;
	}
	propBuffer += "; ";

	// indents - first, left and right, top and bottom
	propBuffer += UT_std_string_sprintf("margin-top:%s; ",		UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(m_currentRTFState.m_paraProps.m_spaceBefore)/1440));

	propBuffer += UT_std_string_sprintf("margin-bottom:%s; ",	UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(m_currentRTFState.m_paraProps.m_spaceAfter)/1440));

	propBuffer += "dom-dir:";
	if(m_currentRTFState.m_paraProps.m_dir == UT_BIDI_RTL)
		propBuffer += "rtl; ";
	else
		propBuffer += "ltr; ";

	//
	// Filled from List deefinition
	//
	if(!bWord97List || bAbiList)
	{
		propBuffer += UT_std_string_sprintf("margin-left:%s; ", UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(m_currentRTFState.m_paraProps.m_indentLeft)/1440));
	}
	propBuffer += UT_std_string_sprintf("margin-right:%s; ", UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(m_currentRTFState.m_paraProps.m_indentRight)/1440));
//
// Filled from List definition
//
	if(!bWord97List || bAbiList)
	{
		propBuffer += UT_std_string_sprintf("text-indent:%s; ", UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(m_currentRTFState.m_paraProps.m_indentFirst)/1440));
	}
// line spacing
	if (m_currentRTFState.m_paraProps.m_lineSpaceExact)
	{
        if (m_currentRTFState.m_paraProps.m_lineSpaceVal < 0) {  // exact spacing
			propBuffer += UT_std_string_sprintf("line-height:%spt;",    UT_convertToDimensionlessString(fabs(m_currentRTFState.m_paraProps.m_lineSpaceVal/20.0)));
		}
		else                                                         // "at least" spacing
		{
			propBuffer += UT_std_string_sprintf("line-height:%spt+;",    UT_convertToDimensionlessString(fabs(m_currentRTFState.m_paraProps.m_lineSpaceVal/20.0)));
		}
	}
	else                 // multiple line spacing
	{
		propBuffer += UT_std_string_sprintf("line-height:%s;",	UT_convertToDimensionlessString(fabs(m_currentRTFState.m_paraProps.m_lineSpaceVal/240)));
	}

	// Lists. If the paragraph has a list element handle it.
	std::string szLevel1;
	std::string szStyle;
	std::string szListID1;
	std::string szParentID1;
	UT_uint32 id = 0,parentID = 0,startValue = 0;
//
// This is for our own extensions to RTF.
//
	if(bUseInsertNotAppend())
	{
		//
		// don't paste lists into hdrftr's
		//
		XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
		FV_View * pView = static_cast<FV_View*>(pFrame->getCurrentView());
		if(pView && pView->isHdrFtrEdit())
		{
			bAbiList = false;
			bWord97List = false;
		}
	}
	if( bAbiList)
	{
	  //
	  // First off assemble the list attributes
	  //
		id = mapID(m_currentRTFState.m_paraProps.m_rawID);
		szListID1 = UT_std_string_sprintf("%d",id);
		parentID = mapParentID(m_currentRTFState.m_paraProps.m_rawParentID);
		szParentID1 = UT_std_string_sprintf("%d",parentID);
		if(parentID == 0)
			m_currentRTFState.m_paraProps.m_level = 1;
		szLevel1 = UT_std_string_sprintf("%d",m_currentRTFState.m_paraProps.m_level);

		attribs1[attribsCount++] = PT_LISTID_ATTRIBUTE_NAME;
		UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );
		attribs1[attribsCount++] = szListID1.c_str();
		UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );
		attribs1[attribsCount++] = PT_PARENTID_ATTRIBUTE_NAME;
		UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );
		attribs1[attribsCount++] = szParentID1.c_str();
		UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );
		attribs1[attribsCount++] = PT_LEVEL_ATTRIBUTE_NAME;
		UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );
		attribs1[attribsCount++] = szLevel1.c_str();
		UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );
		attribs1[attribsCount] = NULL;
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
		std::string val;
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
		attribs1[attribsCount++] = PT_LISTID_ATTRIBUTE_NAME;
		attribs1[attribsCount++] = szListID;
		attribs1[attribsCount++] = PT_PARENTID_ATTRIBUTE_NAME;
		attribs1[attribsCount++] = szParentID;
		attribs1[attribsCount++] = PT_LEVEL_ATTRIBUTE_NAME;
		attribs1[attribsCount++] = szLevel;
		attribs1[attribsCount]   = NULL;

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
			static std::string decors;
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
			propBuffer +=  UT_std_string_sprintf(" font-size:%spt;", std_size_string(static_cast<float>(pOver->getFontSize(iLevel))));
			UT_DEBUGMSG(("RTF: IMPORT!!!!! font sized changed in override %f \n",pOver->getFontSize(iLevel)));
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
				propBuffer += UT_std_string_sprintf(" color:%06x;", colour);
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
					propBuffer += UT_std_string_sprintf(" bgcolor:%06x;", bgColour);
				}
			}
		}
		//
		// Now handle the List properties
		//

		propBuffer += UT_std_string_sprintf("list-style:%s;",szListStyle);
		propBuffer += UT_std_string_sprintf("list-decimal:%s; ",szListDecimal);
		propBuffer += UT_std_string_sprintf("list-delim:%s; ",szListDelim);
		propBuffer += UT_std_string_sprintf("field-font:%s; ",szFieldFont);
		propBuffer += UT_std_string_sprintf("start-value:%s; ",szStartat);
		propBuffer += UT_std_string_sprintf("margin-left:%s; ",szAlign);
		propBuffer += UT_std_string_sprintf("text-indent:%s;", szIndent); // Note last entry has no ;
	}


	if(bAbiList)
	{
		//
		// Now handle the Abi List properties
		//
		szStyle = UT_std_string_sprintf("%s",m_currentRTFState.m_paraProps.m_pszStyle);
		propBuffer += UT_std_string_sprintf("list-style:%s;",m_currentRTFState.m_paraProps.m_pszStyle);
		propBuffer += UT_std_string_sprintf("list-decimal:%s; ",m_currentRTFState.m_paraProps.m_pszListDecimal);
		propBuffer += UT_std_string_sprintf("list-delim:%s; ",m_currentRTFState.m_paraProps.m_pszListDelim);
		propBuffer += UT_std_string_sprintf("field-font:%s; ",m_currentRTFState.m_paraProps.m_pszFieldFont);
		startValue = m_currentRTFState.m_paraProps.m_startValue;
		propBuffer += UT_std_string_sprintf("start-value:%d",startValue);
	}
	// Style name
	if( static_cast<UT_uint32>(m_currentRTFState.m_paraProps.m_styleNumber) < m_styleTable.size() &&
		(m_currentRTFState.m_paraProps.m_styleNumber >= 0) )
	{
		UT_uint32 styleNumber = m_currentRTFState.m_paraProps.m_styleNumber;
		const std::string & styleName = m_styleTable[styleNumber];
		attribs1[attribsCount++] = PT_STYLE_ATTRIBUTE_NAME;
		attribs1[attribsCount++] = styleName.c_str();
		attribs1[attribsCount]   = NULL;
	}

	// Borders & Shading are exported here
	double w = 0.0;
	UT_sint32 iCol = 0;
	if(m_currentRTFState.m_paraProps.m_bMergeBordersShading)
	{
		propBuffer += "border-merge:1; ";
	}
	if( m_currentRTFState.m_paraProps.m_bBotBorder)
	{
		propBuffer += UT_std_string_sprintf("bot-style:%d; ",m_currentRTFState.m_paraProps.m_iBotBorderStyle);
		w = static_cast<double>(m_currentRTFState.m_paraProps.m_iBotBorderWidth)/1440.;
		UT_LocaleTransactor t(LC_NUMERIC, "C");
		propBuffer += UT_std_string_sprintf("bot-thickness:%fin; ",w);
		w = static_cast<double>(m_currentRTFState.m_paraProps.m_iBotBorderSpacing)/1440.;
		propBuffer += UT_std_string_sprintf("bot-space:%fin; ",w);
		iCol = GetNthTableBgColour(m_currentRTFState.m_paraProps.m_iBotBorderCol);
		propBuffer += UT_std_string_sprintf("bot-color:%06x; ",iCol);
	}
	if( m_currentRTFState.m_paraProps.m_bLeftBorder)
	{
		propBuffer += UT_std_string_sprintf("left-style:%d; ",m_currentRTFState.m_paraProps.m_iLeftBorderStyle);
		UT_LocaleTransactor t(LC_NUMERIC, "C");
		w = static_cast<double>(m_currentRTFState.m_paraProps.m_iLeftBorderWidth)/1440.;
		propBuffer += UT_std_string_sprintf("left-thickness:%fin; ",w);
		w = static_cast<double>(m_currentRTFState.m_paraProps.m_iLeftBorderSpacing)/1440.;
		propBuffer += UT_std_string_sprintf("left-space:%fin; ",w);
		iCol = GetNthTableBgColour(m_currentRTFState.m_paraProps.m_iLeftBorderCol);
		propBuffer += UT_std_string_sprintf("left-color:%06x; ",iCol);
	}
	if( m_currentRTFState.m_paraProps.m_bRightBorder)
	{
		propBuffer += UT_std_string_sprintf("right-style:%d; ",m_currentRTFState.m_paraProps.m_iRightBorderStyle);
		UT_LocaleTransactor t(LC_NUMERIC, "C");
		w = static_cast<double>(m_currentRTFState.m_paraProps.m_iRightBorderWidth)/1440.;
		propBuffer += UT_std_string_sprintf("right-thickness:%fin; ",w);
		w = static_cast<double>(m_currentRTFState.m_paraProps.m_iRightBorderSpacing)/1440.;
		propBuffer += UT_std_string_sprintf("right-space:%fin; ",w);
		iCol = GetNthTableBgColour(m_currentRTFState.m_paraProps.m_iRightBorderCol);
		propBuffer += UT_std_string_sprintf("right-color:%06x; ",iCol);
	}
	if( m_currentRTFState.m_paraProps.m_bTopBorder)
	{
		propBuffer += UT_std_string_sprintf("top-style:%d; ",m_currentRTFState.m_paraProps.m_iTopBorderStyle);
		UT_LocaleTransactor t(LC_NUMERIC, "C");
		w = static_cast<double>(m_currentRTFState.m_paraProps.m_iTopBorderWidth)/1440.;
		propBuffer += UT_std_string_sprintf("top-thickness:%fin; ",w);
		w = static_cast<double>(m_currentRTFState.m_paraProps.m_iTopBorderSpacing)/1440.;
		propBuffer += UT_std_string_sprintf("top-space:%fin; ",w);
		iCol = GetNthTableBgColour(m_currentRTFState.m_paraProps.m_iTopBorderCol);
		propBuffer += UT_std_string_sprintf("top-color:%06x; ",iCol);
	}
	if(m_currentRTFState.m_paraProps.m_iShadingPattern)
	{
		propBuffer += UT_std_string_sprintf("shading-pattern:%d; ",m_currentRTFState.m_paraProps.m_iShadingPattern);

		if(m_currentRTFState.m_paraProps.m_iShadingForeCol > -1)
		{
			iCol = GetNthTableBgColour(m_currentRTFState.m_paraProps.m_iShadingForeCol);
			propBuffer += UT_std_string_sprintf("shading-foreground-color:%06x; ",iCol);
		}

		if(m_currentRTFState.m_paraProps.m_iShadingBackCol > -1)
		{
			iCol = GetNthTableBgColour(m_currentRTFState.m_paraProps.m_iShadingBackCol);
			propBuffer += UT_std_string_sprintf("shading-background-color:%06x; ",iCol);
			if(m_currentRTFState.m_paraProps.m_iShadingPattern == 1)
			{
				propBuffer += UT_std_string_sprintf("shading-foreground-color:%06x; ",iCol);
			}
		}
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
	attribs1[attribsCount++] = PT_PROPS_ATTRIBUTE_NAME;
//
// if we are reading a file or parsing header and footers
// and we're in a list, append char props to this.
//
	if ( !(bUseInsertNotAppend()) && (bAbiList || bWord97List ))
	{
		buildCharacterProps(propBuffer);
		xxx_UT_DEBUGMSG(("SEVIOR: propBuffer = %s \n",propBuffer.c_str()));
	}
	attribs1[attribsCount++] = propBuffer.c_str();
	attribs1[attribsCount] = NULL;

	if(m_currentRTFState.m_revAttr.size())
	{
		attribs1[attribsCount++] = "revision";
		attribs1[attribsCount++] = m_currentRTFState.m_revAttr.utf8_str();
		attribs1[attribsCount] = NULL;
	}
	
	if (!bUseInsertNotAppend()) // if we are reading a file or parsing header and footers
	{
		if(bAbiList || bWord97List )
		{
			UT_DEBUGMSG(("Append block 1 \n"));
			bool bret = false;
			if(m_pDelayedFrag)
			{
				bret = getDoc()->insertStruxBeforeFrag(m_pDelayedFrag,PTX_Block,attribs1);
			}
			else
			{
				bret = getDoc()->appendStrux(PTX_Block, attribs1);
			}
			m_bEndTableOpen = false;
			m_bCellBlank = false;
			m_newParaFlagged = false;
			m_bSectionHasPara = true;
			getDoc()->appendFmtMark();
			//
			// Insert a list-label field??
			//
			const gchar* fielddef[5];
			fielddef[0] ="type";
			fielddef[1] = "list_label";
			fielddef[2] = NULL;
			fielddef[3] = NULL;
			fielddef[4] = NULL;
			if(bWord97List)
			{
					fielddef[2] = "props";
					fielddef[3] = "text-decoration:none";
			}
			bret =   getDoc()->appendObject(PTO_Field,fielddef);
			UT_UCSChar cTab = UCS_TAB;
//
// Put the tab back in.
//
			if(bWord97List)
			{
					const gchar* attribs[3] = {"props","text-decoration:none",NULL};
					getDoc()->appendFmt(attribs);
			}
			getDoc()->appendSpan(&cTab,1);
			return bret;
		}
		else
		{
			//UT_DEBUGMSG(("SEVIOR: Apply Para's atributes append strux -2 \n"));
			bool ok = false;
			if(m_pDelayedFrag)
			{
				ok = getDoc()->insertStruxBeforeFrag(m_pDelayedFrag,PTX_Block,attribs1);
			}
			else
			{
				ok = getDoc()->appendStrux(PTX_Block, attribs1);
			}
			m_newParaFlagged = false;
			m_bSectionHasPara = true;
			m_bEndTableOpen = false;
			m_bCellBlank = false;
			return ok;
		}
	}
	else
	{
		bool bSuccess = true;
		if(bAbiList || bWord97List )
		{
			if(!bDontInsert)
			{
				UT_DEBUGMSG(("Insert block at 1 \n"));
				markPasteBlock();
				insertStrux(PTX_Block);
			}
			//
			// Put the tab back in.
			//
			UT_UCSChar cTab = UCS_TAB;
			getDoc()->insertSpan(m_dposPaste,&cTab,1);
			m_newParaFlagged = false;
			m_bSectionHasPara = true;
			m_dposPaste++;
			if(m_posSavedDocPosition > 0)
				m_posSavedDocPosition++;
			pf_Frag_Strux* sdh_cur;
			UT_uint32 j;
			fl_AutoNum * pAuto = getDoc()->getListByID(id);
			if(pAuto == NULL)
			/*
			* Got to create a new list here.
			* Old one may have been cut out or ID may have
			* been remapped.
			*/
			{
				FL_ListType lType = NOT_A_LIST;
				fl_AutoLists al;
				UT_uint32 size_xml_lists = al.getXmlListsSize();
				for(j=0; j< size_xml_lists; j++)
				{
					if( strcmp(szStyle.c_str(),al.getXmlList(j)) ==0)
					{
						break;
					}
				}
				if(j < size_xml_lists)
					lType = static_cast<FL_ListType>(j);
				else
					lType = static_cast<FL_ListType>(0);
				pAuto = new fl_AutoNum(id, parentID, lType, startValue,static_cast<gchar *>(m_currentRTFState.m_paraProps.m_pszListDelim),static_cast<gchar *>(m_currentRTFState.m_paraProps.m_pszListDecimal), getDoc(), NULL);
				getDoc()->addList(pAuto);
				pAuto->fixHierarchy();
			}
			bSuccess = getDoc()->getStruxOfTypeFromPosition(m_dposPaste,PTX_Block,&sdh_cur);
			///
			/// Now insert this into the pAuto List
			///
			pAuto->addItem(sdh_cur);
			if(parentID != 0)
			{
				pAuto->findAndSetParentItem();
				pAuto->markAsDirty();
			}
			bSuccess = getDoc()->changeStruxFmt(PTC_SetFmt,m_dposPaste,m_dposPaste,attribs1, NULL,PTX_Block);
		}
		else if(bUseInsertNotAppend())
		{
			ABI_Paste_Table * pPaste = NULL;		
			m_pasteTableStack.viewTop((void**)(&pPaste));
			if(pPaste != NULL)
			{
				if(!pPaste->m_bHasPastedCellStrux && pPaste->m_bHasPastedTableStrux)
				{
//
// We have either a bare table strux or a bare endcell strux. No blocks
// allowed here.
//
					return true;
				}
			}
			UT_DEBUGMSG((" Insert block at 2 \n"));
			if(!bDontInsert)
			{
				markPasteBlock();
				insertStrux(PTX_Block);
			}
			m_newParaFlagged = false;
			m_bSectionHasPara = true;
			bSuccess = getDoc()->changeStruxFmt(PTC_SetFmt,m_dposPaste,m_dposPaste, attribs1,NULL,PTX_Block);
			//
			// Now check if this strux has associated list element. If so stop the list!
			//
			pf_Frag_Strux* sdh = NULL;
			getDoc()->getStruxOfTypeFromPosition(m_dposPaste,PTX_Block,&sdh);
			bool bisListItem = false;
			//
			// Have to loop so that multi-level lists get stopped. Each StopList removes
			// the sdh from the next highest level.
			//
			UT_sint32 iLoop = 20;
			do
			{
				fl_AutoNum * pAuto = NULL;
				bisListItem = false;
				for(UT_uint32 i=0; (i< getDoc()->getListsCount() && !bisListItem); i++)
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
					UT_DEBUGMSG(("SEVIOR: Stopping list at %p \n",sdh));
					getDoc()->StopList(sdh);
				}
				iLoop--;
			}
			while(bisListItem && (iLoop > 0));
		}
		return bSuccess;
	}
	return true;
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
	xxx_UT_DEBUGMSG(("Reset Para Attributes \n"));
	// the \pard keyword always implies we are already in a paragraph
	bool ok = FlushStoredChars();
	m_currentRTFState.m_paraProps = RTFProps_ParaProps();
	m_currentRTFState.m_cellProps = RTFProps_CellProps();
	return ok;
}


bool IE_Imp_RTF::ResetSectionAttributes()
{
	// the \sectd keyword always implies we are in a section
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
	const gchar* pProps = "props";
	std::string propBuffer;
	std::string tempBuffer;
	std::string szHdrID;
	std::string szFtrID;
	std::string szHdrEvenID;
	std::string szFtrEvenID;
	std::string szHdrFirstID;
	std::string szFtrFirstID;
	std::string szHdrLastID;
	std::string szFtrLastID;
	short paramIndex = 0;

	UT_DEBUGMSG (("Applying SectionAttributes\n"));

	// columns
	propBuffer += UT_std_string_sprintf("columns:%d", m_currentRTFState.m_sectionProps.m_numCols);

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
			UT_ASSERT_HARMLESS (UT_SHOULD_NOT_HAPPEN);
		}
	}

	UT_LocaleTransactor t(LC_NUMERIC, "C");
	if(true /*m_currentRTFState.m_sectionProps.m_leftMargTwips != 0*/)
	{
		double inch = static_cast<double>(m_currentRTFState.m_sectionProps.m_leftMargTwips)/1440.;
		propBuffer += UT_std_string_sprintf("; page-margin-left:%fin",inch);
	}
	if(true /*m_currentRTFState.m_sectionProps.m_rightMargTwips != 0*/)
	{
		double inch = static_cast<double>(m_currentRTFState.m_sectionProps.m_rightMargTwips)/1440.;
		propBuffer += UT_std_string_sprintf("; page-margin-right:%fin",inch);
	}
	if(true /*m_currentRTFState.m_sectionProps.m_topMargTwips != 0*/)
	{
		double inch = static_cast<double>(m_currentRTFState.m_sectionProps.m_topMargTwips)/1440.;
		propBuffer += UT_std_string_sprintf("; page-margin-top:%fin",inch);
	}
	if(true /*m_currentRTFState.m_sectionProps.m_bottomMargTwips != 0*/)
	{
		double inch = static_cast<double>(m_currentRTFState.m_sectionProps.m_bottomMargTwips)/1440.;
		propBuffer += UT_std_string_sprintf("; page-margin-bottom:%fin",inch);
	}
	if(true /*m_currentRTFState.m_sectionProps.m_colSpaceTwips != 0*/)
	{
		double inch = static_cast<double>(m_currentRTFState.m_sectionProps.m_colSpaceTwips)/1440.;
		propBuffer += UT_std_string_sprintf("; column-gap:%fin",inch);
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
		double inch = static_cast<double>(sheader)/1440.;
		propBuffer += UT_std_string_sprintf("; page-margin-header:%fin",inch);
	}
#if 0
	if(m_currentRTFState.m_sectionProps.m_gutterTwips != 0)
	{
		double inch = static_cast<double>( m_currentRTFState.m_sectionProps.m_gutterTwips)/1440.;
		propBuffer += UT_std_string_sprintf("; page-margin-footer:%fin",inch);
	}
	UT_DEBUGMSG(("SEVIOR: propBuffer = %s \n",propBuffer.c_str()));
#endif
	if(m_currentRTFState.m_sectionProps.m_footerYTwips != 0)
	{
		double inch = static_cast<double>( m_currentRTFState.m_sectionProps.m_footerYTwips)/1440.;
		propBuffer += UT_std_string_sprintf("; page-margin-footer:%fin",inch);
	}
	UT_DEBUGMSG(("SEVIOR: propBuffer = %s \n",propBuffer.c_str()));
	if(m_currentRTFState.m_sectionProps.m_dir != static_cast<UT_uint32>(UT_BIDI_UNSET))
	{
		const char r[] = "rtl";
		const char l[] = "ltr";
		const char ar[] = "right";
		const char al[] = "left";
		const char * d, * a;
		if(m_currentRTFState.m_sectionProps.m_dir == UT_BIDI_RTL)
		{
			d = r;
			a = ar;
		}
		else
		{
			d = l;
			a = al;
		}

        propBuffer += UT_std_string_sprintf("; dom-dir:%s; text-align:%s",d,a);
        xxx_UT_DEBUGMSG(("Apply sect prop: [%s]\n", tempBuffer.c_str()));
	}

	xxx_UT_DEBUGMSG(("SEVIOR: propBuffer = %s \n",propBuffer.c_str()));

	const gchar* propsArray[15];
	propsArray[0] = pProps;
	propsArray[1] = propBuffer.c_str();
	paramIndex = 2;
	if (m_currentHdrID >= 0)
	{
		UT_DEBUGMSG (("Applying header\n"));
		propsArray [paramIndex] = "header";
		paramIndex++;
		szHdrID = UT_std_string_sprintf ("%u", m_currentHdrID);
		propsArray [paramIndex] = szHdrID.c_str();
		paramIndex++;
	}
	if (m_currentHdrEvenID >= 0)
	{
		UT_DEBUGMSG (("Applying header even\n"));
		propsArray [paramIndex] = "header-even";
		paramIndex++;
		szHdrEvenID = UT_std_string_sprintf ("%u", m_currentHdrEvenID);
		propsArray [paramIndex] = szHdrEvenID.c_str();
		paramIndex++;
	}
	if (m_currentHdrFirstID >= 0)
	{
		UT_DEBUGMSG (("Applying header first\n"));
		propsArray [paramIndex] = "header-first";
		paramIndex++;
		szHdrFirstID = UT_std_string_sprintf ("%u", m_currentHdrFirstID);
		propsArray [paramIndex] = szHdrFirstID.c_str();
		paramIndex++;
	}
	if (m_currentHdrLastID >= 0)
	{
		UT_DEBUGMSG (("Applying header last\n"));
		propsArray [paramIndex] = "header-last";
		paramIndex++;
		szHdrLastID = UT_std_string_sprintf ("%u", m_currentHdrLastID);
		propsArray [paramIndex] = szHdrLastID.c_str();
		paramIndex++;
	}
	if (m_currentFtrID >= 0)
	{
		UT_DEBUGMSG (("Applying footer\n"));
		propsArray [paramIndex] = "footer";
		paramIndex++;
		szFtrID = UT_std_string_sprintf("%u", m_currentFtrID);
		propsArray [paramIndex] = szFtrID.c_str();
		paramIndex++;
	}
	if (m_currentFtrEvenID >= 0)
	{
		UT_DEBUGMSG (("Applying footer even\n"));
		propsArray [paramIndex] = "footer-even";
		paramIndex++;
		szFtrEvenID = UT_std_string_sprintf("%u", m_currentFtrEvenID);
		propsArray [paramIndex] = szFtrEvenID.c_str();
		paramIndex++;
	}
	if (m_currentFtrFirstID >= 0)
	{
		UT_DEBUGMSG (("Applying footer first\n"));
		propsArray [paramIndex] = "footer-first";
		paramIndex++;
		szFtrFirstID = UT_std_string_sprintf ("%u", m_currentFtrFirstID);
		propsArray [paramIndex] = szFtrFirstID.c_str();
		paramIndex++;
	}
	if (m_currentFtrLastID >= 0)
	{
		UT_DEBUGMSG (("Applying footer last\n"));
		propsArray [paramIndex] = "footer-last";
		paramIndex++;
		szFtrLastID = UT_std_string_sprintf ("%u", m_currentFtrLastID);
		propsArray [paramIndex] = szFtrLastID.c_str();
		paramIndex++;
	}
	if(m_currentRTFState.m_revAttr.size())
	{
		propsArray[paramIndex++] = "revision";
		propsArray[paramIndex++] = m_currentRTFState.m_revAttr.utf8_str();
	}

	UT_ASSERT_HARMLESS (paramIndex < 15);
	propsArray [paramIndex] = NULL;

	if (!bUseInsertNotAppend()) // if we are reading a file or parsing a header and footer
	{
		UT_DEBUGMSG(("Appending Section strux now \n"));
		return getDoc()->appendStrux(PTX_Section, propsArray);
	}
	else
	{
		// Add a block before the section so there's something content
		// can be inserted into.
		UT_DEBUGMSG(("Insert block at 3 \n"));
		markPasteBlock();
		bool bSuccess = insertStrux(PTX_Block);

		if (bSuccess)
		{
			m_dposPaste--;
			if(m_posSavedDocPosition > 0)
				m_posSavedDocPosition--;
			XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
			if(pFrame == NULL)
			{
				return false;
			}
			FV_View * pView = static_cast<FV_View*>(pFrame->getCurrentView());
			if(pView == NULL)
			{
				return false;
			}
			if(!pView->isInDocSection(m_dposPaste))
			{
				return false;
			}
			bSuccess = insertStrux(PTX_Section);
			if (bSuccess)
			{
				bSuccess = getDoc()->changeStruxFmt(PTC_SetFmt,m_dposPaste,m_dposPaste,
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
	while(nesting > 0 && count < MAX_KEYWORD_LEN - 1)
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
	UT_std_vector_purgeall(m_vecWord97Lists);
	unsigned char keyword[MAX_KEYWORD_LEN];
	unsigned char ch;
	UT_sint32 parameter = 0;
	bool paramUsed = false;
	UT_uint32 nesting = 1;
	xxx_UT_DEBUGMSG(("Doing Read List Table \n"));
	while (nesting >0) // Outer loop
	{
		xxx_UT_DEBUGMSG(("Nesting %d \n",nesting));
		if (!ReadCharFromFile(&ch))
		{
			return false;
		}
		if(ch == '{')  //new list or listoverride?
		{
			nesting++;
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

// HandleTableList eats the last "}"

				nesting--;
			}
			else
			{
				UT_DEBUGMSG(("Unexpected keyword in listable %s Here \n",keyword));
			}
		}
		else if(ch == '}')
		{
			nesting--;
		}
	}
	// Reclaim group }
	if (ch=='}') SkipBackChar(ch);
	xxx_UT_DEBUGMSG(("Return from List Table \n"));
	return true;
}

/*!
 * This method parses out the \list item in a list tabledefinition.
 */
bool IE_Imp_RTF::HandleTableList(void)
{
	unsigned char keyword[MAX_KEYWORD_LEN];
	unsigned char ch;
    UT_sint32 parameter = 0;
	bool paramUsed = false;
	UT_uint32 nesting = 1;
	UT_uint32 levelCount = 0;
//
// Increment list counting vector
//
	RTF_msword97_list * pList = new  RTF_msword97_list(this);
	m_vecWord97Lists.push_back(pList);
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
	UT_sint32 parameter = 0;
	bool paramUsed = false;
	UT_uint32 nesting = 1;
	std::string szLevelNumbers;
	std::string szLevelText;
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
	delete pList->m_RTF_level[levelCount];
	pList->m_RTF_level[levelCount] = pLevel;
#if 0 // Sevior use this!! The other method can lead to inccorect results upon
	// import. If we export RTF list ID starting at 10000 they might clash
    // with these later.
	pLevel->m_AbiLevelID = UT_rand();
	while(pLevel->m_AbiLevelID < 10000)
		pLevel->m_AbiLevelID = UT_rand();
#else
	//pLevel->m_AbiLevelID = pLevel->m_sLastAssignedLevelID++;
	pLevel->m_AbiLevelID = getDoc()->getUID(UT_UniqueId::List);
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
 * OK this method parses the RTF against all the character and 
 * paragraph properties.
 * and fills the pointers to the character and paragraph classes.
 * These are used by the list table and stylesheet reader.
 */
bool IE_Imp_RTF::ParseCharParaProps( unsigned char * pKeyword, 
                                     UT_sint32 param, bool fParam, 
                                     RTFProps_CharProps * pChars, 
                                     RTFProps_ParaProps * pParas, 
                                     RTFProps_bCharProps * pbChars, 
                                     RTFProps_bParaProps * pbParas)
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
	else if (strcmp(reinterpret_cast<char*>(pKeyword), "lang") == 0)
	{
		pChars->m_szLang = wvLIDToLangConverter(static_cast<unsigned short>(param));
		return true;
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
		pParas->m_lineSpaceExact = (!fParam  ||  param == 0);   // this means exact or "at least" - which depends on sign of \sl param
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
	UT_std_vector_purgeall(m_vecWord97ListOverride);
	unsigned char keyword[MAX_KEYWORD_LEN];
	unsigned char ch;
	UT_sint32 parameter = 0;
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
	if (ch=='}') SkipBackChar(ch);
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
		pLOver = m_vecWord97ListOverride.at(i);
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
	UT_sint32 parameter = 0;
	bool paramUsed = false;
//
// OK define this in the data structure.
//
	RTF_msword97_listOverride * pLOver = new  RTF_msword97_listOverride(this);
//
// Increment override counting vector
//
	m_vecWord97ListOverride.push_back(pLOver);
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
				xxx_UT_DEBUGMSG(("SEVIOR: Found list listoverride count. Ignore for now\n"));
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

/**
 * Reads back data that was written with IE_Exp_RTF::s_escapeXMLString()
 */
std::string
IE_Imp_RTF::s_unEscapeXMLString()
{
	std::stringstream ss;
	unsigned char ch = 0;
	// Since the star keyword group is between brackets, a new RTF state was generated. Remove it now.
	// The closing bracket will be ignored.
	PopRTFState();
	while(ReadCharFromFile(&ch) && ch != '}')
	{
		ss << ch;
	}

	std::string s = ss.str();

	// We want &7d;&7d; -> &7d;
	// And         &7d; -> }
	// as we know there are no occurances of }} in the string
	// we use that as a temporary state to hold the case of two 7d in a row.
	s = replace_all( s, "&7d;&7d;", "}}" );
	s = replace_all( s, "&7d;", "}" );
	s = replace_all( s, "}}", "&7d;" );
	
	return s;
}


// rdf triples are an rdf/xml file
// {\*\rdf RDF/XML}
bool IE_Imp_RTF::ReadRDFTriples()
{
	std::string rdfxml = s_unEscapeXMLString();
	PD_DocumentRDFHandle rdf = getDoc()->getDocumentRDF();
	UT_DEBUGMSG(("rdf triples before read of rdf tag size:%ld\n", (long)rdf->size() ));

	PD_DocumentRDFMutationHandle m = rdf->createMutation();
	/*UT_Error e = */loadRDFXML( m, rdfxml );
	m->commit();
	UT_DEBUGMSG(("rdf triples after read of rdf tag size:%ld\n", (long)rdf->size() ));
	return true;
}


// the revision table looks
// \*\revtb{{Author1;}{Author2;} ... }
bool IE_Imp_RTF::ReadRevisionTable()
{
	unsigned char ch = 0;
	UT_UCS4String s;
	UT_sint32 i = 1;

	while(ReadCharFromFile(&ch) && ch != '}')
	{
		while(ch != '{' && ReadCharFromFile(&ch)){;}
	
		if(ch != '{')
			return false;
		
		s.clear();
		
		while(ReadCharFromFile(&ch) && ch != ';')
		{
			s += ch;
		}
		// the semicolon should be followed by a closing brace
		ReadCharFromFile(&ch);
		UT_return_val_if_fail(ch == '}', false);

		// now add the revision to the document
		// the rtf doc stores the time stamp for each individual rev. operation rather
		// than for the revision set; we will set it when we encounter the first revision

		// the first entry is typically author Unknown; we will ignore it
		// hack around non stricmp
		UT_UCS4Char u1[] = {'U','n','k','n','o','w','n',0};
		UT_UCS4Char u2[] = {'u','n','k','n','o','w','n',0};
		
		if(i == 1 && (!UT_UCS4_strcmp(s.ucs4_str(), u1) || !UT_UCS4_strcmp(s.ucs4_str(), u2)))
			continue;
		
		getDoc()->addRevision(i,s.ucs4_str(),s.length(),0,0);
		++i;
	}

	UT_return_val_if_fail( ch == '}', false );
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// Font table reader
//////////////////////////////////////////////////////////////////////////////

/* 
 * Reads the RTF font table, storing it for future use
 *
 * This function is very tolerant. It can read entries such as:
 *
 * Eg:
 *     {\f18\fnil\fcharset134\fprq2{\*\panose 02010600030101010101}
 *     \'cb\'ce\'cc\'e5{\*\falt SimSun};}
 *
 * or even:
 *	   {\f20\froman Times New {\*\unknowncommand Fibble!}Roman;}
 *
 * It reads in alternative font names for Asian fonts (as specified
 * with the \falt keyword, panose numbers and supports \uXXXXX sequences
 * and \'XX hex escaping in the font names.
 * 
 */

/* The state used while reading the font table.
 * iFontNum is the index of the font name that we are currently writing.
 * Initially it's set to FontName. We switch it to point to AltFontName when we
 * see a \falt command. When the group containing the \falt ends we pop the
 * state of the stack and so this index reverts back to FontName.
 */
struct SFontTableState {
	enum DataType { MainFontName=0, AltFontName=1, Panose=2};
	enum DataType iCurrentInputData;// Are we reading the main name, the alt name or panose?
	UT_uint32 iUniSkipCount;        // How many characters should we skip after a /uXXXXX sequence?
	UT_uint32 iUniCharsLeftToSkip;  // How many remaining skippable ANSI chars are there?
	bool bSeenStar;                 // Have we seen a "\*" in this group?
};

bool IE_Imp_RTF::ReadFontTable()
{
	/* Declare variables for the information to be read from each entry */
	RTFFontTableItem::FontFamilyEnum fontFamily = RTFFontTableItem::ffNone;
	RTFFontTableItem::FontPitch pitch = RTFFontTableItem::fpDefault;
	UT_uint16 fontIndex = 0;
	int charSet = -1;                    // -1 indicates "none defined".
	int codepage = 0;
	UT_UTF8String sFontNamesAndPanose[3];// The font names and panose data in UTF-8.
	UT_ByteBuf RawDataBuf[3];           // The Font names and panose data in orig. enc. 
	/* Variables needed to process the entry. */
	bool bGotFontIndex = false;          // Did the entry specify a font index?
	bool bSeenNonWhiteSpaceData = false; // Have we seen non-ws data in the current entry
	bool bFoundFinalClosingBracket;      // Have we seen the bracket which closes the font table?
	unsigned char keyword[MAX_KEYWORD_LEN];
	RTFTokenType tokenType;
	RTF_KEYWORD_ID keywordID;
	UT_sint32 parameter = 0;
	bool paramUsed = false;
	UT_Byte ch;
	UT_Stack stateStack;
	// RTF state pointers.
	struct SFontTableState *currentState = new SFontTableState;
	UT_DEBUGMSG(("Made new currentState -1 %p \n",currentState));
	struct SFontTableState *oldState = NULL;
	UT_sint32 i;                         // Generic loop index.

	// Initialise the current state.
	currentState->iCurrentInputData = SFontTableState::MainFontName; 
	currentState->iUniSkipCount = m_currentRTFState.m_unicodeAlternateSkipCount;
	currentState->iUniCharsLeftToSkip = 0;
	currentState->bSeenStar = false;
	
	bFoundFinalClosingBracket = false;
	while (!bFoundFinalClosingBracket)
	{
		// NB: Ignores whitespace until we've seen non-whitespace data.
		//     This means we pick up the spaces in font names like
		//     "Times New Roman", but it also means that any font names
		//     that genuinely start with spaces will have them discarded.
		//     This is hopefully not a problem.
		tokenType = NextToken(keyword, &parameter,& paramUsed,
	 	                      MAX_KEYWORD_LEN, !bSeenNonWhiteSpaceData);
		switch (tokenType)
		{
		case RTF_TOKEN_OPEN_BRACE:
			// An open brace can prematurely terminate ANSI data after a \uXXXXX
			// sequence. Thus, we reset iUniCharsLeftToSkip here.
			currentState->iUniCharsLeftToSkip = 0;
			// Keep a pointer to the current state.
			oldState = currentState;
			// Push the current state onto the stack...
			stateStack.push(reinterpret_cast<void*>(currentState));
			// ...allocate a new one...
			currentState = new SFontTableState;
			UT_DEBUGMSG(("Made new currentState -2 %p \n",currentState));
			if (!currentState) {
				UT_DEBUGMSG(("RTF: Out of memory.\n"));
				goto IEImpRTF_ReadFontTable_ErrorExit;
			}
			// ...and initialise it as a copy of the old one.
			currentState->iCurrentInputData = oldState->iCurrentInputData;
			currentState->iUniSkipCount = oldState->iUniSkipCount;
			currentState->iUniCharsLeftToSkip = oldState->iUniCharsLeftToSkip;
			currentState->bSeenStar = oldState->bSeenStar;
			break;
		case RTF_TOKEN_CLOSE_BRACE:
			// Throw away the current state.
			UT_DEBUGMSG(("Deleting currentState -4 %p \n",currentState));
			DELETEP(currentState);
			// Pop an old state off the stack .
			if (!stateStack.pop(reinterpret_cast<void**>(&currentState))) 
			{
				// If there's no state on the stack then this must be the
				// bracket that ends the font table.
				bFoundFinalClosingBracket = true;
				// Put the closing brace back onto the input stream.
				SkipBackChar('}');
				currentState = NULL;
			}
			break;
		case RTF_TOKEN_DATA:
			// Are we skipping ANSI data after a \uXXXXX?
			if (currentState->iUniCharsLeftToSkip)
			{
				currentState->iUniCharsLeftToSkip--;
				break;
			}
			// We found the font name terminator.
			if (keyword[0] == ';')
			{
				// Check that at the very least we got a font index.
				if (!bGotFontIndex) {
					UT_DEBUGMSG(("RTF: Font table didn't specify a font index.\n"));
					goto IEImpRTF_ReadFontTable_ErrorExit;
				}
				// Flush any data in the buffers to the font name and panose
				// strings, converting to UTF8.
				for (i=SFontTableState::MainFontName; i<=SFontTableState::Panose; i++) 
				{
					sFontNamesAndPanose[i].appendBuf(RawDataBuf[i], m_mbtowc);
					RawDataBuf[i].truncate(0);
				}
				// It's possible that the font name will be empty. This might happend 
				// because the font table didn't specify a name, or because the \ansicpgN
				// command was invalid, in which case the mbtowc convertion might fail.
				// In these cases, substitute "Times New Roman".
				if (!sFontNamesAndPanose[SFontTableState::MainFontName].length())
				{
					UT_DEBUGMSG(("RTF: Font Index %d: Substituting \"Times New Roman\" for missing font name.\n", fontIndex));
					sFontNamesAndPanose[SFontTableState::MainFontName] = "Times New Roman";
				}
				// Validate and post-process the Panose string.
				if (!PostProcessAndValidatePanose(sFontNamesAndPanose[SFontTableState::Panose])) 
				{
					// If the panose string was invalid, then clear it.
					// I don't think it's worth refusing to load the file just because
					// the panose string is wrong.
					UT_DEBUGMSG(("RTF: Panose string for font with index %d invalid, ignoring.\n", fontIndex));
					sFontNamesAndPanose[SFontTableState::Panose] = "";
				}
				// Register the font.
				if (!RegisterFont(fontFamily, pitch, fontIndex, charSet, 
				                  codepage, sFontNamesAndPanose)         ) 
				{
					goto IEImpRTF_ReadFontTable_ErrorExit;
				}
				// Reset both font names/panose.
				for (i=SFontTableState::MainFontName; i<=SFontTableState::Panose; i++)
					sFontNamesAndPanose[i] = "";
				bGotFontIndex = false;
				bSeenNonWhiteSpaceData = false;
			}
			else 
			{
				// Other data must be one of the font names, so write it to the
				// current font name pointer.
				RawDataBuf[currentState->iCurrentInputData].append(keyword, 1);	
				bSeenNonWhiteSpaceData = true;
			}
			break;
		case RTF_TOKEN_KEYWORD:
			keywordID = KeywordToID(reinterpret_cast<char *>(keyword));
			// Are we skipping ANSI data after a \uXXXXX?
			if (currentState->iUniCharsLeftToSkip)
			{
				currentState->iUniCharsLeftToSkip--;
				break;
			}

			switch(keywordID) 
			{
			// Handle all the face names.
			case RTF_KW_fnil:
				fontFamily = RTFFontTableItem::ffNone;
				break;
			case RTF_KW_froman:
				fontFamily = RTFFontTableItem::ffRoman;
				break;
			case RTF_KW_fswiss:
				fontFamily = RTFFontTableItem::ffSwiss;
				break;
			case RTF_KW_fmodern:
				fontFamily = RTFFontTableItem::ffModern;
				break;
			case RTF_KW_fscript:
				fontFamily = RTFFontTableItem::ffScript;
				break;
			case RTF_KW_fdecor:
				fontFamily = RTFFontTableItem::ffDecorative;
				break;
			case RTF_KW_ftech:
				fontFamily = RTFFontTableItem::ffTechnical;
				break;
			case RTF_KW_fbidi:
				fontFamily = RTFFontTableItem::ffBiDirectional;
				break;

			// Handle hex escaped data.
			case RTF_KW_QUOTE:
				ch = static_cast<UT_Byte>(ReadHexChar());
				RawDataBuf[currentState->iCurrentInputData].append(&ch, 1);	
				break;
			// Handle the "*" keyword.
			case RTF_KW_STAR:
				currentState->bSeenStar = true;
				break;
			// Handle the "\f", font index, keyword.
			case RTF_KW_f:
				// If this is a duplicate font index then it is highly likely
				// that the font table is corrupt. For example, a missing
				// semi-colon causes this.
				if (bGotFontIndex) 
				{
					UT_DEBUGMSG(("RTF: Invalid duplicate font index in font table.\n"));
					goto IEImpRTF_ReadFontTable_ErrorExit;
				}
				bGotFontIndex = true;
				fontIndex = parameter;
				break;
			// Handle the "\fcharset", character set, keyword.
			case RTF_KW_fcharset:
				charSet = parameter;
				break;
			// Handle "\falt" keyword.
			case RTF_KW_falt:
				// Change the input data index so that data will be written to
				// the alternative fontname.
				currentState->iCurrentInputData = SFontTableState::AltFontName;
				break;
			// Handle panose numbers.
			case RTF_KW_panose:
				// Change the input data index so that data will be written to
				// the panose string.
				currentState->iCurrentInputData = SFontTableState::Panose;
				break;
			// Handle \uXXXXX escaped data.
			case RTF_KW_u:
			{
				/* RTF is limited to +/-32K ints so we need to use negative
				 * numbers for large unicode values. So, check for Unicode chars
				 * wrapped to negative values.
				 */
				if (parameter < 0)
				{
					unsigned short tmp = (unsigned short) ((signed short) parameter);
					parameter = (UT_sint32) tmp;
				}
				// First flush any data in the buffer to the font name
				// string, converting to UTF8.
				sFontNamesAndPanose[currentState->iCurrentInputData].appendBuf(RawDataBuf[currentState->iCurrentInputData], m_mbtowc);
				RawDataBuf[currentState->iCurrentInputData].truncate(0);
				// Then append the UCS2 char.
				// TODO Since we process one character at a time, this code 
				// will not handle surrogate pairs.
				sFontNamesAndPanose[currentState->iCurrentInputData].appendUCS2(reinterpret_cast<UT_UCS2Char *>(&parameter), 1);

				// Set the reader to skip the appropriate number of ANSI
				// characters after the \uXXXXX command.
				currentState->iUniCharsLeftToSkip = currentState->iUniSkipCount;
				break;  // Break out after handling keyword.
			}
			case RTF_KW_uc:
				currentState->iUniSkipCount = parameter;
				break;
			// Handle unknown keywords.
			default:
				// If this group contained a \* command then we should skip to
				// the end of this group. Otherwise, we just ignore this unknown
				// command.
				if (currentState->bSeenStar) 
				{
					if (!SkipCurrentGroup(/*Consume last brace =*/false))
						goto IEImpRTF_ReadFontTable_ErrorExit;
				}
				break;
			} // Keyword switch
			break; // End of tokenType == RTF_TOKEN_KEYWORD case statement.
		case RTF_TOKEN_NONE:
			UT_DEBUGMSG(("RTF: Premature end of file reading font table.\n"));
			goto IEImpRTF_ReadFontTable_ErrorExit;
		case RTF_TOKEN_ERROR:
			UT_DEBUGMSG(("RTF: Error reading token from file.\n"));
			goto IEImpRTF_ReadFontTable_ErrorExit;
		default:
			break;
		} // Token type switch
	}; // while (we've finished reading the font entry).
	UT_DEBUGMSG(("Deleting currentState -2 %p \n",currentState));
	DELETEP(currentState);
	return true;

/*
  Gotos are evil. However, so are memory leaks and code duplication. Exceptions
  might be a neater solution, but apparently they're not portable.
*/
IEImpRTF_ReadFontTable_ErrorExit:
	UT_DEBUGMSG(("RTF: ReadFontTable: Freeing memory due to error.\n"));
	// Delete the current state and everything on the state stack.
	UT_DEBUGMSG(("Deleting currentState -2 %p \n",currentState));
	DELETEP(currentState);
	while (stateStack.pop(reinterpret_cast<void**>(&currentState))) 
	{
		UT_DEBUGMSG(("Deleting currentState -3  %p \n",currentState));
		DELETEP(currentState);
	}
	return false;
}


/*
 * This function creates a new RTFFontTableItem and adds it to the document
 * font table.
 */
bool IE_Imp_RTF::RegisterFont(RTFFontTableItem::FontFamilyEnum fontFamily,
                              RTFFontTableItem::FontPitch pitch,
                              UT_uint16 fontIndex,
                              int charSet, int codepage,
                              UT_UTF8String sFontNamesAndPanose[]) {
#ifndef TOOLKIT_COCOA
	/*work around "helvetica" font name -replace it with "Helvetic"*/
	if (sFontNamesAndPanose[SFontTableState::MainFontName] == "helvetica")
	{
		sFontNamesAndPanose[SFontTableState::MainFontName] = "Helvetic";
	}
#endif /* ! TOOLKIT_COCOA */


	// Create the font entry and put it into the font table
	// NB: If the font table didn't specify a font name then we want to pass
	//     NULLs to RTFFontTableItem() rather than zero length strings.
	RTFFontTableItem* pNewFont = new RTFFontTableItem(
							fontFamily, charSet, codepage, pitch, 
							sFontNamesAndPanose[SFontTableState::Panose].length() ?
							  sFontNamesAndPanose[SFontTableState::Panose].utf8_str() : NULL,
							sFontNamesAndPanose[SFontTableState::MainFontName].length() ?
							  sFontNamesAndPanose[SFontTableState::MainFontName].utf8_str() : NULL,
							sFontNamesAndPanose[SFontTableState::AltFontName].length() ? 
							  sFontNamesAndPanose[SFontTableState::AltFontName].utf8_str() : NULL);
	if (pNewFont == NULL)
	{
		return false;
	}

	// ensure that the font table is large enough for this index
	while (m_fontTable.size() <= fontIndex)
	{
		m_fontTable.push_back(NULL);
	}
	RTFFontTableItem* pOld = NULL;
	// some RTF files define the fonts several time. This is INVALID according to the
	// specifications. So we ignore it.

	// Ugly hack for MSVC and GCC comlilant warnings
	#ifdef __GNUC__
		#warning(maybe not the right behaviour)
	#else
		#pragma message("WARNING: maybe not the right behaviour" __FILE__)
	#endif
	if (m_fontTable[fontIndex] == NULL)
	{
		pOld = m_fontTable[fontIndex];
		m_fontTable[fontIndex] = pNewFont;
		UT_return_val_if_fail(pOld == NULL, false);
	}
	else
	{
		UT_DEBUGMSG (("RTF: font %d (named %s) already defined. Ignoring\n",
		                 fontIndex,
		                 sFontNamesAndPanose[SFontTableState::MainFontName].utf8_str()));
		DELETEP (pNewFont);
	}

	return true;
}

/*
 * This function validates a panose string and does any pose-processing of
 * this string that might be necessary. It's called from ReadFontTable().
 *
 * Note: I don't know anything much about Panose numbers. The implementation
 * of ReadFontTable() that this commit replaces (July 2005) read in every other
 * byte of the 20 character Panose string to yield a final 10 byte string.
 * Therefore, this function does the same. In addition, it checks that all
 * characters in the string are digits (0-9) and nothing else.
 *
 * This function returns false on error. 
 */
bool IE_Imp_RTF::PostProcessAndValidatePanose(UT_UTF8String &Panose) 
{
	UT_UTF8Stringbuf::UTF8Iterator iter = Panose.getIterator ();
	UT_UTF8String sProcessedPanose;
	UT_sint32 i;

	// If the panose string is not empty.
	iter = iter.start(); // Set the iterator to the first character.
	// There should be 20 characters
	for (i=0; i<20; i++, ++iter)
	{
		const char * pUTF = iter.current ();
		// If we run out of data then the panose string is too short.
		if (!pUTF || !*pUTF)
		{
			// An empty string is valid, so return true if this is the 
			// first character.
			if (i==0)
				return true;
			UT_DEBUGMSG(("RTF: Panose string too short.\n"));
			return false;
		}
		// Only hex digits are allowed so we bail if we see anything
		// else.
		if (!isxdigit(*pUTF))
		{
			UT_DEBUGMSG(("RTF: Invalid character in panose string.\n"));
			return false;
		}
		// Store alternate characters in the output string.
		if ((i%2)==1)
		{
			// This wouldn't work for genuine UTF-8, since it's a variable
			// byte encoding. However, we've already check that the string
			// only contains the characters 0-9, so it's effectively ASCII.
			sProcessedPanose += *pUTF;
		}
	}
	// Replace the original panose string with the new one.
	Panose = sProcessedPanose;
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// Colour table reader
//////////////////////////////////////////////////////////////////////////////

bool IE_Imp_RTF::ReadColourTable()
{
	// Ensure the table is empty before we start
	UT_return_val_if_fail(m_colourTable.empty(), false);

	unsigned char keyword[MAX_KEYWORD_LEN];
	unsigned char ch;
	UT_sint32 parameter = 0;
	bool paramUsed = false;
	if (!ReadCharFromFile(&ch))
		return false;
	bool bValidColor = false;
	while (ch != '}')
	{
		UT_uint32 colour = 0;
		bool tableError = false;
		while(ch == ' ')
		{
			if (!ReadCharFromFile(&ch))
				return false;
		}
		bValidColor = false;
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
				hasRed = false;
				hasGreen = false;
				hasBlue = false;
				
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
				colour = static_cast<UT_uint32>(red << 16) | static_cast<UT_uint32>(green << 8) | static_cast<UT_uint32>(blue);
				UT_DEBUGMSG(("colour %d red %ld green %ld blue %ld \n",colour,red,green,blue)); 
				bValidColor = true;
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
		else if(ch!= '}' || bValidColor)
		{
			UT_DEBUGMSG(("Add colour %d to table \n",colour));
			m_colourTable.push_back(colour);

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
	UT_sint32 parameter = 0;
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
				/* 
				   dest indicate the destination 
				   0 = none (ignore)
				   1 = after
				   2 = before
				   Any other value has no legal meaning.
				*/
				int dest = 0; 
				if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pntxta") == 0) {
					dest = 1;
				}
				else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "pntxtb") == 0) {
					dest = 2;
				} 
				else {
					UT_DEBUGMSG(("Unknown keyword %s found in List stream  \n",keyword));
				}
				if (dest != 0) {
					// OK scan through the text until a closing delimeter is
					// found
					int level = 0;
					int count = 0;
					if (!ReadCharFromFile(&ch))
						return false;
					while ((level != 0 || (ch != '}' && ch != ';')) && count < MAX_KEYWORD_LEN - 1)
					{
						if (ch == '{') {
							level++;
						}
						else if (ch == '}') {
							UT_ASSERT_HARMLESS(level);
							level--;
						}
						else {
							keyword[count++] = ch;
						}
						if (!ReadCharFromFile(&ch))
							return false;
					}
					keyword[count++] = 0;
					switch (dest) {
					case 1:
						strncpy(rtfTable.textafter,reinterpret_cast<char*>(&keyword[0]), sizeof(rtfTable.textafter));
						rtfTable.textafter[sizeof(rtfTable.textafter) - 1] = 0;
						UT_DEBUGMSG(("FOUND pntxta in stream, copied %s to input  \n",keyword));
						break;
					case 2:
						strncpy(rtfTable.textbefore,reinterpret_cast<char*>(&keyword[0]), sizeof(rtfTable.textbefore));
						rtfTable.textbefore[sizeof(rtfTable.textbefore) - 1] = 0;
						UT_DEBUGMSG(("FOUND pntxtb in stream,copied %s to input  \n",keyword));
						break;
					default:
						UT_ASSERT_NOT_REACHED();
					}
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
				UT_DEBUGMSG(("FOUND ls in stream - override number %d\n",parameter));
				rtfTable.iWord97Override =  static_cast<UT_uint32>(parameter);
				// Word 97 list table identifier
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "ilvl") == 0)
			{
				UT_DEBUGMSG(("FOUND ilvl in stream - levelnumber %d\n",parameter));
				rtfTable.iWord97Level = static_cast<UT_uint32>(_sanitizeListLevel(parameter));
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

/////////////////////////////////////////////////////////////////////////
// Handle copy/paste of MathML by extending RTF
/////////////////////////////////////////////////////////////////////////
bool IE_Imp_RTF::HandleAbiMathml(void)
{
	std::string sProps;
	unsigned char ch;
	if (!ReadCharFromFile(&ch))
		return false;
	while(ch == ' ')
	{
		if (!ReadCharFromFile(&ch))
			return false;
	}
	// Since the star keyword group is between brackets, a new RTF state was generated. Remove it now.
	// The closing bracket will be ignored.
	PopRTFState();
	while (ch != '}') // Outer loop
	{
		sProps += ch;
		if (!ReadCharFromFile(&ch))
			return false;
	}

	std::string sPropName;
	std::string sInputAbiProps;
	const gchar * attrs[7] = {"dataid",NULL,NULL,NULL,NULL,NULL,NULL};
	sPropName = "dataid";
	std::string sDataIDVal = UT_std_string_getPropVal(sProps,sPropName);
	attrs[1] = sDataIDVal.c_str();
	UT_std_string_removeProperty(sProps,sPropName);
	sPropName ="latexid";
	std::string sLatexIDVal = UT_std_string_getPropVal(sProps,sPropName);
	if(sLatexIDVal.size() > 0)
	{
		UT_std_string_removeProperty(sProps,sPropName);
		attrs[2] = "latexid";
		attrs[3] =  sLatexIDVal.c_str();
		attrs[4]= "props";
		attrs[5] = sProps.c_str();
	}
	else
	{
		attrs[2] = "props";
		attrs[3] = sProps.c_str();
	}
	getDoc()->getUID(UT_UniqueId::Image); // Increment the image uid counter
	//
	// OK put in all the complex handling we need. Code taken from insert field
	//
	bool ok = FlushStoredChars(true);
	UT_return_val_if_fail (ok, false);
	if (!bUseInsertNotAppend() || m_bAppendAnyway)
	{
		UT_DEBUGMSG(("SEVIOR: Appending Math Object m_bCellBlank %d m_bEndTableOpen %d \n",m_bCellBlank,m_bEndTableOpen));
		if(m_bCellBlank || m_bEndTableOpen)
		{
			UT_DEBUGMSG(("Append block 5 \n"));
			if(m_pDelayedFrag)
			{
				getDoc()->insertStruxBeforeFrag(m_pDelayedFrag,PTX_Block,NULL);
			}
			else
			{
				getDoc()->appendStrux(PTX_Block,NULL);
			}
			m_bCellBlank = false;
			m_bEndTableOpen = false;
		}
		if(m_pDelayedFrag)
	    {
			getDoc()->insertObjectBeforeFrag(m_pDelayedFrag,PTO_Math, attrs);
		}
		else
		{
			getDoc()->appendObject(PTO_Math, attrs);
		}
	}
	else
	{
		XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
		if(pFrame == NULL)
		{
			 m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
			 return true;
		}
		FV_View * pView = static_cast<FV_View*>(pFrame->getCurrentView());
		if(pView == NULL)
		{
			m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
			return true;
		}
		getDoc()->insertObject(m_dposPaste, PTO_Math, attrs, NULL);
		m_dposPaste++;
		if(m_posSavedDocPosition > 0)
			m_posSavedDocPosition++;
	}
	return true;
}

bool IE_Imp_RTF::CreateDataItemfromStream(void)
{
	UT_UTF8String sName;
	unsigned char ch;
	if (!ReadCharFromFile(&ch))
		return false;
	//
	// Skip leading spaces
	//
	while(ch == ' ')
	{
		if (!ReadCharFromFile(&ch))
			return false;
	}
	while (ch != ' ') // extract name of name item
	{
		sName += ch;
		if (!ReadCharFromFile(&ch))
			return false;
	}
	//
	// skip trailing spaces
	//
	while(ch == ' ')
	{
		if (!ReadCharFromFile(&ch))
			return false;
	}
	//
	// read the mime type if any
	//
	std::string mime;
	if (ch == 'm')
	{
		while(ch != ' ' && ch != ':')
		{
			mime += ch;
			if (!ReadCharFromFile(&ch))
				return false;
		}
		if (mime != "mime-type")
			return false;
		if (!ReadCharFromFile(&ch))
			return false;
		mime = "";
		while(ch != ' ')
		{
			mime += ch;
			if (!ReadCharFromFile(&ch))
				return false;
		}
		//
		// skip trailing spaces
		//
		while(ch == ' ')
		{
			if (!ReadCharFromFile(&ch))
				return false;
		}
	}
	//
	// We're at the start of the data item. 
	//

	const UT_uint16 chars_per_byte = 2;
	const UT_uint16 BITS_PER_BYTE = 8;
	const UT_uint16 bits_per_char = BITS_PER_BYTE / chars_per_byte;
	bool retval = true;
	bool bFound = true;

	UT_ByteBuf BinData;
	UT_uint16 chLeft = chars_per_byte;
	UT_Byte bin_byte = 0;
	const UT_ByteBuf * pDum = NULL;
	while (ch != '}')
	{
		int digit;
		if (!hexVal(ch, digit)) 
		{
			return false;
		}
			
		bin_byte = (bin_byte << bits_per_char) + digit;
			
		// if we have a complete byte, we put it in the buffer
		if (--chLeft == 0)
		{
			BinData.append(&bin_byte, 1);
			chLeft = chars_per_byte;
			bin_byte = 0;
		}
			
		if (!ReadCharFromFile(&ch)) 
		{
			return false;
		}
	}
	// Put the '}' back into the input stream
	SkipBackChar(ch);
	
	bFound = getDoc()->getDataItemDataByName(sName.utf8_str(),&pDum, NULL, NULL);
	if(bFound)
	{
		return true;
	}
	//
	// Now create the data item from the RTF data stream
	//
	
	retval = getDoc()->createDataItem(sName.utf8_str(),false,&BinData,mime,NULL);
	return retval;

}

/////////////////////////////////////////////////////////////////////////
// Handle copy/paste of Embedded Objects by extending RTF
/////////////////////////////////////////////////////////////////////////
bool IE_Imp_RTF::HandleAbiEmbed(void)
{
	UT_UTF8String sProps;
	unsigned char ch;
	if (!ReadCharFromFile(&ch))
		return false;
	while(ch == ' ')
	{
		if (!ReadCharFromFile(&ch))
			return false;
	}
	// Since the star keyword group is between brackets, a new RTF state was generated. Remove it now.
	// The closing bracket will be ignored.
	PopRTFState();
	while (ch != '}') // Outer loop
	{
		sProps += ch;
		if (!ReadCharFromFile(&ch))
			return false;
	}

	UT_UTF8String sPropName;
	UT_UTF8String sInputAbiProps;
	const gchar * attrs[7] = {"dataid",NULL,NULL,NULL,NULL};
	sPropName = "dataid";
	UT_UTF8String sDataIDVal = UT_UTF8String_getPropVal(sProps,sPropName);
	attrs[1] = sDataIDVal.utf8_str();
	UT_UTF8String_removeProperty(sProps,sPropName);
	attrs[2]= "props";
	attrs[3] = sProps.utf8_str();
	bool ok = FlushStoredChars(true);
	UT_return_val_if_fail (ok, false);
	if (!bUseInsertNotAppend() || m_bAppendAnyway)
	{
		UT_DEBUGMSG(("SEVIOR: Appending Embedded Object m_bCellBlank %d m_bEndTableOpen %d \n",m_bCellBlank,m_bEndTableOpen));
		if(m_bCellBlank || m_bEndTableOpen)
		{
			UT_DEBUGMSG(("Append block 5 \n"));
			if(m_pDelayedFrag)
			{
				getDoc()->insertStruxBeforeFrag(m_pDelayedFrag,PTX_Block,NULL);
			}
			else
			{
				getDoc()->appendStrux(PTX_Block,NULL);
			}
			m_bCellBlank = false;
			m_bEndTableOpen = false;
		}
		if(m_pDelayedFrag)
	    {
			getDoc()->insertObjectBeforeFrag(m_pDelayedFrag,PTO_Embed, attrs);
		}
		else
		{
			getDoc()->appendObject(PTO_Embed, attrs);
		}
	}
	else
	{
		XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
		if(pFrame == NULL)
		{
			 m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
			 return true;
		}
		FV_View * pView = static_cast<FV_View*>(pFrame->getCurrentView());
		if(pView == NULL)
		{
			m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
			return true;
		}
		getDoc()->insertObject(m_dposPaste, PTO_Embed, attrs, NULL);
		m_dposPaste++;
		if(m_posSavedDocPosition > 0)
			m_posSavedDocPosition++;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
// Handle copy and paste of tables by extending RTF. These handlers do that
///////////////////////////////////////////////////////////////////////////

bool IE_Imp_RTF::HandleAbiTable(void)
{
	std::string sProps;
	unsigned char ch;
	if (!ReadCharFromFile(&ch))
		return false;
	while(ch == ' ')
	{
		if (!ReadCharFromFile(&ch))
			return false;
	}
	// Since the star keyword group is between brackets, a new RTF state was generated. Remove it now.
	// The closing bracket will be ignored.
	PopRTFState();
	while (ch != '}') // Outer loop
	{
		sProps += ch;
		if (!ReadCharFromFile(&ch))
			return false;
	}

	ABI_Paste_Table * pPaste = new ABI_Paste_Table();
	m_pasteTableStack.push(pPaste);
	pPaste->m_bHasPastedTableStrux = false;
	pPaste->m_iRowNumberAtPaste = 0;
	
	UT_DEBUGMSG(("RTF_Import: Paste: Tables props are: %s \n",sProps.c_str()));
	bool bIsPasteIntoSame = false;
	pf_Frag_Strux* sdhTable = NULL;
	pf_Frag_Strux* sdhEndTable = NULL;
	bool bFound = getDoc()->getStruxOfTypeFromPosition(m_dposPaste,PTX_SectionTable,&sdhTable);
	PT_DocPosition posTable = 0;
	XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
	if(pFrame == NULL)
	{
		return false;
	}
	FV_View * pView = static_cast<FV_View*>(pFrame->getCurrentView());
	if(pView == NULL)
	{
		return false;
	}
	if(bFound)
	{
		posTable = getDoc()->getStruxPosition(sdhTable);
		sdhEndTable = getDoc()->getEndTableStruxFromTableSDH(sdhTable);
		if(sdhEndTable != NULL)
		{
			PT_DocPosition posEndTable = getDoc()->getStruxPosition(sdhEndTable);
			if(posEndTable > m_dposPaste)
			{
				std::string sPasteTableSDH;
				std::string sProp = "table-sdh";
				sPasteTableSDH = UT_std_string_getPropVal(sProps,sProp);
				std::string sThisTableSDH = UT_std_string_sprintf("%p",sdhTable);
				UT_DEBUGMSG(("sThisTableSDH %s sPasteTableSDH %s \n",sThisTableSDH.c_str(),sPasteTableSDH.c_str()));
				bool isRow = (pView->getSelectionMode() == FV_SelectionMode_TableRow);
				if(!isRow && pView->getSelectionMode() == FV_SelectionMode_NONE)
				{
					isRow = (pView->getPrevSelectionMode() == FV_SelectionMode_TableRow);
				}
				if((sThisTableSDH == sPasteTableSDH) && isRow)
				{
					UT_DEBUGMSG(("Paste Whole Row into same Table!!!!! \n"));
					bIsPasteIntoSame = true;
					pPaste->m_bPasteAfterRow = true;
					pf_Frag_Strux* sdhCell = NULL;
					bool b = getDoc()->getStruxOfTypeFromPosition(m_dposPaste,PTX_SectionCell,&sdhCell);
					UT_return_val_if_fail(b,false);
					const char * szTop = NULL;
					getDoc()->getPropertyFromSDH(sdhCell,
												 true,
												 PD_MAX_REVISION,
												 "top-attach",&szTop);
					UT_return_val_if_fail(szTop,false);
					UT_sint32 iOldTop = atoi(szTop); 
					PT_DocPosition posCell = getDoc()->getStruxPosition(sdhCell);
					b = true;
					bool atEnd = false;
					while(b)
					{
						b = getDoc()->getNextStruxOfType(sdhCell,PTX_SectionCell,&sdhCell);
						if(b && sdhCell)
						{
							posCell = getDoc()->getStruxPosition(sdhCell);
						}
						if(!b || (posCell > posEndTable))
						{
							atEnd = true;
							b = false;
						}
						else if(b)
						{
							getDoc()->getPropertyFromSDH(sdhCell,
														 true,
														 PD_MAX_REVISION,
														 "top-attach",&szTop);
							UT_return_val_if_fail(szTop,false);
							UT_sint32 iNewTop = atoi(szTop); 
							b = (iNewTop == iOldTop);
						}
					}
					pPaste->m_iRowNumberAtPaste = iOldTop;
					if(atEnd)
					{
//
// At end of Table. Position just after last encCell strux
//
						m_dposPaste = posEndTable;
					}
					else
					{
//
// Position just before the first cell of the next row.
//
						m_dposPaste = getDoc()->getStruxPosition(sdhCell);
					}
//
// Now a change strux on the table to make it rebuild.
//
					const char * sDumProp[3] = {NULL,NULL,NULL};
					sDumProp[0] = "list-tag";
					std::string sVal = UT_std_string_sprintf("%d",getDoc()->getUID(UT_UniqueId::List));
					sDumProp[1] = sVal.c_str();
					sDumProp[2] = NULL;
					getDoc()->changeStruxFmt(PTC_AddFmt,posTable+1,posTable+1,NULL,sDumProp,PTX_SectionTable);

				}
			}
		}
	}

//
// Remove the table-sdh property
//
	std::string sProp = "table-sdh";	
	UT_std_string_removeProperty(sProps,sProp);
	const gchar * attrs[3] = {"props",NULL,NULL};
	if(! bIsPasteIntoSame)
	{
		attrs[1] = sProps.c_str();
//
// insert a block to terminate the text before this if needed,
//
		if(getDoc()->isBlockAtPos(m_dposPaste) || getDoc()->isTableAtPos(m_dposPaste) || getDoc()->isEndFrameAtPos(m_dposPaste) ||getDoc()->isFrameAtPos(m_dposPaste) || getDoc()->isHdrFtrAtPos(m_dposPaste) || getDoc()->isCellAtPos(m_dposPaste))
		{
			FlushStoredChars(false);
		}
		else
		{
			m_newParaFlagged = true;
			FlushStoredChars(true);
			m_dposPaste--;
			if(m_posSavedDocPosition > 0)
				m_posSavedDocPosition--;
		}
//
// Insert the table strux at the same spot. This will make the table link correctly in the
// middle of the broken text.
		pPaste->m_bHasPastedTableStrux = true;
		insertStrux(PTX_SectionTable,attrs,NULL);
	}	
	return true;
}

bool IE_Imp_RTF:: HandleAbiEndTable(void)
{
	ABI_Paste_Table * pPaste = NULL;
	m_pasteTableStack.viewTop((void**)(&pPaste));
	if(pPaste == NULL)
	{
		return false;
	}
	if(pPaste->m_bPasteAfterRow)
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		UT_sint32 numRows = pPaste->m_iCurTopCell -pPaste->m_iRowNumberAtPaste;
		pf_Frag_Strux* sdhCell = NULL;
		pf_Frag_Strux* sdhTable = NULL;
		pf_Frag_Strux* sdhEndTable = NULL;
		bool b = getDoc()->getStruxOfTypeFromPosition(m_dposPaste,PTX_SectionTable,&sdhTable);
		UT_return_val_if_fail(b,false);
		sdhEndTable = getDoc()->getEndTableStruxFromTableSDH(sdhTable);
		UT_return_val_if_fail(sdhEndTable,false);
		PT_DocPosition posEndTable = getDoc()->getStruxPosition(sdhEndTable);
		b = getDoc()->getStruxOfTypeFromPosition(m_dposPaste,PTX_SectionCell,&sdhCell);
		b = getDoc()->getNextStruxOfType(sdhCell,PTX_SectionCell,&sdhCell);
		std::string sTop;
		std::string sBot;
		const char * szVal = NULL;
		const gchar * sProps[5] = {NULL,NULL,NULL,NULL};
		PT_DocPosition posCell = getDoc()->getStruxPosition(sdhCell);
		while(b && (posCell < posEndTable))
		{
			getDoc()->getPropertyFromSDH(sdhCell,
										 true,
										 PD_MAX_REVISION,
										 "top-attach", &szVal);
			UT_return_val_if_fail(szVal,false);
			UT_sint32 iTop = atoi(szVal);
			iTop += numRows;
			sTop = UT_std_string_sprintf("%d",iTop);
			getDoc()->getPropertyFromSDH(sdhCell,
										 true,
										 PD_MAX_REVISION,
										 "bot-attach", &szVal);
			UT_return_val_if_fail(szVal,false);
			UT_sint32 iBot = atoi(szVal);
			iBot += numRows;
			sTop = UT_std_string_sprintf("%d",iBot);
			sProps[0] = "top-attach";
			sProps[1] = sTop.c_str();
			sProps[2] = "bot-attach";
			sProps[3] = sBot.c_str();
			getDoc()->changeStruxFmt(PTC_AddFmt,posCell+1,posCell+1,NULL,sProps,PTX_SectionCell);
			b = getDoc()->getNextStruxOfType(sdhCell,PTX_SectionCell,&sdhCell);
			if(b)
			{
				posCell = getDoc()->getStruxPosition(sdhCell);
			}
		}
		return true;
	}

	insertStrux(PTX_EndTable);
	m_pasteTableStack.pop((void**)(&pPaste));
	delete pPaste;
	return true;
}

/*!
 * If there is an open paste cell mark that the a block has been pasted.
 */
bool IE_Imp_RTF::markPasteBlock(void)
{
	if(!bUseInsertNotAppend())
	{
		return false;
	}
	ABI_Paste_Table * pPaste = NULL;
	m_pasteTableStack.viewTop((void**)(&pPaste));
	if(pPaste == NULL)
	{
		return false;
	}
	if(!pPaste->m_bHasPastedBlockStrux)
	{
		pPaste->m_bHasPastedBlockStrux = true;
		return true;
	}
	return false;
}

bool IE_Imp_RTF::isBlockNeededForPasteTable(void)
{
	ABI_Paste_Table * pPaste = NULL;
	if(m_pasteTableStack.getDepth() == 0)
	{
		return false;
	}
	m_pasteTableStack.viewTop((void**)(&pPaste));
	if(pPaste == NULL)
	{
		return false;
	}
	if(!pPaste->m_bHasPastedBlockStrux)
	{
		return true;
	}
	return false;
}

bool IE_Imp_RTF::HandleAbiEndCell(void)
{
	ABI_Paste_Table * pPaste = NULL;
	m_pasteTableStack.viewTop((void**)(&pPaste));
	if(pPaste == NULL)
	{
		return false;
	}
	if(!pPaste->m_bHasPastedBlockStrux)
	{
		UT_DEBUGMSG(("Insert Block  -4 \n"));
	    insertStrux(PTX_Block);
	}
	UT_DEBUGMSG(("Insert EndCell -1!!!!!!!!!!!!!! \n"));
	insertStrux(PTX_EndCell);
	pPaste->m_bHasPastedCellStrux = false;
	pPaste->m_bHasPastedBlockStrux = false;
	return true;
}

bool IE_Imp_RTF::HandleAbiCell(void)
{
	std::string sProps;
	unsigned char ch;
	if (!ReadCharFromFile(&ch))
		return false;
	while(ch == ' ')
	{
		if (!ReadCharFromFile(&ch))
			return false;
	}
	// Since the star keyword group is between brackets, a new RTF state was generated. Remove it now.
	// The closing bracket will be ignored.
	PopRTFState();
	while (ch != '}') // Outer loop
	{
		sProps += ch;
		if (!ReadCharFromFile(&ch))
			return false;
	}

	ABI_Paste_Table * pPaste = NULL;
	m_pasteTableStack.viewTop((void**)(&pPaste));
	if(pPaste == NULL)
	{
		return false;
	}
	std::string sDum = "top-attach";
	std::string sTop = UT_std_string_getPropVal(sProps,sDum);
	pPaste->m_iCurTopCell = atoi(sTop.c_str());
	UT_sint32 iAdditional = 0;
	//
	// Look for the next row of the table
	//
	iAdditional = pPaste->m_iCurTopCell - pPaste->m_iPrevPasteTop;
	pPaste->m_iPrevPasteTop = pPaste->m_iCurTopCell;
	pPaste->m_iRowNumberAtPaste += iAdditional;
	pPaste->m_iNumRows += iAdditional;
	sDum = "right-attach";
	std::string sRight = UT_std_string_getPropVal(sProps,sDum);
	pPaste->m_iCurRightCell = atoi(sRight.c_str());
	if(pPaste->m_iCurRightCell > pPaste->m_iMaxRightCell)
	{
		pPaste->m_iMaxRightCell = pPaste->m_iCurRightCell;
	}
	pPaste->m_bHasPastedCellStrux = true;
	pPaste->m_bHasPastedBlockStrux = false;
	UT_sint32 iMyTop = pPaste->m_iCurTopCell;
	sDum = "bot-attach";
	std::string sBot =  UT_std_string_getPropVal(sProps,sDum);
	UT_sint32 iMyBot = atoi(sBot.c_str());
	if(pPaste->m_bPasteAfterRow)
	{
		UT_sint32 idiff = pPaste->m_iRowNumberAtPaste - iMyTop +1;
		iMyTop += idiff;
		sTop = UT_std_string_sprintf("%d",iMyTop);
		iMyBot += idiff;
		sBot = UT_std_string_sprintf("%d",iMyBot);
		std::string sTopProp = "top-attach";
		std::string sBotProp = "bot-attach";
		UT_std_string_setProperty(sProps,sTopProp,sTop);
		UT_std_string_setProperty(sProps,sBotProp,sBot);
		pPaste->m_iCurTopCell = iMyTop;
	}
	UT_DEBUGMSG(("RTF_Import: Pos %d Paste: Cell props are: %s \n",m_dposPaste,sProps.c_str()));
	const gchar * attrs[3] = {"props",NULL,NULL};
	attrs[1] = sProps.c_str();
 	insertStrux(PTX_SectionCell,attrs,NULL);
	m_newParaFlagged = true;
	m_bSectionHasPara = true;

	return true;
}

/*!
 * Handle context senstive inserts. Like inserting a table into a block
 * Requires an extra block insert
 * Insert into a hyperlink means the m_dposPate is additionally incremented
 * to handle the extra end hyperlink run.
 */
bool IE_Imp_RTF::insertStrux(PTStruxType pts , const gchar ** attrs, const gchar ** props)
{
	bool bInHyperlink = false;
	bool bDoExtraBlock = false;
	bool res = false;
	XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
	if(pFrame == NULL)
	{
		m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
		return true;
	}
	FV_View * pView = static_cast<FV_View*>(pFrame->getCurrentView());
	PT_DocPosition posEOD = 0;
	pView->getEditableBounds(true,posEOD);
	if(pView == NULL)
	{
		m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
		return true;
	}
	if(!m_bStruxInserted)
	{
		fp_Run *pHyperRun = pView->getHyperLinkRun(m_dposPaste);
		if((pHyperRun != NULL) ||(m_iHyperlinkOpen > 0) )
		{
			fp_HyperlinkRun * pRHyper = static_cast<fp_HyperlinkRun *>(pHyperRun);
			if(pRHyper->getHyperlinkType() == HYPERLINK_NORMAL)
				bInHyperlink = true;
		}
		fl_BlockLayout * pBL = pView->getBlockAtPosition(m_dposPaste);
		if(pBL->getPosition() < m_dposPaste)
		{
			bDoExtraBlock = true;
		}
	}
	bool isInHdrFtr = pView->isInHdrFtr(m_dposPaste);
	if(isInHdrFtr)
	{
		if((pts != PTX_Block) &&  (pts != PTX_SectionTable) && (pts != PTX_SectionCell) && (pts != PTX_EndTable) &&(pts != PTX_EndCell))
		{
			m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
			return true;
		} 
		//
		// No nested tables in header/footers
		//
		if(pView->isInTable(m_dposPaste))
		{ 
			fl_TableLayout * pTL =pView->getTableAtPos(m_dposPaste);
			if(pTL && pTL->isEndTableIn() && ((pts == PTX_SectionTable)|| (pts == PTX_SectionCell) || (pts == PTX_EndTable) || (pts == PTX_EndCell)))
			{
				m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
				return true;
			}
		}
		if((m_pasteTableStack.getDepth() > 2) && 
		   ((pts == PTX_SectionTable) || (pts == PTX_SectionCell) || (pts == PTX_EndTable) || (pts == PTX_EndCell)))
		{
			return true;
		} 
	}
//
// Can't insert into a TOC 
//
	if(getDoc()->isTOCAtPos(m_dposPaste) && 
	   getDoc()->isTOCAtPos(m_dposPaste-1) && 
		(pts != PTX_EndTOC))
	{
		m_dposPaste--;
		if(m_posSavedDocPosition > 0)
			m_posSavedDocPosition--;
	}
	if(bDoExtraBlock && (pts == PTX_SectionTable))
	{
		//		getDoc()->insertStrux(m_dposPaste,PTX_Block);
		if(bInHyperlink)
		{
			//	m_dposPaste++;
			bInHyperlink = false;
		}
	}
	if(pts == PTX_SectionFrame)
	{
		pf_Frag_Strux * pfs = NULL;
		if(pView->isInFrame(m_dposPaste))
		{
			PT_DocPosition pos = m_dposPaste;
			
			while(( getDoc()->isFrameAtPos(pos) || pView->isInFrame(pos)) && pos <= posEOD)
			{
				pos++;
			} 
			if(pos > posEOD)
			{
				pos = posEOD;
			}
			m_dposPaste = pos;
		}
		res = getDoc()->insertStrux(m_dposPaste,pts,attrs,props,&pfs);
		m_dposPaste = pfs->getPos()+1;
		return res;
	}
	if((pts == PTX_EndFrame) && (getDoc()->isFrameAtPos(m_dposPaste)))
	{
		res = getDoc()->insertStrux(m_dposPaste,PTX_Block);
		m_dposPaste++;
		res = getDoc()->insertStrux(m_dposPaste,pts,attrs,props);
		m_dposPaste++;
		if(	bInHyperlink)
		{
			m_iHyperlinkOpen =0;
		}
		m_bStruxInserted = true;
		return res;
	}
	//
	// Can't  paste sections in Footnotes/Endnotes
	//
	if(pts == PTX_Section)
	{ 
		if(pView->getEmbedDepth(m_dposPaste) > 0)
		{
			return false;
		}
		fl_BlockLayout * pBL = pView->getBlockAtPosition(m_dposPaste);
		if(pBL == NULL)
		{
			return false;
		}
		if(pBL->myContainingLayout() == NULL)
		{
			return false;
		}
		if(pBL->myContainingLayout()->getContainerType() !=  FL_CONTAINER_DOCSECTION)
		{
			return false;
		}
		if(pBL->getPosition() > m_dposPaste)
		{
			return false;
		}
		if((pBL->getPosition(true) + pBL->getLength()) < m_dposPaste)
		{
			return false;
		}
		if((pBL->getPrev() == NULL))
		{
			return false;
		}
		if((pBL->getNext() == NULL))
		{
			return false;
		}
		if(pBL->getNext()->getContainerType() != FL_CONTAINER_BLOCK)
		{
			return false;
		}
		if(pBL->getPrev()->getContainerType() != FL_CONTAINER_BLOCK)
		{
			return false;
		}
	}
	res = getDoc()->insertStrux(m_dposPaste,pts,attrs,props);
	m_dposPaste++;
	if(m_posSavedDocPosition > 0)
		m_posSavedDocPosition++;
	if(	bInHyperlink)
	{
		m_dposPaste++;
		m_iHyperlinkOpen =0;
		if(m_posSavedDocPosition > 0)
			m_posSavedDocPosition++;
	}
	m_bStruxInserted = true;
	return res;
}

		

//////////////////////////////////////////////////////////////////////////////
// AbiList table reader
//////////////////////////////////////////////////////////////////////////////

bool IE_Imp_RTF::HandleAbiLists()
{
	unsigned char keyword[MAX_KEYWORD_LEN];
	unsigned char ch;
	UT_sint32 parameter = 0;
	bool paramUsed = false;
	if (!ReadCharFromFile(&ch))
		return false;

	// Since the star keyword group is between brackets, a new RTF state was generated. Remove it now.
	// The closing bracket will be ignored.
	PopRTFState();
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
					 while ( ch != '}'  && ch != ';' && count < MAX_KEYWORD_LEN - 1)
					 {
						 keyword[count++] = ch;
						 if (!ReadCharFromFile(&ch))
							 return false;
					 }
					 keyword[count++] = 0;
					 strncpy(m_currentRTFState.m_paraProps.m_pszStyle,reinterpret_cast<char*>(&keyword[0]), sizeof(m_currentRTFState.m_paraProps.m_pszStyle));
					 m_currentRTFState.m_paraProps.m_pszStyle[sizeof(m_currentRTFState.m_paraProps.m_pszStyle) - 1] = 0;
				 }
				 else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "abilistdecimal") == 0)
				 {
			  // OK scan through the text until a closing delimeter is
			  // found
					 int count = 0;
					 if (!ReadCharFromFile(&ch))
						 return false;
					 while ( ch != '}'  && ch != ';' && count < MAX_KEYWORD_LEN - 1)
					 {
						 keyword[count++] = ch;
						 if (!ReadCharFromFile(&ch))
							 return false;
					 }
					 keyword[count++] = 0;
					 strncpy(m_currentRTFState.m_paraProps.m_pszListDecimal,reinterpret_cast<char*>(&keyword[0]), sizeof(m_currentRTFState.m_paraProps.m_pszListDecimal));
					 m_currentRTFState.m_paraProps.m_pszListDecimal[sizeof(m_currentRTFState.m_paraProps.m_pszListDecimal) - 1] = 0;
				 }
				 else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "abilistdelim") == 0)
				 {
			  // OK scan through the text until a closing delimeter is
			  // found
					 int count = 0;
					 if (!ReadCharFromFile(&ch))
						 return false;
					 while ( ch != '}'  && ch != ';' && count < MAX_KEYWORD_LEN - 1)
					 {
						 keyword[count++] = ch;
						 if (!ReadCharFromFile(&ch))
							 return false;
					 }
					 keyword[count++] = 0;
					 strncpy(m_currentRTFState.m_paraProps.m_pszListDelim,reinterpret_cast<char*>(&keyword[0]), sizeof(m_currentRTFState.m_paraProps.m_pszListDelim));
					 m_currentRTFState.m_paraProps.m_pszListDelim[sizeof(m_currentRTFState.m_paraProps.m_pszListDelim) - 1] = 0;
				 }
				 else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "abifieldfont") == 0)
				 {
			  // OK scan through the text until a closing delimeter is
			  // found
					 int count = 0;
					 if (!ReadCharFromFile(&ch))
						 return false;
					 while ( ch != '}'  && ch != ';' && count < MAX_KEYWORD_LEN - 1)
					 {
						 keyword[count++] = ch;
						 if (!ReadCharFromFile(&ch))
							 return false;
					 }
					 keyword[count++] = 0;
					 strncpy(m_currentRTFState.m_paraProps.m_pszFieldFont,reinterpret_cast<char*>(&keyword[0]), sizeof(m_currentRTFState.m_paraProps.m_pszFieldFont));
					 m_currentRTFState.m_paraProps.m_pszFieldFont[sizeof(m_currentRTFState.m_paraProps.m_pszFieldFont) - 1] = 0;
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
			m_vecAbiListTable.push_back(new _rtfAbiListTable);
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

	return true;
}



//////////////////////////////////////////////////////////////////////////////
// Character Properties keyword handlers
//////////////////////////////////////////////////////////////////////////////
bool IE_Imp_RTF::HandleRevisedText(PP_RevisionType eType, UT_uint32 iId)
{
	UT_return_val_if_fail( FlushStoredChars(), false);

	// remember the id for future reference
	m_currentRTFState.m_charProps.m_iCurrentRevisionId = iId;
	m_currentRTFState.m_charProps.m_eRevision = eType;
	
#if 0
	switch(eType)
	{
		case PP_REVISION_ADDITION:
		case PP_REVISION_ADDITION_AND_FMT:

		case PP_REVISION_DELETION:

		case PP_REVISION_FMT_CHANGE:
			;
	}
#endif
	return true;
}

bool IE_Imp_RTF::HandleRevisedTextTimestamp(UT_uint32 iDttm)
{
	// we basically rely on the dttm keyword to follow the auth keyword -- Word outputs
	// them in this sequence, and so do we
	UT_return_val_if_fail( m_currentRTFState.m_charProps.m_iCurrentRevisionId > 0,true); // was false (this enables RTF spec to load)
	
	const UT_GenericVector<AD_Revision*> & Rtbl = getDoc()->getRevisions();
	UT_return_val_if_fail(Rtbl.getItemCount(),true); // was false (This enables RTF spec to load)

	// valid revision id's start at 1, but vector is 0-based
	AD_Revision * pRev = Rtbl.getNthItem(m_currentRTFState.m_charProps.m_iCurrentRevisionId - 1);

	UT_return_val_if_fail( pRev, false );

	if(!pRev->getStartTime())
	{
		// set the start time to what ever is represented by dttm
		struct tm TM;
		TM.tm_sec = 0;
		TM.tm_min = (iDttm & 0x3f);
		TM.tm_hour = (iDttm & 0x7c0) >> 6;
		TM.tm_mday = (iDttm & 0xf800) >> 11;
		TM.tm_mon = ((iDttm & 0xf0000) >> 16)- 1;
		TM.tm_year = (iDttm & 0x1ff00000) >> 20;
		TM.tm_isdst = 0;

		time_t tT = mktime(&TM);
		pRev->setStartTime(tT);
	}
	
	return true;
}


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

bool IE_Imp_RTF::HandleHidden(bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_Hidden);
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
	bool retval;

	retval = HandleU32CharacterProp(fontNumber, &m_currentRTFState.m_charProps.m_fontNumber);
	setEncoding();  // Activate character encoding
	return retval;
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
	m_currentRTFState.m_paraProps.m_tabStops.push_back(stopDist);	// convert from twip to inch
	if(tabType >=FL_TAB_LEFT && tabType <= FL_TAB_BAR  )
	{
		m_currentRTFState.m_paraProps.m_tabTypes.push_back(tabType);
	}
	else
	{
		m_currentRTFState.m_paraProps.m_tabTypes.push_back(FL_TAB_LEFT);
	}
	if(tabLeader >= FL_LEADER_NONE  && tabLeader <= FL_LEADER_EQUALSIGN)
	{
		m_currentRTFState.m_paraProps.m_tabLeader.push_back(tabLeader);
	}
	else
	{
		m_currentRTFState.m_paraProps.m_tabLeader.push_back(FL_LEADER_NONE);
	}

	return true;
}



bool IE_Imp_RTF::AddTabstop(UT_sint32 stopDist, eTabType tabType, eTabLeader tabLeader,  RTFProps_ParaProps * pParas)
{
	pParas->m_tabStops.push_back(stopDist);	// convert from twip to inch
	if(tabType >=FL_TAB_LEFT && tabType <= FL_TAB_BAR  )
	{
		pParas->m_tabTypes.push_back(tabType);
	}
	else
	{
		pParas->m_tabTypes.push_back(FL_TAB_LEFT);
	}
	if(tabLeader >= FL_LEADER_NONE  && tabLeader <= FL_LEADER_EQUALSIGN)
	{
		pParas->m_tabLeader.push_back(tabLeader);
	}
	else
	{
		pParas->m_tabLeader.push_back(FL_LEADER_NONE);
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
  \retval pParam is the keyword parameter if any, otherwise "". RTF spec says it should be
  a signed 16-bits int.
  \retval pParamUsed is a flag to tell whether there is a parameter.
  \return the type of the next token parsed.
  \note Both pParam amd pParamUsed are only used if tokenType is
  RTF_TOKEN_KEYWORD
  \note this changes the state of the file
*/
IE_Imp_RTF::RTFTokenType IE_Imp_RTF::NextToken (unsigned char *pKeyword, UT_sint32* pParam,
												bool* pParamUsed, UT_uint32 len, bool bIgnoreWhiteSpace /* = false */ )
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

		while(pKeyword[0] == ' ')
		{
			if (!ReadCharFromFile(pKeyword))
			{
				tokenType = RTF_TOKEN_ERROR;
				return tokenType;
			}
		}
	}
	else
	{
		if (!ReadCharFromFile(pKeyword))
		{
			tokenType = RTF_TOKEN_ERROR;
			return tokenType;
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
	UT_UTF8String bookmarkName;

	xxx_UT_DEBUGMSG(("hub: HandleBookmark of type %d\n", type));

	/* read the bookmark name. It is PCDATA hence we are likely to find non ASCII data.*/
	HandlePCData(bookmarkName);

	const gchar * props [5];
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
	props [3] = bookmarkName.utf8_str();
	props [4] = NULL;
	UT_DEBUGMSG(("SEVIOR: Appending Object 3 m_bCellBlank %d m_bEndTableOpen %d \n",m_bCellBlank,m_bEndTableOpen));
	if(m_bCellBlank || m_bEndTableOpen || !m_bSectionHasPara)
	{
		UT_DEBUGMSG(("Insert/Append block 3 \n"));
		if (m_newSectionFlagged)
		{
			ApplySectionAttributes();
			m_newSectionFlagged = false;
		}
		if(!bUseInsertNotAppend()) 
		{
			if(m_pDelayedFrag)
			{
				getDoc()->insertStruxBeforeFrag(m_pDelayedFrag,PTX_Block,NULL);
			}
			else
			{
				getDoc()->appendStrux(PTX_Block,NULL);
			}
		}
		else
		{
			markPasteBlock();
			insertStrux(PTX_Block);
		}
		m_bCellBlank = false;
		m_bEndTableOpen = false;
		m_bSectionHasPara = true;
		m_newParaFlagged = false;
	}

	if (!bUseInsertNotAppend()) 
	{
			if(m_pDelayedFrag)
			{
				getDoc()->insertObjectBeforeFrag(m_pDelayedFrag,PTO_Bookmark,props);
			}
			else
			{
				getDoc()->appendObject(PTO_Bookmark, props);
			}
	}
	else 
	{
		if(isBlockNeededForPasteTable())
		{
			markPasteBlock();
			insertStrux(PTX_Block);
		}
		getDoc()->insertObject(m_dposPaste, PTO_Bookmark, props, NULL);
		m_dposPaste++;
		if(m_posSavedDocPosition > 0)
			m_posSavedDocPosition++;
	}
	return true;
}


bool IE_Imp_RTF::HandleRDFAnchor (RTFBookmarkType type)
{
	UT_DEBUGMSG(("HandleRDFAnchor() of type %d is-start:%d is-end:%d use-app:%d\n",
				 type,
				 type == RBT_START, type == RBT_END,
				 !bUseInsertNotAppend()  ));

	std::string xmlid;
	HandlePCData(xmlid);

	UT_DEBUGMSG(("HandleRDFAnchor() of type %d original xmlid:%s\n", type, xmlid.c_str()));
	if( type == RBT_START )
	{
		PD_XMLIDCreatorHandle xidc = m_XMLIDCreatorHandle;
		std::string updatedxmlid = xidc->createUniqueXMLID( xmlid );
		m_rdfAnchorCloseXMLIDs.insert( make_pair( xmlid, updatedxmlid ) );
		xmlid = updatedxmlid;
	}
	else
	{
		xmlid = m_rdfAnchorCloseXMLIDs[ xmlid ];
		m_rdfAnchorCloseXMLIDs.erase( xmlid );
	}
	UT_DEBUGMSG(("HandleRDFAnchor() of type %d updated  xmlid:%s\n", type, xmlid.c_str()));
	
	const gchar* ppAtts[10] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
	int ppIdx = 0;
	ppAtts[ppIdx++] = PT_XMLID;
	ppAtts[ppIdx++] = xmlid.c_str();
	ppAtts[ppIdx++] = "this-is-an-rdf-anchor";
	ppAtts[ppIdx++] = "yes";
	switch (type)
	{
		case RBT_START:
			m_iRDFAnchorOpen--;
			break;
		case RBT_END:
			m_iRDFAnchorOpen++;
			ppAtts[ppIdx++] = PT_RDF_END;
			ppAtts[ppIdx++] = "yes";
			break;
	}
	
	if (!bUseInsertNotAppend()) 
	{
			if(m_pDelayedFrag)
			{
				getDoc()->insertObjectBeforeFrag(m_pDelayedFrag,PTO_RDFAnchor,ppAtts);
			}
			else
			{
				getDoc()->appendObject(PTO_RDFAnchor, ppAtts);
			}
	}
	else 
	{
		if(isBlockNeededForPasteTable())
		{
			markPasteBlock();
			insertStrux(PTX_Block);
		}
		getDoc()->insertObject(m_dposPaste, PTO_RDFAnchor, ppAtts, NULL);
		m_dposPaste++;
		if(m_posSavedDocPosition > 0)
			m_posSavedDocPosition++;
	}
	
	return true;
}

bool IE_Imp_RTF::HandleDeltaMoveID()
{
	std::string moveid;
	HandlePCData(moveid);
	
	UT_DEBUGMSG(("HandleDeltaMoveID() of t %d\n", 1 ));
	UT_DEBUGMSG(("HandleDeltaMoveID() of dposPaste %d\n", m_dposPaste ));
	UT_DEBUGMSG(("HandleDeltaMoveID() move-id %s\n", moveid.c_str() ));
	if( !moveid.empty() )
	{
//		m_ctMoveID = moveid;
		pf_Frag_Strux* sdh;
		bool bResult = getDoc()->getStruxOfTypeFromPosition( m_dposPaste, PTX_Block, &sdh);
		if( bResult )
		{
			getDoc()->changeStruxAttsNoUpdate( sdh, "delta:move-idref", moveid.c_str() );
		}
	}
	return true;
}


void IE_Imp_RTF::_appendHdrFtr ()
{
	UT_uint32 i;
	UT_uint32 numHdrFtr;
	const RTFHdrFtr * header;
	std::string tempBuffer;
	const gchar* szType = NULL;

	UT_return_if_fail(m_pImportFile);

	numHdrFtr = m_hdrFtrTable.size();

	for (i = 0; i < numHdrFtr; i++)
	{
		header = m_hdrFtrTable[i];

		m_pPasteBuffer = reinterpret_cast<const unsigned char *>(header->m_buf.getPointer (0));
		m_lenPasteBuffer = header->m_buf.getLength ();
		m_pCurrentCharInPasteBuffer = m_pPasteBuffer;
		m_dposPaste = FV_DOCPOS_EOD;
		const gchar* propsArray[9];
		std::string hdrftrID;
		switch (header->m_type)
		{
		case RTFHdrFtr::hftHeader:
			tempBuffer = UT_std_string_sprintf ("%u", header->m_id);
			szType = "header";
			break;
		case RTFHdrFtr::hftHeaderEven:
			tempBuffer = UT_std_string_sprintf ("%u", header->m_id);
			szType = "header-even";
			break;
		case RTFHdrFtr::hftHeaderFirst:
			tempBuffer = UT_std_string_sprintf ("%u", header->m_id);
			szType = "header-first";
			break;
		case RTFHdrFtr::hftHeaderLast:
			tempBuffer = UT_std_string_sprintf ("%u", header->m_id);
			szType = "header-last";
			break;
		case RTFHdrFtr::hftFooter:
			tempBuffer = UT_std_string_sprintf ("%u", header->m_id);
			szType = "footer";
			break;
		case RTFHdrFtr::hftFooterEven:
			tempBuffer = UT_std_string_sprintf ("%u", header->m_id);
			szType = "footer-even";
			break;
		case RTFHdrFtr::hftFooterFirst:
			tempBuffer = UT_std_string_sprintf ("%u", header->m_id);
			szType = "footer-first";
			break;
		case RTFHdrFtr::hftFooterLast:
			tempBuffer = UT_std_string_sprintf ("%u", header->m_id);
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
			pf_Frag_Strux* sdh = getDoc()->getLastSectionMutableSDH();
			getDoc()->changeStruxAttsNoUpdate(sdh,szType,hdrftrID.c_str());
		}
		getDoc()->appendStrux (PTX_SectionHdrFtr, propsArray);
		propsArray[0] = NULL;
		// actually it appears that we have to append a block for some cases.
		UT_DEBUGMSG(("Append block 4 with props \n"));
#if 0 //#TF
		getDoc()->appendStrux(PTX_Block, propsArray);
#endif
		// tell that we are parsing headers and footers
		m_parsingHdrFtr = true;
		m_newParaFlagged = true;
		m_bSectionHasPara = false;
		_parseFile (NULL);
		m_parsingHdrFtr = false;
	}
}

/*!
  Appends a field to the document.
  \param xmlField the field type value
  \return true if OK
 */
bool IE_Imp_RTF::_appendField (const gchar *xmlField, const gchar ** pszAttribs)
{
	bool ok;
	const gchar** propsArray = NULL;
	std::string propBuffer;
	buildCharacterProps(propBuffer);

	const gchar * pStyle = NULL;
	std::string styleName;
	if(m_currentRTFState.m_charProps.m_styleNumber >= 0
	   && static_cast<UT_uint32>(m_currentRTFState.m_charProps.m_styleNumber) < m_styleTable.size())
	{
		pStyle = PT_STYLE_ATTRIBUTE_NAME;
		styleName = m_styleTable[m_currentRTFState.m_charProps.m_styleNumber];
	}
	bool bNoteRef = false;
	if((strcmp(xmlField,"endnote_ref") == 0) || (strcmp(xmlField,"footnote_ref") == 0))
	{
		bNoteRef = true;
	}
	if(pszAttribs == NULL)
	{
		propsArray = static_cast<const gchar **>(UT_calloc(7, sizeof(gchar *)));		
		propsArray [0] = "type";
		propsArray [1] = xmlField;
		propsArray [2] = "props";
		propsArray [3] = propBuffer.c_str();
		propsArray [4] = pStyle;
		propsArray [5] = styleName.c_str();
		propsArray [6] = NULL;
	}
	else
	{
		UT_uint32 isize =0;
		while(pszAttribs[isize] != NULL)
		{
			isize++;
		}
		propsArray = static_cast<const gchar **>(UT_calloc(7+isize, sizeof(gchar *)));

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
			propsArray[iEmptyAttrib++] = styleName.c_str();
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
	if (!bUseInsertNotAppend() || m_bAppendAnyway)
	{
		UT_DEBUGMSG(("SEVIOR: Appending Object m_bCellBlank %d m_bEndTableOpen %d \n",m_bCellBlank,m_bEndTableOpen));
		if(m_bCellBlank || m_bEndTableOpen)
		{
			UT_DEBUGMSG(("Append block 5 \n"));
			if(m_pDelayedFrag)
			{
				getDoc()->insertStruxBeforeFrag(m_pDelayedFrag,PTX_Block,NULL);
			}
			else
			{
				getDoc()->appendStrux(PTX_Block,NULL);
			}
			m_bCellBlank = false;
			m_bEndTableOpen = false;
		}
		if(m_pDelayedFrag)
	    {
			getDoc()->insertObjectBeforeFrag(m_pDelayedFrag,PTO_Field, propsArray);
		}
		else
		{
			getDoc()->appendObject(PTO_Field, propsArray);
		}
	}
	else
	{
		XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
		if(pFrame == NULL)
		 {
			 m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
			 return true;
		 }
		FV_View * pView = static_cast<FV_View*>(pFrame->getCurrentView());
		if(pView == NULL)
		{
			m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
			return true;
		}
		//
		// No pasting footnotes/endnotes into text boxes, paste just before
		// it.
		//
		if(bNoteRef && pView->isInFrame(m_dposPaste))
		{
			fl_FrameLayout * pFL = pView->getFrameLayout(m_dposPaste);
			if(pFL == NULL)
			{
				m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
				return true;
			}
			PT_DocPosition newPos = pFL->getPosition(true);
			while(newPos > 2 && getDoc()->isEndFrameAtPos(newPos-1))
			{
				pFL = pView->getFrameLayout(newPos-2);
				if(pFL)
				{
					newPos = pFL->getPosition(true);
				}
			}
			m_dPosBeforeFootnote = m_dposPaste - newPos;
			m_bMovedPos = true;
			m_dposPaste = newPos;
		}
		getDoc()->insertObject(m_dposPaste, PTO_Field, propsArray, NULL);
		m_dposPaste++;
		if(m_posSavedDocPosition > 0)
			m_posSavedDocPosition++;
	}
	g_free(propsArray);
	m_bFieldRecognized = true;
	return ok;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

bool IE_Imp_RTF::pasteFromBuffer(PD_DocumentRange * pDocRange,
								 const unsigned char * pData, UT_uint32 lenData, const char * /* szEncoding */)
{
	UT_return_val_if_fail(getDoc() == pDocRange->m_pDoc,false);
	UT_return_val_if_fail(pDocRange->m_pos1 == pDocRange->m_pos2,false);

	m_pPasteBuffer = pData;
	m_lenPasteBuffer = lenData;
	m_pCurrentCharInPasteBuffer = pData;
	m_dposPaste = pDocRange->m_pos1;
	setClipboard(m_dposPaste);  
	m_dOrigPos = m_dposPaste;
	// some values to start with -- most often we are somewhere in the middle of doc,
	// i.e., in section and in block
	m_newParaFlagged = false;
	m_bSectionHasPara = true;
	m_newSectionFlagged = false;
	
	// we need to work out if we are in section and block
	pf_Frag * pf = getDoc()->getFragFromPosition(m_dposPaste);

	if(!pf)
	{
		// the doc is entirely empty
		m_newParaFlagged = true;
		m_bSectionHasPara = false;
		m_newSectionFlagged = true;
	}
	else
	{
		// pf is a frag that starts at m_dposPaste -- we want the frag that ends there
		pf = pf->getPrev();
		
		// now find the nearest strux to the left
		while(pf && pf->getType() != pf_Frag::PFT_Strux)
			pf = pf->getPrev();

		if(!pf)
		{
			// this is a malformed doc -- it has content but no stuxes !!!
			UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
			m_newParaFlagged = true;
			m_bSectionHasPara = false;
			m_newSectionFlagged = true;
		}
		else
		{
			// what kind of strux have we hit ?
			pf_Frag_Strux * pfs = (pf_Frag_Strux*) pf;
			switch(pfs->getStruxType())
			{
				case PTX_Block:
				case PTX_EndFootnote:
				case PTX_EndEndnote:
					// we are ok
					break;

				default:
					UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
					// fall through
					
				case PTX_Section:
				case PTX_SectionHdrFtr:
				case PTX_SectionEndnote:
				case PTX_SectionTable:
				case PTX_SectionCell:
				case PTX_SectionFootnote:
				case PTX_SectionMarginnote:
				case PTX_SectionFrame:
				case PTX_EndCell:
				case PTX_EndTable:
				case PTX_EndMarginnote:
				case PTX_EndFrame:
				case PTX_SectionTOC:
				case PTX_EndTOC:
				case PTX_StruxDummy:
					// flag block
					m_newParaFlagged = true;
					m_bSectionHasPara = false;
			}
		}
		
		
	}
	
	

	UT_DEBUGMSG(("Pasting %d bytes of RTF\n",lenData));
#if 1 //def DEBUG
	{
		const char * p = (const char*)pData;
		for(UT_uint32 i = 0; i < lenData; i += 50)
		{
			if(lenData - i < 50)
			{
				std::string s(p);
				UT_DEBUGMSG(("%s\n", s.c_str()));
			}
			else
			{
				std::string s(p, 50);
				UT_DEBUGMSG(("%s\n", s.c_str()));
				p += 50;
			}
		}
	}
#endif

	// to do a paste, we set the fp to null and let the
	// read-a-char routines know about our paste buffer.

	UT_return_val_if_fail(m_pImportFile==NULL,false);

	// note, we skip the _writeHeader() call since we don't
	// want to assume that selection starts with a section
	// break.
	_parseFile(NULL);

	if(m_newParaFlagged)
	{
//
// Finish off any remaining stuff
//
		FlushStoredChars(false);
	}
	//
	// Look if we're at the end of the document
	//
	PT_DocPosition posEnd;
	getDoc()->getBounds(true,posEnd);
	if(getDoc()->isEndTableAtPos(m_dposPaste-1))
	{
		if((posEnd==m_dposPaste) || (getDoc()->isSectionAtPos(m_dposPaste)) ||
		   (getDoc()->isHdrFtrAtPos(m_dposPaste)))
		{
			getDoc()->insertStrux(m_dposPaste,PTX_Block);
			m_dposPaste++;
			if(m_posSavedDocPosition > 0)
				m_posSavedDocPosition++;
		}
	}
	m_pPasteBuffer = NULL;
	m_lenPasteBuffer = 0;
	m_pCurrentCharInPasteBuffer = NULL;
	return true;
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
	const char styleTypeP[] = "P";
	const char styleTypeC[] = "C";
	const char * styleType = styleTypeP;
	
	UT_sint32 BasedOn[2000]; // 2000 styles. I know this should be a Vector.
	UT_sint32 FollowedBy[2000]; // 2000 styles. I know this should be a Vector.
	UT_sint32 styleCount = 0;
	UT_GenericVector<UT_GenericVector<const gchar*>*> vecStyles;
	RTFProps_ParaProps * pParas =  new RTFProps_ParaProps();
	RTFProps_CharProps *  pChars = new	RTFProps_CharProps();
	RTFProps_bParaProps * pbParas =  new RTFProps_bParaProps();
	RTFProps_bCharProps *  pbChars = new	RTFProps_bCharProps();

	static std::string propBuffer;

	const gchar* attribs[PT_MAX_ATTRIBUTES*2 + 1];
	UT_uint32 attribsCount=0;
	UT_UCS4String styleName;// = "";
	UT_sint32 styleNumber = 0;
	while (nesting>0 && status == true)
	{
        unsigned char keyword[MAX_KEYWORD_LEN];
        UT_sint32 parameter = 0;
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
			else if (strcmp(reinterpret_cast<const char *>(&keyword[0]), "'") == 0) {
				/* FIXME really hackish. What if we have this in middle of keywords */
				UT_UCS4Char wc;
				wc = ReadHexChar();
				styleName += wc;
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
					UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );
					attribs[attribsCount++] = NULL;
					UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );
					attribs[attribsCount]   = NULL;
				}
				else if(0)
				{
					// TODO: Why is this code here? It left over from before the BasedOn array
					const std::string & val = m_styleTable[parameter];
					if (!val.empty())
					{
						attribs[attribsCount++] = PT_BASEDON_ATTRIBUTE_NAME;
						UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );
						attribs[attribsCount++] = val.c_str();
						UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );
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
					UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );
					attribs[attribsCount++] = NULL;
					UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );
					attribs[attribsCount]   = NULL;
				}
				else if(parameter < styleNumber)
				{
					// TODO: Why is this code here? It left over from before the FollowedBy array
					const std::string & val = m_styleTable[parameter];
					if (!val.empty())
					{
	               		attribs[attribsCount++] = PT_FOLLOWEDBY_ATTRIBUTE_NAME;
						UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );
						attribs[attribsCount++] = val.c_str();
						UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );
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
				xxx_UT_DEBUGMSG(("Stylesheet RTF Found style number %d Paragraph type \n",styleNumber));
			}
			if (strcmp(reinterpret_cast<char*>(&keyword[0]), "cs") == 0)
			{
				styleNumber = parameter;
				styleType = styleTypeC;
				xxx_UT_DEBUGMSG(("Stylesheet: RTF Found style number %d Character type \n",styleNumber));
			}
			else if (strcmp(reinterpret_cast<char*>(&keyword[0]), "*") == 0)
			{
//
// Get next keyword
//
				xxx_UT_DEBUGMSG(("Found * in StyleSheet reading \n"));
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

			// clear the m_mbtowc buffer
			m_mbtowc.initialize(true);
			
			while (ch != '}' && ch != ';')
			{
				/* 
				   we have seen cases, including AbiWord, were stylename
				   were generated with non ASCII names encoded as 8bits...
				   We assume it is the document charset.
				*/
				UT_UCS4Char wc;
				if(m_mbtowc.mbtowc(wc,static_cast<UT_Byte>(ch)))
					styleName += wc;
				else
					styleName += ch;
				
                if (!ReadCharFromFile(&ch)) {
		            return false;
				}
				if (ch =='}')
				{
					UT_DEBUGMSG(("RTF: Badly formatted style name, no ';'"));
					nesting--;
				}
			}
			if(styleNumber >= 0)
			{
				std::vector<std::string>::size_type newSize = styleNumber + 1;
				if(m_styleTable.size() < newSize) {
					m_styleTable.resize(newSize);
				}
				m_styleTable[styleNumber] = styleName.utf8_str();
			}
			break;
		}

		// if the stylesheet is malformed there might be nothing in the table ...
		if (nesting == 1 && static_cast<UT_sint32>(m_styleTable.size()) > styleNumber )
		{
			// Reached the end of a single style definition.
			// Use it.
			buildAllProps(propBuffer,pParas,pChars,pbParas,pbChars);
			attribs[attribsCount++] = PT_PROPS_ATTRIBUTE_NAME;
			UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );
			attribs[attribsCount++] = propBuffer.c_str();
			UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );

			attribs[attribsCount++] = PT_NAME_ATTRIBUTE_NAME;
			UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );
			attribs[attribsCount++] = m_styleTable[styleNumber].c_str();
			UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );

			attribs[attribsCount++] = PT_TYPE_ATTRIBUTE_NAME;
			UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );
			attribs[attribsCount++] = styleType;
			UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );
//			attribs[attribsCount] = NULL;
//
// OK now we clone this and save it so we can set basedon's and followedby's
//
			UT_sint32 i =0;
			UT_GenericVector<const gchar*>* pVecAttr = new UT_GenericVector<const gchar*>();
			for( i= 0; i< static_cast<UT_sint32>(attribsCount); i++)
			{
				if(attribs[i] != NULL)
				{
					pVecAttr->addItem(g_strdup(attribs[i]));
				}
				else
				{
					pVecAttr->addItem(NULL);
				}
			}
			vecStyles.addItem(pVecAttr);

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
			propBuffer.clear();
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
	UT_sint32 count = vecStyles.getItemCount();
	UT_sint32 i = 0;
	for(i=0; i< count; i++)
	{
		// Reset

		attribsCount = 0;
		attribs[attribsCount] = NULL;
		UT_GenericVector<const gchar*> * pCurStyleVec = vecStyles.getNthItem(i);
		UT_sint32 nAtts = pCurStyleVec->getItemCount();
		UT_sint32 j = 0;
		const char * szName = NULL;

		while(j < nAtts)
		{
			const char * szAtt = pCurStyleVec->getNthItem(j++);
			attribs[attribsCount++] = szAtt;
			if( strcmp(szAtt, PT_NAME_ATTRIBUTE_NAME)== 0)
			{
				szName = pCurStyleVec->getNthItem(j++);
				attribs[attribsCount++] = szName;
			}
			else if( strcmp(szAtt, PT_BASEDON_ATTRIBUTE_NAME)== 0)
			{
				const char * szNext = pCurStyleVec->getNthItem(j++);
				if(NULL == szNext)
				{
					UT_sint32 istyle = BasedOn[i];
					// must not mix static and dynamically allocated strings in the same
					// array, otherwise there is no way we can g_free it !!!
					//attribs[attribsCount++] = g_strdup(static_cast<const char *>(m_styleTable[istyle]));
					attribs[attribsCount++] = m_styleTable[istyle].c_str();
					UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );
				}
				else
				{
					attribs[attribsCount++] = szNext;
					UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );
				}
			}
			else if( strcmp(szAtt, PT_FOLLOWEDBY_ATTRIBUTE_NAME)== 0)
			{
				const char * szNext = pCurStyleVec->getNthItem(j++);
				if(NULL == szNext)
				{
					UT_sint32 istyle = FollowedBy[i];
					// must not mix static and dynamically allocated strings in the same
					// array, otherwise there is no way we can g_free it !!!
					// attribs[attribsCount++] = g_strdup(static_cast<const char *>(m_styleTable[istyle]));
					attribs[attribsCount++] = m_styleTable[istyle].c_str();
					UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );
				}
				else
				{
					attribs[attribsCount++] = szNext;
					UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );
				}
			}
			else
			{
				szAtt = pCurStyleVec->getNthItem(j++);
				attribs[attribsCount++] = szAtt;
				UT_return_val_if_fail( attribsCount < PT_MAX_ATTRIBUTES * 2,false );
			}
			attribs[attribsCount] = NULL;
		}
//
// If style exists we have to redefine it like this
//
		// need to test that we have a name, as there are some malformed docs around ...
		if(szName && *szName)
		{
			xxx_UT_DEBUGMSG(("Looking at style %s \n",szName));
			PD_Style * pStyle = NULL;
			if(getDoc()->getStyle(szName, &pStyle))
			{
				if (!isPasting())
				{
					pStyle->addAttributes(attribs);
					pStyle->getBasedOn();
					pStyle->getFollowedBy();
				} 
				else
				{
					UT_DEBUGMSG(("DOM: refusing to append props to an already existing style while pasting\n"));
				}
			}
			else
			{
				getDoc()->appendStyle(attribs);
			}
		}
		
//
// OK Now delete all this allocated memory...
//
		for(j=0; j< nAtts; j++)
		{
			const gchar * sz = pCurStyleVec->getNthItem(j);
			if(sz != NULL)
			{
				// MUST NOT USED delete[] on strings allocated by g_try_malloc/UT_calloc !!!
				// delete [] sz;
				g_free(const_cast<gchar*>(sz));
				sz = NULL;
			}
		}
		delete pCurStyleVec;

	}
	status = PopRTFState();
	m_bStyleImportDone = true;
	return status;

}

/*!
 * This method builds the property list from Paragraph and character classes pParas
 * and pChars
 */
bool IE_Imp_RTF::buildAllProps(std::string &s,  RTFProps_ParaProps * pParas,
							   RTFProps_CharProps * pChars,
							   RTFProps_bParaProps * pbParas,
							   RTFProps_bCharProps * pbChars)
{
//
// Tab stops.
//
	std::string tempBuffer;
	UT_sint32 count =pParas->m_tabStops.size();
	if(count > 0)
		s += "tabstops:";
	UT_sint32 i=0;
	for (i = 0; i < count; i++)
	{
		if (i > 0)
			s += ",";

		UT_sint32 tabTwips = pParas->m_tabStops.at(i);
		double tabIn = tabTwips/(20.0*72.);
		eTabType tabType = pParas->m_tabTypes.at(i);
		eTabLeader tabLeader = pParas->m_tabLeader.at(i);
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
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		}
		char cLeader = '0' + static_cast<char>(tabLeader);
		s += UT_std_string_sprintf("%s/%c%c", UT_convertInchesToDimensionString(DIM_IN,tabIn,"04"),cType,cLeader);
	}
	if( count > 0)
		s += "; ";
//
// Top and bottom paragraph margins
//
	if(pbParas->bm_spaceBefore)
	{
		s += UT_std_string_sprintf("margin-top:%s; ",		UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(pParas->m_spaceBefore)/1440));
	}
	if(pbParas->bm_spaceAfter)
	{
		s += UT_std_string_sprintf("margin-bottom:%s; ",	UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(pParas->m_spaceAfter)/1440));
	}
//
// Left and right margins
//
	if(pbParas->bm_indentLeft)
	{
		s += UT_std_string_sprintf("margin-left:%s; ",		UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(pParas->m_indentLeft)/1440));
	}
	if(pbParas->bm_indentRight)
	{
		s += UT_std_string_sprintf("margin-right:%s; ",	UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(pParas->m_indentRight)/1440));
	}
//
// First line indent
//

	if(pbParas->bm_indentFirst)
	{
		s += UT_std_string_sprintf("text-indent:%s; ",	UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(pParas->m_indentFirst)/1440));
	}

//
// line spacing
//
	if(pbParas->bm_lineSpaceVal)
	{
		if (pParas->m_lineSpaceExact)
		{
			if (pParas->m_lineSpaceVal < 0) {  // exact spacing
				s += UT_std_string_sprintf("line-height:%spt; ",    UT_convertToDimensionlessString(fabs(pParas->m_lineSpaceVal/20.0)));
			}
			else                                                         // "at least" spacing
			{
				s += UT_std_string_sprintf("line-height:%spt+; ",    UT_convertToDimensionlessString(fabs(pParas->m_lineSpaceVal/20.0)));
			}
		}
		else   // multiple spacing
		{
			s += UT_std_string_sprintf("line-height:%s; ",	UT_convertToDimensionlessString(fabs(pParas->m_lineSpaceVal/240)));
		}
	}

//
// justification
//
	if (pbParas->bm_justification)
	{
		s += "text-align:";
		switch(pParas->m_justification)
		{
			case RTFProps_ParaProps::pjCentre:
				s += "center; ";
				break;
		    case RTFProps_ParaProps::pjRight:
			    s += "right; ";
			    break;
		    case RTFProps_ParaProps::pjFull:
			    s += "justify; ";
		     	break;
		    default:
			    UT_ASSERT_NOT_REACHED();	// so what is it?
			    // fall through
		    case RTFProps_ParaProps::pjLeft:
			    s += "left; ";
			    break;
		}
	}

//
// Character Properties.
//
	// bold
	if(pbChars->bm_bold)
	{
		s += "font-weight:";
		s += pChars->m_bold ? "bold" : "normal";
		s += ";";
	}
	// italic
	if(pbChars->bm_italic)
	{
		s += " font-style:";
		s += pChars->m_italic ? "italic" : "normal";
		s += ";";
	}
	// underline & overline & strike-out
	if(pbChars->bm_underline || pbChars->bm_strikeout || pbChars->bm_overline
	   || pbChars->bm_topline || pbChars->bm_botline )
	{
		s += "; text-decoration:";
		static std::string decors;
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
		s += decors;
		s += ";";
	}
	//superscript and subscript
	if(pbChars->bm_superscript || pbChars->bm_subscript)
	{
		s += " text-position:";
		if (pChars->m_superscript)
		{
			if (pbChars->bm_superscript_pos)
			{
				UT_DEBUGMSG (("RTF: TODO: Handle text position in pt.\n"));
			}
			s += "superscript;";
		}
		else if (pChars->m_subscript)
		{
			if (pbChars->bm_subscript_pos)
			{
				UT_DEBUGMSG (("RTF: TODO: Handle text position in pt.\n"));
			}
			s += "subscript;";
		}
		else
		{
			s += "normal;";
		}
	}

	// font size
	if(pbChars->bm_fontSize)
	{
		s += UT_std_string_sprintf(" font-size:%spt;", std_size_string(static_cast<float>(pChars->m_fontSize)));
	}
	// typeface
	if(pbChars->bm_fontNumber)
	{
		RTFFontTableItem* pFont = GetNthTableFont(pChars->m_fontNumber);
		if (pFont != NULL)
		{
			s += " font-family:";
			s += pFont->m_pFontName;
			s += ";";
		}
	}
	// Foreground Colour
	if(pbChars->bm_hasColour)
	{
		if (pChars->m_hasColour)
		{
			// colour, only if one has been set. See bug 1324
			UT_uint32 colour = GetNthTableColour(pChars->m_colourNumber);
			s += UT_std_string_sprintf(" color:%06x;", colour);
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
				s += UT_std_string_sprintf(" bgcolor:%06x;", bgColour);
			}
		}
	}
// Language
	if (pChars->m_szLang)
	{
		s += " lang:";
		s += pChars->m_szLang;
		s += ";";
	}
// List Tag to hang lists off
	if(pbChars->bm_listTag)
	{
		s += UT_std_string_sprintf(" list-tag:%d; ",pChars->m_listTag);
	}
//
// Now remove any trailing ";"'s
//
	UT_sint32 eol = s.length() - 1;
	while(eol >= 0 && (s[eol] == ' ' || s[eol] == 0))
	{
		eol--;
	}
	if(eol >= 0 && s[eol] == ';')
	{
		s[eol] = 0;
	}
	return true;
}


/*!
  Handle document meta data
 */
bool IE_Imp_RTF::HandleInfoMetaData()
{
	RTF_KEYWORD_ID keywordID;
	RTFTokenType tokenType;
	unsigned char keyword[MAX_KEYWORD_LEN];
	UT_sint32 parameter = 0;
	bool paramUsed = false;	
	int nested = 0;
	//bool result;
	const char * metaDataKey = NULL;
	std::string metaDataProp;
	enum {
		ACT_NONE,
		ACT_PCDATA,
		ACT_DATETIME
	} action = ACT_NONE;

	// Since the metadata group is enclosed between brackets, a new RTF state was generated. Remove it now.
	// The closing bracket will be ignored.
	PopRTFState();
	do {
		tokenType = NextToken (keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN,false);
		switch (tokenType) {
		case RTF_TOKEN_ERROR:
			UT_ASSERT_NOT_REACHED();
			return false;
			break;
		case RTF_TOKEN_KEYWORD:			
			keywordID = KeywordToID(reinterpret_cast<char *>(keyword));
			
			switch(keywordID) {
			case RTF_KW_title:
				metaDataKey = PD_META_KEY_TITLE;
				action = ACT_PCDATA;
				break;
			case RTF_KW_subject:
				metaDataKey = PD_META_KEY_SUBJECT;
				action = ACT_PCDATA;
				break;
			case RTF_KW_author:
				metaDataKey = PD_META_KEY_CREATOR;
				action = ACT_PCDATA;
				break;
			case RTF_KW_manager:
				metaDataKey = PD_META_KEY_PUBLISHER;
				action = ACT_PCDATA;
				break;
			case RTF_KW_company:
				/*result =*/ SkipCurrentGroup();
				action = ACT_NONE;
				break;
			case RTF_KW_operator:
				/*result =*/ SkipCurrentGroup();
				action = ACT_NONE;
				break;
			case RTF_KW_keywords:
				metaDataKey = PD_META_KEY_KEYWORDS;
				action = ACT_PCDATA;
				break;
			case RTF_KW_comment:
				/*result =*/ SkipCurrentGroup();
				action = ACT_NONE;
				break;
			case RTF_KW_doccomm:
				metaDataKey = PD_META_KEY_DESCRIPTION;
				action = ACT_PCDATA;
				break;		
			case RTF_KW_hlinkbase:
				/*result =*/ SkipCurrentGroup();
				action = ACT_NONE;
				break;
			case RTF_KW_creatim:
				metaDataKey = PD_META_KEY_DATE;
				action = ACT_NONE;
				break;
			case RTF_KW_revtim:
				metaDataKey = PD_META_KEY_DATE_LAST_CHANGED;
				action = ACT_DATETIME;
				break;
			case RTF_KW_printim:
				/*result =*/ SkipCurrentGroup();
				action = ACT_NONE;
				break;
			case RTF_KW_buptim:
				/*result =*/ SkipCurrentGroup();
				action = ACT_NONE;
				break;
			default:
				/*result =*/ SkipCurrentGroup();
				action = ACT_NONE;
			}
			if (action == ACT_PCDATA) {
				metaDataProp = "";
				/*result =*/ HandlePCData(metaDataProp);
			}
			else if (action == ACT_DATETIME) {
				/*result =*/ SkipCurrentGroup();
				action = ACT_NONE;
				//result = HandlMetaDataTime(&metaTime);
			}
			// if any action needs to be done
			if (action !=  ACT_NONE) {
				getDoc()->setMetaDataProp(metaDataKey, metaDataProp);
			}
			break;
		case RTF_TOKEN_OPEN_BRACE:
			nested++;
			break;
		case RTF_TOKEN_CLOSE_BRACE:
			nested--;
			break;
		case RTF_TOKEN_DATA:
			// Ignore data because we don't know what to do with it
			break;
		default:
			break;
		}
	} while ((tokenType != RTF_TOKEN_CLOSE_BRACE) || (nested >= 0));
	return true;
}

bool IE_Imp_RTF::HandlePCData(std::string& str)
{
	UT_UTF8String t;
	bool ret = HandlePCData(t);
	str = t.utf8_str();
	return ret;
}

/* 
 * TODO:
 * This reads PCData until it reaches a terminating '}'.
 * It assumes that a PCData block will not contain sub-groups. This may well be
 * fine but we should check the spec. If there is a sub-group then it's closing
 * brace will close the PCData block.
 *
 * Also we assume that the \uc keyword cannot appear.
 *
 * In general, I think the handling of keywords within this function is
 * extremely dangerous. They should be pushed back onto the stream and left for
 * the caller to deal with. Otherwise, a "\b" in the header would cause the
 * start of the document to be bold as well, for example.
 */
bool IE_Imp_RTF::HandlePCData(UT_UTF8String & str)
{
	RTF_KEYWORD_ID keywordID;
	RTFTokenType tokenType;
	unsigned char keyword[MAX_KEYWORD_LEN];
	UT_sint32 parameter = 0;
	bool paramUsed = false;	
	bool bStop = false;
	UT_ByteBuf buf;
	UT_sint32 iUniCharsLeftToSkip = 0;
	
	do {
		tokenType = NextToken (keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN, false);
		switch (tokenType) {
		case RTF_TOKEN_KEYWORD:			
			keywordID = KeywordToID(reinterpret_cast<char *>(keyword));
			switch(keywordID)
			{
			case RTF_KW_QUOTE:
			{
				UT_UCS4Char wc;
				UT_Byte ch;
				wc = ReadHexChar();
				// here we assume that the read char fit on ONE byte. Should be correct.
				ch = static_cast<UT_Byte>(wc);
				buf.append(&ch, 1);
				break;
			}
			case RTF_KW_u:
			{
				UT_UCS2Char ch = 0;
				/* RTF is limited to +/-32K ints so we need to use negative
				 * numbers for large unicode values. So, check for Unicode chars
				 * wrapped to negative values.
		 		 */
				if (parameter < 0)
				{
					unsigned short tmp = (unsigned short) ((signed short) parameter);
					parameter = (UT_sint32) tmp;
				}
				ch = parameter;

				// First flush any data in the byte buffer to str. Then append
				// the unicode char.
				str.appendBuf(buf, m_mbtowc);
				buf.truncate(0);
				str.appendUCS2(&ch, 1);

				// Make sure we skip alternative chars.
				iUniCharsLeftToSkip = m_currentRTFState.m_unicodeAlternateSkipCount;
				break;
			}
			case RTF_KW_uc:
				// A little bit evil, but I'd like to know if this happens! - R.Kay
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			default:
				bStop = true; // regular keyword stop reading data and handle it
				break;
			}
			break;
		case RTF_TOKEN_DATA:
			// Don't append data if we're skipping a unicode alternative.
			if (iUniCharsLeftToSkip > 0)
				iUniCharsLeftToSkip--;
			else
				buf.append(keyword, 1);
			break;
		case RTF_TOKEN_ERROR:
			// force close brace to exit loop
			tokenType = RTF_TOKEN_CLOSE_BRACE;
			break;
		case RTF_TOKEN_OPEN_BRACE:
			// A little bit evil, but I'd like to know if this happens! - R.Kay
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			break;
		case RTF_TOKEN_CLOSE_BRACE:
			SkipBackChar('}');
			break;
		default:
			UT_DEBUGMSG(("Unknown token !!!!!!!!!!!\n"));
			break;
		}
	}
	while ((tokenType != RTF_TOKEN_CLOSE_BRACE) && !bStop);

	/*
	 * TODO: Think about how much sense handling keywords here makes.
	 * What keywords are legal and where should the changes they make appear?
	 * (E.g. in the document, in the header) It probably depends on where this
	 * function is called.
	 */
	str.appendBuf(buf, m_mbtowc);
	if(bStop)
	{
		//
		// Have to insert the data before we process the keyword
		//
		const char * sz = str.utf8_str();
		while(*sz)
		{
			ParseChar(*sz);
			sz++;
		}
		keywordID = KeywordToID(reinterpret_cast<char *>(keyword));
		TranslateKeywordID(keywordID, parameter, paramUsed);
		str.clear();
	}

	return true;
}

/*
 * Activates the appropriate encoding. This will be the document default, set by
 * \ansicpg, unless overridden by the current font.
 */
void IE_Imp_RTF::setEncoding() { 
	RTFFontTableItem *pFont; 

	// Activate the current encoding.
	pFont = GetNthTableFont(m_currentRTFState.m_charProps.m_fontNumber);
	if (pFont != NULL && pFont->m_szEncoding) {
		m_mbtowc.setInCharset(pFont->m_szEncoding);
	}
	else if (m_szDefaultEncoding != NULL) {
		m_mbtowc.setInCharset(m_szDefaultEncoding);
	}
}


