/* AbiWord
 * Copyright (C) 1998-2001 AbiSource, Inc.
 * Copyright (C) 2001 Dom Lachowicz <dominicl@seas.upenn.edu>
 * Copyright (C) 2001-2003 Tomas Frydrych
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
#include <locale.h>

#include "wv.h"

#include "ut_string_class.h"
#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ut_units.h"
#include "ut_math.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_EncodingManager.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_Password.h"

#include "fg_Graphic.h"
#include "fg_GraphicRaster.h"
#include "fg_GraphicVector.h"

#include "pd_Document.h"

#include "ie_imp_MsWord_97.h"
#include "ie_impGraphic.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"

#include "pf_Frag_Strux.h"
#include "pt_PieceTable.h"
#include "pd_Style.h"

#include "ut_Language.h"

#ifdef DEBUG
#define IE_IMP_MSWORD_DUMP
#include "ie_imp_MsWord_dump.h"
#undef IE_IMP_MSWORD_DUMP
#endif

#define X_CheckError(v) 		do { if (!(v)) return 1; } while (0)

// undef this to disable support for older images (<= Word95)
#define SUPPORTS_OLD_IMAGES 1

#include <fribidi/fribidi.h>

//#define BIDI_DEBUG
//
// Forward decls. to wv's callbacks
//
static int charProc (wvParseStruct *ps, U16 eachchar, U8 chartype, U16 lid);
static int specCharProc (wvParseStruct *ps, U16 eachchar, CHP* achp);
static int eleProc (wvParseStruct *ps, wvTag tag, void *props, int dirty);
static int docProc (wvParseStruct *ps, wvTag tag);

//
// DOC uses an unsigned int color index
//
typedef UT_uint32 Doc_Color_t;

//
// A mapping between Word's colors and Abi's RGB color scheme
//
static Doc_Color_t word_colors [][3] = {
	{0x00, 0x00, 0x00}, /* black */
	{0x00, 0x00, 0xff}, /* blue */
	{0x00, 0xff, 0xff}, /* cyan */
	{0x00, 0xff, 0x00}, /* green */
	{0xff, 0x00, 0xff}, /* magenta */
	{0xff, 0x00, 0x00}, /* red */
	{0xff, 0xff, 0x00}, /* yellow */
	{0xff, 0xff, 0xff}, /* white */
	{0x00, 0x00, 0x80}, /* dark blue */
	{0x00, 0x80, 0x80}, /* dark cyan */
	{0x00, 0x80, 0x00}, /* dark green */
	{0x80, 0x00, 0x80}, /* dark magenta */
	{0x80, 0x00, 0x00}, /* dark red */
	{0x80, 0x80, 0x00}, /* dark yellow */
	{0x80, 0x80, 0x80}, /* dark gray */
	{0xc0, 0xc0, 0xc0}, /* light gray */
};

static UT_String sMapIcoToColor (UT_uint16 ico)
{
  return UT_String_sprintf("%02x%02x%02x",
			   word_colors[ico-1][0],
			   word_colors[ico-1][1],
			   word_colors[ico-1][2]);
}

//
// Field Ids that are useful later for mapping
//
typedef enum {
	F_TIME,
	F_DATE,
	F_EDITTIME,
	F_AUTHOR,
	F_PAGE,
	F_NUMCHARS,
	F_NUMPAGES,
	F_NUMWORDS,
	F_FILENAME,
	F_HYPERLINK,
	F_PAGEREF,
	F_EMBED,
	F_TOC,
	F_DateTimePicture,
	F_TOC_FROM_RANGE,
	F_DATEINAME,
	F_SPEICHERDAT,
	F_OTHER
} Doc_Field_t;

//
// A mapping between DOC's field names and our given IDs
//
typedef struct
{
	char * m_name;
	Doc_Field_t m_id;
} Doc_Field_Mapping_t;

/*
 * This next bit of code enables us to import many of Word's fields
 */

static Doc_Field_Mapping_t s_Tokens[] =
{
	{"TIME",	   F_TIME},
	{"EDITTIME",   F_EDITTIME},
	{"DATE",	   F_DATE},
	{"date",	   F_DATE},
	{"DATEINAME",      F_DATE}, // F_DATEINAME
	{"SPEICHERDAT",    F_DATE}, // F_SPEICHERDAT
	{"\\@", 	   F_DateTimePicture},

	{"FILENAME",   F_FILENAME},
	{"\\filename", F_FILENAME},
	{"PAGE",	   F_PAGE},
	{"NUMCHARS",   F_NUMCHARS},
	{"NUMWORDS",   F_NUMWORDS},

	// these below aren't handled by AbiWord, but they're known about
	{"HYPERLINK",  F_HYPERLINK},
	{"PAGEREF",    F_PAGEREF},
	{"EMBED",	   F_EMBED},
	{"TOC", 	   F_TOC},
	{"\\o", 	   F_TOC_FROM_RANGE},
	{"AUTHOR",	   F_AUTHOR},

	{ "*",		   F_OTHER}
};

#define FieldMappingSize (sizeof(s_Tokens)/sizeof(s_Tokens[0]))

static Doc_Field_t
s_mapNameToField (const char * name)
{
	for (unsigned int k = 0; k < FieldMappingSize; k++)
	{
		if (!UT_strcmp(s_Tokens[k].m_name,name))
			return s_Tokens[k].m_id;
	}
	return F_OTHER;
}

#undef FieldMappingSize

static const char *
s_mapPageIdToString (UT_uint16 id)
{
	// TODO: make me way better when we determine code names

	switch (id)
	{
	case 0:
		return "Letter";
	case 9:
		return "A4";

	default:
		return 0;
	}
}

/*!
  Surprise, surprise, there are more list numerical formats than the 5 the
  MS documentation states happens to mention, so here I will put what I found
  out (latter we will move it to some better place)
*/
typedef enum
{
  WLNF_INVALID		   = -1,
  WLNF_EUROPEAN_ARABIC = 0,
  WLNF_UPPER_ROMAN	   = 1,
  WLNF_LOWER_ROMAN	   = 2,
  WLNF_UPPER_LETTER    = 3,
  WLNF_LOWER_LETTER    = 4,
  WLNF_ORDINAL		   = 5,
  WLNF_BULLETS		   = 23,
  WLNF_HEBREW_NUMBERS  = 45
} MSWordListIdType;

typedef struct{
  UT_uint32 listId;
  UT_uint32 level;
} ListIdLevelPair;

/*!
 * Map msword list enums back to abi's
 */
static const char *
s_mapDocToAbiListId (MSWordListIdType id)
{
  switch (id)
	{
	case WLNF_UPPER_ROMAN: // upper roman
	  return "4";

	case WLNF_LOWER_ROMAN: // lower roman
	  return "3";

	case WLNF_UPPER_LETTER: // upper letter
	  return "2";

	case WLNF_LOWER_LETTER: // lower letter
	  return "1";

	case WLNF_BULLETS: // bullet list
	  return "5";

	case WLNF_HEBREW_NUMBERS:
	  return "129";

	case WLNF_ORDINAL: // ordinal
	default:
	  return "0";
	}
}

/*!
 * Map msword list enum back to an abiword list delimiter tag
 */
static const char *
s_mapDocToAbiListDelim (MSWordListIdType id)
{
  switch (id)
	{
	case WLNF_UPPER_ROMAN: // upper roman
	  return "%L.";

	case WLNF_LOWER_ROMAN: // lower roman
	  return "%L.";

	case WLNF_UPPER_LETTER: // upper letter
	  return "%L)";

	case WLNF_LOWER_LETTER: // lower letter
	  return "%L)";

	case WLNF_BULLETS: // bullet list
	  return "%L";

	case WLNF_ORDINAL: // ordinal

	case WLNF_HEBREW_NUMBERS:

	default:
	  return "%L.";
	}
}

/*!
 * Map msword list enums back to abi's list styles
 */
static const char *
s_mapDocToAbiListStyle (MSWordListIdType id)
{
  switch (id)
	{
	case WLNF_UPPER_ROMAN: // upper roman
	  return "Upper Roman List";

	case WLNF_LOWER_ROMAN: // lower roman
	  return "Lower Roman List";

	case WLNF_UPPER_LETTER: // upper letter
	  return "Upper Case List";

	case WLNF_LOWER_LETTER: // lower letter
	  return "Lower Case List";

	case WLNF_BULLETS: // bullet list
	  return "Bullet List";

	case WLNF_ORDINAL: // ordinal
	default:
	  return "Numbered List";
	}
}

/*!
 * Map msword list enums back to abi's field font for that given style
 */
static const char *
s_fieldFontForListStyle (MSWordListIdType id)
{
  switch (id)
	{
	case WLNF_UPPER_ROMAN: // upper roman
	  return "NULL";

	case WLNF_LOWER_ROMAN: // lower roman
	  return "NULL";

	case WLNF_UPPER_LETTER: // upper letter
	  return "Times New Roman";

	case WLNF_LOWER_LETTER: // lower letter
	  return "Times New Roman";

	case WLNF_BULLETS: // bullet list
	  return "Symbol";

	case WLNF_ORDINAL: // ordinal
	default:
	  return "Times New Roman";
	}
}

// MS Word uses the langauge codes as explicit overrides when treating
// weak characters; this function translates language id to the
// overrided direction
static bool s_isLanguageRTL(short unsigned int lid)
{
	const char * s = wvLIDToLangConverter (lid);
	UT_Language l;
	return (UTLANG_RTL == l.getOrderFromProperty(s));
}
/****************************************************************************/
/****************************************************************************/

IE_Imp_MsWord_97_Sniffer::IE_Imp_MsWord_97_Sniffer ()
	: IE_ImpSniffer(IE_IMPEXPNAME_MSWORD97)
{
	//
}

UT_Confidence_t IE_Imp_MsWord_97_Sniffer::supportsMIME (const char * szMIME)
{
	if (UT_strcmp (IE_FileInfo::mapAlias (szMIME), IE_MIME_MSWord) == 0)
		{
			return UT_CONFIDENCE_GOOD;
		}
	return UT_CONFIDENCE_ZILCH;
}

UT_Confidence_t IE_Imp_MsWord_97_Sniffer::recognizeContents (const char * szBuf,
												  UT_uint32 iNumbytes)
{
	char * magic	= 0;
	int magicoffset = 0;

	magic = "Microsoft Word 6.0 Document";
	magicoffset = 2080;
	if (iNumbytes > (magicoffset + strlen (magic)))
	{
		if (!strncmp (szBuf + magicoffset, magic, strlen (magic)))
		{
			return UT_CONFIDENCE_PERFECT;
		}
	}

	magic = "Documento Microsoft Word 6";
	magicoffset = 2080;
	if (iNumbytes > (magicoffset + strlen (magic)))
	{
		if (!strncmp(szBuf + magicoffset, magic, strlen (magic)))
		{
			return UT_CONFIDENCE_PERFECT;
		}
	}

	magic = "MSWordDoc";
	magicoffset = 2112;
	if (iNumbytes > (magicoffset + strlen (magic)))
	{
		if (!strncmp (szBuf + magicoffset, magic, strlen (magic)))
		{
			return UT_CONFIDENCE_PERFECT;
		}
	}

	// ok, that didn't work, we'll try to dig through the OLE stream
	if (iNumbytes > 8)
	{
	        // this code is too generic - also picks up .wri documents
		if (szBuf[0] == static_cast<char>(0x31)
			&& static_cast< unsigned char>(szBuf[1]) == static_cast< unsigned char>(0xbe)
			&&  szBuf[2] == static_cast<char>(0)
			&& szBuf[3] == static_cast<char>(0))
		{
		  return UT_CONFIDENCE_SOSO; //POOR
		}

		// this identifies staroffice dox as well
		if (static_cast< unsigned char>(szBuf[0]) == static_cast<unsigned char>(0xd0)
			&& static_cast< unsigned char>(szBuf[1]) == static_cast<unsigned char>(0xcf)
			&& szBuf[2] == static_cast<char>(0x11)
			&& static_cast< unsigned char>(szBuf[3]) == static_cast<unsigned char>(0xe0)
			&& static_cast< unsigned char>(szBuf[4]) == static_cast<unsigned char>(0xa1)
			&& static_cast< unsigned char>(szBuf[5]) == static_cast<unsigned char>(0xb1)
			&& szBuf[6] == static_cast<char>(0x1a)
			&& static_cast< unsigned char>(szBuf[7]) == static_cast<unsigned char>(0xe1))
		{
		  return UT_CONFIDENCE_SOSO; // POOR
		}

		if (szBuf[0] == 'P' && szBuf[1] == 'O' &&
			szBuf[2] == '^' && szBuf[3] == 'Q' && szBuf[4] == '`')
		{
			return UT_CONFIDENCE_POOR;
		}
		if (static_cast< unsigned char>(szBuf[0]) == static_cast<unsigned char>(0xfe)
			&& szBuf[1] == static_cast<char>(0x37)
			&& szBuf[2] == static_cast<char>(0)
			&& szBuf[3] == static_cast<char>(0x23))
		{
			return UT_CONFIDENCE_POOR;
		}

		if (static_cast< unsigned char>(szBuf[0]) == static_cast<unsigned char>(0xdb)
			&& static_cast< unsigned char>(szBuf[1]) == static_cast<unsigned char>(0xa5)
			&& szBuf[2] == static_cast<char>(0x2d)
			&& szBuf[3] == static_cast<char>(0)
			&& szBuf[4] == static_cast<char>(0) && szBuf[5] == static_cast<char>(0))
		{
			return UT_CONFIDENCE_POOR;
		}
	}
	return UT_CONFIDENCE_ZILCH;
}

UT_Confidence_t IE_Imp_MsWord_97_Sniffer::recognizeSuffix (const char * szSuffix)
{
	// We recognize both word documents and their template versions
	if (!UT_stricmp(szSuffix,".doc") ||
			!UT_stricmp(szSuffix,".dot"))
	  return UT_CONFIDENCE_PERFECT;
	return UT_CONFIDENCE_ZILCH;
}

UT_Error IE_Imp_MsWord_97_Sniffer::constructImporter (PD_Document * pDocument,
													  IE_Imp ** ppie)
{
	IE_Imp_MsWord_97 * p = new IE_Imp_MsWord_97(pDocument);
	*ppie = p;
	return UT_OK;
}

bool	IE_Imp_MsWord_97_Sniffer::getDlgLabels (const char ** pszDesc,
												const char ** pszSuffixList,
												IEFileType * ft)
{
	*pszDesc = "Microsoft Word (.doc, .dot)";
	*pszSuffixList = "*.doc; *.dot";
	*ft = getFileType();
	return true;
}

/****************************************************************************/
/****************************************************************************/

// just buffer sizes, arbitrarily chosen
#define DOC_TEXTRUN_SIZE 2048
#define DOC_PROPBUFFER_SIZE 1024

IE_Imp_MsWord_97::~IE_Imp_MsWord_97()
{
	if(m_pBookmarks)
	{
		// free the names from the bookmarks
		for(UT_uint32 i = 0; i < m_iBookmarksCount; i++)
		{
			// make sure we do not delete any name twice
			if(m_pBookmarks[i].name && m_pBookmarks[i].start)
			{
			   delete[] m_pBookmarks[i].name;
			   m_pBookmarks[i].name = NULL;
			}
		}
		delete [] m_pBookmarks;
	}

	UT_VECTOR_PURGEALL(ListIdLevelPair *, m_vLists);
	UT_VECTOR_PURGEALL(emObject *, m_vecEmObjects);
}

IE_Imp_MsWord_97::IE_Imp_MsWord_97(PD_Document * pDocument)
  : IE_Imp (pDocument),
	m_iImageCount (0),
	m_nSections(0),
	m_bSetPageSize(false),
	m_fieldWhich(NULL),
	m_fieldI(0),
	m_fieldDepth(0),
	m_fieldRet(0),
	m_fieldC(NULL),
	//m_fieldA(NULL),
	m_bIsLower(false),
	m_bInSect(false),
	m_bInPara(false),
	m_bLTRCharContext(true),
	m_bLTRParaContext(true),
	m_iOverrideIssued(FRIBIDI_TYPE_UNSET),
	m_bBidiDocument(false),
	m_iDocPosition(0),
	m_pBookmarks(NULL),
	m_iBookmarksCount(0),
	m_iMSWordListId(0),
    m_bEncounteredRevision(false),
    m_bInTable(false),
	m_iRowsRemaining(0),
    m_iCellsRemaining(0),
    m_iCurrentRow(0),
    m_iCurrentCell(0),
    m_bRowOpen(false),
    m_bCellOpen(false)
{
  for(UT_uint32 i = 0; i < 9; i++)
	  m_iListIdIncrement[i] = 0;
}

/****************************************************************************/
/****************************************************************************/

#define ErrCleanupAndExit(code)  do {wvOLEFree (&ps); return(code);} while(0)

#define GetPassword() _getPassword ( getDoc()->getApp()->getLastFocussedFrame() )

#define ErrorMessage(x) do { XAP_Frame *_pFrame = getDoc()->getApp()->getLastFocussedFrame(); if ( _pFrame ) _errorMessage (_pFrame, (x)); } while (0)

static UT_String _getPassword (XAP_Frame * pFrame)
{
  UT_String password ( "" );

  if ( pFrame )
    {
      pFrame->raise ();

      XAP_DialogFactory * pDialogFactory
	= (XAP_DialogFactory *)(pFrame->getDialogFactory());

      XAP_Dialog_Password * pDlg = static_cast<XAP_Dialog_Password*>(pDialogFactory->requestDialog(XAP_DIALOG_ID_PASSWORD));
      UT_return_val_if_fail(pDlg, password);

      pDlg->runModal (pFrame);

      XAP_Dialog_Password::tAnswer ans = pDlg->getAnswer();
      bool bOK = (ans == XAP_Dialog_Password::a_OK);

      if (bOK)
	password = pDlg->getPassword ();

      UT_DEBUGMSG(("Password is %s\n", password.c_str()));

      pDialogFactory->releaseDialog(pDlg);
    }

  return password;
}

static void _errorMessage (XAP_Frame * pFrame, int id)
{
  UT_return_if_fail(pFrame);

  const XAP_StringSet * pSS = XAP_App::getApp ()->getStringSet ();

  const char * text = pSS->getValue (id, pFrame->getApp()->getDefaultEncoding()).c_str();

  pFrame->showMessageBox (text,
			  XAP_Dialog_MessageBox::b_O,
			  XAP_Dialog_MessageBox::a_OK);
}

UT_Error IE_Imp_MsWord_97::importFile(const char * szFilename)
{
  wvParseStruct ps;

  int ret = wvInitParser (&ps, const_cast<char *>(szFilename));
  const char * password = NULL;

  // HACK!!
  bool decrypted = false ;

  if (ret & 0x8000)		/* Password protected? */
    {
      UT_String pass = GetPassword();
      if ( pass.size () != 0 )
	password = pass.c_str();

      if ((ret & 0x7fff) == WORD8)
	{
	  ret = 0;
	  if (password == NULL)
	    {
	      ErrorMessage(AP_STRING_ID_WORD_PassRequired);
	      ErrCleanupAndExit(UT_IE_PROTECTED);
	    }
	  else
	    {
	      wvSetPassword (password, &ps);
	      if (wvDecrypt97 (&ps))
		{
		  ErrorMessage(AP_STRING_ID_WORD_PassInvalid);
		  ErrCleanupAndExit(UT_IE_PROTECTED);
		}
	      decrypted = true ;
	    }
	}
      else if (((ret & 0x7fff) == WORD7) || ((ret & 0x7fff) == WORD6))
	{
	  ret = 0;
	  if (password == NULL)
	    {
	      ErrorMessage(AP_STRING_ID_WORD_PassRequired);
	      ErrCleanupAndExit(UT_IE_PROTECTED);
	    }
	  else
	    {
	      wvSetPassword (password, &ps);
	      if (wvDecrypt95 (&ps))
		{
		  //("Incorrect Password\n"));
		  ErrCleanupAndExit(UT_IE_PROTECTED);
		}
	      decrypted = true ;
	    }
	}
    }

  if (ret)
    ErrCleanupAndExit(UT_IE_BOGUSDOCUMENT);

  // register ourself as the userData
  ps.userData = this;

  // register callbacks
  wvSetElementHandler (&ps, eleProc);
  wvSetCharHandler (&ps, charProc);
  wvSetSpecialCharHandler(&ps, specCharProc);
  wvSetDocumentHandler (&ps, docProc);

  UT_DEBUGMSG(("DOM: wvText\n"));

  wvText(&ps);

  UT_DEBUGMSG(("DOM: about to get summary information\n"));

  // now get the summary information, if available
  ret = wvQuerySupported (&ps.fib, NULL);

  // word 2 used an OLE like mechanism inside of a FILE*, but
  // good luck trying to ms_ole_summary_open something using that...
  if (WORD2 != ret)
    {
      MsOleSummary *summary = ms_ole_summary_open (ps.ole_file);
      if (summary)
	{
	  UT_DEBUGMSG(("DOM: getting summary information\n"));

	  UT_UTF8String prop_str;
	  gboolean found = FALSE;

	  // title
	  prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_TITLE, &found);
	  if (found && prop_str.size())
	    getDoc()->setMetaDataProp ( PD_META_KEY_TITLE, prop_str ) ;

	  // subject
	  prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_SUBJECT, &found);
	  if (found && prop_str.size())
	    getDoc()->setMetaDataProp ( PD_META_KEY_SUBJECT, prop_str ) ;

	  // author
	  prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_AUTHOR, &found);
	  if (found && prop_str.size())
	    getDoc()->setMetaDataProp ( PD_META_KEY_CREATOR, prop_str ) ;

	  prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_LASTAUTHOR, &found);
	  if (found && prop_str.size())
	    getDoc()->setMetaDataProp ( PD_META_KEY_CONTRIBUTOR, prop_str ) ;

	  // keywords
	  prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_KEYWORDS, &found);
	  if (found && prop_str.size())
	    getDoc()->setMetaDataProp ( PD_META_KEY_KEYWORDS, prop_str ) ;

	  // comments
	  prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_COMMENTS, &found);
	  if (found && prop_str.size())
	    getDoc()->setMetaDataProp ( PD_META_KEY_DESCRIPTION, prop_str ) ;

	  // below this line are from Document Summary Information

	  ms_ole_summary_close (summary);
	  summary = ms_ole_docsummary_open(ps.ole_file);

	  if(summary){
	    // category
	    prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_CATEGORY, &found);
	    if (found && prop_str.size())
	      getDoc()->setMetaDataProp ( PD_META_KEY_TYPE, prop_str ) ;

	    // organization
	    prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_COMPANY, &found);
	    if (found && prop_str.size())
	      getDoc()->setMetaDataProp ( PD_META_KEY_PUBLISHER, prop_str ) ;

	    ms_ole_summary_close (summary);
	  }
	}
    }

  UT_DEBUGMSG(("DOM: finished summary info\n"));

  // HACK - this will do until i sort out some global stream ugliness in wv
  if ( !decrypted && WORD2 != ret )
    wvOLEFree(&ps);

  // We can't be in a good state if we didn't add any sections!
  if (m_nSections == 0)
    return UT_IE_BOGUSDOCUMENT;

  return UT_OK;
}

void IE_Imp_MsWord_97::_flush ()
{
  if(!m_pTextRun.size())
	return;

  // we've got to ensure that we're inside of a section & paragraph
  if (!m_bInSect)
	{
	  // append a blank default section - assume it works
	  UT_DEBUGMSG(("#TF: _flush: appending default section\n"));
	  getDoc()->appendStrux(PTX_Section, NULL);
	  m_bInSect = true;
	  m_nSections++;
	}

  if(!m_bInPara)
  {
	  // append a blank defaul paragraph - assume it works
	  UT_DEBUGMSG(("#TF: _flush: appending default block\n"));
	  getDoc()->appendStrux(PTX_Block, NULL);
	  m_bInPara = true;
	  emObject * pObject = NULL;
	  if(m_vecEmObjects.getItemCount() > 0)
	  {
		  UT_sint32 i =0;
		  for(i=0;i< static_cast<UT_sint32>(m_vecEmObjects.getItemCount()); i++)
		  {
			  pObject = static_cast<emObject *>(m_vecEmObjects.getNthItem(i));
			  const XML_Char* propsArray[5];
			  if(pObject->objType == PTO_Bookmark)
			  {
				  propsArray[0] = static_cast<const XML_Char *>("name");
				  propsArray[1] = static_cast<const XML_Char *>(pObject->props1.c_str());
				  propsArray[2] = static_cast<const XML_Char *>("type");
				  propsArray[3] = static_cast<const XML_Char *>(pObject->props2.c_str());
				  propsArray[4] = static_cast<const XML_Char *>(NULL);
				  getDoc()->appendObject (PTO_Bookmark, propsArray);
			  }
			  else
			  {
				  UT_DEBUGMSG(("MSWord 97 _flush: Object not handled \n"));
				  UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			  }
			  delete pObject;
		  }
		  m_vecEmObjects.clear();
	  }
  }

  if (m_pTextRun.size())
  {
	  // bidi adjustments for neutrals
	  // 
	  // We have a problem in bidi documents caused by the fact that
	  // Word does not use the Unicode bidi algorithm, but rather one of
	  // its own, which adds keyboard language to the equation. We get
	  // around this by issuing an explicit direction override on the
	  // neutral characters. We do it here in the _flush() function
	  // because when we have both left and right context available
	  // for these characters we can tell if the override is
	  // superfluous, which it is most of the time; omitting the
	  // sufperfluous overrides allows us to import documents in a
	  // manner that will make them feel more like native AW docs.
	  // (This does not get rid of all the unnecessary overrides, for
	  // that we would need to have the text of an entire paragraph)
	  //
	  // I goes without saying that it would be highly desirable to be
	  // able to determine at the start if a document is pure LTR (as
	  // we do in the RTF importer), since that would save us lot of
	  // extra processing
	  // Tomas, May 8, 2003
	  
	  if(m_bBidiDocument)
	  {
		  const XML_Char* pProps = "props";
		  UT_String prop_basic = m_charProps;

		  UT_String prop_ltr = prop_basic;
		  UT_String prop_rtl = prop_basic;

		  if(prop_basic.size())
		  {
			  prop_ltr += ";";
			  prop_rtl += ";";
		  }
		  else
		  {
			  // if the char props are empty, we need replace them
			  // with the following to avoid asserts in PP_AttrProp
			  prop_basic = "dir-override:";
		  }
		  
		  
		  prop_ltr += "dir-override:ltr";
		  prop_rtl += "dir-override:rtl";
	
		  const XML_Char rev[] ="revision";

		  const XML_Char* propsArray[5];
		  propsArray[0] = pProps;
		  propsArray[1] = prop_basic.c_str();
		  propsArray[2] = NULL;
		  propsArray[3] = NULL;
		  propsArray[4] = NULL;

		  UT_uint32 iEmptyAttrib = 2;

		  if(m_charRevs.size())
		  {
			  propsArray[iEmptyAttrib++] = &rev[0];
			  propsArray[iEmptyAttrib++] = m_charRevs.c_str();
		  }
		  
		  const UT_UCS4Char * p;
		  const UT_UCS4Char * pStart = m_pTextRun.ucs4_str();
		  UT_uint32 iLen = m_pTextRun.size();
		  
		  FriBidiCharType iOverride = FRIBIDI_TYPE_UNSET, cType, cLastType = FRIBIDI_TYPE_UNSET, cNextType;
		  UT_uint32 iLast = 0;
		  UT_UCS4Char c = *pStart;
	
		  cType = fribidi_get_type(c);
	
		  for(UT_uint32 i = 0; i < iLen; i++)
		  {
			  if(i < iLen - 1 )
			  {
				  c = *(pStart+i+1);
				  cNextType = fribidi_get_type(c);
			  }
			  else
			  {
				  cNextType = FRIBIDI_TYPE_UNSET;
			  }
		
		
			  if(FRIBIDI_IS_NEUTRAL(cType))
			  {
				  if(m_bLTRCharContext
					 && iOverride != FRIBIDI_TYPE_LTR
					 && (cLastType != FRIBIDI_TYPE_LTR || cNextType != FRIBIDI_TYPE_LTR))
				  {
					  if(i - iLast > 0)
					  {
						  p = pStart + iLast;
						  if(!getDoc()->appendFmt(propsArray))
							  return;
					
						  if(!getDoc()->appendSpan(p, i - iLast))
							  return;
					  }
					  iOverride = FRIBIDI_TYPE_LTR;
					  propsArray[1] = prop_ltr.c_str();
					  iLast = i;
				  }
				  else if(!m_bLTRCharContext
						  && iOverride != FRIBIDI_TYPE_RTL
						  && (cLastType != FRIBIDI_TYPE_RTL || cNextType != FRIBIDI_TYPE_RTL))
				  {
					  if(i - iLast > 0)
					  {
						  p = pStart + iLast;
						  if(!getDoc()->appendFmt(propsArray))
							  return;

						  if(!getDoc()->appendSpan(p, i - iLast))
							  return;
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
						  p = pStart + iLast;
						  if(!getDoc()->appendFmt(propsArray))
							  return;
					
						  if(!getDoc()->appendSpan(p, i - iLast))
							  return;
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
			  p = pStart + iLast;
			  if(!getDoc()->appendFmt(propsArray))
				  return;
					
			  if(!getDoc()->appendSpan(p, iLen - iLast))
				  return;
		  }
	  }
	  else
	  {
		  // non-bidi document, just do it the easy way
		  if (!getDoc()->appendSpan(m_pTextRun.ucs4_str(), m_pTextRun.size()))
		  {
			  UT_DEBUGMSG(("DOM: error appending text run\n"));
			  return;
		  }
	  }
	  
	  m_pTextRun.clear();
  }
}

void IE_Imp_MsWord_97::_appendChar (UT_UCSChar ch)
{
  if (m_bInTable) {
    switch (ch) {
    case 7:			// eat tab characters
      return;
    case 30:		// ??
      ch = '-';
		  break;
    }
  }

  if ( m_bIsLower )
    ch = UT_UCS4_tolower ( ch );
  m_pTextRun += ch;
}

/****************************************************************************/
/****************************************************************************/

static int s_cmp_bookmarks_qsort(const void * a, const void * b)
{
	const bookmark * A = static_cast<const bookmark *>(a);
	const bookmark * B = static_cast<const bookmark *>(b);

	if(A->pos != B->pos)
		return (A->pos - B->pos);
	else
		// for bookmarks with identical position we want any start bookmarks to be
		// before end bookmarks.
		return static_cast<UT_sint32>(B->start) - static_cast<UT_sint32>(A->start);
}

static int s_cmp_bookmarks_bsearch(const void * a, const void * b)
{
	UT_uint32 A = *static_cast<const UT_uint32 *>(a);
	const bookmark * B = static_cast<const bookmark *>(b);

	return (A - B->pos);
}

XML_Char * IE_Imp_MsWord_97::_getBookmarkName(wvParseStruct * ps, UT_uint32 pos)
{
	XML_Char *str;
	UT_iconv_t ic_handle;
	// word bookmarks can be at most 30 characters, so make a reasonable buffer
	// for the UTF-8 version
	char buff[200];
	char *buff_ptr = &buff[0];
	const char *in_ptr;
	size_t out_left = 200, in_left;

	if (!XAP_EncodingManager::get_instance()->cjk_locale()
	   &&(XAP_EncodingManager::get_instance()->try_nativeToU(0xa1) != 0xa1))
	{
		ic_handle = UT_iconv_open(XAP_EncodingManager::get_instance()->getNativeEncodingName(), "UCS-2");
	}
	else
	{
		// use UTF-8
		ic_handle = UT_iconv_open("UTF-8", "UCS-2LE");
	}

	if(ps->Sttbfbkmk.extendedflag == 0xFFFF)
	{
		// 16 bit stuff
		in_ptr = reinterpret_cast<const char *>(ps->Sttbfbkmk.u16strings[pos]);

		// TODO is this really UCS-2 or UTF-16?
		// TODO and are we using strlen for the number of 16-bit words
		// TODO or the number of characters?
		// TODO Because UTF-16 characters are sometimes expressed as 2 words
		in_left = 2 * UT_UCS2_strlen(static_cast<const UT_UCS2Char*>(ps->Sttbfbkmk.u16strings[pos])) + 2;

		UT_iconv( ic_handle, &in_ptr, &in_left, &buff_ptr,&out_left);
		str = new XML_Char[200 - out_left];
		strcpy(str, buff);
	}
	else
	{
		// 8 bit stuff
		// there is a bug in wv, and the table gets incorrectly retrieved
		// if it contains 8-bit strings
		if(ps->Sttbfbkmk.s8strings[pos])
		{
			UT_uint32 len = strlen(ps->Sttbfbkmk.s8strings[pos]);
			str = new XML_Char[len + 1];
			UT_uint32 i = 0;
			for(i = 0; i < len; i++)
				str[i] = ps->Sttbfbkmk.s8strings[pos][i];
			str[i] = 0;
		}
		else
			str = NULL;
	}

	return str;
}

int IE_Imp_MsWord_97::_docProc (wvParseStruct * ps, UT_uint32 tag)
{
	// flush out any pending character data
	this->_flush ();

	switch (static_cast<wvTag>(tag))
	{
	case DOCBEGIN:

		// test the bidi nature of this document
#ifdef BIDI_DEBUG
		m_bBidiDocument = wvIsBidiDocument(ps);
		UT_DEBUGMSG(("IE_Imp_MsWord_97::_docProc: complex %d, bidi %d\n",
					 ps->fib.fComplex,m_bBidiDocument));
#else
		// for now we will assume that all documents are bidi
		// documents (Tomas, Apr 12, 2003)
		
		m_bBidiDocument = false;
#endif
		// import styles
		_handleStyleSheet(ps);
		
		UT_uint32 i,j;

		if(m_pBookmarks)
		{
			for(i = 0; i < m_iBookmarksCount; i++)
			{
				if(m_pBookmarks[i].name && m_pBookmarks[i].start)
				{
					delete []m_pBookmarks[i].name;
					m_pBookmarks[i].name = NULL;
				}
			}
			delete [] m_pBookmarks;
		}
		BKF *bkf;
		BKL *bkl;
		U32 *posf, *posl, nobkf, nobkl;

		if(!wvGetBKF_PLCF (&bkf, &posf, &nobkf, ps->fib.fcPlcfbkf, ps->fib.lcbPlcfbkf, ps->tablefd))
		{
			m_iBookmarksCount = nobkf;
		}
		else
			m_iBookmarksCount = 0;

		if(!wvGetBKL_PLCF (&bkl, &posl, &nobkl, ps->fib.fcPlcfbkl, ps->fib.lcbPlcfbkl, ps->fib.fcPlcfbkf, ps->fib.lcbPlcfbkf, ps->tablefd))
		{
			m_iBookmarksCount += nobkl;
		}
		else
		{
			if(m_iBookmarksCount > 0)
			{
				//free the bkf and posf
				wvFree(bkf);
				wvFree(posf);
				m_iBookmarksCount = 0;
			}
		}
		UT_return_val_if_fail(nobkl == nobkf, 0);
		if(m_iBookmarksCount > 0)
		{
			m_pBookmarks = new bookmark[m_iBookmarksCount];
			UT_return_val_if_fail(m_pBookmarks, 0);
			for(i = 0; i < nobkf; i++)
			{
				m_pBookmarks[i].name = _getBookmarkName(ps, i);
				m_pBookmarks[i].pos  = posf[i];
				m_pBookmarks[i].start = true;
			}

			for(j = i; j < nobkl + i; j++)
			{
				// since the name is shared with the start of the bookmark,
				// we reuse it
				UT_sint32 iBkf = static_cast<UT_sint32>(bkl[j-i].ibkf) < 0 ? nobkl + static_cast<UT_sint32>(bkl[j-i].ibkf) : bkl[j-i].ibkf;
				m_pBookmarks[j].name = m_pBookmarks[iBkf].name;
				m_pBookmarks[j].pos  = posl[j - i];
				m_pBookmarks[j].start = false;
			}
			// free bkf, bkl, posf, posl
			wvFree(bkf);
			wvFree(bkl);
			wvFree(posf);
			wvFree(posl);

			//now sort the bookmarks by position
			qsort(static_cast<void*>(m_pBookmarks), m_iBookmarksCount, sizeof(bookmark), s_cmp_bookmarks_qsort);
#ifdef DEBUG
			for(UT_uint32 k = 0; k < m_iBookmarksCount; k++)
			{
				UT_DEBUGMSG(("Bookmark: name [%s], pos %d, start %d\n", m_pBookmarks[k].name,m_pBookmarks[k].pos,m_pBookmarks[k].start));
			}

#endif
		}

			break;
	case DOCEND:
	default:
		break;
	}

	return 0;
}

bool IE_Imp_MsWord_97::_insertBookmark(bookmark * bm)
{
	// first of all flush what is in the buffers
	this->_flush();
	bool error = false;

	const XML_Char* propsArray[5];
	propsArray[0] = static_cast<const XML_Char *>("name");
	propsArray[1] = static_cast<const XML_Char *>(bm->name);
	propsArray[2] = static_cast<const XML_Char *>("type");
	propsArray[4] = 0;

	if(bm->start)
		propsArray[3] = static_cast<const XML_Char *>("start");
	else
		propsArray[3] = static_cast<const XML_Char *>("end");

	if(m_bInTable && !m_bCellOpen)
	{
		emObject * pObject = new emObject;
		pObject->props1 = propsArray[1];
		pObject->objType = PTO_Bookmark;
		pObject->props2 = propsArray[3];
		m_vecEmObjects.addItem(static_cast<void *>(pObject));
	}
	else
	{
		if (!getDoc()->appendObject (PTO_Bookmark, propsArray))
		{
			UT_DEBUGMSG (("Could not append bookmark object\n"));
			error = true;
		}
	}
	return error;
}

bool IE_Imp_MsWord_97::_insertBookmarkIfAppropriate()
{
	//now search for position m_iDocPosition in our bookmark list;
	bookmark * bm;
	if (m_iBookmarksCount == 0) {
		bm = static_cast<bookmark*>(NULL);
	}
	else {
		bm = static_cast<bookmark*>( bsearch(static_cast<const void *>(&m_iDocPosition),
				m_pBookmarks, m_iBookmarksCount, sizeof(bookmark),
				s_cmp_bookmarks_bsearch));
	}
	bool error = false;
	if(bm)
	{
	   // there is a bookmark at the current position
	   // first make sure the returned bookmark is the first one at this position
	   while(bm > m_pBookmarks && (bm - 1)->pos == m_iDocPosition)
		   bm--;

	   while(bm->pos == m_iDocPosition)
		  error |= _insertBookmark(bm++);
	}
	return error;
}

int IE_Imp_MsWord_97::_charProc (wvParseStruct *ps, U16 eachchar, U8 chartype, U16 lid)
{
	_insertBookmarkIfAppropriate();

	// convert incoming character to unicode
	if (chartype)
		eachchar = wvHandleCodePage(eachchar, lid);

	switch (eachchar)
	{

	case 11: // forced line break
		eachchar = UCS_LF;
		break;

	case 12: // page or section break
		this->_flush ();
		eachchar = UCS_FF;
		break;

	case 13: // end of paragraph
		m_iDocPosition++;
		return 0;

	case 14: // column break
		eachchar = UCS_VTAB;
		break;

	case 19: // field begin
		this->_flush ();
		ps->fieldstate++;
		ps->fieldmiddle = 0;
		this->_fieldProc (ps, eachchar, chartype, lid);
		return 0;

	case 20: // field separator
		this->_fieldProc (ps, eachchar, chartype, lid);
		ps->fieldmiddle = 1;
		return 0;

	case 21: // field end
		ps->fieldstate--;
		ps->fieldmiddle = 0;
		this->_fieldProc (ps, eachchar, chartype, lid);
		return 0;

	}

	// i'm not sure if this is needed any more
	// yes, it is, for instance hyperlinks need it
	if (ps->fieldstate)
	{
		xxx_UT_DEBUGMSG(("DOM: fieldstate\n"));
		if(this->_fieldProc (ps, eachchar, chartype, lid))
		{
			return 0;
		}
	}

	// take care of any oddities in Microsoft's character encoding
	if (chartype == 1 && eachchar == 146)
		eachchar = 39; // apostrophe

	this->_appendChar (static_cast<UT_UCSChar>(eachchar));
	m_iDocPosition++;

	return 0;
}

int IE_Imp_MsWord_97::_specCharProc (wvParseStruct *ps, U16 eachchar, CHP *achp)
{
	Blip blip;
	long pos;
	FSPA * fspa;
	FDOA * fdoa;
#ifdef SUPPORTS_OLD_IMAGES
	wvStream *fil;
	PICF picf;
#endif

	//
	// This next bit of code is to handle fields
	//

	switch (eachchar)
	{

	case 19: // field begin
		this->_flush ();
		ps->fieldstate++;
		ps->fieldmiddle = 0;
		this->_fieldProc (ps, eachchar, 0, 0x400);
		return 0;

	case 20: // field separator
		if (achp->fOle2)
		{
			UT_DEBUGMSG(("Field has an assocaited embedded OLE object\n"));
		}
		ps->fieldmiddle = 1;
		this->_fieldProc (ps, eachchar, 0, 0x400);
		return 0;

	case 21: // field end
		ps->fieldstate--;
		ps->fieldmiddle = 0;
		this->_fieldProc (ps, eachchar, 0, 0x400);
		return 0;

	}

	/* it seems some fields characters slip through here which tricks
	 * the import into thinking it has an image with it really does
	 * not. this catches special characters in a field
	 */
	if (ps->fieldstate) {
		if (this->_fieldProc(ps, eachchar, 0, 0x400))
			return 0;
	}

	//
	// This next bit of code is to handle OLE2 embedded objects and images
	//

	switch (eachchar)
	{
	case 0x01: // Older ( < Word97) image, currently not handled very well

		if (achp->fOle2) {
			UT_DEBUGMSG(("embedded OLE2 component. currently unsupported"));
			return 0;
		}

		pos = wvStream_tell(ps->data);

#ifdef SUPPORTS_OLD_IMAGES
		UT_DEBUGMSG(("Pre W97 Image format.\n"));
		wvStream_goto(ps->data, achp->fcPic_fcObj_lTagObj);

		if (1 == wvGetPICF(wvQuerySupported(&ps->fib, NULL), &picf,
				   ps->data) && NULL != picf.rgb)
		  {
			fil = picf.rgb;

			if (wv0x01(&blip, fil, picf.lcb - picf.cbHeader))
			  {
                       this->_handleImage(&blip, picf.mx * picf.dxaGoal / 1000, picf.my * picf.dyaGoal / 1000);
			  }
			else
			  {
			UT_DEBUGMSG(("Dom: no graphic data\n"));
			  }
#else
			UT_DEBUGMSG(("DOM: 0x01 graphics support is disabled at the moment\n"));
#endif

			wvStream_goto(ps->data, pos);

			return 0;
		  }
		else
		  {
			UT_DEBUGMSG(("Couldn't import graphic!\n"));
			return 0;
		  }
	case 0x08: // Word 97, 2000, XP image

		if (wvQuerySupported(&ps->fib, NULL) >= WORD8) // sanity check
		{
			if (ps->nooffspa > 0)
			{

				fspa = wvGetFSPAFromCP(ps->currentcp, ps->fspa,
									   ps->fspapos, ps->nooffspa);

				if(!fspa)
				{
					UT_DEBUGMSG(("No fspa! Panic and Insanity Abounds!\n"));
					return 0;
				}

				if (wv0x08(&blip, fspa->spid, ps))
				{
					this->_handleImage(&blip, fspa->xaRight-fspa->xaLeft,
									   fspa->yaBottom-fspa->yaTop);
				}
				else
				{
					UT_DEBUGMSG(("Dom: no graphic data!\n"));
					return 0;
				}
			}
			else
			{
				xxx_UT_DEBUGMSG(("nooffspa was <= 0 -- ignoring"));
			}
		}
		else
		{
			UT_DEBUGMSG(("pre Word8 0x08 graphic -- unsupported at the moment"));
			fdoa = wvGetFDOAFromCP(ps->currentcp, NULL, ps->fdoapos,
								   ps->nooffdoa);

			// TODO: do something with the data in this fdoa someday...
		}

		return 0;
	}

	return 0;
}

int IE_Imp_MsWord_97::_beginComment(wvParseStruct *ps, UT_uint32 tag,
					void *props, int dirty)
{
  UT_DEBUGMSG(("DOM: begin comment\n"));
  return 0;
}

int IE_Imp_MsWord_97::_endComment(wvParseStruct *ps, UT_uint32 tag,
				  void *props, int dirty)
{
  UT_DEBUGMSG(("DOM: begin comment\n"));
  return 0;
}


int IE_Imp_MsWord_97::_eleProc(wvParseStruct *ps, UT_uint32 tag,
							   void *props, int dirty)
{
	//
	// Marshall these off to the correct handlers
	//

	switch (static_cast<wvTag>(tag))
	{

	case SECTIONBEGIN:
		return _beginSect (ps, tag, props, dirty);

	case SECTIONEND:
		return _endSect (ps, tag, props, dirty);

	case PARABEGIN:
		return _beginPara (ps, tag, props, dirty);

	case PARAEND:
		return _endPara (ps, tag, props, dirty);

	case CHARPROPBEGIN:
		return _beginChar (ps, tag, props, dirty);

	case CHARPROPEND:
		return _endChar (ps, tag, props, dirty);

	case COMMENTBEGIN:
	  return _beginComment (ps, tag, props, dirty);

	case COMMENTEND:
	  return _endComment (ps, tag, props, dirty);

	default:
	  UT_ASSERT_NOT_REACHED();

	}

	return 0;
}

/****************************************************************************/
/****************************************************************************/

int IE_Imp_MsWord_97::_beginSect (wvParseStruct *ps, UT_uint32 tag,
				  void *prop, int dirty)
{
	SEP * asep = static_cast <SEP *>(prop);

	const XML_Char * propsArray[3];
	UT_String propBuffer;
	UT_String props;

	// flush any character runs
	this->_flush ();

	// page-margin-left
	UT_String_sprintf(propBuffer,
		"page-margin-left:%s;",
		UT_convertInchesToDimensionString(DIM_IN, (static_cast<float>(asep->dxaLeft) / 1440), "1.4"));
	props += propBuffer;

	// page-margin-right
	UT_String_sprintf(propBuffer,
		"page-margin-right:%s;",
		UT_convertInchesToDimensionString(DIM_IN, (static_cast<float>(asep->dxaRight) / 1440), "1.4"));
	props += propBuffer;

	// page-margin-top
	UT_String_sprintf(propBuffer,
		"page-margin-top:%s;",
		UT_convertInchesToDimensionString(DIM_IN, (static_cast<float>(asep->dyaTop) / 1440), "1.4"));
	props += propBuffer;

	// page-margin-bottom
	UT_String_sprintf(propBuffer,
		"page-margin-bottom:%s;",
		UT_convertInchesToDimensionString(DIM_IN, (static_cast<float>(asep->dyaBottom) / 1440), "1.4"));
	props += propBuffer;

	// page-margin-header
	UT_String_sprintf(propBuffer,
		"page-margin-header:%s;",
		UT_convertInchesToDimensionString(DIM_IN, (static_cast<float>(asep->dyaHdrTop) / 1440), "1.4"));
	props += propBuffer;

	// page-margin-footer
	UT_String_sprintf(propBuffer,
		"page-margin-footer:%s;",
		UT_convertInchesToDimensionString(DIM_IN, (static_cast<float>(asep->dyaHdrBottom) / 1440),
						  "1.4"));
	props += propBuffer;

	if(asep->fBidi)
	{
		// this is an RTL section, set dominant direction to rtl
		props += "dom-dir:rtl;";
	}
	{
		// this is an LTR section, we want to set the direction
		// explicitely so that we do not end up with wrong default
		props += "dom-dir:ltr;";
	}


	if(asep->fPgnRestart)
	  {
		// set to 1 when page numbering should be restarted at the beginning of this section
		props += "section-restart:1;";
	  }

	// user specified starting page number
	UT_String_sprintf(propBuffer, "section-restart-value:%d;", asep->pgnStart);
	props += propBuffer;

	// columns
	if (asep->ccolM1) {
		// number of columns
		UT_String_sprintf(propBuffer,
				"columns:%d;", (asep->ccolM1+1));
		props += propBuffer;

		// columns gap
		UT_String_sprintf(propBuffer,
				"column-gap:%s;",
				UT_convertInchesToDimensionString(DIM_IN, (static_cast<float>(asep->dxaColumns) / 1440),
												  "1.4"));
		props += propBuffer;
	}

	// darw a vertical line between columns
	if (asep->fLBetween == 1)
	{
		props += "column-line:on;";
	}

	// space after section (gutter)
	UT_String_sprintf(propBuffer,
			"section-space-after:%s",
			UT_convertInchesToDimensionString(DIM_IN, (static_cast<float>(asep->dzaGutter) / 1440), "1.4"));
	props += propBuffer;

	//
	// TODO: headers/footers, section breaks
	//
//
// Sevior: Only do this ONCE!!! Abiword can only handle one page size.
//
	if(!m_bSetPageSize)
	{
		//
		// all of this data is related to Abi's <pagesize> tag
		//
		m_bSetPageSize = true;
		double page_width  = 0.0;
		double page_height = 0.0;
		double page_scale  = 1.0;

		if (asep->dmOrientPage == 1)
			getDoc()->m_docPageSize.setLandscape ();
		else
			getDoc()->m_docPageSize.setPortrait ();

		page_width = asep->xaPage / 1440.0;
		page_height = asep->yaPage / 1440.0;

		UT_DEBUGMSG(("DOM: pagesize: (landscape: %d) (width: %f) (height: %f) (paper-type: %d)\n",
					 asep->dmOrientPage, page_width, page_height, asep->dmPaperReq));

		const char * paper_name = s_mapPageIdToString (asep->dmPaperReq);

		if (paper_name)
		  {
			// we found a paper name, let's use it
			getDoc()->m_docPageSize.Set (paper_name);
		  }
		else
		  {
			getDoc()->m_docPageSize.Set ("Custom");
		  }

		// always use the passed size
		getDoc()->m_docPageSize.Set (page_width, page_height, DIM_IN);
		getDoc()->m_docPageSize.setScale(page_scale);
	}

	xxx_UT_DEBUGMSG (("DOM:SEVIOR the section properties are: '%s'\n", props.c_str()));

	propsArray[0] = static_cast<const XML_Char *>("props");
	propsArray[1] = static_cast<const XML_Char *>(props.c_str());
	propsArray[2] = 0;

	if (!getDoc()->appendStrux(PTX_Section, static_cast<const XML_Char **>(&propsArray[0])))
	{
		UT_DEBUGMSG (("DOM: error appending section props!\n"));
		return 1;
	}

	// increment our section count
	m_bInSect = true;
	m_bInPara = false; // reset paragraph status
	m_nSections++;

	// TODO: we need to do some work on Headers/Footers

	/*
	 * break codes:
	 * 0 No break
	 * 1 New column
	 * 2 New page
	 * 3 Even page
	 * 4 Odd page
	 */

//	if (asep->bkc > 1 && m_nSections > 1) // don't apply on the 1st page
	if (m_nSections > 1) // don't apply on the 1st page
	{
		// new sections always need a block
		if (!getDoc()->appendStrux(PTX_Block, static_cast<const XML_Char **>(NULL)))
		{
			UT_DEBUGMSG (("DOM: error appending new block\n"));
			return 1;
		}
		m_bInPara = true;

		UT_UCSChar ucs = UCS_FF;
		switch (asep->bkc) {
		case 1:
			ucs = UCS_VTAB;
			X_CheckError(getDoc()->appendSpan(&ucs,1));
			break;

		case 2:
			X_CheckError(getDoc()->appendSpan(&ucs,1));
			break;

		case 3: // TODO: handle me better (not even)
			X_CheckError(getDoc()->appendSpan(&ucs,1));
			break;

		case 4: // TODO: handle me better (not odd)
			X_CheckError(getDoc()->appendSpan(&ucs,1));
			break;

		case 0:
		default:
			break;
		}
	}

	return 0;
}

int IE_Imp_MsWord_97::_endSect (wvParseStruct *ps, UT_uint32 tag,
								void *prop, int dirty)
{
	// if we're at the end of a section, we need to check for a section mark
	// at the end of our character stream and remove it (to prevent page breaks
	// between sections)
	if (m_pTextRun.size() &&
		m_pTextRun[m_pTextRun.size()-1] == UCS_FF)
	  {
		m_pTextRun[m_pTextRun.size()-1] = 0;
	  }
	m_bInSect = false;
	m_bInPara = false; // reset paragraph status
	return 0;
}

int IE_Imp_MsWord_97::_beginPara (wvParseStruct *ps, UT_uint32 tag,
				  void *prop, int dirty)
{
	PAP *apap = static_cast <PAP *>(prop);

	{
	  if (apap->fInTable) {
	    if (!m_bInTable) {
	      m_bInTable = true;
	        _table_open();
	    }

	    if (ps->endcell) {
	      ps->endcell = 0;
	      _cell_close();
	      if (m_iCellsRemaining > 0) {
		m_iCellsRemaining--;
		if (m_iCellsRemaining == 0) {
		  _row_close();
		}
	      }
	    }

	    _row_open();

	    // determine column spans
	    if (!m_bCellOpen) {
	      m_vecColumnSpansForCurrentRow.clear();

	      for (int column = 1; column < ps->nocellbounds; column++) {
		int span = 0;

		for (int i = column; i < ps->nocellbounds; i++) {
		  if (ps->cellbounds[i] >= apap->ptap.rgdxaCenter[column]) {
		    span = (i - column);
		    break;
		  }
		}
		m_vecColumnSpansForCurrentRow.addItem(reinterpret_cast<void *>(span));
	      }
	    }

	    _cell_open(ps, apap);

	    if (m_iCellsRemaining == 0) {
	      m_iCellsRemaining = apap->ptap.itcMac + 1;
	    }

	    if (m_iRowsRemaining == 0) {
	      m_iRowsRemaining = ps->norows;
	    }

	    m_iRowsRemaining--;
	  }
	  else if (m_bInTable) {
	    m_bInTable = false;
	    _table_close(ps, apap);
	  }
	}

	// first, flush any character data in any open runs
	this->_flush ();

	if (apap->fTtp)
	  {
	    m_bInPara = true;
	    return 0;
	  }

	if (apap->fBidi == 1)
	{
		m_bLTRParaContext = false;
	} else
	{
		m_bLTRParaContext = true;
	}

	m_bBidiDocument = false;

	// break before paragraph?
	if (apap->fPageBreakBefore)
	{
		// TODO: this should really set a property in
		// TODO: in the paragraph, instead; but this
		// TODO: gives a similar effect for now.
		// TOOD: when it is handled properly the code needs to be
		// moved into _generateParaProps()
		UT_DEBUGMSG(("_beginPara: appending default block\n"));
		getDoc()->appendStrux(PTX_Block, NULL);
		UT_UCSChar ucs = UCS_FF;
		getDoc()->appendSpan(&ucs,1);
	}
	
	UT_String props;
	_generateParaProps(props, apap, ps);
	
	//props, level, listid, parentid, style (TODO), NULL
	const XML_Char * propsArray[11];

	/* lists */
	UT_uint32 myListId = 0;
	LVLF * myLVLF = NULL;
	UT_String szListId;
	UT_String szParentId;
	UT_String szStartValue;
	UT_String szLevel;
	UT_String propBuffer;
	/*
	  The MS documentation on lists really sucks, but we've been able to decipher
	  some meaning from it and get simple lists to sorta work. This code mostly prints out
	  debug messages with useful information in them, but it will also append a list
	  and add a given paragraph to a given list
	*/

	if ( apap->ilfo && ps->lfo )
	{
		// all lists have ilfo set
		UT_DEBUGMSG(("list: ilvl %d, ilfo %d\n",apap->ilvl,apap->ilfo));	//ilvl is the list level
		LVL * myLVL = NULL;
		LFO * myLFO = NULL;
		LST * myLST = NULL;
		LFOLVL * myLFOLVL = NULL;

		UT_sint32 myStartAt = -1;
		U8 * mygPAPX = NULL;
		U8 * mygCHPX = NULL;
		XCHAR * myNumberStr = NULL;
		UT_sint32 myNumberStr_count = 0;
		UT_uint32 mygPAPX_count = 0, mygCHPX_count = 0;

		PAPX myPAPX;
		CHPX myCHPX;

		// first, get the LFO, and then find the lfovl for this paragraph
		myLFO = &ps->lfo[apap->ilfo - 1];

		UT_uint32 i = 0, j = 0, k = 0;
		while(static_cast<UT_sint32>(i) < apap->ilfo - 1 && i < ps->nolfo)
		{
			j += ps->lfo[i].clfolvl;
			i++;
		}

		// remember how many overrides are there for this record
		k = ps->lfo[i].clfolvl;

		// if there are any overrides, then see if one of them applies to this level
		if(k && ps->lfolvl)
		{
			i = 0;
			while(i < k && ps->lfolvl[j].ilvl != apap->ilvl)
			{
				j++;
				i++;
			}

			if(ps->lfolvl[--j].ilvl != apap->ilvl)
			{
				UT_DEBUGMSG(("list: no LFOLVL found for this level (1)\n"));
				myLFOLVL = NULL;
			}
			else
			{
				myLFOLVL = &ps->lfolvl[j];
				UT_DEBUGMSG(("list: lfovl: iStartAt %d, fStartAt\n", myLFOLVL->iStartAt,myLFOLVL->fStartAt,myLFOLVL->fFormatting));
				if(!myLFOLVL->fFormatting && myLFOLVL->fStartAt)
					myStartAt = myLFOLVL->iStartAt;
			}
		}
		else
		{
			UT_DEBUGMSG(("list: no LFOLVL found for this level (2)\n"));
			myLFOLVL = NULL;
		}

		// now that we might have the LFOLVL, let's see if we should use the LVL from the LFO
		bool bNeedLST_LVL = (!myLFOLVL || !myLFOLVL->fStartAt || !myLFOLVL->fFormatting);
		bool bLST_LVL_format = true;
		if(myLFOLVL)
		{
			// this branch has not been (thoroughly) debugged
			// Abi bugs 2205 and 2393 exhibit this behavior
			UT_DEBUGMSG(("list: using the LVL from LFO\n"));
			myListId = myLFOLVL->iStartAt;
			i = 0;
			UT_DEBUGMSG(("list: number of LSTs %d, my lsid %d\n", ps->noofLST,myListId));
			while(i < ps->noofLST && ps->lst[i].lstf.lsid != myListId)
			{
				i++;
				UT_DEBUGMSG(("list: lsid in LST %d\n", ps->lst[i-1].lstf.lsid));
			}

			if(i == ps->noofLST || ps->lst[i].lstf.lsid != myListId)
			{
				UT_DEBUGMSG(("error: could not locate LST entry\n"));
				goto list_error;
			}

			myLST = &ps->lst[i];
			myLVL = &myLST->lvl[apap->ilvl];

			// now we should have the LVL
			UT_ASSERT(myLVL);

			myLVLF = &myLVL->lvlf;
			UT_ASSERT(myLVLF);

			myStartAt = myLFOLVL->fStartAt ? static_cast<signed>(myLVLF->iStartAt) : -1;

			mygPAPX = myLFOLVL->fFormatting ? myLVL->grpprlPapx : NULL;
			mygPAPX_count = myLFOLVL->fFormatting ? myLVLF->cbGrpprlPapx : 0;

			// not sure about this, the CHPX applies to the number, so it might be
			// that we should take this if the fStartAt is set -- the docs are not clear
			mygCHPX = myLFOLVL->fFormatting ? myLVL->grpprlChpx : NULL;
			mygCHPX_count = myLFOLVL->fFormatting ? myLVLF->cbGrpprlChpx : 0;

			myNumberStr = myLFOLVL->fStartAt && myLVL->numbertext ? myLVL->numbertext + 1 : NULL;
			myNumberStr_count = myNumberStr ? *(myLVL->numbertext) : 0;

			if(myLFOLVL->fFormatting)
				bLST_LVL_format = false;

		}

		if(bNeedLST_LVL)
		{
			LVL * prevLVL = myLVL;
			LVLF * prevLVLF = myLVLF;
			myListId = myLFO->lsid;
			UT_DEBUGMSG(("list: using the LVL from LST\n"));
			i = 0;
			UT_DEBUGMSG(("list: number of LSTs %d, my lsid %d\n", ps->noofLST,myListId));
			while(i < ps->noofLST && ps->lst[i].lstf.lsid != myListId)
			{
				i++;
				xxx_UT_DEBUGMSG(("list: lsid in LST %d\n", ps->lst[i-1].lstf.lsid));
			}

			if(i == ps->noofLST || ps->lst[i].lstf.lsid != myListId)
			{
				UT_DEBUGMSG(("error: could not locate LST entry\n"));
				goto list_error;
			}

			myLST = &ps->lst[i];
			myLVL = &myLST->lvl[apap->ilvl];

			// now we should have the correct LVL
			UT_ASSERT(myLVL);

			myLVLF = &myLVL->lvlf;
			UT_ASSERT(myLVLF);

			// retrieve any stuff we need from here (i.e., only what we did not get from the LFO LVL)
			myStartAt = myStartAt == -1 ? myLVLF->iStartAt : myStartAt;

			mygPAPX_count = !mygPAPX ? myLVLF->cbGrpprlPapx : mygPAPX_count;
			mygPAPX = !mygPAPX ? myLVL->grpprlPapx : mygPAPX;

			mygCHPX_count = !mygCHPX ? myLVLF->cbGrpprlChpx : mygCHPX_count;
			mygCHPX = !mygCHPX ? myLVL->grpprlChpx : mygCHPX;

			myNumberStr_count = !myNumberStr && myLVL->numbertext ? *(myLVL->numbertext) : myNumberStr_count;
			myNumberStr = !myNumberStr && myLVL->numbertext ? myLVL->numbertext + 1 : myNumberStr;


			// if there was a valid LFO LVL record that pertained to formatting
			// then we will set the myLVL and myLVLF variables back to this record
			// so that it can be used
			if(!bLST_LVL_format && prevLVL && prevLVLF)
			{
				myLVL = prevLVL;
				myLVLF = prevLVLF;
			}
		}

		UT_DEBUGMSG(("list: number text len %d, papx len %d, chpx len%d\n",myNumberStr_count,mygPAPX_count,mygCHPX_count));
		myPAPX.cb = mygPAPX_count;
		myPAPX.grpprl = mygPAPX;
		myPAPX.istd = 4095; // no style

		myCHPX.cbGrpprl = mygCHPX_count;
		myCHPX.grpprl = mygCHPX;
		myCHPX.istd = 4095; // no style


		// if we are in a new list, then do some clean up first and remember the list id
		if(m_iMSWordListId != myListId)
		{
			m_iMSWordListId = myListId;

			for(UT_uint32 i = 0; i < 9; i++)
				m_iListIdIncrement[i] = 0;

			UT_VECTOR_PURGEALL(ListIdLevelPair *, m_vLists);
			m_vLists.clear();
		}

		// a hack -- see the note on myListId below
		myListId += myLVLF->nfc;
		myListId += apap->ilvl;

		/*
		  IMPORTANT now we have the list formatting sutff retrieved; it is found in several
		  different places:
		  apap->ilvl - the level of this list (0-8)

		  myStartAt	- the value at which the numbering for this listshould start
		  (i.e., the number of the first item on the list)

		  myListId	- the id of this list, we need this to know to which list this
		  paragraph belongs; unfortunately, there seem to be some cases where separate
		  lists *share* the same id, for instance when two lists, of different formatting,
		  are separated by only empty paragraphs. As a hack, I have added the format number
		  to the list id, so gaining different id for different formattings (it is not foolproof,
		  for if id1 + format1 == id2 + format2 then we get two lists joined, but the probability
		  of that should be small). Further problem is that in AW, list id refers to the set of
		  list elements on the same level, while in Word the id is that of the entire list. The
		  easiest way to tranform the Word id to AW id is to add the level to the id, which
		  is what has been done above

		  PAPX		- the formatting information that needs to be added to the format
		  of this list

		  CHPX		- the formatting of the list number

		  myNumberStr - the actual number string to display (XCHAR *); we probably need
		  this to work out the number separator, since there does not seem
		  to be any reference to this anywhere

		  myNumberStr_count - length of the number string

		  myLVLF->nfc - number format (see the enum below)

		  myLVLF->jc	- number alignment [0: lft, 1: rght, 2: cntr]

		  myLVLF->ixchFollow - what character stands between the number and the para
		  [0:= tab, 1: spc, 2: none]

		*/
		UT_DEBUGMSG(("list: id %d \n",myListId));
		UT_DEBUGMSG(("list: iStartAt %d\n", myStartAt));
		UT_DEBUGMSG(("list: lvlf: format %d\n",myLVLF->nfc)); // see the comment above for nfc values
		UT_DEBUGMSG(("list: lvlf: number align %d [0: lft, 1: rght, 2: cntr]\n",myLVLF->jc));
		UT_DEBUGMSG(("list: lvlf: ixchFollow %d [0:= tab, 1: spc, 2: none]\n",myLVLF->ixchFollow));

		// If a given list id has already been defined, appending a new list with
		// same values will have a harmless effect


		// we will use this to keep track of how many entries of given level we have had
		// every time we get here, we increase the counter for all levels lower that ours
		// then we will add the counter for our level to myListId; this way subsections of
		// the list separated by a higher level list entry will have different id's


		for(j = apap->ilvl + 1; j < 9; j++)
			m_iListIdIncrement[j]++;

		myListId += m_iListIdIncrement[apap->ilvl];

		const XML_Char * list_atts[13];
		
		// list id number
		list_atts[0] = "id";
		UT_String_sprintf(propBuffer, "%d", myListId);
		szListId = propBuffer;
		list_atts[1] = szListId.c_str();

		// parent id
		list_atts[2] = "parentid";

		// we will search backward our list vector for the first entry that has a lower level than we
		// and that will be our parent
		UT_uint32 myParentID = 0;
		for(UT_sint32 n = m_vLists.getItemCount(); n > 0; n--)
		{
			ListIdLevelPair * llp = (ListIdLevelPair *)(m_vLists.getNthItem(n - 1));
			if(llp->level < apap->ilvl)
			{
				myParentID = llp->listId;
				break;
			}
		}
		UT_String_sprintf(propBuffer, "%d", myParentID);
		szParentId = propBuffer;
		list_atts[3] = szParentId.c_str();

		// list type
		list_atts[4] = "type";
		list_atts[5] = s_mapDocToAbiListId (static_cast<MSWordListIdType>(myLVLF->nfc));

		// start value
		list_atts[6] = "start-value";
		UT_String_sprintf(propBuffer, "%d", myStartAt);
		szStartValue = propBuffer;
		list_atts[7] = szStartValue.c_str();

		// list delimiter
		list_atts[8] = "list-delim";
		list_atts[9] = s_mapDocToAbiListDelim (static_cast<MSWordListIdType>(myLVLF->nfc));

		list_atts[10] = "level";
		UT_String_sprintf(propBuffer, "%d", apap->ilvl + 1); // Word level starts at 0, Abi's at 1
		szLevel = propBuffer;
		list_atts[11] = szLevel.c_str();

		// NULL
		list_atts[12] = 0;

		// now add this to our vector of lists
		ListIdLevelPair * llp = new ListIdLevelPair;
		llp->listId = myListId;
		llp->level = apap->ilvl;
		m_vLists.addItem(static_cast<void*>(llp));

		getDoc()->appendList(list_atts);
		UT_DEBUGMSG(("DOM: appended a list\n"));

		// TODO: merge in list properties and such here with the variable 'props',
		// such as list-style, field-font, ...

		// start-value
		props += "start-value:";
		props += szStartValue;
		props += ";";

		// list style
		props += "list-style:";
		props += s_mapDocToAbiListStyle (static_cast<MSWordListIdType>(myLVLF->nfc));
		props += ";";

		// field-font
		props += "field-font:";
		props += s_fieldFontForListStyle (static_cast<MSWordListIdType>(myLVLF->nfc));
		// Put in margin-left and text-indent - Use MS info if available or
		// AbiWord defaults if these aren't present
		//

		UT_String sMargeLeft("margin-left"),sMargeLeftV;
		UT_String sMargeLeftFirst("text-indent"),sMargeLeftFirstV;
		// margin-left
		if (apap->dxaLeft) {
			UT_String_sprintf(sMargeLeftV,
							  "margin-left:%s;",
							  UT_convertInchesToDimensionString(DIM_IN, (static_cast<float>(apap->dxaLeft) / 1440),
																"1.4"));
			UT_DEBUGMSG(("Margin-left from MSWord Lists info %s \n",sMargeLeftV.c_str()));
		}
		//
		// Overide the margin controls for lists.
		// Abi's defaults for levels gives a better fit to average documents. We need
		// to decypher where this info is stored in the MS Word document
		// 
		float fIndent = 0.5; // LIST_DEFAULT_INDENT
		fIndent = static_cast<float>(apap->ilvl + 1) * fIndent;
		if (apap->dxaLeft) 
		{
			fIndent += static_cast<float>(apap->dxaLeft)/(float)1440.0;
		}
		UT_String_sprintf(sMargeLeftV,"%fin",fIndent);

		UT_String_setProperty(props,sMargeLeft,sMargeLeftV);

		// margin-left first line (indent) text-indent

		if (apap->dxaLeft1) {
			UT_String_sprintf(sMargeLeftFirst,
							  "%s",
							  UT_convertInchesToDimensionString(DIM_IN, (static_cast<float>(apap->dxaLeft1) / 1440),"1.4"));

			UT_DEBUGMSG(("Margin-left-first (text-indent)  from MSWord Lists info %s \n",sMargeLeftFirst.c_str()));
			sMargeLeftFirst = "text-indent";
		}

		float fFirstIndent  = (float)-0.3; // LIST_DEFAULT_INDENT_LABEL
		fFirstIndent = fFirstIndent - static_cast<float>(apap->ilvl) * (float)0.2;
		UT_String_sprintf(sMargeLeftFirstV,"%fin;",fFirstIndent);


		UT_String_setProperty(props,sMargeLeftFirst,sMargeLeftFirstV);
		xxx_UT_DEBUGMSG(("props for MSWORD are %s \n",props.c_str()));


	} // end of list-related code

 list_error:

 	// props
	UT_uint32 i = 0;
	propsArray[i++] = static_cast<const XML_Char *>("props");
	propsArray[i++] = static_cast<const XML_Char *>(props.c_str());

	
	// level, or 0 for default, normal level
	if (myListId > 0)
	{
		propsArray[i++] = "level";
		propsArray[i++] = szLevel.c_str();
		propsArray[i++] = "listid";
		propsArray[i++] = szListId.c_str();
		propsArray[i++] = "parentid";
		propsArray[i++] = szParentId.c_str();
	}

	// handle style
	// TODO from wv we get the style props expanded and applied to the
	// characters in the paragraph (i.e., part of the CHP structure);
	// we need to be able to tell to wv not to do this expansion
	if(apap->stylename[0])
	{
		propsArray[i++] = "style";
		propsArray[i++] = apap->stylename;
	}

	// NULL
	propsArray[i] = 0;

	if (!m_bInSect)
	{
		// check for should-be-impossible case
		UT_ASSERT_NOT_REACHED();
		getDoc()->appendStrux(PTX_Section, NULL);
		m_bInSect = true ;
	}

	if (!getDoc()->appendStrux(PTX_Block, static_cast<const XML_Char **>(&propsArray[0])))
	{
		UT_DEBUGMSG(("DOM: error appending paragraph block\n"));
		return 1;
	}

	if (myListId > 0 && myLVLF)
	  {
		// TODO: honor more props
		const XML_Char *list_field_fmt[3];
		list_field_fmt[0] = "type";
		list_field_fmt[1] = "list_label";
		list_field_fmt[2] = 0;

		getDoc()->appendObject(PTO_Field, static_cast<const XML_Char**>(&list_field_fmt[0]));

		// the character following the list label - 0=tab, 1=space, 2=none
		if ( myLVLF->ixchFollow == 0 ) // tab
		  {
		UT_UCSChar tab = UCS_TAB;
		getDoc()->appendSpan(&tab, 1);
		  }
		else if ( myLVLF->ixchFollow == 1 ) // space
		  {
		UT_UCSChar space = UCS_SPACE;
		getDoc()->appendSpan(&space, 1);
		  }
		// else none
	  }
	m_bInPara = true;
	return 0;
}

int IE_Imp_MsWord_97::_endPara (wvParseStruct *ps, UT_uint32 tag,
								void *prop, int dirty)
{
	xxx_UT_DEBUGMSG(("#DOM: _endPara\n"));
	// have to flush here, otherwise flushing later on will result in
	// an empty paragraph being inserted
	this->_flush ();
	m_bInPara = false;
	return 0;
}

int IE_Imp_MsWord_97::_beginChar (wvParseStruct *ps, UT_uint32 tag,
								  void *prop, int dirty)
{
	// flush any data in our character runs
	this->_flush ();

	CHP *achp = static_cast <CHP *>(prop);

	const XML_Char * propsArray[7];
	UT_uint32 propsOffset = 0;

	m_charProps.clear();

	memset (propsArray, 0, sizeof(propsArray));

	_generateCharProps(m_charProps, achp, ps);

	if (!achp->fBidi)
		m_bLTRCharContext = true;
	else
		m_bLTRCharContext = false;

	m_bBidiDocument = m_bLTRCharContext ^ m_bLTRParaContext;

	propsArray[propsOffset++] = static_cast<const XML_Char *>("props");
	propsArray[propsOffset++] = static_cast<const XML_Char *>(m_charProps.c_str());

	if(!m_bEncounteredRevision && (achp->fRMark || achp->fRMarkDel))
	{
		// revision "hack" - add a single revision for all revisioned text
		UT_UCS4String revisionStr ("msword_revisioned_text");
		getDoc()->addRevision(1, revisionStr.ucs4_str(), revisionStr.size());
		m_bEncounteredRevision = true;
	}

	if (achp->fRMark)
	{
	    propsArray[propsOffset++] = static_cast<XML_Char *>("revision");
		m_charRevs = "1";
	    propsArray[propsOffset++] = m_charRevs.c_str();
	}
	else if (achp->fRMarkDel)
	{
	    propsArray[propsOffset++] = static_cast<XML_Char *>("revision");
		m_charRevs = "-1";
	    propsArray[propsOffset++] = m_charRevs.c_str();
	}
	else
		m_charRevs.clear();
	
	
	if(achp->stylename[0])
	{
	    propsArray[propsOffset++] = static_cast<XML_Char *>("style");
	    propsArray[propsOffset++] = static_cast<XML_Char *>(achp->stylename);
	}

	// woah - major error here
	if(!m_bInSect)
	{
		UT_ASSERT_NOT_REACHED();
		getDoc()->appendStrux(PTX_Section, NULL);
		m_bInSect = true ;
	}

	if(!m_bInPara)
	{
		UT_ASSERT_NOT_REACHED();
		getDoc()->appendStrux(PTX_Block, NULL);
		m_bInPara = true ;
	}

	if (!getDoc()->appendFmt(static_cast<const XML_Char **>(&propsArray[0])))
	{
		UT_DEBUGMSG(("DOM: error appending character formatting\n"));
		return 1;
	}
	return 0;
}

int IE_Imp_MsWord_97::_endChar (wvParseStruct *ps, UT_uint32 tag,
								void *prop, int dirty)
{
	// nothing is needed here
	return 0;
}

/****************************************************************************/
/****************************************************************************/

int IE_Imp_MsWord_97::_fieldProc (wvParseStruct *ps, U16 eachchar,
								  U8 chartype, U16 lid)
{
	xxx_UT_DEBUGMSG(("DOM: fieldProc: %c %x\n", static_cast<char>(eachchar),
					 static_cast<int>(eachchar)));

	//
	// The majority of this code has just been ripped out of wv/field.c
	//

	if (eachchar == 0x13) // beginning of a field
	{
		//m_fieldA = 0;
		m_fieldRet = 1;
		if (m_fieldDepth == 0)
		{
			m_fieldWhich = m_command;
			m_command[0] = 0;
			m_argument[0] = 0;
			m_fieldI = 0;
		}
		m_fieldDepth++;
	}
	else if (eachchar == 0x14) // field trigger
	{
		if (m_fieldDepth == 1)
		{
			m_command[m_fieldI] = 0;

			// TODO is this really UCS-2 or UTF-16?
			// TODO and are we using strlen for the number of 16-bit words
			// TODO or the number of characters?
			// TODO Because UTF-16 characters are sometimes expressed as 2 words
			m_iDocPosition += UT_UCS2_strlen(m_command) + 1; // +1 for the 0x14

			m_fieldC = wvWideStrToMB (m_command);
			if (this->_handleCommandField(m_fieldC))
				m_fieldRet = 1;
			else
				m_fieldRet = 0;

			wvFree(m_fieldC);
			m_fieldWhich = m_argument;
			m_fieldI = 0;
		}
	}

	if (m_fieldI >= FLD_SIZE)
	{
		UT_DEBUGMSG(("DOM: Something completely absurd in the fields implementation!\n"));
		UT_ASSERT_NOT_REACHED();
		return 1;
	}

	if (!m_fieldWhich) {
		UT_DEBUGMSG(("DOM: _fieldProc - 'which' is null\n"));
		UT_ASSERT_NOT_REACHED();
		return 1;
	}

	if (chartype)
		m_fieldWhich[m_fieldI] = wvHandleCodePage(eachchar, lid);
	else
		m_fieldWhich[m_fieldI] = eachchar;

	m_fieldI++;

	if (eachchar == 0x15) // end of field marker
	{
		m_fieldDepth--;
		if (m_fieldDepth == 0)
		{
			m_fieldWhich[m_fieldI] = 0;
			//I do not think we should convert this -- this is the field value
			//displayed in the document; in most cases we do not need it, as we
			//calulate it ourselves, but for instance for hyperlinks this is the
			//the text to which the link is tied
			//m_fieldA = wvWideStrToMB (m_argument);
			m_fieldC = wvWideStrToMB (m_command);
			_handleFieldEnd (m_fieldC);
			wvFree (m_fieldC);
		}
	}
	return m_fieldRet;
}

bool IE_Imp_MsWord_97::_handleFieldEnd (char *command)
{
  Doc_Field_t tokenIndex = F_OTHER;
	char *token;

	if (*command != 0x13)
	  {
	  UT_DEBUGMSG (("field did not begin with 0x13\n"));
	  return true;
	  }
	strtok (command, "\t, ");
	while ((token = strtok (NULL, "\t, ")))
	  {
	tokenIndex = s_mapNameToField (token);
	switch (tokenIndex)
	{
		case F_HYPERLINK:
		{
			token = strtok (NULL, "\"\" ");
			UT_return_val_if_fail(m_argument[0] == 0x14 && m_argument[m_fieldI - 1] == 0x15, false);
			m_argument[m_fieldI - 1] = 0;
			UT_UCS2Char * a = m_argument + 1;
			while(*a)
			{
				this->_appendChar(*a++);
				m_iDocPosition++;
				_insertBookmarkIfAppropriate();
			}
			this->_flush();

			if(!m_bInPara)
			{
				getDoc()->appendStrux(PTX_Block, NULL);
				m_bInPara = true ;
			}

			getDoc()->appendObject(PTO_Hyperlink,NULL);
			// increase doc position for the 0x15
			m_iDocPosition++;
			break;
		}
		default:
		break;
		}
	  }
	return false;
}

bool IE_Imp_MsWord_97::_handleCommandField (char *command)
{
	Doc_Field_t tokenIndex = F_OTHER;
	char *token = NULL;

	xxx_UT_DEBUGMSG(("DOM: handleCommandField '%s'\n", command));

	const XML_Char* atts[5];
	atts[0] = "type";
	atts[1] = 0;
	atts[2] = 0;
	atts[3] = 0;
	atts[4] = 0;

	if (*command != 0x13)
	{
		UT_DEBUGMSG(("DOM: field did not begin with 0x13\n"));
		return true;
	}
	strtok(command, "\t, ");

	while ((token = strtok(NULL, "\t, ")))
	{
		tokenIndex = s_mapNameToField (token);

		switch (tokenIndex)
		{
		case F_EDITTIME:
			case F_TIME:
			atts[1] = "time";
			break;

		case F_DateTimePicture: // this isn't totally correct, but it's close
		case F_DATE:
			atts[1] = "date";
			break;

		case F_PAGE:
			atts[1] = "page_number";
			break;

		case F_NUMCHARS:
			atts[1] = "char_count";
			break;

		case F_NUMPAGES:
			atts[1] = "page_count";
			break;

		case F_NUMWORDS:
			atts[1] = "word_count";
			break;

		case F_FILENAME:
			atts[1] = "file_name";
			break;

		case F_PAGEREF:
			token = strtok (NULL, "\"\" ");
			atts[1] = "page_ref";
			atts[2] = "param";
			if(token)
				atts[3] = static_cast<const XML_Char *>(token);
			else
				atts[3] = "no_bookmark_given";
			break;

		case F_HYPERLINK:
		  {
		const XML_Char *new_atts[3];
		token = strtok (NULL, "\"\" ");

		// hyperlink or hyperlink to bookmark
		new_atts[0] = "xlink:href";
		UT_String href;
		if ( !strcmp(token, "\\l") )
		  {
			token = strtok (NULL, "\"\" ");
			href = "#";
			href += token;
		  }
		else
		  {
			href = token;
		  }
		new_atts[1] = href.c_str();
		new_atts[2] = 0;
		this->_flush();

		if(!m_bInPara)
		{
			getDoc()->appendStrux(PTX_Block, NULL);
			m_bInPara = true ;
		}

		getDoc()->appendObject(PTO_Hyperlink, new_atts);
		return true;
		  }

		default:
			// unhandled field type
			continue;
		}

		this->_flush();

		if(!m_bInPara)
		{
			getDoc()->appendStrux(PTX_Block, NULL);
			m_bInPara = true ;
		}

		if (!getDoc()->appendObject (PTO_Field, static_cast<const XML_Char**>(&atts[0])))
		{
			UT_DEBUGMSG(("Dom: couldn't append field (type = '%s')\n", atts[1]));
		}
	}

	return true;
}

typedef enum {
  MSWord_UnknownImage,
  MSWord_VectorImage,
  MSWord_RasterImage
} MSWord_ImageType;

static MSWord_ImageType s_determineImageType ( Blip * b )
{
  if ( !b )
	return MSWord_UnknownImage;

  switch ( b->type )
	{
	case msoblipEMF:
	case msoblipWMF:
	case msoblipPICT:
	  return MSWord_VectorImage;

	case msoblipJPEG:
	case msoblipPNG:
	case msoblipDIB:
	  return MSWord_RasterImage;

	case msoblipERROR:
	case msoblipUNKNOWN:
	default:
	  return MSWord_UnknownImage;
	}
}

UT_Error IE_Imp_MsWord_97::_handleImage (Blip * b, long width, long height)
{
	const char * old_locale = "" ;
	const char * mimetype 	= UT_strdup ("image/png");
	IE_ImpGraphic * importer	= 0;
	FG_Graphic* pFG		= 0;
	UT_Error error		= UT_OK;
	UT_ByteBuf * buf		= 0;
	UT_ByteBuf * pictData 	= new UT_ByteBuf();

  // suck the data into the ByteBuffer

  MSWord_ImageType imgType = s_determineImageType ( b );

  wvStream *pwv;

  if ( imgType == MSWord_RasterImage )
	{
	  pwv = b->blip.bitmap.m_pvBits;
	}
  else if ( imgType == MSWord_VectorImage )
	{
	  pwv = b->blip.metafile.m_pvBits;
	}
  else
	{
	  UT_DEBUGMSG(("UNKNOWN IMAGE TYPE!!"));
	  DELETEP(pictData);
	  FREEP(mimetype);
	  return UT_ERROR;
	}

  size_t size = wvStream_size (pwv);
  char *data = new char[size];
  wvStream_rewind(pwv);
  wvStream_read(data,size,sizeof(char),pwv);
  pictData->append(reinterpret_cast<const UT_Byte*>(data), size);
  delete [] data;

  UT_String propBuffer;
  UT_String propsName;

  if(!pictData->getPointer(0))
	  error =  UT_ERROR;
  else
	  error = IE_ImpGraphic::constructImporter (pictData, IEGFT_Unknown, &importer);

  if ((error != UT_OK) || !importer)
	{
	  UT_DEBUGMSG(("Could not create image importer object\n"));
	  DELETEP(pictData);
	  FREEP(mimetype);
	  goto Cleanup;
	}

  error = importer->importGraphic(pictData, &pFG);
  if ((error != UT_OK) || !pFG)
	{
	  UT_DEBUGMSG(("Could not import graphic\n"));
	  // pictData is already freed in ~FG_Graphic
	  FREEP(mimetype);
	  goto Cleanup;
	}

  // TODO: can we get back a vector graphic?
  buf = static_cast<FG_GraphicRaster *>(pFG)->getRaster_PNG();

  if (!buf)
	{
	  // i don't think that this could ever happen, but...
	  UT_DEBUGMSG(("Could not convert to PNG\n"));
	  DELETEP(pictData);
	  FREEP(mimetype);
	  error = UT_ERROR;
	  goto Cleanup;
	}

  //
  // This next bit of code will set up our properties based on the image attributes
  //

  old_locale = setlocale(LC_NUMERIC, "C");
  UT_String_sprintf(propBuffer, "width:%fin; height:%fin",
		    static_cast<double>(width) / static_cast<double>(1440),
		    static_cast<double>(height) / static_cast<double>(1440));
  setlocale(LC_NUMERIC, old_locale);

  UT_String_sprintf(propsName, "image%d", m_iImageCount++);

  const XML_Char* propsArray[5];
  propsArray[0] = static_cast<const XML_Char *>("props");
  propsArray[1] = static_cast<const XML_Char *>(propBuffer.c_str());
  propsArray[2] = static_cast<const XML_Char *>("dataid");
  propsArray[3] = static_cast<const XML_Char *>(propsName.c_str());
  propsArray[4] = 0;

  if(!m_bInPara)
  {
	  getDoc()->appendStrux(PTX_Block, NULL);
	  m_bInPara = true ;
  }

  if (!getDoc()->appendObject (PTO_Image, propsArray))
	{
	  UT_DEBUGMSG (("Could not create append object\n"));
	  error = UT_ERROR;
	  FREEP(mimetype);
	  goto Cleanup;
	}

  if (!getDoc()->createDataItem(propsName.c_str(), false,
				buf, const_cast<void*>(static_cast<const void*>(mimetype)), NULL))
	{
	  UT_DEBUGMSG (("Could not create data item\n"));
	  error = UT_ERROR;
	  goto Cleanup;
	}

 Cleanup:
  DELETEP(importer);

  return error;
}

/****************************************************************************/
/****************************************************************************/

//
// wv callbacks to marshall data back to our importer class
//

static int charProc (wvParseStruct *ps, U16 eachchar, U8 chartype, U16 lid)
{
	IE_Imp_MsWord_97 * pDocReader = static_cast <IE_Imp_MsWord_97 *> (ps->userData);
	return pDocReader->_charProc (ps, eachchar, chartype, lid);
}

static int specCharProc (wvParseStruct *ps, U16 eachchar, CHP* achp)
{
	IE_Imp_MsWord_97 * pDocReader = static_cast <IE_Imp_MsWord_97 *> (ps->userData);
	return pDocReader->_specCharProc (ps, eachchar, achp);
}

static int eleProc (wvParseStruct *ps, wvTag tag, void *props, int dirty)
{
	IE_Imp_MsWord_97 * pDocReader = static_cast <IE_Imp_MsWord_97 *> (ps->userData);
	return pDocReader->_eleProc (ps, tag, props, dirty);
}

static int docProc (wvParseStruct *ps, wvTag tag)
{
	IE_Imp_MsWord_97 * pDocReader = static_cast <IE_Imp_MsWord_97 *> (ps->userData);
	return pDocReader->_docProc (ps, tag);
}


//--------------------------------------------------------------------------/
//--------------------------------------------------------------------------/

void IE_Imp_MsWord_97::_table_open ()
{
  m_iCurrentRow = 0;
  m_iCurrentCell = 0;

  getDoc()->appendStrux(PTX_Block, NULL);
  getDoc()->appendStrux(PTX_SectionTable, NULL);

  m_bRowOpen = false;
  m_bCellOpen = false;
  m_bInPara = false;
  xxx_UT_DEBUGMSG(("\n<TABLE>"));
}

//--------------------------------------------------------------------------/
//--------------------------------------------------------------------------/

void IE_Imp_MsWord_97::_table_close (const wvParseStruct *ps, const PAP *apap)
{
  _cell_close();
  _row_close();

  UT_String props("table-column-props:");

  if (m_vecColumnWidths.size()) {
    // build column width properties string
    UT_String propBuffer;

    for (UT_uint32 i = 0; i < m_vecColumnWidths.size(); i++) {
      UT_String_sprintf(propBuffer,
			"%s/",
			UT_convertInchesToDimensionString(DIM_IN, (static_cast<float>(reinterpret_cast<int>(m_vecColumnWidths.getNthItem(i))))/1440.0, "1.4")
			);
      props += propBuffer;
    }

    props += "; ";
    m_vecColumnWidths.clear ();
  }

  props += UT_String_sprintf("table-line-ignore:0; table-line-type:1; table-line-thickness:0.8pt; table-col-spacing:%din", (2 * apap->ptap.dxaGapHalf)/ 1440);

  // apply properties
  PL_StruxDocHandle sdh = getDoc()->getLastStruxOfType(PTX_SectionTable);
  getDoc()->changeStruxAttsNoUpdate(sdh,"props",props.c_str());

  // end-of-table
  getDoc()->appendStrux(PTX_EndTable, NULL);
  m_bInPara = false ;

  xxx_UT_DEBUGMSG(("\n</TABLE>\n"));
}

//--------------------------------------------------------------------------/
//--------------------------------------------------------------------------/

void IE_Imp_MsWord_97::_row_open ()
{
  if (m_bRowOpen)
    return;

  m_bRowOpen = true;
  m_iCurrentRow++;
  m_iCurrentCell = 0;

  xxx_UT_DEBUGMSG(("\n\t<ROW:%d>", m_iCurrentRow));
}

//--------------------------------------------------------------------------/
//--------------------------------------------------------------------------/

void IE_Imp_MsWord_97::_row_close ()
{
  if (m_bRowOpen) {
    xxx_UT_DEBUGMSG(("\t</ROW>"));
  }
  m_bRowOpen = false;
}

//--------------------------------------------------------------------------/
//--------------------------------------------------------------------------/

// from fp_TableContainer.h
enum
{
  LS_OFF = 0,	        // No line style, which means no line is drawn
  LS_NORMAL = 1 	// A normal solid line
};

static int
sConvertLineStyle (short lineType)
{
  switch (lineType)
    {
    case 0: return LS_NORMAL;
    case 1:
      return LS_NORMAL;

      // TODO: more cases here
    default:
      return LS_NORMAL;
    }
}

static double
brc_to_pixel (int x)
{
  // each unit is 1/8 of a pixel. abi only deals with whole numbers,
  if(x == 255)
    return  0.;
  return x/8.;
}

void IE_Imp_MsWord_97::_cell_open (const wvParseStruct *ps, const PAP *apap)
{
  if (m_bCellOpen || apap->fTtp)
    return;

  // determine column widths
  UT_Vector columnWidths;

  for (int i = 1; i < ps->nocellbounds; i++) {
    int width = apap->ptap.rgdxaCenter[i] - apap->ptap.rgdxaCenter[i - 1];
    if (width <= 0)
      break;
    columnWidths.addItem(reinterpret_cast<void *>(width));
  }

  if (columnWidths.size() > m_vecColumnWidths.size()) {
    m_vecColumnWidths.clear();
    m_vecColumnWidths = columnWidths;
  }

  // add a new cell
  m_bCellOpen = true;
  m_iCurrentCell++;

  UT_String propBuffer;

  int vspan = ps->vmerges[m_iCurrentRow - 1][m_iCurrentCell - 1];

  if (vspan > 0)
    vspan--;

  UT_String_sprintf(propBuffer,
		    "left-attach:%d; right-attach:%d; top-attach:%d; bot-attach:%d; ",
		    m_iCurrentCell - 1,
		    m_iCurrentCell + reinterpret_cast<int>(m_vecColumnSpansForCurrentRow.getNthItem(m_iCurrentCell - 1)),
		    m_iCurrentRow - 1,
		    m_iCurrentRow + vspan
		    );

  propBuffer += UT_String_sprintf("color:%s;", sMapIcoToColor(apap->ptap.rgshd[m_iCurrentCell - 1].icoFore).c_str());
  propBuffer += UT_String_sprintf("bgcolor:%s;", sMapIcoToColor(apap->ptap.rgshd[m_iCurrentCell - 1].icoBack).c_str());
  // so long as it's not the "auto" color
  if (apap->ptap.rgshd[m_iCurrentCell - 1].icoBack != 0)
    propBuffer += "bg-style:1;";

  const char * old_locale = setlocale(LC_NUMERIC, "C");

  propBuffer += UT_String_sprintf("top-color:%s; top-thickness:%fpt; top-style:%d;",
				  sMapIcoToColor(apap->ptap.rgtc[m_iCurrentCell - 1].brcTop.ico).c_str(),
				  brc_to_pixel(apap->ptap.rgtc[m_iCurrentCell - 1].brcTop.dptLineWidth),
				  sConvertLineStyle(apap->ptap.rgtc[m_iCurrentCell - 1].brcTop.brcType));
  propBuffer += UT_String_sprintf("left-color:%s; left-thickness:%fpx; left-style:%d;",
				  sMapIcoToColor(apap->ptap.rgtc[m_iCurrentCell - 1].brcLeft.ico).c_str(),
				  brc_to_pixel(apap->ptap.rgtc[m_iCurrentCell - 1].brcLeft.dptLineWidth),
				  sConvertLineStyle(apap->ptap.rgtc[m_iCurrentCell - 1].brcLeft.brcType));
  propBuffer += UT_String_sprintf("bot-color:%s; bot-thickness:%fpx; bot-style:%d;",
				  sMapIcoToColor(apap->ptap.rgtc[m_iCurrentCell - 1].brcBottom.ico).c_str(),
				  brc_to_pixel(apap->ptap.rgtc[m_iCurrentCell - 1].brcBottom.dptLineWidth),
				  sConvertLineStyle(apap->ptap.rgtc[m_iCurrentCell - 1].brcBottom.brcType));
  propBuffer += UT_String_sprintf("right-color:%s; right-thickness:%fpx; right-style:%d",
				  sMapIcoToColor(apap->ptap.rgtc[m_iCurrentCell - 1].brcRight.ico).c_str(),
				  brc_to_pixel(apap->ptap.rgtc[m_iCurrentCell - 1].brcRight.dptLineWidth),
				  sConvertLineStyle(apap->ptap.rgtc[m_iCurrentCell - 1].brcRight.brcType));

  setlocale (LC_NUMERIC, old_locale);
  xxx_UT_DEBUGMSG(("propbuffer: %s \n",propBuffer.c_str()));

  const XML_Char* propsArray[3];
  propsArray[0] = static_cast<const XML_Char*>("props");
  propsArray[1] = propBuffer.c_str();
  propsArray[2] = NULL;

  getDoc()->appendStrux(PTX_SectionCell, propsArray);
  m_bInPara = false;

  xxx_UT_DEBUGMSG(("\t<CELL:%d:%d>", static_cast<int>(m_vecColumnSpansForCurrentRow.getNthItem(m_iCurrentCell - 1)), ps->vmerges[m_iCurrentRow - 1][m_iCurrentCell - 1]));
}

//--------------------------------------------------------------------------/
//--------------------------------------------------------------------------/

void IE_Imp_MsWord_97::_cell_close ()
{
  if (!m_bCellOpen)
    return;

  m_bCellOpen = false;
  getDoc()->appendStrux(PTX_EndCell, NULL);
  m_bInPara = false ;

  xxx_UT_DEBUGMSG(("</CELL>"));
}


void IE_Imp_MsWord_97::_generateCharProps(UT_String &s, const CHP * achp, wvParseStruct *ps)
{
	UT_String propBuffer;

	// set char tolower if fSmallCaps && fLowerCase
	if ( achp->fSmallCaps && achp->fLowerCase )
		m_bIsLower = true;
	else
		m_bIsLower = false;

	// set language based the lid - TODO: do we want to handle -none- differently?
	s += "lang:";

	unsigned short iLid = 0;
	// I am not sure how the various lids are supposed to work, but
	// achp->fBidi does not mean that the lidBidi is set ...
	if (achp->fBidi)
	{
		iLid = achp->lidBidi;
	}
	else if(ps->fib.fFarEast)
	{
		iLid = achp->lidFE;
	}
	else
	{
		iLid = achp->lid;
	}
	

	// if we do not have meaningful lid, try default ...
	if(!iLid)
		iLid = achp->lidDefault;
	
	s += wvLIDToLangConverter (iLid);
	s += ";";

	// decide best codepage based on the lid (as lang code above)
	UT_String codepage;
	if (achp->fBidi)
		codepage = wvLIDToCodePageConverter (achp->lidBidi);
	else if (!ps->fib.fFarEast)
		codepage = wvLIDToCodePageConverter (achp->lidDefault);
	else
		codepage = wvLIDToCodePageConverter (achp->lidFE);

	// watch out for codepage 0 = unicode
	const char * pNUE = XAP_EncodingManager::get_instance()->getNativeUnicodeEncodingName();

	if (codepage == "CP0")
		codepage = pNUE;
	
	// if this is the first codepage we've seen, use it.
	// if we see more than one different codepage in a document, use unicode.
	if (!getDoc()->getEncodingName())
		getDoc()->setEncodingName(codepage.c_str());
	else if (getDoc()->getEncodingName() != codepage)
		getDoc()->setEncodingName(pNUE);

	// bold text
	bool fBold = (achp->fBidi ? achp->fBoldBidi : achp->fBold);
	if (fBold) {
		s += "font-weight:bold;";
	}

	// italic text
	bool fItalic = (achp->fBidi ? achp->fItalicBidi : achp->fItalic);
	if (fItalic) {
		s += "font-style:italic;";
	}

	// foreground color
	U8 ico = (achp->fBidi ? achp->icoBidi : achp->ico);
	if (ico) {
		UT_String_sprintf(propBuffer, "color:%s;",
						  sMapIcoToColor(ico).c_str());
		s += propBuffer;
	}

	// underline and strike-through
	if (achp->fStrike || achp->kul) {
		s += "text-decoration:";
		if ((achp->fStrike || achp->fDStrike) && achp->kul) {
			s += "underline line-through;";
		} else if (achp->kul) {
			s += "underline;";
		} else {
			s += "line-through;";
		}
	}

	// background color
	if (achp->fHighlight) {
		UT_String_sprintf(propBuffer,"bgcolor:%s;",
						  sMapIcoToColor(achp->icoHighlight).c_str());
		s += propBuffer;
	}

	// superscript && subscript
	if (achp->iss == 1) {
		s += "text-position: superscript;";
	} else if (achp->iss == 2) {
		s += "text-position: subscript;";
	}

	if (achp->fVanish)
	{
	    s += "display:none;";
	}

	// font size (hps is half-points)
	// I have seen a bidi doc that had hpsBidi == 0, and the actual size in hps
	U16 hps = (achp->fBidi &&  achp->hpsBidi ? achp->hpsBidi : achp->hps);
	UT_String_sprintf(propBuffer,
					  "font-size:%dpt;", (int)(hps/2));
	s += propBuffer;

	// font family
	char *fname;

	// if the FarEast flag is set, use the FarEast font,
	// otherwise, we'll use the ASCII font.
	if (achp->fBidi) {
		fname = wvGetFontnameFromCode(&ps->fonts, achp->ftcBidi);
	} else if (!ps->fib.fFarEast) {
		fname = wvGetFontnameFromCode(&ps->fonts, achp->ftcAscii);
	} else {
		fname = wvGetFontnameFromCode(&ps->fonts, achp->ftcFE);

		if (strlen (fname) > 6)
			fname[6] = '\0';

		const char *f=XAP_EncodingManager::cjk_word_fontname_mapping.lookupByTarget(fname);

		if (f == fname)
		{
			FREEP (fname);
			fname = UT_strdup ("song");
		}
		else
		{
			FREEP (fname);
			fname = UT_strdup (f ? f : "helvetic");
		}
	}

	// there are times when we should use the third, Other font,
	// and the logic to know when somehow depends on the
	// character sets or encoding types? it's in the docs.

	UT_ASSERT_HARMLESS(fname != NULL);
	xxx_UT_DEBUGMSG(("font-family = %s\n", fname));

	s += "font-family:";

	if(fname)
		s += fname;
	else
		s += "Times New Roman";
	FREEP(fname);
}

void IE_Imp_MsWord_97::_generateParaProps(UT_String &s, const PAP * apap, wvParseStruct *ps)
{
	UT_String propBuffer;

	// DOM TODO: i think that this is right
	if (apap->fBidi == 1)
	{
		s += "dom-dir:rtl;";
	}
	else
	{
		s += "dom-dir:ltr;";
	}

	// paragraph alignment/justification
	switch(apap->jc)
	{
		case 0:
			s += "text-align:left;";
			break;
		case 1:
			s += "text-align:center;";
			break;
		case 2:
			s += "text-align:right;";
			break;
		case 3:
			s += "text-align:justify;";
			break;
		case 4:
			/* this type of justification is of unknown purpose and is
			 * undocumented , but it shows up in asian documents so someone
			 * should be able to tell me what it is someday
			 */
			s += "text-align:justify;";
			break;
	}

	// keep paragraph together?
	if (apap->fKeep) {
		s += "keep-together:yes;";
	}

	// keep with next paragraph?
	if (apap->fKeepFollow) {
		s += "keep-with-next:yes;";
	}

	// widowed/orphaned lines
	if (!apap->fWidowControl) {
		// these AbiWord properties give the same effect
		s += "orphans:0;widows:0;";
	}

	// line spacing (single-spaced, double-spaced, etc.)
	if (apap->lspd.fMultLinespace) {
		UT_String_sprintf(propBuffer,
						  "line-height:%s;",
						  UT_convertToDimensionlessString( (static_cast<float>(apap->lspd.dyaLine) / 240), "1.1"));
		s += propBuffer;
	} else {
		// TODO: handle exact line heights
	}

	//
	// margins
	//

	// margin-right
	if (apap->dxaRight) {
		UT_String_sprintf(propBuffer,
						  "margin-right:%s;",
						  UT_convertInchesToDimensionString(DIM_IN, (static_cast<float>(apap->dxaRight) / 1440),
															"1.4"));
		s += propBuffer;
	}

	// margin-left
	if (apap->dxaLeft) {
		UT_String_sprintf(propBuffer,
						  "margin-left:%s;",
						  UT_convertInchesToDimensionString(DIM_IN, (static_cast<float>(apap->dxaLeft) / 1440),
															"1.4"));
		s += propBuffer;
	}

	// margin-left first line (indent)
	if (apap->dxaLeft1) {
		UT_String_sprintf(propBuffer,
						  "text-indent:%s;",
						  UT_convertInchesToDimensionString(DIM_IN, (static_cast<float>(apap->dxaLeft1) / 1440),
															"1.4"));
		s += propBuffer;
	}

	// margin-top
	if (apap->dyaBefore) {
		UT_String_sprintf(propBuffer,
						  "margin-top:%dpt;", (apap->dyaBefore / 20));
		s += propBuffer;
	}

	// margin-bottom
	if (apap->dyaAfter) {
		UT_String_sprintf(propBuffer,
						  "margin-bottom:%dpt;", (apap->dyaAfter / 20));
	}

	// tab stops
	if (apap->itbdMac) {
		propBuffer += "tabstops:";

		for (int iTab = 0; iTab < apap->itbdMac; iTab++) {
			propBuffer += UT_String_sprintf(
											"%s/",
											UT_convertInchesToDimensionString(DIM_IN, ((static_cast<float>(apap->rgdxaTab[iTab]))
																					   / 1440), "1.4"));
			switch (apap->rgtbd[iTab].jc) {
				case 1:
					propBuffer += "C,";
					break;
				case 2:
					propBuffer += "R,";
					break;
				case 3:
					propBuffer += "D,";
					break;
				case 4:
					propBuffer += "B,";
					break;
				case 0:
				default:
					propBuffer += "L,";
					break;
			}
		}
		// replace final comma with a semi-colon
		propBuffer[propBuffer.size()-1] = ';';
		s += propBuffer;
	}


	// remove the trailing semi-colon
	s [s.size()-1] = 0;

}

/*!
    Translates MS numerical id's for standard styles into our names
	The style names that have been commented out are those that do not
    have currently a localised equivalent in AW
*/
static const XML_Char * s_translateStyleId(UT_uint32 id)
{
	if(id == 4094)
	{
		UT_DEBUGMSG(("Custom style\n"));
		UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
		return NULL;
	}

	// The style names that have been commented out are those that do
	// not currently have a localised equivalent in AW
	switch(id)
	{
		case 0:  return "Normal";
		case 1:  return "Heading 1";
		case 2:  return "Heading 2";
		case 3:  return "Heading 3";
		case 4:  return NULL /*"Heading 4"*/;
		case 5:  return NULL /*"Heading 5"*/;
		case 6:  return NULL /*"Heading 6"*/;
		case 7:  return NULL /*"Heading 7"*/;
		case 8:  return NULL /*"Heading 8"*/;
		case 9:  return NULL /*"Heading 9"*/;
			
		case 29: return "Footnote Text";

		case 31: return NULL /*"Header"*/;			
		case 32: return NULL /*"Footer"*/;
			
		case 34: return NULL /*"Caption"*/;
			
		case 36: return NULL /*"Envelope Address"*/;
		case 37: return NULL /*"Envelope Return"*/;
		case 38: return "Footnote Reference";

		case 41: return NULL /*"Page Number"*/;
		case 42: return "Endnote Reference";
		case 43: return "Endnote Text";

		case 65: return NULL /*"Default Paragraph Font"*/;
		case 66: return NULL /*"Body Text"*/;

		case 85: return NULL /*"Hyperlink"*/;

		default:
			UT_DEBUGMSG(("Unknown style Id [%d]; Please submit this document with a bug report!\n", id));
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return NULL;
	}
	return NULL;
}

/*! imports a stylesheet from our document */

#define PT_MAX_ATTRIBUTES 8
void IE_Imp_MsWord_97::_handleStyleSheet(const wvParseStruct *ps)
{
	UT_uint32 iCount = ps->stsh.Stshi.cstd;
	UT_uint16 iBase  = ps->stsh.Stshi.cbSTDBaseInFile;

	const XML_Char * attribs[PT_MAX_ATTRIBUTES*2 + 1];
	UT_uint32 iOffset = 0;
	
	const STD * pSTD = ps->stsh.std;
	const STD * pSTDBase = pSTD;
	UT_String props;

	for(UT_uint32 i = 0; i < iCount; i++, pSTD++)
	{
		iOffset = 0;

		if(!pSTD->xstzName)
		{
			continue;
		}

		UT_DEBUGMSG(("Style name: [%s], id: %d\n", pSTD->xstzName, pSTD->sti));

		attribs[iOffset++] = PT_NAME_ATTRIBUTE_NAME;

		// make sure we use standard names for standard styles
		const XML_Char * pName = NULL;
		if(pSTD->sti < 4094)
		{
			pName = s_translateStyleId(pSTD->sti);
		}

		if(pName)
		{
			attribs[iOffset++] = pName;
		}
		else
		{
			attribs[iOffset++] = pSTD->xstzName;
		}
		
		
		attribs[iOffset++] = PT_TYPE_ATTRIBUTE_NAME;
		if(pSTD->sgc == sgcChp)
		{
			attribs[iOffset++] = "C";
		}
		else
		{
			attribs[iOffset++] = "P";

			// also handle the followed-by, since that only applies to
			// paragraph style
			if(pSTD->istdNext != istdNil)
			{
				attribs[iOffset++] = PT_FOLLOWEDBY_ATTRIBUTE_NAME;
				attribs[iOffset++] = (pSTDBase + pSTD->istdNext)->xstzName;
			}
		}

		if(pSTD->istdBase != istdNil)
		{
			attribs[iOffset++] = PT_BASEDON_ATTRIBUTE_NAME;
			attribs[iOffset++] = (pSTDBase + pSTD->istdBase)->xstzName;
		}
		
		// now we want to generate props
		props.clear();

		wvParseStruct * PS = const_cast<wvParseStruct *>(ps);
		
		CHP achp;
		wvInitCHPFromIstd(&achp, (U16)i, &(PS->stsh));
		_generateCharProps(props,&achp,PS);

		if(props.size())
		{
			props += ";";
		}
		
		PAP apap;
		wvInitPAPFromIstd (&apap, (U16)i, &(PS->stsh));
		_generateParaProps(props,&apap,PS);

		// remove trailing semicolon
		if(props[props.size()-1] == ';')
		{
			props[props.size()-1] = 0;
		}
		
		xxx_UT_DEBUGMSG(("Style props: %s\n", props.c_str()));

		if(props.size())
		{
			attribs[iOffset++] = PT_PROPS_ATTRIBUTE_NAME;
			attribs[iOffset++] = props.c_str();
		}
		
		attribs[iOffset] = NULL;

		PD_Style * pStyle = NULL;
		if(getDoc()->getStyle(pSTD->xstzName, &pStyle))
		{
			xxx_UT_DEBUGMSG(("Redefining style %s\n", pSTD->xstzName));
			pStyle->addAttributes(attribs);
			pStyle->getBasedOn();
			pStyle->getFollowedBy();
		}
		else
		{
			getDoc()->appendStyle(attribs);
		}
	}
}
